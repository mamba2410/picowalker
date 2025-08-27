#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <stdio.h>
#include "hardware/uart.h"
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/pio.h>
#include <hardware/irq.h>
#include "pico/time.h"
#include "pico/stdlib.h"

#include "ir_pico_pio.h"

#define USE_DMA

/*
 * PIO code adapted from dmitry.gr
 */
#define SIDE_SET_HAS_ENABLE_BIT 	0
#define SIDE_SET_NUM_BITS			0
#define DEFINE_PIO_INSTRS
#include "pioAsm.h"
#undef DEFINE_PIO_INSTRS

#define CPU_CLOCK_RATE                  150000000 // 150 MHz default
#define RX_MACHINERY_CLOCK				10000000		//fast enough for high rates, slow enough that in-pio-instr delays are enough for 1us

#define NUM_INSTRS_WE_NEED				9
#define NUM_SMS_WE_NEED					1
#define NUM_DMAS_WE_NEED				0

#define IR_DMA_IRQ_NUM 0

//#define CIRC_BUF_SZ						64
#define CIRC_BUF_LEN    (128+8+1)

#define FLAT_BUF_LEN    (128+8+1)

// Callback function analogous to `RepalmUartRxF`
// the first void* is the `mIrRxD` context, likely unused
typedef void (*rx_callback_t)(void*, uint16_t*, size_t);

struct pw_ir_circ_buf_s {
    uint16_t write;
    uint16_t read;
    uint16_t data[CIRC_BUF_LEN];
};
static volatile struct pw_ir_circ_buf_s g_ir_pio_circ_buf;

static volatile uint16_t g_ir_pio_flat_buf[FLAT_BUF_LEN];

struct pw_ir_pio_state_s {
    // PIO and DMA admin
    uint8_t pio_sm;
    uint8_t pio_start_pc;
    pio_hw_t *pio_hw;
    uint dma_chan;
    
    // Serial things
    // probably gonna be hardcoded
    uint8_t data_bits;
    uint8_t parity;
    uint8_t stop_bits;
    uint32_t baudrate;

    // Module state
    bool state_tx;
    bool state_rx;

    // User config stuff
    rx_callback_t user_rx_callback;
};
static volatile struct pw_ir_pio_state_s g_ir_pio_state;

static uint16_t g_rx_buffer[CIRC_BUF_LEN];

static bool pw_ir_pio_tx_is_ongoing(void);

/*
 * Checks if the global TX circular buffer is full
 * Equivalent of `palmcardIrPrvCircBufIsFull()`
 */
static bool pw_ir_pio_circ_buf_is_full(void) {
    uint8_t next_write = ((g_ir_pio_circ_buf.write + 1) == CIRC_BUF_LEN) ? 0 : (g_ir_pio_circ_buf.write + 1);
    return next_write != g_ir_pio_circ_buf.read;
}

/*
 * Cecks if the global circular buffer is empty
 */
static bool pw_ir_pio_circ_buf_is_empty(void) {
    return g_ir_pio_circ_buf.write == g_ir_pio_circ_buf.read;
}

/*
 * Adds a singular value to the global TX circular buffer
 * Equivalent of `palmcardIrPrvCircBufAdd()`
 */
static bool pw_ir_pio_circ_buf_add(uint16_t val) {
    uint8_t next_write = ((g_ir_pio_circ_buf.write + 1) == CIRC_BUF_LEN) ? 0 : (g_ir_pio_circ_buf.write + 1);

    if(next_write == g_ir_pio_circ_buf.read) { return false; }

    g_ir_pio_circ_buf.data[g_ir_pio_circ_buf.write] = val;
    g_ir_pio_circ_buf.write = next_write;

    return true;
}

/*
 * Removes and returns a single value from the global TX circular buffer
 * Returns `-1` as `int32_t` if the buffer was empty, else returns the 
 * `uint16_t` as a `uint32_t`
 * Equivalent of `palmcardIrPrvCircBufGet()`
 */
