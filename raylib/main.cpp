// Direct2DShooter.cpp
#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#include <vector>
#include <cmath>
#include <algorithm>    // std::clamp のために追加
#include <string>
#include <cstdlib>
#include <ctime>
#include <cwchar>
#include <cmath>

#define SCREEN_WIDTH  (256*2)
#define SCREEN_HEIGHT (192*2)
#define COUNT1S 60.0f

#define FONT_SIZE 32
float X_SCALE = 2.0f;
float Y_SCALE = 2.0f;

Texture2D chrTex;
Texture2D fontTex;
Sound laserSound;
Sound explosionSound;

Music bgm;
float scale;

float m_deltaTime;

// 構造体
struct Bullet { float x, y; };
struct Enemy { float x, y, shootTimer, nextShootTime; int type; float count, count2; int count_hp; bool count_flag; };
struct eBullets { float x, y, vx=0.0f, vy=0.0f; };

struct Option {
    float offset_y;     // 自機からの相対Y（-25 or +25 など）
    float x, y;         // 現在の位置
//    float angle;        // 回転角度（滑らかに回す用）
};

struct Item { float x, y, timer; int type; };

struct ChainItem {
    float x, y;
    float timer;
};

struct Star { float x, y, baseSpeed, speed, size;};

struct Particle {
    float x, y;
    float vx, vy;
    float life;           // 残りフレーム
    int color_index;    // 0?4で星ブラシと同じ色を使う
    int type;           // 0=通常破片、1=大きな爆発など（後で拡張用）
};

std::wstring m_exeDir;


class ShooterGame {
public:
    ShooterGame();
    ~ShooterGame();

    void GameUpdate();
    void OnRender();
	void InitStars();
	int gameOver = 0;

private:
    void StarUpdate();
    void CheckCollisions();
    void ResetGame();
    void ClampPlayer();

	bool put_sprite(float x, float y, int pat);
    void put_strings(float x, float y, wchar_t* str, int mode);
    void put_strings(float x, float y, wchar_t *str);
    void put_strings_num(float x, float y, wchar_t* str, int num, int digit, int mode);
    void put_strings_num(float x, float y, wchar_t* str, int num, int digit);

    void CreateParticles(float x, float y, int count, int type); // = 0);
	void UseBomb();

    int scrollX = 0;

    const float TARGET_FRAME_TIME = 1.0f / COUNT1S; // 60FPS目標
    float m_accumulator = 0.0f;

    float m_gameTime = 0.0f;           // 経過時間（秒）
    float m_enemySpawnRate = 1.0f;     // 現在の敵生成頻度（小さいほど頻繁）

	// 各種変数
    float playerX = COUNT1S, playerY = 160.0f;
    std::vector<Bullet> playerBullets;
    std::vector<eBullets> enemyBullets;
    std::vector<Particle> Particles;
    std::vector<Option> Options;
    std::vector<Item> Items;
    std::vector<ChainItem> chain_items;   // クラス内に追加

    std::vector<Enemy> enemies;
    std::vector<Star> stars;

    std::vector<Particle> particles;

	int bomb_stock = 0;
	bool bomb_active = false;
	float bomb_timer = 0.0f;
	const float BOMB_DURATION = 0.0f;
    bool key_b_flag = false;

    bool shield_active = false;
    float shield_timer = 0.0f;     // 残り時間
    const float SHIELD_DURATION = 8.0f;   // シールド持続時間（秒）

	int chain_count = 0;
    float chain_timer = 0.0f;     // floatに変更推奨
    const float CHAIN_TIME_LIMIT = 3.5f;   // チェイン持続時間（秒）

	int option_cooldown = 10;
	int enemy_spawn_timer = 0;
	int kill_count = 0;
	int shoot_timer = 0;

	int score = 0, lives = 3, high_score=5000;

//	int play_time = 0;		  // 経過時間（フレーム）

