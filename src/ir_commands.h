#ifndef IR_COMMANDS_H
#define IR_COMMANDS_H


#define PACKET_HEADER_LEN	8

// http://dmitry.gr/?r=05.Projects&proj=28.%20pokewalker#_TOC_34cd8beec5c4813f4d151130632174e7
typedef enum {
	CMD_00 = 0x00, // Compressed EEPROM write
	CMD_02 = 0x02, // Direct eeprom write
	CMD_04 = 0x04, // ack for eeprom write
	CMD_06 = 0x06, // direct internal memory write
	CMD_0A = 0x0A, // direct eeprom write
	CMD_0C = 0x0C, // eeprom read
	CMD_0E = 0x0E, // eeprom read reply
	CMD_10 = 0x10, // master peer-play exchange
	CMD_12 = 0x12, // slave peer-play exchange
	CMD_14 = 0x14, // master peer-play exchange ack
	CMD_16 = 0x16, // ?
	CMD_1C = 0x1C, // peer-play too early 
	CMD_20 = 0x20, // request walker identity data
	CMD_22 = 0x22, // identity data from walker
	CMD_24 = 0x24, // ping request
	CMD_26 = 0x26, // ping reply
	CMD_2A = 0x2A, // unique id data
	CMD_2C = 0x2C, // same as above, with nuance
	CMD_32 = 0x32, // identity data, RTC only
	CMD_34 = 0x34, // reply to CMD_32
	CMD_36 = 0x36, // nothing?
	CMD_38 = 0x38, // performs walk start
	CMD_42 = 0x42, // same as CMD_32
	CMD_44 = 0x44, // reply to CMD_44
	CMD_4E = 0x4E, // performs walk end
	CMD_50 = 0x50, // reply to CMD_4E
	CMD_52 = 0x52, // same as CMD_32
	CMD_54 = 0x54, // reply to CMD_52
	CMD_56 = 0x56, // nothing?
	CMD_5A = 0x5A, // walk start
	CMD_60 = 0x60, // same as CMD_32
	CMD_62 = 0x62, // reply to CMD_62
	CMD_64 = 0x64, // nothing?
	CMD_66 = 0x66, // nothing?
	CMD_68 = 0x68, // reply to CMD_68
	CMD_80 = 0x80, // compressed eeprom write
	CMD_82 = 0x82, // direct eeprom write
	CMD_9C = 0x9C, // nothing?
	CMD_9E = 0x9E, // nothing?
	CMD_A0 = 0xA0, // ?
	CMD_A2 = 0xA2, // 
	CMD_A4 = 0xA4, // 
	CMD_A6 = 0xA6, // 
	CMD_A8 = 0xA8, // 
	CMD_AA = 0xAA, // 
	CMD_AC = 0xAC, // 
	CMD_AE = 0xAE, // 
	CMD_B8 = 0xB8, // ?
	CMD_BB = 0xBB, // 
	CMD_BC = 0xBC, // 
	CMD_BE = 0xBE, // 
	CMD_C0 = 0xC0, // give user special map
	CMD_C2 = 0xC2, // give user event pokemon
	CMD_C4 = 0xC4, // give user event item
	CMD_C6 = 0xC6, // move user to event route
	CMD_C8 = 0xC8, // reply to CMD_B8
	CMD_CA = 0xCA, // reply to CMD_BA
	CMD_CC = 0xCC, // reply to CMD_BC
	CMD_CE = 0xCE, // reply to CMD_BE
	CMD_D0 = 0xD0, // same as CMD_C0 but give user all stamps
	CMD_D2 = 0xD2, // "
	CMD_D4 = 0xD4, // "
	CMD_D6 = 0xD6, // "
	CMD_D8 = 0xD8, // nothing?
	CMD_F0 = 0xF0, // not understood fully
	CMD_F4 = 0xF4, // immediate disconnect
	CMD_F8 = 0xF8, // related to CMD_F0
	CMD_FA = 0xFA, // related to CMD_F0
	CMD_FC = 0xFC, // advertise for connection
	CMD_FE = 0xFE, // not understood fully
} ir_command_t;


#endif /* IR_COMMANDS_H */