static int32_t pw_ir_pio_circ_buf_get(void) {
    uint8_t next_read = g_ir_pio_circ_buf.read;

    if(next_read == g_ir_pio_circ_buf.write) { return -1; }

    uint16_t ret = g_ir_pio_circ_buf.data[next_read];
    g_ir_pio_circ_buf.read = ((next_read + 1) == CIRC_BUF_LEN) ? 0 : (next_read + 1);

    return (uint32_t)ret;

}

/*
 * Resets the PIO and module state
 * TODO: Dependancy on PIO1 for IRQ
 * Equivalent of `palmcardIrPrvUnsetup()`
 */
static void pw_ir_pio_reset_state() {

    if(g_ir_pio_state.state_tx) {
        while( pw_ir_pio_tx_is_ongoing() );
    }

	//NVIC_DisableIRQ(PIO1_0_IRQn);
    irq_set_enabled(PIO1_IRQ_0, false);
	
	//stop SM
	g_ir_pio_state.pio_hw->ctrl &=~ ((1 << PIO_CTRL_SM_ENABLE_LSB) << g_ir_pio_state.pio_sm);
	
	//reset SM
	g_ir_pio_state.pio_hw->ctrl |= ((1 << PIO_CTRL_SM_RESTART_LSB) << g_ir_pio_state.pio_sm);
	
	//wait
	while (g_ir_pio_state.pio_hw->ctrl & ((1 << PIO_CTRL_SM_RESTART_LSB) << g_ir_pio_state.pio_sm))

	//nop
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].instr = I_MOV(0, 0, MOV_DST_X, MOV_OP_COPY, MOV_SRC_X);

	//clear fifos
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].shiftctrl = PIO_SM0_SHIFTCTRL_FJOIN_RX_BITS;
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].shiftctrl = PIO_SM0_SHIFTCTRL_FJOIN_TX_BITS;
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].shiftctrl = 0;
	
	//clear buffer
	g_ir_pio_circ_buf.write = 0;
	g_ir_pio_circ_buf.read = 0;
	
	//clear state
	g_ir_pio_state.state_tx = false;
	g_ir_pio_state.state_rx = false;
	
	g_ir_pio_state.pio_hw->inte0 = 0;
	//NVIC_ClearPendingIRQ(PIO1_0_IRQn);
    irq_clear(PIO1_IRQ_0);

    // clear DMA
    dma_irqn_set_channel_enabled(IR_DMA_IRQ_NUM, g_ir_pio_state.dma_chan, false);
    //dma_channel_unclaim(g_ir_pio_state.dma_chan);
}

/*
 * Sets up PIO SM to run in TX mode and starts the SM
 * Feed it with `pw_ir_pio_send_data()` 
 * Equivalent of `palmcardIrPrvSetupTx()`
 */
