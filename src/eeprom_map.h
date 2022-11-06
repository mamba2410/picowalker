// vim:  nowrap
#ifndef PW_EEPROM_MAP_H
#define PW_EEPROM_MAP_H

#define PW_EEPROM_ADDR_NINTENDO 0x0000  // "nintendo" string as a magic marker. if rom does not find this at boot, it will consider the walker empty and uninitialized
#define PW_EEPROM_SIZE_NINTENDO 8
#define PW_EEPROM_ADDR_PERSONALISATION 0x0008  // some value written during personalization. never read.
#define PW_EEPROM_SIZE_PERSONALISATION 8
//#define PW_EEPROM_ADDR_0x0010 0x0010  // ???
//#define PW_EEPROM_SIZE_0x0010 98
#define PW_EEPROM_ADDR_WATCHDOG_RESETS 0x0072 // number of watchdog resets
//#define PW_EEPROM_ADDR_0x0073 0x0073  // ???
//#define PW_EEPROM_SIZE_0x0073 13
// Worth emphasising that EACH struct has a checksum byte after it, not one checksum at the end
// of the RELIABLE_DATA
#define PW_EEPROM_ADDR_FACTORY_DATA_1 0x0080  // factory-provided adc calibration data. (reliable data format, copy at 0x0180)
#define PW_EEPROM_SIZE_FACTORY_DATA_1 2
#define PW_EEPROM_ADDR_FACTORY_DATA_CHK_1 (PW_EEPROM_ADDR_FACTORY_DATA_1+PW_EEPROM_SIZE_FACTORY_DATA_1)
#define PW_EEPROM_ADDR_UNIQUE_IDENTITY_DATA_1 0x0083  // struct uniqueidentitydata. provisioned at game pairing time (reliable data format, copy at 0x0183)
#define PW_EEPROM_SIZE_UNIQUE_IDENTITY_DATA_1 40
#define PW_EEPROM_ADDR_UNIQUE_IDENTITY_DATA_CHK_1 (PW_EEPROM_ADDR_UNIQUE_IDENTITY_DATA_1+PW_EEPROM_SIZE_UNIQUE_IDENTITY_DATA_1)
#define PW_EEPROM_ADDR_LCD_COMMANDS_1 0x00ac  // struct lcdconfigcmds. provisioned at game pairing time (reliable data format, copy at 0x01ac)
#define PW_EEPROM_SIZE_LCD_COMMANDS_1 64
#define PW_EEPROM_ADDR_LCD_COMMANDS_CHK_1 (PW_EEPROM_ADDR_LCD_COMMANDS_1+PW_EEPROM_SIZE_LCD_COMMANDS_1)
#define PW_EEPROM_ADDR_IDENTITY_DATA_1 0x00ed  // struct identitydata. provisioned at walk start time (reliable data format, copy at 0x01ed)
#define PW_EEPROM_SIZE_IDENTITY_DATA_1 104
#define PW_EEPROM_ADDR_IDENTITY_DATA_CHK_1 (PW_EEPROM_ADDR_IDENTITY_DATA_1+PW_EEPROM_SIZE_IDENTITY_DATA_1)
#define PW_EEPROM_ADDR_HEALTH_DATA_1 0x0156  // struct healthdata. provisioned at walk start time (reliable data format, copy at 0x0256)
#define PW_EEPROM_SIZE_HEALTH_DATA_1 24
#define PW_EEPROM_ADDR_HEALTH_DATA_CHK_1 (PW_EEPROM_ADDR_HEALTH_DATA_1+PW_EEPROM_SIZE_HEALTH_DATA_1)
#define PW_EEPROM_ADDR_COPY_MARKER_1 0x016f  // struct copymarker. used at walk init time (reliable data format, copy at 0x26f)
//#define PW_EEPROM_SIZE_COPY_MARKER_1 3 // TODO: dmitry typo? copy marker only 1 byte
//#define PW_EEPROM_ADDR_0x0172 0x0172  // unused
//#define PW_EEPROM_SIZE_0x0172 14    // TODO: dmitry typo, should be 16 bytes?
#define PW_EEPROM_ADDR_FACTORY_DATA_2 0x0180  // factory-provided adc calibration data. (reliable data format, copy at 0x0080)
#define PW_EEPROM_SIZE_FACTORY_DATA_2 2
#define PW_EEPROM_ADDR_FACTORY_DATA_CHK_2 (PW_EEPROM_ADDR_FACTORY_DATA_2+PW_EEPROM_SIZE_FACTORY_DATA_2)
#define PW_EEPROM_ADDR_UNIQUE_IDENTITY_DATA_2 0x0183  // struct uniqueidentitydata. provisioned at game pairing time (reliable data format, copy at 0x0083)
#define PW_EEPROM_SIZE_UNIQUE_IDENTITY_DATA_2 40
#define PW_EEPROM_ADDR_UNIQUE_IDENTITY_DATA_CHK_2 (PW_EEPROM_ADDR_UNIQUE_IDENTITY_DATA_2+PW_EEPROM_SIZE_UNIQUE_IDENTITY_DATA_2)
#define PW_EEPROM_ADDR_LCD_COMMANDS_2 0x01ac  // struct lcdconfigcmds. provisioned at game pairing time (reliable data format, copy at 0x00ac)
#define PW_EEPROM_SIZE_LCD_COMMANDS_2 64
#define PW_EEPROM_ADDR_LCD_COMMANDS_CHK_2 (PW_EEPROM_ADDR_LCD_COMMANDS_2+PW_EEPROM_SIZE_LCD_COMMANDS_2)
#define PW_EEPROM_ADDR_IDENTITY_DATA_2 0x01ed  // struct identitydata. provisioned at walk start time (reliable data format, copy at 0x00ed)
#define PW_EEPROM_SIZE_IDENTITY_DATA_2 104
#define PW_EEPROM_ADDR_IDENTITY_DATA_CHK_2 (PW_EEPROM_ADDR_IDENTITY_DATA_2+PW_EEPROM_SIZE_IDENTITY_DATA_2)
#define PW_EEPROM_ADDR_HEALTH_DATA_2 0x0256  // struct healthdata. provisioned at walk start time (reliable data format, copy at 0x0156)
#define PW_EEPROM_SIZE_HEALTH_DATA_2 24
#define PW_EEPROM_ADDR_HEALTH_DATA_CHK_2 (PW_EEPROM_ADDR_HEALTH_DATA_2+PW_EEPROM_SIZE_HEALTH_DATA_2)
#define PW_EEPROM_ADDR_COPY_MARKER_2 0x026f  // struct copymarker. used at walk init time (reliable data format, copy at 0x16f)
//#define PW_EEPROM_SIZE_COPY_MARKER_2 3
#define PW_EEPROM_ADDR_0x0272 0x0272  // unused
#define PW_EEPROM_SIZE_0x0272 14    // TODO: Same issues as above
#define PW_EEPROM_ADDR_IMG_DIGITS 0x0280  // numeric character images: "0123456789:-/", 8x16 each, in this order
#define PW_EEPROM_SIZE_IMG_DIGITS 416
#define PW_EEPROM_SIZE_IMG_CHAR         32
#define PW_EEPROM_ADDR_IMG_CHAR_ZERO    (PW_EEPROM_ADDR_IMG_DIGITS+0*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_CHAR_ONE     (PW_EEPROM_ADDR_IMG_DIGITS+1*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_CHAR_TWO     (PW_EEPROM_ADDR_IMG_DIGITS+2*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_CHAR_THREE   (PW_EEPROM_ADDR_IMG_DIGITS+3*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_CHAR_FOUR    (PW_EEPROM_ADDR_IMG_DIGITS+4*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_CHAR_FIVE    (PW_EEPROM_ADDR_IMG_DIGITS+5*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_CHAR_SIX     (PW_EEPROM_ADDR_IMG_DIGITS+6*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_CHAR_SEVEN   (PW_EEPROM_ADDR_IMG_DIGITS+7*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_CHAR_EIGHT   (PW_EEPROM_ADDR_IMG_DIGITS+8*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_CHAR_NINE    (PW_EEPROM_ADDR_IMG_DIGITS+9*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_CHAR_COLON   (PW_EEPROM_ADDR_IMG_DIGITS+10*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_CHAR_DASH    (PW_EEPROM_ADDR_IMG_DIGITS+11*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_CHAR_SLASH   (PW_EEPROM_ADDR_IMG_DIGITS+12*PW_EEPROM_SIZE_IMG_DIGITS)
#define PW_EEPROM_ADDR_IMG_WATTS 0x0420  // watt symbol image 16x16
#define PW_EEPROM_SIZE_IMG_WATTS 64
#define PW_EEPROM_ADDR_IMG_BALL 0x0460  // pokeball 8x8
#define PW_EEPROM_SIZE_IMG_BALL 16
#define PW_EEPROM_ADDR_IMG_BALL_LIGHT 0x0470  // pokeball light grey 8x8 (used for event pokemon)
#define PW_EEPROM_SIZE_IMG_BALL_LIGHT 16
//#define PW_EEPROM_ADDR_0x0480 0x0480  // unused
//#define PW_EEPROM_SIZE_0x0480 8
#define PW_EEPROM_ADDR_IMG_ITEM 0x0488  // item symbol 8x8
#define PW_EEPROM_SIZE_IMG_ITEM 16
#define PW_EEPROM_ADDR_IMG_ITEM_LIGHT 0x0498  // item symbol light grey 8x8 (used for event items)
#define PW_EEPROM_SIZE_IMG_ITEM_LIGHT 16
#define PW_EEPROM_ADDR_IMG_MAP_SMALL 0x04a8  // tiny map icon 8x8 (used for "special map" reception)
#define PW_EEPROM_SIZE_IMG_MAP_SMALL 16
#define PW_EEPROM_ADDR_IMG_CARD_SUITS 0x04b8  // card faces: heart, spade, diamond, club, 8x8 each (used for "stamp" reception)
#define PW_EEPROM_SIZE_IMG_CARD_SUITS 64
#define PW_EEPROM_SIZE_IMG_CARD_SUIT_SYMBOL 16
#define PW_EEPROM_ADDR_IMG_CARD_SUIT_HEART (PW_EEPROM_ADDR_IMG_CARD_SUITS+0*PW_EEPROM_SIZE_IMG_CARD_SUIT_SYMBOL)
#define PW_EEPROM_ADDR_IMG_CARD_SUIT_SPADE (PW_EEPROM_ADDR_IMG_CARD_SUITS+0*PW_EEPROM_SIZE_IMG_CARD_SUIT_SYMBOL)
#define PW_EEPROM_ADDR_IMG_CARD_SUIT_DIAMOND (PW_EEPROM_ADDR_IMG_CARD_SUITS+0*PW_EEPROM_SIZE_IMG_CARD_SUIT_SYMBOL)
#define PW_EEPROM_ADDR_IMG_CARD_SUIT_CLUB (PW_EEPROM_ADDR_IMG_CARD_SUITS+0*PW_EEPROM_SIZE_IMG_CARD_SUIT_SYMBOL)
#define PW_EEPROM_ADDR_IMG_ARROWS 0x04f8  // arrows (up down left right), each in 3 configs (normal, offset, inverted) 8x8 each
#define PW_EEPROM_SIZE_IMG_ARROWS 192
#define PW_EEPROM_SIZE_IMG_ARROW 16
#define PW_EEPROM_ADDR_IMG_ARROW_UP_NORMAL    (PW_EEPROM_ADDR_IMG_ARROWS+0*PW_EEPROM_SIZE_IMG_ARROW)
#define PW_EEPROM_ADDR_IMG_ARROW_UP_OFFSET    (PW_EEPROM_ADDR_IMG_ARROWS+1*PW_EEPROM_SIZE_IMG_ARROW)
#define PW_EEPROM_ADDR_IMG_ARROW_UP_INVERT    (PW_EEPROM_ADDR_IMG_ARROWS+2*PW_EEPROM_SIZE_IMG_ARROW)
#define PW_EEPROM_ADDR_IMG_ARROW_DOWN_NORMAL  (PW_EEPROM_ADDR_IMG_ARROWS+3*PW_EEPROM_SIZE_IMG_ARROW)
#define PW_EEPROM_ADDR_IMG_ARROW_DOWN_OFFSET  (PW_EEPROM_ADDR_IMG_ARROWS+4*PW_EEPROM_SIZE_IMG_ARROW)
#define PW_EEPROM_ADDR_IMG_ARROW_DOWN_INVERT  (PW_EEPROM_ADDR_IMG_ARROWS+5*PW_EEPROM_SIZE_IMG_ARROW)
#define PW_EEPROM_ADDR_IMG_ARROW_LEFT_NORMAL  (PW_EEPROM_ADDR_IMG_ARROWS+6*PW_EEPROM_SIZE_IMG_ARROW)
#define PW_EEPROM_ADDR_IMG_ARROW_LEFT_OFFSET  (PW_EEPROM_ADDR_IMG_ARROWS+7*PW_EEPROM_SIZE_IMG_ARROW)
#define PW_EEPROM_ADDR_IMG_ARROW_LEFT_INVERT  (PW_EEPROM_ADDR_IMG_ARROWS+8*PW_EEPROM_SIZE_IMG_ARROW)
#define PW_EEPROM_ADDR_IMG_ARROW_RIGHT_NORMAL (PW_EEPROM_ADDR_IMG_ARROWS+9*PW_EEPROM_SIZE_IMG_ARROW)
#define PW_EEPROM_ADDR_IMG_ARROW_RIGHT_OFFSET (PW_EEPROM_ADDR_IMG_ARROWS+10*PW_EEPROM_SIZE_IMG_ARROW)
#define PW_EEPROM_ADDR_IMG_ARROW_RIGHT_INVERT (PW_EEPROM_ADDR_IMG_ARROWS+11*PW_EEPROM_SIZE_IMG_ARROW)
#define PW_EEPROM_ADDR_IMG_MENU_ARROW_LEFT 0x05b8  // left arrow for menu 8x16
#define PW_EEPROM_SIZE_IMG_MENU_ARROW_LEFT 32
#define PW_EEPROM_ADDR_IMG_MENU_ARROW_RIGHT 0x05d8  // right arrow for menu 8x16
#define PW_EEPROM_SIZE_IMG_MENU_ARROW_RIGHT 32
#define PW_EEPROM_ADDR_IMG_MENU_ARROW_RETURN 0x05f8  // "return" symbol for menu 8x16
#define PW_EEPROM_SIZE_IMG_MENU_ARROW_RETURN 32
//#define PW_EEPROM_ADDR_0x0618 0x0618  // unused
//#define PW_EEPROM_SIZE_0x0618 40
#define PW_EEPROM_ADDR_0x0638 0x0638  // symbol for "have more message" in the bottom right of messages. orred into last 8 columns (thus 16 bytes). applied after 0x0648
#define PW_EEPROM_SIZE_0x0638 16
#define PW_EEPROM_ADDR_0x0648 0x0648  // symbol for "have more messages" in the bottom right of messages. each byte here is anded with each col of last 8 in the message. both bitplanes (so you can make it black or keep as is). applied before 0x0638
#define PW_EEPROM_SIZE_0x0648 8
#define PW_EEPROM_ADDR_0x0650 0x0650  // medicine vial (?) icon 8x8
#define PW_EEPROM_SIZE_0x0650 16
#define PW_EEPROM_ADDR_IMG_LOW_BATTERY 0x0660  // low battery icon 8x8
#define PW_EEPROM_SIZE_IMG_LOW_BATTERY 16
#define PW_EEPROM_ADDR_IMG_TALK_FACES 0x0670  // large talk bubbles from bottom right with pokemon feeling icon (exclamation, heart, music note, smile, neutral face, ellipsis) 24x16 each, 6 of them
#define PW_EEPROM_SIZE_IMG_TALK_FACES 576
#define PW_EEPROM_SIZE_IMG_TALK_FACE 96
#define PW_EEPROM_ADDR_IMG_TALK_FACE_EXCLAIM  (PW_EEPROM_ADDR_IMG_TALK_FACES+0*PW_EEPROM_SIZE_IMG_TALK_FACE)
#define PW_EEPROM_ADDR_IMG_TALK_FACE_HEART    (PW_EEPROM_ADDR_IMG_TALK_FACES+1*PW_EEPROM_SIZE_IMG_TALK_FACE)
#define PW_EEPROM_ADDR_IMG_TALK_FACE_MUSIC    (PW_EEPROM_ADDR_IMG_TALK_FACES+2*PW_EEPROM_SIZE_IMG_TALK_FACE)
#define PW_EEPROM_ADDR_IMG_TALK_FACE_SMILE    (PW_EEPROM_ADDR_IMG_TALK_FACES+3*PW_EEPROM_SIZE_IMG_TALK_FACE)
#define PW_EEPROM_ADDR_IMG_TALK_FACE_NEUTRAL  (PW_EEPROM_ADDR_IMG_TALK_FACES+4*PW_EEPROM_SIZE_IMG_TALK_FACE)
#define PW_EEPROM_ADDR_IMG_TALK_FACE_ELLIPSIS (PW_EEPROM_ADDR_IMG_TALK_FACES+5*PW_EEPROM_SIZE_IMG_TALK_FACE)
#define PW_EEPROM_ADDR_IMG_TALK_EXCLAMATION 0x08b0  // large talk bubble from bottom left with exclamation point in it 24x16
#define PW_EEPROM_SIZE_IMG_TALK_EXCLAMATION 96
#define PW_EEPROM_ADDR_IMG_MENU_TITLE_POKERADAR 0x0910  // "pok&eacute; radar" menu heading in a box. 80x16
#define PW_EEPROM_SIZE_IMG_MENU_TITLE_POKERADAR 320
#define PW_EEPROM_ADDR_IMG_MENU_TITLE_DOWSING 0x0a50  // "dowsing" menu heading in a box. 80x16
#define PW_EEPROM_SIZE_IMG_MENU_TITLE_DOWSING 320
#define PW_EEPROM_ADDR_IMG_MENU_TITLE_CONNECT 0x0b90  // "connect" menu heading in a box. 80x16
#define PW_EEPROM_SIZE_IMG_MENU_TITLE_CONNECT 320
#define PW_EEPROM_ADDR_IMG_MENU_TITLE_TRAINERCARD 0x0cd0  // "trainer card" menu heading in a box. 80x16
#define PW_EEPROM_SIZE_IMG_MENU_TITLE_TRAINERCARD 320
#define PW_EEPROM_ADDR_IMG_MENU_TITLE_INVENTORY 0x0e10  // "pok&eacute;mon &amp; items" menu heading in a box. 80x16
#define PW_EEPROM_SIZE_IMG_MENU_TITLE_INVENTORY 320
#define PW_EEPROM_ADDR_IMG_MENU_TITLE_SETTINGS 0x0f50  // "settings" menu heading in a box. 80x16
#define PW_EEPROM_SIZE_IMG_MENU_TITLE_SETTINGS 320
#define PW_EEPROM_ADDR_IMG_MENU_ICON_POKERADAR 0x1090  // "poke-radar" icon for main menu 16x16
#define PW_EEPROM_SIZE_IMG_MENU_ICON_POKERADAR 64
#define PW_EEPROM_ADDR_IMG_MENU_ICON_DOWSING 0x10d0  // "dowsing" icon for main menu 16x16
#define PW_EEPROM_SIZE_IMG_MENU_ICON_DOWSING 64
#define PW_EEPROM_ADDR_IMG_MENU_ICON_CONNECT 0x1110  // "connect" icon for main menu 16x16
#define PW_EEPROM_SIZE_IMG_MENU_ICON_CONNECT 64
#define PW_EEPROM_ADDR_IMG_MENU_ICON_TRAINERCARD 0x1150  // "trainer card" icon for main menu 16x16
#define PW_EEPROM_SIZE_IMG_MENU_ICON_TRAINERCARD 64
#define PW_EEPROM_ADDR_IMG_MENU_ICON_INVENTORY 0x1190  // "pokemon & items" icon for main menu 16x16
#define PW_EEPROM_SIZE_IMG_MENU_ICON_INVENTORY 64
#define PW_EEPROM_ADDR_IMG_MENU_ICON_SETTINGS 0x11d0  // "settings" icon for main menu 16x16
#define PW_EEPROM_SIZE_IMG_MENU_ICON_SETTINGS 64
#define PW_EEPROM_ADDR_IMG_PERSON 0x1210  // "person" icon for trainer card screen 16x16
#define PW_EEPROM_SIZE_IMG_PERSON 64
#define PW_EEPROM_ADDR_IMG_TRAINER_NAME 0x1250  // trainer's name rendered as an image 80x16
#define PW_EEPROM_SIZE_IMG_TRAINER_NAME 320
#define PW_EEPROM_ADDR_IMG_ROUTE_SMALL 0x1390  // small route image for "trainer card" screen 16x16
#define PW_EEPROM_SIZE_IMG_ROUTE_SMALL 64
#define PW_EEPROM_ADDR_IMG_STEPS_FRAME 0x13d0  // "steps" in frame for second screen of trainer card 40x16
#define PW_EEPROM_SIZE_IMG_STEPS_FRAME 160
#define PW_EEPROM_ADDR_IMG_TIME_FRAME 0x1470  // "time" in frame for second screen of trainer card 32x16
#define PW_EEPROM_SIZE_IMG_TIME_FRAME 128
#define PW_EEPROM_ADDR_IMG_DAYS_FRAME 0x14f0  // "days" in frame for second screen of trainer card 40x16
#define PW_EEPROM_SIZE_IMG_DAYS_FRAME 160
#define PW_EEPROM_ADDR_IMG_TOTAL_DAYS_FRAME 0x1590  // "total days:" in frame for second screen of trainer card 64x16
#define PW_EEPROM_SIZE_IMG_TOTAL_DAYS_FRAME 256
#define PW_EEPROM_ADDR_IMG_SOUND_FRAME 0x1690  // "sound" in frame for preferences screen 40x16
#define PW_EEPROM_SIZE_IMG_SOUND_FRAME 160
#define PW_EEPROM_ADDR_IMG_SHADE_FRAME 0x1730  // "shade" in frame for preferences screen 40x16
#define PW_EEPROM_SIZE_IMG_SHADE_FRAME 160
#define PW_EEPROM_ADDR_IMG_SPEAKER_OFF 0x17d0  // speaker icon with no waves (no sound) for preferences screen 24x16
#define PW_EEPROM_SIZE_IMG_SPEAKER_OFF 96
#define PW_EEPROM_ADDR_IMG_SPEAKER_LOW 0x1830  // speaker icon with one wave (low sound) for preferences screen 24x16
#define PW_EEPROM_SIZE_IMG_SPEAKER_LOW 96
#define PW_EEPROM_ADDR_IMG_SPEAKER_HIGH 0x1890  // speaker icon with two waves (high sound) for preferences screen 24x16
#define PW_EEPROM_SIZE_IMG_SPEAKER_HIGH 96
#define PW_EEPROM_ADDR_IMG_CONTRAST_DEMONSTRATOR 0x18f0  // contrast demonstrator (drawn a bunch of times over) 8x16
#define PW_EEPROM_SIZE_IMG_CONTRAST_DEMONSTRATOR 32
#define PW_EEPROM_ADDR_IMG_TREASURE_LARGE 0x1910  // large treasure chest icon for item view 32x24
#define PW_EEPROM_SIZE_IMG_TREASURE_LARGE 192
#define PW_EEPROM_ADDR_0x19d0 0x19d0  // large map scroll thingie 32x24
#define PW_EEPROM_SIZE_0x19d0 192
#define PW_EEPROM_ADDR_IMG_PRESENT_LARGE 0x1a90  // large present icon for item view 32x24
#define PW_EEPROM_SIZE_IMG_PRESENT_LARGE 192
#define PW_EEPROM_ADDR_IMG_DOWSING_BUSH_DARK 0x1b50  // small bush dark-colored, for dowsing 16x16
#define PW_EEPROM_SIZE_IMG_DOWSING_BUSH_DARK 64
#define PW_EEPROM_ADDR_IMG_DOWSING_BUSH_LIGHT 0x1b90  // small bush light-colored, for dowsing 16x16
#define PW_EEPROM_SIZE_IMG_DOWSING_BUSH_LIGHT 64
//#define PW_EEPROM_ADDR_0x1bd0 0x1bd0  // "left: "string on white background. seems unreferenced 32x16
//#define PW_EEPROM_SIZE_0x1bd0 128
//#define PW_EEPROM_ADDR_0x1c50 0x1c50  // blank image 16x24
//#define PW_EEPROM_SIZE_0x1c50 96
#define PW_EEPROM_ADDR_IMG_RADAR_BUSH 0x1cb0  // bush dark 32x24
#define PW_EEPROM_SIZE_IMG_RADAR_BUSH 192   // TODO: dmitry typo 0x1c6f -> 0x1d6f
#define PW_EEPROM_ADDR_IMG_RADAR_BUBBLE_ONE 0x1d70  // word bubble with one exclamation point (for poke hunting) 16x16
#define PW_EEPROM_SIZE_IMG_RADAR_BUBBLE_ONE 64
#define PW_EEPROM_ADDR_IMG_RADAR_BUBBLE_TWO 0x1db0  // word bubble with two exclamation points (for poke hunting) 16x16
#define PW_EEPROM_SIZE_IMG_RADAR_BUBBLE_TWO 64
#define PW_EEPROM_ADDR_IMG_RADAR_BUBBLE_THREE 0x1df0  // word bubble with three exclamation points (for poke hunting) 16x16
#define PW_EEPROM_SIZE_IMG_RADAR_BUBBLE_THREE 64    // TODO: dmitry typo 0x1e20 -> 0x1e2f
#define PW_EEPROM_ADDR_IMG_RADAR_CLICK 0x1e30  // three lines radiating from bottom left (for bush we just clicked) 16x16
#define PW_EEPROM_SIZE_IMG_RADAR_CLICK 64
#define PW_EEPROM_ADDR_IMG_RADAR_ATTACK_HIT 0x1e70  // skewed small 7-pointed star (attack) 16x32
#define PW_EEPROM_SIZE_IMG_RADAR_ATTACK_HIT 128
#define PW_EEPROM_ADDR_IMG_RADAR_CRITICAL_HIT 0x1ef0  // skewed large 7-pointed star (critical hit attack) 16x32
#define PW_EEPROM_SIZE_IMG_RADAR_CRITICAL_HIT 128
#define PW_EEPROM_ADDR_IMG_RADAR_APPEAR_CLOUD 0x1f70  // cloud "for pokemon appeared" 32x24
#define PW_EEPROM_SIZE_IMG_RADAR_APPEAR_CLOUD 192
#define PW_EEPROM_ADDR_IMG_RADAR_HP_BLIP 0x2030  // "hp" item (4 of these make up an hp bar) 8x8
#define PW_EEPROM_SIZE_IMG_RADAR_HP_BLIP 16
#define PW_EEPROM_ADDR_IMG_RADAR_CACH_EFFECT 0x2040  // a little 5-pointed star image for when we catch something 8x8
#define PW_EEPROM_SIZE_IMG_RADAR_CACH_EFFECT 16
#define PW_EEPROM_ADDR_TEXT_RADAR_ACTION 0x2050  // "attack/evade/catch" directions placard for battles 96x32
#define PW_EEPROM_SIZE_TEXT_RADAR_ACTION 768
#define PW_EEPROM_ADDR_IMG_POKEWALKER_BIG 0x2350  // pokewalker image, blank screen, 32x32
#define PW_EEPROM_SIZE_IMG_POKEWALKER_BIG 256
#define PW_EEPROM_ADDR_IMG_IR_ARCS 0x2450  // ir xmit icon (like wifi arcs) 8x16
#define PW_EEPROM_SIZE_IMG_IR_ARCS 32
#define PW_EEPROM_ADDR_IMG_MUSIC_NOTE 0x2470  // music note icon 8x8
#define PW_EEPROM_SIZE_IMG_MUSIC_NOTE 16
//#define PW_EEPROM_ADDR_0x2480 0x2480  // blank icon 8x8
//#define PW_EEPROM_SIZE_0x2480 16
#define PW_EEPROM_ADDR_IMG_HOURS_FRAME 0x2490  // "hours" in a pretty frame. appears unused 40x16
#define PW_EEPROM_SIZE_IMG_HOURS_FRAME 160
#define PW_EEPROM_ADDR_TEXT_CONNECTING 0x2530  // "connecting..." string for comms 96x16
#define PW_EEPROM_SIZE_TEXT_CONNECTING 384
#define PW_EEPROM_ADDR_TEXT_NO_TRAINER 0x26b0  // "no trainer found" string for comms 96x16
#define PW_EEPROM_SIZE_TEXT_NO_TRAINER 128  // TODO: dmitry typo 0x272f -> 0x282f
#define PW_EEPROM_ADDR_TEXT_CANNOT_COMPLETE 0x2830  // "cannot complete thisconnection" string for comms 96x32
#define PW_EEPROM_SIZE_TEXT_CANNOT_COMPLETE 768
#define PW_EEPROM_ADDR_TEXT_CANNOT_CONNECT 0x2b30  // "cannot connect" string for comms 96x16
#define PW_EEPROM_SIZE_TEXT_CANNOT_CONNECT 384
#define PW_EEPROM_ADDR_TEXT_TRAINER_UNAVAILABLE 0x2cb0  // "other trainer isunavailable" string for comms 96x32
#define PW_EEPROM_SIZE_TEXT_TRAINER_UNAVAILABLE 768
#define PW_EEPROM_ADDR_TEXT_ALREADY_RECV_EVENT 0x2fb0  // "already received this event" string for comms 96x32
#define PW_EEPROM_SIZE_TEXT_ALREADY_RECV_EVENT 384 // TODO: dmitry typo 0x21af -> 0x31af
#define PW_EEPROM_ADDR_TEXT_CANNOT_CONNECT_AGAIN 0x32b0  // "canont connect to trainer again" string for comms 96x32
#define PW_EEPROM_SIZE_TEXT_CANNOT_CONNECT_AGAIN 384   // TODO: dmitry typo 0x34af -> 0x35af
#define PW_EEPROM_ADDR_TEXT_COULD_NOT_RECV 0x35b0  // "could not receive..." string for comms 96x32
#define PW_EEPROM_SIZE_TEXT_COULD_NOT_RECV 384   // TODO: 0x37af -> 0x38af
#define PW_EEPROM_ADDR_TEXT_HAS_ARRIVED 0x38b0  // "has arrived!" string for comms 96x16
#define PW_EEPROM_SIZE_TEXT_HAS_ARRIVED 384   // TODO: dmitry typo 0x39af -> 3a2f ??
#define PW_EEPROM_ADDR_TEXT_HAS_LEFT 0x3a30  // "has left." string for comms 96x16
#define PW_EEPROM_SIZE_TEXT_HAS_LEFT 384
#define PW_EEPROM_ADDR_TEXT_RECV 0x3bb0  // "received!" string for comms 96x16
#define PW_EEPROM_SIZE_TEXT_RECV 384   // TODO: dmitry typo 0x3c2f -> 0x3d2f
#define PW_EEPROM_ADDR_TEXT_COMPLETED 0x3d30  // "completed!" string for comms 96x16
#define PW_EEPROM_SIZE_TEXT_COMPLETED 384
#define PW_EEPROM_ADDR_TEXT_SPECIAL_MAP 0x3eb0  // "special map" string for comms 96x16
#define PW_EEPROM_SIZE_TEXT_SPECIAL_MAP 384 // TODO: dmitry typo 0x302f -> 0x402f
#define PW_EEPROM_ADDR_TEXT_STAMP 0x4030  // "stamp" string for comms 96x16
#define PW_EEPROM_SIZE_TEXT_STAMP 384
#define PW_EEPROM_ADDR_TEXT_SPECIAL_ROUTE 0x41b0  // "special route" string for comms 96x16
#define PW_EEPROM_SIZE_TEXT_SPECIAL_ROUTE 384   // TODO: dmitry typo 0x422f -> 0x432f
#define PW_EEPROM_ADDR_TEXT_NEED_WATTS 0x4330  // "need more watts." string 96x16
#define PW_EEPROM_SIZE_TEXT_NEED_WATTS 384
#define PW_EEPROM_ADDR_TEXT_NO_POKEMON_HELD 0x44b0  // "no pokemon held!" string 96x16
#define PW_EEPROM_SIZE_TEXT_NO_POKEMON_HELD 384
#define PW_EEPROM_ADDR_TEXT_NOTHING_HELD 0x4630  // "nothing held!" string 96x16
#define PW_EEPROM_SIZE_TEXT_NOTHING_HELD 384
#define PW_EEPROM_ADDR_TEXT_DISCOVER_ITEM 0x47b0  // "discover an item!" string 96x16
#define PW_EEPROM_SIZE_TEXT_DISCOVER_ITEM 384
#define PW_EEPROM_ADDR_TEXT_FOUND 0x4930  // "found!" string 90x16
#define PW_EEPROM_SIZE_TEXT_FOUND 384
#define PW_EEPROM_ADDR_TEXT_NOTHING_FOUND 0x4ab0  // "nothing found!" string 90x16
#define PW_EEPROM_SIZE_TEXT_NOTHING_FOUND 384
#define PW_EEPROM_ADDR_TEXT_ITS_NEAR 0x4c30  // "it's near!" string 90x16
#define PW_EEPROM_SIZE_TEXT_ITS_NEAR 384
#define PW_EEPROM_ADDR_TEXT_FAR_AWAY 0x4db0  // "it's far away..." string 90x16
#define PW_EEPROM_SIZE_TEXT_FAR_AWAY 384    // TODO: dmitry typo 0x4e2f -> 0x4f2f
#define PW_EEPROM_ADDR_TEXT_FIND_POKEMON 0x4f30  // "find a pokemon!" string 90x16
#define PW_EEPROM_SIZE_TEXT_FIND_POKEMON 384 // TODO: dmitry typo 0x40af -> 0x50af
#define PW_EEPROM_ADDR_TEXT_FOUND_SOMETHING 0x50b0  // "found something!" string 90x16
#define PW_EEPROM_SIZE_TEXT_FOUND_SOMETHING 384
#define PW_EEPROM_ADDR_TEXT_GOT_AWAY 0x5230  // "it got away..." string 90x16
#define PW_EEPROM_SIZE_TEXT_GOT_AWAY 384   // TODO: dmitry typo 0x52af -> 0x53af
#define PW_EEPROM_ADDR_TEXT_APPEARED 0x53b0  // "appeared!" string 90x16
#define PW_EEPROM_SIZE_TEXT_APPEARED 384
#define PW_EEPROM_ADDR_TEXT_WAS_CAUGHT 0x5530  // "was caught!" string 90x16
#define PW_EEPROM_SIZE_TEXT_WAS_CAUGHT 384   // TODO: dmitry typo 0x55af -> 0x56af
#define PW_EEPROM_ADDR_TEXT_FLED 0x56b0  // "fled..." string 96x16
#define PW_EEPROM_SIZE_TEXT_FLED 384   // TODO: dmitry typo 0x572f -> 0x582f
#define PW_EEPROM_ADDR_TEXT_TOO_STRONG 0x5830  // "was too strong." string 96x16
#define PW_EEPROM_SIZE_TEXT_TOO_STRONG 384
#define PW_EEPROM_ADDR_TEXT_ATTACKED 0x59b0  // "attached!" string 96x16
#define PW_EEPROM_SIZE_TEXT_ATTACKED 384
#define PW_EEPROM_ADDR_TEXT_EVADED 0x5b30  // "evaded!" string 96x16
#define PW_EEPROM_SIZE_TEXT_EVADED 384
#define PW_EEPROM_ADDR_TEXT_CRITICAL_HIT 0x5cb0  // "a critical hit!" string 96x16
#define PW_EEPROM_SIZE_TEXT_CRITICAL_HIT 384
#define PW_EEPROM_ADDR_TEXT_SPACES 0x5e30  // "       " (yes a bunch of spaces) string 96x16
#define PW_EEPROM_SIZE_TEXT_SPACES 384
#define PW_EEPROM_ADDR_TEXT_THREW_BALL 0x5fb0  // "threw a poke ball." string 96x16
#define PW_EEPROM_SIZE_TEXT_THREW_BALL 384 // TODO: dmitry typo 0x512f -> 0x612f
#define PW_EEPROM_ADDR_TEXT_ALMOST_HAD 0x6130  // "almost had it!" string 96x16
#define PW_EEPROM_SIZE_TEXT_ALMOST_HAD 384   // TODO: dmitry typo 0x61af -> 0x62af
#define PW_EEPROM_ADDR_TEXT_STARE_DOWN 0x62b0  // "stare down!" string 96x16
#define PW_EEPROM_SIZE_TEXT_STARE_DOWN 384
#define PW_EEPROM_ADDR_TEXT_LOST 0x6430  // "lost!" string 96x16
#define PW_EEPROM_SIZE_TEXT_LOST 384
#define PW_EEPROM_ADDR_TEXT_PEER_HAS_ARRIVED 0x65b0  // "has arrived" (for walker to walker) string 96x16
#define PW_EEPROM_SIZE_TEXT_PEER_HAS_ARRIVED 384   // TODO: dmitry typo 0x662f -> 0x672f
#define PW_EEPROM_ADDR_TEXT_HAD_ADVENTURES 0x6730  // "had adventures!" string 96x16
#define PW_EEPROM_SIZE_TEXT_HAD_ADVENTURES 384
#define PW_EEPROM_ADDR_TEXT_PLAY_BATTLED 0x68b0  // "play-battled." string 96x16
#define PW_EEPROM_SIZE_TEXT_PLAY_BATTLED 384
#define PW_EEPROM_ADDR_TEXT_WENT_RUN 0x6a30  // "went for a run." string 96x16
#define PW_EEPROM_SIZE_TEXT_WENT_RUN 384
#define PW_EEPROM_ADDR_TEXT_WENT_WALK 0x6bb0  // "went for a walk." string 96x16
#define PW_EEPROM_SIZE_TEXT_WENT_WALK 384
#define PW_EEPROM_ADDR_TEXT_PLAYED 0x6d30  // "played a bit." string 96x16
#define PW_EEPROM_SIZE_TEXT_PLAYED 384
#define PW_EEPROM_ADDR_TEXT_RECV_GIFT 0x6eb0  // "here's a gift..." string 96x16
#define PW_EEPROM_SIZE_TEXT_RECV_GIFT 384 // TODO: dmitry typo 0x602f -> 0x702f
#define PW_EEPROM_ADDR_TEXT_CHEERED 0x7030  // "cheered!" string 96x16
#define PW_EEPROM_SIZE_TEXT_CHEERED 384
#define PW_EEPROM_ADDR_TEXT_VERY_HAPPY 0x71b0  // "is very happy!" string 96x16
#define PW_EEPROM_SIZE_TEXT_VERY_HAPPY 384
#define PW_EEPROM_ADDR_TEXT_FUN 0x7330  // "is having fun!" string 96x16
#define PW_EEPROM_SIZE_TEXT_FUN 384
#define PW_EEPROM_ADDR_TEXT_FEEL_GOOD 0x74b0  // "is feeling good!" string 96x16
#define PW_EEPROM_SIZE_TEXT_FEEL_GOOD 384
#define PW_EEPROM_ADDR_TEXT_HAPPY 0x7630  // "is happy." string 96x16
#define PW_EEPROM_SIZE_TEXT_HAPPY 384
#define PW_EEPROM_ADDR_TEXT_SMILING 0x77b0  // "is smiling." string 96x16
#define PW_EEPROM_SIZE_TEXT_SMILING 384
#define PW_EEPROM_ADDR_TEXT_CHEERFUL 0x7930  // "is cheerful." string 96x16
#define PW_EEPROM_SIZE_TEXT_CHEERFUL 384
#define PW_EEPROM_ADDR_TEXT_PATIENT 0x7ab0  // "is being patient." string 96x16
#define PW_EEPROM_SIZE_TEXT_PATIENT 384
#define PW_EEPROM_ADDR_TEXT_SITTING 0x7c30  // "sits quietly." string 96x16
#define PW_EEPROM_SIZE_TEXT_SITTING 384
#define PW_EEPROM_ADDR_TEXT_TURN_LOOK 0x7db0  // "turned to look." string 96x16
#define PW_EEPROM_SIZE_TEXT_TURN_LOOK 384
#define PW_EEPROM_ADDR_TEXT_LOOKING_AROUND 0x7f30  // "is looking around." string 96x16
#define PW_EEPROM_SIZE_TEXT_LOOKING_AROUND -3712    // TODO: dmitry typo 0x70af -> 0x80af
#define PW_EEPROM_ADDR_TEXT_LOOKING_HERE 0x80b0  // "is looking this way." string 96x16
#define PW_EEPROM_SIZE_TEXT_LOOKING_HERE 384
#define PW_EEPROM_ADDR_TEXT_DAYDREAMING 0x8230  // "is daydreaming." string 96x16
#define PW_EEPROM_SIZE_TEXT_DAYDREAMING 384
#define PW_EEPROM_ADDR_TEXT_INTERACT_FOUND_SOMETHING 0x83b0  // "found something." string 96x16
#define PW_EEPROM_SIZE_TEXT_INTERACT_FOUND_SOMETHING 384
#define PW_EEPROM_ADDR_TEXT_WHAT 0x8530  // "what?" string 96x16
#define PW_EEPROM_SIZE_TEXT_WHAT 384
#define PW_EEPROM_ADDR_TEXT_JOINED 0x86b0  // "joined you!" string 96x16
#define PW_EEPROM_SIZE_TEXT_JOINED 384
#define PW_EEPROM_ADDR_TEXT_REWARD 0x8830  // "reward" string 96x16
#define PW_EEPROM_SIZE_TEXT_REWARD 384
#define PW_EEPROM_ADDR_TEXT_GOOD_JOB 0x89b0  // "good job!" string 96x16
#define PW_EEPROM_SIZE_TEXT_GOOD_JOB 384
#define PW_EEPROM_ADDR_TEXT_SWITCH 0x8b30  // "switch?" string 80x16
#define PW_EEPROM_SIZE_TEXT_SWITCH 320
//#define PW_EEPROM_ADDR_0x8c70 0x8c70  // ???
//#define PW_EEPROM_SIZE_0x8c70 64
#define PW_EEPROM_ADDR_RANDOM_CHECKSUM_INFO 0x8cb0  // random checksum area descriptor addrs	(see 	0x36f2:randomeepromchecksumcheck	, struct struct randomcheckinfo)
#define PW_EEPROM_SIZE_RANDOM_CHECKSUM_INFO 48
#define PW_EEPROM_ADDR_RANDOM_CHECKSUM_AREA 0x8cf0  // random garbage data that is checksummed by randomeepromchecksumcheck()
#define PW_EEPROM_SIZE_RANDOM_CHECKSUM_AREA 528
#define PW_EEPROM_ADDR_ROUTE_INFO 0x8f00  // struct routeinfo - current route data
#define PW_EEPROM_SIZE_ROUTE_INFO 190
#define PW_EEPROM_ADDR_IMG_ROUTE_LARGE 0x8fbe  // current "area" we are strolling in graphic 32x24
#define PW_EEPROM_SIZE_IMG_ROUTE_LARGE 192
#define PW_EEPROM_ADDR_TEXT_ROUTE_NAME 0x907e  // current "area" we are strolling in textual name 80x16
#define PW_EEPROM_SIZE_TEXT_ROUTE_NAME 320
#define PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED 0x91be  // current pokemon animated sprite for "held items and pokemon" screen, fights, etc. 32 x 24 x 2 frames
#define PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED 384
#define PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED_FRAME 192
#define PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED_FRAME1 (PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED+0*PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED_FRAME2 (PW_EEPROM_ADDR_IMG_POKEMON_SMALL_ANIMATED+1*PW_EEPROM_SIZE_IMG_POKEMON_SMALL_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED 0x933e  // current pokemon large nimated sprite for main screen 64 x 48 x 2 frames
#define PW_EEPROM_SIZE_IMG_POKEMON_LARGE_ANIMATED 1536
#define PW_EEPROM_SIZE_IMG_POKEMON_LARGE_ANIMATED_FRAME 768
#define PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED_FRAME1 (PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED+0*PW_EEPROM_SIZE_IMG_POKEMON_LARGE_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED_FRAME2 (PW_EEPROM_ADDR_IMG_POKEMON_LARGE_ANIMATED+1*PW_EEPROM_SIZE_IMG_POKEMON_LARGE_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_TEXT_POKEMON_NAME 0x993e  // cur pokemon name image 80x16
#define PW_EEPROM_SIZE_TEXT_POKEMON_NAME 320
#define PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED 0x9a7e  // route available pokemon selected by the same animated small sprites  32 x 24 x 2 frames x 3 pokemon
#define PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED 1152
#define PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME 192
#define PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED_P1_FRAME1 (PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED+0*PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED_P1_FRAME2 (PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED+1*PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED_P2_FRAME1 (PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED+2*PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED_P2_FRAME2 (PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED+3*PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED_P3_FRAME1 (PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED+4*PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED_P3_FRAME2 (PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_SMALL_ANIMATED+5*PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_SMALL_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_LARGE_ANIMATED 0x9efe  // large animated image (like at 0x933e) but of the third (option c) available pokemon on this route. used for "joined your walk" situation 64 x 48 x 2 frame
#define PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_LARGE_ANIMATED 1536
#define PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_LARGE_ANIMATED_FRAME 768
#define PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_LARGE_ANIMATED_P3_FRAME1 (PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_LARGE_ANIMATED+0*PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_LARGE_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_LARGE_ANIMATED_P3_FRAME2 (PW_EEPROM_ADDR_IMG_ROUTE_POKEMON_LARGE_ANIMATED+1*PW_EEPROM_SIZE_IMG_ROUTE_POKEMON_LARGE_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_TEXT_POKEMON_NAMES 0xa4fe  // available pokemon name images 80x16 x3 pokemon
#define PW_EEPROM_SIZE_TEXT_POKEMON_NAMES 960
#define PW_EEPROM_SIZE_TEXT_POKEMON_NAMES_SINGLE 320
#define PW_EEPROM_ADDR_TEXT_POKEMON_NAME_P1 (PW_EEPROM_ADDR_TEXT_POKEMON_NAMES+0*PW_EEPROM_SIZE_TEXT_POKEMON_NAMES_SINGLE)
#define PW_EEPROM_ADDR_TEXT_POKEMON_NAME_P2 (PW_EEPROM_ADDR_TEXT_POKEMON_NAMES+1*PW_EEPROM_SIZE_TEXT_POKEMON_NAMES_SINGLE)
#define PW_EEPROM_ADDR_TEXT_POKEMON_NAME_P3 (PW_EEPROM_ADDR_TEXT_POKEMON_NAMES+2*PW_EEPROM_SIZE_TEXT_POKEMON_NAMES_SINGLE)
#define PW_EEPROM_ADDR_TEXT_ITEM_NAMES 0xa8be  // item names as images. 96x16 x 10 images (one per item)
#define PW_EEPROM_SIZE_TEXT_ITEM_NAMES 3840
#define PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE 384
#define PW_EEPROM_ADDR_TEXT_ITEM_NAME_0 (PW_EEPROM_ADDR_TEXT_ITEM_NAMES+0*PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE)
#define PW_EEPROM_ADDR_TEXT_ITEM_NAME_1 (PW_EEPROM_ADDR_TEXT_ITEM_NAMES+1*PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE)
#define PW_EEPROM_ADDR_TEXT_ITEM_NAME_2 (PW_EEPROM_ADDR_TEXT_ITEM_NAMES+2*PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE)
#define PW_EEPROM_ADDR_TEXT_ITEM_NAME_3 (PW_EEPROM_ADDR_TEXT_ITEM_NAMES+3*PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE)
#define PW_EEPROM_ADDR_TEXT_ITEM_NAME_4 (PW_EEPROM_ADDR_TEXT_ITEM_NAMES+4*PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE)
#define PW_EEPROM_ADDR_TEXT_ITEM_NAME_5 (PW_EEPROM_ADDR_TEXT_ITEM_NAMES+5*PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE)
#define PW_EEPROM_ADDR_TEXT_ITEM_NAME_6 (PW_EEPROM_ADDR_TEXT_ITEM_NAMES+6*PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE)
#define PW_EEPROM_ADDR_TEXT_ITEM_NAME_7 (PW_EEPROM_ADDR_TEXT_ITEM_NAMES+7*PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE)
#define PW_EEPROM_ADDR_TEXT_ITEM_NAME_8 (PW_EEPROM_ADDR_TEXT_ITEM_NAMES+8*PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE)
#define PW_EEPROM_ADDR_TEXT_ITEM_NAME_9 (PW_EEPROM_ADDR_TEXT_ITEM_NAMES+9*PW_EEPROM_SIZE_TEXT_ITEM_NAME_SINGLE)
//#define PW_EEPROM_ADDR_0xb7be 0xb7be  // ???
//#define PW_EEPROM_SIZE_0xb7be 66
#define PW_EEPROM_ADDR_RECEIVED_BITFIELD 0xb800 // bitfield of special things received. 0x01 - \"heart\" stamp received, 0x02 - \"spade\" stamp received, 0x04 - \"diamond\" stamp received, 0x08 - \"club\" stamp received, 0x10 - \"special map\" received, 0x20 - walker contains an event pokemon (gifted or caught), 0x40 - walker contains event item (gifted or dowsed), 0x80 - walker has received a \"special route\"
//#define PW_EEPROM_ADDR_0xb801 0xb801  // unused
//#define PW_EEPROM_SIZE_0xb801 3
//#define PW_EEPROM_ADDR_0xb804 0xb804  // data for "special map received". format unknown. possibly used by the ds games, but no evidence of this found in the games.
//#define PW_EEPROM_SIZE_0xb804 576
// TODO: multiple columns
#define PW_EEPROM_ADDR_EVENT_POKEMON_BASIC_DATA 0xba44  // gifted event poke, or radar-caught event poke, basic data. struct PokemonSummary
#define PW_EEPROM_SIZE_EVENT_POKEMON_BASIC_DATA 16
#define PW_EEPROM_ADDR_EVENT_POKEMON_EXTRA_DATA 0xba54  // extra data. struct eventpokeextradata
#define PW_EEPROM_SIZE_EVENT_POKEMON_EXTRA_DATA 44
#define PW_EEPROM_ADDR_IMG_EVENT_POKEMON_SMALL_ANIMATED 0xba80  // small sprite 32 x 24 x 2 frames
#define PW_EEPROM_SIZE_IMG_EVENT_POKEMON_SMALL_ANIMATED 384
#define PW_EEPROM_SIZE_IMG_EVENT_POKEMON_SMALL_ANIMATED_FRAME 192
#define PW_EEPROM_ADDR_IMG_EVENT_POKEMON_SMALL_ANIMATED_FRAME1 (PW_EEPROM_ADDR_IMG_EVENT_POKEMON_SMALL_ANIMATED+0*PW_EEPROM_SIZE_IMG_EVENT_POKEMON_SMALL_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_IMG_EVENT_POKEMON_SMALL_ANIMATED_FRAME2 (PW_EEPROM_ADDR_IMG_EVENT_POKEMON_SMALL_ANIMATED+1*PW_EEPROM_SIZE_IMG_EVENT_POKEMON_SMALL_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_TEXT_EVENT_POKEMON_NAME 0xbc00  // name image 80x16
#define PW_EEPROM_SIZE_TEXT_EVENT_POKEMON_NAME 320
// TODO: multiple columns
#define PW_EEPROM_ADDR_EVENT_ITEM 0xbd40  // gifted event item, or dowsed event item, item data. 6 bytes of zeroes, then u16 item, LE
#define PW_EEPROM_SIZE_EVENT_ITEM 8
#define PW_EEPROM_ADDR_TEXT_EVENT_ITEM_NAME 0xbd48  // item name image 96x16
#define PW_EEPROM_SIZE_TEXT_EVENT_ITEM_NAME 384
//#define PW_EEPROM_ADDR_0xbec8 0xbec8  // unused
//#define PW_EEPROM_SIZE_0xbec8 56
// TODO: multiple columns
#define PW_EEPROM_ADDR_SPECIAL_ROUTE_STRUCT 0xbf00  // "special route" info (struct specialroute):
#define PW_EEPROM_SIZE_SPECIAL_ROUTE_STRUCT 3260
//#define PW_EEPROM_ADDR_0xbf00 0xbf00  // "special route" info (struct specialroute):
//#define PW_EEPROM_SIZE_0xbf00 6
#define PW_EEPROM_ADDR_ROUTE_IMAGE_IDX 0xbf06 // enum routeimageidx
#define PW_EEPROM_ADDR_0xbf07 0xbf07 // unused
#define PW_EEPROM_ADDR_SPECIAL_POKEMON_BASIC_DATA 0xbf08  // special route-available pokemon basic data. struct pokemonsummary
#define PW_EEPROM_SIZE_SPECIAL_POKEMON_BASIC_DATA 16
#define PW_EEPROM_ADDR_SPECIAL_POKEMON_EXTRA_DATA 0xbf18  // special route-available pokemon extra data. struct eventpokeextradata
#define PW_EEPROM_SIZE_SPECIAL_POKEMON_EXTRA_DATA 44
#define PW_EEPROM_ADDR_SPECIAL_POKEMON_STEPS_REQUIRED 0xbf44  // min steps to encounter this poke on the route. u16 le
#define PW_EEPROM_SIZE_SPECIAL_POKEMON_STEPS_REQUIRED 2
#define PW_EEPROM_ADDR_SPECIAL_POKEMON_PERCENT_CHANCE 0xbf46 // percent chance to encounter this poke on route after step minimum met
//#define PW_EEPROM_ADDR_0xbf47 0xbf47 // unused
#define PW_EEPROM_ADDR_SPECIAL_ITEM 0xbf48  // special route-available item. u16 le
#define PW_EEPROM_SIZE_SPECIAL_ITEM 2
#define PW_EEPROM_ADDR_SPECIAL_ITEM_STEPS_REQUIRED 0xbf4a  // min steps dowse this item. u16 le
#define PW_EEPROM_SIZE_SPECIAL_ITEM_STEPS_REQUIRED 2
#define PW_EEPROM_ADDR_SPECIAL_ITEM_PERCENT_CHANCE 0xbf4c // percent chance to dowse this item on route after step minimum met
//#define PW_EEPROM_ADDR_0xbf4d 0xbf4d  // unused
//#define PW_EEPROM_SIZE_0xbf4d 3
#define PW_EEPROM_ADDR_SPECIAL_ROUTE_NAME_NINTENDOENC 0xbf50  // routename u16[21]
#define PW_EEPROM_SIZE_SPECIAL_ROUTE_NAME_NINTENDOENC 42
#define PW_EEPROM_ADDR_SPECIAL_POKEMON_EVENT_INDEX 0xbf7a // \"event index\" for catching this route's special pokemon
#define PW_EEPROM_ADDR_SPECIAL_ITEM_EVENT_INDEX 0xbf7b // \"event index\" for dowsing this route's special item
#define PW_EEPROM_ADDR_IMG_SPECIAL_POKEMON_SMALL_ANIMATED 0xbf7c  // special route pokemon animates small sprite. 32 x 24 x 2 frames. should be 0x180 bytes big, but it 0x170. no idea why but confirmed
#define PW_EEPROM_SIZE_IMG_SPECIAL_POKEMON_SMALL_ANIMATED 368
#define PW_EEPROM_SIZE_IMG_SPECIAL_POKEMON_SMALL_ANIMATED_FRAME 172 // NOTE: NOT 192
#define PW_EEPROM_ADDR_IMG_SPECIAL_POKEMON_SMALL_ANIMATED_FRAME1 (PW_EEPROM_ADDR_IMG_SPECIAL_POKEMON_SMALL_ANIMATED+0*PW_EEPROM_SIZE_IMG_SPECIAL_POKEMON_SMALL_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_IMG_SPECIAL_POKEMON_SMALL_ANIMATED_FRAME2 (PW_EEPROM_ADDR_IMG_SPECIAL_POKEMON_SMALL_ANIMATED+1*PW_EEPROM_SIZE_IMG_SPECIAL_POKEMON_SMALL_ANIMATED_FRAME)
#define PW_EEPROM_ADDR_TEXT_SPECIAL_POKEMON_NAME 0xc6fc  // special route pokemon name image 80x16
#define PW_EEPROM_SIZE_TEXT_SPECIAL_POKEMON_NAME 320
#define PW_EEPROM_ADDR_IMG_SPECIAL_ROUTE_IMAGE 0xc83c  // special routes's large image for home screen, like 0x8fbe is for a normal route 32x24
#define PW_EEPROM_SIZE_IMG_SPECIAL_ROUTE_IMAGE 192
#define PW_EEPROM_ADDR_TEXT_SPECIAL_ROUTE_NAME_SMALL 0xc8fc  // special routes's textual name 80x16
#define PW_EEPROM_SIZE_TEXT_SPECIAL_ROUTE_NAME_SMALL 320
#define PW_EEPROM_ADDR_TEXT_SPECIAL_ROUTE_NAME 0xca3c  // special route item textual name 96x16
#define PW_EEPROM_SIZE_TEXT_SPECIAL_ROUTE_NAME 384
//#define PW_EEPROM_ADDR_0xcbbc 0xcbbc  // unused
//#define PW_EEPROM_SIZE_0xcbbc 68
#define PW_EEPROM_ADDR_TEAM_DATA_STRUCT 0xcc00  // struct teamdata on our whole team, so that any walkers we peer play with transfer it to their ds game and we can be battled in the trainer house
#define PW_EEPROM_SIZE_TEAM_DATA_STRUCT 548
//#define PW_EEPROM_ADDR_0xce24 0xce24  // also written at walk start time as part of the above. probably just to keep the write a multiple of 0x80 bytes
//#define PW_EEPROM_SIZE_0xce24 92
//#define PW_EEPROM_ADDR_0xce80 0xce80  // unused
//#define PW_EEPROM_SIZE_0xce80 8
#define PW_EEPROM_ADDR_0xce88 0xce88 // if low bit set, game will give player a starf berry once per savefile. used when 99999 steps reached
//#define PW_EEPROM_ADDR_0xce89 0xce89 // unused
#define PW_EEPROM_ADDR_0xce8a 0xce8a  // current watts written to eeprom by cmd 0x20 before replying (likely so remote can read them directly). u16 be
#define PW_EEPROM_SIZE_0xce8a 2
#define PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY 0xce8c  // 3x route-available pokemon we've caught so far. 3x struct pokemonsummary
#define PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY 48
#define PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE 16
#define PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY0 (PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY+0*PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE)
#define PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY1 (PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY+1*PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE)
#define PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY2 (PW_EEPROM_ADDR_CAUGHT_POKEMON_SUMMARY+2*PW_EEPROM_SIZE_CAUGHT_POKEMON_SUMMARY_SINGLE)
#define PW_EEPROM_ADDR_OBTAINED_ITEMS 0xcebc  // 3x route-available items we've dowsed so far. 3x {u16 le item, u16 le unused}
#define PW_EEPROM_SIZE_OBTAINED_ITEMS 12
#define PW_EEPROM_SIZE_OBTAINED_ITEMS_SINGLE 4
#define PW_EEPROM_ADDR_OBTAINED_ITEM0 (PW_EEPROM_ADDR_OBTAINED_ITEMS+0*PW_EEPROM_SIZE_OBTAINED_ITEMS_SINGLE)
#define PW_EEPROM_ADDR_OBTAINED_ITEM1 (PW_EEPROM_ADDR_OBTAINED_ITEMS+1*PW_EEPROM_SIZE_OBTAINED_ITEMS_SINGLE)
#define PW_EEPROM_ADDR_OBTAINED_ITEM2 (PW_EEPROM_ADDR_OBTAINED_ITEMS+2*PW_EEPROM_SIZE_OBTAINED_ITEMS_SINGLE)
#define PW_EEPROM_ADDR_PEER_PLAY_ITEMS 0xcec8  // 10x route-available items we've been gifted by peer play. 3x {u16 le item, u16 le unused}
#define PW_EEPROM_SIZE_PEER_PLAY_ITEMS 40
#define PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE 4
#define PW_EEPROM_ADDR_PEER_PLAY_ITEM0 (PW_EEPROM_ADDR_PEER_PLAY_ITEMS+0*PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE)
#define PW_EEPROM_ADDR_PEER_PLAY_ITEM1 (PW_EEPROM_ADDR_PEER_PLAY_ITEMS+1*PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE)
#define PW_EEPROM_ADDR_PEER_PLAY_ITEM2 (PW_EEPROM_ADDR_PEER_PLAY_ITEMS+2*PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE)
#define PW_EEPROM_ADDR_PEER_PLAY_ITEM3 (PW_EEPROM_ADDR_PEER_PLAY_ITEMS+3*PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE)
#define PW_EEPROM_ADDR_PEER_PLAY_ITEM4 (PW_EEPROM_ADDR_PEER_PLAY_ITEMS+4*PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE)
#define PW_EEPROM_ADDR_PEER_PLAY_ITEM5 (PW_EEPROM_ADDR_PEER_PLAY_ITEMS+5*PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE)
#define PW_EEPROM_ADDR_PEER_PLAY_ITEM6 (PW_EEPROM_ADDR_PEER_PLAY_ITEMS+6*PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE)
#define PW_EEPROM_ADDR_PEER_PLAY_ITEM7 (PW_EEPROM_ADDR_PEER_PLAY_ITEMS+7*PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE)
#define PW_EEPROM_ADDR_PEER_PLAY_ITEM8 (PW_EEPROM_ADDR_PEER_PLAY_ITEMS+8*PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE)
#define PW_EEPROM_ADDR_PEER_PLAY_ITEM9 (PW_EEPROM_ADDR_PEER_PLAY_ITEMS+9*PW_EEPROM_SIZE_PEER_PLAY_ITEM_SINGLE)
#define PW_EEPROM_ADDR_HISTORIC_STEP_COUNT 0xcef0  // historic step count per day. u32 each, be, [0] is yesterday, [1] is day before, etc...
#define PW_EEPROM_SIZE_HISTORIC_STEP_COUNT 28
#define PW_EEPROM_ADDR_EVENT_LOG 0xcf0c  // event log. circularly-written, displayed in time order. 24x struct eventlogitem
#define PW_EEPROM_SIZE_EVENT_LOG 3264
#define PW_EEPROM_SIZE_EVENT_LOG_SINGLE 136
#define PW_EEPROM_ADDR_TEAM_DATA_STAGING 0xd480  // team data written here before walk start action. struct teamdata
#define PW_EEPROM_SIZE_TEAM_DATA_STAGING 640
#define PW_EEPROM_ADDR_SCENARIO_DATA_STAGING 0xd700 // scenario data written here before walk start action. everything that 0x8F00-0xB7FF would have
#define PW_EEPROM_SIZE_SCENARIO_DATA_STAGING 8184
#define PW_EEPROM_ADDR_CURRENT_PEER_TEAM_DATA 0xdc00  // current peer play peer. struct teamdata. uploaded as part of peer play. later shifted to index [0] at 0xde24 list of peers
#define PW_EEPROM_SIZE_CURRENT_PEER_TEAM_DATA 548
#define PW_EEPROM_ADDR_MET_PEER_DATA 0xde24  // peers we've met. for battle house info. newest element is first. 10x struct teamdata
#define PW_EEPROM_SIZE_MET_PEER_DATA 5480
#define PW_EEPROM_SIZE_MET_PEER_DATA_SINGLE 548
//#define PW_EEPROM_ADDR_0xf38c 0xf38c  // unused
//#define PW_EEPROM_SIZE_0xf38c 116
#define PW_EEPROM_ADDR_IMG_CURRENT_PEER_POKEMON_ANIMATED_SMALL 0xf400  // peer play temporary data about peer, medium pokemon animated image of pokemon we are peer-playing with (never erased) 32x24 x 2 frames
#define PW_EEPROM_SIZE_IMG_CURRENT_PEER_POKEMON_ANIMATED_SMALL 384
#define PW_EEPROM_SIZE_IMG_CURRENT_PEER_POKEMON_ANIMATED_SMALL_FRAME 192
#define PW_EEPROM_ADDR_IMG_CURRENT_PEER_POKEMON_ANIMATED_SMALL_FRAME1 (PW_EEPROM_ADDR_IMG_CURRENT_PEER_POKEMON_ANIMATED_SMALL+0*PW_EEPROM_SIZE_IMG_CURRENT_PEER_POKEMON_ANIMATED_SMALL_FRAME)
#define PW_EEPROM_ADDR_IMG_CURRENT_PEER_POKEMON_ANIMATED_SMALL_FRAME2 (PW_EEPROM_ADDR_IMG_CURRENT_PEER_POKEMON_ANIMATED_SMALL+1*PW_EEPROM_SIZE_IMG_CURRENT_PEER_POKEMON_ANIMATED_SMALL_FRAME)
#define PW_EEPROM_ADDR_TEXT_CURRENT_PEER_POKEMON_NAME 0xf580  // rendered text name of pokemon we are peer-playing with 80x16
#define PW_EEPROM_SIZE_TEXT_CURRENT_PEER_POKEMON_NAME 320
#define PW_EEPROM_ADDR_CURRENT_PEER_DATA 0xf6c0  // data. struct peerplaydata
#define PW_EEPROM_SIZE_CURRENT_PEER_DATA 56

#endif /* PW_EEPROM_MAP_H */
