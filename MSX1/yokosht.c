#include <msx.h>
#include "msxcom.h"

#include <stdlib.h>
#include <stdio.h>

// ゲーム定数
#define PLAYER_SPEED 4 * DIV
#define BULLET_SPEED 8 * DIV
#define ENEMY_SPEED  3 * DIV

#define MAX_BULLETS  6
#define MAX_ENEMIES  8
#define MAX_e_bullets 6

#define MAX_Particle 0
#define MAX_Option 2
#define MAX_ChainItem 4
#define MAX_OptionItem 1
#define MAX_ShieldItem 2
#define MAX_BombItem 2

#define width     256
#define height     192

// スプライト番号
/*#define SPR_PLAYER   0
#define SPR_BULLETS  1
#define SPR_ENEMIES  7
#define SPR_e_bullets 15*/

#define DIV 256

// グローバル変数
unsigned short player_x = 40 * DIV;
unsigned short player_y = 80 * DIV;

//struct Bullet  { unsigned short x, y, active; };
/*struct Enemy {
    unsigned short x, y;
    unsigned char active;
    unsigned char hp;
    unsigned char shoot_timer;
};*/
struct EBullet { unsigned short x, y; signed short dx, dy; unsigned char active; };

//struct Bullet 

unsigned short bullets_x[MAX_BULLETS];
unsigned short bullets_y[MAX_BULLETS];
signed short bullets_dx[MAX_BULLETS];
signed short bullets_dy[MAX_BULLETS];
unsigned char bullets_active[MAX_BULLETS];

//struct Enemy  
unsigned short enemies_x[MAX_ENEMIES];
unsigned short enemies_y[MAX_ENEMIES];
unsigned char enemies_active[MAX_ENEMIES];
unsigned char enemies_hp[MAX_ENEMIES];
unsigned char enemies_shoot_timer[MAX_ENEMIES];

//struct EBullet 
unsigned short e_bullets_x[MAX_e_bullets];
unsigned short e_bullets_y[MAX_e_bullets];
signed short e_bullets_dx[MAX_e_bullets];
signed short e_bullets_dy[MAX_e_bullets];
unsigned char e_bullets_active[MAX_e_bullets];

unsigned int score = 0;
unsigned int high_score = 0;
unsigned int play_time = 0;
unsigned char shoot_timer = 0;
unsigned char enemy_spawn_timer = 0;
unsigned char game_over = 0;
unsigned char ui_update_flag = 1;
unsigned char score_update_flag = 1;
unsigned char high_score_update_flag = 1;
unsigned char time_update_flag = 1;

struct Star { unsigned char x, y, speed; };
struct Star stars[40];