static void pw_ir_pio_setup_tx() {

	uint_fast8_t pc = g_ir_pio_state.pio_start_pc, start_pc, restart_pc, end_pc, jmp_dst;
    uint32_t baudrate = g_ir_pio_state.baudrate;
	
	//TX operates in non-low-power mode and needs a clock of 16x bandwidth
	//data needs to be inserted precisely as sent, eg for 8n1, insert (0x01 + ~data << 1) where 1 is the start bit, total bitlength should be programmed into PIO, shift right
	//code is more complex than you'd think to allow a SURE determination when TX is done (by examining pc)
	
	start_pc = restart_pc = pc;
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_PULL(0, 0, 0, 1);
    //                                                8   N   1   Start
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_SET(0, 0, SET_DST_X, 8 + 0 + 1 + 1 - 1);
	
	jmp_dst = pc;
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_OUT(2, 0, OUT_DST_PINS, 1);
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_SET(11, 0, SET_DST_PINS, 0);
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_JMP(0, 0, JMP_X_POSTDEC, jmp_dst);
	end_pc = pc - 1;
	
	//configure.pio_sm0
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].clkdiv = ((CPU_CLOCK_RATE / 16 * 256ull + baudrate / 2) / baudrate) << PIO_SM0_CLKDIV_FRAC_LSB;
	g_ir_pio_state.pio_hw->ctrl |= ((1 << PIO_CTRL_CLKDIV_RESTART_LSB) << g_ir_pio_state.pio_sm);
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].execctrl = (g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].execctrl &~ (PIO_SM0_EXECCTRL_WRAP_TOP_BITS | PIO_SM0_EXECCTRL_WRAP_BOTTOM_BITS | PIO_SM2_EXECCTRL_SIDE_EN_BITS)) |(end_pc << PIO_SM0_EXECCTRL_WRAP_TOP_LSB) | (restart_pc << PIO_SM0_EXECCTRL_WRAP_BOTTOM_LSB) | (SIDE_SET_HAS_ENABLE_BIT ? PIO_SM2_EXECCTRL_SIDE_EN_BITS : 0);
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].shiftctrl = (g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].shiftctrl &~ (PIO_SM1_SHIFTCTRL_PULL_THRESH_BITS | PIO_SM1_SHIFTCTRL_PUSH_THRESH_BITS | PIO_SM0_SHIFTCTRL_IN_SHIFTDIR_BITS | PIO_SM0_SHIFTCTRL_AUTOPUSH_BITS | PIO_SM0_SHIFTCTRL_AUTOPULL_BITS)) | PIO_SM0_SHIFTCTRL_OUT_SHIFTDIR_BITS | PIO_SM0_SHIFTCTRL_FJOIN_TX_BITS;
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].pinctrl = (SIDE_SET_BITS_USED << PIO_SM1_PINCTRL_SIDESET_COUNT_LSB) | (1 << PIO_SM1_PINCTRL_OUT_COUNT_LSB) | (PIN_IRDA_OUT << PIO_SM1_PINCTRL_OUT_BASE_LSB) | (1 << PIO_SM1_PINCTRL_SET_COUNT_LSB)| (PIN_IRDA_OUT << PIO_SM1_PINCTRL_SET_BASE_LSB);

	//set out direction
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].instr = I_SET(0, 0, SET_DST_PINDIRS, 1);
	
	//logi("starting TX SM\n");
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].instr = I_JMP(0, 0, JMP_ALWAYS, start_pc);
	g_ir_pio_state.pio_hw->ctrl |= ((1 << PIO_CTRL_SM_ENABLE_LSB) << g_ir_pio_state.pio_sm);
	
    
    /*
	//irq on TX not full, but not enabled since it is empty now and we have no data
	g_ir_pio_state.pio_hw->inte0 = 0;
	//NVIC_ClearPendingIRQ(PIO1_0_IRQn);
	//NVIC_EnableIRQ(PIO1_0_IRQn);
    irq_clear(PIO1_IRQ_0);
    pio_interrupt_clear(g_ir_pio_state.pio_hw, g_ir_pio_state.pio_sm);
    irq_set_enabled(PIO1_IRQ_0, true);
	g_ir_pio_state.pio_hw->inte0 = PIO_IRQ0_INTE_SM0_TXNFULL_BITS << g_ir_pio_state.pio_sm;
    */

    // Disable IRQ
	g_ir_pio_state.pio_hw->inte0 = 0;

    // Set up DMA for TX
    dma_irqn_set_channel_enabled(IR_DMA_IRQ_NUM, g_ir_pio_state.dma_chan, true);
    dma_channel_config config_tx = dma_channel_get_default_config(g_ir_pio_state.dma_chan);
    channel_config_set_transfer_data_size(&config_tx, DMA_SIZE_16);
    channel_config_set_read_increment(&config_tx, true);
    channel_config_set_write_increment(&config_tx, false);
    channel_config_set_dreq(&config_tx, pio_get_dreq(g_ir_pio_state.pio_hw, g_ir_pio_state.pio_sm, true));
    dma_channel_configure(
        g_ir_pio_state.dma_chan,
        &config_tx,
        &g_ir_pio_state.pio_hw->txf[g_ir_pio_state.pio_sm],
        g_ir_pio_flat_buf,
        FLAT_BUF_LEN,
        false // Don't start yet
    );

    g_ir_pio_state.state_tx = true;
}


/*
 * Sets up PIO SM to run in RX mode and starts the SM
 * Data gets sent out via an interrupt.
 * Timeouts and determining when air is empty is up to the caller.
 * Equivalent of `palmcardIrPrvSetupRx()`
 */
