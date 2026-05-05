#include "msxcom.h"
//#include <time.h>
#include <stdio.h>
#include <stdlib.h>

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
//	while(((old_jiffy - *jiffy + 256) % 256) < 1);
//	old_jiffy = *jiffy;
	while(old_jiffy == *jiffy);
	old_jiffy = *jiffy;
}

/*unsigned char spr_x[32];
unsigned char spr_y[32];
unsigned char spr_no[32];
unsigned char spr_color[32];
*/
typedef struct {
	unsigned char y, x, no, color;
}SPR;

SPR spr_chr[32];


inline void VDP_put_sprite_16(unsigned char spr_count, unsigned char x, unsigned char y, unsigned char no, unsigned char color)
{
//	y-=1;
/*	spr_x[spr_count] = x;
	spr_y[spr_count] = y-1;
	spr_no[spr_count] = no * 4;
	spr_color[spr_count] = color;
*/
	spr_chr[spr_count].x = x;
	spr_chr[spr_count].y = y-1;
	spr_chr[spr_count].no = no * 4;
	spr_chr[spr_count].color = color;
}



unsigned char VDP_writeadr;

void write_vram_adr(unsigned char highadr, int lowadr) __sdcccall(1)
{
__asm
	push	de
__endasm;
//	MSX1(TMS9918)では不要
//	write_VDP(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
__asm
	pop	de
	ld	a,(_VDP_writeadr)
	inc	a
	ld	c,a
	out	(c),e
	ld	a,d
	and	a,0x3f
	set	6,a
	out	(c),a
__endasm;
//	outp(VDP_writeport(VDP_WRITECONTROL), (lowadr & 0xff));
//	outp(VDP_writeport(VDP_WRITECONTROL), 0x40 | ((lowadr >> 8) & 0x3f));
}

void write_vram_data(unsigned char data) __sdcccall(1)
{
__asm
//	outp(VDP_writeport(VDP_WRITEDATA), data);
	ld	b,a
	ld	a,(_VDP_writeadr)
	ld	c,a
	out	(c),b
__endasm;
}

void VPOKE(unsigned char data, unsigned short lowadr) __sdcccall(1)
{
//	return;
	DI();
	write_vram_adr(0, lowadr);
	write_vram_data(data);
	EI();
/*	DI();
__asm
	push	af
;	ld	a,d
	call	0x004d
	pop	af
__endasm;*/
	EI();
}

unsigned char spr_page = 0;
unsigned char spr_count2 = 0;
//unsigned short spr_base;
unsigned char end2;

