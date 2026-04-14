#include <stdlib.h>
#include <math.h>

#define randint(min, max) (rand() % (max-min) + min)
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

void reset(void);
void update(void);
void draw(void);
void use_bomb(void);

enum {
	player_speed = 2,

	True = 1,
	False = 0,

	MAX_BULLETS = 10,
	MAX_ENEMIES = 10,
	MAX_e_bullets = 30,
	MAX_Particle = 20,
	MAX_Option = 2,
	MAX_ChainItem = 10,
	MAX_OptionItem = 2,
	MAX_ShieldItem = 2,
	MAX_BombItem = 2,

	DIV = 256 //1024
};

short player_x = 30;
short player_y = 80;

short bullets_x[MAX_BULLETS];
short bullets_y[MAX_BULLETS];
unsigned char bullets_active[MAX_BULLETS];

short enemies_x[MAX_ENEMIES];
short enemies_y[MAX_ENEMIES];
short enemies_type[MAX_ENEMIES];
short enemies_count[MAX_ENEMIES];
short enemies_count2[MAX_ENEMIES];
short enemies_count_hp[MAX_ENEMIES];
unsigned char enemies_count_flag[MAX_ENEMIES];
unsigned char enemies_active[MAX_ENEMIES];

long e_bullets_x[MAX_e_bullets];
long e_bullets_y[MAX_e_bullets];
short e_bullets_xx[MAX_e_bullets];
short e_bullets_yy[MAX_e_bullets];
unsigned char e_bullets_active[MAX_e_bullets];

short Particle_x[MAX_Particle];
short Particle_y[MAX_Particle];
short Particle_vx[MAX_Particle];
short Particle_vy[MAX_Particle];
short Particle_life[MAX_Particle];
short Particle_color[MAX_Particle];
unsigned char Particle_active[MAX_Particle];

short Option_offset_y[MAX_Option];
short Option_x[MAX_Option];
short Option_y[MAX_Option];
unsigned char Option_active[MAX_Option];

short ChainItem_x[MAX_ChainItem];
short ChainItem_y[MAX_ChainItem];
short ChainItem_timer[MAX_ChainItem];
unsigned char ChainItem_active[MAX_ChainItem];

short OptionItem_x[MAX_OptionItem];
short OptionItem_y[MAX_OptionItem];
short OptionItem_timer[MAX_OptionItem];
unsigned char OptionItem_active[MAX_OptionItem];

short ShieldItem_x[MAX_ShieldItem];
short ShieldItem_y[MAX_ShieldItem];
short ShieldItem_timer[MAX_ShieldItem];
unsigned char ShieldItem_active[MAX_ShieldItem];

short BombItem_x[MAX_BombItem];
short BombItem_y[MAX_BombItem];
short BombItem_timer[MAX_BombItem];
unsigned char BombItem_active[MAX_BombItem];

short sin_table[256 * 4];

unsigned char b_idx, eb_idx, opt_idx, item_idx, e_idx;

unsigned char bomb_stock = 0;
unsigned char shield_active = False;

unsigned char chain_count = 0;
unsigned char chain_timer = 0;
unsigned char option_cooldown = 10;
unsigned long enemy_spawn_timer = 0;
unsigned short kill_count = 0;
unsigned short shoot_timer = 0;

unsigned long play_time = 0;		  // 経過時間（フレーム）
unsigned char game_over = 0; //False;

unsigned short old_i;
unsigned char k;
unsigned char enemy_type;
unsigned char rand_num;
short dist_x, dist_y;
short base_y;
unsigned char hp;
short difficulty;
short sx,sy,dx,dy,ex,ey,ph_x,ph_y,ph_w,ph_h;

short enemy_bullet_speed;

short shoot_interval;
short dist;
short speed;
short base_speed;
short direction_factor;
short offset;
short i;
short idx;

unsigned char keycode;
unsigned char key_b_flag;

int score = 0, hiscore = 5000, combo = 0;

unsigned char score_display_flag;
unsigned char bomb_display_flag;
unsigned char time_display_flag;
unsigned char all_display_flag;
unsigned char chain_display_flag;

unsigned short rgb(unsigned char r, unsigned char g, unsigned char b)
{
	return (g << 11) + (r << 6) + (b << 1);
}

unsigned char p_idx;