	int sx,sy,dx,dy,ex,ey,ph_x,ph_y,ph_w,ph_h;

	int enemy_bullet_speed;

	int shoot_interval;
	int dist;
	int direction_factor;
	int offset;

    const int MAX_OPTIONS = 2;   // 最大オプション数

	bool FPS_flag = false;
    bool easy_mode = false;
};

ShooterGame::ShooterGame() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

ShooterGame::~ShooterGame() {
}

void ShooterGame::InitStars() {
    stars.clear();
    for (int i = 0; i < 80; ++i) {
        Star s;
        s.x = static_cast<float>(rand() % SCREEN_WIDTH);
        s.y = static_cast<float>(rand() % SCREEN_HEIGHT);
		s.baseSpeed = 0.5f + static_cast<float>(rand() % 100) / 30.0f;
		s.speed = s.baseSpeed;   // もし個別速度も欲しい場合
        s.size = (s.speed > 2.0f) ? 2.0f : 1.0f;
        stars.push_back(s);
    }
}

// ゲームリセット
void ShooterGame::ResetGame() {
    playerX = 60.f; playerY = 160.0f; //SCREEN_HEIGHT / 2 - 32;
    score = 0;
    if (easy_mode == true)
        lives = 3;
    else
        lives = 1;
    gameOver = 0;
    playerBullets.clear();
    enemyBullets.clear();
    enemies.clear();
    Options.clear();
    Items.clear();
    scrollX = 0;
    m_gameTime = 0;
    particles.clear();
    option_cooldown = 10;
	shield_active = false;
    bomb_active = false;
    bomb_stock = 0;
    key_b_flag = false;
    chain_items.clear();
    chain_count = 0;

//	InitStars();

	StopMusicStream(bgm);
	PlayMusicStream(bgm);
}


void ShooterGame::StarUpdate() {
    // 星移動
    for (auto& s : stars) {
		s.x -= s.baseSpeed * m_deltaTime * COUNT1S;   // これでFPSが60の時に元の速度と同じになる
        if (s.x < 0) {
            s.x = SCREEN_WIDTH; //800;
            s.y = static_cast<float>(rand() % SCREEN_HEIGHT); //600);
        }
    }
}

void ShooterGame::ClampPlayer() {
    if (playerX < 0) playerX = 0;
    if (playerY < 0) playerY = 0;
    if (playerX > SCREEN_WIDTH-40) playerX = SCREEN_WIDTH-40;
    if (playerY > SCREEN_HEIGHT-32) playerY = SCREEN_HEIGHT-32;
}

// ゲーム進行
void ShooterGame::GameUpdate() {
	int gamepad = 0;

    if (gameOver == 1){
//		if
//		(!((IsGamepadAvailable(gamepad) && (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) || IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) )
//         || (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_Z) || IsKeyDown(KEY_R) || IsKeyDown(KEY_X) || IsKeyDown(KEY_B))) )
		{
			gameOver = 2;
		}
	}
	if (gameOver == 2){
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_R) || (IsGamepadAvailable(gamepad) && IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))) {
			easy_mode = false; ResetGame();
        }else if (IsKeyPressed(KEY_X) || IsKeyPressed(KEY_B) || (IsGamepadAvailable(gamepad) && IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))) {
			easy_mode = true; ResetGame();
		}
	}

    StarUpdate();
    if (gameOver)
		return;

    m_gameTime += m_deltaTime;

	float moveSpeed = 4.0f * COUNT1S * m_deltaTime;
	float enemySpeed = 4.0f * COUNT1S * m_deltaTime;
	float enemySpeed2 = 5.0f * COUNT1S * m_deltaTime;


    // 1. ゲームパッドが接続されているかチェック
	float axisX = 0;
	float axisY = 0;
    if (IsGamepadAvailable(gamepad))
    {
        // 2. アナログスティック（左スティック）の入力を取得
        // 戻り値は -1.0f から 1.0f の間
        axisX = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X);
        axisY = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y);
	}

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) || (axisY < -0.2f) || (IsGamepadAvailable(gamepad) && IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_UP))) playerY -= moveSpeed;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S) || (axisY > 0.2f) || (IsGamepadAvailable(gamepad) && IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_DOWN)))  playerY += moveSpeed;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A) || (axisX < -0.2f) || (IsGamepadAvailable(gamepad) && IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_LEFT)))  playerX -= moveSpeed;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) || (axisX > 0.2f) || (IsGamepadAvailable(gamepad) && IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))) playerX += moveSpeed;

    ClampPlayer();

    // オプション更新
    for (auto& opt : Options) {
//        opt.angle += 0.08f * COUNT1S * m_deltaTime;   // 回転速度

        // 滑らかに追従
        opt.x += ((playerX + 16) - opt.x) / 4 * m_deltaTime * COUNT1S;
        opt.y += ((playerY + opt.offset_y) - opt.y) / 4 * m_deltaTime * COUNT1S;
//		float t = 1.0f - pow(1.0f - 0.25f, m_deltaTime * COUNT1S);
//		opt.x = std::lerp(opt.x, playerX + 16, t);
//		opt.y = std::lerp(opt.y, playerY + opt.offset_y, t);
    }

