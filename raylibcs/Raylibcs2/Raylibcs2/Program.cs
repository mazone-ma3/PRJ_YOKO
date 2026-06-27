using System;
using System.Collections.Generic;
using System.Drawing;
using System.Numerics;
using System.Timers;
using Raylib_cs;
using RaylibSideScrollerShooter;
using static System.Net.Mime.MediaTypeNames;
using Color = Raylib_cs.Color;
using Image = Raylib_cs.Image;
using Rectangle = Raylib_cs.Rectangle;

namespace RaylibSideScrollerShooter
{

    internal class Enemy {
        public float X = 0f;
        public float Y = 0f;
        public float shootTimer = 0f;
        public float nextShootTime = 0f;
        public int type = 0;
        public float count = 0f;
        public float count2 = 0f;
        public int count_hp = 0;
        public bool count_flag = false;


        public Enemy(float initX, float initY, float initshootTimer, float initnextShootTime,
            int inittype, float initcount, float initcount2, int inithp, bool initflag)
        {
            X = initX;
            Y = initY;
            shootTimer = initshootTimer;
            nextShootTime = initnextShootTime;
            type = inittype;
            count = initcount;
            count2 = initcount2;
            count_hp = inithp;
            count_flag = initflag;
        }
    };
    internal class eBullet {
        public float X;
        public float Y;
        public float vx;
        public float vy;

        public eBullet(float initX, float initY, float initvx, float initvy)
        {
            X = initX;
            Y = initY;
            vx = initvx;
            vy = initvy;
        }
    };

    internal class Option {
        public float offset_y;
        public float X;
        public float Y;

        public Option(float initoffset_y, float initX, float initY)
        {
            offset_y = initoffset_y;
            X = 0; // initX;
            Y = 0; // initY;
        }
    }

    internal class Item {
        public float X;
        public float Y;
        public float timer;
        public int type;
        public Item(float initX, float initY, float inittimer, int inittype)
        {
            X = initX;
            Y = initY;
            timer = inittimer;
            type = inittype;
        }
    }

    internal class ChainItem {
        public float X;
        public float Y;
        public float timer;
        public ChainItem(float initX, float initY, float inittimer)
        {
            X = initX;
            Y = initY;
            timer = inittimer;
        }
    }

    public class Star { public float x, y, baseSpeed, speed, size; }; // ID2D1SolidColorBrush* brush = nullptr; };

    internal class Particle {
        public float X, Y;
        public float vx, vy;
        public float life;           // 残りフレーム
        public int color_index;    // 0?4で星ブラシと同じ色を使う
        public int type;           // 0=通常破片、1=大きな爆発など（後で拡張用）
        public Particle(float initX, float initY, float initvx, float initvy, int initlife, int initcolor_index, int inittype)
        {
            X = initX;
            Y = initY;
            vx = initvx;
            vy = initvy;
            life = initlife;
            color_index = initcolor_index;
            type = inittype;
        }
    };

    public static class GameConfig
    {
        // 画面サイズ
        public const int X_SCALE = 2;
        public const int Y_SCALE = 2;

        public const int ScreenWidth = 256 * 2 * X_SCALE;
        public const int ScreenHeight = 192 * 2 * Y_SCALE;
        public const int FONT_SIZE = 32;
        public const int MAX_OPTIONS = 2;
        public const float BOMB_DURATION = 0.0f;
    }

 
    public class GameWorld
    {
        private const float COUNT1S = 60f;

        private float scale;

        private int bomb_stock = 0;
        private bool bomb_active = false;
        private float bomb_timer = 0.0f;
        private bool key_b_flag = false;

        private bool shield_active = false;

        private float gametime = 0;
        private int option_cooldown = 0;

        private int chain_count = 0;
        private float chain_timer = 0f;

        private bool easy_mode = false;
        private int lives = 0;

        // プレイヤー
        private Rectangle player = new Rectangle(60f, 160f, 32f, 12f);//60, 50);
        private float playerSpeed = 4f * COUNT1S;

        // 自機弾
        private List<Rectangle> bullets = new List<Rectangle>();
        private float bulletSpeed = 12f * COUNT1S;
        private float shootCooldown = 0f;

        // 敵
        //        static List<Rectangle> enemies = new List<Rectangle>();
        private List<Enemy> enemies = new List<Enemy>();
        private float enemySpawnTimer = 0f;
        //        static float enemySpeed = 200f;

        // 敵弾
        private List<eBullet> eBullets = new List<eBullet>();

        // パーティクル
        private List<Particle> particles = new List<Particle>();


        private List<ChainItem> chain_items = new List<ChainItem>();
        private List<Item> Items = new List<Item>();
        private List<Option> options = new List<Option>();


        // スコア
        public int score { get; private set; } = 0;

        // ゲームオーバー
        public int gameOver { get; private set; } = 1; // = 1;

        public AssetManager assets;

        //        static float enemySpawnTimer = 0.0f;
        //	}
        public RenderTexture2D target;
        public List<Star> stars;