static void pw_ir_pio_setup_rx() {
	uint32_t bitcounterValue = (RX_MACHINERY_CLOCK + g_ir_pio_state.baudrate / 2) / g_ir_pio_state.baudrate - 4;
	uint_fast8_t pc = g_ir_pio_state.pio_start_pc, startPC, restartPC, endPC, jmpDest1, jmpDest2;
	
    // 8N1
    uint8_t dataBits = g_ir_pio_state.data_bits;
    uint8_t parityBits = 0;
    uint8_t stopBits = g_ir_pio_state.stop_bits;
	
	//1.6276 us is the pulse width of 115,200 and of low power IrDA, so we wait for a low, check again in 1.5 us, and if it is still low, we consider this a start bit
	//clock should be ~20MHz, shifter should be to the right, autopush at 32 bits. autopull at 32 bits, input should be an infinite stream of words that represent the
	//inter-bit delay, 20e6 / baudrate - 33
	
	
	restartPC = startPC = pc;
	jmpDest1 = pc;
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_WAIT(10, 0, 0, WAIT_FOR_GPIO, PIN_IRDA_IN);			//wait for low
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_JMP(0, 0, JMP_PIN, jmpDest1);							//1us later, if high now, consider it a glitch
		
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_SET(0, 0, SET_DST_Y, dataBits + stopBits + parityBits - 1);			//num bits per transport unit minus two
	
	jmpDest1 = pc;
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_MOV(0, 0, MOV_DST_X, MOV_OP_COPY, IN_SRC_OSR);
	
	jmpDest2 = pc;
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_JMP(0, 0, JMP_X_POSTDEC, jmpDest2);					//delay
	
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_IN(0, 0, IN_SRC_PINS, 1);
	
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_JMP(0, 0, JMP_Y_POSTDEC, jmpDest1);
	
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_IN(0, 0, IN_SRC_ZEROES, 32 - dataBits - stopBits - parityBits);
	
	g_ir_pio_state.pio_hw->instr_mem[pc++] = I_WAIT(4, 0, 1, WAIT_FOR_GPIO, PIN_IRDA_IN);				//wait for high
	endPC = pc - 1;
	
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].clkdiv = ((CPU_CLOCK_RATE * 256ull + RX_MACHINERY_CLOCK / 2) / RX_MACHINERY_CLOCK) << PIO_SM0_CLKDIV_FRAC_LSB;
	g_ir_pio_state.pio_hw->ctrl |= ((1 << PIO_CTRL_CLKDIV_RESTART_LSB) << g_ir_pio_state.pio_sm);
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].execctrl = (g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].execctrl &~ (PIO_SM0_EXECCTRL_WRAP_TOP_BITS | PIO_SM0_EXECCTRL_WRAP_BOTTOM_BITS | PIO_SM2_EXECCTRL_SIDE_EN_BITS)) |(endPC << PIO_SM0_EXECCTRL_WRAP_TOP_LSB) | (restartPC << PIO_SM0_EXECCTRL_WRAP_BOTTOM_LSB) | (SIDE_SET_HAS_ENABLE_BIT ? PIO_SM2_EXECCTRL_SIDE_EN_BITS : 0) | (PIN_IRDA_IN << PIO_SM2_EXECCTRL_JMP_PIN_LSB);
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].shiftctrl = (g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].shiftctrl &~ (PIO_SM1_SHIFTCTRL_PULL_THRESH_BITS | PIO_SM1_SHIFTCTRL_PUSH_THRESH_BITS | PIO_SM0_SHIFTCTRL_OUT_SHIFTDIR_BITS | PIO_SM0_SHIFTCTRL_AUTOPULL_BITS)) | PIO_SM0_SHIFTCTRL_IN_SHIFTDIR_BITS | PIO_SM0_SHIFTCTRL_AUTOPUSH_BITS;
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].pinctrl = (SIDE_SET_BITS_USED << PIO_SM1_PINCTRL_SIDESET_COUNT_LSB) | (PIN_IRDA_IN << PIO_SM1_PINCTRL_IN_BASE_LSB);
	
	
	//prepare OSR for SM1
	g_ir_pio_state.pio_hw->txf[g_ir_pio_state.pio_sm] = bitcounterValue;
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].instr = I_PULL(0, 0, 0, 0);
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].shiftctrl |= PIO_SM0_SHIFTCTRL_FJOIN_RX_BITS;
	
	//logi("starting RX SM\n");
	g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].instr = I_JMP(0, 0, JMP_ALWAYS, startPC);
	g_ir_pio_state.pio_hw->ctrl |= ((1 << PIO_CTRL_SM_ENABLE_LSB) << g_ir_pio_state.pio_sm);
	
	//irq on RX not empty
	g_ir_pio_state.pio_hw->inte0 = 0;
	//NVIC_ClearPendingIRQ(PIO1_0_IRQn);
	//NVIC_EnableIRQ(PIO1_0_IRQn);
    irq_clear(PIO1_IRQ_0);
    pio_interrupt_clear(g_ir_pio_state.pio_hw, g_ir_pio_state.pio_sm);
    irq_set_enabled(PIO1_IRQ_0, true);
	g_ir_pio_state.pio_hw->inte0 = PIO_IRQ0_INTE_SM0_RXNEMPTY_BITS << g_ir_pio_state.pio_sm;

    g_ir_pio_state.state_rx = true;

}