// 射撃クールタイムも時間ベースに
    static float shootTimer = 0.0f;
    shootTimer += m_deltaTime;
    if ((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_Z) || (IsGamepadAvailable(gamepad) && IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))) && shootTimer >= 8 / COUNT1S) {
        playerBullets.push_back({ playerX + 32, playerY + 12 });

        // オプションからも発射
        for (const auto& opt : Options) {
            playerBullets.push_back({ opt.x + 8, opt.y + 12 });
        }
        shootTimer = 0.0f;
    }

	// GameUpdate()内がおすすめ
	if ((IsKeyPressed(KEY_X) || IsKeyPressed(KEY_B) || (IsGamepadAvailable(gamepad) && IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))) && bomb_stock > 0 && !bomb_active) {
        if (key_b_flag == false)
            UseBomb();
        key_b_flag = true;
    }
    else{
        key_b_flag = false;
 	}

// === 敵生成処理（スコアベース）===
    static float enemySpawnTimer = 0.0f;
    enemySpawnTimer += m_deltaTime;

    // 元のロジックをdeltaTimeに変換
    float baseInterval = 50.0f - (score / 250.0f);   // scoreが増えるほど短く
    float spawnInterval = std::max(18.0f / COUNT1S, baseInterval / COUNT1S);  // フレーム→秒に変換

    if (enemySpawnTimer >= spawnInterval) {

        int type, rand_num;

		rand_num = rand() % 100;
		if(rand_num < 60) type = 0;
		else if(rand_num < 85) type = 1;
		else type = 2;

        float count = rand() % (30 * 2 + SCREEN_HEIGHT - 40 * 2) - 30 * 2;
        enemies.push_back({ SCREEN_WIDTH+0.0f, 32.0f + (rand() % (SCREEN_HEIGHT-32-32-32)), 0.0f, 5.0f / COUNT1S, type, 0.0f, count,
            (type == 0)? 1:3, false});
		enemySpawnTimer = 0.0f;
    }

    // 敵移動 + 敵弾発射
    for (auto& e : enemies) {
        e.count += m_deltaTime * COUNT1S;

        if (e.type == 0)		// 通常敵
            e.x -= enemySpeed;
		else if(e.type == 1){	  // ヘリザコ - 勢いよく突っ込む
//			static float dist_x = e.x - player_x;
            if (e.count < 24) {	// 1段階：超急接近
                 e.x -= 6 * 2 * m_deltaTime * COUNT1S;
                e.y += ((playerY + 8 - e.y) / 8) / 2  * m_deltaTime * COUNT1S;
            }
            else if (e.count < 49)	// 2段階：短くホバリング
                e.x -= 0;
            else							// 3段階：右へ全力逃走
                e.x += 6 * 2 * m_deltaTime * COUNT1S;
		}

		else if(e.type == 2){	  // サインカーブ
			e.x -= enemySpeed;
            e.y = (e.count2 + sinf(e.count * 0.12) * 55 * 2);
		}

        // 敵弾発射処理
        e.shootTimer += m_deltaTime;

        int difficulty = (std::min(1.0f, m_gameTime / (180))); // * COUNT1S)));
        float enemy_bullet_speed = (4 + difficulty * 2);
        float shoot_interval = ((82 - difficulty * 36) - 5) / COUNT1S;


        if (e.shootTimer >= e.nextShootTime) {

            float dx = playerX - e.x;
	        float dy = playerY - e.y;

//            dx -= 4.0f;

			float dist;
			if(abs(dx) > abs(dy))
				dist = abs(dx);
			else
				dist = abs(dy);

			if (dist == 0) dist = 1;


	        // ベクトルの長さを計算
//	        float length = sqrtf(dx * dx + dy * dy);
//	        if (length > 0.001f) {   // 0除算防止
//	            dx /= length;   // 正規化
//	            dy /= length;


	            // 弾を発射
                float bulletSpeed = enemy_bullet_speed;

				dx = (dx * bulletSpeed/dist);
				dy = (dy * bulletSpeed/dist);
				dx = std::max(-3*2.0f, dx);
				dx = std::min(dx, 4*2.0f);
				dy = std::max(-4*2.0f, dy);
				dy = std::min(dy, 4*2.0f);

	            enemyBullets.push_back({
	                e.x + 16, 
	                e.y + 16,
	                dx, // * bulletSpeed - 1.0f*1,   // vx
	                dy, // * bulletSpeed     // vy
	            });
//	        }

            // 次回の発射間隔を設定
            e.nextShootTime = shoot_interval;

            e.shootTimer = 0.0f;
            e.count++;
        }
    }

    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if ((it->x < -32) || (it->x > SCREEN_WIDTH)){// || (it->y < 32) || (it->y > SCREEN_HEIGHT)) {
            it = enemies.erase(it);
        }
        else {
            ++it;
        }
    }

    // 自機弾移動
    for (auto it = playerBullets.begin(); it != playerBullets.end(); ) {
        it->x += 12.0f *COUNT1S * m_deltaTime;
        if ((it->x < -32) || (it->x > SCREEN_WIDTH) || (it->y < -32)|| (it->y > SCREEN_HEIGHT))  it = playerBullets.erase(it);
        else ++it;
    }

    // 敵弾移動&画面範囲外判定
	for (auto it = enemyBullets.begin(); it != enemyBullets.end(); ) {
	    it->x += it->vx * m_deltaTime * COUNT1S;
	    it->y += it->vy * m_deltaTime * COUNT1S;

	    if ((it->x < -32) || (it->x > SCREEN_WIDTH) || (it->y < 32) || (it->y > SCREEN_HEIGHT)) {
		    it = enemyBullets.erase(it);
	    } else {
	        ++it;
	    }
	}

    CheckCollisions();


    // パーティクル更新
    for (auto it = particles.begin(); it != particles.end(); ) {
        it->x += it->vx * m_deltaTime * COUNT1S;
        it->y += it->vy * m_deltaTime * COUNT1S;
//        it->vx *= 0.96f;      // 少し減速（空気抵抗）
//        it->vy *= 0.96f;

		float damping = pow(0.96f, m_deltaTime * COUNT1S); 
		it->vx *= damping;
		it->vy *= damping;

        it->life -=  m_deltaTime * COUNT1S;

        if (it->life <= 0) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }

    // ボム更新
    if (bomb_active) {
        bomb_timer -= m_deltaTime;
        if (bomb_timer <= 0.0f) {
            bomb_active = false;
        }
    }

    // チェインアイテム更新
    for (auto it = chain_items.begin(); it != chain_items.end(); ) {
        it->x -= 4.0f * m_deltaTime * COUNT1S;   // 左に流れる
        it->timer -= m_deltaTime;

        // 自機取得判定
        if (abs(it->x - playerX) < 44-16 && abs(it->y - playerY) < 44-16) {
            chain_count++;
            chain_timer = 240 / COUNT1S;           // チェイン持続時間リセット
            score += chain_count * 100;    // チェイン数に応じたボーナス

            it = chain_items.erase(it);
			PlaySound(laserSound);
            continue;
        }

        // 時間切れ or 画面外
        if (it->timer <= 0.0f || it->x < -20) {
            chain_count = 0;
            it = chain_items.erase(it);
        } else {
            ++it;
        }
    }

    // チェインタイマー減少
    if (chain_timer > 0.0f) {
        chain_timer -= m_deltaTime;
        if (chain_timer <= 0.0f) {
            chain_count = 0;
        }
    }
    if(gameOver && (score > high_score))
        high_score = score;

}

