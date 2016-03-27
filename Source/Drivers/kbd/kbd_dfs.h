#ifndef KBD_DFS_H
#define KBD_DFS_H

#define	KEY_F1		0x80
#define	KEY_F2		(KEY_F1 + 1)
#define	KEY_F3		(KEY_F2 + 1)
#define	KEY_F4		(KEY_F3 + 1)
#define	KEY_F5		(KEY_F4 + 1)
#define	KEY_F6		(KEY_F5 + 1)
#define	KEY_F7		(KEY_F6 + 1)
#define	KEY_F8		(KEY_F7 + 1)
#define	KEY_F9		(KEY_F8 + 1)
#define	KEY_F10		(KEY_F9 + 1)
#define	KEY_F11		(KEY_F10 + 1)
#define	KEY_F12		(KEY_F11 + 1)
#define	KEY_INS		0x90
#define	KEY_DEL		(KEY_INS + 1)
#define	KEY_HOME	(KEY_DEL + 1)
#define	KEY_END		(KEY_HOME + 1)
#define	KEY_PGUP	(KEY_END + 1)
#define	KEY_PGDN	(KEY_PGUP + 1)
#define	KEY_LFT		(KEY_PGDN + 1)
#define	KEY_UP		(KEY_LFT + 1)
#define	KEY_DN		(KEY_UP + 1)
#define	KEY_RT		(KEY_DN + 1)
#define	KEY_PRNT	(KEY_RT + 1)
#define	KEY_PAUSE	(KEY_PRNT + 1)
#define	KEY_LWIN	(KEY_PAUSE + 1)
#define	KEY_RWIN	(KEY_LWIN + 1)
#define	KEY_MENU	(KEY_RWIN + 1)

#define	KBD_META_SHIFT	0x0001
#define	KBD_META_ALT 	0x0002
#define	KBD_META_CTRL	0x0004
#define	KBD_META_ANY	(KBD_META_ALT | KBD_META_CTRL | KBD_META_SHIFT)
#define	KBD_META_CAPS	0x0010
#define	KBD_META_NUM	0x0020
#define	KBD_META_SCRL 	0x0040
#define KBD_META_ESC	0x0100
#define KBD_META_BRKCD	0x0200

#define	RAW1_LEFT_CTRL		0x1D
#define	RAW1_LEFT_SHIFT		0x2A
#define	RAW1_CAPS_LOCK		0x3A
#define	RAW1_LEFT_ALT		0x38
#define	RAW1_RIGHT_ALT		0x38
#define	RAW1_RIGHT_CTRL		0x1D
#define	RAW1_RIGHT_SHIFT	0x36
#define	RAW1_SCROLL_LOCK	0x46
#define	RAW1_NUM_LOCK		0x45
#define	RAW1_DEL			0x53

const uint8 KeyMapLength = 0x58 + 1;

const uint8 KeyTable[] = {
/* 00 */0,		0x1B,	'1',	'2',	'3',	'4',	'5',	'6',
/* 08 */'7',	'8',	'9',	'0',	'-',	'=',	'\b',	'\t',
/* 10 */'q',	'w',	'e',	'r',	't',	'y',	'u',	'i',
/* 1Dh to lewy Ctrl */
/* 18 */'o',	'p',	'[',	']',	'\n',	0,		'a',	's',
/* 20 */'d',	'f',	'g',	'h',	'j',	'k',	'l',	';',
/* 2Ah to lewy Shift */
/* 28 */'\'',	'`',	0,		'\\',	'z',	'x',	'c',	'v',
/* 36h to prawy Shift */
/* 30 */'b',	'n',	'm',	',',	'.',	'/',	0,		0,
/* 38h to lewy Alt, 3Ah to Caps Lock */
/* 38 */0,		' ',	0,		KEY_F1,	KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,
/* 45h to Num Lock, 46h to Scroll Lock */
/* 40 */KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10,0,		0,		KEY_HOME,
/* 48 */KEY_UP,	KEY_PGUP,'-',	KEY_LFT,'5',	KEY_RT,	'+',	KEY_END,
/* 50 */KEY_DN,	KEY_PGDN,KEY_INS,KEY_DEL,0,		0,		0,		KEY_F11,
/* 58 */KEY_F12,
};

const uint8 KeyTableShift[] = {
/* 00 */0,		0x1B,	'!',	'@',	'#',	'$',	'%',	'^',
/* 08 */'&',	'*',	'(',	')',	'_',	'+',	'\b',	'\t',
/* 10 */'Q',	'W',	'E',	'R',	'T',	'Y',	'U',	'I',
/* 18 */'O',	'P',	'{',	'}',	'\n',	0,		'A',	'S',
/* 20 */'D',	'F',	'G',	'H',	'J',	'K',	'L',	':',
/* 28 */'"',	'~',	0,		'|',	'Z',	'X',	'C',	'V',
/* 30 */'B',	'N',	'M',	'<',	'>',	'?',	0,		0,
/* 38 */0,		' ',	0,		KEY_F1,	KEY_F2,	KEY_F3,	KEY_F4,	KEY_F5,
/* 40 */KEY_F6,	KEY_F7,	KEY_F8,	KEY_F9,	KEY_F10,0,		0,		KEY_HOME,
/* 48 */KEY_UP,	KEY_PGUP,'-',	KEY_LFT,'5',	KEY_RT,	'+',	KEY_END,
/* 50 */KEY_DN,	KEY_PGDN,KEY_INS,KEY_DEL,0,		0,		0,		KEY_F11,
/* 58 */KEY_F12,
};

#endif


