        public GameWorld(List<Star> _stars, AssetManager _assets, RenderTexture2D _target, bool _easy_mode, int _gameover)
        {
            assets = _assets;
            target = _target;
            stars = _stars;
			easy_mode = _easy_mode;
            gameOver = _gameover;

            player.X = 60f;
            player.Y = 160f; // ScreenHeight / 2 - 30;
//            if (score > highScore) highScore = score;
            score = 0;
            if (easy_mode == true)
            {
                lives = 3;
            }
            else
            {
                lives = 1;
            }
//            gameOver = 0; // false;
            bullets.Clear();
            eBullets.Clear();
            enemies.Clear();
            options.Clear();
            Items.Clear();
            chain_items.Clear();
            particles.Clear();

            gametime = 0;
		    option_cooldown = 10;
			shield_active = false;
		    bomb_active = false;
	        bomb_stock = 0;
		    key_b_flag = false;
            chain_count = 0;
            chain_timer = 0f;

            enemySpawnTimer = 0.0f;

//            Raylib.StopMusicStream(assets.bgm);
//            Raylib.PlayMusicStream(assets.bgm);
        }

        // ゲームの更新
        public void UpdateGame(int gamepad)
        {
            float delta = Raylib.GetFrameTime();
            float rate = COUNT1S * delta;
            Raylib.UpdateMusicStream(assets.bgm);

            // 星移動
            foreach (var s in stars)
            {
                s.x -= s.baseSpeed * rate;   // これでFPSが60の時に元の速度と同じになる
                if (s.x < 0)
                {
                    s.x = GameConfig.ScreenWidth / GameConfig.X_SCALE;
//                    s.y = Raylib.GetRandomValue(0, ScreenHeight/Y_SCALE - 1);
                }
            }

            if (gameOver == 1)
            {
                if (!(Raylib.IsKeyDown(KeyboardKey.Space) || Raylib.IsKeyDown(KeyboardKey.Z) || Raylib.IsKeyDown(KeyboardKey.R) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonDown(gamepad, GamepadButton.RightFaceDown))
                    || Raylib.IsKeyDown(KeyboardKey.X) || Raylib.IsKeyDown(KeyboardKey.B) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonDown(gamepad, GamepadButton.RightFaceRight))))
                {
                    gameOver = 2;
                }
            }

            if (gameOver != 0)
                return;

            gametime += delta;

		    // 1. ゲームパッドが接続されているかチェック
			float axisX = 0;
			float axisY = 0;
		    if (Raylib.IsGamepadAvailable(gamepad))
		    {
		        // 2. アナログスティック（左スティック）の入力を取得
		        // 戻り値は -1.0f から 1.0f の間
				        axisX = Raylib.GetGamepadAxisMovement(gamepad, GamepadAxis.LeftX);
				        axisY = Raylib.GetGamepadAxisMovement(gamepad, GamepadAxis.LeftY);
			}