void ShooterGame::CheckCollisions() {
    // 自弾 vs 敵
    for (auto bit = playerBullets.begin(); bit != playerBullets.end(); ) {
        bool hit = false;
        for (auto eit = enemies.begin(); eit != enemies.end(); ) {
            if (bit->x + 16 > eit->x && bit->x < eit->x + 32 &&
                bit->y + 8> eit->y && bit->y < eit->y + 32) {

				CreateParticles(eit->x + 16, eit->y + 16, 8, 0);   // 通常爆発

                if (--eit->count_hp == 0) {

				    // オプションアイテム出現（確率20%くらい）
//				    if (rand() % 100 < 22 && Options.size() < MAX_OPTIONS) {
                    if (option_cooldown <= 0){
				        Item item;
				        item.x = eit->x;
				        item.y = eit->y;
                        item.timer = 300.0f;        // 約5秒で消える
				        item.type = 1;           // 1 = オプションアイテム
				        Items.push_back(item);
                        option_cooldown = 10;
                    }
                    else {
                        --option_cooldown;
                    }

				    // シールドアイテム出現（確率12%程度）
				    if (rand() % 100 < 12 && !shield_active) {
				        Item item;
				        item.x = eit->x;
				        item.y = eit->y;
				        item.timer = 280.0f;
				        item.type = 2;         // 2 = シールド
				        Items.push_back(item);
				    }

                    // ボムアイテム出現
                    if (rand() % 100 < 10) {        // 約10%の確率
					    Item item;
					    item.x = eit->x;
					    item.y = eit->y;
					    item.timer = 270.0f;
					    item.type = 3;              // 3 = ボム
					    Items.push_back(item);
					}
				    // === チェインアイテム出現 ===
				    if (rand() % 100 < 40) {        // 40%くらいの確率で落とす
				        ChainItem item;
				        item.x = eit->x;
				        item.y = eit->y;
				        item.timer = 240.0f;
				        chain_items.push_back(item);
				    }

                    eit = enemies.erase(eit);
					PlaySound(explosionSound);
                    score += 100;

                }
                else {
                    ++eit;
                }
                hit = true;
                break;
            }
            else ++eit;
        }
        if (hit) bit = playerBullets.erase(bit);
        else ++bit;
    }

    // 敵弾 vs 自機
    for (auto it = enemyBullets.begin(); it != enemyBullets.end(); ) {
        if (it->x > playerX && it->x + 8 < playerX + 32 &&
            it->y > playerY + 6 && it->y + 8 < playerY + 6 + 20) {

	        if (shield_active) {
	            shield_active = false;           // シールド消費
	            CreateParticles(playerX + 16, playerY + 16, 18, 1); // 大きな爆発
	        } else {
	            lives--;
	            if (lives <= 0) {
	                gameOver = 1;
					StopMusicStream(bgm);
	            }
			}
            it = enemyBullets.erase(it);
            break;
        }
        else ++it;
    }

    // 敵 vs 自機
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (it->x + 32 > playerX && it->x < playerX + 32 &&
            it->y + 32 > playerY + 6 && it->y < playerY + 6 + 32) {
            if (shield_active) {
                shield_active = false;           // シールド消費
                CreateParticles(playerX + 16, playerY + 16, 18, 1); // 大きな爆発
            }
            else {
                lives--;
                if (lives <= 0) {
                    gameOver = 1;
					StopMusicStream(bgm);
                    return;
                }
            }
            it = enemies.erase(it);
            break;
        }
        else ++it;
    }
 
    // アイテム更新
    for (auto it = Items.begin(); it != Items.end(); ) {
        if (it->type == 1) {
            it->x -= 2.0f * COUNT1S * m_deltaTime;   // 左に流れる
        }
        else if (it->type == 2) {
            it->x -= 4.0f * COUNT1S * m_deltaTime;   // 左に流れる
        }
        else if (it->type == 3) {
            it->x -= 4.0f * COUNT1S * m_deltaTime;   // 左に流れる
        }
        it->timer -= m_deltaTime;

        // 自機との当たり判定
        if (abs(it->x - playerX) < 44-16 && abs(it->y - playerY) < 44-16) {

            if (it->type == 1 && Options.size() < MAX_OPTIONS) {   // オプションアイテム
                float offset = (Options.size() == 0) ? 25.0f : -25.0f;
                Option opt;
                opt.offset_y = offset*2;
                opt.x = 0;//playerX + 20;
                opt.y = 0;//playerY + 16 + offset;
//                opt.angle = 0.0f;
                Options.push_back(opt);
            }
            else if (it->type == 2) {                    // シールド
                shield_active = true;
                shield_timer = SHIELD_DURATION;
            }
			else if (it->type == 3) {        // 3 = ボムアイテム
			    bomb_stock = std::min(3, bomb_stock + 1);
			}

			PlaySound(laserSound);

            it = Items.erase(it);
            continue;
        }

        // 画面外 or 時間切れ
        if (it->x < -40 || it->timer <= 0) {
            it = Items.erase(it);
        } else {
            ++it;
        }
    }

    // シールドタイマー更新
