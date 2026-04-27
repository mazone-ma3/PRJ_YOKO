#include <x68k/iocs.h>
#include "inkey.h"
#include "fmd68.h"

//#define XSP

#define SCREEN2 0
#define COUNT1S 56

enum{
	width = 256,
	height = 192
};

#define CHRPAL_NO 0
#define REVPAL_NO 1
#define BGPAL_NO 2

#ifdef XSP
#define MAX_SPRITE 512
#else
#define MAX_SPRITE 128
#endif

#define SHIFT_NUM	0 //3

enum {
	SPR_OFS_X = -16,
	SPR_OFS_Y = -16
};

#define PCG_SPACE (0x40 - '0')
#define PCGPARTS (16 * 4)
#define PCG_SPACE (0x40 - '0')
#define SPRPARTS 128

#ifdef XSP
#define CHR_TOP 0
#else
#define CHR_TOP (PCGPARTS / 4)
#endif

#ifndef XSP
/* スプライト位置を定義するマクロ */
/*#define DEF_SP_SINGLE( NO, X, Y, PAT, PAL, ATR, PRO) {\
	pchr_data = &chr_data[NO];\
	pchr_data->x = (X >> SHIFT_NUM) + SPR_OFS_X + 16; \
	pchr_data->y = (Y >> SHIFT_NUM) + SPR_OFS_Y + 16; \
	pchr_data->pat_num = PAT + (PAL << 8) + (ATR << 14); \
	pchr_data->atr = 0x0002; \
	NO++; \
}*/
#define  sprite_set( NO, X, Y, PAT, PAL, VS) {\
	pchr_data = &chr_data[NO];\
	pchr_data->x = (X >> SHIFT_NUM) + SPR_OFS_X + 16; \
	pchr_data->y = (Y >> SHIFT_NUM) + SPR_OFS_Y + 16; \
	pchr_data->pat_num = PAT + CHR_TOP; \
	pchr_data->atr = 0x0002; \
}
#else
/*#define DEF_SP_SINGLE( NO, X, Y, PAT, PAL, ATR, PRO) {\
	pchr_data = &chr_data[NO];\
	pchr_data->x = (X >> SHIFT_NUM) + SPR_OFS_X + 16; \
	pchr_data->y = (Y >> SHIFT_NUM) + SPR_OFS_Y + 16; \
	pchr_data->pat_num = PAT & 0x3fff; \
	pchr_data->atr = (PAL << 8) | (ATR << 14) | 0x20 | PRO | (PAT & 0xc000);\
	NO++; \
}*/
#define  sprite_set( NO, X, Y, PAT, PAL, VS) {\
	pchr_data = &chr_data[NO];\
	pchr_data->x = (X >> SHIFT_NUM) + SPR_OFS_X + 16; \
	pchr_data->y = (Y >> SHIFT_NUM) + SPR_OFS_Y + 16; \
	pchr_data->pat_num = PAT & 0x3fff; \
	pchr_data->atr = 0x0020 | (PAT & 0xc000); \
}
#endif

/* 各キャラクタの構造体(CHR_PARA) */
typedef struct chr_para{
	short x,y, pat_num,atr;
} CHR_PARA;

void init_star(void);
void set_sprite_all(void);
void bg_roll(void);
void wait_vsync(void);
void se(void);
void cls(void);
unsigned char keyscan(void);
void put_strings(int scr, int x, int y,  char *str, char pal);
void put_numd(long j, char digit);

extern volatile int spr_count;
extern unsigned char seflag;
extern int playmode;

extern CHR_PARA chr_data[MAX_SPRITE * 2];

extern CHR_PARA *pchr_data;

extern unsigned char str_temp[11];