void set_sprite_all(unsigned char start, unsigned char end) __sdcccall(1)
{
/*	if(spr_page)
		spr_base = 0x1b00;
	else
		spr_base = 0x1b80;
*/
//	DI();
/*	for(spr_count = 0; start < end; ++spr_count, ++start){
		VPOKE(spr_base + 0 + spr_count * 4, spr_y[start]);
		VPOKE(spr_base + 1 + spr_count * 4, spr_x[start]);
		VPOKE(spr_base + 2 + spr_count * 4, spr_no[start]);
		VPOKE(spr_base + 3 + spr_count * 4, spr_color[start]);
	}*/
__asm

	ld	c,a
	ld	b,0

	ld	a,l
	ld	(_end2),a

	ld	a,(_spr_page)
	or	a
	jr	z,sprskip1
	ld	de,0x1b00	;(_sprbase)
	jr	sprskip2
sprskip1:
	ld	de,0x1b80
sprskip2:

	xor	a
	ld	(_spr_count2),a

	ld	hl,_spr_chr
	add	hl,bc
	add	hl,bc
	add	hl,bc
	add	hl,bc

	ld	b,c

	di
	xor	a
	call	_write_vram_adr
	ei

	ld	a,(_VDP_writeadr)
	ld	c,a

sprloop:

	ld	a,(hl)
	out	(c),a
	inc	hl

	ld	a,(hl)
	out	(c),a
	inc	hl

	ld	a,(hl)
	out	(c),a
	inc	hl

	ld	a,(hl)
	out	(c),a
	inc	hl

	ld	a,(_spr_count2)
	inc	a
	ld	(_spr_count2),a


	inc	b
	ld	a,b
	ex	hl,de
	ld	hl,_end2
	cp	(hl)
	ex	hl,de

	jr	nz,sprloop

	ld	a,(_spr_count2)
	cp	32
	jr	nc,sprend

	ld	a,208
	out	(c),a

sprend:
__endasm;


/*	if(spr_count < 32)
		VPOKE(208, spr_base + 0 + spr_count * 4);
*/
	msx_wait_vsync();
//	msx_wait_vsync();
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
		VPOKE(chr, vramadr++);
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

unsigned char randdata;

void msx_cls(void) {
    // msxcom.h で実装してもらう想定
    // 例: VDPをクリアするか、SCREEN 1ならテキスト領域をスペースで埋める

	unsigned char j; //i,j,k,l;
	int a;
	unsigned short adr;
/*
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
__endasm;*/
/*
	for(j = 0; j < 32; ++j)
		for(i = 0; i < 8; ++i)
			VPOKE(0, i+j*8);
*/
	a = rand();
	srand(1);
	adr = 0x1800 + 2 * 32;

	DI();
	write_vram_adr(0, adr);
	EI();

//	for(j = 2; j < 192/8; ++j){
	j = 192/8-2;
	while(j--){
		randdata = rand() % 32;
//		l = k+32;
//		for(i = k; i < l; ++i){
//			write_vram_data(data);
//			VPOKE(i % 32, adr++);
__asm
	push	bc
	push	af
	ld	b,32
	ld	a,(_VDP_writeadr)
	ld	c,a
	ld	a,(_randdata)
clsloop:
	out	(c),a
	inc	a
	cp	32
	jr	nz,clsloop2
	xor	a
clsloop2:
	djnz	clsloop
	pop	af
	pop	bc
__endasm;
//		}
	}
	srand(a);
}


 // 適当に音を鳴らす（後で調整）
void msx_sound(unsigned char no, unsigned char dummy) __sdcccall(1)
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

volatile FILE *stream[2];

/* MSX BLOADデータをファイルからメモリに読み込む */
short msxload(char *loadfil, unsigned short offset)
{
	unsigned short size;
	unsigned char *address;
	unsigned char buffer[2];

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
//		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}

	fread( buffer, 1, 1, stream[0]);
	fread( buffer, 1, 2, stream[0]);
	address = (unsigned short *)(buffer[0] + buffer[1] * 256);
	fread( buffer, 1, 2, stream[0]);
	size = (buffer[0] + buffer[1] * 256) - (unsigned short)address;
	fread( buffer, 1, 2, stream[0]);
	address -= offset;
	printf("Load file %s. Address %x Size %x End %x\n", loadfil, address, size, (unsigned short)address + size);

	fread( address , 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}
unsigned char bgmmode = 0;

char checkbgm(void) __sdcccall(1)
{
__asm
	ld	hl,_checkbgm
	ld	a,0x80
	cp	h
	jr	c,bgmerr1

	ld	hl,#0xbc00
	ld	a,(hl)
	cp	#0xc3
	jr	nz,bgmon
	inc	hl
	ld	a,(hl)
	cp	#0x0d
	jr	nz,bgmon
	inc	hl
	ld	a,(hl)
	cp	#0xbc
	jr	nz,bgmon
	ld	a,1
	ret

bgmerr1:
	ld	hl,#0xdc00
	ld	a,(hl)
	cp	#0x2a
	jr	nz,bgmerr2
	inc	hl
	ld	a,(hl)
	cp	#0xf8
	jr	nz,bgmerr2
	inc	hl
	ld	a,(hl)
	cp	#0xf7
	jr	nz,bgmerr2
	ld	a,2
	ret
bgmerr2:
	xor	a
	ret

bgmon:
__endasm;
	if(msxload("psgtone.dat", 0x2000*1) == ERROR)
		return 0;

	if(msxload("C_6_2.PDT", 0x2000*1) == ERROR)
		return 0;

	if(msxload("PSGMSXD.MSX", 0) == ERROR)
		return 0;

	return 1;

__asm
__endasm;
	return 0;
}

void play_bgm(unsigned char mode) __sdcccall(1)
{
__asm
	ld	(#0xf7f8),a
	call	#0xdc00
__endasm;
}

void playbgm(void) __sdcccall(1)
{
//	unsigned char a;
//	a = checkbgm();
	if(!bgmmode)
		return;
	if(bgmmode == 1){
__asm
	call #0xbc00
__endasm;
	}else if(bgmmode == 2){
		play_bgm(0);
	}
}

void stopbgm(void) __sdcccall(1)
{
//	unsigned char a;
//	a = checkbgm();
	if(!bgmmode)
		return;
	if(bgmmode == 1){
__asm
	call #0xbc03
__endasm;
	}else if(bgmmode == 2){
		play_bgm(-1);
	}
}
