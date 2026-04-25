#include "inkey.h"

#define COUNT1S 60

#define width 256
#define height 192
#define CHRPAL_NO 0
#define REVPAL_NO 1
#define TRUE 1
#define FALSE 0
#define ERROR 1
#define NOERROR 0
#define SCREEN2 0
#define MAX_SPRITE 64			/* 最大許容スプライト表示数 */

#define SHIFT_NUM_Y	0
#define SHIFT_NUM	0		/* 座標系をシフトする回数(固定小数点演算用) */

enum {
	SPR_OFS_X = -16, //16,
	SPR_OFS_Y = -16, //-16,
};

typedef struct chr_para3{
	unsigned char yl;	// Y -512～+511
	unsigned char yh;	// Y -512～+511
	unsigned char mgy;	// 垂直サイズ (0=256)
	unsigned char ps;	// 透明度(TP)、反転フラグ(RVY/RVX、パレット(PS3-0))
	unsigned char xl;	// X -512～+511
	unsigned char xh;	// X -512～+511
	unsigned char mgx;	// 水平サイズ (0=256)
	unsigned char pat_num;	// Pattern Set パーツ番号(PY/PX) 16dot毎
} CHR_PARA3;

typedef struct chr_para4{
	unsigned char pat_num,atr;
//	unsigned char pal;
} CHR_PARA4;

// V9968 SPMode3
#define DEF_SP_SINGLE(NO, X, Y, PAT, PAL, VS) {\
	pchr_data = &chr_data[tmp_spr_count++];\
	tx = (((X) >> SHIFT_NUM) + SPR_OFS_X); \
	pchr_data->xl = tx & 0xff; \
	pchr_data->xh = (tx >> 8) & 0x03; \
	ty = (((Y) >> SHIFT_NUM_Y) + SPR_OFS_Y - 1); \
	pchr_data->yl = ty & 0xff; \
	pchr_data->yh = (ty >> 8) & 0x03; \
	pchr_data->pat_num = (PAT); \
	pchr_data->mgx = 16; \
	pchr_data->mgy = 16; \
	pchr_data->ps = PAL; \
}
//	NO++;\
//}

#define sprite_set(NO, X, Y, PAT, PAL, VS) DEF_SP_SINGLE(NO, X, Y, PAT, 0, VS)

void init_star(void);
void bg_roll(void);
unsigned char get_key(unsigned char matrix) __sdcccall(1);
void se(void);
unsigned char play_fmdbgm(void) __sdcccall(1);
void stop_fmdbgm(void) __sdcccall(1);
void wait_vsync(void);
void put_strings(unsigned char scr, unsigned char x, unsigned char y,  char *str, unsigned char pal);
void put_numd(long j, unsigned char digit) __sdcccall(1);

void set_sprite_all(void);
void cls(void);
unsigned char keyscan(void);

void key_flush(void);

extern unsigned char seflag;
extern unsigned char playmode;
extern unsigned char str_temp[11];
extern unsigned char total_count;
extern unsigned char spr_count, tmp_spr_count;

extern short tx,ty;

extern CHR_PARA3 *pchr_data;
extern CHR_PARA3 chr_data[MAX_SPRITE * 2];

extern unsigned char keycode;