            // プレイヤー移動（WASD or ↑↓）
            if (Raylib.IsKeyDown(KeyboardKey.W) || Raylib.IsKeyDown(KeyboardKey.Up) || (axisY < -0.2f) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonDown(gamepad, GamepadButton.LeftFaceUp)))
                player.Y -= playerSpeed * delta;
            if (Raylib.IsKeyDown(KeyboardKey.S) || Raylib.IsKeyDown(KeyboardKey.Down) || (axisY > 0.2f) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonDown(gamepad, GamepadButton.LeftFaceDown)))
                player.Y += playerSpeed * delta;
            if (Raylib.IsKeyDown(KeyboardKey.A) || Raylib.IsKeyDown(KeyboardKey.Left) || (axisX < -0.2f) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonDown(gamepad, GamepadButton.LeftFaceLeft)))
                player.X -= playerSpeed * delta; // * 0.7f;
            if (Raylib.IsKeyDown(KeyboardKey.D) || Raylib.IsKeyDown(KeyboardKey.Right) || (axisX > 0.2f) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonDown(gamepad, GamepadButton.LeftFaceRight)))
                player.X += playerSpeed * delta; // * 0.7f;

            // 画面内制限
            player.X = Math.Clamp(player.X, 0, GameConfig.ScreenWidth / GameConfig.X_SCALE - 40); // 左側に留める
            player.Y = Math.Clamp(player.Y, 0, GameConfig.ScreenHeight / GameConfig.Y_SCALE - 32);


            // オプション更新
            foreach(var opt in options) {
//              opt.angle += 0.08f * rate;   // 回転速度
                // 滑らかに追従
//                opt.X += ((player.X + 16) - opt.X) / 4;
//                opt.Y += ((player.Y + opt.offset_y) - opt.Y) / 4;

				float t = 1.0f - MathF.Pow(1.0f - 0.25f, rate);
                opt.X = Single.Lerp(opt.X, player.X + 16, t);
				opt.Y = Single.Lerp(opt.Y, player.Y + opt.offset_y, t);
            }


            // 射撃（Space）
            shootCooldown -= delta;
            if ((Raylib.IsKeyDown(KeyboardKey.Space) || Raylib.IsKeyDown(KeyboardKey.Z) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonDown(gamepad, GamepadButton.RightFaceDown))) && shootCooldown <= 0)
            {
                bullets.Add(new Rectangle(player.X + player.Width, player.Y + player.Height, 20, 10));

		        // オプションからも発射
		        foreach(var opt in options) {
		            bullets.Add(new Rectangle(opt.X + 8, opt.Y + 12 , 0, 0));
		        }

                shootCooldown = 8 / COUNT1S; // 連射速度
            }


			if ((Raylib.IsKeyPressed(KeyboardKey.X) || Raylib.IsKeyPressed(KeyboardKey.B) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonPressed(gamepad, GamepadButton.RightFaceRight))) && bomb_stock > 0 && !bomb_active) {
		        if (key_b_flag == false)
		            UseBomb();
		        key_b_flag = true;
		    }
		    else{
		        key_b_flag = false;
			}


            // 弾の更新
            for (int i = bullets.Count - 1; i >= 0; i--)
            {
                Rectangle b = bullets[i];           // コピー取得
                b.X += bulletSpeed * delta;
                bullets[i] = b;                     // リストに戻す

                if (b.X > GameConfig.ScreenWidth / GameConfig.X_SCALE)
                    bullets.RemoveAt(i);
            }

            // 敵の生成
            //            enemySpawnTimer -= delta;
            //            if (enemySpawnTimer <= 0)
            enemySpawnTimer += delta;

            // 元のロジックをdeltaTimeに変換
            float baseInterval = 50.0f - (score / 250.0f);   // scoreが増えるほど短く
            float spawnInterval = Math.Max(18.0f / COUNT1S, baseInterval / COUNT1S);  // フレーム→秒に変換

            if (enemySpawnTimer >= spawnInterval)
            {
                int type, rand_num;

                rand_num = Raylib.GetRandomValue(0, 100-1);
                if (rand_num < 60) type = 0;
                else if (rand_num < 85) type = 1;
                else type = 2;

				int hp = 1;
				if(type != 0)
					hp = 3;

                //                enemies.Add(new Enemy(ScreenWidth + 50,
                //                  Raylib.GetRandomValue(50, ScreenHeight - 80), 50, 40));
                enemies.Add(new Enemy(GameConfig.ScreenWidth / GameConfig.X_SCALE + 0f,
                    Raylib.GetRandomValue(32, GameConfig.ScreenHeight / GameConfig.Y_SCALE - 32 - 32), 0f, 5f / COUNT1S, type, 0f,
                        Raylib.GetRandomValue(-30*2, GameConfig.ScreenHeight / GameConfig.Y_SCALE - 40 * 2), hp, false));
                    enemySpawnTimer = 0f; // (float)Raylib.GetRandomValue(60, 120) / 100f;
            }

            float enemySpeed = 4.0f * rate;
            float enemySpeed2 = 5.0f * rate;

            // 敵の更新
          for (int i = enemies.Count - 1; i >= 0; i--)
          {
                Enemy e = enemies[i];
                e.count += rate;

                if (e.type == 0){
                    e.X -= enemySpeed;
                }
                else if (e.type == 1)
                {

                    if (e.count < 24)
                    {   // 1段階：超急接近
                        e.X -= 6 * 2 * rate;
                        e.Y += ((player.Y + 8 - e.Y) / 8) / 2 * rate;
                    }
                    else if (e.count < 49)  // 2段階：短くホバリング
                        e.X -= 0;
                    else                            // 3段階：右へ全力逃走
                        e.X += 6 * 2 * rate;
                }
                else if(e.type == 2)
                {     // サインカーブ
                    e.X -= enemySpeed;
                    e.Y = (e.count2 + (float)Math.Sin(e.count * 0.12) * 55 * 2);

                }
                enemies[i] = e;


                // 敵弾発射処理
                e.shootTimer += delta;

                int difficulty = (int)(Math.Min(1.0f, gametime / (180)));
                float enemy_bullet_speed = (4 + difficulty * 2);
                float shoot_interval = ((82 - difficulty * 36) - 5) / COUNT1S;

                if (e.shootTimer >= e.nextShootTime) {

                    float dx = player.X - e.X;
                    float dy = player.Y - e.Y;

                    float dist;
                    if(Math.Abs(dx) > Math.Abs(dy))
                        dist = Math.Abs(dx);
                    else
                        dist = Math.Abs(dy);

                    if (dist == 0) dist = 1;

                    // 弾を発射
                    float bulletSpeed = enemy_bullet_speed;

                    dx = (dx * bulletSpeed/dist);
                    dy = (dy * bulletSpeed/dist);
                    dx = Math.Max(-3*2.0f, dx);
                    dx = Math.Min(dx, 4*2.0f);
                    dy = Math.Max(-4*2.0f, dy);
                    dy = Math.Min(dy, 4*2.0f);

                    eBullets.Add(new eBullet(
                        e.X + 16, 
                        e.Y + 16,
                        dx, // * bulletSpeed - 1.0f*1,   // vx
                        dy // * bulletSpeed     // vy
                    ));

                    // 次回の発射間隔を設定
                    e.nextShootTime = shoot_interval;

                    e.shootTimer = 0.0f;
//                    e.count += rate;
                }
//            }

                // 自機 vs 敵 衝突判定（playerはRectangleなのでOK）
                //                if (Raylib.CheckCollisionRecs(player, e))
                if (e.X + 32 > player.X && e.X < player.X + 32 &&
                    e.Y + 32 > player.Y + 6 && e.Y < player.Y + 6 + 20)
                {
                    if (shield_active)
                    {
                        shield_active = false;           // シールド消費
                        CreateParticles(player.X + 16, player.Y + 16, 18, 1); // 大きな爆発
                    }
                    else
                    {
                        lives--;
                        if (lives <= 0)
                        {
                            Raylib.StopMusicStream(assets.bgm);
                            gameOver = 1;
                        }
                    }
                    enemies.RemoveAt(i);
                }
                //                i--;
            }

			// 敵画面外判定
            for (int i = enemies.Count - 1; i >= 0; i--)
            {
                Enemy e = enemies[i];
                if ((e.X < -32) || (e.X > GameConfig.ScreenWidth / GameConfig.X_SCALE))   // || (e.Y < 32) || (e.Y > ScreenHeight)) {
                                                                    //                 if (e.X < -60)
                {
                    enemies.RemoveAt(i);
//                    continue;
                }
                else
                {
  //                  i--;
                }
            }

            // 敵弾移動&画面範囲外判定
            for (int i = eBullets.Count - 1; i >= 0; --i)  
                //auto it = enemyBullets.begin(); it != enemyBullets.end();)
            {
                eBullet it = eBullets[i];
                it.X += it.vx * rate;
                it.Y += it.vy * rate;

                if ((it.X < -32) || (it.X > GameConfig.ScreenWidth / GameConfig.X_SCALE) || (it.Y < 32) || (it.Y > GameConfig.ScreenHeight / GameConfig.Y_SCALE))
                {
                    eBullets.RemoveAt(i);
                }
                else
                {
//                    ++it;
                }
            }


            // 自弾 vs 敵 衝突判定
            for (int b = bullets.Count - 1; b >= 0; b--)
            {
                Rectangle bullet = bullets[b];
		        bool hit = false;

                for (int e = enemies.Count - 1; e >= 0; e--)
                {
                    /*                    if (Raylib.CheckCollisionRecs(bullet, enemies[e]))
                                        {*/
                    Enemy enemy = enemies[e];
                    if (bullet.X + 16 > enemy.X && bullet.X < enemy.X + 32 &&
                        bullet.Y + 8 > enemy.Y && bullet.Y < enemy.Y + 32)
                    {
                        CreateParticles(enemy.X + 16, enemy.Y + 16, 8, 0);   // 通常爆発

		                if (--enemy.count_hp == 0)
						{

                            // オプションアイテム出現（確率20%くらい）
//                            if (Raylib.GetRandomValue(0, 100 - 1) < 22 && options.Count() < MAX_OPTIONS)
 //                           {
                                if (option_cooldown <= 0)
                                {
 //							        Item item;
 //							        item.x = eit->x;
 //							        item.y = eit->y;
 //			                        item.timer = 300.0f;        // 約5秒で消える
 //							        item.type = 1;           // 1 = オプションアイテム
                                    Items.Add(new Item(enemy.X, enemy.Y, 300f, 1)); //push_back(item);

                                    option_cooldown = 10;
                                }
                                else
                                {
                                    --option_cooldown;
                                }
 //                           }
							    // シールドアイテム出現（確率12%程度）
						    if (Raylib.GetRandomValue(0, 100 - 1) < 12 && !shield_active) {
//						        Item item;
//						        item.x = eit->x;
//						        item.y = eit->y;
//						        item.timer = 280.0f;
//						        item.type = 2;         // 2 = シールド
						        Items.Add(new Item(enemy.X, enemy.Y, 280f, 2)); //push_back(item);
						    }

		                    // ボムアイテム出現
		                    if (Raylib.GetRandomValue(0, 100 - 1) < 10) {        // 約10%の確率
//							    Item item;
//							    item.x = eit->x;
//							    item.y = eit->y;
//	    					    item.timer = 270.0f;
//							    item.type = 3;              // 3 = ボム
							    Items.Add(new Item(enemy.X, enemy.Y, 270f, 3));//push_back(item);
							}
						    // === チェインアイテム出現 ===
						    if (Raylib.GetRandomValue(0, 100 - 1) < 40) {        // 40%くらいの確率で落とす
//						        ChainItem item;
//						        item.x = eit->x;
//						        item.y = eit->y;
//						        item.timer = 240.0f;
						        chain_items.Add(new ChainItem(enemy.X, enemy.Y, 240)); //)push_back(item);
						    }

//		                      bullets.RemoveAt(b);
		                    enemies.RemoveAt(e);
		                    score += 100;
		                    Raylib.PlaySound(assets.explosionSound);
//		                    break;
							
						}
						hit = true;
						break;
                    }
                }
				if(hit)
				{
	                bullets.RemoveAt(b);
				}
            }

            // 敵弾 vs 自機
            for (int i = eBullets.Count - 1; i >= 0; i--) // auto it = enemyBullets.begin(); it != enemyBullets.end();)
            {
                eBullet it = eBullets[i];
                if (it.X > player.X && it.X + 8 < player.X + 32 &&
                    it.Y > player.Y + 6 && it.Y + 8 < player.Y + 6 + 20)
                {

                    if (shield_active)
                    {
                        shield_active = false;           // シールド消費
                        CreateParticles(player.X + 16, player.Y + 16, 18, 1); // 大きな爆発
                    }
                    else
                    {
                        lives--;
                        if (lives <= 0)
                        {
                            gameOver = 1;
                            //                  StopBGM();
                            Raylib.StopMusicStream(assets.bgm);
                        }
                    }
                    eBullets.RemoveAt(i);
                    break;
                }
//                else ++it;
            }

            // パーティクル更新
            for (int i = particles.Count - 1; i >= 0; i--)
            {
                Particle it = particles[i];
                it.X += it.vx * rate;
                it.Y += it.vy * rate;
//                it.vx *= 0.96f;      // 少し減速（空気抵抗）
//                it.vy *= 0.96f;
				float damping = MathF.Pow(0.96f, delta * 60f); 
				it.vx *= damping;
				it.vy *= damping;
                it.life -= rate;

                if (it.life <= 0)
                {
                    particles.RemoveAt(i);
//                    it = particles.erase(it);
                }
                else
                {
//                    ++it;
                }
            }

		    // ボム更新
				    if (bomb_active) {
		        bomb_timer -= delta;
		        if (bomb_timer <= 0f) {
		            bomb_active = false;
		        }
		    }


		    // アイテム更新
		    for (int i = Items.Count() - 1; i >= 0; i--) //auto it = Items.begin(); it != Items.end(); )
			{
				Item it = Items[i];
		        if (it.type == 1) {
		            it.X -= 2.0f * rate;   // 左に流れる
		        }
		        else if (it.type == 2) {
		            it.X -= 4.0f * rate;   // 左に流れる
		        }
		        else if (it.type == 3) {
		            it.X-= 4.0f * rate;   // 左に流れる
		        }
		        it.timer -= delta;

		        // 自機との当たり判定
		        if (Math.Abs(it.X - player.X) < 44-16 && Math.Abs(it.Y - player.Y) < 44-16) {

		            if (it.type == 1 && options.Count() < GameConfig.MAX_OPTIONS) {   // オプションアイテム
		                float offset;
						if(options.Count() == 0)
							offset = 25.0f;
						else
							offset =  -25.0f;
//		                Option opt;
//		                opt.offset_y = offset*2;
//		                opt.x = 0;//playerX + 20;
// 		               opt.y = 0;//playerY + 16 + offset;
//		                opt.angle = 0.0f;
		                options.Add(new Option(offset * 2, player.X + 20, player.Y + 16 + offset)); //push_back(opt);
		            }
		            else if (it.type == 2) {                    // シールド
		                shield_active = true;
//		                shield_timer = SHIELD_DURATION;
		            }
					else if (it.type == 3) {        // 3 = ボムアイテム
					    bomb_stock = Math.Min(3, bomb_stock + 1);
					}

//		            PlaySound(m_seLaser);
					Raylib.PlaySound(assets.laserSound);

		            Items.RemoveAt(i); //it = Items.erase(it);
		            continue;
		        }

		        // 画面外 or 時間切れ
		        if (it.X < -40 || it.timer <= 0) {
		            Items.RemoveAt(i); //it = Items.erase(it);
//                    continue;
		        } else {
//		            ++it;
		        }
		    }

		    // チェインアイテム更新
		    for (int i = chain_items.Count() - 1; i >= 0; i--)// { auto it = chain_items.begin(); it != chain_items.end(); ) {
			{
				ChainItem it = chain_items[i];
		        it.X -= 4f * rate;   // 左に流れる
		        it.timer -= delta;

                // 自機取得判定
                if (Math.Abs(it.X - player.X) < 44 - 16 && Math.Abs(it.Y - player.Y) < 44 - 16)
                {
                    chain_count++;
                    chain_timer = 240 / COUNT1S;           // チェイン持続時間リセット
                    score += chain_count * 100;    // チェイン数に応じたボーナス

                    chain_items.RemoveAt(i);
                    //		            PlaySound(m_seLaser);     // 取得音
                    Raylib.PlaySound(assets.laserSound);
                    continue;
                }
                chain_items[i] = it;
                // 時間切れ or 画面外
                if (it.timer <= 0.0f || it.X < -20)
                {
                    chain_count = 0;
                    chain_items.RemoveAt(i);
//                    continue;
                }
                else
                {
//                    ++it;
                }
            }

            // チェインタイマー減少
            if (chain_timer > 0.0f) {
                chain_timer -= delta;
               if (chain_timer <= 0.0f) {
                    chain_count = 0;
                }
			}