/*    if (shield_active) {
        shield_timer -= m_deltaTime;
        if (shield_timer <= 0.0f) {
            shield_active = false;
        }
    }*/
}



void ShooterGame::UseBomb() {
    if (bomb_stock <= 0 || bomb_active) return;

    bomb_stock--;
    bomb_active = true;
    bomb_timer = BOMB_DURATION;

    // 敵と敵弾を全滅
    enemies.clear();
    enemyBullets.clear();

    // 大量の破片を発生
    CreateParticles(playerX + 16, playerY + 16, 45, 1);   // 大爆発

    // 画面全体に破片を散らす
    for (int i = 0; i < 60; ++i) {
        float rx = rand() % SCREEN_WIDTH;
        float ry = rand() % SCREEN_HEIGHT;
        CreateParticles(rx, ry, 6, 1);
    }

    score += 200;
	PlaySound(explosionSound);
}


bool ShooterGame::put_sprite(float x, float y, int pat_no) {

	float rotation = 0.0f;

    Rectangle destRect = {x * X_SCALE, y * Y_SCALE, 32 * X_SCALE - 1, 32 * Y_SCALE - 1};
    Rectangle sourceRect = {32.0f * pat_no, 0, 32.0f, 32.0f};
	Vector2 origin = { 0, 0};

	DrawTexturePro(chrTex, sourceRect, destRect, origin, rotation, WHITE);
	return true;
}

