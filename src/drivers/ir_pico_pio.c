#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <hardware/uart.h>
#include <hardware/gpio.h>
#include "pico/time.h"

#include "ir_pico_pio.h"

/*
 * PIO code adapted from dmitry.gr
 */
#define SIDE_SET_HAS_ENABLE_BIT 	0
#define SIDE_SET_NUM_BITS			0
#define DEFINE_PIO_INSTRS
#include "pioAsm.h"
#undef DEFINE_PIO_INSTRS

#define RX_MACHINERY_CLOCK				10000000		//fast enough for high rates, slow enough that in-pio-instr delays are enough for 1us

#define NUM_INSTRS_WE_NEED				9
#define NUM_SMS_WE_NEED					1
#define NUM_DMAS_WE_NEED				0

#define CIRC_BUF_SZ						64


static uint8_t mMySm, mMyFirstDmaChannel, mMyStartPc;
static uint_fast8_t (*mParityFunc)(uint_fast8_t val);
static volatile uint16_t mCircBuf[CIRC_BUF_SZ];
static volatile uint8_t mCircBufW, mCircBufR;	//equals means empty
static uint8_t mDataBits, mStopBits;
static RepalmUartRxF mIrRxF;
static volatile bool mCurTx, mCurRx;
static void *mIrRxD;


static bool palmcardIrPrvCircBufIsFull(void)
{
	uint8_t mCircBufWnext = (mCircBufW + 1 == CIRC_BUF_SZ ? 0 : mCircBufW + 1);
	
	return mCircBufWnext != mCircBufR;
}

static bool palmcardIrPrvCircBufAdd(uint_fast16_t val)
{
	uint_fast8_t cirBufWnow = mCircBufW, circBufWnext = (cirBufWnow + 1 == CIRC_BUF_SZ ? 0 : cirBufWnow + 1);
	
	if (circBufWnext == mCircBufR)
		return false;
	
	mCircBuf[cirBufWnow] = val;
	mCircBufW = circBufWnext;

	return true;
}

static int32_t palmcardIrPrvCircBufGet(void)
{
	uint_fast8_t cirBufRnow = mCircBufR;
	uint_fast16_t ret;
	
	if (cirBufRnow == mCircBufW)
		return -1;
	
	ret = mCircBuf[cirBufRnow];
	mCircBufR = (cirBufRnow + 1 == CIRC_BUF_SZ ? 0 : cirBufRnow + 1);
	
	return (uint32_t)ret;
}

static void palmcardIrPrvUnsetup(void)
{
	NVIC_DisableIRQ(PIO1_0_IRQn);
	
	//stop SM
	pio1_hw->ctrl &=~ ((1 << PIO_CTRL_SM_ENABLE_LSB) << mMySm);
	
	//reset SM
	pio1_hw->ctrl |= ((1 << PIO_CTRL_SM_RESTART_LSB) << mMySm);
	
	//wait
	while (pio1_hw->ctrl & ((1 << PIO_CTRL_SM_RESTART_LSB) << mMySm))

	//nop
	pio1_hw->sm[mMySm].instr = I_MOV(0, 0, MOV_DST_X, MOV_OP_COPY, MOV_SRC_X);

	//clear fifos
	pio1_hw->sm[mMySm].shiftctrl = PIO_SM0_SHIFTCTRL_FJOIN_RX_BITS;
	pio1_hw->sm[mMySm].shiftctrl = PIO_SM0_SHIFTCTRL_FJOIN_TX_BITS;
	pio1_hw->sm[mMySm].shiftctrl = 0;
	
	//clear buffer
	mCircBufW = 0;
	mCircBufR = 0;
	
	//clear state
	mCurTx = false;
	mCurRx = false;
	
	pio1_hw->inte0 = 0;
	NVIC_ClearPendingIRQ(PIO1_0_IRQn);
}

