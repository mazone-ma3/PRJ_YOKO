//#include <msx.h>
#include "msxcom.h"
//#include <math.h>

//#include <stdlib.h>
//#include <stdio.h>


unsigned char randint(unsigned char min, unsigned char max)
{
	return (simple_rnd() % (max-min) + min);
}

int abs(int num)
{
	if(num < 0)
		return -num;
	return num;
}

long absl(long num)
{
	if(num < 0)
		return -num;
	return num;
}

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

// ゲーム定数
#define True 1
#define False 0

#define DIV 256
#define COUNT1S 60

#define PLAYER_SPEED 2
#define BULLET_SPEED 6
#define ENEMY_SPEED 2

#define MAX_BULLETS 8
#define MAX_ENEMIES 8
#define MAX_e_bullets 6

#define MAX_Particle 6
#define MAX_Option 2
#define MAX_ChainItem 4
#define MAX_OptionItem 1
#define MAX_ShieldItem 2
#define MAX_BombItem 2

#define width 256
#define height 192

#define width_bullet width-BULLETSPEED

// スプライト番号
/*#define SPR_PLAYER   0
#define SPR_BULLETS  1
#define SPR_ENEMIES  7
#define SPR_e_bullets 15*/


// グローバル変数
unsigned char player_x = 40 ;
unsigned char player_y = 80 ;

//struct Bullet  { unsigned short x, y, active; };
/*struct Enemy {
	unsigned short x, y;
	unsigned char active;
	unsigned char hp;
	unsigned char shoot_timer;
};*/
struct EBullet { unsigned short x, y; signed short dx, dy; unsigned char active; };

//struct Bullet 

unsigned char bullets_x[MAX_BULLETS];
unsigned char bullets_y[MAX_BULLETS];
signed short bullets_dx[MAX_BULLETS];
signed short bullets_dy[MAX_BULLETS];
unsigned char bullets_active[MAX_BULLETS];

//struct Enemy  
unsigned char enemies_x[MAX_ENEMIES];
unsigned char enemies_y[MAX_ENEMIES];
unsigned char enemies_active[MAX_ENEMIES];
unsigned char enemies_hp[MAX_ENEMIES];
unsigned char enemies_shoot_timer[MAX_ENEMIES];
unsigned char enemies_type[MAX_ENEMIES];
unsigned char enemies_count[MAX_ENEMIES];
unsigned char enemies_count2[MAX_ENEMIES];
unsigned char enemies_count_flag[MAX_ENEMIES];

//struct EBullet 
unsigned short e_bullets_x[MAX_e_bullets];
unsigned short e_bullets_y[MAX_e_bullets];
signed short e_bullets_dx[MAX_e_bullets];
signed short e_bullets_dy[MAX_e_bullets];
unsigned char e_bullets_active[MAX_e_bullets];

unsigned char Particle_x[MAX_Particle];
unsigned char Particle_y[MAX_Particle];
signed char Particle_vx[MAX_Particle];
signed char Particle_vy[MAX_Particle];
unsigned char  Particle_life[MAX_Particle];
//short Particle_color[MAX_Particle];
unsigned char Particle_active[MAX_Particle];

short Option_offset_y[MAX_Option];
short Option_x[MAX_Option];
short Option_y[MAX_Option];
unsigned char Option_active[MAX_Option];

unsigned char ChainItem_x[MAX_ChainItem];
unsigned char ChainItem_y[MAX_ChainItem];
short ChainItem_timer[MAX_ChainItem];
unsigned char ChainItem_active[MAX_ChainItem];

unsigned char OptionItem_x[MAX_OptionItem];
unsigned char OptionItem_y[MAX_OptionItem];
short OptionItem_timer[MAX_OptionItem];
unsigned char OptionItem_active[MAX_OptionItem];

unsigned char ShieldItem_x[MAX_ShieldItem];
unsigned char ShieldItem_y[MAX_ShieldItem];
short ShieldItem_timer[MAX_ShieldItem];
unsigned char ShieldItem_active[MAX_ShieldItem];

unsigned char BombItem_x[MAX_BombItem];
unsigned char BombItem_y[MAX_BombItem];
short BombItem_timer[MAX_BombItem];
unsigned char BombItem_active[MAX_BombItem];

//short sin_table[256 * 4];
extern const short sin_table[256 * 4];

unsigned char b_idx, eb_idx, opt_idx, item_idx, e_idx, p_idx;

unsigned char bomb_stock = 0;
unsigned char shield_active = False;

unsigned char chain_count = 0;
unsigned char chain_timer = 0;
unsigned char option_cooldown = 10;
//unsigned long enemy_spawn_timer = 0;
unsigned short kill_count = 0;
//unsigned short shoot_timer = 0;

unsigned long play_time = 0;		  // 経過時間（フレーム）
//unsigned char game_over = 0; //False;

//unsigned short old_i;
unsigned char k;
unsigned char enemy_type;
unsigned char rand_num;
short dist_x, dist_y;
short base_y;
unsigned char hp;
short difficulty;
//short sx,sy,dx,dy;
unsigned char ex,ey;
short ph_x,ph_y,ph_w,ph_h;

short enemy_bullet_speed;

short shoot_interval;
//short dist;
short speed;
short base_speed;
short direction_factor;
short offset;
//long i;
unsigned char i;
short idx;


unsigned char key_b_flag;

unsigned long score = 0;
unsigned long high_score = 0;
//unsigned int play_time = 0;
unsigned char shoot_timer = 0;
unsigned long enemy_spawn_timer = 0;
unsigned char game_over = 1;

unsigned char all_display_flag = True;
unsigned char score_display_flag = True;
//unsigned char high_score_display_flag = True;
unsigned char time_display_flag = True;
unsigned char bomb_display_flag = True;
unsigned char chain_display_flag = True;

//struct Star { unsigned char x, y, speed; };
//struct Star stars[40];


unsigned char bullets_mode = 0;

const unsigned char fontdata[] = {
	0x78,0x84,0x84,0x84,0x84,0x84,0x78,0x00,
	0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,
	0x78,0x04,0x04,0x78,0x80,0x80,0xfc,0x00,
	0x78,0x04,0x04,0x78,0x04,0x04,0xf8,0x00,
	0x84,0x84,0x84,0x7c,0x04,0x04,0x04,0x00,
	0xfc,0x80,0x80,0xf8,0x04,0x04,0xf8,0x00,
	0x78,0x80,0x80,0xf8,0x84,0x84,0x78,0x00,
	0xfc,0x04,0x04,0x04,0x04,0x04,0x04,0x00,
	0x78,0x84,0x84,0x78,0x84,0x84,0x78,0x00,
	0x78,0x84,0x84,0x7c,0x04,0x04,0x04,0x00,

	//0x00,0xe0,0xf8,0xfe,0xf8,0xe0,0x00,0x00,

	0x7c,0x82,0x82,0xfe,0x82,0x82,0x82,0x00,
	0xfc,0x82,0x82,0xfc,0x82,0x82,0xfe,0x00,
	0x7c,0x82,0x80,0x80,0x80,0x82,0x7c,0x00,
	0xfc,0x84,0x84,0x82,0x82,0x82,0xfe,0x00,
	0xfe,0x80,0x80,0xfc,0x80,0x80,0xfe,0x00,
	0xfe,0x80,0x80,0xfc,0x80,0x80,0x80,0x00,
	0x7c,0x82,0x80,0x9e,0x82,0x82,0x7e,0x00,
	0x82,0x82,0x82,0xfe,0x82,0x82,0x82,0x00,
	0x38,0x10,0x10,0x10,0x10,0x10,0x38,0x00,
	0x0e,0x02,0x02,0x82,0x82,0x44,0x3c,0x00,
	0x82,0x8c,0xb0,0xc0,0xb0,0x8c,0x82,0x00,
	0x80,0x80,0x80,0x80,0x80,0x82,0xfe,0x00,
	0xc6,0xaa,0xaa,0x92,0x82,0x82,0x82,0x00,
	0xc2,0xa2,0xa2,0x92,0x8a,0x8a,0x86,0x00,
	0x7c,0x82,0x82,0x82,0x82,0x82,0x7c,0x00,
	0xfc,0x82,0x82,0xfc,0x80,0x80,0x80,0x00,
	0x38,0x44,0x82,0x82,0x8a,0x44,0x3a,0x00,
	0xfc,0x82,0x82,0xfc,0x82,0x82,0x82,0x00,
	0x7c,0x82,0x80,0x7c,0x02,0x82,0x7c,0x00,
	0xfe,0x10,0x10,0x10,0x10,0x10,0x10,0x00,
	0x82,0x82,0x82,0x82,0x82,0x82,0x7c,0x00,
	0x82,0x82,0x82,0x44,0x44,0x28,0x10,0x00,
	0x82,0x82,0x92,0x92,0xaa,0x44,0x44,0x00,
	0x82,0x44,0x28,0x10,0x28,0x44,0x82,0x00,
	0x82,0x82,0x44,0x28,0x10,0x10,0x10,0x00,
	0xfe,0x02,0x0c,0x10,0x60,0x80,0xfe,0x00,
	0xfe,0x80,0x80,0x80,0x80,0x80,0x80,0x00,
	0x7c,0x82,0xba,0xa2,0xba,0x82,0x7c,0x00,
	0xec,0x92,0x92,0x92,0x92,0x92,0x92,0x00,
	0x7a,0x86,0x82,0x82,0x82,0x86,0x7a,0x00,
	0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,
};

void define_fonts(void) {
	unsigned char i,j;
	unsigned short k = 0, l;
	l = '0'*8;
	for(j = 0; j < ('9'-'0'+1); ++j){
		for(i = 0; i < 8; ++i){
			VPOKE(fontdata[k++], l++);
		}
	}
	l = 'A'*8;
	for(j = 0; j < ('Z'-'A'+1); ++j){
		for(i = 0; i < 8; ++i){
			VPOKE(fontdata[k++], l++);
		}
	}
}