// ====================== スプライト定義 ======================
void define_sprites(void) {
    // 自機 (16x16)
    static unsigned char player_pat[] = {
		0x00,0x00,0xe0,0xf8,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xf8,0xe0,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0xe0,0xf8,0xff,
		0xff,0xf8,0xe0,0x00,0x00,0x00,0x00,0x00,
    };
    msx_set_sprite_pattern(0, player_pat);

    // 自機弾
    static unsigned char bullet_pat[] = {
        0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    msx_set_sprite_pattern(4, bullet_pat);   // 呼び出し側で4の倍数扱い

    // 敵
    static unsigned char enemy_pat[] = {
		0xff,0xff,0xff,0xff,0xf0,0xf0,0xf0,0xf0,
		0xf0,0xf0,0xf0,0xf0,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0x0f,0x0f,0x0f,0x0f,
		0x0f,0x0f,0x0f,0x0f,0xff,0xff,0xff,0xff,
    };
    msx_set_sprite_pattern(8, enemy_pat);

    // 敵弾
    static unsigned char ebullet_pat[] = {
        0xf0,0xf0,0xf0,0xf0,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    msx_set_sprite_pattern(12, ebullet_pat);
}

// ====================== 初期化 ======================
void init_game(void) {
    unsigned char i;

    player_x = 40 * DIV;
    player_y = 80 * DIV;
    score = 0;
    play_time = 0;
    shoot_timer = 0;
    enemy_spawn_timer = 0;
    game_over = 0;
    ui_update_flag = 1;

    for (i = 0; i < MAX_BULLETS; i++) bullets_active[i] = 0;
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemies_active[i] = 0;
        enemies_hp[i] = 1;
        enemies_shoot_timer[i] = 0;
    }
    for (i = 0; i < MAX_e_bullets; i++) e_bullets_active[i] = 0;

    for (i = 0; i < 40; i++) {
        stars[i].x = (rand() % width) * DIV;
        stars[i].y = (rand() % height) * DIV;
        stars[i].speed = 1 + (rand() % 3);
    }

    msx_cls();
}

void fire_aimed_bullet(unsigned short ex, unsigned short ey)
{
    unsigned char i;
    long deltax, deltay;
    unsigned short dist;
    signed short dx = 0;
    signed short dy = 0;

    deltax = (long)player_x + 12 * DIV - (long)ex;
    deltay = (long)player_y + 8 * DIV  - (long)ey;

    // 距離近似（横・縦方向を少し強調して斜めばかりを減らす）
//    dist = (unsigned short)(abs(deltax) * 1 + abs(deltay));   // 横方向を重視

	if(abs(deltax) >= abs(deltay))
		dist = abs(deltax);
	else
		dist = abs(deltay);

//    if (dist < 12) dist = 12;
    if (dist == 0) dist = 1;

    // 符号を完全に分離して計算
//    if (deltax > 0) {
        dx = (signed short)((deltax * 5 * DIV) / dist);
//    } else if (deltax < 0) {
//        dx = -(signed short)(((-deltax) * 5 * DIV) / dist);
//    }

//    if (deltay > 0) {
        dy = (signed short)((deltay * 5 * DIV) / dist);
//    } else if (deltay < 0) {
//        dy = -(signed short)(((-deltay) * 5 * DIV) / dist);
//    }

    // 最大速度を4に制限（前回の希望通り）
    if (dx > 4 * DIV) dx = 4 * DIV;
    if (dx < -4 * DIV) dx = -4 * DIV;
    if (dy > 4 * DIV) dy = 4 * DIV;
    if (dy < -4 * DIV) dy = -4 * DIV;

    // 発射
    for (i = 0; i < MAX_e_bullets; i++) {
        if (!e_bullets_active[i]) {
            e_bullets_x[i] = ex + 8 * DIV;
            e_bullets_y[i] = ey + 8 * DIV;
            e_bullets_dx[i] = dx;
            e_bullets_dy[i] = dy;
            e_bullets_active[i] = 1;
            break;
        }
    }
}

void fire_aimed_bullet2(unsigned short ex, unsigned short ey)
{
    unsigned char i;
    long dx_raw, dy_raw;
    unsigned short adx, ady; // 絶対値を保持する変数
    unsigned char dir = 0;

    // プレイヤーの中心（+12, +8）と敵の座標の差分
    dx_raw = (long)player_x + 12 * DIV - (long)ex;
    dy_raw = (long)player_y + 8 *DIV - (long)ey;

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
}
// ====================== update_enemies ======================
void update_enemies(void) {
    unsigned char i;
    unsigned char base_interval = 50;   // デフォルト

    if (play_time < 120)        base_interval = 55;
    else if (play_time < 360)   base_interval = 47;
    else if (play_time < 720)   base_interval = 40;
    else                        base_interval = 35;

    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies_active[i]) continue;

        if (enemies_x[i] <= (ENEMY_SPEED + 8 * DIV)) {
            enemies_active[i] = 0;
            continue;
        }
        enemies_x[i] -= ENEMY_SPEED;

        enemies_shoot_timer[i]++;

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
}

