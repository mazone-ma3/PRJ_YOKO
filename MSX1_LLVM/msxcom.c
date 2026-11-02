#include "msxcom.h"

#ifndef DEBUG2
#include "PSGMSXD.H"
#endif

//#include <time.h>
//#include <stdio.h>
//#include <stdlib.h>

// 8ビットポートに出力する例
static inline void outp(unsigned char port, unsigned char val) {
    __asm__ volatile (
        "ld c, %0\n\t"
        "ld a, %1\n\t"
        "out (c), a"
        : // 出力オペランドなし
        : "r" (port), "r" (val) // 入力オペランド (%0: port, %1: val)
        : "c", "a", "cc"        // 破壊されるレジスタ・フラグ
    );
}


// 8ビットポートから値を読み込む関数
static inline unsigned char inp(unsigned char port) {
    unsigned char val;
    __asm__ volatile (
        "ld c, %1\n\t"       // Cレジスタにポート番号をセット
        "in a, (c)\n\t"      // ポートからAレジスタに読み込み
        "ld %0, a"           // 読み込んだ値をC側の変数（val）に渡す
        : "=r" (val)         // 出力オペランド (%0)
        : "r" (port)         // 入力オペランド (%1)
        : "c", "a", "cc"     // 破壊されるレジスタ・フラグ
    );
    return val;
}

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
asm volatile(
	"ld	hl,(_pdata)\n"
	"ld	de,(_padr)\n"
	"ld	bc,32\n"
	"call	0x005c\n"
);
}

unsigned char old_jiffy;
#ifdef DEBUG_FPS
unsigned char old_jiffy2=0;
unsigned char total_count=0;
#endif
__attribute__((optnone)) void msx_wait_vsync(void)
{
//	while(((old_jiffy - *jiffy + 256) % 256) < 1);
//	old_jiffy = *jiffy;
	while(old_jiffy == *jiffy);
	old_jiffy = *jiffy;

#ifdef DEBUG_FPS
	++total_count;
	if((unsigned char)(old_jiffy - old_jiffy2) >= 60){
//	if(*jiffy >= 60){
		old_jiffy2 = old_jiffy;
		EI();
		msx_print_num(27, 22, (total_count), 2);
		total_count = 0;
//		*jiffy = 0;
	}
	EI();
#endif
}


enum {
	VDP_READDATA = 0,
	VDP_READSTATUS = 1
};

enum {
	VDP_WRITEDATA = 0,
	VDP_WRITECONTROL = 1,
	VDP_WRITEPAL = 2,
	VDP_WRITEINDEX = 3
};

unsigned char VDP_writeadr;
unsigned char VDP_readadr;
unsigned char vdp_readport[4], vdp_writeport[4];

void write_vdp(unsigned char regno, unsigned char data)
{
	outp(vdp_writeport[VDP_WRITECONTROL], data);
	outp(vdp_writeport[VDP_WRITECONTROL], 0x80 | regno);
}

void vdp_set_sprite_mode(unsigned char mode)
{
	write_vdp(1, 0x60 | mode);
}

/*unsigned char spr_x[32];
unsigned char spr_y[32];
unsigned char spr_no[32];
unsigned char spr_color[32];
*/

SPR spr_chr[32];


void msx_set_sprite(unsigned char spr_count, unsigned char x, unsigned char y, unsigned char no, unsigned char color)
{
	if(spr_count < 32){
//	y-=1;
/*		spr_x[spr_count] = x;
		spr_y[spr_count] = y-1;
		spr_no[spr_count] = no * 4;
		spr_color[spr_count] = color;
*/
		spr_chr[spr_count].x = x;
		spr_chr[spr_count].y = y-1;
		spr_chr[spr_count].no = no * 4;
		spr_chr[spr_count].color = color;
	}
}



void write_vram_adr(unsigned char highadr, int lowadr)
{
/*__asm__ volatile(
	"push	de\n"
);*/
	register int rde asm("de") = lowadr;

//	MSX1(TMS9918)では不要
//	write_VDP(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
__asm__ volatile(
//	"pop	de\n"
	"ld	a,(_VDP_writeadr)\n"
	"inc	a\n"
	"ld	c,a\n"
	"out	(c),e\n"
	"ld	a,d\n"
	"and	a,0x3f\n"
	"set	6,a\n"
	"out	(c),a\n"

	:	/* 値が返るレジスタ変数 */
	:"r"(rde)	/* 引数として使われる変数 */
	: 	/* 破壊されるレジスタ */
);
//	outp(VDP_writeport(VDP_WRITECONTROL), (lowadr & 0xff));
//	outp(VDP_writeport(VDP_WRITECONTROL), 0x40 | ((lowadr >> 8) & 0x3f));
}
/*
void write_vram_data(unsigned char data)
{
__asm
//	outp(VDP_writeport(VDP_WRITEDATA), data);
	ld	b,a
	ld	a,(_VDP_writeadr)
	ld	c,a
	out	(c),b
__endasm;
}
*/