static void palmcardIrPrvSetupTx(uint32_t baudrate, uint_fast8_t dataBits, uint_fast8_t parityBits, uint_fast8_t stopBits)
{
	uint_fast8_t pc = mMyStartPc, startPC, restartPC, endPC, jmpDest;
	
	
	//TX operates in non-low-power mode and needs a clock of 16x bandwidth
	//data needs to be inserted precisely as sent, eg for 8n1, insert (0x01 + ~data << 1) where 1 is the start bit, total bitlength should be programmed into PIO, shift right
	//code is more complex than you'd think to allow a SURE determination when TX is done (by examining pc)
	
	startPC = restartPC = pc;
	pio1_hw->instr_mem[pc++] = I_PULL(0, 0, 0, 1);
	pio1_hw->instr_mem[pc++] = I_SET(0, 0, SET_DST_X, dataBits + parityBits + stopBits + 1 /* start bit */ - 1);
	
	jmpDest = pc;
	pio1_hw->instr_mem[pc++] = I_OUT(2, 0, OUT_DST_PINS, 1);
	pio1_hw->instr_mem[pc++] = I_SET(11, 0, SET_DST_PINS, 0);
	pio1_hw->instr_mem[pc++] = I_JMP(0, 0, JMP_X_POSTDEC, jmpDest);
	endPC = pc - 1;
	
	//configure sm0
	pio1_hw->sm[mMySm].clkdiv = ((CPU_CLOCK_RATE / 16 * 256ull + baudrate / 2) / baudrate) << PIO_SM0_CLKDIV_FRAC_LSB;
	pio1_hw->ctrl |= ((1 << PIO_CTRL_CLKDIV_RESTART_LSB) << mMySm);
	pio1_hw->sm[mMySm].execctrl = (pio1_hw->sm[mMySm].execctrl &~ (PIO_SM0_EXECCTRL_WRAP_TOP_BITS | PIO_SM0_EXECCTRL_WRAP_BOTTOM_BITS | PIO_SM2_EXECCTRL_SIDE_EN_BITS)) |(endPC << PIO_SM0_EXECCTRL_WRAP_TOP_LSB) | (restartPC << PIO_SM0_EXECCTRL_WRAP_BOTTOM_LSB) | (SIDE_SET_HAS_ENABLE_BIT ? PIO_SM2_EXECCTRL_SIDE_EN_BITS : 0);
	pio1_hw->sm[mMySm].shiftctrl = (pio1_hw->sm[mMySm].shiftctrl &~ (PIO_SM1_SHIFTCTRL_PULL_THRESH_BITS | PIO_SM1_SHIFTCTRL_PUSH_THRESH_BITS | PIO_SM0_SHIFTCTRL_IN_SHIFTDIR_BITS | PIO_SM0_SHIFTCTRL_AUTOPUSH_BITS | PIO_SM0_SHIFTCTRL_AUTOPULL_BITS)) | PIO_SM0_SHIFTCTRL_OUT_SHIFTDIR_BITS | PIO_SM0_SHIFTCTRL_FJOIN_TX_BITS;
	pio1_hw->sm[mMySm].pinctrl = (SIDE_SET_BITS_USED << PIO_SM1_PINCTRL_SIDESET_COUNT_LSB) | (1 << PIO_SM1_PINCTRL_OUT_COUNT_LSB) | (PIN_IRDA_OUT << PIO_SM1_PINCTRL_OUT_BASE_LSB) | (1 << PIO_SM1_PINCTRL_SET_COUNT_LSB)| (PIN_IRDA_OUT << PIO_SM1_PINCTRL_SET_BASE_LSB);

	//set out direction
	pio1_hw->sm[mMySm].instr = I_SET(0, 0, SET_DST_PINDIRS, 1);
	
	//logi("starting TX SM\n");
	pio1_hw->sm[mMySm].instr = I_JMP(0, 0, JMP_ALWAYS, startPC);
	pio1_hw->ctrl |= ((1 << PIO_CTRL_SM_ENABLE_LSB) << mMySm);
	
	//irq on TX not full, but not enabled since it is empty now and we have no data
	pio1_hw->inte0 = 0;
	NVIC_ClearPendingIRQ(PIO1_0_IRQn);
	NVIC_EnableIRQ(PIO1_0_IRQn);
	pio1_hw->inte0 = PIO_IRQ0_INTE_SM0_TXNFULL_BITS << mMySm;
}