/*
 * Process input from PIO RX machine which has stop and potentially parity bits
 * In 8N1 mode, the bottom 8 bits will be the data and the top bits contain flags which the caller can choose to accept or reject
 * TODO: rename to decode
 * Equivalent of `palmcardIrPrvProcessInput()`
 */
static uint16_t pw_ir_pio_process_input(uint32_t val) {
    uint16_t ret = 0;
    uint16_t stop_bit_mask = ((1<<g_ir_pio_state.stop_bits) - 1) << g_ir_pio_state.data_bits;
    uint16_t data_val_mask = (1<<g_ir_pio_state.data_bits) - 1;
    uint16_t data_val = val & data_val_mask;

    // Don't bother with parity since we don't use it

    // Check stop bit
    if( (val & stop_bit_mask) != stop_bit_mask) {
        ret |= PW_IR_PIO_FRAME_ERROR_BIT;
    }

    // Add in data bits
    ret |= data_val;

    return ret;
}


/*
 * IRQ callback for the PIO SM. Performs different functions in TX and RX modes.
 * TX mode: place processed value (containing stop bits, parity etc) from global circular buffer into the PIO TX FIFO
 * RX mode: grab value from RX FIFO, processes it and calls the user callback to do whatever they want with it
 */
void __attribute__((used)) pw_ir_pio_irq_handler() {
    if(g_ir_pio_state.state_tx) {
        // Expecting a TXNFULL event so we take transformed data from the
        // circ buffer and feed it byte-by-byte into the PIO FIFO
        // If circ buffer is empty, cancel the interrupt to stop feeding

		while (!(g_ir_pio_state.pio_hw->fstat & ((1 << PIO_FSTAT_TXFULL_LSB) << g_ir_pio_state.pio_sm))) {  //space in fifo?
            int32_t val = pw_ir_pio_circ_buf_get();

			if (val < 0) { //no more data
                // Stop interrupts
				pio1_hw->inte0 = 0;
				break;
			} else { //have data
				g_ir_pio_state.pio_hw->txf[g_ir_pio_state.pio_sm] = val;
			}
		}

    } else if(g_ir_pio_state.state_rx) {
		uint_fast8_t nItems = 0;
		//uint16_t buf[29];
	
        // While there is data in the RX buffer and `buf` isn't full
		while (!(g_ir_pio_state.pio_hw->fstat & ((1 << PIO_FSTAT_RXEMPTY_LSB) << g_ir_pio_state.pio_sm)) && nItems < CIRC_BUF_LEN) {		//data & space in fifo?
			
			uint16_t input = g_ir_pio_state.pio_hw->rxf[g_ir_pio_state.pio_sm];
            uint16_t val = pw_ir_pio_process_input(input);
			
			g_rx_buffer[nItems++] = val;
		}

        // Call user callback
        if(nItems > 0) {
		    g_ir_pio_state.user_rx_callback((void*)0, g_rx_buffer, nItems);
        }

    } else {
        // spurious
		g_ir_pio_state.pio_hw->inte0 = 0;
    }
}