// ここにあなたの update_input, update_bullets, spawn_enemy, update_e_bullets, check_collisions をそのまま貼り付けてください
void update_input(void) {
    // ジョイスティック + キーボード対応（簡易）
    if (joystick(0) & JOY_LEFT)  if (player_x > PLAYER_SPEED) player_x -= PLAYER_SPEED;
    if (joystick(0) & JOY_RIGHT) if (player_x < (width - 24) * DIV) player_x += PLAYER_SPEED;
    if (joystick(0) & JOY_UP)    if (player_y > PLAYER_SPEED) player_y -= PLAYER_SPEED;
    if (joystick(0) & JOY_DOWN)  if (player_y < (height - 24) * DIV) player_y += PLAYER_SPEED;

    // スペース or ボタンで射撃
    if ((joyfire(0) || msx_get_key(0x20)) && shoot_timer == 0) {  // 適当キーコード例
        for (unsigned char i = 0; i < MAX_BULLETS; i++) {
            if (!bullets_active[i]) {
                bullets_x[i] = player_x + 20 * DIV;
                bullets_y[i] = player_y + 8 * DIV;
                bullets_active[i] = 1;
                // 簡易サウンド（PSG）
                msx_sound(0, 0x00); // 適当に音を鳴らす（後で調整）
                break;
            }
        }
        shoot_timer = 8;  // 連射間隔
    }
    if (shoot_timer > 0) shoot_timer--;
}

void update_bullets(void) {
    for (unsigned char i = 0; i < MAX_BULLETS; i++) {
        if (bullets_active[i]) {
            if (bullets_x[i] >= (width * DIV - BULLET_SPEED)) bullets_active[i] = 0;
            else
                bullets_x[i] += BULLET_SPEED;
        }
    }
}

void spawn_enemy(void) {
    enemy_spawn_timer++;
    if (enemy_spawn_timer > (40 - (play_time / 60))) {  // 時間で少し難易度アップ
        for (unsigned char i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies_active[i]) {
                enemies_x[i] = (width-1) * DIV;
                enemies_y[i] = (30 + (rand() % (height - 60))) * DIV;
                enemies_active[i] = 1;
                enemies_hp[i] = 1;
	            enemies_shoot_timer[i] = 0;          // タイマーリセット
                break;
            }
        }
        enemy_spawn_timer = 0;
    }
}


void update_e_bullets(void) {
    unsigned char i;

    for (i = 0; i < MAX_e_bullets; i++) {
        if (!e_bullets_active[i]) continue;

        // 加算前に判定（オーバーフローする前に非アクティブ化）
        if ( (short)(e_bullets_x[i] / DIV) > (width - 1) - (short)(e_bullets_dx[i] / DIV) ||      // 右に出る予定
            (short)(e_bullets_x[i] / DIV) < - (short)(e_bullets_dx[i] / DIV) ||                // 左に出る予定
            (short)(e_bullets_y[i] / DIV) > (height) - (short)(e_bullets_dy[i] / DIV) ||      // 下に出る予定
            (short)(e_bullets_y[i] / DIV) < - (short)(e_bullets_dy[i] / DIV) ) {                // 上に出る予定
            e_bullets_active[i] = 0;
        } else {
            e_bullets_x[i] += e_bullets_dx[i];
            e_bullets_y[i] += e_bullets_dy[i];
        }
    }
}
void check_collisions(void) {
    unsigned char b, e, eb;

    // 自機弾 vs 敵（既存のまま）
    for (b = 0; b < MAX_BULLETS; b++) {
        if (!bullets_active[b]) continue;
        for (e = 0; e < MAX_ENEMIES; e++) {
            if (!enemies_active[e]) continue;

            if (bullets_x[b] < enemies_x[e] + 20 * DIV &&
                bullets_x[b] + 8 * DIV > enemies_x[e] &&
                bullets_y[b] < enemies_y[e] + 16 * DIV &&
                bullets_y[b] + 6 * DIV > enemies_y[e]) {

                bullets_active[b] = 0;
                if (--enemies_hp[e] == 0) {
                    enemies_active[e] = 0;
                    score += 100;
					score_update_flag = 1;
//					ui_update_flag = 1;
	                // 簡易サウンド（PSG）
	                msx_sound(1, 0); // 適当に音を鳴らす（後で調整）
                }
                break;
            }
        }
    }

    // 敵弾 vs 自機（既存のまま）
    for (eb = 0; eb < MAX_e_bullets; eb++) {
        if (!e_bullets_active[eb]) continue;
        if (player_x < e_bullets_x[eb] + 4 * DIV &&
            player_x + 24 * DIV > e_bullets_x[eb] &&
            player_y < e_bullets_y[eb] + 4 * DIV &&
            player_y + 14 * DIV > e_bullets_y[eb]) {

            e_bullets_active[eb] = 0;
            game_over = 1;
        }
    }

    // ★ 新規追加：自機 vs 敵機 当たり判定
    for (e = 0; e < MAX_ENEMIES; e++) {
        if (!enemies_active[e]) continue;

        // 自機と敵の矩形が重なっているかチェック
        if (player_x + 4 * DIV < enemies_x[e] + 16 * DIV &&   // 自機左 < 敵右
            player_x + 24 * DIV > enemies_x[e] &&       // 自機右 > 敵左
            player_y + 4 * DIV < enemies_y[e] + 8 * DIV &&   // 自機上 < 敵下
            player_y + 14 * DIV > enemies_y[e]) {       // 自機下 > 敵上

            game_over = 1;
            // ここに爆発エフェクトやサウンドを後で追加可能
            break;
        }
    }
}
// ====================== 描画 ======================
void draw_sprites(void) {
    unsigned char i, j;

    msx_set_sprite(0, player_x / DIV, player_y / DIV, 0, 10);

	j = 1;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (bullets_active[i]){
            msx_set_sprite(j, bullets_x[i] / DIV, bullets_y[i] / DIV, 4, 9);
			++j;
		}
//        else
//            msx_set_sprite(SPR_BULLETS + i, 0, 208, 0, 0);
    }

    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies_active[i]){
            msx_set_sprite(j , enemies_x[i] / DIV, enemies_y[i] / DIV, 8, 8);
			++j;
		}