void ShooterGame::put_strings(float x, float y, wchar_t *text, int mode) {
	int len=wcslen(text);
    if (!mode) {
		for(int i = 0; i < len; ++i){
            if (text[i] != ' ') {
                int pat_no = text[i] - '0';

				float rotation = 0.0f;

			    Rectangle destRect = {x, y, 16 * X_SCALE - 1, 16 * Y_SCALE - 1};
			    Rectangle sourceRect = {16.0f * (pat_no % 16), 16.0f * (pat_no / 16), 16.0f, 16.0f};
				Vector2 origin = { 0, 0};

				DrawTexturePro(fontTex, sourceRect, destRect, origin, rotation, WHITE);
            }

            x += 16 * X_SCALE;
		}
		return;
    }
}

void ShooterGame::put_strings(float x, float y, wchar_t* text) {
    put_strings(x, y, text, 0);
}

void ShooterGame::put_strings_num(float x, float y, wchar_t *str, int num, int digit, int mode) {
    wchar_t text[128];
    int len = wcslen(str), i = digit, j = num;
	put_strings(x, y, str);

    while (i--) {
        text[i] = j % 10 + '0';
        j /= 10;
    }
    text[digit] = '\0';
    put_strings(x+len * FONT_SIZE, y, text);
}

void ShooterGame::put_strings_num(float x, float y, wchar_t* str, int num, int digit) {
    put_strings_num(x, y, str, num, digit, 0);
}