/*
 * Turns raw bytes ito data to be sent out over PIO (calcs parity, start bit, stop bits)
 * TODO: rename to encode
 * Equivalent of `palmcardIrPrvXformData()`
 */
static uint16_t pw_ir_pio_transform_data(uint8_t byte) {
    uint32_t val = 1 + ((uint32_t)((uint8_t)~byte)) * 2; // start bit and data
    
    // we don't do parity

    // stop bits are zero here and thus no work to do for them

    return val;
}

/*
 * Take data, add start/stop/parity bits then feed it to circular buffer
 * Crucially sets interrupt for when PIO TX isn't full. This calls interrupt handler
 * Returns the number of bytes added to the buffer. User should check the return value
 */
static uint32_t __attribute__((noinline)) pw_ir_pio_serial_tx_blocking(const uint8_t *data, size_t len) {
    size_t len_orig = len;

    if(!data) return 0;
    if(!g_ir_pio_state.state_tx) return 0;

    while(len != 0) {
        uint16_t transformed_data = pw_ir_pio_transform_data(*data);
        bool add_success = pw_ir_pio_circ_buf_add(transformed_data);
        if(add_success) {
            g_ir_pio_state.pio_hw->inte0 = PIO_IRQ0_INTE_SM0_TXNFULL_BITS << g_ir_pio_state.pio_sm;
            len--;
            data++;
        } else {
            printf("[Error] PIO TX add failed\n");
        }
    }

    return len_orig - len;
}


/*
 *
 */
static uint32_t __attribute__((noinline)) pw_ir_pio_serial_tx_dma(const uint8_t *data, size_t len) {
    if(!data) return 0;
    if(!g_ir_pio_state.state_tx) return 0;

    // TODO: Set up DMA? Channel should be configured when putting into TX mode

    size_t i = 0;
    for(i = 0; i < len; data++, i++) {
        uint16_t transformed_data = pw_ir_pio_transform_data(*data);
        g_ir_pio_flat_buf[i] = transformed_data;
        //g_ir_pio_state.pio_hw->inte0 = PIO_IRQ0_INTE_SM0_TXNFULL_BITS << g_ir_pio_state.pio_sm;
    }

    // TODO: Start DMA transfer
    dma_channel_transfer_from_buffer_now(g_ir_pio_state.dma_chan, g_ir_pio_flat_buf, i);
    dma_channel_wait_for_finish_blocking(g_ir_pio_state.dma_chan);

    return len - i;

}


/*
 * Returns `true` if there is a TX ongoing
 * Equivalent of `palmcardIrPrvIsTxOngoing()`
 */
static bool pw_ir_pio_tx_is_ongoing(void) {

    // We must be in TX mode for TX to be ongoing
    if(!g_ir_pio_state.state_tx)
        return false;

    // If ints are on (and we are in TX mode) then TX is ongoing
    if(g_ir_pio_state.pio_hw->inte0)
        return true;

    // If the TX FIFO is not empty, TX is ongoing
	if (!(g_ir_pio_state.pio_hw->fstat & ((1 << PIO_FSTAT_TXEMPTY_LSB) << g_ir_pio_state.pio_sm)))
        return true;

    // If PC has not yet reached our "parking PC then TX is still ongoing
	if (g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].addr != g_ir_pio_state.pio_start_pc)
		return true;

    // We could have read the above just as the SM read the last word from the FIFO, so recheck again
	if (g_ir_pio_state.pio_hw->sm[g_ir_pio_state.pio_sm].addr != g_ir_pio_state.pio_start_pc)
		return true;

    return false;

}