static void palmcardIrPrvSetupRx(uint32_t baudrate, uint_fast8_t dataBits, uint_fast8_t parityBits, uint_fast8_t stopBits)
{
	uint32_t bitcounterValue = (RX_MACHINERY_CLOCK + baudrate / 2) / baudrate - 4;
	uint_fast8_t pc = mMyStartPc, startPC, restartPC, endPC, jmpDest1, jmpDest2;
	
	
	//1.6276 us is the pulse width of 115,200 and of low power IrDA, so we wait for a low, check again in 1.5 us, and if it is still low, we consider this a start bit
	//clock should be ~20MHz, shifter should be to the right, autopush at 32 bits. autopull at 32 bits, input should be an infinite stream of words that represent the
	//inter-bit delay, 20e6 / baudrate - 33
	
	
	restartPC = startPC = pc;
	jmpDest1 = pc;
	pio1_hw->instr_mem[pc++] = I_WAIT(10, 0, 0, WAIT_FOR_GPIO, PIN_IRDA_IN);			//wait for low
	pio1_hw->instr_mem[pc++] = I_JMP(0, 0, JMP_PIN, jmpDest1);							//1us later, if high now, consider it a glitch
		
	pio1_hw->instr_mem[pc++] = I_SET(0, 0, SET_DST_Y, dataBits + stopBits + parityBits - 1);			//num bits per transport unit minus two
	
	jmpDest1 = pc;
	pio1_hw->instr_mem[pc++] = I_MOV(0, 0, MOV_DST_X, MOV_OP_COPY, IN_SRC_OSR);
	
	jmpDest2 = pc;
	pio1_hw->instr_mem[pc++] = I_JMP(0, 0, JMP_X_POSTDEC, jmpDest2);					//delay
	
	pio1_hw->instr_mem[pc++] = I_IN(0, 0, IN_SRC_PINS, 1);
	
	pio1_hw->instr_mem[pc++] = I_JMP(0, 0, JMP_Y_POSTDEC, jmpDest1);
	
	pio1_hw->instr_mem[pc++] = I_IN(0, 0, IN_SRC_ZEROES, 32 - dataBits - stopBits - parityBits);
	
	pio1_hw->instr_mem[pc++] = I_WAIT(4, 0, 1, WAIT_FOR_GPIO, PIN_IRDA_IN);				//wait for high
	endPC = pc - 1;
	
	pio1_hw->sm[mMySm].clkdiv = ((CPU_CLOCK_RATE * 256ull + RX_MACHINERY_CLOCK / 2) / RX_MACHINERY_CLOCK) << PIO_SM0_CLKDIV_FRAC_LSB;
	pio1_hw->ctrl |= ((1 << PIO_CTRL_CLKDIV_RESTART_LSB) << mMySm);
	pio1_hw->sm[mMySm].execctrl = (pio1_hw->sm[mMySm].execctrl &~ (PIO_SM0_EXECCTRL_WRAP_TOP_BITS | PIO_SM0_EXECCTRL_WRAP_BOTTOM_BITS | PIO_SM2_EXECCTRL_SIDE_EN_BITS)) |(endPC << PIO_SM0_EXECCTRL_WRAP_TOP_LSB) | (restartPC << PIO_SM0_EXECCTRL_WRAP_BOTTOM_LSB) | (SIDE_SET_HAS_ENABLE_BIT ? PIO_SM2_EXECCTRL_SIDE_EN_BITS : 0) | (PIN_IRDA_IN << PIO_SM2_EXECCTRL_JMP_PIN_LSB);
	pio1_hw->sm[mMySm].shiftctrl = (pio1_hw->sm[mMySm].shiftctrl &~ (PIO_SM1_SHIFTCTRL_PULL_THRESH_BITS | PIO_SM1_SHIFTCTRL_PUSH_THRESH_BITS | PIO_SM0_SHIFTCTRL_OUT_SHIFTDIR_BITS | PIO_SM0_SHIFTCTRL_AUTOPULL_BITS)) | PIO_SM0_SHIFTCTRL_IN_SHIFTDIR_BITS | PIO_SM0_SHIFTCTRL_AUTOPUSH_BITS;
	pio1_hw->sm[mMySm].pinctrl = (SIDE_SET_BITS_USED << PIO_SM1_PINCTRL_SIDESET_COUNT_LSB) | (PIN_IRDA_IN << PIO_SM1_PINCTRL_IN_BASE_LSB);
	
	
	//prepare OSR for SM1
	pio1_hw->txf[mMySm] = bitcounterValue;
	pio1_hw->sm[mMySm].instr = I_PULL(0, 0, 0, 0);
	pio1_hw->sm[mMySm].shiftctrl |= PIO_SM0_SHIFTCTRL_FJOIN_RX_BITS;
	
	//logi("starting RX SM\n");
	pio1_hw->sm[mMySm].instr = I_JMP(0, 0, JMP_ALWAYS, startPC);
	pio1_hw->ctrl |= ((1 << PIO_CTRL_SM_ENABLE_LSB) << mMySm);
	
	//irq on RX not empty
	pio1_hw->inte0 = 0;
	NVIC_ClearPendingIRQ(PIO1_0_IRQn);
	NVIC_EnableIRQ(PIO1_0_IRQn);
	pio1_hw->inte0 = PIO_IRQ0_INTE_SM0_RXNEMPTY_BITS << mMySm;
}