void ShooterGame::CreateParticles(float x, float y, int count, int type = 0) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.x = x;
        p.y = y;
        p.vx = (rand() % 100 - 50) * 0.12f;   // -6.0 ~ +6.0
        p.vy = (rand() % 100 - 50) * 0.12f;
        p.life = 20.0f + (rand() % 25);
        p.color_index = rand() % 5;
        p.type = type;
        particles.push_back(p);
    }
}


// ゲーム表示
void ShooterGame::OnRender() {
    ClearBackground(BLACK);

    // 星背景
        for (const auto& s : stars) {
            DrawCircle(s.x * X_SCALE, s.y * Y_SCALE, 1.5f, WHITE);
        }


	// パーティクル描画
	if (!particles.empty()){ // && m_pTextBrush) {
	    for (const auto& p : particles) {
	        if (p.life > 0) {
                DrawCircle(p.x * X_SCALE, p.y * Y_SCALE, 1.5f*2, YELLOW);
//	            put_sprite(p.x, p.y, 5);
	        }
	    }
	}

    // チェインアイテム描画
    for (const auto& item : chain_items) {
        put_sprite(item.x, item.y, 3);   // 3番パターンにチェインアイテムの画像を入れる
    }

    for (const auto& i : Items) {
        if (i.type == 1)
            put_sprite(i.x, i.y, 8);
        else if (i.type == 2)
            put_sprite(i.x, i.y, 7);
        else if (i.type == 3)
            put_sprite(i.x, i.y, 9);
    }

    // オプション描画
    for (const auto& opt : Options) {
        put_sprite(opt.x, opt.y, 10);   // 10 = オプションのパターン番号（要調整）
    }

    // 敵弾
    for (const auto& b : enemyBullets) {
        put_sprite(b.x, b.y, 0);
    }

    // 敵
    for (const auto& e : enemies) {
		if(put_sprite(e.x, e.y, 2) == false){
        }
	}

    // 自機弾
        for (const auto& b : playerBullets) {
			put_sprite(b.x, b.y, 4);
        }

    // シールド描画
        if (shield_active){
            put_sprite(playerX, playerY, 6);
        }

        // 自機
        put_sprite(playerX, playerY, 1);

    // UI
        if (score >= high_score)
            put_strings_num(0, 0, const_cast<wchar_t*>(L"HIGH  "), score, 7);
        else
            put_strings_num(0, 0, const_cast<wchar_t *>(L"SCORE "), score, 7);
        if(easy_mode == true)
    		put_strings_num(0, 2 * FONT_SIZE, const_cast<wchar_t *>(L"LIVES "), lives, 1);

        put_strings_num(0, 1 * FONT_SIZE, const_cast<wchar_t *>(L"BOMB  "), bomb_stock , 1);

        wchar_t text[128];
        put_strings_num(16 * FONT_SIZE, 0, const_cast<wchar_t *>(L"COUNT "), m_gameTime, 7);

        if (chain_count > 0) {
            put_strings_num(16 * FONT_SIZE, 1 * FONT_SIZE, const_cast<wchar_t *>(L"CHAIN "), chain_count, 3);
        }

    if (gameOver){
		put_strings(11 * FONT_SIZE, 12 * FONT_SIZE, const_cast<wchar_t *>(L"GAME OVER"));
        put_strings_num(7 * FONT_SIZE, 15 * FONT_SIZE, const_cast<wchar_t *>(L"HIGH SCORE "), high_score, 7);

			put_strings(7 * FONT_SIZE, 18 * FONT_SIZE, const_cast<wchar_t *>(L"PRESS A TO RESTART"));
    }

}

