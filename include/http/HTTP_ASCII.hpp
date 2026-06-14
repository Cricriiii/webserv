#ifndef HTTP_ASCII_HPP
# define HTTP_ASCII_HPP

# define NUL	'\x00'
# define SOH	'\x01'
# define STX	'\x02'
# define ETX	'\x03'
# define EOT	'\x04'
# define ENQ	'\x05'
# define ACK	'\x06'
# define BEL	'\x07'
# define BS		'\x08'
# define HT		'\x09'
# define LF		'\x0A'
# define VT		'\x0B'
# define FF		'\x0C'
# define CR		'\x0D'
# define SO		'\x0E'
# define SI		'\x0F'
# define DLE	'\x10'
# define DC1	'\x11'
# define DC2	'\x12'
# define DC3	'\x13'
# define DC4	'\x14'
# define NAK	'\x15'
# define SYN	'\x16'
# define ETB	'\x17'
# define CAN	'\x18'
# define EM		'\x19'
# define SUB	'\x1A'
# define ESC	'\x1B'
# define FS		'\x1C'
# define GS		'\x1D'
# define RS		'\x1E'
# define US		'\x1F'
# define DEL	'\x7F'


# define HTTP_BIT		(1 << 0)
# define HTTP_ALPHA		(1 << 1)
# define HTTP_CHAR		(1 << 2)
# define HTTP_CR		(1 << 3)
# define HTTP_LF		(1 << 4)
# define HTTP_CRLF		(1 << 5)
# define HTTP_CTL		(1 << 6)
# define HTTP_DIGIT		(1 << 7)
# define HTTP_DQUOTE	(1 << 8)
# define HTTP_HEXDIG	(1 << 9)
# define HTTP_HTAB		(1 << 10)
# define HTTP_LWSP		(1 << 11)
# define HTTP_OCTET		(1 << 12)
# define HTTP_SP		(1 << 13)
# define HTTP_VCHAR		(1 << 14)
# define HTTP_ICHAR		(1 << 15)
# define HTTP_TCHAR		(1 << 16)
# define HTTP_WSP		(1 << 17)
# define HTTP_HOSTNAME	(1 << 18)


