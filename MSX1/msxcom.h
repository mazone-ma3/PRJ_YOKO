#include <video/tms99x8.h>
#include <msx\gfx.h>

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

#define msx_set_sprite_pattern(A,B) VDP_set_sprite_16(A,B)

unsigned char *pdata;
unsigned char *padr;

#define VPOKE vpoke

void VDP_set_sprite_16(unsigned char chr, unsigned char *data)
{
	pdata = (unsigned char *)data;
	padr = (unsigned char *)0x03800 + chr * 8 * 4;
//	padr = (unsigned char *)0x0 + chr * 8;
/*	for(i = 0; i < 32; ++i){
		VPOKE(0x03800L + chr * 8 + i, data[i]);
	}
	return;*/
__asm
	ld	hl,(_pdata)
	ld	de,(_padr)
	ld	bc,32
	call	0x005c
__endasm;
}

inline void VDP_put_sprite_16(unsigned char spr_count, unsigned char x, unsigned char y, unsigned char no, unsigned char color)
{
	y-=1;
	VPOKE(0x1b00 + 0 + spr_count * 4, y);
	VPOKE(0x1b00 + 1 + spr_count * 4, x);
	VPOKE(0x1b00 + 2 + spr_count * 4, no * 4);
	VPOKE(0x1b00 + 3 + spr_count * 4, color);
}

#define msx_set_sprite VDP_put_sprite_16

#define JOY_UP 1
#define JOY_DOWN 2
#define JOY_LEFT 4
#define JOY_RIGHT 8

unsigned char joystick(unsigned char no)
{
	unsigned char keycode = get_stick(0) | get_stick(1), keycode2 = 0;

	if((keycode == 1) || (keycode == 2) || (keycode == 8)){	/* UP */
		keycode2 |= JOY_UP;
	}
	if((keycode == 4) || (keycode == 5) || (keycode == 6)){	/* DOWN */
		keycode2 |= JOY_DOWN;
	}
	if((keycode == 6) || (keycode == 7) || (keycode == 8)){	/* LEFT */
		keycode2 |= JOY_LEFT;
	}
	if((keycode == 2) || (keycode == 3) || (keycode == 4)){	/* RIGHT */
		keycode2 |= JOY_RIGHT;
	}

	return keycode2;
}

#define joyfire(A) get_trigger(1)
#define msx_get_key(A) get_trigger(0)


#define clicksw ((volatile unsigned char *)0xf3db)

#define msx_set_color vdp_color
#define msx_screen(A) {\
	*clicksw = 0;\
	vdp_set_mode(A);\
	vdp_set_sprite_mode(sprite_large);\
}

#define jiffy ((volatile unsigned char *)0xfc9e)
unsigned char old_jiffy;

inline void msx_wait_vsync(void)
{
	while(*jiffy == old_jiffy);
	old_jiffy = *jiffy;
}

// 文字列表示（左上を基準にしたタイル座標）
void msx_print(unsigned char x, unsigned char y, char *str)
{
	char chr;
	unsigned short vramadr = 0x1800 + x + y * 32;

	while((chr = *(str++)) != '\0'){
		if((chr < 0x30)) //|| (chr > 0x5f))
			chr = 0x20;
		VPOKE(vramadr++, chr);
	}
}

char str_temp[9];

// 数字表示（左上を基準にしたタイル座標、桁数指定）
void msx_print_num(unsigned char x, unsigned char y, int number, unsigned char digits)
{
	unsigned char i = digits;

	while(i--){
		str_temp[i] = number % 10 + 0x30;
		number /= 10;
	}
	str_temp[digits] = '\0';

	msx_print(x, y, str_temp);
}

void msx_cls(void) {
    // msxcom.h で実装してもらう想定
    // 例: VDPをクリアするか、SCREEN 1ならテキスト領域をスペースで埋める
__asm
	push	hl
	push	iy
	push	ix

	push	af
	push	bc
	push	de

	xor	a
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x00c3	; CLS(MAINROM)

	call	#0x001c	; CALSLT

	pop	de
	pop	bc
	pop	af

	pop	ix
	pop	iy
	pop	hl
__endasm;
}

#define msx_initpsg()

 // 適当に音を鳴らす（後で調整）


void msx_sound(unsigned char no, unsigned char dummy)
{
	DI();
	if(no == 1){
		set_psg(6,127);
		set_psg(11,0);
		set_psg(12,15);
		set_psg(7,0x9c);  // 10011100
		set_psg(13,9);
		set_psg(10,0x10);
	}else if(no == 0){
			set_psg(4, 55);
			set_psg(5, 0);
			set_psg(6,127);
			set_psg(11,0);
			set_psg(12,15);
			set_psg(7,0xb8);  // 10111000
			set_psg(13,9);
			set_psg(10,0x10);

	}
	EI();
}