const int screenWidth = SCREEN_WIDTH * X_SCALE;
const int screenHeight = SCREEN_HEIGHT * Y_SCALE;

int monitor;
int width;
int height;

ShooterGame game;
RenderTexture2D target;

void UpdateDrawFrame(void)
{
		UpdateMusicStream(bgm);
// Fキーが押されたらフルスクリーンを切り替える
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

		scale = std::min((float)GetScreenWidth() / screenWidth, (float)GetScreenHeight() / screenHeight);

Rectangle destRec = {
            ((float)GetScreenWidth() - ((float)screenWidth * scale)) * 0.5f,
            ((float)GetScreenHeight() - ((float)screenHeight * scale)) * 0.5f,
            (float)screenWidth * scale,
            (float)screenHeight * scale
        };

    	m_deltaTime = GetFrameTime();
		game.GameUpdate();

		BeginTextureMode(target);
		game.OnRender();
		EndTextureMode();

		BeginDrawing();
            ClearBackground(BLACK); // フルスクリーン時の「黒帯」になる部分の色

            // レンダーテクスチャは上下の座標が反転しているため、sourceのheightをマイナスにする必要があります
            Rectangle sourceRec = { 0.0f, 0.0f, (float)target.texture.width, -(float)target.texture.height };
            Vector2 origin = { 0.0f, 0.0f };

            // 計算した位置・サイズ（destRec）で綺麗に拡大描画
            DrawTexturePro(target.texture, sourceRec, destRec, origin, 0.0f, WHITE);

            // デバッグ情報（実際の現在のウィンドウサイズを表示）
//            DrawFPS(10, 10);
//            DrawText("F: Toggle Fullscreen", 10, 30, 20, GREEN);
        EndDrawing();
}


int main(void) {
	monitor = GetCurrentMonitor();
    width = GetMonitorWidth(monitor);
    height = GetMonitorHeight(monitor);

//	COUNT1S = GetMonitorRefreshRate(monitor);

	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    InitWindow(screenWidth, screenHeight, "Raylib 横スクロールシューティング");

	target = LoadRenderTexture(screenWidth, screenHeight);
	SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

//    SetTargetFPS(COUNT1S);

    chrTex = LoadTexture("yokosht.png"); // 画像がなければ後で矩形で代用
    fontTex = LoadTexture("FONTYOKO.png");

    InitAudioDevice();
    laserSound = LoadSound("laser.wav");
    explosionSound = LoadSound("explosion.wav");
	bgm  = LoadMusicStream("bgm.mp3");

	game.InitStars();
    game.gameOver = 1;


#if defined(PLATFORM_WEB)
    // Webプラウザの場合は、emscriptenにループを任せる（毎フレーム UpdateDrawFrame を呼ぶ）
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose()) {
		UpdateDrawFrame();
	}
#endif



	UnloadRenderTexture(target);

	UnloadTexture(fontTex);
	UnloadTexture(chrTex);

	UnloadSound(explosionSound);
	UnloadSound(laserSound);

	UnloadMusicStream(bgm);

    CloseWindow();
    return 0;
}