void Particles_append(short x, short y)
{
	for (p_idx = 0; p_idx < MAX_Particle; ++p_idx){
		if(Particle_active[p_idx] == True)
			continue;
		Particle_x[p_idx] = x;
		Particle_y[p_idx] = y;
		Particle_vx[p_idx] = randint(0,9)-4;
		Particle_vy[p_idx] = randint(0,9)-4;
		Particle_life[p_idx] = 35 + randint(0, 20);
		Particle_active[p_idx] = True;
		break;
	}
}

void main2(void)
{
	hiscore = 5000;
	reset();

	for(i = 0; i < (256*4); ++i){
		sin_table[i]  =  sin(i * 0.12) * 55;
	}
	old_i = 0;
	init_star();

	for(;;){
		if(_iocs_b_sftsns() & 1)
			break;
		if (_iocs_bitsns(0) & 2)
			break;
		update();
		if(seflag){
			se();
			seflag = 0;
		}

		draw();

		wait_vsync();
		set_sprite_all();
		bg_roll();
	}
}

void reset(void)
{
	cls();
	all_display_flag = True;
	chain_display_flag = False;
	seflag = 0;

	player_x = 30;
	player_y = 80;

	key_b_flag = False;

	for(b_idx = 0; b_idx<  MAX_BULLETS; ++b_idx)
		bullets_active[b_idx] = False;

	for(eb_idx = 0; eb_idx < MAX_e_bullets; ++eb_idx)
		e_bullets_active[eb_idx] = False;

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

	for(e_idx = 0; e_idx < MAX_ENEMIES; ++e_idx)
		enemies_active[e_idx] = 0;
	enemy_spawn_timer = 0;
	kill_count = 0;
	shoot_timer = 0;
	score = 0;
	play_time = 0;		  // 経過時間（フレーム）
	game_over = 0;
}

