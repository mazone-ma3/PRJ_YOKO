#include "msxcom.h"

unsigned char *pdata;
unsigned char *padr;

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

unsigned char old_jiffy;

inline void msx_wait_vsync(void)
{
	while(((old_jiffy - *jiffy + 256) % 256) < 1);
	old_jiffy = *jiffy;
}

unsigned char spr_x[32];
unsigned char spr_y[32];
unsigned char spr_no[32];
unsigned char spr_color[32];

inline void VDP_put_sprite_16(unsigned char spr_count, unsigned char x, unsigned char y, unsigned char no, unsigned char color)
{
//	y-=1;
	spr_x[spr_count] = x;
	spr_y[spr_count] = y-1;
	spr_no[spr_count] = no * 4;
	spr_color[spr_count] = color;
}

unsigned char spr_page = 0;

void set_sprite_all(unsigned char start, unsigned char end)
{
	unsigned char spr_count;
	unsigned short spr_base;
	if(spr_page)
		spr_base = 0x1b00;
	else
		spr_base = 0x1b80;

//	DI();
	for(spr_count = 0; start < end; ++spr_count, ++start){
		VPOKE(spr_base + 0 + spr_count * 4, spr_y[start]);
		VPOKE(spr_base + 1 + spr_count * 4, spr_x[start]);
		VPOKE(spr_base + 2 + spr_count * 4, spr_no[start]);
		VPOKE(spr_base + 3 + spr_count * 4, spr_color[start]);
	}
	if(spr_count < 32)
		VPOKE(spr_base + 0 + spr_count * 4, 208);

	msx_wait_vsync();
	if(spr_page)
		set_vdp(5, 0x36);
	else
		set_vdp(5, 0x37);

	spr_page = 1 - spr_page;
//	EI();
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

unsigned char get_key(unsigned char matrix) __sdcccall(1)
{
	outp(0xaa, ((inp(0xaa) & 0xf0) | matrix));
	return inp(0xa9);
/*
__asm
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x0141	; SNSMAT(MAINROM)

	ld	 hl, #2
	add	hl, sp
	ld	a, (hl)	; a = mode

	call	#0x001c	; CALSLT

	ld	l,a
	ld	h,#0
__endasm;
*/
}

unsigned char get_stick1(unsigned char trigno) __sdcccall(1)
{
	(void)trigno;
__asm
;	ld	 hl, #2
;	add	hl, sp
	ld	l,a

	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x00d5	; GTSTCK(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT
;	ld	l,a
;	ld	h,#0

	pop	ix
	ret
__endasm;
	return 0;
}


unsigned char get_trigger1(unsigned char trigno) __sdcccall(1)
{
	(void)trigno;
__asm
;	ld	 hl, #2
;	add	hl, sp
	ld	l,a

	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x00d8	; GTTRIG(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT
;	ld	l,a
;	ld	h,#0

	pop	ix
	ret
__endasm;
	return 0;
}

unsigned char st0, st1, pd0, pd1, pd2, k3, k5, k7, k9, k10;
unsigned char keycode = 0;

unsigned char keyscan(void)
{
	DI();
	keycode = 0;

	k3 = get_key(3);

	k9 = get_key(9);
	k10 = get_key(10);
	k5 = get_key(5);

	st0 = get_stick1(0);
	st1 = get_stick1(1);

	pd0 = get_trigger1(0);
	pd1 = get_trigger1(1);
	pd2 = get_trigger1(3);

	EI();

	if((pd0) || (pd1) || !(k5 & 0x20)) /* X,SPACE */
		keycode |= KEY_A;
	if((pd2) || !(k3 & 0x01)) /* C */
		keycode |= KEY_B;
	if((st0 >= 1 && st0 <=2) || (st0 == 8) || (st1 >= 1 && st1 <=2) || (st1 ==8) || !(k10 & 0x08)) /* 8 */
		keycode |= KEY_UP1;
	if((st0 >= 4 && st0 <=6) || (st1 >= 4 && st1 <=6) || !(k9 & 0x20)) /* 2 */
		keycode |= KEY_DOWN1;

//	if(!(st & 0x0c)){ /* RL */
//		keycode |= KEY_START;
//	}else{
	if((st0 >= 6 && st0 <=8) || (st1 >= 6 && st1 <=8) || !(k9 & 0x80)) /* 4 */
		keycode |= KEY_LEFT1;
	if((st0 >= 2 && st0 <=4) || (st1 >= 2 && st1 <=4) || !(k10 & 0x02)) /* 6 */
		keycode |= KEY_RIGHT1;
//	}

	return keycode;
}