void VPOKE(unsigned char data, unsigned short lowadr)
{
/*	DI();
	write_vram_adr(0, lowadr);
	write_vram_data(data);
	EI();*/

	register unsigned short rhl asm("hl") = lowadr;
	register unsigned char ra asm("a") = data;

	asm volatile(
		"call	0x004d\n"

		:	/* 値が返るレジスタ変数 */
		:"r"(rhl),"r"(ra)	/* 引数として使われる変数 */
		: 	/* 破壊されるレジスタ */
	);
//	EI();
}

unsigned char VPEEK(unsigned short lowadr)
{
/*	DI();
	read_vram_adr(0, lowadr);
	return read_vram_data();*/
//	EI();

	register unsigned short rhl asm("hl") = lowadr;
	register unsigned char ra asm("a");

	asm volatile(
		"call	0x004a\n"

		:"=r"(ra)	/* 値が返るレジスタ変数 */
		:"r"(rhl)	/* 引数として使われる変数 */
		: 	/* 破壊されるレジスタ */
	);

	return ra;
}

unsigned char spr_page = 0;
unsigned char spr_count2 = 0;
//unsigned short spr_base;
unsigned char end2;

unsigned char spr_flag = 0;

void set_sprite_all(unsigned char start, unsigned char end)
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
__asm__ volatile(
	"push	bc\n"

	"ld	c,a\n"
	"ld	b,0\n"

	"ld	a,l\n"
	"ld	(_end2),a\n"

	"ld	a,(_spr_page)\n"
	"or	a\n"
	"jr	z,sprskip1\n"
	"ld	de,0x1b00	;(_sprbase)\n"
	"jr	sprskip2\n"
"sprskip1:\n"
	"ld	de,0x1b80\n"
"sprskip2:\n"

	"xor	a\n"
	"ld	(_spr_count2),a\n"

	"ld	hl,_spr_chr\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"add	hl,bc\n"

	"ld	b,c\n"

	"di\n"
	"xor	a\n"
	"call	_write_vram_adr\n"
	"ei\n"

	"ld	a,(_VDP_writeadr)\n"
	"ld	c,a\n"

"sprloop:\n"

	"ld	a,(hl)\n"
	"out	(c),a\n"
	"inc	hl\n"

	"ld	a,(hl)\n"
	"out	(c),a\n"
	"inc	hl\n"

	"ld	a,(hl)\n"
	"out	(c),a\n"
	"inc	hl\n"

	"ld	a,(hl)\n"
	"out	(c),a\n"
	"inc	hl\n"

	"ld	a,(_spr_count2)\n"
	"inc	a\n"
	"ld	(_spr_count2),a\n"


	"inc	b\n"
	"ld	a,b\n"
	"ex	de,hl\n"
	"ld	hl,_end2\n"
	"cp	(hl)\n"
	"ex	de,hl\n"

	"jr	nz,sprloop\n"

	"ld	a,(_spr_count2)\n"
	"cp	32\n"
	"jr	nc,sprend\n"

	"ld	a,208\n"
	"out	(c),a\n"

"sprend:\n"

	"pop	bc\n"
);


/*	if(spr_count < 32)
		VPOKE(208, spr_base + 0 + spr_count * 4);
*/
//	msx_wait_vsync();
//	msx_wait_vsync();

/*	if(spr_page)
		set_vdp(5, 0x36);
	else
		set_vdp(5, 0x37);
*/
	DI();
	spr_flag = 1;
	spr_page = 1 - spr_page;
	EI();
	msx_wait_vsync();

//	EI();
}

char *chr;

// 文字列表示（左上を基準にしたタイル座標）
void msx_print(unsigned char x, unsigned char y, char *str)
{
//	char chr;
	unsigned short vramadr = 0x1800 + x + y * 32;

	chr = str;

	DI();
	write_vram_adr(0, vramadr);
//	EI();

/*	while(*chr != '\0'){
		if((*chr < 0x30)) //|| (chr > 0x5f))
			*chr = 0x20;
//		VPOKE(chr, vramadr++);
		DI();
		write_vram_data(*chr);
		EI();
		chr++;
	}*/

__asm__ volatile(
	"ld	a,(_VDP_writeadr)\n"
	"ld	c,a\n"
	"ld	hl,(_chr)\n"
"msx_printloop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	z,msx_printend\n"
	"cp	0x30\n"
	"jr	nc,msx_printskip\n"
	"ld	a,0x20\n"
"msx_printskip:\n"
	"out	(c),a\n"
	"inc	hl\n"
	"jr	msx_printloop\n"
"msx_printend:\n"
);
	EI();
}