inline const int*	http_symbols() {

	static int	table[256] = {0};
	static bool	build = true;

	if (build) {

	table[0x00]	= HTTP_CTL,												/* NUL */
	table[0x01]	= HTTP_CHAR | HTTP_CTL,									/* SOH */
	table[0x02]	= HTTP_CHAR | HTTP_CTL,									/* STX */
	table[0x03]	= HTTP_CHAR | HTTP_CTL,									/* ETX */
	table[0x04]	= HTTP_CHAR | HTTP_CTL,									/* EOT */
	table[0x05]	= HTTP_CHAR | HTTP_CTL,									/* ENQ */
	table[0x06]	= HTTP_CHAR | HTTP_CTL,									/* ACK */
	table[0x07]	= HTTP_CHAR | HTTP_CTL,									/* BEL */
	table[0x08]	= HTTP_CHAR | HTTP_CTL,									/* BS  */
	table[0x09]	= HTTP_CHAR | HTTP_CTL | HTTP_HTAB | HTTP_WSP,			/* HT  */
	table[0x0A]	= HTTP_CHAR | HTTP_LF | HTTP_CRLF | HTTP_CTL,			/* LF  */
	table[0x0B]	= HTTP_CHAR | HTTP_CTL,									/* VT  */
	table[0x0C]	= HTTP_CHAR | HTTP_CTL,									/* FF  */
	table[0x0D]	= HTTP_CHAR | HTTP_CR | HTTP_CRLF | HTTP_CTL,			/* CR  */
	table[0x0E]	= HTTP_CHAR | HTTP_CTL,									/* SO  */
	table[0x0F]	= HTTP_CHAR | HTTP_CTL,									/* SI  */
	table[0x10]	= HTTP_CHAR | HTTP_CTL,									/* DLE */
	table[0x11]	= HTTP_CHAR | HTTP_CTL,									/* DC1 */
	table[0x12]	= HTTP_CHAR | HTTP_CTL,									/* DC2 */
	table[0x13]	= HTTP_CHAR | HTTP_CTL,									/* DC3 */
	table[0x14]	= HTTP_CHAR | HTTP_CTL,									/* DC4 */
	table[0x15]	= HTTP_CHAR | HTTP_CTL,									/* NAK */
	table[0x16]	= HTTP_CHAR | HTTP_CTL,									/* SYN */
	table[0x17]	= HTTP_CHAR | HTTP_CTL,									/* ETB */
	table[0x18]	= HTTP_CHAR | HTTP_CTL,									/* CAN */
	table[0x19]	= HTTP_CHAR | HTTP_CTL,									/* EM  */
	table[0x1A]	= HTTP_CHAR | HTTP_CTL,									/* SUB */
	table[0x1B]	= HTTP_CHAR | HTTP_CTL,									/* ESC */
	table[0x1C]	= HTTP_CHAR | HTTP_CTL,									/* FS  */
	table[0x1D]	= HTTP_CHAR | HTTP_CTL,									/* GS  */
	table[0x1E]	= HTTP_CHAR | HTTP_CTL,									/* RS  */
	table[0x1F]	= HTTP_CHAR | HTTP_CTL,									/* US  */
	table[0x20]	= HTTP_CHAR | HTTP_SP | HTTP_VCHAR | HTTP_WSP,			/* ' ' */
	table[0x21]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR,					/* '!' */
	table[0x22]	= HTTP_CHAR | HTTP_DQUOTE | HTTP_VCHAR,					/* '"' */
	table[0x23]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR,					/* '#' */
	table[0x24]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR,					/* '$' */
	table[0x25]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR,					/* '%' */
	table[0x26]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR,					/* '&' */
	table[0x27]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR,					/* '\'' */
	table[0x28]	= HTTP_CHAR | HTTP_VCHAR,								/* '(' */
	table[0x29]	= HTTP_CHAR | HTTP_VCHAR,								/* ')' */
	table[0x2A]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR,					/* '*' */
	table[0x2B]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR,					/* '+' */
	table[0x2C]	= HTTP_CHAR | HTTP_VCHAR,								/* ',' */
	table[0x2D]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,	/* '-' */
	table[0x2E]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,	/* '.' */
	table[0x2F]	= HTTP_CHAR | HTTP_VCHAR,								/* '/' */
	table[0x30]	= HTTP_BIT | HTTP_CHAR | HTTP_DIGIT | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,	/* '0' */
	table[0x31]	= HTTP_BIT | HTTP_CHAR | HTTP_DIGIT | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,	/* '1' */
	table[0x32]	= HTTP_CHAR | HTTP_DIGIT | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* '2' */
	table[0x33]	= HTTP_CHAR | HTTP_DIGIT | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* '3' */
	table[0x34]	= HTTP_CHAR | HTTP_DIGIT | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* '4' */
	table[0x35]	= HTTP_CHAR | HTTP_DIGIT | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* '5' */
	table[0x36]	= HTTP_CHAR | HTTP_DIGIT | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* '6' */
	table[0x37]	= HTTP_CHAR | HTTP_DIGIT | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* '7' */
	table[0x38]	= HTTP_CHAR | HTTP_DIGIT | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* '8' */
	table[0x39]	= HTTP_CHAR | HTTP_DIGIT | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* '9' */
	table[0x3A]	= HTTP_CHAR | HTTP_VCHAR,																		/* ':' */
	table[0x3B]	= HTTP_CHAR | HTTP_VCHAR,																		/* ';' */
	table[0x3C]	= HTTP_CHAR | HTTP_VCHAR,																		/* '<' */
	table[0x3D]	= HTTP_CHAR | HTTP_VCHAR,																		/* '=' */
	table[0x3E]	= HTTP_CHAR | HTTP_VCHAR,																		/* '>' */
	table[0x3F]	= HTTP_CHAR | HTTP_VCHAR,																		/* '?' */
	table[0x40]	= HTTP_CHAR | HTTP_VCHAR,																		/* '@' */
	table[0x41]	= HTTP_ALPHA | HTTP_CHAR | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* 'A' */
	table[0x42]	= HTTP_ALPHA | HTTP_CHAR | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* 'B' */
	table[0x43]	= HTTP_ALPHA | HTTP_CHAR | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* 'C' */
	table[0x44]	= HTTP_ALPHA | HTTP_CHAR | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* 'D' */
	table[0x45]	= HTTP_ALPHA | HTTP_CHAR | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* 'E' */
	table[0x46]	= HTTP_ALPHA | HTTP_CHAR | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* 'F' */
	table[0x47]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'G' */
	table[0x48]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'H' */
	table[0x49]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'I' */
	table[0x4A]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'J' */
	table[0x4B]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'K' */
	table[0x4C]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'L' */
	table[0x4D]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'M' */
	table[0x4E]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'N' */
	table[0x4F]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'O' */
	table[0x50]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'P' */
	table[0x51]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'Q' */
	table[0x52]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'R' */
	table[0x53]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'S' */
	table[0x54]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'T' */
	table[0x55]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'U' */
	table[0x56]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'V' */
	table[0x57]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'W' */
	table[0x58]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'X' */
	table[0x59]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'Y' */
	table[0x5A]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'Z' */
	table[0x5B]	= HTTP_CHAR | HTTP_VCHAR | HTTP_HOSTNAME,														/* '[' */
	table[0x5C]	= HTTP_CHAR | HTTP_VCHAR,																		/* '\\' */
	table[0x5D]	= HTTP_CHAR | HTTP_VCHAR | HTTP_HOSTNAME,														/* ']' */
	table[0x5E]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR,															/* '^' */
	table[0x5F]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,											/* '_' */
	table[0x60]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR,															/* '`' */
	table[0x61]	= HTTP_ALPHA | HTTP_CHAR | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* 'a' */
	table[0x62]	= HTTP_ALPHA | HTTP_CHAR | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* 'b' */
	table[0x63]	= HTTP_ALPHA | HTTP_CHAR | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* 'c' */
	table[0x64]	= HTTP_ALPHA | HTTP_CHAR | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* 'd' */
	table[0x65]	= HTTP_ALPHA | HTTP_CHAR | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* 'e' */
	table[0x66]	= HTTP_ALPHA | HTTP_CHAR | HTTP_HEXDIG | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,				/* 'f' */
	table[0x67]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'g' */
	table[0x68]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'h' */
	table[0x69]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'i' */
	table[0x6A]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'j' */
	table[0x6B]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'k' */
	table[0x6C]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'l' */
	table[0x6D]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'm' */
	table[0x6E]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'n' */
	table[0x6F]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'o' */
	table[0x70]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'p' */
	table[0x71]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'q' */
	table[0x72]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'r' */
	table[0x73]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 's' */
	table[0x74]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 't' */
	table[0x75]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'u' */
	table[0x76]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'v' */
	table[0x77]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'w' */
	table[0x78]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'x' */
	table[0x79]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'y' */
	table[0x7A]	= HTTP_ALPHA | HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,								/* 'z' */
	table[0x7B]	= HTTP_CHAR | HTTP_VCHAR,																		/* '{' */
	table[0x7C]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR,															/* '|' */
	table[0x7D]	= HTTP_CHAR | HTTP_VCHAR,																		/* '}' */
	table[0x7E]	= HTTP_CHAR | HTTP_VCHAR | HTTP_TCHAR | HTTP_HOSTNAME,											/* '~' */
	table[0x7F]	= HTTP_CHAR,																					/* DEL */


	/* [128-255] */
	table[0x80] = HTTP_OCTET | HTTP_ICHAR, table[0x81] = HTTP_OCTET | HTTP_ICHAR, table[0x82] = HTTP_OCTET | HTTP_ICHAR, table[0x83] = HTTP_OCTET | HTTP_ICHAR,
	table[0x84] = HTTP_OCTET | HTTP_ICHAR, table[0x85] = HTTP_OCTET | HTTP_ICHAR, table[0x86] = HTTP_OCTET | HTTP_ICHAR, table[0x87] = HTTP_OCTET | HTTP_ICHAR,
	table[0x88] = HTTP_OCTET | HTTP_ICHAR, table[0x89] = HTTP_OCTET | HTTP_ICHAR, table[0x8A] = HTTP_OCTET | HTTP_ICHAR, table[0x8B] = HTTP_OCTET | HTTP_ICHAR,
	table[0x8C] = HTTP_OCTET | HTTP_ICHAR, table[0x8D] = HTTP_OCTET | HTTP_ICHAR, table[0x8E] = HTTP_OCTET | HTTP_ICHAR, table[0x8F] = HTTP_OCTET | HTTP_ICHAR,
	table[0x90] = HTTP_OCTET | HTTP_ICHAR, table[0x91] = HTTP_OCTET | HTTP_ICHAR, table[0x92] = HTTP_OCTET | HTTP_ICHAR, table[0x93] = HTTP_OCTET | HTTP_ICHAR,
	table[0x94] = HTTP_OCTET | HTTP_ICHAR, table[0x95] = HTTP_OCTET | HTTP_ICHAR, table[0x96] = HTTP_OCTET | HTTP_ICHAR, table[0x97] = HTTP_OCTET | HTTP_ICHAR,
	table[0x98] = HTTP_OCTET | HTTP_ICHAR, table[0x99] = HTTP_OCTET | HTTP_ICHAR, table[0x9A] = HTTP_OCTET | HTTP_ICHAR, table[0x9B] = HTTP_OCTET | HTTP_ICHAR,
	table[0x9C] = HTTP_OCTET | HTTP_ICHAR, table[0x9D] = HTTP_OCTET | HTTP_ICHAR, table[0x9E] = HTTP_OCTET | HTTP_ICHAR, table[0x9F] = HTTP_OCTET | HTTP_ICHAR,
	table[0xA0] = HTTP_OCTET | HTTP_ICHAR, table[0xA1] = HTTP_OCTET | HTTP_ICHAR, table[0xA2] = HTTP_OCTET | HTTP_ICHAR, table[0xA3] = HTTP_OCTET | HTTP_ICHAR,
	table[0xA4] = HTTP_OCTET | HTTP_ICHAR, table[0xA5] = HTTP_OCTET | HTTP_ICHAR, table[0xA6] = HTTP_OCTET | HTTP_ICHAR, table[0xA7] = HTTP_OCTET | HTTP_ICHAR,
	table[0xA8] = HTTP_OCTET | HTTP_ICHAR, table[0xA9] = HTTP_OCTET | HTTP_ICHAR, table[0xAA] = HTTP_OCTET | HTTP_ICHAR, table[0xAB] = HTTP_OCTET | HTTP_ICHAR,
	table[0xAC] = HTTP_OCTET | HTTP_ICHAR, table[0xAD] = HTTP_OCTET | HTTP_ICHAR, table[0xAE] = HTTP_OCTET | HTTP_ICHAR, table[0xAF] = HTTP_OCTET | HTTP_ICHAR,
	table[0xB0] = HTTP_OCTET | HTTP_ICHAR, table[0xB1] = HTTP_OCTET | HTTP_ICHAR, table[0xB2] = HTTP_OCTET | HTTP_ICHAR, table[0xB3] = HTTP_OCTET | HTTP_ICHAR,
	table[0xB4] = HTTP_OCTET | HTTP_ICHAR, table[0xB5] = HTTP_OCTET | HTTP_ICHAR, table[0xB6] = HTTP_OCTET | HTTP_ICHAR, table[0xB7] = HTTP_OCTET | HTTP_ICHAR,
	table[0xB8] = HTTP_OCTET | HTTP_ICHAR, table[0xB9] = HTTP_OCTET | HTTP_ICHAR, table[0xBA] = HTTP_OCTET | HTTP_ICHAR, table[0xBB] = HTTP_OCTET | HTTP_ICHAR,
	table[0xBC] = HTTP_OCTET | HTTP_ICHAR, table[0xBD] = HTTP_OCTET | HTTP_ICHAR, table[0xBE] = HTTP_OCTET | HTTP_ICHAR, table[0xBF] = HTTP_OCTET | HTTP_ICHAR,
	table[0xC0] = HTTP_OCTET | HTTP_ICHAR, table[0xC1] = HTTP_OCTET | HTTP_ICHAR, table[0xC2] = HTTP_OCTET | HTTP_ICHAR, table[0xC3] = HTTP_OCTET | HTTP_ICHAR,
	table[0xC4] = HTTP_OCTET | HTTP_ICHAR, table[0xC5] = HTTP_OCTET | HTTP_ICHAR, table[0xC6] = HTTP_OCTET | HTTP_ICHAR, table[0xC7] = HTTP_OCTET | HTTP_ICHAR,
	table[0xC8] = HTTP_OCTET | HTTP_ICHAR, table[0xC9] = HTTP_OCTET | HTTP_ICHAR, table[0xCA] = HTTP_OCTET | HTTP_ICHAR, table[0xCB] = HTTP_OCTET | HTTP_ICHAR,
	table[0xCC] = HTTP_OCTET | HTTP_ICHAR, table[0xCD] = HTTP_OCTET | HTTP_ICHAR, table[0xCE] = HTTP_OCTET | HTTP_ICHAR, table[0xCF] = HTTP_OCTET | HTTP_ICHAR,
	table[0xD0] = HTTP_OCTET | HTTP_ICHAR, table[0xD1] = HTTP_OCTET | HTTP_ICHAR, table[0xD2] = HTTP_OCTET | HTTP_ICHAR, table[0xD3] = HTTP_OCTET | HTTP_ICHAR,
	table[0xD4] = HTTP_OCTET | HTTP_ICHAR, table[0xD5] = HTTP_OCTET | HTTP_ICHAR, table[0xD6] = HTTP_OCTET | HTTP_ICHAR, table[0xD7] = HTTP_OCTET | HTTP_ICHAR,
	table[0xD8] = HTTP_OCTET | HTTP_ICHAR, table[0xD9] = HTTP_OCTET | HTTP_ICHAR, table[0xDA] = HTTP_OCTET | HTTP_ICHAR, table[0xDB] = HTTP_OCTET | HTTP_ICHAR,
	table[0xDC] = HTTP_OCTET | HTTP_ICHAR, table[0xDD] = HTTP_OCTET | HTTP_ICHAR, table[0xDE] = HTTP_OCTET | HTTP_ICHAR, table[0xDF] = HTTP_OCTET | HTTP_ICHAR,
	table[0xE0] = HTTP_OCTET | HTTP_ICHAR, table[0xE1] = HTTP_OCTET | HTTP_ICHAR, table[0xE2] = HTTP_OCTET | HTTP_ICHAR, table[0xE3] = HTTP_OCTET | HTTP_ICHAR,
	table[0xE4] = HTTP_OCTET | HTTP_ICHAR, table[0xE5] = HTTP_OCTET | HTTP_ICHAR, table[0xE6] = HTTP_OCTET | HTTP_ICHAR, table[0xE7] = HTTP_OCTET | HTTP_ICHAR,
	table[0xE8] = HTTP_OCTET | HTTP_ICHAR, table[0xE9] = HTTP_OCTET | HTTP_ICHAR, table[0xEA] = HTTP_OCTET | HTTP_ICHAR, table[0xEB] = HTTP_OCTET | HTTP_ICHAR,
	table[0xEC] = HTTP_OCTET | HTTP_ICHAR, table[0xED] = HTTP_OCTET | HTTP_ICHAR, table[0xEE] = HTTP_OCTET | HTTP_ICHAR, table[0xEF] = HTTP_OCTET | HTTP_ICHAR,
	table[0xF0] = HTTP_OCTET | HTTP_ICHAR, table[0xF1] = HTTP_OCTET | HTTP_ICHAR, table[0xF2] = HTTP_OCTET | HTTP_ICHAR, table[0xF3] = HTTP_OCTET | HTTP_ICHAR,
	table[0xF4] = HTTP_OCTET | HTTP_ICHAR, table[0xF5] = HTTP_OCTET | HTTP_ICHAR, table[0xF6] = HTTP_OCTET | HTTP_ICHAR, table[0xF7] = HTTP_OCTET | HTTP_ICHAR,
	table[0xF8] = HTTP_OCTET | HTTP_ICHAR, table[0xF9] = HTTP_OCTET | HTTP_ICHAR, table[0xFA] = HTTP_OCTET | HTTP_ICHAR, table[0xFB] = HTTP_OCTET | HTTP_ICHAR,
	table[0xFC] = HTTP_OCTET | HTTP_ICHAR, table[0xFD] = HTTP_OCTET | HTTP_ICHAR, table[0xFE] = HTTP_OCTET | HTTP_ICHAR, table[0xFF] = HTTP_OCTET | HTTP_ICHAR;

	build = false;
	}

	return table;
}


