#include "inkey.h"
#include "fmdtwg.h"

#define COUNT1S 60
#define MAX_SPRITE 227
#define CHRPAL_NO 0
#define REVPAL_NO 1
#define BGPAL_NO 2
#define CHR_TOP 128
#define SCREEN2 0
#define CHRPAL_NO 0
#define FONTPARTS (16 * 4)
#define TITLEPARTS 0 //(8 * 4)
#define SPRPARTS 128

#define ERROR 1
#define NOERROR 0

#define SHIFT_NUM	0 //3

enum {
	IMG_SIZE_X = 16,
	SPR_OFS_X = -16,
	SPR_OFS_Y = -16
};

enum{
	width = 256,
	height = 192
};

/* スプライト位置を定義するマクロ */
#define DEF_SP_SINGLE( NO, X, Y, PAT, PAL, ATR) {\
	pchr_data = &chr_data[NO];\
	pchr_data->x = (X >> SHIFT_NUM) + SPR_OFS_X; \
	pchr_data->y = (Y >> SHIFT_NUM) + SPR_OFS_Y + 2; \
	pchr_data->pat_num = PAT | (ATR << 10); \
	pchr_data->atr = (PAL + 256) | 0x8000; \
	NO++; \
}

#define sprite_set( NO, X, Y, PAT, ATR, PRO) {\
	pchr_data = &chr_data[NO];\
	pchr_data->x = (X >> SHIFT_NUM) + SPR_OFS_X; \
	pchr_data->y = (Y >> SHIFT_NUM) + SPR_OFS_Y + 2; \
	pchr_data->pat_num = PAT + CHR_TOP + FONTPARTS + TITLEPARTS;\
	pchr_data->atr = (CHRPAL_NO + 256) | 0x8000; \
}

/* 各キャラクタの構造体(CHR_PARA) */
typedef struct chr_para{
	int x,y,xx,yy,pat_num,atr,count,hp;
} CHR_PARA;

typedef struct chr_para2{
	short x , y, pat_num, atr;
} CHR_PARA2;

void init_star(void);
void wait_sprite(void);
void set_sprite_all(void);
void wait_vsync(void);
void bg_roll(void);
void se(void);
void cls(void);
unsigned char keyscan(void);
void put_strings(int scr, int x, int y,  char *str, char pal);
void put_numd(long j, char digit);

extern unsigned char seflag;
extern unsigned int encode;

extern int playmode;

extern volatile int spr_count;

extern CHR_PARA2 chr_data[MAX_SPRITE * 2];
extern CHR_PARA2 *pchr_data;

extern unsigned char str_temp[11];