char str_temp[9];

// 数字表示（左上を基準にしたタイル座標、桁数指定）
void msx_print_num(unsigned char x, unsigned char y, unsigned short number, unsigned char digits)
{
	unsigned char i = digits;

	while(i--){
		str_temp[i] = number % 10 + 0x30;
		number /= 10;
	}
	str_temp[digits] = '\0';

	msx_print(x, y, str_temp);
}

const unsigned long pow10[7] = {
	1L,			// 1桁目
	10L,
	100L,
	1000L,
	10000L,
	100000L,	// 6桁目
	1000000L,	// 7桁目
};

void msx_print_num_l(unsigned char x, unsigned char y, unsigned long number, unsigned char digits)
{
	unsigned char i = digits;
	unsigned long p;
	unsigned char digit, j = 0;

/*	while(i--){
		str_temp[i] = number % 10 + 0x30;
		number /= 10;
	}*/

//	for (int i = 0; i < 7; i++) {
	while(i--){
		digit = 0;

		p = pow10[i];

		while (number >= p) {	// 引き算ループ
			number -= p;
			digit++;
		}
		str_temp[j++] = digit + 0x30;
	}

	str_temp[digits] = '\0';

	msx_print(x, y, str_temp);
}

unsigned char randdata;
unsigned char seed = 1;

unsigned char simple_rnd(void) {
/*	static unsigned char r = 1;
	r = r * 37 + 41;  // 適当な定数
	return r;*/
	seed = (seed * 5) + 1;
	return seed;  // 0-255
}

void msx_cls(void) {
    // msxcom.h で実装してもらう想定
    // 例: VDPをクリアするか、SCREEN 1ならテキスト領域をスペースで埋める

	unsigned char j; //i,j,k,l;
	int a;
	unsigned short adr;
	unsigned char old_seed;
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
//	a = rand();
//	srand(1);
	old_seed = seed;
	seed=1;
	adr = 0x1800 + 2 * 32;

	DI();
	write_vram_adr(0, adr);
	EI();

//	for(j = 2; j < 192/8; ++j){
	j = 192/8-2;
	while(j--){
		randdata = simple_rnd() % 32;
//		l = k+32;
//		for(i = k; i < l; ++i){
//			write_vram_data(data);
//			VPOKE(i % 32, adr++);
__asm__ volatile(
	"push	bc\n"
	"push	af\n"
	"ld	b,32\n"
	"ld	a,(_VDP_writeadr)\n"
	"ld	c,a\n"
	"ld	a,(_randdata)\n"
"clsloop:\n"
	"out	(c),a\n"
	"inc	a\n"
	"cp	32\n"
	"jr	nz,clsloop2\n"
	"xor	a\n"
"clsloop2:\n"
	"djnz	clsloop\n"
	"pop	af\n"
	"pop	bc\n"
);
//		}
	}
//	srand(a);
	seed = old_seed;
}