# define IS_BIT(c)		(http_symbols()[(unsigned char)c] & HTTP_BIT)
# define IS_ALPHA(c)	(http_symbols()[(unsigned char)c] & HTTP_ALPHA)
# define IS_CHAR(c)		(http_symbols()[(unsigned char)c] & HTTP_CHAR)
# define IS_CR(c)		(http_symbols()[(unsigned char)c] & HTTP_CR)
# define IS_LF(c)		(http_symbols()[(unsigned char)c] & HTTP_LF)
# define IS_CRLF(c)		(http_symbols()[(unsigned char)c] & (HTTP_CR | HTTP_LF))
# define IS_CTL(c)		(http_symbols()[(unsigned char)c] & HTTP_CTL)
# define IS_DIGIT(c)	(http_symbols()[(unsigned char)c] & HTTP_DIGIT)
# define IS_DQUOTE(c)	(http_symbols()[(unsigned char)c] & HTTP_DQUOTE)
# define IS_HEXDIG(c)	(http_symbols()[(unsigned char)c] & HTTP_HEXDIG)
# define IS_HTAB(c)		(http_symbols()[(unsigned char)c] & HTTP_HTAB)
# define IS_LWSP(c)		(http_symbols()[(unsigned char)c] & HTTP_LWSP)
# define IS_OCTET(c)	(http_symbols()[(unsigned char)c] & HTTP_OCTET)
# define IS_SP(c)		(http_symbols()[(unsigned char)c] & HTTP_SP)
# define IS_VCHAR(c)	(http_symbols()[(unsigned char)c] & HTTP_VCHAR)
# define IS_ICHAR(c)	(http_symbols()[(unsigned char)c] & HTTP_ICHAR)
# define IS_TCHAR(c)	(http_symbols()[(unsigned char)c] & HTTP_TCHAR)
# define IS_WSP(c)		(http_symbols()[(unsigned char)c] & HTTP_WSP)
# define IS_ESCAPAB(c)	(http_symbols()[(unsigned char)c] & (HTTP_HTAB | HTTP_SP | HTTP_VCHAR | HTTP_ICHAR))
# define IS_HOST(c)		(http_symbols()[(unsigned char)c] & HTTP_HOSTNAME)


static inline int	hex_to_int(char c)
{
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	else if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	}
	else if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	}
	else {
		return -1;
	}
}

#endif