void update(void)
{
	keycode = keyscan();

	if(game_over){
		switch(game_over){
			case 1:
				if (!(keycode & KEY_A))
					game_over = 2;
				break;

			case 2:
				if ((keycode & KEY_A))
					reset();
				break;
		}
		return;
	}

	play_time += 1;
	if(!(play_time % COUNT1S))
		time_display_flag = True;

	// 移動
	if (keycode & KEY_UP1) player_y -= player_speed;
	if (keycode & KEY_DOWN1) player_y += player_speed;
	if (keycode & KEY_LEFT1) player_x -= player_speed;
	if (keycode & KEY_RIGHT1) player_x += player_speed;
	player_x = max(16, min(player_x, width - 20 + 16));
	player_y = max(16, min(player_y, height - 16 + 16));

	// 射撃
	shoot_timer += 1;
	if ((keycode & KEY_A) && (shoot_timer > 8)){
		for(b_idx = 0; b_idx < MAX_BULLETS; ++b_idx){
			if(bullets_active[b_idx] == True)
				continue;
			bullets_x[b_idx] = (player_x + 16);
			bullets_y[b_idx] = (player_y + 6);
			bullets_active[b_idx] = True;
			break;
		}

		for(opt_idx = 0; opt_idx < MAX_Option; ++opt_idx){
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
		shoot_timer = 0;
	}

	// ボム使用
	if((keycode & KEY_B) && (bomb_stock > 0)){
		if(key_b_flag == False)
			use_bomb();
		key_b_flag = True;
	}else
		key_b_flag = False;

	for(b_idx = 0; b_idx < MAX_BULLETS; ++b_idx){
		if(bullets_active[b_idx] == False)
			continue;
		bullets_x[b_idx] += 6;

		if(bullets_x[b_idx] > width+16)
			bullets_active[b_idx] = False;
	}

	// 敵出現
	enemy_spawn_timer += 1;
	if(enemy_spawn_timer > max(18, 50 - score / 250)){
		rand_num = randint(0,100);
		if(rand_num < 60) enemy_type = 0;
		else if(rand_num < 85) enemy_type = 1;
		else enemy_type = 2;

		base_y = randint(30, height - 40);
		if(enemy_type == 0) hp = 1; else hp = 3;
		for(e_idx = 0; e_idx < MAX_ENEMIES; ++e_idx){
			if(enemies_active[e_idx] == True)
				continue;
			enemies_x[e_idx] = width;
			enemies_y[e_idx] = base_y;
			enemies_type[e_idx] = enemy_type;
			enemies_count[e_idx] = 0;
			enemies_count2[e_idx] = base_y;
			enemies_count_hp[e_idx] = hp;
			enemies_count_flag[e_idx] = False;
			enemies_active[e_idx] = True;
			break;
		}
		enemy_spawn_timer = 0;
	}

	// 敵更新
	for(e_idx = 0; e_idx < MAX_ENEMIES; ++e_idx) {
		if(enemies_active[e_idx] == False)
			continue;
		enemies_count[e_idx] += 1;

		if (enemies_type[e_idx] == 0)		// 通常敵
			enemies_x[e_idx] -= 2;

		else if(enemies_type[e_idx] == 1){	  // ヘリザコ - 勢いよく突っ込む
			dist_x = enemies_x[e_idx] - player_x;
				if(enemies_count[e_idx] < 24){	// 1段階：超急接近
				enemies_x[e_idx] -= 6;
				enemies_y[e_idx] += ((player_y + 8 - enemies_y[e_idx]) / 8);
			}else if(enemies_count[e_idx] < 49)	// 2段階：短くホバリング
				enemies_x[e_idx] -= 0;
			else							// 3段階：右へ全力逃走
				enemies_x[e_idx] += 6;
		}

		else if(enemies_type[e_idx] == 2){	  // サインカーブ
			enemies_x[e_idx] -= 2;
			enemies_y[e_idx] = enemies_count2[e_idx] + sin_table[enemies_count[e_idx]];
		}
		// 難易度計算（時間経過）
		difficulty = (min(1, play_time / (180*COUNT1S)));
		enemy_bullet_speed = 2 + difficulty;
		shoot_interval = (82 - difficulty * 36);

		// 射撃
		if ((!enemies_count_flag[e_idx] && (18 <= enemies_count[e_idx]) && (enemies_count[e_idx] <= 40))){
			sx = enemies_x[e_idx] + 8;
			sy = enemies_y[e_idx] + 8;
			dx = player_x + 8 - sx;
			dy = player_y + 8 - sy;

			if(abs(dx) > abs(dy))
				dist = abs(dx);
			else
				dist = abs(dy);

			if (dist == 0) dist = 1;
			speed = enemy_bullet_speed;
			dx = (dx * DIV * speed/dist);
			dy = (dy * DIV * speed/dist);
			dx = max(-3 * DIV, dx);
			dx = min(dx, 4 * DIV);
			dy = max(-4 * DIV, dy);
			dy = min(dy, 4 * DIV);

			for(eb_idx = 0; eb_idx <  MAX_e_bullets; ++eb_idx){
				if(e_bullets_active[eb_idx] == True)
					continue;
				e_bullets_x[eb_idx] = sx * DIV;
				e_bullets_y[eb_idx] = sy * DIV;
				e_bullets_xx[eb_idx] = dx;
				e_bullets_yy[eb_idx] = dy;
				e_bullets_active[eb_idx] = True;
				break;
			}

			enemies_count_flag[e_idx] = True;
		}
		else if(enemies_count_flag[e_idx] && ((enemies_count[e_idx] % shoot_interval) == 0) && (randint(0,100) < 60)){
			sx = enemies_x[e_idx] + 8;
			sy = enemies_y[e_idx] + 8;
			dx = player_x + 8 - sx;
			dy = player_y + 8 - sy;

			if(abs(dx) > abs(dy))
				dist = abs(dx);
			else
				dist = abs(dy);

			if(dist == 0) dist = 1;
			base_speed = enemy_bullet_speed;
			direction_factor = dx / dist;
			speed = base_speed;
			dx = (DIV * dx*speed/dist);
			dy = (DIV * dy*speed/dist);
			dx = max(-3 * DIV, dx);
			dx = min(dx, 4 * DIV);
			dy = max(-4 * DIV, dy);
			dy = min(dy, 4 * DIV);

			for(eb_idx = 0; eb_idx < MAX_e_bullets; ++eb_idx){
				if(e_bullets_active[eb_idx] == True)
					continue;
				e_bullets_x[eb_idx] = sx * DIV;
				e_bullets_y[eb_idx] = sy * DIV;
				e_bullets_xx[eb_idx] = dx;
				e_bullets_yy[eb_idx] = dy;
				e_bullets_active[eb_idx] = True;
				break;
			}
		}

		if((enemies_x[e_idx] < -40+16) || (enemies_x[e_idx] > (width+16)) ){
			enemies_active[e_idx] = False;
		}
	}

	// 敵弾更新
	for(eb_idx = 0; eb_idx < MAX_e_bullets; ++eb_idx){
		if(e_bullets_active[eb_idx] == False)
			continue;
		e_bullets_x[eb_idx] += e_bullets_xx[eb_idx];
		e_bullets_y[eb_idx] += e_bullets_yy[eb_idx];

		if(((-10+16)*DIV >= e_bullets_x[eb_idx]) ||
			(e_bullets_x[eb_idx] >= (width + 10 + 16)*DIV) ||
			((-10+16)*DIV >= e_bullets_y[eb_idx]) ||
			(e_bullets_y[eb_idx] >= (height + 10 + 16)*DIV)){
			e_bullets_active[eb_idx] = False;
			continue;
		}
	}

	// 自機弾 vs 敵
	for(b_idx = 0; b_idx < MAX_BULLETS; ++b_idx){
		if(bullets_active[b_idx]== False)
			continue;

		for(e_idx = 0; e_idx < MAX_ENEMIES; ++e_idx){
			if(enemies_active[e_idx] == False)
				continue;

			if (bullets_x[b_idx] < enemies_x[e_idx] + 16){
				if(bullets_x[b_idx] + 8 > enemies_x[e_idx]){
					if(bullets_y[b_idx] < enemies_y[e_idx] + 16){
						if(bullets_y[b_idx] + 4 > enemies_y[e_idx]){

							enemies_count_hp[e_idx] -= 1;
							bullets_active[b_idx] = False;

							if(enemies_count_hp[e_idx] <= 0){
								ex = enemies_x[e_idx];
								ey = enemies_y[e_idx];
								for(i = 0; i < 4; ++i)
									Particles_append(ex, ey);

								enemies_active[e_idx] = False;

								score += 100;
								score_display_flag = True;
								kill_count += 1;
								seflag = 1;

								if(randint(0,100) < 40){

									for(item_idx = 0; item_idx < MAX_ChainItem; ++item_idx){
										if(ChainItem_active[item_idx] == True)
											continue;
										ChainItem_x[item_idx] = ex;
										ChainItem_y[item_idx] = ey;
										ChainItem_timer[item_idx] = COUNT1S * 4;
										ChainItem_active[item_idx] = True;
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
								for (i = 0; i < 4; ++i)
									Particles_append(enemies_x[e_idx] + 8, enemies_y[e_idx] + 8);
							}
							break;
						}
					}
				}
			}
		}
	}

	// アイテム処理
	for(item_idx = 0; item_idx < MAX_ChainItem; ++item_idx){
		if(ChainItem_active[item_idx]  == False)
			continue;
		ChainItem_x[item_idx] -= 2;
		ChainItem_timer[item_idx] -= 1;
		if((abs(player_x + 8 - ChainItem_x[item_idx]) < 20) && (abs(player_y + 8 - ChainItem_y[item_idx]) < 20)){
			chain_count += 1;
			chain_timer = COUNT1S * 4;
			score += 100 * chain_count;
			score_display_flag = True;
			seflag = 4;
			ChainItem_active[item_idx] = False;
			chain_display_flag = True;
			continue;
		}
		if((ChainItem_x[item_idx] < -20+16) || (ChainItem_timer[item_idx] <= 0)){
			chain_count = 0;
			ChainItem_active[item_idx] = False;
			chain_display_flag = True;
		}
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
		OptionItem_x[item_idx] -= 1;
		OptionItem_timer[item_idx] -= 1;

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
			seflag = 4;
			continue;
		}
		if((OptionItem_x[item_idx] < -20+16) || (OptionItem_timer[item_idx] <= 0))
			OptionItem_active[item_idx] = False;
	}
	for(item_idx =0; item_idx < MAX_ShieldItem; ++item_idx){
		if(ShieldItem_active[item_idx] == False)
			continue;
		ShieldItem_x[item_idx] -= 2;
		ShieldItem_timer[item_idx] -= 1;


		if((abs(player_x + 8 - ShieldItem_x[item_idx]) < 22) && (abs(player_y + 8 - ShieldItem_y[item_idx]) < 22)){
			shield_active = True;
			ShieldItem_active[item_idx] = False;
			seflag = 4;
			continue;
		}
		if((ShieldItem_x[item_idx] < -20+16) || (ShieldItem_timer[item_idx] <= 0))
			ShieldItem_active[item_idx] = False;
	}
	for(item_idx = 0; item_idx < MAX_BombItem; ++item_idx){
		if(BombItem_active[item_idx] == False)
			continue;
		BombItem_x[item_idx] -= 2;
		BombItem_timer[item_idx] -= 1;

		if((abs(player_x + 8 - BombItem_x[item_idx]) < 22) && (abs(player_y + 8 - BombItem_y[item_idx]) < 22)){
			bomb_stock = min(3, bomb_stock + 1);
			bomb_display_flag = True;
			BombItem_active[item_idx] = False;
			seflag = 4;
			continue;
		}
		if((BombItem_x[item_idx] < -20+16) || (BombItem_timer[item_idx] <= 0))
			BombItem_active[item_idx] = False;
	}
	for(opt_idx = 0; opt_idx < MAX_Option; ++opt_idx){
		Option_x[opt_idx] += (((player_x + 8) - Option_x[opt_idx]) / 4);
		Option_y[opt_idx] += (((player_y + Option_offset_y[opt_idx]) - Option_y[opt_idx]) / 4);
	}

	// 当たり判定
	ph_x = player_x;
	ph_y = player_y + 3;
	ph_w = 16;
	ph_h = 10;

	for(eb_idx = 0; eb_idx < MAX_e_bullets; ++eb_idx){
		if(e_bullets_active[eb_idx] == False)
			continue;

		if((ph_x < (e_bullets_x[eb_idx]/DIV) + 4)){
			if(ph_x + ph_w > (e_bullets_x[eb_idx]/DIV)) {
				if(ph_y < ((e_bullets_y[eb_idx]/DIV) + 4)){
					if(ph_y + ph_h > (e_bullets_y[eb_idx]/DIV)){
						if(shield_active){
							shield_active = False;
							for(i = 0; i < 4; ++i)
								Particles_append(player_x + 8, player_y + 8);
						}else
							game_over = 1;

						e_bullets_active[eb_idx] = False;
						break;
					}
				}
			}
		}
	}
	for(e_idx = 0; e_idx < MAX_ENEMIES; ++e_idx){
		if(enemies_active[e_idx] == False)
			continue;

		if (ph_x < (enemies_x[e_idx] + 16)){
			if((ph_x + ph_w) > enemies_x[e_idx]){
				if(ph_y < (enemies_y[e_idx] + 16)){
					if((ph_y + ph_h) > enemies_y[e_idx]){
						if(shield_active){
							shield_active = False;
							for (i = 0; i < 4; ++i)
								Particles_append(player_x + 8, player_y + 8);
						}else
							game_over = 1;
						enemies_active[e_idx] = False;
						break;
					}
				}
			}
		}
	}

	// パーティクル更新
	for(p_idx = 0; p_idx < MAX_Particle; ++p_idx){
		if(Particle_active[p_idx] == False)
			continue;
		Particle_x[p_idx] += Particle_vx[p_idx];
		Particle_y[p_idx] += Particle_vy[p_idx];
		Particle_life[p_idx] -= 1;

		if(Particle_life[p_idx] <= 0)
			Particle_active[p_idx] = False;
	}

	if((score > hiscore)){
		if(game_over == 1){
			hiscore = score;
		}
		all_display_flag  = True;
	}
}

void use_bomb(void)
{
	if(bomb_stock <= 0) return;
	bomb_stock -= 1;
	bomb_display_flag = True;
	for(i = 0; i < 30; ++i){
		Particles_append(randint(20, 236), randint(20, 172));
	}
	for(e_idx = 0; e_idx <  MAX_ENEMIES; ++e_idx)
		enemies_active[e_idx] = False;
	for(eb_idx = 0; eb_idx <  MAX_e_bullets; ++eb_idx)
		e_bullets_active[eb_idx] = False;

	score += 200;
	score_display_flag = True;
}

void draw(void)
{
	spr_count = 0;
	sprite_set(spr_count,player_x,player_y,1,1, 0);
	spr_count += 1;

	if(shield_active){
		sprite_set(spr_count, player_x, player_y,6,1, 0);
		spr_count += 1;
	}
	for(b_idx = 0; b_idx <  MAX_BULLETS; ++b_idx){
		if (bullets_active[b_idx] == False)
			continue;
		sprite_set(spr_count,bullets_x[b_idx], bullets_y[b_idx],4,1, 0);
		spr_count += 1;
	}

	for(e_idx = 0; e_idx <  MAX_ENEMIES; ++e_idx){
		if(enemies_active[e_idx] == False)
			continue;
		sprite_set(spr_count,max(enemies_x[e_idx], 0), max(enemies_y[e_idx], 0),2,1, 0);
		spr_count += 1;
	}

	for(eb_idx = 0; eb_idx < MAX_e_bullets; ++eb_idx){
		if(e_bullets_active[eb_idx] == False)
			continue;

		sprite_set(spr_count, (max(e_bullets_x[eb_idx]/DIV, 0)), (max(e_bullets_y[eb_idx]/DIV, 0)),0,1, 0);
		spr_count += 1;
	}

	for(opt_idx = 0; opt_idx < MAX_Option; ++opt_idx){
		if(Option_active[opt_idx] == False)
			continue;
		sprite_set(spr_count ,max(Option_x[opt_idx], 0), max(Option_y[opt_idx], 0),10,1, 0);
		spr_count += 1;
	}

	for(item_idx = 0; item_idx < MAX_ChainItem; ++item_idx){
		if(ChainItem_active[item_idx] == False)
			continue;
		sprite_set(spr_count,max(ChainItem_x[item_idx], 0), ChainItem_y[item_idx],3,1, 0);
		spr_count += 1;
	}

	for(item_idx = 0; item_idx < MAX_OptionItem; ++item_idx){
		if(OptionItem_active[item_idx] == False)
			continue;
		sprite_set(spr_count,max(OptionItem_x[item_idx], 0), OptionItem_y[item_idx],8,1, 0);
		spr_count += 1;
	}

	for(item_idx = 0; item_idx < MAX_ShieldItem; ++item_idx){
		if(ShieldItem_active[item_idx] == False)
			continue;
		sprite_set(spr_count,max(ShieldItem_x[item_idx], 0), ShieldItem_y[item_idx],7,1, 0);
		spr_count += 1;
	}

	for(item_idx = 0; item_idx < MAX_BombItem; ++item_idx){
		if(BombItem_active[item_idx] == False)
			continue;
		sprite_set(spr_count,max(BombItem_x[item_idx], 0), BombItem_y[item_idx],9,1, 0);
		spr_count += 1;
	}

	for(p_idx = 0; p_idx < MAX_Particle; ++p_idx){
		if(Particle_active[p_idx] == False)
			continue;
		sprite_set(spr_count,max(Particle_x[p_idx], 0), Particle_y[p_idx], 5, 1, 0);
		spr_count += 1;
	}

	// UI
	if(all_display_flag){
		put_strings(SCREEN2, 0, 1, "BOMB: ", CHRPAL_NO);
		put_strings(SCREEN2, 16, 0, "COUNT: ", CHRPAL_NO);

		all_display_flag = False;
		score_display_flag = True;
		bomb_display_flag = True;
		time_display_flag = True;
	}

	if(score_display_flag){
		put_numd(score, 7);
		put_strings(SCREEN2, 7, 0, str_temp, CHRPAL_NO);
		if(score >= hiscore){
			if((score % 10) == 0){
				put_strings(SCREEN2, 0, 0, "HIGH: ", CHRPAL_NO);
			}
		}else{
			put_strings(SCREEN2, 0, 0, "SCORE:", CHRPAL_NO);
		}
		score_display_flag = False;
	}

	if(bomb_display_flag){
		put_numd(bomb_stock, 1);
		put_strings(SCREEN2, 7, 1, str_temp, CHRPAL_NO);
		bomb_display_flag = False;
	}

	// タイムカウント表示
	if(time_display_flag){
		put_numd(play_time / COUNT1S, 7);
		put_strings(SCREEN2, 7+16, 0, str_temp, CHRPAL_NO);
		time_display_flag = False;
	}
	if(chain_display_flag){
		if(chain_count > 0){
			put_strings(SCREEN2, 16, 1, "CHAIN", CHRPAL_NO);
			put_numd(chain_count, 3);
			put_strings(SCREEN2, 7+16, 1, str_temp, CHRPAL_NO);
		}else{
			put_strings(SCREEN2, 16, 1, "     ", CHRPAL_NO);
			put_strings(SCREEN2, 7+16, 1, "   ", CHRPAL_NO);
		}
		chain_display_flag = False;
	}

	if(game_over == 1){
		put_strings(SCREEN2, 11, 12, "GAME OVER", CHRPAL_NO);
		put_strings(SCREEN2, 7, 15, "HIGH SCORE: ", CHRPAL_NO);
		put_numd(hiscore, 7);
		put_strings(SCREEN2, 7+11, 15, str_temp, CHRPAL_NO);
		put_strings(SCREEN2, 7, 18, "PRESS A TO RESTART", CHRPAL_NO);
	}
}