//		    if((gameOver != 0) && (score > high_score))
//		        high_score = score;

            // 背景スクロール
            //            bgX -= 120 * 2 * delta;
            //            if (bgX <= -bgTexture.Width) bgX = 0;
        }

        private void UseBomb() {
		    if (bomb_stock <= 0 || bomb_active) return;

		    bomb_stock--;
		    bomb_active = true;
//		    bomb_timer = BOMB_DURATION;

		    // 敵と敵弾を全滅
		    enemies.Clear();
		    eBullets.Clear();

		    // 大量の破片を発生
		    CreateParticles(player.X + 16, player.Y + 16, 45, 1);   // 大爆発

		    // 画面全体に破片を散らす
		    for (int i = 0; i < 60; ++i) {
		        float rx = Raylib.GetRandomValue(0, (GameConfig.ScreenWidth / GameConfig.X_SCALE) - 1);
		        float ry = Raylib.GetRandomValue(0, (GameConfig.ScreenHeight / GameConfig.Y_SCALE) - 1);
		        CreateParticles(rx, ry, 6, 1);
		    }

		    score += 200;
//		    PlaySound(m_seExplosion);   // ボム音（大きめの効果音を使う）
			Raylib.PlaySound(assets.explosionSound);
		}


        // 描画
        public void DrawGame(int high_score)
        {
            scale = MathF.Min((float)Raylib.GetScreenWidth() / GameConfig.ScreenWidth, (float)Raylib.GetScreenHeight() / GameConfig.ScreenHeight);
            //            X_SCALE = scale;
            //            Y_SCALE = scale;
            Rectangle destRec = new Rectangle(
            ((float)Raylib.GetScreenWidth() - ((float)GameConfig.ScreenWidth * scale)) * 0.5f,
            ((float)Raylib.GetScreenHeight() - ((float)GameConfig.ScreenHeight * scale)) * 0.5f,
            (float)GameConfig.ScreenWidth * scale,
            (float)GameConfig.ScreenHeight * scale
            );


            //            Raylib.BeginDrawing();
            Raylib.BeginTextureMode(target);

            Raylib.ClearBackground(Color.Black);

            // 背景（2枚並べてループ）
//            Raylib.DrawTexture(bgTexture, (int)bgX, 0, Color.White);
//            Raylib.DrawTexture(bgTexture, (int)bgX + bgTexture.Width, 0, Color.White);

            foreach (var s in stars) {
                Raylib.DrawCircle((int)s.x * GameConfig.X_SCALE, (int)s.y * GameConfig.Y_SCALE, 1.5f, Color.White);
            }


            // パーティクル描画（星の後くらいがおすすめ）
//          if (!particles.empty()){ // && m_pTextBrush) {
                foreach (var p in particles) {
                    if (p.life > 0) {
                        Raylib.DrawCircle((int)p.X * GameConfig.X_SCALE, (int)p.Y * GameConfig.Y_SCALE, 1.5f*2, Color.Yellow);
                    }
                }
//          }


            // チェインアイテム描画
            foreach (var item in chain_items)
            {
                put_sprite(item.X, item.Y, 3);   // 3番パターンにチェインアイテムの画像を入れる
            }

            foreach(var i in Items)
            {
                if (i.type == 1)
                    put_sprite(i.X, i.Y, 8);
                else if (i.type == 2)
                    put_sprite(i.X, i.Y, 7);
                else if (i.type == 3)
                    put_sprite(i.X, i.Y, 9);
            }

            // オプション描画
            foreach (var opt in options)
            {
                put_sprite(opt.X, opt.Y, 10);   // 10 = オプションのパターン番号（要調整）
            }

            // 敵弾
            foreach (var eBullet in eBullets)
            {
                put_sprite(eBullet.X, eBullet.Y, 0);
            }

            // 敵（赤い四角＋目）
            foreach (var enemy in enemies)
            {
//                Raylib.DrawRectangleRec(enemy, Color.Red);
/*                Raylib.DrawCircle((int)(enemy.X + 15), (int)(enemy.Y + 15), 8, Color.Black);
                Raylib.DrawCircle((int)(enemy.X + 35), (int)(enemy.Y + 15), 8, Color.Black);*/
                put_sprite(enemy.X, enemy.Y, 2);
            }

            // 自機弾
            foreach (var bullet in bullets)
            {
//                Raylib.DrawRectangleRec(bullet, Color.Yellow);
                put_sprite(bullet.X, bullet.Y, 4);
            }

            // シールド描画
            if (shield_active){ // && m_pSpriteBitmap) {
                    put_sprite(player.X, player.Y, 6);
            }

            // プレイヤー（簡易三角形風）
/*            Raylib.DrawRectangleRec(player, Color.Lime);
            Raylib.DrawTriangle(
                new Vector2(player.X + player.Width, player.Y + 10),
                new Vector2(player.X + player.Width, player.Y + player.Height - 10),
                new Vector2(player.X + player.Width + 30, player.Y + player.Height / 2),
                Color.Yellow);*/
                put_sprite(player.X, player.Y, 1);

            // UI
//            Raylib.DrawText($"SCORE: {score}", 20, 20, 30, Color.White);
            //            Raylib.DrawText("WASD / ↑↓移動   Spaceで射撃", 20, ScreenHeight - 40, 20, Color.Gray);

            if (score >= high_score)
                put_strings_num(0, 0, "HIGH  ", score, 7);
            else
                put_strings_num(0, 0, "SCORE ", score, 7);

            if (easy_mode == true)
                put_strings_num(0, 2 * GameConfig.FONT_SIZE, "LIVES ", lives, 1);

            put_strings_num(0, 1 * GameConfig.FONT_SIZE, "BOMB  ", bomb_stock, 1);

//            wchar_t text[128];
            put_strings_num(16 * GameConfig.FONT_SIZE, 0, "COUNT ", (int)gametime, 7);

            if (chain_count > 0)
            {
                put_strings_num(16 * GameConfig.FONT_SIZE, 1 * GameConfig.FONT_SIZE, "CHAIN ", chain_count, 3);
            }


            if (gameOver != 0)
            {
                //                Raylib.DrawRectangle(0, 0, ScreenWidth, ScreenHeight, new Color(0, 0, 0, 180));
                //                Raylib.DrawText("GAME OVER", ScreenWidth / 2 - 180, ScreenHeight / 2 - 80, 70, Color.Red);
                //                Raylib.DrawText($"FINAL SCORE: {score}", ScreenWidth / 2 - 160, ScreenHeight / 2 + 20, 40, Color.White);
                //                Raylib.DrawText("RESTART R KEY", ScreenWidth / 2 - 140, ScreenHeight / 2 + 80, 30, Color.Yellow);
                put_strings(11 * GameConfig.FONT_SIZE, 12 * GameConfig.FONT_SIZE, "GAME OVER");
                put_strings_num(7 * GameConfig.FONT_SIZE, 15 * GameConfig.FONT_SIZE, "HIGH SCORE ", high_score, 7);

                put_strings(7 * GameConfig.FONT_SIZE, 18 * GameConfig.FONT_SIZE, "PRESS A TO RESTART");
            }

            Raylib.EndTextureMode();
            //            Raylib.EndDrawing();

            Raylib.BeginDrawing();
            Raylib.ClearBackground(Color.Black); // フルスクリーン時の「黒帯」になる部分の色

            // レンダーテクスチャは上下の座標が反転しているため、sourceのheightをマイナスにする必要があります
            Rectangle sourceRec = new Rectangle( 0.0f, 0.0f, (float)target.Texture.Width, -(float)target.Texture.Height );
            Vector2 origin = new Vector2( 0.0f, 0.0f );

            // 計算した位置・サイズ（destRec）で綺麗に拡大描画
            Raylib.DrawTexturePro(target.Texture, sourceRec, destRec, origin, 0.0f, Color.White);

            // デバッグ情報（実際の現在のウィンドウサイズを表示）
            //            DrawFPS(10, 10);
            //            DrawText("F: Toggle Fullscreen", 10, 30, 20, GREEN);
            Raylib.EndDrawing();
        }


        void CreateParticles(float x, float y, int count, int type = 0)
        {
            for (int i = 0; i < count; ++i)
            {
                particles.Add(new Particle(x, y,
                    Raylib.GetRandomValue(-50, (100 - 50) - 1) * 0.12f,   // -6.0 ~ +6.0
                    Raylib.GetRandomValue(-50, 100 - 50 - 1) * 0.12f,
                    20 + Raylib.GetRandomValue(0, 25),
                    Raylib.GetRandomValue(0, 5),
                    type
                ));
            }
        }

        private void put_strings(float x, float y, string text) { //, int mode) {
            int len = text.Count(); // wcslen(text);
            for (int i = 0; i < len; ++i)
            {
                if (text[i] != ' ')
                {
                    int pat_no = text[i] - '0';
                    float rotation = 0.0f;

                    Rectangle destRect = new Rectangle(x, y, 16 * GameConfig.X_SCALE - 1, 16 * GameConfig.Y_SCALE - 1);
                    Rectangle sourceRect = new Rectangle(16.0f * (pat_no % 16), 16.0f * (pat_no / 16), 16.0f, 16.0f);
                    Vector2 origin = new Vector2(0, 0);

                    Raylib.DrawTexturePro(assets.fontTex, sourceRect, destRect, origin, rotation, Color.White);
                }
                x += 16 * GameConfig.X_SCALE;
            }
		}

        private void put_strings_num(float x, float y, string str, int num, int digit){ //, int mode) {
		    string text = ""; //[128];
            int len = str.Count(); // wcslen(str), i = digit, j = num;
            int i = digit;
            int j = num;
			put_strings(x, y, str);

		    while (--i >= 0) {
                //		        text[i] = j % 10 + '0';
                text = (j % 10) + text; // + '0');
		        j /= 10;
		    }
//		    text[digit] = '\0';
		    put_strings(x+len * GameConfig.FONT_SIZE, y, text);
		}

        void put_sprite(float x, float y, int pat_no)
        {
            float rotation = 0.0f;

            Rectangle destRect = new Rectangle(x * GameConfig.X_SCALE, y * GameConfig.Y_SCALE, 32 * GameConfig.X_SCALE - 1, 32 * GameConfig.Y_SCALE - 1);
            Rectangle sourceRect = new(32.0f * pat_no, 0, 32.0f, 32.0f);
            Vector2 origin = new Vector2( 0, 0);//destRect.width/2, destRect.height/2 };

            Raylib.DrawTexturePro(assets.chrTex, sourceRect, destRect, origin, rotation, Color.White);
        }
    }
    //}

    public class AssetManager
    {
        public Texture2D chrTex;
        public Texture2D fontTex;
        public Sound laserSound;
        public Sound explosionSound;
        public Music bgm;

        public void LoadAll()
        {
            chrTex = Raylib.LoadTexture("yokosht.png"); // 画像がなければ後で矩形で代用
            fontTex = Raylib.LoadTexture("FONTYOKO.png");

            Raylib.InitAudioDevice();
            laserSound = Raylib.LoadSound("laser.wav");
            explosionSound = Raylib.LoadSound("explosion.wav");
            bgm = Raylib.LoadMusicStream("bgm.mp3");
        }
        public void UnloadAll()
        {
            Raylib.UnloadTexture(fontTex);
            Raylib.UnloadTexture(chrTex);

            Raylib.UnloadSound(explosionSound);
            Raylib.UnloadSound(laserSound);

            Raylib.UnloadMusicStream(bgm);
        }
    }

    class Program
    {
        static int high_score = 5000;
        static List<Star> stars = new List<Star>();

        static RenderTexture2D target;


        static void Main(string[] args)
        {
            // 背景
            //        static float bgX = 0f;
            //        static Texture2D bgTexture;
            Raylib.SetConfigFlags(ConfigFlags.ResizableWindow | ConfigFlags.VSyncHint);
            Raylib.InitWindow(GameConfig.ScreenWidth, GameConfig.ScreenHeight, "Raylib C# 横スクロールシューティング");
            //            Raylib.SetTargetFPS(60);

            target = Raylib.LoadRenderTexture(GameConfig.ScreenWidth, GameConfig.ScreenHeight);
            Raylib.SetTextureFilter(target.Texture, TextureFilter.Point);

            AssetManager assets = new AssetManager();
            assets.LoadAll();

            // 簡易的な背景用テクスチャ（星空）
            /*            Image bgImage = Raylib.GenImageColor(ScreenWidth, ScreenHeight, Color.Black);
                        // 星を少し描画
                        for (int i = 0; i < 100; i++)
                        {
                            int x = Raylib.GetRandomValue(0, 799);
                            int y = Raylib.GetRandomValue(0, ScreenHeight - 1);
                            Raylib.ImageDrawPixel(ref bgImage, x, y, Color.White);
                        }
                        bgTexture = Raylib.LoadTextureFromImage(bgImage);
                        Raylib.UnloadImage(bgImage);
            */

            for (int i = 0; i < 80; ++i)
            {
                stars.Add(new Star());
                Star s = stars[i];
                s.x = Raylib.GetRandomValue(0, GameConfig.ScreenWidth / GameConfig.X_SCALE - 1);
                s.y = Raylib.GetRandomValue(0, GameConfig.ScreenHeight / GameConfig.Y_SCALE - 1);
                s.baseSpeed = 0.5f + Raylib.GetRandomValue(0, 99) / 30f; // static_cast<float>(rand() % 100) / 30.0f;
                s.speed = s.baseSpeed;   // もし個別速度も欲しい場合
                if (s.speed > 2f)
                    s.size = 2f;
                else
                    s.size = 1f;
                //              s.brush = nullptr;                    // 最初はnullptrにしておく
                //              stars.push_back(s);
                stars[i] = s;//.Add(new s);
            }

            //            ResetGame();

            GameWorld game = new GameWorld(stars, assets, target, false, 1);
//            game.gameOver = 1;

            while (!Raylib.WindowShouldClose())
            {
	            if (Raylib.IsKeyPressed(KeyboardKey.F11))
	            {
	                Raylib.ToggleFullscreen();
	            }

                int gamepad = 0;

                if (game.gameOver == 2){
					bool shouldReset = false;
			        bool useEasyMode = false;
	                if (Raylib.IsKeyPressed(KeyboardKey.Space) || Raylib.IsKeyPressed(KeyboardKey.Z) || Raylib.IsKeyPressed(KeyboardKey.R) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonPressed(gamepad, GamepadButton.RightFaceDown)))
	                {
						shouldReset = true;
						useEasyMode = false;
					}
                    else if (Raylib.IsKeyPressed(KeyboardKey.X) || Raylib.IsKeyPressed(KeyboardKey.B) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonPressed(gamepad, GamepadButton.RightFaceRight)))
	                {
						shouldReset = true;
						useEasyMode = true;
					}

					if (shouldReset)
					{
//	                    ResetGame();
                        game = new GameWorld(stars, assets, target, useEasyMode, 0);
                        Raylib.StopMusicStream(assets.bgm);
                        Raylib.PlayMusicStream(assets.bgm);
                    }
                }

                game.UpdateGame(gamepad);
                //                }
                if ((game.gameOver != 0) && (game.score > high_score))
                    high_score = game.score;

                game.DrawGame(high_score);
            }

            //          Raylib.UnloadTexture(bgTexture);
            assets.UnloadAll();

            Raylib.CloseWindow();
        }
        //	}
    }
}