static uint_fast16_t palmcardIrPrvProcessInput(uint32_t rawVal)		//data is missing start bit, but otherwise is correct
{
	uint_fast16_t ret = 0, stopBitMask, dataValMask = (1 << mDataBits) - 1, dataVal = rawVal & dataValMask;
	uint_fast8_t i;
	
	//pre-calc stop bit mask
	stopBitMask = ((1 << mStopBits) - 1) << mDataBits;
	
	//calc parity
	if (mParityFunc) {
		
		uint_fast16_t corectParity = mParityFunc(dataVal) << mDataBits, parityMask = 1 << mDataBits;
		
		if ((rawVal ^ corectParity) & parityMask)
			ret |= UART_BIT_MASK_PAR_ERR;
		
		stopBitMask <<= 1;	//stop bits come after parity
	}
	
	//verify stop bits
	if ((rawVal & stopBitMask) != stopBitMask)
		ret |= UART_BIT_MASK_FRM_ERR;
	
	//add data val to return value
	ret += dataVal;
	
	return ret;
}

void __attribute__((used)) PIO1_0_IRQHandler(void)
{
	if (mCurTx) {	//in tx mode
	
		while (!(pio1_hw->fstat & ((1 << PIO_FSTAT_TXFULL_LSB) << mMySm))) {		//space in fifo?
			
			int32_t val = palmcardIrPrvCircBufGet();
						
			if (val < 0) {	//no more data
				
				pio1_hw->inte0 = 0;
				break;
			}
			else {			//have data
				
				pio1_hw->txf[mMySm] = val;
			}
		}
	}
	else if (mCurRx) {	//in rx mode
		
		uint_fast8_t nItems = 0;
		uint16_t buf[29];
		uint32_t oldR9;
	
		while (!(pio1_hw->fstat & ((1 << PIO_FSTAT_RXEMPTY_LSB) << mMySm)) && nItems < sizeof(buf) / sizeof(*buf)) {		//data & space in fifo?
			
			uint_fast16_t input = pio1_hw->rxf[mMySm], val = palmcardIrPrvProcessInput(input);
			
			buf[nItems++] = val;
		}
		
		if (mIrRxF) {
		
			oldR9 = ralSetSafeR9();
			mIrRxF(mIrRxD, buf, nItems);
			ralRestoreR9(oldR9);
		}
	}
	else {
		
		//spurious
		pio1_hw->inte0 = 0;
	}
}

static uint_fast8_t palmcardIrPrvEvenParity(uint_fast8_t val)	//calc bit to add to data to keep numbr of high bits even
{
	val ^= val >> 4;
	val ^= val >> 2;
	val ^= val >> 1;
	
	return val & 1;
}

static uint_fast8_t palmcardIrPrvOddParity(uint_fast8_t val)	//calc bit to add to data to keep numbr of high bits odd
{
	val ^= val >> 4;
	val ^= val >> 2;
	val ^= val >> 1;
	
	return 1 - (val & 1);
}