//        else
//            msx_set_sprite(SPR_ENEMIES + i, 0, 208, 0, 0);
    }

    for (i = 0; i < MAX_e_bullets; i++) {
        if (e_bullets_active[i]){
            msx_set_sprite(j, e_bullets_x[i] / DIV, e_bullets_y[i] / DIV, 12, 12);
			++j;
		}
//        else
//            msx_set_sprite(SPR_e_bullets + i, 0, 208, 0, 0);
    }
    msx_set_sprite(j, 0, 208+1, 0, 0);
}


// ====================== UI表示関数 ======================
void draw_ui(void) {
    if (ui_update_flag){
	    msx_print(1, 1, "SCORE");
	    msx_print(1, 2, "HIGH");
	    msx_print(19, 1, "COUNT");
	    ui_update_flag = 0;        // 更新済みフラグを下ろす
		score_update_flag = 1;
		high_score_update_flag = 1;
		time_update_flag = 1;
	}

    // SCORE
	if(score_update_flag){
	    msx_print_num(7, 1, score, 6);
		score_update_flag = 0;
	}

    // HIGH
	if(high_score_update_flag){
	    msx_print_num(7, 2, high_score, 6);
		high_score_update_flag = 0;
	}

    // TIME
	if(time_update_flag){
	    msx_print_num(25, 1, play_time / 60, 4);
		time_update_flag = 0;
	}

}
// ====================== ゲームオーバー表示 ======================
void draw_game_over(void) {
    msx_print(11, 10, "GAME OVER");           // 中央より少し左
    msx_print(9, 12, "FINAL SCORE");
    msx_print_num(21, 12, score, 6);

    if (score > high_score) {
        high_score = score;
        msx_print(10, 14, "NEW HIGH SCORE!");
    }

    msx_print(8, 17, "PRESS SPACE TO RESTART");
}
// ====================== main ======================
void main(void) {
    msx_set_color(15, 1, 0);
    msx_screen(1);
    msx_initpsg();

    define_sprites();
    init_game();

    while (1) {
        if (game_over) {
            draw_game_over();
	        while(msx_get_key(0x20));
	        while(!msx_get_key(0x20));
			init_game();
            msx_wait_vsync();
        }

        play_time++;

        if (play_time % 60 == 0) time_update_flag = 1;

        update_input();
        update_bullets();
        spawn_enemy();
        update_enemies();
        update_e_bullets();
        check_collisions();

        draw_ui();

        msx_wait_vsync();
        draw_sprites();
    }
}