/*
static bool palmcardIrPrvConfig(union UartCfg *cfg, RepalmUartRxF rxf, void *userData)
{
	if (mCurTx) {	//wait for TX to be done
	
		while(palmcardIrPrvIsTxOngoing());
	}
	
	palmcardIrPrvUnsetup();
	mIrRxF = NULL;
	
	
	if (cfg->rxEn && cfg->txEn)		//invalid config
		return false;
	
	if (!cfg->rxEn && !cfg->txEn) {	//off
		
		i2cIoIrdaEnable(false);
		return true;
	}
	
	cfg->baudrate = palmcardIrPrvCalcProperBaudrate(cfg->baudrate);
	if (!cfg->baudrate)
		return false;
	
	if (!cfg->parEna)
		mParityFunc = NULL;
	else
		mParityFunc = cfg->parEven ? palmcardIrPrvEvenParity : palmcardIrPrvOddParity;
	
	mStopBits = 1 + cfg->stopBits / 2;	//round up to full bits
	mDataBits = 5 + cfg->charBits;
	cfg->stopBits = 2 * mStopBits - 1;
	
	if (cfg->rxEn) {
		
		mCurRx = true;
		palmcardIrPrvSetupRx(cfg->baudrate, mDataBits, mParityFunc ? 1 : 0, mStopBits);
	}
	else if (cfg->txEn) {
		
		mCurTx = true;
		palmcardIrPrvSetupTx(cfg->baudrate, mDataBits, mParityFunc ? 1 : 0, mStopBits);
	}
	
	i2cIoIrdaEnable(true);
	
	asm volatile("":::"memory");
	mIrRxD = userData;
	asm volatile("":::"memory");
	mIrRxF = rxf;

	return true;
}
*/

/*
 * Checks if TX FIFO is full/empty and checks if RX FIFO is empty
 */
/*
static uint32_t palmcardIrPrvGetSta(void)
{
	uint32_t ret = 0;
	
	if (mCurTx) {
		
		if (!palmcardIrPrvIsTxOngoing())
			ret += UART_STA_BIT_TX_FIFO_EMPTY;
		
		if (palmcardIrPrvCircBufIsFull())
			ret += UART_STA_BIT_TX_FIFO_FULL;
		
		ret += UART_STA_BIT_RX_FIFO_EMPTY;
	}
	else if (mCurRx) {
		
		ret += UART_STA_BIT_TX_FIFO_EMPTY;
		
		if (pio1_hw->fstat & ((1 << PIO_FSTAT_RXEMPTY_LSB) << mMySm))
			ret += UART_STA_BIT_RX_FIFO_EMPTY;
	}
	
	return ret;
}
*/


/*
bool palmcardIrSetup(uint8_t *firstFreeSmP, uint8_t *firstFreePioInstrP, uint8_t *firstFreeDmaChP, uint8_t nDmaCh, uint8_t nPioSms, uint8_t nPioInstrs)
{
	if (*firstFreeSmP > nPioSms - NUM_SMS_WE_NEED || *firstFreeDmaChP > nDmaCh - NUM_DMAS_WE_NEED || *firstFreePioInstrP > nPioInstrs - NUM_INSTRS_WE_NEED)
		return false;
	
	mMySm = *firstFreeSmP;
	mMyStartPc = *firstFreePioInstrP;
	mMyFirstDmaChannel = *firstFreeDmaChP;
	
	(*firstFreeSmP) += NUM_SMS_WE_NEED;
	(*firstFreePioInstrP) += NUM_INSTRS_WE_NEED;
	(*firstFreeDmaChP) += NUM_DMAS_WE_NEED;
	
	palmcardIrPrvUnsetup();
	
	
	if (!ralSetRePalmTabFunc(REPALM_FUNC_IDX_UART_CONFIG, &repalmUartConfig))
		return false;
	if (!ralSetRePalmTabFunc(REPALM_FUNC_IDX_UART_TX, &repalmUartTx))
		return false;
	if (!ralSetRePalmTabFunc(REPALM_FUNC_IDX_UART_GET_STA, &repalmUartGetSta))
		return false;
	
	
	return true;
}
*/

/*
 * On receive data, throw it into the global circular buffer
 * Safe because by the time we want to fill it with TX data, we are done with RX
 */
void pw_ir_pio_rx_callback(void* _context, uint16_t *data, size_t len) {
    for(size_t i = 0; i < len; i++) {
        if( !(data[i] & PW_IR_PIO_FRAME_ERROR_BIT) )
            pw_ir_pio_circ_buf_add(data[i]);
    }
}

/*
 * ============================================================================
 * Functions for picowalker operation
 * ============================================================================
 */