static uint_fast16_t palmcardIrPrvXformData(uint8_t byte)
{
	uint32_t val = 1 + ((uint32_t)((uint8_t)~byte)) * 2;		//start bit and data
	
	if (mParityFunc)
		val += ((uint32_t)mParityFunc(byte)) << (mDataBits + 1);
	
	//stop bits are zero here and thus no work to do for them
	
	return val;
}

static uint32_t __attribute__((noinline)) palmcardIrPrvSerialTx(const uint8_t *data, uint32_t len, bool block)
{
	uint32_t lenOrig = len;
		
	if (!data)			//we do not support sending breaks using IrDA
		return 0;
	
	if (!mCurTx)
		return 0;
	
	while (len) {
		
		if (palmcardIrPrvCircBufAdd(palmcardIrPrvXformData(*data))) {
						
			pio1_hw->inte0 = PIO_IRQ0_INTE_SM0_TXNFULL_BITS << mMySm;
			
			len--;
			data++;
			continue;
		}
				
		if (!block)
			break;
	}
	
	return lenOrig - len;
}

static uint32_t palmcardIrPrvCalcProperBaudrate(uint32_t requested)
{
	static const uint32_t acceptableBaudrates[] = {2400, 9600, 19200, 38400, 57600, 115200};
	uint_fast8_t i;
	
	for (i = 0; i < sizeof(acceptableBaudrates) && requested >= acceptableBaudrates[i]; i++);
	
	return i ? acceptableBaudrates[i - 1] : 0;
}

static bool palmcardIrPrvIsTxOngoing(void)
{
	//we must be in TX mode for TX to be ongiong
	if (!mCurTx)
		return false;
	
	//if ints are on that means TX is ongloing
	if (pio1_hw->inte0)
		return true;
	
	//if the tx FIFO is not empty, TX is ongoing
	if (!(pio1_hw->fstat & ((1 << PIO_FSTAT_TXEMPTY_LSB) << mMySm)))
		return true;
	
	//if PX has not yet reached our "parking" pc, tx is ongoing
	if (pio1_hw->sm[mMySm].addr != mMyStartPc)
		return true;
	
	//we could have read the above just as the SM read the last word from the FIFO, so recheck again
	if (pio1_hw->sm[mMySm].addr != mMyStartPc)
		return true;
	
	return false;
}

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

static bool repalmUartConfig(enum UartPort which, union UartCfg *cfg, RepalmUartRxF rxf, void *userData)
{
	switch (which) {
		case UartPortCradleSerial:
			return palmcardCommsSerialConfig(cfg, rxf, userData);
			
		case UartPortIrDA:
			return palmcardIrPrvConfig(cfg, rxf, userData);
		
		default:
			return false;
	}	
}

static uint32_t repalmUartTx(enum UartPort which, const uint8_t *data, uint32_t len, bool block)
{
	switch (which) {
		case UartPortCradleSerial:
			return palmcardCommsSerialTx(data, len, block);
			
		case UartPortIrDA:
			return palmcardIrPrvSerialTx(data, len, block);
		
		default:
			return false;
	}
}

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

static uint32_t repalmUartGetSta(enum UartPort which)
{
	switch (which) {
		case UartPortCradleSerial:
			return palmcardCommsSerialGetSta();
			
		case UartPortIrDA:
			return palmcardIrPrvGetSta();
		
		default:
			return false;
	}
}

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

/*
 * ============================================================================
 * Functions for picowalker operation
 * ============================================================================
 */

int pw_ir_read(uint8_t *buf, size_t max_len) {
    // TODO: replace with PIO code
    // See `palmcardIrPrvSerialTx()`
    return 0;
}


int pw_ir_write(uint8_t *buf, size_t len) {
    // TODO: replace with PIO code
    return 0;
}

void pw_ir_init() {
    // TODO: replace with PIO code
}

void pw_ir_clear_rx() {
    // TODO: replace with PIO code or remove
}

void pw_ir_deinit() {
    /* TODO: power saving (IR shutdown, stop uart1, etc.) */
}

