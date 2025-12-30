#ifndef _PIO_ASM_H_
#define _PIO_ASM_H_


	
	#define JMP_ALWAYS		0x00	//always
	#define JMP_X_ZERO		0x01	//if !X
	#define JMP_X_POSTDEC	0x02	//if X--
	#define JMP_Y_ZERO		0x03	//if !Y
	#define JMP_Y_POSTDEC	0x04	//if Y--
	#define JMP_X_NE_Y		0x05	//if X != Y
	#define JMP_PIN			0x06	//if: input pin state of (EXECCTRL_JMP_PIN)
	#define JMP_OSR_NE		0x07	//if OSR is not empty
	
	#define WAIT_FOR_GPIO	0x00	//absolute gpio address, unrelated ot any configs
	#define WAIT_FOR_PIN	0x01	//input pin mapped by PINCTRL_IN_BASE
	#define WAIT_FOR_IRQ	0x02	//irq flag selected by index
	
	#define IN_SRC_PINS		0x00
	#define IN_SRC_X		0x01
	#define IN_SRC_Y		0x02
	#define IN_SRC_ZEROES	0x03
	#define IN_SRC_ISR		0x06
	#define IN_SRC_OSR		0x07
	
	#define OUT_DST_PINS	0x00
	#define OUT_DST_X		0x01
	#define OUT_DST_Y		0x02
	#define OUT_DST_NULL	0x03
	#define OUT_DST_PINDIRS	0x04
	#define OUT_DST_PC		0x05
	#define OUT_DST_ISR		0x06
	#define OUT_DST_EXEC	0x07
	
	#define MOV_DST_PINS	0x00
	#define MOV_DST_X		0x01
	#define MOV_DST_Y		0x02
	#define MOV_DST_EXEC	0x04
	#define MOV_DST_PC		0x05
	#define MOV_DST_ISR		0x06
	#define MOV_DST_OSR		0x07
	
	#define MOV_OP_COPY		0x00
	#define MOV_OP_INVERT	0x01
	#define MOV_OP_BITREV	0x02		//only 32 bits wide
	
	#define MOV_SRC_PINS	0x00
	#define MOV_SRC_X		0x01
	#define MOV_SRC_Y		0x02
	#define MOV_SRC_ZEROES	0x03
	#define MOV_SRC_STATUS	0x05		//see EXECCTRL_STATUS_SEL
	#define MOV_SRC_ISR		0x06
	#define MOV_SRC_OSR		0x07
	
	#define SET_DST_PINS	0x00
	#define SET_DST_X		0x01
	#define SET_DST_Y		0x02
	#define SET_DST_PINDIRS	0x04


#endif

//define SIDE_SET_HAS_ENABLE_BIT and SIDE_SET_NUM_BITS before including this
#ifdef DEFINE_PIO_INSTRS

	#define SIDE_SET_BITS_USED					(SIDE_SET_HAS_ENABLE_BIT + SIDE_SET_NUM_BITS)	//one bit is data, one is enable
	#define WAIT_BITS_AVAIL						(5 - SIDE_SET_BITS_USED)

	#define PIO_INSTR(_opc, _delay, _sideSet, _misc)		(((_opc) << 13) | ((_sideSet) << (13 - SIDE_SET_BITS_USED)) | ((_delay) << 8) | (_misc))
	#define I_JMP(_delay, _sideSet, _condition, _dst)		PIO_INSTR(0x00, _delay, _sideSet, (((_condition) << 5) | (_dst)))
	#define I_WAIT(_delay, _sideSet, _polarity, _src, _idx)	PIO_INSTR(0x01, _delay, _sideSet, (((_polarity) << 7) | ((_src) << 5) | (_idx)))
	#define I_IN(_delay, _sideSet, _src, _nbits)			PIO_INSTR(0x02, _delay, _sideSet, (((_src) << 5) | ((_nbits) & 31)))
	#define I_OUT(_delay, _sideSet, _dst, _nbits)			PIO_INSTR(0x03, _delay, _sideSet, (((_dst) << 5) | ((_nbits) & 31)))
	#define I_PUSH(_delay, _sideSet, _ifF, _blk)			PIO_INSTR(0x04, _delay, _sideSet, (((_ifF) << 6) | ((_blk) << 5)))
	#define I_PULL(_delay, _sideSet, _ifE, _blk)			PIO_INSTR(0x04, _delay, _sideSet, (0x80 | ((_ifE) << 6) | ((_blk) << 5)))
	#define I_MOV(_delay, _sideSet, _dst, _op, _src)		PIO_INSTR(0x05, _delay, _sideSet, (((_dst) << 5) | ((_op) << 3) | (_src)))
	#define I_IRQ(_delay, _sideSet, _clr, _wait, _idx)		PIO_INSTR(0x06, _delay, _sideSet, (((_clr) << 6) | ((_wait) << 5) | (_idx)))
	#define I_SET(_delay, _sideSet, _dst, _val)				PIO_INSTR(0x07, _delay, _sideSet, (((_dst) << 5) | (_val)))

#endif

#ifdef UNDEF_PIO_INSTRS
	
	#undef SIDE_SET_BITS_USED
	#undef WAIT_BITS_AVAIL

	#undef I_JMP
	#undef I_WAIT
	#undef I_IN
	#undef I_OUT
	#undef I_PUSH
	#undef I_PULL
	#undef I_MOV
	#undef I_IRQ
	#undef I_SET

#endif