int pw_ir_read(uint8_t *buf, size_t max_len) {
    size_t cursor = 0;
    int64_t diff;
    
    // Reset RX buffer
    g_ir_pio_circ_buf.read = g_ir_pio_circ_buf.write = 0;

    // Set up PIO SM as RX mode
    pw_ir_pio_setup_rx();

    // Spin until either something is in the callback buffer or 50ms has elapsed
    volatile absolute_time_t start, now, last_read;
    start = get_absolute_time();
    do {
        now = get_absolute_time();
        diff = absolute_time_diff_us(start, now);
    } while( pw_ir_pio_circ_buf_is_empty() && diff < 50000);

    // If something did come through, collect data until time since last byte exceeds 3742us
    diff = 0;
    last_read = get_absolute_time();
    do {
        if(!pw_ir_pio_circ_buf_is_empty()) {
		    buf[cursor] = (uint8_t)(pw_ir_pio_circ_buf_get() & 0xff);
		    cursor++;
            last_read = get_absolute_time();
        }
        now = get_absolute_time();
        diff = absolute_time_diff_us(last_read, now); // signed difference
    } while( diff < 3742);
    

    // (unset PIO RX mode?)
    pw_ir_pio_reset_state();

    /*
    // Debug
    printf("read: (%d)", cursor);
    for(size_t i = 0; i < cursor; i++) {
        if(i%16 == 0) printf("\n");
        if(i%i == 0)  printf(" ");
        printf("%02x", buf[i]^0xaa);
    }
    printf("\n");
    */

    // return number of bytes read
    return cursor;
}


int pw_ir_write(uint8_t *buf, size_t len) {
	
    /*
     * Set up PIO SM as TX mode
     * Feed TX cirvc buffer
     * ???
     * profit
     * (unset PIO TX mode)
     */
    pw_ir_pio_setup_tx();
    //pw_ir_pio_serial_tx_blocking(buf, len);
    pw_ir_pio_serial_tx_dma(buf, len);
    pw_ir_pio_reset_state();

    /*
    // Debug
    printf("write: (%d)", len);
    for(size_t i = 0; i < len; i++) {
        if(i%16 == 0) printf("\n");
        if(i%i == 0)  printf(" ");
        printf("%02x", buf[i]^0xaa);
    }
    printf("\n");
    */

    // TODO
    return len;
}

void pw_ir_init() {
    // TODO: replace with PIO code
    
    // ONCE: Set up IR shutdown pin
    gpio_init(IR_SD_PIN);
    gpio_set_dir(IR_SD_PIN, GPIO_OUT);

    pio_gpio_init(pio1, IR_PIO_TX);
    gpio_init(IR_PIO_RX);
    gpio_set_dir(IR_PIO_RX, GPIO_IN);

    // De-assert IR_SD
    gpio_put(IR_SD_PIN, 0);

    // Set `g_ir_pio_state` for callbacks, baud rate, etc.

    g_ir_pio_state = (struct pw_ir_pio_state_s){
        .pio_sm = 0,
        .dma_chan = 0,
        .pio_start_pc = 0,
        .pio_hw = pio1,

        .data_bits = 8,
        .parity = 0,
        .stop_bits = 1,
        .baudrate = 115200,

        .state_tx = false,
        .state_rx = false,

        .user_rx_callback = pw_ir_pio_rx_callback,

    };

    // Set IRQ handler
    irq_set_exclusive_handler(PIO1_IRQ_0, pw_ir_pio_irq_handler);

    // unset PIO mode to start PIO SM
    pw_ir_pio_reset_state();

    // done?

}

void pw_ir_clear_rx() {
    // Remove all data in RX buffer
    g_ir_pio_circ_buf.read = g_ir_pio_circ_buf.write = 0;
}

void pw_ir_deinit() {
    /* TODO: power saving (IR shutdown, stop uart1, etc.) */

    // unset PIO mode/stop PIO SM
    // Assert IR_SD
}

void pw_ir_sleep() {
    gpio_put(IR_SD_PIN, 1);
}

void pw_ir_wake() {
    gpio_put(IR_SD_PIN, 0);
}