void set_psg(unsigned char reg, unsigned char tone)
{
	register unsigned char rh asm("h") = reg;
	register unsigned char rl asm("l") = tone;

__asm__ volatile(
/*;	ld	hl, #2
;	add	hl, sp
	push	ix

	ld	h,a

	ld	a,(#0xfcc1)	; exptbl
	ld	b,a
	ld	c,0
	push	bc
	pop	iy

;	ld	c,(hl)
;	inc	hl
;	ld	b,(hl)	;bc = reg
;	inc	hl
;	ld	e, (hl)
;	inc	hl
;	ld	d, (hl)	; de = tone

;	ld	a,e
;	ld	e,d*/
	"ld	a,%0\n"
	"ld	e,%1\n"

//	ld ix,#0x0093	; WRTPSG(MAINROM)
//	call	#0x001c	; CALSLT
	"call 0x0093"
//	pop	ix

	:
	: "r"(rh), "r"(rl)
	:

);
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

unsigned char get_key(unsigned char matrix)
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

unsigned char get_stick1(unsigned char trigno)
{
	register unsigned char ra asm("a");
	register unsigned char rd asm("d") = trigno;

__asm__ volatile(
	"push	de\n"
	"push	ix\n"

	"ld	a,(0xfcc1)	; exptbl\n"
	"ld	b,a\n"
	"ld	c,0\n"
	"push	bc\n"
	"pop	iy\n"
	"ld ix,0x00d5	; GTSTCK(MAINROM)\n"
	"ld	a,%1\n"

	";call	0x001c	; CALSLT\n"
	"call	0x00d5	; GTSTCK(MAINROM)\n"

	"pop	ix\n"
	"pop	de\n"

	:"=r"(ra)	/* 値が返るレジスタ変数 */
	:"r"(rd)	/* 引数として使われる変数 */
	: "bc", "iy","hl"	/* 破壊されるレジスタ */
);

	return ra;
}
/*
unsigned char get_stick1(unsigned char trigno)
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
*/

unsigned char get_trigger1(unsigned char trigno)
{
	register unsigned char ra asm("a");
	register unsigned char rd asm("d") = trigno;

__asm__ volatile(
	"push	ix\n"

	"ld	a,(0xfcc1)	; exptbl\n"
	"ld	b,a\n"
	"ld	c,0\n"
	"push	bc\n"
	"pop	iy\n"
	"ld ix,0x00d8	; GTTRIG(MAINROM)\n"

	"ld	a,%1\n"
	";call	0x001c	; CALSLT\n"
	"call	0x00d8	; GTTRIG(MAINROM)\n"

	"pop	ix\n"

	:"=r"(ra)	/* 値が返るレジスタ変数 */
	:"r"(rd)	/* 引数として使われる変数 */
	: "bc", "iy"	/* 破壊されるレジスタ */
);

	return ra;
}

/*
unsigned char get_trigger1(unsigned char trigno)
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
}*/

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

unsigned char bgmmode = 0;

#ifdef DEBUG2

char checkbgm(void)
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

void play_bgm(unsigned char mode)
{
__asm
	ld	(#0xf7f8),a
	call	#0xdc00
__endasm;
}

void playbgm(void)
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

void stopbgm(void)
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


#endif

unsigned char INTWORK[5] = {0,0,0,0,0};
unsigned char vdps0;

void intvsync(void)
{
__asm__ volatile(
//;intvsync:
	"ld	(_vdps0),a\n"
	"push	af\n"
//;	push	ix
);

	if(spr_flag == 1){
		spr_flag = 0;
//		set_spr_atr_adr(spr_next); 
//		write_VDP(11, ((spr_next << 1) & 0x02));

//		if(spr_page)
//			set_vdp(5, 0x36);
//		else
//			set_vdp(5, 0x37);


__asm__ volatile(
	"ld	a,(_VDP_writeadr)\n"
	"inc	a\n"
	"ld	c,a\n"
	"ld	a,(_spr_page)\n"
	"or	a\n"
	"jr	nz,intvsyncskip\n"
	"ld	a,0x36\n"
	"jr	intvsyncend\n"
"intvsyncskip:\n"
	"ld	a,0x37\n"
"intvsyncend:\n"
	"out	(c),a\n"
	"ld	a,5 | 0x80\n"
	"out	(c),a\n"
);

/*
	ld	a,(_spr_next)
	add	a,a
	and	a,#0x02
	out	(c),a
	ld	a,11 | 0x80
	out	(c),a
__endasm;*/
	}
__asm__ volatile(
//;	pop	ix
	"pop	af\n"
	"jp	_INTWORK\n"
);
}

void set_int(void)
{
__asm__ volatile(
	"DI\n"
	"LD	IY,-609\n"
	"PUSH	IY\n"
	"POP	HL\n"
	"LD	DE,_INTWORK\n"
	"LD	BC,5\n"
	"LDIR\n"
	"LD	HL,_intvsync\n"
	"LD	(IY+1),L\n"
	"LD	(IY+2),H\n"
	"LD	(IY+0),#0x0C3\n"
	"EI\n"
);
}

__attribute__((optnone)) void reset_int(void)
{
	EI();
	if(INTWORK[0] != 0){
		while(spr_flag != 0);
	}
__asm__ volatile(
	"DI\n"
	"LD	HL,_INTWORK\n"
	"LD	A,(HL)\n"
	"OR	A\n"
	"JR	Z,reset_intend\n"
	"LD	DE,-609\n"
	"LD	BC,5\n"
	"PUSH	HL\n"
	"LDIR\n"
	"POP	HL\n"
	"XOR	A\n"
	"LD	(HL),A\n"
"reset_intend:\n"
	"EI\n"
);
}