const unsigned char player_pat[] = {
	0x00,0x00,0xe0,0xf0,0xfc,0xfe,0xff,0xff,
	0xff,0xff,0xff,0xe0,0xe0,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0xff,
	0xff,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,
};
const unsigned char player2_pat[] = {
	0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xc0,0xe0,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
const unsigned char playershield_pat[] = {
	0x07,0x18,0x20,0x40,0x41,0x81,0x80,0x80,
	0x80,0x80,0x80,0x40,0x40,0x20,0x18,0x07,
	0xe0,0x18,0x04,0x02,0xc2,0xe1,0x01,0x01,
	0x01,0x01,0x01,0x02,0x02,0x04,0x18,0xe0,
};
const unsigned char bullet_pat[] = {
	0xbc,0xbf,0xbf,0xbc,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
const unsigned char option_pat[] = {
	0x00,0x00,0x00,0x00,0x07,0x07,0x07,0x07,
	0x07,0x07,0x07,0x07,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0xc0,0xf8,0xff,
	0xff,0xf8,0xc0,0x00,0x00,0x00,0x00,0x00,
};
const unsigned char enemy_pat[] = {
	0xff,0xff,0xff,0xff,0xf0,0xf0,0xf0,0xf0,
	0xf0,0xf0,0xf0,0xf0,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x0f,0x0f,0x0f,0x0f,
	0x0f,0x0f,0x0f,0x0f,0xff,0xff,0xff,0xff,
};
const unsigned char ebullet_pat[] = {
//	0x60,0xf0,0xf0,0x60,0x00,0x00,0x00,0x00,
//	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x78,0xfc,0xfc,0xfc,0xfc,0x78,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
const unsigned char chain_pat[] = {
	0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff,
	0xff,0x7f,0x3f,0x1f,0x0f,0x07,0x03,0x01,
	0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff,
	0xff,0xfe,0xfc,0xf8,0xf0,0xe0,0xc0,0x80
};
const unsigned char optionitem_pat[] = {
//	0x01,0x03,0x07,0x0f,0x18,0x3b,0x7b,0xfb,
//	0xfb,0x7b,0x38,0x1f,0x0f,0x07,0x03,0x01,
//	0x80,0xc0,0xe0,0xf0,0x18,0xdc,0xde,0xdf,
//	0xdf,0xde,0x1c,0xf8,0xf0,0xe0,0xc0,0x80,
	0x03,0x07,0x0f,0x1f,0x38,0x7b,0xfb,0xfb,
	0xfb,0xfb,0x78,0x3f,0x1f,0x0f,0x07,0x03,
	0xc0,0xe0,0xf0,0xf8,0x1c,0xde,0xdf,0xdf,
	0xdf,0xdf,0x1e,0xfc,0xf8,0xf0,0xe0,0xc0,
};
const unsigned char shield_pat[] = {
	0x01,0x03,0x07,0x0f,0x18,0x3b,0x7b,0xf8,
	0xff,0x7f,0x38,0x1f,0x0f,0x07,0x03,0x01,
	0x80,0xc0,0xe0,0xf0,0x38,0xfc,0xfe,0x1f,
	0xdf,0xde,0x1c,0xf8,0xf0,0xe0,0xc0,0x80,
};
const unsigned char bomb_pat[] = {
	0x01,0x03,0x07,0x0f,0x18,0x3b,0x7b,0xf8,
	0xfb,0x7b,0x38,0x1f,0x0f,0x07,0x03,0x01,
	0x80,0xc0,0xe0,0xf0,0x38,0xdc,0xde,0x3f,
	0xdf,0xde,0x3c,0xf8,0xf0,0xe0,0xc0,0x80,
};
const unsigned char particle_pat[] = {
	0xc0,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};


// ====================== スプライト定義 ======================
void define_sprites(void) {
	// 自機 (16x16)
	msx_set_sprite_pattern(0, (unsigned char *)player_pat);

	msx_set_sprite_pattern(40, (unsigned char *)player2_pat);

	msx_set_sprite_pattern(44, (unsigned char *)playershield_pat);


	// 自機弾
	msx_set_sprite_pattern(4, (unsigned char *)bullet_pat);   // 呼び出し側で4の倍数扱い

	// 敵
	msx_set_sprite_pattern(8, (unsigned char *)enemy_pat);

	// 敵弾
	msx_set_sprite_pattern(12, (unsigned char *)ebullet_pat);

	// オプション
	msx_set_sprite_pattern(16, (unsigned char *)option_pat);

	// チェインボーナスアイテム
	msx_set_sprite_pattern(20, (unsigned char *)chain_pat);

	// オプションアイテム
	msx_set_sprite_pattern(24, (unsigned char *)optionitem_pat);

	// シールドアイテム
	msx_set_sprite_pattern(28, (unsigned char *)shield_pat);

	// ボムアイテム
	msx_set_sprite_pattern(32, (unsigned char *)bomb_pat);

	// パーティクル
	msx_set_sprite_pattern(36, (unsigned char *)particle_pat);
//	for(i = 0; i < 10; ++i){
//		msx_set_sprite_pattern(16 + i * 4, player_pat);
//	}
}

// ====================== 初期化 ======================
void reset(void) {
	bullets_mode = 0;
	key_b_flag = False;

	player_x = 40 ;
	player_y = 80 ;
	score = 0;
	play_time = 0;
	shoot_timer = 0;
	enemy_spawn_timer = 0;
//	game_over = 0;

	all_display_flag = True;
	chain_display_flag = True;

	for (i = 0; i < MAX_ENEMIES; i++) {
		enemies_active[i] = 0;
		enemies_hp[i] = 1;
		enemies_shoot_timer[i] = 0;
	}

//	for (i = 0; i < MAX_BULLETS; i++) bullets_active[i] = False;

__asm__ volatile(
	"push	bc\n"
	"push	hl\n"
	"ld	b,#%0\n"
	"ld	hl, _bullets_active\n"
	"xor	a	;ld	a,#False\n"
"resetloop1:\n"
	"ld	(hl),a\n"
	"inc	hl\n"
	"djnz	resetloop1\n"

	"pop	hl\n"
	"pop	bc\n"

	:
	: "i"(MAX_BULLETS)
	:

);

	for (i = 0; i < MAX_e_bullets; i++) e_bullets_active[i] = False;


	for(p_idx =  0; p_idx < MAX_Particle; ++p_idx)
		Particle_active[p_idx] = False;

	for(opt_idx = 0; opt_idx<  MAX_Option; ++opt_idx)
		Option_active[opt_idx] = False;

	for(item_idx = 0; item_idx <  MAX_ChainItem; ++item_idx)
		ChainItem_active[item_idx] = False;

	for(item_idx = 0; item_idx <  MAX_OptionItem; ++item_idx)
		OptionItem_active[item_idx] = False;

	for(item_idx = 0; item_idx < MAX_ShieldItem; ++item_idx)
		ShieldItem_active[item_idx] = False;

	for(item_idx = 0; item_idx < MAX_BombItem; ++item_idx)
		BombItem_active[item_idx] = False;

	bomb_stock = 0;
	shield_active = False;

	chain_count = 0;
	chain_timer = 0;
	option_cooldown = 10;

//	for(e_idx = 0; e_idx < MAX_ENEMIES; ++e_idx)
//		enemies_active[e_idx] = 0;

	enemy_spawn_timer = 0;
	kill_count = 0;
	shoot_timer = 0;
	score = 0;
	play_time = 0;		  // 経過時間（フレーム）
//	game_over = 0;

/*	for (i = 0; i < 40; i++) {
		stars[i].x = (rand() % width) ;
		stars[i].y = (rand() % height) ;
		stars[i].speed = 1 + (rand() % 3);
	}
*/
	for(i = 0; i < 32; ++i)
		msx_set_sprite(i, 0, 208+1, 0, 0);

	msx_cls();
}

unsigned char p_idx;

//#define PARTICLE_LAST_OFFSET (MAX_Particle - 1)

__attribute__((noinline)) void Particles_append(unsigned char x, unsigned char y)
{
//	volatile unsigned char *last_particle = &Particle_active[MAX_Particle - 1];

/*	for (p_idx = 0; p_idx < MAX_Particle; ++p_idx){
		if(Particle_active[p_idx] == True)
			continue;
		Particle_x[p_idx] = x;
		Particle_vx[p_idx] = randint(0,9)-4;
		Particle_y[p_idx] = y;
		Particle_vy[p_idx] = randint(0,9)-4;
		Particle_life[p_idx] = 35 + randint(0, 20);
		Particle_active[p_idx] = True;
		break;
	}
*/
__asm__ volatile(
	"push	bc\n"
	"push	hl\n"
	"push	de\n"

	"ex	de,hl\n"
	"ld	d,a\n"

	"ld	c,#%1\n"
	"ld	b,0\n"
	//";ld	hl,_Particle_active+MAX_Particle-1\n"
	"ld	hl,_Particle_active+%0\n"
"ParticleAppendloop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	nz,ParticleAppendskip1\n"

	"ld	hl,_Particle_x-1\n"
	"add	hl,bc\n"
	"ld	a,d\n"
	"ld	(hl),a\n"

	"ld	hl,_Particle_y-1\n"
	"add	hl,bc\n"
	"ld	a,e\n"
	"ld	(hl),a\n"

	"push	bc\n"
	"xor	a\n"
	"ld	l,9\n"
	"call	_randint\n"
	"pop	bc\n"
	"add	a,#-4\n"
	"ld	hl,_Particle_vx-1\n"
	"add	hl,bc\n"
	"ld	(hl),a\n"

	"push	bc\n"
	"xor	a\n"
	"ld	l,9\n"
	"call	_randint\n"
	"pop	bc\n"
	"add	a,#-4\n"
	"ld	hl,_Particle_vy-1\n"
	"add	hl,bc\n"
	"ld	(hl),a\n"

	"push	bc\n"
	"xor	a\n"
	"ld	l,20\n"
	"call	_randint\n"
	"pop	bc\n"
	"ld	hl,_Particle_life-1\n"
	"add	hl,bc\n"
	"add	a,35\n"
	"ld	(hl),a\n"

	"ld	hl,_Particle_active-1\n"
	"add	hl,bc\n"
	"ld	a,#%2\n"
	"ld	(hl),a\n"

	"jr	ParticleAppendEnd\n"

"ParticleAppendskip1:\n"
	"dec	hl\n"
	"dec	c\n"
	"jr	nz,ParticleAppendloop\n"

"ParticleAppendEnd:\n"
	"pop	de\n"
	"pop	hl\n"
	"pop	bc\n"

	: 
	: "i" (MAX_Particle-1), "i" (MAX_Particle), "i"(True)
	: "hl"
);
}

void use_bomb(void)
{
	if(bomb_stock <= 0) return;
	bomb_stock -= 1;
	bomb_display_flag = True;
	for(i = 0; i < MAX_Particle; ++i){
		Particles_append(randint(20, 236), randint(20, 172));
	}
	for(e_idx = 0; e_idx <  MAX_ENEMIES; ++e_idx)
		enemies_active[e_idx] = False;
	for(eb_idx = 0; eb_idx <  MAX_e_bullets; ++eb_idx)
		e_bullets_active[eb_idx] = False;

	score += 200;
	score_display_flag = True;
}

long deltax, deltay;
unsigned short dist;
signed short dx = 0;
signed short dy = 0;
unsigned char j;

void fire_aimed_bullet(unsigned char ex, unsigned char ey)
{
	deltax = (long)player_x  + 8  - (long)ex ;
	deltay = (long)player_y  + 8  - (long)ey ;

	// 距離近似（横・縦方向を少し強調して斜めばかりを減らす）
//	dist = (unsigned short)(abs(deltax) * 1 + abs(deltay));   // 横方向を重視

	if(absl(deltax) >= absl(deltay))
		dist = absl(deltax);
	else
		dist = absl(deltay);

//	if (dist < 12) dist = 12;
	if (dist == 0) dist = 1;

	// 符号を完全に分離して計算
//	if (deltax > 0) {
		dx = (signed short)((deltax *  enemy_bullet_speed * DIV) / dist);
//	} else if (deltax < 0) {
//		dx = -(signed short)(((-deltax) * 5 ) / dist);
//	}

//	if (deltay > 0) {
		dy = (signed short)((deltay *  enemy_bullet_speed * DIV) / dist);
//	} else if (deltay < 0) {
//		dy = -(signed short)(((-deltay) * 5 ) / dist);
//	}

	// 最大速度を4に制限（前回の希望通り）
	if (dx > 4 * DIV) dx = 4 * DIV;
	if (dx < -3 * DIV) dx = -3 * DIV;
	if (dy > 4 * DIV) dy = 4 * DIV;
	if (dy < -4 * DIV) dy = -4 * DIV;

	// 発射
	for (j = 0; j < MAX_e_bullets; j++) {
		if (!e_bullets_active[j]) {
			e_bullets_x[j] = ex * DIV + 8 * DIV;
			e_bullets_y[j] = ey * DIV  + 8 * DIV;
			e_bullets_dx[j] = dx;
			e_bullets_dy[j] = dy;
			e_bullets_active[j] = True;
			break;
		}
	}
}
/*
void fire_aimed_bullet2(unsigned short ex, unsigned short ey)
{
	unsigned char i;
	long dx_raw, dy_raw;
	unsigned short adx, ady; // 絶対値を保持する変数
	unsigned char dir = 0;

	// プレイヤーの中心（+12, +8）と敵の座標の差分
	dx_raw = (long)player_x * DIV + 12 * DIV - (long)ex * DIV;
	dy_raw = (long)player_y * DIV + 8 *DIV - (long)ey * DIV;

	// 絶対値を計算（ここを確実に通す）
	adx = (dx_raw < 0) ? -dx_raw : dx_raw;
	ady = (dy_raw < 0) ? -dy_raw : dy_raw;

	// 8方向の判定ロジック
	if (ady * 2 < adx) {
		// 横方向が圧倒的に強い場合（右か左）
		dir = (dx_raw > 0) ? 0 : 4;
	}
	else if (adx * 2 < ady) {
		// 縦方向が圧倒的に強い場合（下か上）
		dir = (dy_raw > 0) ? 2 : 6;
	}
	else {
		// 斜め領域
		if (dx_raw > 0) {
			dir = (dy_raw > 0) ? 1 : 7; // 右下 or 右上
		} else {
			dir = (dy_raw > 0) ? 3 : 5; // 左下 or 左上
		}
	}

	// --- 以下、弾の発射処理（変更なし） ---
static const signed short speeds[8][2] = {
	{  5 * DIV,  0 * DIV }, // 右
	{  4 * DIV,  3 * DIV }, // 右下
	{  0 * DIV,  5 * DIV }, // 下
	{ -4 * DIV,  3 * DIV }, // 左下 (斜めを強調)
	{ -5 * DIV,  0 * DIV }, // 左
	{ -4 * DIV, -3 * DIV }, // 左上 (斜めを強調)
	{  0 * DIV, -5 * DIV }, // 上
	{  4 * DIV, -3 * DIV }  // 右上
};

	for (i = 0; i < MAX_e_bullets; i++) {
		if (!e_bullets_active[i]) {
			e_bullets_x[i] = ex + 8 * DIV;
			e_bullets_y[i] = ey + 8 * DIV;
			e_bullets_dx[i] = speeds[dir][0];
			e_bullets_dy[i] = speeds[dir][1];
			e_bullets_active[i] = 1;
			break;
		}
	}
}*/

unsigned char base_interval = 50;   // デフォルト
// ====================== update_enemies ======================
__attribute__((optnone)) __attribute__((noinline)) void update_enemies(void) {
	difficulty = (min(1, play_time / (180*COUNT1S)));
	enemy_bullet_speed = 2 + difficulty;
	base_interval = (82 - difficulty * 36);

/*	if (play_time < 120)		base_interval = 55;
	else if (play_time < 360)   base_interval = 47;
	else if (play_time < 720)   base_interval = 40;
	else						base_interval = 35;
*/

//	volatile unsigned char *last_enemies = &enemies_active[MAX_ENEMIES - 1];


__asm__ volatile (
	"push	bc\n"
	"push	hl\n"
	"push	de\n"

	"ld	c,#%1\n"
	"ld	b,0\n"
	//";ld	hl,_enemies_active+MAX_ENEMIES-1\n"
	"ld	hl,_enemies_active + %0\n"
"enemiesloop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jp	z,enemiesskip1\n"

	"push	hl\n"

	"ld	hl,_enemies_x-1\n"
	"add	hl,bc\n"
	"ld	d,h\n"
	"ld	e,l\n"
	"ld	a,#(%2 + 8)\n"
	"cp	(hl)\n"

	"jr	c,enemiesskip2				; a < 0\n"

"enemiesoff:\n"
	"pop	hl\n"
	"ld	(hl),#%4\n"
	"jp	enemiesskip1\n"


"enemiesskip2:\n"
	"ld	hl,_enemies_count-1\n"
	"add	hl,bc\n"
	"inc	(hl)\n"

	"ld	hl,_enemies_type-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	nz,enemiestype1\n"

//";	normal\n"

	"ld	h,d\n"
	"ld	l,e\n"
	"ld	a,(hl)\n"
	"sub	a,#%2\n"
	"ld	(hl),a\n"

	"jr	enemiesskip3\n"


"enemiestype1:\n"
	"cp	1\n"
	"jr	nz,enemiestype2\n"

//";	herizako\n"

	"ld	hl,_enemies_count-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"cp	24\n"
	"jr	nc,enemiestype1_1\n"

	"ld	h,d\n"
	"ld	l,e\n"
	"ld	a,(hl)\n"
	"add	a,#-6 ;ENEMY_SPEED*2\n"
	"ld	(hl),a\n"

	"ld	hl,_enemies_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	e,a\n"

	"ld	a,(_player_y)\n"
	"add	a,#8\n"
	"sub	e\n"

	"sra	a\n"
	"sra	a\n"
	"sra	a\n"
	"ld	e,a\n"

	"ld	a,(hl)\n"
	"add	a,e\n"
	"ld	(hl),a\n"

	"jr	enemiesskip3\n"

"enemiestype1_1:\n"
	"cp	40\n"
	"jr	nc,enemiestype1_2\n"

	"jr	enemiesskip3\n"

"enemiestype1_2:\n"
	"ld	h,d\n"
	"ld	l,e\n"
	"ld	a,(hl)\n"
	"cp	#(%3-6) ;ENEMY_SPEED*4)\n"
	"jr	nc,enemiesoff\n"

	"add	a,#6 ;ENEMY_SPEED*4\n"
	"ld	(hl),a\n"

	"jr	enemiesskip3\n"

"enemiestype2:"
	"cp	2\n"
	"jr	nz,enemiestype3\n"

//";	sin curve\n"
//";	x\n"

	"ld	h,d\n"
	"ld	l,e\n"
	"ld	a,(hl)\n"
	"add	a,#-2\n"
	"ld	(hl),a\n"

//";	y\n"

	"ld	hl,_enemies_count-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"add	a,a\n"
	"ld	e,a\n"
	"ld	d,0\n"

	"ld	hl,_sin_table\n"
	"add	hl,de\n"
	"ld	a,(hl)\n"
	"ld	e,a\n"

	"ld	hl,_enemies_count2-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"add	a,e\n"

	"ld	hl,_enemies_y-1\n"
	"add	hl,bc\n"
	"ld	(hl),a\n"

	//";jr	enemiesskip3\n"

"enemiestype3:\n"

"enemiesskip3:\n"
	"ld	hl,_enemies_shoot_timer-1\n"
	"add	hl,bc\n"
	"inc	(hl)\n"
	"ld	a,(hl)\n"
	"cp	5\n"
	"jr	z,enemiesskip6\n"

"enemiesskip4:\n"
	"ld	a,(_base_interval)\n"
	"cp	(hl)\n"
	"jr	nc,enemiesskip5\n"

	"ld	a,#5\n"
	"ld	(hl),a\n"

"enemiesskip6:\n"

	"ld	hl,_enemies_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	e,a\n"

	"ld	hl,_enemies_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"

	"ld	l,e\n"

	"push	bc\n"
	"call	_fire_aimed_bullet\n"
	"pop	bc\n"

"enemiesskip5:\n"
	"pop	hl\n"

"enemiesskip1:\n"
	"dec	hl\n"
	"dec	c\n"
	"jp	nz,enemiesloop\n"

	"pop	de\n"
	"pop	hl\n"
	"pop	bc\n"

	:
	: "i" (MAX_ENEMIES-1),"i"(MAX_ENEMIES),"i"(ENEMY_SPEED),"i"(width),"i"(False)
	:

);


/*
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (!enemies_active[i]) continue;

		if ((enemies_x[i] <= (ENEMY_SPEED + 8 ))) {
			enemies_active[i] = False;
			continue;
		}
		enemies_count[i] += 1;

		if (enemies_type[i] == 0)		// 通常敵
			enemies_x[i] -= ENEMY_SPEED;
		else if(enemies_type[i] == 1){	  // ヘリザコ - 勢いよく突っ込む
//			dist_x = enemies_x[i] - player_x;
			if(enemies_count[i] < 24){	// 1段階：超急接近
				enemies_x[i] -= 6 //ENEMY_SPEED*2;
				enemies_y[i] += ((player_y + 8 - enemies_y[i]) / 8);
			}else if(enemies_count[i] < 49)	// 2段階：短くホバリング
				enemies_x[i] -= 0;
			else{							// 3段階：右へ全力逃走
				if(enemies_x[i] >= (width - 6)) { //ENEMY_SPEED * 4)) {
					enemies_active[i] = False;
					continue;
				}
				enemies_x[i] += 6 //ENEMY_SPEED*4;
			}
		}

		else if(enemies_type[i] == 2){	  // サインカーブ
			enemies_x[i] -= 2;
			enemies_y[i] = enemies_count2[i] + sin_table[enemies_count[i]];
		}

*/

/*	for (i = 0; i < MAX_ENEMIES; i++) {
		if (!enemies_active[i]) continue;

//		enemies_shoot_timer[i]++;

		if (enemies_shoot_timer[i] == 5) {
			fire_aimed_bullet(enemies_x[i], enemies_y[i]);
			enemies_shoot_timer[i] = 5;
			continue;
		}

		if (enemies_shoot_timer[i] >= base_interval) {
			fire_aimed_bullet(enemies_x[i], enemies_y[i]);
			enemies_shoot_timer[i] = 5;
		}
	}
*/
}


//#define MAX_BULLETS_LAST_OFFSET (MAX_BULLETS-1)
//const unsigned char MAX_BULLETS_LAST_OFFSET = (MAX_BULLETS-1);

void update_input(void) {
	keycode = keyscan();
	// 移動
	// ジョイスティック + キーボード対応（簡易）
	if (keycode & KEY_LEFT1)  if (player_x > PLAYER_SPEED) player_x -= PLAYER_SPEED;
	if (keycode & KEY_RIGHT1) if (player_x < (width - 24) ) player_x += PLAYER_SPEED;
	if (keycode & KEY_UP1)	if (player_y > PLAYER_SPEED) player_y -= PLAYER_SPEED;
	if (keycode & KEY_DOWN1)  if (player_y < (height - 24) ) player_y += PLAYER_SPEED;

	// スペース or ボタンで射撃
	if ((keycode & KEY_A) && shoot_timer == 0) {  // 適当キーコード例
		for (i = 0; i < MAX_BULLETS; i++) {
			if (!bullets_active[i]) {
				bullets_x[i] = player_x + 16 ;
				bullets_y[i] = player_y + 6 ;
				bullets_active[i] = True;
				// 簡易サウンド（PSG）
//				msx_sound(0, 0x00); // 適当に音を鳴らす（後で調整）
				break;
			}
		}

		opt_idx = bullets_mode;
		for(j = 0; j < MAX_Option; ++j){
			++opt_idx;
			if(opt_idx >= MAX_Option)
				opt_idx = 0;
//		for(opt_idx = 0; opt_idx < MAX_Option; ++opt_idx){
			if(Option_active[opt_idx] == False)
				continue;
			for(b_idx = 0; b_idx < MAX_BULLETS; ++b_idx){
				if(bullets_active[b_idx] == True)
					continue;
				bullets_x[b_idx] = (Option_x[opt_idx] + 4);
				bullets_y[b_idx] = (Option_y[opt_idx] + 6);
				bullets_active[b_idx] = True;
				break;
			}
		}
		++bullets_mode;
		if(bullets_mode >= MAX_Option){
			bullets_mode = 0;
		}
		shoot_timer = 8;  // 連射間隔
	}
	if (shoot_timer > 0) shoot_timer--;

	// ボム使用
	if((keycode & KEY_B) && (bomb_stock > 0)){
		if(key_b_flag == False)
			use_bomb();
		key_b_flag = True;
	}else
		key_b_flag = False;
}

__attribute__((optnone)) __attribute__((noinline)) void update_bullets(void) {
	// 射撃
/*	for (i = 0; i < MAX_BULLETS; i++) {
		if (bullets_active[i]) {
			if (bullets_x[i] >= (width  - BULLET_SPEED)) bullets_active[i] = False;
			else
				bullets_x[i] += BULLET_SPEED;
		}
	}*/
__asm__ volatile(
	"push	bc\n"
	"push	hl\n"
	"push	de\n"

	//";ld	b,#MAX_BULLETS\n"
	"ld	c,#%1\n"
	"ld	b,0\n"
	//";ld	hl,_bullets_active+MAX_BULLETS-1\n"
	"ld	hl,_bullets_active+%0\n"
"bulletsloop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	z,bulletsskip1\n"
	//";push	bc\n"
	"push	hl\n"
	//";ld	c,b\n"
	//";ld	b,0\n"
	"ld	hl,_bullets_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	//";cp	#(width - BULLET_SPEED)\n"
	"cp	%2\n"
	"jr	c,bulletskip2				; a < 0\n"
	"pop	hl\n"
	"ld	(hl),#%4\n"
	"jr	bulletskip3\n"

"bulletskip2:\n"
	"ld	e,#%3\n"
	"add	a,e\n"
	"ld	(hl),a\n"
	"pop	hl\n"

"bulletskip3:\n"
	//";pop	bc\n"
"bulletsskip1:\n"
	"dec	hl\n"
	//";djnz	bulletsloop\n"
	"dec	c\n"
	"jr	nz,bulletsloop\n"

	"pop	de\n"
	"pop	hl\n"
	"pop	bc\n"

	: 
	: "i" (MAX_BULLETS - 1), "i" (MAX_BULLETS),"i"(width-BULLET_SPEED),"i"(BULLET_SPEED),"i"(False)
	: "hl"
);
}


void spawn_enemy(void) {
	// 敵出現
	unsigned short k;
	enemy_spawn_timer++;
//	if (enemy_spawn_timer > max(0, (40 - (play_time / (COUNT1S*4))))) {  // 時間で少し難易度アップ
	k = score / 250;
	if(k > 50)
		i = 0;
	else
		i = 50-k;
	if(enemy_spawn_timer > max(18, i)){
		rand_num = randint(0,100);
		if(rand_num < 60) enemy_type = 0;
		else if(rand_num < 85) enemy_type = 1;
		else enemy_type = 2;
		if(enemy_type == 0) hp = 1; else hp = 3;
		base_y = randint(30, height - 40);

		for (i = 0; i < MAX_ENEMIES; i++) {
			if (!enemies_active[i]) {
				enemies_x[i] = (width-1) ;
				enemies_y[i] = randint(30, height - 40); //(30 + (rand() % (height - 40))) ;
				enemies_active[i] = True;
				enemies_hp[i] = hp;
				enemies_shoot_timer[i] = 0;		  // タイマーリセット

				enemies_type[i] = enemy_type;
				enemies_count[i] = 0;
				enemies_count2[i] = base_y;
//				enemies_count_hp[e_idx] = hp;
				enemies_count_flag[i] = False;

				break;
			}
		}
		enemy_spawn_timer = 0;
	}
}

//#define MAX_ebullets_LAST_OFFSET MAX_e_bullets-1

void addscore(unsigned char add)
{
	score += add;
}

__attribute__((optnone)) __attribute__((noinline)) void update_e_bullets(void) {
	// 敵弾更新
//	for (i = 0; i < MAX_e_bullets; i++) {
/*	i = MAX_e_bullets-1;
	do{
		if (!e_bullets_active[i]) continue;

		// 加算前に判定（オーバーフローする前に非アクティブ化）
		if ( (short)(e_bullets_x[i] / DIV) > (width - 1) - (short)(e_bullets_dx[i] / DIV) ||	  // 右に出る予定
			(short)(e_bullets_x[i] / DIV) < - (short)(e_bullets_dx[i] / DIV) ||				// 左に出る予定
			(short)(e_bullets_y[i] / DIV) > (height) - (short)(e_bullets_dy[i] / DIV) ||	  // 下に出る予定
			(short)(e_bullets_y[i] / DIV) < - (short)(e_bullets_dy[i] / DIV) ) {				// 上に出る予定
			e_bullets_active[i] = False;
		} else {
			e_bullets_x[i] += e_bullets_dx[i];
			e_bullets_y[i] += e_bullets_dy[i];
		}
	}while(i--);*/


__asm__ volatile(
//	"push	bc\n"
//	"push	hl\n"
//	"push	de\n"

	"ld	c,#%1\n"
	"ld	b,0\n"
	//";ld	hl,_e_bullets_active+MAX_e_bullets-1\n"
	"ld	hl,_e_bullets_active+%0\n"
"e_bullets2loop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jp	z,e_bullets2skip1\n"

	"push	hl\n"

	//";jr	e_bullets2skip3\n"

	//";x\n"

	"ld	hl,_e_bullets_x-1\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	e,a\n"

	"ld	hl,_e_bullets_dx-1\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"

	"cp	128\n"
	"jr	nc,e_bullets2skip_x				; dx < 0\n"

	"add	a,e\n"
	"jr	c,e_bullets2skip5\n"
	"jr	e_bullets2skip_x2				; a > 256\n"

"e_bullets2skip_x:\n"

	"add	a,e\n"
	"jr	nc,e_bullets2skip5				; a < 0\n"


"e_bullets2skip_x2:\n"

	//";y\n"

	"ld	hl,_e_bullets_y-1\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	e,a\n"

	"ld	hl,_e_bullets_dy-1\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"

	"cp	128\n"
	"jr	nc,e_bullets2skip_y				; dy < 0\n"

	"add	a,e\n"
	"cp	#%2\n"
	"jr	nc,e_bullets2skip5				; a >=192\n"

	"jr	e_bullets2skip3\n"

"e_bullets2skip_y:\n"

	"add	a,e\n"
	"jr	nc,e_bullets2skip5				; a < 0\n"


"e_bullets2skip3:\n"
//";//			e_bullets_x[i] += e_bullets_dx[i];\n"

	"push	bc\n"
	"ld	hl,_e_bullets_x-2\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ex	de,hl\n"
	//";ld	hl,(de)\n"
	"ex	de,hl\n"
	"ld		e,(hl)\n"
	"inc	hl\n"
	"ld		d,(hl)\n"
	"dec	hl\n"
	"ex	de,hl\n"

	"push	de\n"
	"push	hl\n"

	"ld	hl,_e_bullets_dx-2\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ex	de,hl\n"
	//";ld	hl,(de)\n"
	"ex	de,hl\n"
	"ld		e,(hl)\n"
	"inc	hl\n"
	"ld		d,(hl)\n"
	"dec	hl\n"
	"ex	de,hl\n"

	"pop	bc\n"
	"pop	de\n"

	"add	hl,bc\n"
	"pop	bc\n"

	//";ld	(de),hl\n"
	"ex	de,hl\n"
	"ld	(hl),e\n"
	"inc	hl\n"
	"ld	(hl),d\n"
	"dec	hl\n"
	"ex	de,hl\n"

//";//			e_bullets_y[i] += e_bullets_dy[i];\n"

	"push	bc\n"
	"ld	hl,_e_bullets_y-2\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ex	de,hl\n"
	//";ld	hl,(de)\n"
	"ex	de,hl\n"
	"ld		e,(hl)\n"
	"inc	hl\n"
	"ld		d,(hl)\n"
	"dec	hl\n"
	"ex	de,hl\n"

	"push	de\n"
	"push	hl\n"

	"ld	hl,_e_bullets_dy-2\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ex	de,hl\n"
	//";ld	hl,(de)\n"
	"ex	de,hl\n"
	"ld		e,(hl)\n"
	"inc	hl\n"
	"ld		d,(hl)\n"
	"dec	hl\n"
	"ex	de,hl\n"

	"pop	bc\n"
	"pop	de\n"

	"add	hl,bc\n"
	"pop	bc\n"

	//";ld	(de),hl\n"
	"ex	de,hl\n"
	"ld	(hl),e\n"
	"inc	hl\n"
	"ld	(hl),d\n"
	"dec	hl\n"
	"ex	de,hl\n"

	"jr	e_bullets2skip4\n"

"e_bullets2skip5:\n"

//";//			e_bullets_active[i] = False;\n"
	"pop	hl\n"
	//";ld	hl,_e_bullets_active\n"
	//";add	hl,bc\n"
	"ld	a,#%3\n"
	"ld	(hl),a\n"
	"jr	e_bullets2skip1\n"

"e_bullets2skip4:\n"
	"pop	hl\n"
"e_bullets2skip1:\n"

	"dec	hl\n"
	"dec	c\n"
	"jp	nz,e_bullets2loop\n"

//	"pop	de\n"
//	"pop	hl\n"
//	"pop	bc\n"

	: 
//	: "i" (MAX_ebullets_LAST_OFFSET)
	: "i" (MAX_e_bullets-1), "i" (MAX_e_bullets),"i"(height),"i"(False)
	: "bc","hl","de"

);
}

unsigned char b; //, e, eb;

//const unsigned char MAX_BULLETS_LAST_OFFSET = (MAX_BULLETS-1);
//#define MAX_ENEMIES_LAST_OFFSET (MAX_ENEMIES-1)


//const unsigned char MAX_BULLETS_LAST_OFFSET = (MAX_BULLETS-1);
//#define MAX_ENEMIES_LAST_OFFSET enemies_active[MAX_ENEMIES-1];

//volatile unsigned char *last_bullets = &bullets_active[MAX_BULLETS-1];
//volatile unsigned char *last_enemies = &enemies_active[MAX_ENEMIES-1];

__attribute__((optnone)) __attribute__((noinline)) void check_collisions(void) {
//	unsigned char b, e, eb;

	if(!(vdps0 & 0x20))
		return;

// 自機弾 vs 敵（既存のまま）
__asm__ volatile(
	//";push	bc\n"
	//";push	hl\n"
	//";push	de\n"

	"ld	c,#%3\n"
	"ld	b,0\n"
	//";ld	hl,_bullets_active+MAX_BULLETS-1\n"
	"ld	hl,_bullets_active+%0\n"
	"ld	hl,#%0\n"
	"ld	a,c\n"
	"ld	(_b),a\n"
	"ld	d,0\n"

"enemiesloop2:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jp	z,bullets2skip1\n"

	"push	hl\n"

	"ld	c,#%4\n"
	//";ld	b,0\n"
	//";ld	hl,_enemies_active+MAX_ENEMIES-1\n"
//	"ld	hl,#_last_enemies\n"
	"ld	hl,_enemies_active+%1\n"

"bullets2loop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jp	z,bulletsskip2\n"

	"push	hl\n"

//";	if(bullets_x[b] - (enemies_x[e] + 20) < 0)){\n"

	//";push	bc\n"
	"ld	hl,_bullets_x-1\n"
	"ld	a,(_b)\n"
	"ld	e,a\n"
	//";ld	d,0\n"
	"add	hl,de\n"
	"ld	a,(hl)\n"
	"ld	e,a		;	e = bullets_x[b]\n"
	"add	a,#-20\n"

	//";pop	bc\n"
	"ld	hl,_enemies_x-1\n"
	"add	hl,bc\n"
	"cp	(hl)\n"

	"jp	nc,bulletsskip5				; a >= 0\n"

//";	if(enemies_x[e] - (bullets_x[b] + 8 ) < 0){\n"

	"ld	a,#8\n"
	"add	a,e\n"
	"ld	e,a\n"
	"ld	a,(hl)\n"
	"cp	e\n"

	"jp	nc,bulletsskip5				; a >= 0\n"

//";	if(bullets_y[b] - (enemies_y[e] + 16) < 0){\n"

	//";push	bc\n"
	"ld	hl,_bullets_y-1\n"
	"ld	a,(_b)\n"
	"ld	e,a\n"
	"add	hl,de\n"
	"ld	a,(hl)\n"
	"ld	e,a		; e= bullets_y[b]\n"
	"add	a,#-16\n"

	//";pop	bc\n"
	"ld	hl,_enemies_y-1\n"
	"add	hl,bc\n"
	"cp	(hl)\n"

	"jp	nc,bulletsskip5				; a >= 0\n"

//";	if(enemies_y[e] - (bullets_y[b] + 6) < 0){\n"

	"ld	a,#6\n"
	"add	a,e\n"
	"ld	e,a\n"
	"ld	a,(hl)\n"
	"cp	e\n"

	"jp	nc,bulletsskip5				; a >= 0\n"

	//";push	bc\n"
	"ld	hl,_bullets_active-1\n"
	"ld	a,(_b)\n"
	"ld	e,a\n"
	"add	hl,de\n"
	//";pop	bc\n"
	"ld	a,#%6\n"
	"ld	(hl),a\n"

	"ld	hl,_enemies_hp-1\n"
	"add	hl,bc\n"
	"dec	(hl)\n"
	"jp	nz,bulletsskip4\n"


	"ld	hl,_enemies_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	(_ex),a\n"
	"ld	hl,_enemies_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	(_ey),a\n"


//";	Particle\n"
	"ld	e,#3\n"
"collisionloop3:\n"

	"ld	hl,_enemies_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"add	a,#8\n"
	"jr	c,collisionloopskip\n"
	"ld	d,a\n"

	"ld	hl,_enemies_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"add	a,#8\n"
	"jr	c,collisionloopskip\n"
	"ld	l,d\n"

	"push	bc\n"
	"push	de\n"
	"call	_Particles_append\n"
	"pop	de\n"
	"pop	bc\n"
"collisionloopskip:\n"
	"dec	e\n"
	"jr	nz,collisionloop3\n"





	"ld	hl,_enemies_active-1\n"
	"add	hl,bc\n"
	"ld	a,#%6\n"
	"ld	(hl),a\n"

	"push	bc\n"
	"ld	a,100\n"
	"call	_addscore\n"
	"pop	bc\n"

	"ld	a,#%5\n"
	"ld	(_score_display_flag),a\n"

	"ld	hl,_kill_count\n"
	"inc	(hl)\n"

	"ld	a,1\n"
	"ld	l,0\n"

	"push	bc\n"
	"call _msx_sound\n"
	"pop	bc\n"

//";	if(randint(0,100) < 40){\n"
	"xor	a\n"
	"ld	l,100\n"
	"push	bc\n"
	"call	_randint\n"
	"pop	bc\n"
	"cp	40\n"
	"jr	nc,ChainItemEnd\n"

	"ld	d,0\n"
	"ld	e,#%7\n"
"ChainItemloop:\n"
	"ld	hl,_ChainItem_active-1\n"
	"add	hl,de\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	nz,ChainItemSkip\n"
	"ld	a,#%5\n"
	"ld	(hl),a\n"

	"ld	hl,_ChainItem_x-1\n"
	"add	hl,de\n"
	"ld	a,(_ex)\n"
	"ld	(hl),a\n"
	"ld	hl,_ChainItem_y-1\n"
	"add	hl,de\n"
	"ld	a,(_ey)\n"
	"ld	(hl),a\n"

	"ld	hl,_ChainItem_timer-2\n"
	"add	hl,de\n"
	"add	hl,de\n"
//	"ld	de,#COUNT1S*4\n"
	"ld	de,%2\n"
	//";ld	(hl),de\n"
	"ld	(hl), e\n"
	"inc	hl\n"
	"ld	(hl), d\n"
	"dec	hl\n"

	"jr	ChainItemEnd\n"

"ChainItemSkip:\n"
	"dec	e\n"
	"jr	nz,ChainItemloop\n"
"ChainItemEnd:\n"


	"ld	a,(_option_cooldown)\n"
	"dec	a\n"
	"jr	nz,OptionItemEnd2\n"

	"ld	d,0\n"
	"ld	e,#%8\n"
"OptionItemloop:\n"
	"ld	hl,_OptionItem_active-1\n"
	"add	hl,de\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	nz,OptionItemSkip\n"
	"ld	a,#%5\n"
	"ld	(hl),a\n"

	"ld	hl,_OptionItem_x-1\n"
	"add	hl,de\n"
	"ld	a,(_ex)\n"
	"ld	(hl),a\n"
	"ld	hl,_OptionItem_y-1\n"
	"add	hl,de\n"
	"ld	a,(_ey)\n"
	"ld	(hl),a\n"

	"ld	hl,_OptionItem_timer-2\n"
	"add	hl,de\n"
	"add	hl,de\n"
	"ld	de,#300\n"
	//";ld	(hl),de\n"
	"ld	(hl), e\n"
	"inc	hl\n"
	"ld	(hl), d\n"
	"dec	hl\n"

	"ld	a,10\n"
	"ld	(_option_cooldown),a\n"
	"jr	OptionItemEnd\n"

"OptionItemSkip:\n"
	"dec	e\n"
	"jr	nz,OptionItemloop\n"

	"jr	OptionItemEnd\n"
"OptionItemEnd2:\n"
	"ld	(_option_cooldown),a\n"

"OptionItemEnd:\n"


	"ld	a,(_shield_active)\n"
	"or	a\n"
	"jr	nz,ShieldItemEnd\n"

	"xor	a\n"
	"ld	l,100\n"
	"push	bc\n"
	"call	_randint\n"
	"pop	bc\n"
	"cp	12\n"
	"jr	nc,ShieldItemEnd\n"

	"ld	d,0\n"
	"ld	e,#%9\n"
"ShieldItemloop:\n"
	"ld	hl,_ShieldItem_active-1\n"
	"add	hl,de\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	nz,ShieldItemSkip\n"
	"ld	a,#%5\n"
	"ld	(hl),a\n"

	"ld	hl,_ShieldItem_x-1\n"
	"add	hl,de\n"
	"ld	a,(_ex)\n"
	"ld	(hl),a\n"
	"ld	hl,_ShieldItem_y-1\n"
	"add	hl,de\n"
	"ld	a,(_ey)\n"
	"ld	(hl),a\n"

	"ld	hl,_ShieldItem_timer-2\n"
	"add	hl,de\n"
	"add	hl,de\n"
	"ld	de,#200\n"
	//";ld	(hl),de\n"
	"ld	(hl), e\n"
	"inc	hl\n"
	"ld	(hl), d\n"
	"dec	hl\n"

	"jr	ShieldItemEnd\n"

"ShieldItemSkip:\n"
	"dec	e\n"
	"jr	nz,ShieldItemloop\n"

"ShieldItemEnd:\n"


	"ld	a,(_bomb_stock)\n"
	"cp	3\n"
	"jr	nc,BombItemEnd\n"

	"xor	a\n"
	"ld	l,100\n"
	"push	bc\n"
	"call	_randint\n"
	"pop	bc\n"
	"cp	10\n"
	"jr	nc,BombItemEnd\n"

	"ld	d,0\n"
	"ld	e,#%10\n"
"BombItemloop:\n"
	"ld	hl,_BombItem_active-1\n"
	"add	hl,de\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	nz,BombItemSkip\n"
	"ld	a,#%5\n"
	"ld	(hl),a\n"

	"ld	hl,_BombItem_x-1\n"
	"add	hl,de\n"
	"ld	a,(_ex)\n"
	"ld	(hl),a\n"
	"ld	hl,_BombItem_y-1\n"
	"add	hl,de\n"
	"ld	a,(_ey)\n"
	"ld	(hl),a\n"

	"ld	hl,_BombItem_timer-2\n"
	"add	hl,de\n"
	"add	hl,de\n"
	"ld	de,#270\n"
	//";ld	(hl),de\n"
	"ld	(hl), e\n"
	"inc	hl\n"
	"ld	(hl), d\n"
	"dec	hl\n"

	"jr	BombItemEnd\n"

"BombItemSkip:\n"
	"dec	e\n"
	"jr	nz,BombItemloop\n"

"BombItemEnd:\n"

	"jr	bulletsskip5\n"

"bulletsskip4:\n"

//";	Particle\n"
	"ld	e,#3\n"
"bulletsloop3:\n"

	"ld	hl,_enemies_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	//";add	a,#8\n"
	//";jr	c,bulletsloopskip\n"
	"ld	d,a\n"

	"ld	hl,_enemies_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	//";add	a,#8\n"
	//";jr	c,bulletsloopskip\n"

	"ld	l,d\n"

	"push	bc\n"
	"push	de\n"
	"call	_Particles_append\n"
	"pop	de\n"
	"pop	bc\n"
"bulletsloopskip:\n"
	"dec	e\n"
	"jr	nz,bulletsloop3\n"



"bulletsskip5:\n"
	"pop	hl\n"

"bulletsskip2:\n"
	"dec	hl\n"
	"dec	c\n"
	"jp	nz,bullets2loop\n"

	"pop	hl\n"

"bullets2skip1:\n"
	"dec	hl\n"
	"ld	a,(_b)\n"
	"dec	a\n"
	"ld	(_b),a\n"
	"jp	nz,enemiesloop2\n"

	//";pop	de\n"
	//";pop	hl\n"
	//";pop	bc\n"

	:
	: "i"(MAX_BULLETS - 1),"i"(MAX_ENEMIES - 1),"i"(COUNT1S*4),"i"(MAX_BULLETS),"i"(MAX_ENEMIES),"i"(True),"i"(False),"i"(MAX_ChainItem),"i"(MAX_OptionItem),"i"(MAX_ShieldItem),"i"(MAX_BombItem)
	: "bc","hl","de"
);

/*	for (b = 0; b < MAX_BULLETS; b++) {
		if (!bullets_active[b]) continue;
		for (e = 0; e < MAX_ENEMIES; e++) {
			if (!enemies_active[e]) continue;

			if (bullets_x[b] < (enemies_x[e] + 20 )){
				if((bullets_x[b] + 8 ) > enemies_x[e]){
					if(bullets_y[b] < (enemies_y[e] + 16 )){
						if((bullets_y[b] + 6 ) > enemies_y[e]){
							bullets_active[b] = False;
							if (--enemies_hp[e] == 0) {
								ex = enemies_x[e];
								ey = enemies_y[e];
//								for(i = 0; i < 4; ++i)
//									Particles_append(ex, ey);

								enemies_active[e] = False;
								score += 100;
								score_display_flag = True;
//								all_display_flag = True;
								kill_count += 1;
								// 簡易サウンド（PSG）
								msx_sound(1, 0); // 適当に音を鳴らす（後で調整）

								if(randint(0,100) < 40){

									for(item_idx = 0; item_idx < MAX_ChainItem; ++item_idx){
										if(ChainItem_active[item_idx] == True)
											continue;
										ChainItem_x[item_idx] = ex;
										ChainItem_y[item_idx] = ey;
										ChainItem_timer[item_idx] = COUNT1S * 4;
										ChainItem_active[item_idx] = True;
//				msx_sound(0, 0x00); // 適当に音を鳴らす（後で調整）
										break;
									}
								}
								if(option_cooldown <= 0){
									for(item_idx = 0; item_idx < MAX_OptionItem; ++item_idx){
										if(OptionItem_active[item_idx] == True)
											continue;
										OptionItem_x[item_idx] = ex;
										OptionItem_y[item_idx] = ey;
										OptionItem_timer[item_idx] = 300;
										OptionItem_active[item_idx] = True;
										break;

									}
									option_cooldown = 10;
								}
								else
									option_cooldown -= 1;

								if((randint(0, 100) < 12) && !shield_active){
									for(item_idx = 0; item_idx < MAX_ShieldItem; ++item_idx){
										if(ShieldItem_active[item_idx] == True)
											continue;
										ShieldItem_x[item_idx] = ex;
										ShieldItem_y[item_idx] = ey;
										ShieldItem_timer[item_idx] = 280;
										ShieldItem_active[item_idx] = True;
										break;
									}
								}

								if((randint(0, 100) < 10) && (bomb_stock < 3)){
									for(item_idx = 0; item_idx < MAX_BombItem; ++item_idx){
										if(BombItem_active[item_idx] == True)
											continue;
										BombItem_x[item_idx] = ex;
										BombItem_y[item_idx] = ey;
										BombItem_timer[item_idx] = 270;
										BombItem_active[item_idx] = True;
										break;
									}
								}
							}
							else{
//								for (i = 0; i < 4; ++i)
//									Particles_append(enemies_x[e_idx] + 8, enemies_y[e_idx] + 8);
							}
							break;
						}
					}
				}
			}
		}
	}
*/

	// 敵弾 vs 自機（既存のまま）
__asm__ volatile(
//	"push	bc\n"
//	"push	hl\n"
//	"push	de\n"

	"ld	c,#%1\n"
	"ld	b,0\n"
	//";ld	hl,_e_bullets_active+MAX_e_bullets-1\n"
	"ld	hl,_e_bullets_active+%0\n"
"e_bulletsloop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jp	z,e_bulletsskip1\n"

	"push	hl\n"


//";	if ((player_x - 4 - e_bullets_x[eb] / DIV) < 0 ){\n"

	"ld	hl,_e_bullets_x-1\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"add	a,#4\n"
	"ld	e,a\n"
	"ld	a,(_player_x)\n"
	"cp	e\n"

	"jr	nc,e_bulletsskip5				; a >= 0\n"

//";	if((e_bullets_x[eb] / DIV - (player_x + (16+4))) < 0){\n"

	"ld	a,(_player_x)\n"
	"add	a,#(16+4)\n"
	"ld	e,a\n"
	"ld	a,(hl)\n"
	"cp	e\n"

	"jr	nc,e_bulletsskip5				; a >= 0\n"

//";	if(player_y - (e_bullets_y[eb] / DIV + 4) < 0 ){\n"

	"ld	hl,_e_bullets_y-1\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"add	a,#4\n"
	"ld	e,a\n"
	"ld	a,(_player_y)\n"
	"cp	e\n"

	"jr	nc,e_bulletsskip5				; a >= 0\n"

//";	if(e_bullets_y[eb] / DIV  - (player_y + (10+4)) < 0 ){\n"

	"ld	a,(_player_y)\n"
	"add	a,#(10+4)\n"
	"ld	e,a\n"
	"ld	a,(hl)\n"
	"cp	e\n"

	"jr	nc,e_bulletsskip5				; a >= 0\n"


	"ld	a,(_shield_active)\n"
	"or	a\n"
	"jr	z,e_bulletsskip4\n"

	"ld	a,#%2\n"
	"ld	(_shield_active),a\n"


//";	Particle\n"
	"ld	e,#3\n"
"e_bulletsloop3:\n"

	"ld	a,(_player_y)\n"
	"add	a,#8\n"
	"jr	c,e_bulletsloopskip\n"
	"ld	l,a\n"

	"ld	a,(_player_x)\n"
	"add	a,#8\n"
	"jr	c,e_bulletsloopskip\n"

	"push	bc\n"
	"push	de\n"
	"call	_Particles_append\n"
	"pop	de\n"
	"pop	bc\n"
"e_bulletsloopskip:\n"
	"dec	e\n"
	"jr	nz,e_bulletsloop3\n"


	"jr	e_bulletsskip3\n"

"e_bulletsskip4:\n"
	"ld	a,#1\n"
	"ld	(_game_over),a\n"

"e_bulletsskip3:\n"
	"ld	hl,_e_bullets_active-1\n"
	"add	hl,bc\n"
	"ld	a,#%2\n"
	"ld	(hl),a\n"


"e_bulletsskip5:\n"
	"pop	hl\n"

"e_bulletsskip1:\n"
	"dec	hl\n"
	"dec	c\n"
	"jp	nz,e_bulletsloop\n"

//	"pop	de\n"
//	"pop	hl\n"
//	"pop	bc\n"

	:
	: "i"(MAX_e_bullets - 1), "i"(MAX_e_bullets), "i"(False)
 //BULLETS_LAST_OFFSET),"i"(MAX_ENEMIES_LAST_OFFSET)
	: "bc","hl","de"

);
/*	for (eb = 0; eb < MAX_e_bullets; eb++) {
		if (!e_bullets_active[eb]) continue;
		if (player_x < e_bullets_x[eb] / DIV + 4 ){
			if(player_x + (16+4)  > e_bullets_x[eb] / DIV){
				if(player_y < e_bullets_y[eb] / DIV  + 4 ){
					if(player_y + (10+4)  > e_bullets_y[eb] / DIV ){
						if(shield_active){
							shield_active = False;
//							for(i = 0; i < 4; ++i)
//								Particles_append(player_x + 8, player_y + 8);
						}else
							game_over = 1;

						e_bullets_active[eb] = False;
					}
				}
			}
		}
	}
*/

	// 自機 vs 敵機 当たり判定
__asm__ volatile(
//	"push	bc\n"
//	"push	hl\n"
//	"push	de\n"

	"ld	c,#%1\n"
	"ld	b,0\n"
//	//";ld	hl,_enemies_active+MAX_ENEMIES-1\n"
	"ld	hl,_enemies_active+%0\n"
"enemies2loop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jp	z,enemies2skip1\n"

	"push	hl\n"


//";	if ((player_x + 4 - (enemies_x[e] + 16) < 0 ){\n"

	"ld	hl,_enemies_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"add	a,#(16-4)\n"
	"ld	e,a\n"
	"ld	a,(_player_x)\n"
	"cp	e\n"

	"jr	nc,enemies2skip5				; a >= 0\n"

//";	if((enemies_x[e]  - (player_x + 24)) < 0){\n"

	"ld	a,(_player_x)\n"
	"add	a,#24\n"
	"ld	e,a\n"
	"ld	a,(hl)\n"
	"cp	e\n"

	"jr	nc,enemies2skip5				; a >= 0\n"

//";	if(player_y + 4 - (enemies_y[e] + 8) < 0 ){\n"

	"ld	hl,_enemies_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"add	a,#(8-4)\n"
	"ld	e,a\n"

	"ld	a,(_player_y)\n"

	"add	a,#(4-8)\n"
	"cp	e\n"

	"jr	nc,enemies2skip5				; a >= 0\n"

//";	if(enemies_y[e] - (player_y + 14)) < 0 ){\n"

	"ld	a,(_player_y)\n"
	"add	a,#14\n"
	"ld	e,a\n"
	"ld	a,(hl)\n"
	"cp	e\n"

	"jr	nc,enemies2skip5				; a >= 0\n"


	"ld	a,(_shield_active)\n"
	"or	a\n"
	"jr	z,enemies2skip4\n"

	"ld	a,#%2\n"
	"ld	(_shield_active),a\n"


//";	Particle\n"
	"ld	e,#3\n"
"enemies2loop3:\n"
	"ld	a,(_player_y)\n"
	"add	a,#8\n"
	"jr	c,enemiesloopskip\n"
	"ld	l,a\n"

	"ld	a,(_player_x)\n"
	"add	a,#8\n"
	"jr	c,enemiesloopskip\n"

	"push	bc\n"
	"push	de\n"
	"call	_Particles_append\n"
	"pop	de\n"
	"pop	bc\n"
"enemiesloopskip:\n"
	"dec	e\n"
	"jr	nz,enemies2loop3\n"


	"jr	enemies2skip3\n"

"enemies2skip4:\n"
	"ld	a,#1\n"
	"ld	(_game_over),a\n"

"enemies2skip3:\n"
	"ld	hl,_enemies_active-1\n"
	"add	hl,bc\n"
	"ld	a,#%2\n"
	"ld	(hl),a\n"
	"ld	c,1		; break\n"

"enemies2skip5:\n"
	"pop	hl\n"

"enemies2skip1:\n"
	"dec	hl\n"
	"dec	c\n"
	"jp	nz,enemies2loop\n"

//	"pop	de\n"
//	"pop	hl\n"
//	"pop	bc\n"

	:
	: "i"(MAX_ENEMIES - 1), "i"(MAX_ENEMIES),"i"(False)
	: "bc","hl","de"

);
/*	for (e = 0; e < MAX_ENEMIES; e++) {
		if (!enemies_active[e]) continue;

		// 自機と敵の矩形が重なっているかチェック
		if (player_x + 4  < enemies_x[e] + 16 ){   // 自機左 < 敵右
			if(player_x + 24  > enemies_x[e]){	   // 自機右 > 敵左
				if(player_y + 4  < enemies_y[e] + 8 ){   // 自機上 < 敵下
					if(player_y + 14  > enemies_y[e]) {	   // 自機下 > 敵上
						if(shield_active){
							shield_active = False;
//							for (i = 0; i < 4; ++i)
//								Particles_append(player_x + 8, player_y + 8);
						}else
							game_over = 1;
						enemies_active[e_idx] = False;
						break;
					}
				}
			}
		}
	}
*/
}

void check_items(void) {
	// アイテム処理
	for(item_idx = 0; item_idx < MAX_ChainItem; ++item_idx){
		if(ChainItem_active[item_idx]  == False)
			continue;

		if((abs(player_x + 8 - ChainItem_x[item_idx]) < 20) && (abs(player_y + 8 - ChainItem_y[item_idx]) < 20)){
			chain_count += 1;
			chain_timer = COUNT1S * 4; //240;
			score += 100 * chain_count;
			score_display_flag = True;
//			seflag = 4;
			ChainItem_active[item_idx] = False;
			chain_display_flag = True;
			msx_sound(0, 0x00); // 適当に音を鳴らす（後で調整）
			continue;
		}
		if((ChainItem_x[item_idx] < 2) || (ChainItem_timer[item_idx] <= 0)){
			chain_count = 0;
			ChainItem_active[item_idx] = False;
			chain_display_flag = True;
		}
		ChainItem_x[item_idx] -= 2;
		ChainItem_timer[item_idx] -= 1;
	}
	if(chain_count > 0){
		chain_timer -= 1;
		if(chain_timer <= 0){
			chain_count = 0;
			chain_display_flag = True;
		}
	}

	for(item_idx = 0; item_idx < MAX_OptionItem; ++item_idx){
		if(OptionItem_active[item_idx] == False)
			continue;

		if((abs(player_x + 8 - OptionItem_x[item_idx]) < 22) && (abs(player_y + 8 - OptionItem_y[item_idx]) < 22)){
			for(opt_idx = 0; opt_idx <  MAX_Option; ++opt_idx){
				if(Option_active[opt_idx] == True)
					continue;
				if(opt_idx == 0)
					offset = 25;
				else
					offset = -25;
				Option_offset_y[opt_idx] = offset;
				Option_x[opt_idx] = 0;
				Option_y[opt_idx] = 0;
				Option_active[opt_idx] = True;
				break;
			}

			OptionItem_active[item_idx] = False;
//			seflag = 4;
			msx_sound(0, 0x00); // 適当に音を鳴らす（後で調整）
			continue;
		}
		else if((OptionItem_x[item_idx] < 1) || (OptionItem_timer[item_idx] <= 0)){
			OptionItem_active[item_idx] = False;
//			msx_print_num(7, 3, OptionItem_x[0], 3);
//			msx_print_num(7, 4, OptionItem_timer[0], 3);
//			msx_print(7, 5, "LOST");
			continue;
		}
		OptionItem_x[item_idx] -= 1;
		OptionItem_timer[item_idx] -= 1;
//		msx_print_num(7, 3, OptionItem_x[0], 3);
//		msx_print(7, 5, "    ");
	}


	for(item_idx =0; item_idx < MAX_ShieldItem; ++item_idx){
		if(ShieldItem_active[item_idx] == False)
			continue;

		if((abs(player_x + 8 - ShieldItem_x[item_idx]) < 22) && (abs(player_y + 8 - ShieldItem_y[item_idx]) < 22)){
			shield_active = True;
			ShieldItem_active[item_idx] = False;
//			seflag = 4;
			msx_sound(0, 0x00); // 適当に音を鳴らす（後で調整）
			continue;
		}
		if((ShieldItem_x[item_idx] < (2)) || (ShieldItem_timer[item_idx] <= 0)){
			ShieldItem_active[item_idx] = False;
			continue;
		}
		ShieldItem_x[item_idx] -= 2;
		ShieldItem_timer[item_idx] -= 1;

	}
	for(item_idx = 0; item_idx < MAX_BombItem; ++item_idx){
		if(BombItem_active[item_idx] == False)
			continue;

		if((abs(player_x + 8 - BombItem_x[item_idx]) < 22) && (abs(player_y + 8 - BombItem_y[item_idx]) < 22)){
			bomb_stock = min(3, bomb_stock + 1);
			bomb_display_flag = True;
			BombItem_active[item_idx] = False;
//			seflag = 4;
			msx_sound(0, 0x00); // 適当に音を鳴らす（後で調整）
			continue;
		}
		if((BombItem_x[item_idx] < (2)) || (BombItem_timer[item_idx] <= 0)){
			BombItem_active[item_idx] = False;
			continue;
		}
		BombItem_x[item_idx] -= 2;
		BombItem_timer[item_idx] -= 1;
	}


	for(opt_idx = 0; opt_idx < MAX_Option; ++opt_idx){
		Option_x[opt_idx] += (((player_x + 8) - Option_x[opt_idx]) / 4);
		Option_y[opt_idx] += (((player_y + Option_offset_y[opt_idx]) - Option_y[opt_idx]) / 4);
	}
}

__attribute__((noinline)) void udpate_Particle(void)
{
	// パーティクル更新
/*	for(p_idx = 0; p_idx < MAX_Particle; ++p_idx){
		if(Particle_active[p_idx] == False)
			continue;

		Particle_x[p_idx] += Particle_vx[p_idx];
		Particle_y[p_idx] += Particle_vy[p_idx];
		Particle_life[p_idx] -= 1;

		if(Particle_life[p_idx] <= 0)
			Particle_active[p_idx] = False;
	}*/

__asm__ volatile(
//	"push	bc\n"
//	"push	hl\n"
//	"push	de\n"

	"ld	c,#%1\n"
	"ld	b,0\n"
//	"ld	hl,_Particle_active+MAX_Particle-1\n"
	"ld	hl,_Particle_active+%0\n"
"Particleloop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jp	z,Particleskip1\n"

	"push	hl\n"

	//";jr	Particleskip3\n"

	"ld	hl,_Particle_life-1\n"
	"add	hl,bc\n"
	"dec	(hl)\n"
	//";ld	a,(hl)\n"
	//";or	a\n"
	"jr	z,Particleskip5\n"

//";//	x\n"

	"ld	hl,_Particle_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"

//";	cp	#254\n"
//";	jr	nc,Particleskip5\n"

	"ld	e,a\n"

	"ld	hl,_Particle_vx-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"

	"cp	128\n"
	"jr	nc,Particleskip_x				; vx < 0\n"

	"add	a,e\n"
	"jr	c,Particleskip5\n"

	"jr	Particleskip_x2					; a > 256\n"

"Particleskip_x:\n"

	"add	a,e\n"
	"jr	nc,Particleskip5				; a < 0\n"


"Particleskip_x2:\n"

//";//	y\n"

	"ld	hl,_Particle_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	e,a\n"

	"ld	hl,_Particle_vy-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"

	"cp	128\n"
	"jr	nc,Particleskip_y				; vy < 0\n"

	"add	a,e\n"
	"cp	#%2\n"
	"jr	nc,Particleskip5				; a >=192\n"

	"jr	Particleskip3\n"

"Particleskip_y:\n"

	"add	a,e\n"
	"jr	nc,Particleskip5				; a < 0\n"


"Particleskip3:\n"
	"ld	hl,_Particle_vx-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	e,a\n"

	"ld	hl,_Particle_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"add	a,e\n"

	"ld	(hl),a\n"


	"ld	hl,_Particle_vy-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	e,a\n"

	"ld	hl,_Particle_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"add	a,e\n"

	"ld	(hl),a\n"
	"jr	Particleskip4\n"

"Particleskip5:\n"

	"pop	hl\n"
	//";ld	hl,_Particle_active\n"
	//";add	hl,bc\n"
	"ld	a,#%3\n"
	"ld	(hl),a\n"
	"jr	Particleskip1\n"

"Particleskip4:\n"
	"pop	hl\n"
"Particleskip1:\n"

	"dec	hl\n"
	"dec	c\n"
	"jp	nz,Particleloop\n"

//	"pop	de\n"
//	"pop	hl\n"
//	"pop	bc\n"

	:
	: "i" (MAX_Particle-1),"i" (MAX_Particle), "i"(height),"i"(False)
	: "bc","hl","de"

);
}

// ====================== 描画 ======================
signed char add = 1;
unsigned char spr_count;
unsigned char max_spr_count = 0;

void draw_sprites(void) {
	if(add > 0){
		spr_count = 0;

	msx_set_sprite(spr_count, player_x , player_y , 0, 15);
	spr_count++;
	msx_set_sprite(spr_count, player_x , player_y , (shield_active) ? 44 : 40, 7);
	spr_count++;

/*	for (i = 0; i < MAX_BULLETS; i++) {
		if (bullets_active[i]){
			msx_set_sprite(spr_count, bullets_x[i] , bullets_y[i] , 4, 9);
			spr_count++;
		}
//		else
//			msx_set_sprite(SPR_BULLETS + i, 0, 208, 0, 0);
	}*/
__asm__ volatile(
	"ld	a,(_spr_count)\n"
	"ld	l,a\n"
	"ld	h,0\n"

	"add	hl,hl\n"
	"add	hl,hl\n"

	"ld	de,_spr_chr\n"
	"add	hl,de\n"
	"ex	de,hl\n"

	"ld	b,0\n"
	"ld	c,#%1\n"
//	"ld	hl,_bullets_active+MAX_BULLETS-1\n"
	"ld	hl,_bullets_active+%0\n"
"sprite_bullets_loop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	z,sprite_bullets_skip\n"
	"push	hl\n"

	"ld	hl,_bullets_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"dec	a\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	hl,_bullets_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	a,4*4\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	a,9\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	a,(_spr_count)\n"
	"inc	a\n"
	"ld	(_spr_count),a\n"

	"pop	hl\n"
"sprite_bullets_skip:\n"
	"dec	hl\n"
	"dec	c\n"
	"jr	nz,sprite_bullets_loop\n"

	:
	:"i" (MAX_BULLETS-1), "i"(MAX_BULLETS)
	:
);

/*	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies_active[i]){
			msx_set_sprite(spr_count , enemies_x[i] , enemies_y[i] , 8, 8);
			spr_count++;
		}
//		else
//			msx_set_sprite(SPR_ENEMIES + i, 0, 208, 0, 0);
	}*/

__asm__ volatile(
	//";ld	a,(_spr_count)\n"
	//";ld	l,a\n"
	//";ld	h,0\n"

	//";add	hl,hl\n"
	//";add	hl,hl\n"

	//";ld	de,_spr_chr\n"
	//";add	hl,de\n"
	//";ex	de,hl\n"

	"ld	b,0\n"
	"ld	c,#%1\n"
//	"ld	hl,_enemies_active+MAX_ENEMIES-1\n"
	"ld	hl,_enemies_active+%0\n"
"sprite_enemies_loop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	z,sprite_enemies_skip\n"
	"push	hl\n"

	"ld	hl,_enemies_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"dec	a\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	hl,_enemies_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	a,8*4\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	a,8\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	a,(_spr_count)\n"
	"inc	a\n"
	"ld	(_spr_count),a\n"

	"pop	hl\n"
"sprite_enemies_skip:\n"
	"dec	hl\n"
	"dec	c\n"
	"jr	nz,sprite_enemies_loop\n"

	:
	:"i"(MAX_ENEMIES-1), "i"(MAX_ENEMIES)
	:
);


/*	for (i = 0; i < MAX_e_bullets; i++) {
		if (e_bullets_active[i]){
			msx_set_sprite(spr_count, e_bullets_x[i] / DIV, e_bullets_y[i] / DIV, 12, 8);
			spr_count++;
		}
//		else
//			msx_set_sprite(SPR_e_bullets + i, 0, 208, 0, 0);
	}*/
__asm__ volatile(
	//";ld	a,(_spr_count)\n"
	//";ld	l,a\n"
	//";ld	h,0\n"

	//";add	hl,hl\n"
	//";add	hl,hl\n"

	//";ld	de,_spr_chr\n"
	//";add	hl,de\n"
	//";ex	de,hl\n"

	"ld	b,0\n"
	"ld	c,#%1\n"
	"ld	hl,_e_bullets_active+%0\n"
"sprite_e_bullets_loop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	z,sprite_e_bullets_skip\n"
	"push	hl\n"

	"ld	hl,_e_bullets_y-1\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"dec	a\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	hl,_e_bullets_x-1\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	a,12*4\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	a,8\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	a,(_spr_count)\n"
	"inc	a\n"
	"ld	(_spr_count),a\n"

	"pop	hl\n"
"sprite_e_bullets_skip:\n"
	"dec	hl\n"
	"dec	c\n"
	"jr	nz,sprite_e_bullets_loop\n"

	:
	: "i"(MAX_e_bullets-1), "i"(MAX_e_bullets)
	:
);


	for(opt_idx = 0; opt_idx < MAX_Option; ++opt_idx){
		if(Option_active[opt_idx] == False)
			continue;
		msx_set_sprite(spr_count ,(Option_x[opt_idx]),(Option_y[opt_idx]),16,2);
		spr_count++;
	}

	for(item_idx = 0; item_idx < MAX_ChainItem; ++item_idx){
		if(ChainItem_active[item_idx] == False)
			continue;
		msx_set_sprite(spr_count,(ChainItem_x[item_idx]), ChainItem_y[item_idx],20,10);
		spr_count++;
	}

	for(item_idx = 0; item_idx < MAX_OptionItem; ++item_idx){
		if(OptionItem_active[item_idx] == False)
			continue;
		msx_set_sprite(spr_count,(OptionItem_x[item_idx]), OptionItem_y[item_idx],24,5);
		spr_count++;
	}

	for(item_idx = 0; item_idx < MAX_ShieldItem; ++item_idx){
		if(ShieldItem_active[item_idx] == False)
			continue;
		msx_set_sprite(spr_count,(ShieldItem_x[item_idx]), ShieldItem_y[item_idx],28, 7);
		spr_count++;
	}

	for(item_idx = 0; item_idx < MAX_BombItem; ++item_idx){
		if(BombItem_active[item_idx] == False)
			continue;
		msx_set_sprite(spr_count,(BombItem_x[item_idx]), BombItem_y[item_idx],32,6);
		spr_count++;
	}

/*	for(p_idx = 0; p_idx < MAX_Particle; ++p_idx){
		if(Particle_active[p_idx] == False)
			continue;
		msx_set_sprite(spr_count,(Particle_x[p_idx]), Particle_y[p_idx], 36, 10);
		spr_count++;
	}*/
__asm__ volatile(
	"ld	a,(_spr_count)\n"

	"ld	l,a\n"
	"ld	h,0\n"

	"add	hl,hl\n"
	"add	hl,hl\n"

	"ld	de,_spr_chr\n"
	"add	hl,de\n"
	"ex	de,hl\n"

	"ld	b,0\n"
	"ld	c,#%1\n"
	"ld	hl,_Particle_active+%0\n"
"sprite_Particle_loop:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	z,sprite_Particle_skip\n"

	"ld	a,(_spr_count)\n"
	"cp	32\n"
	"jr	nc,sprite_Particle_end\n"
	"inc	a\n"
	"ld	(_spr_count),a\n"

	"push	hl\n"

	"ld	hl,_Particle_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"dec	a\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	hl,_Particle_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	a,36*4\n"
	"ld	(de),a\n"
	"inc	de\n"

	"ld	a,10\n"
	"ld	(de),a\n"
	"inc	de\n"


	"pop	hl\n"
"sprite_Particle_skip:\n"
	"dec	hl\n"
	"dec	c\n"
	"jr	nz,sprite_Particle_loop\n"

"sprite_Particle_end:\n"
	:
	:"i" (MAX_Particle-1),"i" (MAX_Particle)
	:
);


	}else{
		spr_count = 31;

	msx_set_sprite(spr_count, player_x , player_y , (shield_active) ? 44 : 40, 7);
	spr_count--;
	msx_set_sprite(spr_count, player_x , player_y , 0, 15);
	spr_count--;

/*	for (i = 0; i < MAX_BULLETS; i++) {
		if (bullets_active[i]){
			msx_set_sprite(spr_count, bullets_x[i] , bullets_y[i] , 4, 9);
			spr_count--;
		}
//		else
//			msx_set_sprite(SPR_BULLETS + i, 0, 208, 0, 0);
	}*/
__asm__ volatile(
	"ld	a,(_spr_count)\n"
	"inc	a\n"
	"ld	l,a\n"
	"ld	h,0\n"

	"add	hl,hl\n"
	"add	hl,hl\n"

	"ld	de,_spr_chr\n"
	"add	hl,de\n"
	"ex	de,hl\n"
	"dec	de\n"

	"ld	b,0\n"
	"ld	c,#%1\n"
	"ld	hl,_bullets_active+%0\n"
"sprite_bullets_loop2:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	z,sprite_bullets_skip2\n"
	"push	hl\n"

	"ld	a,9\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	a,4*4\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	hl,_bullets_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	hl,_bullets_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"dec	a\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	a,(_spr_count)\n"
	"dec	a\n"
	"ld	(_spr_count),a\n"

	"pop	hl\n"
"sprite_bullets_skip2:\n"
	"dec	hl\n"
	"dec	c\n"
	"jr	nz,sprite_bullets_loop2\n"

	:
	: "i" (MAX_BULLETS-1), "i" (MAX_BULLETS)
	:
);

/*	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies_active[i]){
			msx_set_sprite(spr_count , enemies_x[i] , enemies_y[i] , 8, 8);
			spr_count--;
		}
//		else
//			msx_set_sprite(SPR_ENEMIES + i, 0, 208, 0, 0);
	}*/

__asm__ volatile(
	"ld	b,0\n"
	"ld	c,#%1\n"
	"ld	hl,_enemies_active+%0\n"
"sprite_enemies_loop2:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	z,sprite_enemies_skip2\n"
	"push	hl\n"

	"ld	a,8\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	a,8*4\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	hl,_enemies_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	hl,_enemies_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"dec	a\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	a,(_spr_count)\n"
	"dec	a\n"
	"ld	(_spr_count),a\n"

	"pop	hl\n"
"sprite_enemies_skip2:\n"
	"dec	hl\n"
	"dec	c\n"
	"jr	nz,sprite_enemies_loop2\n"

	:
	: "i" (MAX_ENEMIES-1), "i"(MAX_ENEMIES)
	:

);


/*	for (i = 0; i < MAX_e_bullets; i++) {
		if (e_bullets_active[i]){
			msx_set_sprite(spr_count, e_bullets_x[i] / DIV, e_bullets_y[i] / DIV, 12, 8);
			spr_count--;
		}
//		else
//			msx_set_sprite(SPR_e_bullets + i, 0, 208, 0, 0);
	}*/

__asm__ volatile(
	"ld	b,0\n"
	"ld	c,#%1\n"
	"ld	hl,_e_bullets_active+%0\n"
"sprite_e_bullets_loop2:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	z,sprite_e_bullets_skip2\n"
	"push	hl\n"

	"ld	a,8\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	a,12*4\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	hl,_e_bullets_x-1\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	hl,_e_bullets_y-1\n"
	"add	hl,bc\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"dec	a\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	a,(_spr_count)\n"
	"dec	a\n"
	"ld	(_spr_count),a\n"

	"pop	hl\n"
"sprite_e_bullets_skip2:\n"
	"dec	hl\n"
	"dec	c\n"
	"jr	nz,sprite_e_bullets_loop2\n"

	:
	: "i" (MAX_e_bullets-1), "i"(MAX_e_bullets)
	:
);



	for(opt_idx = 0; opt_idx < MAX_Option; ++opt_idx){
		if(Option_active[opt_idx] == False)
			continue;
		msx_set_sprite(spr_count ,(Option_x[opt_idx]),(Option_y[opt_idx]),16,2);
		spr_count--;
	}

	for(item_idx = 0; item_idx < MAX_ChainItem; ++item_idx){
		if(ChainItem_active[item_idx] == False)
			continue;
		msx_set_sprite(spr_count,(ChainItem_x[item_idx]), ChainItem_y[item_idx],20,10);
		spr_count--;
	}

	for(item_idx = 0; item_idx < MAX_OptionItem; ++item_idx){
		if(OptionItem_active[item_idx] == False)
			continue;
		msx_set_sprite(spr_count,(OptionItem_x[item_idx]), OptionItem_y[item_idx],24,5);
		spr_count--;
	}

	for(item_idx = 0; item_idx < MAX_ShieldItem; ++item_idx){
		if(ShieldItem_active[item_idx] == False)
			continue;
		msx_set_sprite(spr_count,(ShieldItem_x[item_idx]), ShieldItem_y[item_idx],28, 7);
		spr_count--;
	}

	for(item_idx = 0; item_idx < MAX_BombItem; ++item_idx){
		if(BombItem_active[item_idx] == False)
			continue;
		msx_set_sprite(spr_count,(BombItem_x[item_idx]), BombItem_y[item_idx],32,6);
		spr_count--;
	}

/*	for(p_idx = 0; p_idx < MAX_Particle; ++p_idx){
		if(Particle_active[p_idx] == False)
			continue;
		msx_set_sprite(spr_count,(Particle_x[p_idx]), Particle_y[p_idx], 36, 10);
		spr_count--;
	}*/
__asm__ volatile(
	"ld	a,(_spr_count)\n"

	"inc	a\n"
	"ld	l,a\n"
	"ld	h,0\n"

	"add	hl,hl\n"
	"add	hl,hl\n"

	"ld	de,_spr_chr\n"
	"add	hl,de\n"
	"ex	de,hl\n"
	"dec	de\n"

	"ld	b,0\n"
	"ld	c,#%1\n"
	"ld	hl,_Particle_active+%0\n"
"sprite_Particle_loop2:\n"
	"ld	a,(hl)\n"
	"or	a\n"
	"jr	z,sprite_Particle_skip2\n"

	"ld	a,(_spr_count)\n"
	"cp	32\n"
	"jr	nc,sprite_Particle_end2\n"
	"dec	a\n"
	"ld	(_spr_count),a\n"

	"push	hl\n"

	"ld	a,10\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	a,36*4\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	hl,_Particle_x-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"ld	(de),a\n"
	"dec	de\n"

	"ld	hl,_Particle_y-1\n"
	"add	hl,bc\n"
	"ld	a,(hl)\n"
	"dec	a\n"
	"ld	(de),a\n"
	"dec	de\n"

	"pop	hl\n"
"sprite_Particle_skip2:\n"
	"dec	hl\n"
	"dec	c\n"
	"jr	nz,sprite_Particle_loop2\n"

"sprite_Particle_end2:\n"

	:
	: "i" (MAX_Particle-1),"i" (MAX_Particle)
	:
);


	}

//	spr_count = add;
//	if(spr_count > 0)
//		msx_set_sprite(spr_count - 1, 0, 209+1, 0, 0);



//	if((add > 0) && (spr_count < 32))
//		msx_set_sprite(spr_count, 0, 208+1, 0, 0);

//	msx_wait_vsync();
	if(spr_count>32)
		set_sprite_all(0, 32);
	else if(add > 0){
		set_sprite_all(0, spr_count);
/*		if(spr_count > max_spr_count){
			max_spr_count = spr_count;
			msx_print_num(7, 2, max_spr_count, 2);
		}*/
#ifdef DEBUG_FPS
		msx_print_num(22, 22, spr_count, 2);
#endif
	}else
		set_sprite_all(spr_count+1, 32);

	add = -add;
//	++add;
//	"add += 32;
//	"add %= 32;
}


// ====================== UI表示関数 ======================
void draw_ui(void) {
	if (all_display_flag){
		msx_print(0, 1, "BOMB: ");
		msx_print(16, 0, "COUNT: ");

		all_display_flag = False;		// 更新済みフラグを下ろす
		score_display_flag = True;
//		high_score_display_flag = True;
		time_display_flag = True;
		bomb_display_flag = True;
	}

	// SCORE
	if(score_display_flag){
		msx_print_num_l(7, 0, score, 7);
		score_display_flag = False;
		if(score >= high_score){
			if((score % 10) == 0){
				msx_print(0, 0, "HIGH: ");
			}
		}else{
			msx_print(0, 0, "SCORE:");
		}
//		msx_print_num(7, 2, option_cooldown, 2);
	}

	// HIGH
/*	if(high_score_display_flag){
		msx_print_num_l(7, 2, high_score, 6);
		high_score_display_flag = False;
	}
*/
	if(bomb_display_flag){
		msx_print_num(7, 1, bomb_stock, 1);
		bomb_display_flag = False;
	}

	if(chain_display_flag){
		if(chain_count > 0){
			msx_print(16, 1, "CHAIN: ");
			msx_print_num(7+16, 1, chain_count, 3);
		}else{
			msx_print(16, 1, "      ");
			msx_print(7+16, 1, "   ");
		}
		chain_display_flag = False;
	}

	// TIME
	if(time_display_flag){
		msx_print_num_l(7+16, 0, play_time / COUNT1S, 7);
		time_display_flag = False;
	}

}
// ====================== ゲームオーバー表示 ======================
void draw_game_over(void) {
	if(score >= high_score)
		high_score = score;

	msx_print(11, 12, "GAME OVER");		   // 中央より少し左
	msx_print(7, 15, "HIGH SCORE");
	msx_print_num_l(7+11, 15, high_score, 7);

/*	if (score > high_score) {
		high_score = score;
		msx_print(10, 14, "NEW HIGH SCORE!");
	}*/

	msx_print(7, 18, "PRESS A  TO RESTART");
}

const unsigned char data[8]={1,2,4,8,16,32,64,128};
unsigned char bg_counter = 0, bg_counter2 = 31*8, bg_counter3 = 0, bg_counter4 = 31*8 + 4;

void draw_bg(void)
{
	if(bg_counter >= 8){
		bg_counter =0;
		VPOKE(0, bg_counter2);
		if(bg_counter2 > 0){
			bg_counter2 -= 8;
		}else{
			bg_counter2 = 31*8;
		}
	}
	VPOKE(data[bg_counter], bg_counter2);
	++bg_counter;


	if(bg_counter3 >= 8){
		bg_counter3 =0;
		VPOKE(0, bg_counter4);
		if(bg_counter4 > 4){
			bg_counter4 -= 8;
		}else{
			bg_counter4 = 31*8 + 4;
		}
	}
	VPOKE(data[bg_counter3], bg_counter4);
	bg_counter3 += 2;
}


// ====================== main ======================

/* screenのBIOS切り替え */
static void set_screenmode(unsigned char mode)
{
	register unsigned char rd asm("d") = mode;

__asm__ volatile(
	"push	de\n"
	"push	ix\n"

	"ld	a,(0xfcc1)	; exptbl\n"
	"ld	b,a\n"
	"ld	c,0\n"
	"push	bc\n"
	"pop	iy\n"
	"ld ix,0x005f	; CHGMOD(MAINROM)\n"
	"ld	a,%0\n"

	//";call	0x001c	; CALSLT\n"
	"call	0x005f	; CHGMOD(MAINROM)\n"

	"pop	ix\n"
	"pop	de\n"

	:	/* 値が返るレジスタ変数 */
	:"r"(rd)	/* 引数として使われる変数 */
	: "bc", "iy","hl"	/* 破壊されるレジスタ */
);
}

#define forclr ((unsigned char *)0xf3e9)
#define bakclr ((unsigned char *)0xf3ea)
#define bdrclr ((unsigned char *)0xf3eb)

void msx_set_color(unsigned char forc, unsigned char bak, unsigned char bdr)
{
	*forclr = forc;
	*bakclr = bak;
	*bdrclr = bdr;
}

int main(void) {
	int i;
	bgmmode = checkbgm();

	VDP_readadr = *((unsigned char *)0x0006);//read_mainrom(0x0006);
	VDP_writeadr = *((unsigned char *)0x0007);//read_mainrom(0x0007);
	for(i = 0; i < 4; i++){
		vdp_readport[i] = VDP_readadr + i;
		vdp_writeport[i] = VDP_writeadr + i;
	}
	*clicksw = 0;

	msx_set_color(15, 1, 0);
//	msx_screen(1);
	set_screenmode(1);
	vdp_set_sprite_mode(sprite_large);
	msx_initpsg();

	define_fonts();
	define_sprites();

/*	for(i = 0; i < (256*4); ++i){
		sin_table[i]  =  sin(i * 0.12) * 55;
	}*/
	for(j = 0; j < 32; ++j)
		for(i = 0; i < 8; ++i)
			VPOKE(0, i+j*8);

	high_score = 5000;

	reset();

#ifdef DEBUG
	do {
#else
	for(;;){
#endif
		if(game_over < 1){
			play_time++;
			if (play_time % COUNT1S == 0) time_display_flag = True;
			update_input();
			update_bullets();
			spawn_enemy();
			update_enemies();
			update_e_bullets();
			check_items();
			check_collisions();
			udpate_Particle();
		}
		if(game_over < 2){
			draw_ui();
			draw_sprites();
		}
		if (game_over == 1) {
			reset_int();
			stopbgm();
			draw_game_over();
//			for(;;){
			game_over = 2;
		}
		if(game_over == 2){
			if(!(keyscan() & KEY_A)){
				game_over = 3;
//					break;
			}
			msx_wait_vsync();
		}else if(game_over == 3){
//			for(;;){
			if((keyscan() & KEY_A)){
//					break;
//			}
				reset();
//			msx_wait_vsync();
				game_over = 0;
//				set_sprite_all(0, 32);
				playbgm();
				set_int();
			}
			else{
				msx_wait_vsync();
			}
		}
		draw_bg();
#ifdef DEBUG
	}while((get_key(7) & 0x04));
#else
	}
#endif
	reset_int();
	stopbgm();
	return 0;
}