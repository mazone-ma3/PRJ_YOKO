#ifndef MSX_COM_H_INCLUDE
#define MSX_COM_H_INCLUDE

#include <video/tms99x8.h>
#include <msx\gfx.h>

#include "inkey.h"

//#define __sdcccall(a)

#define DI() {\
__asm\
	di\
__endasm;\
}

#define EI() {\
__asm\
	ei\
__endasm;\
}

extern unsigned char VDP_writeadr;

#define msx_set_sprite_pattern(A,B) VDP_set_sprite_16(A,B)

extern unsigned char *pdata;
extern unsigned char *padr;

//#define VPOKE vpoke
void VPOKE(unsigned char data, unsigned short lowadr) __sdcccall(1);

void VDP_set_sprite_16(unsigned char chr, unsigned char *data);

#define jiffy ((volatile unsigned char *)0xfc9e)
extern unsigned char old_jiffy;

inline void msx_wait_vsync(void);

extern unsigned char spr_x[32];
extern unsigned char spr_y[32];
extern unsigned char spr_no[32];
extern unsigned char spr_color[32];

inline void VDP_put_sprite_16(unsigned char spr_count, unsigned char x, unsigned char y, unsigned char no, unsigned char color);

extern unsigned char spr_page;

void set_sprite_all(unsigned char start, unsigned char end) __sdcccall(1);

#define msx_set_sprite VDP_put_sprite_16

#define clicksw ((volatile unsigned char *)0xf3db)

#define msx_set_color vdp_color
#define msx_screen(A) {\
	*clicksw = 0;\
	vdp_set_mode(A);\
	vdp_set_sprite_mode(sprite_large);\
}

void msx_print(unsigned char x, unsigned char y, char *str);

extern char str_temp[9];

void msx_print_num(unsigned char x, unsigned char y, int number, unsigned char digits);

void msx_cls(void);

#define msx_initpsg()

void msx_sound(unsigned char no, unsigned char dummy) __sdcccall(1);

unsigned char get_key(unsigned char matrix) __sdcccall(1);

unsigned char get_stick1(unsigned char trigno) __sdcccall(1);

unsigned char get_trigger1(unsigned char trigno) __sdcccall(1);

extern unsigned char keycode;

unsigned char keyscan(void);


#endif
