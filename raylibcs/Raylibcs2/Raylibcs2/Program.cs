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
        public float ShootTimer = 0f;
        public float NextShootTime = 0f;
        public int Type = 0;
        public float Count = 0f;
        public float Count2 = 0f;
        public int CountHP = 0;
        public bool CountFlag = false;


        public Enemy(float initX, float initY, float initshootTimer, float initnextShootTime,
            int inittype, float initcount, float initcount2, int inithp, bool initflag)
        {
            X = initX;
            Y = initY;
            ShootTimer = initshootTimer;
            NextShootTime = initnextShootTime;
            Type = inittype;
            Count = initcount;
            Count2 = initcount2;
            CountHP = inithp;
            CountFlag = initflag;
        }
    };
    internal class EnemyBullet {
        public float X;
        public float Y;
        public float Vx;
        public float Vy;

        public EnemyBullet(float initX, float initY, float initvx, float initvy)
        {
            X = initX;
            Y = initY;
            Vx = initvx;
            Vy = initvy;
        }
    };

    internal class Option {
        public float offsetY;
        public float X;
        public float Y;

        public Option(float initoffsetY, float initX, float initY)
        {
            offsetY = initoffsetY;
            X = 0; // initX;
            Y = 0; // initY;
        }
    }

    internal class Item {
        public float X;
        public float Y;
        public float Timer;
        public int Type;
        public Item(float initX, float initY, float inittimer, int inittype)
        {
            X = initX;
            Y = initY;
            Timer = inittimer;
            Type = inittype;
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
        public float Vx, Vy;
        public float Life;           // 残りフレーム
        public int ColorIndex;    // 0?4で星ブラシと同じ色を使う
        public int Type;           // 0=通常破片、1=大きな爆発など（後で拡張用）
        public Particle(float initX, float initY, float initvx, float initvy, int initlife, int initcolorIndex, int inittype)
        {
            X = initX;
            Y = initY;
            Vx = initvx;
            Vy = initvy;
            Life = initlife;
            ColorIndex = initcolorIndex;
            Type = inittype;
        }
    };

    public static class GameConfig
    {
        // 画面サイズ
        public const int X_SCALE = 2;
        public const int Y_SCALE = 2;

        public const int SCREEN_WIDTH = 256 * 2 * X_SCALE;
        public const int SCREEN_HEIGHT = 192 * 2 * Y_SCALE;
        public const int FONT_SIZE = 32;
        public const int MAX_OPTIONS = 2;
        public const float BOMB_DURATION = 0.0f;
    }

 
    public class GameWorld
    {
        private const float COUNT1S = 60f;

        private float _scale;

        private int _bombStock = 0;
        private bool _bombActive = false;
        private float _bombTimer = 0.0f;
        private bool _keybFlag = false;

        private bool _shieldActive = false;

        private float _gametime = 0;
        private int _optionCooldown = 0;

        private int _chainCount = 0;
        private float _chainTimer = 0f;

        private bool _easyMode = false;
        private int _lives = 0;

        // プレイヤー
        private Rectangle _player = new Rectangle(60f, 160f, 32f, 12f);//60, 50);
        private float _playerSpeed = 4f * COUNT1S;

        // 自機弾
        private List<Rectangle> _bullets = new List<Rectangle>();
        private float _bulletSpeed = 12f * COUNT1S;
        private float _shootCooldown = 0f;

        // 敵
        //        static List<Rectangle> enemies = new List<Rectangle>();
        private List<Enemy> _enemies = new List<Enemy>();
        private float _enemySpawnTimer = 0f;
        //        static float enemySpeed = 200f;

        // 敵弾
        private List<EnemyBullet> _eBullets = new List<EnemyBullet>();

        // パーティクル
        private List<Particle> _particles = new List<Particle>();


        private List<ChainItem> _chainItems = new List<ChainItem>();
        private List<Item> _items = new List<Item>();
        private List<Option> _options = new List<Option>();


        // スコア
        public int score { get; private set; } = 0;

        // ゲームオーバー
        public int gameOver { get; private set; } = 1; // = 1;

        public AssetManager assets;

        //        static float enemySpawnTimer = 0.0f;
        //  }
        public RenderTexture2D target;
        public List<Star> stars;

        public GameWorld(List<Star> initstars, AssetManager initassets, RenderTexture2D inittarget, bool initeasyMode, int initgameOver)
        {
            assets = initassets;
            target = inittarget;
            stars = initstars;
            _easyMode = initeasyMode;
            gameOver = initgameOver;

            _player.X = 60f;
            _player.Y = 160f; // ScreenHeight / 2 - 30;
//            if (score > highScore) highScore = score;
            score = 0;
            if (_easyMode == true)
            {
                _lives = 3;
            }
            else
            {
                _lives = 1;
            }
//            gameOver = 0; // false;
            _bullets.Clear();
            _eBullets.Clear();
            _enemies.Clear();
            _options.Clear();
            _items.Clear();
            _chainItems.Clear();
            _particles.Clear();

            _gametime = 0;
            _optionCooldown = 10;
            _shieldActive = false;
            _bombActive = false;
            _bombStock = 0;
            _keybFlag = false;
            _chainCount = 0;
            _chainTimer = 0f;

            _enemySpawnTimer = 0.0f;

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
                    s.x = GameConfig.SCREEN_WIDTH / GameConfig.X_SCALE;
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

            _gametime += delta;

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
                _player.Y -= _playerSpeed * delta;
            if (Raylib.IsKeyDown(KeyboardKey.S) || Raylib.IsKeyDown(KeyboardKey.Down) || (axisY > 0.2f) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonDown(gamepad, GamepadButton.LeftFaceDown)))
                _player.Y += _playerSpeed * delta;
            if (Raylib.IsKeyDown(KeyboardKey.A) || Raylib.IsKeyDown(KeyboardKey.Left) || (axisX < -0.2f) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonDown(gamepad, GamepadButton.LeftFaceLeft)))
                _player.X -= _playerSpeed * delta; // * 0.7f;
            if (Raylib.IsKeyDown(KeyboardKey.D) || Raylib.IsKeyDown(KeyboardKey.Right) || (axisX > 0.2f) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonDown(gamepad, GamepadButton.LeftFaceRight)))
                _player.X += _playerSpeed * delta; // * 0.7f;

            // 画面内制限
            _player.X = Math.Clamp(_player.X, 0, GameConfig.SCREEN_WIDTH / GameConfig.X_SCALE - 40); // 左側に留める
            _player.Y = Math.Clamp(_player.Y, 0, GameConfig.SCREEN_HEIGHT / GameConfig.Y_SCALE - 32);


            // オプション更新
            foreach(var opt in _options) {
//              opt.angle += 0.08f * rate;   // 回転速度
                // 滑らかに追従
//                opt.X += ((player.X + 16) - opt.X) / 4;
//                opt.Y += ((player.Y + opt.offset_y) - opt.Y) / 4;

                float t = 1.0f - MathF.Pow(1.0f - 0.25f, rate);
                opt.X = Single.Lerp(opt.X, _player.X + 16, t);
                opt.Y = Single.Lerp(opt.Y, _player.Y + opt.offsetY, t);
            }


            // 射撃（Space）
            _shootCooldown -= delta;
            if ((Raylib.IsKeyDown(KeyboardKey.Space) || Raylib.IsKeyDown(KeyboardKey.Z) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonDown(gamepad, GamepadButton.RightFaceDown))) && _shootCooldown <= 0)
            {
                _bullets.Add(new Rectangle(_player.X + _player.Width, _player.Y + _player.Height, 20, 10));

                // オプションからも発射
                foreach(var opt in _options) {
                    _bullets.Add(new Rectangle(opt.X + 8, opt.Y + 12 , 0, 0));
                }

                _shootCooldown = 8 / COUNT1S; // 連射速度
            }


            if ((Raylib.IsKeyPressed(KeyboardKey.X) || Raylib.IsKeyPressed(KeyboardKey.B) || (Raylib.IsGamepadAvailable(gamepad) && Raylib.IsGamepadButtonPressed(gamepad, GamepadButton.RightFaceRight))) && _bombStock > 0 && !_bombActive) {
                if (_keybFlag == false)
                    UseBomb();
                _keybFlag = true;
            }
            else{
                _keybFlag = false;
            }


            // 弾の更新
            for (int i = _bullets.Count - 1; i >= 0; i--)
            {
                Rectangle b = _bullets[i];           // コピー取得
                b.X += _bulletSpeed * delta;
                _bullets[i] = b;                     // リストに戻す

                if (b.X > GameConfig.SCREEN_WIDTH / GameConfig.X_SCALE)
                    _bullets.RemoveAt(i);
            }

            // 敵の生成
            //            enemySpawnTimer -= delta;
            //            if (enemySpawnTimer <= 0)
            _enemySpawnTimer += delta;

            // 元のロジックをdeltaTimeに変換
            float baseInterval = 50.0f - (score / 250.0f);   // scoreが増えるほど短く
            float spawnInterval = Math.Max(18.0f / COUNT1S, baseInterval / COUNT1S);  // フレーム→秒に変換

            if (_enemySpawnTimer >= spawnInterval)
            {
                int _type, _randNum;

                _randNum = Raylib.GetRandomValue(0, 100-1);
                if (_randNum < 60) _type = 0;
                else if (_randNum < 85) _type = 1;
                else _type = 2;

                int hp = 1;
                if(_type != 0)
                    hp = 3;

                //                enemies.Add(new Enemy(ScreenWidth + 50,
                //                  Raylib.GetRandomValue(50, ScreenHeight - 80), 50, 40));
                _enemies.Add(new Enemy(GameConfig.SCREEN_WIDTH / GameConfig.X_SCALE + 0f,
                    Raylib.GetRandomValue(32, GameConfig.SCREEN_HEIGHT / GameConfig.Y_SCALE - 32 - 32), 0f, 5f / COUNT1S, _type, 0f,
                        Raylib.GetRandomValue(-30*2, GameConfig.SCREEN_HEIGHT / GameConfig.Y_SCALE - 40 * 2), hp, false));
                    _enemySpawnTimer = 0f; // (float)Raylib.GetRandomValue(60, 120) / 100f;
            }

            float enemySpeed = 4.0f * rate;
            float enemySpeed2 = 5.0f * rate;

            // 敵の更新
          for (int i = _enemies.Count - 1; i >= 0; i--)
          {
                Enemy e = _enemies[i];
                e.Count += rate;

                if (e.Type == 0){
                    e.X -= enemySpeed;
                }
                else if (e.Type == 1)
                {

                    if (e.Count < 24)
                    {   // 1段階：超急接近
                        e.X -= 6 * 2 * rate;
                        e.Y += ((_player.Y + 8 - e.Y) / 8) / 2 * rate;
                    }
                    else if (e.Count < 49)  // 2段階：短くホバリング
                        e.X -= 0;
                    else                            // 3段階：右へ全力逃走
                        e.X += 6 * 2 * rate;
                }
                else if(e.Type == 2)
                {     // サインカーブ
                    e.X -= enemySpeed;
                    e.Y = (e.Count2 + (float)Math.Sin(e.Count * 0.12) * 55 * 2);

                }
                _enemies[i] = e;


                // 敵弾発射処理
                e.ShootTimer += delta;

                int difficulty = (int)(Math.Min(1.0f, _gametime / (180)));
                float _enemyBulletSpeed = (4 + difficulty * 2);
                float _shootInterval = ((82 - difficulty * 36) - 5) / COUNT1S;

                if (e.ShootTimer >= e.NextShootTime) {

                    float dx = _player.X - e.X;
                    float dy = _player.Y - e.Y;

                    float dist;
                    if(Math.Abs(dx) > Math.Abs(dy))
                        dist = Math.Abs(dx);
                    else
                        dist = Math.Abs(dy);

                    if (dist == 0) dist = 1;

                    // 弾を発射
                    float bulletSpeed = _enemyBulletSpeed;

                    dx = (dx * bulletSpeed/dist);
                    dy = (dy * bulletSpeed/dist);
                    dx = Math.Max(-3*2.0f, dx);
                    dx = Math.Min(dx, 4*2.0f);
                    dy = Math.Max(-4*2.0f, dy);
                    dy = Math.Min(dy, 4*2.0f);

                    _eBullets.Add(new EnemyBullet(
                        e.X + 16, 
                        e.Y + 16,
                        dx, // * bulletSpeed - 1.0f*1,   // vx
                        dy // * bulletSpeed     // vy
                    ));

                    // 次回の発射間隔を設定
                    e.NextShootTime = _shootInterval;

                    e.ShootTimer = 0.0f;
//                    e.count += rate;
                }
//            }

                // 自機 vs 敵 衝突判定（playerはRectangleなのでOK）
                //                if (Raylib.CheckCollisionRecs(player, e))
                if (e.X + 32 > _player.X && e.X < _player.X + 32 &&
                    e.Y + 32 > _player.Y + 6 && e.Y < _player.Y + 6 + 20)
                {
                    if (_shieldActive)
                    {
                        _shieldActive = false;           // シールド消費
                        CreateParticles(_player.X + 16, _player.Y + 16, 18, 1); // 大きな爆発
                    }
                    else
                    {
                        _lives--;
                        if (_lives <= 0)
                        {
                            Raylib.StopMusicStream(assets.bgm);
                            gameOver = 1;
                        }
                    }
                    _enemies.RemoveAt(i);
                }
                //                i--;
            }

            // 敵画面外判定
            for (int i = _enemies.Count - 1; i >= 0; i--)
            {
                Enemy e = _enemies[i];
                if ((e.X < -32) || (e.X > GameConfig.SCREEN_WIDTH / GameConfig.X_SCALE))   // || (e.Y < 32) || (e.Y > ScreenHeight)) {
                                                                    //                 if (e.X < -60)
                {
                    _enemies.RemoveAt(i);
//                    continue;
                }
                else
                {
  //                  i--;
                }
            }

            // 敵弾移動&画面範囲外判定
            for (int i = _eBullets.Count - 1; i >= 0; --i)  
                //auto it = enemyBullets.begin(); it != enemyBullets.end();)
            {
                EnemyBullet it = _eBullets[i];
                it.X += it.Vx * rate;
                it.Y += it.Vy * rate;

                if ((it.X < -32) || (it.X > GameConfig.SCREEN_WIDTH / GameConfig.X_SCALE) || (it.Y < 32) || (it.Y > GameConfig.SCREEN_HEIGHT / GameConfig.Y_SCALE))
                {
                    _eBullets.RemoveAt(i);
                }
                else
                {
//                    ++it;
                }
            }


            // 自弾 vs 敵 衝突判定
            for (int b = _bullets.Count - 1; b >= 0; b--)
            {
                Rectangle bullet = _bullets[b];
                bool hit = false;

                for (int e = _enemies.Count - 1; e >= 0; e--)
                {
                    /*                    if (Raylib.CheckCollisionRecs(bullet, enemies[e]))
                                        {*/
                    Enemy enemy = _enemies[e];
                    if (bullet.X + 16 > enemy.X && bullet.X < enemy.X + 32 &&
                        bullet.Y + 8 > enemy.Y && bullet.Y < enemy.Y + 32)
                    {
                        CreateParticles(enemy.X + 16, enemy.Y + 16, 8, 0);   // 通常爆発

                        if (--enemy.CountHP == 0)
                        {

                            // オプションアイテム出現（確率20%くらい）
//                            if (Raylib.GetRandomValue(0, 100 - 1) < 22 && options.Count() < MAX_OPTIONS)
 //                           {
                                if (_optionCooldown <= 0)
                                {
 //                                 Item item;
 //                                 item.x = eit->x;
 //                                 item.y = eit->y;
 //                                 item.timer = 300.0f;        // 約5秒で消える
 //                                 item.type = 1;           // 1 = オプションアイテム
                                    _items.Add(new Item(enemy.X, enemy.Y, 300f, 1)); //push_back(item);

                                    _optionCooldown = 10;
                                }
                                else
                                {
                                    --_optionCooldown;
                                }
 //                           }
                                // シールドアイテム出現（確率12%程度）
                            if (Raylib.GetRandomValue(0, 100 - 1) < 12 && !_shieldActive) {
//                              Item item;
//                              item.x = eit->x;
//                              item.y = eit->y;
//                              item.timer = 280.0f;
//                              item.type = 2;         // 2 = シールド
                                _items.Add(new Item(enemy.X, enemy.Y, 280f, 2)); //push_back(item);
                            }

                            // ボムアイテム出現
                            if (Raylib.GetRandomValue(0, 100 - 1) < 10) {        // 約10%の確率
//                              Item item;
//                              item.x = eit->x;
//                              item.y = eit->y;
//                              item.timer = 270.0f;
//                              item.type = 3;              // 3 = ボム
                                _items.Add(new Item(enemy.X, enemy.Y, 270f, 3));//push_back(item);
                            }
                            // === チェインアイテム出現 ===
                            if (Raylib.GetRandomValue(0, 100 - 1) < 40) {        // 40%くらいの確率で落とす
//                              ChainItem item;
//                              item.x = eit->x;
//                              item.y = eit->y;
//                              item.timer = 240.0f;
                                _chainItems.Add(new ChainItem(enemy.X, enemy.Y, 240)); //)push_back(item);
                            }

//                            bullets.RemoveAt(b);
                            _enemies.RemoveAt(e);
                            score += 100;
                            Raylib.PlaySound(assets.explosionSound);
//                          break;
                            
                        }
                        hit = true;
                        break;
                    }
                }
                if(hit)
                {
                    _bullets.RemoveAt(b);
                }
            }

            // 敵弾 vs 自機
            for (int i = _eBullets.Count - 1; i >= 0; i--) // auto it = enemyBullets.begin(); it != enemyBullets.end();)
            {
                EnemyBullet it = _eBullets[i];
                if (it.X > _player.X && it.X + 8 < _player.X + 32 &&
                    it.Y > _player.Y + 6 && it.Y + 8 < _player.Y + 6 + 20)
                {

                    if (_shieldActive)
                    {
                        _shieldActive = false;           // シールド消費
                        CreateParticles(_player.X + 16, _player.Y + 16, 18, 1); // 大きな爆発
                    }
                    else
                    {
                        _lives--;
                        if (_lives <= 0)
                        {
                            gameOver = 1;
                            //                  StopBGM();
                            Raylib.StopMusicStream(assets.bgm);
                        }
                    }
                    _eBullets.RemoveAt(i);
                    break;
                }
//                else ++it;
            }

            // パーティクル更新
            for (int i = _particles.Count - 1; i >= 0; i--)
            {
                Particle it = _particles[i];
                it.X += it.Vx * rate;
                it.Y += it.Vy * rate;
//                it.vx *= 0.96f;      // 少し減速（空気抵抗）
//                it.vy *= 0.96f;
                float damping = MathF.Pow(0.96f, delta * 60f); 
                it.Vx *= damping;
                it.Vy *= damping;
                it.Life -= rate;

                if (it.Life <= 0)
                {
                    _particles.RemoveAt(i);
//                    it = particles.erase(it);
                }
                else
                {
//                    ++it;
                }
            }

            // ボム更新
                    if (_bombActive) {
                _bombTimer -= delta;
                if (_bombTimer <= 0f) {
                    _bombActive = false;
                }
            }


            // アイテム更新
            for (int i = _items.Count() - 1; i >= 0; i--) //auto it = Items.begin(); it != Items.end(); )
            {
                Item it = _items[i];
                if (it.Type == 1) {
                    it.X -= 2.0f * rate;   // 左に流れる
                }
                else if (it.Type == 2) {
                    it.X -= 4.0f * rate;   // 左に流れる
                }
                else if (it.Type == 3) {
                    it.X-= 4.0f * rate;   // 左に流れる
                }
                it.Timer -= delta;

                // 自機との当たり判定
                if (Math.Abs(it.X - _player.X) < 44-16 && Math.Abs(it.Y - _player.Y) < 44-16) {

                    if (it.Type == 1 && _options.Count() < GameConfig.MAX_OPTIONS) {   // オプションアイテム
                        float offset;
                        if(_options.Count() == 0)
                            offset = 25.0f;
                        else
                            offset =  -25.0f;
//                      Option opt;
//                      opt.offset_y = offset*2;
//                      opt.x = 0;//playerX + 20;
//                     opt.y = 0;//playerY + 16 + offset;
//                      opt.angle = 0.0f;
                        _options.Add(new Option(offset * 2, _player.X + 20, _player.Y + 16 + offset)); //push_back(opt);
                    }
                    else if (it.Type == 2) {                    // シールド
                        _shieldActive = true;
//                      shield_timer = SHIELD_DURATION;
                    }
                    else if (it.Type == 3) {        // 3 = ボムアイテム
                        _bombStock = Math.Min(3, _bombStock + 1);
                    }

//                  PlaySound(m_seLaser);
                    Raylib.PlaySound(assets.laserSound);

                    _items.RemoveAt(i); //it = Items.erase(it);
                    continue;
                }

                // 画面外 or 時間切れ
                if (it.X < -40 || it.Timer <= 0) {
                    _items.RemoveAt(i); //it = Items.erase(it);
//                    continue;
                } else {
//                  ++it;
                }
            }

            // チェインアイテム更新
            for (int i = _chainItems.Count() - 1; i >= 0; i--)// { auto it = chain_items.begin(); it != chain_items.end(); ) {
            {
                ChainItem it = _chainItems[i];
                it.X -= 4f * rate;   // 左に流れる
                it.timer -= delta;

                // 自機取得判定
                if (Math.Abs(it.X - _player.X) < 44 - 16 && Math.Abs(it.Y - _player.Y) < 44 - 16)
                {
                    _chainCount++;
                    _chainTimer = 240 / COUNT1S;           // チェイン持続時間リセット
                    score += _chainCount * 100;    // チェイン数に応じたボーナス

                    _chainItems.RemoveAt(i);
                    //                  PlaySound(m_seLaser);     // 取得音
                    Raylib.PlaySound(assets.laserSound);
                    continue;
                }
                _chainItems[i] = it;
                // 時間切れ or 画面外
                if (it.timer <= 0.0f || it.X < -20)
                {
                    _chainCount = 0;
                    _chainItems.RemoveAt(i);
//                    continue;
                }
                else
                {
//                    ++it;
                }
            }

            // チェインタイマー減少
            if (_chainTimer > 0.0f) {
                _chainTimer -= delta;
               if (_chainTimer <= 0.0f) {
                    _chainCount = 0;
                }
            }

//          if((gameOver != 0) && (score > high_score))
//              high_score = score;

            // 背景スクロール
            //            bgX -= 120 * 2 * delta;
            //            if (bgX <= -bgTexture.Width) bgX = 0;
        }

        private void UseBomb() {
            if (_bombStock <= 0 || _bombActive) return;

            _bombStock--;
            _bombActive = true;
//          bomb_timer = BOMB_DURATION;

            // 敵と敵弾を全滅
            _enemies.Clear();
            _eBullets.Clear();

            // 大量の破片を発生
            CreateParticles(_player.X + 16, _player.Y + 16, 45, 1);   // 大爆発

            // 画面全体に破片を散らす
            for (int i = 0; i < 60; ++i) {
                float rx = Raylib.GetRandomValue(0, (GameConfig.SCREEN_WIDTH / GameConfig.X_SCALE) - 1);
                float ry = Raylib.GetRandomValue(0, (GameConfig.SCREEN_HEIGHT / GameConfig.Y_SCALE) - 1);
                CreateParticles(rx, ry, 6, 1);
            }

            score += 200;
//          PlaySound(m_seExplosion);   // ボム音（大きめの効果音を使う）
            Raylib.PlaySound(assets.explosionSound);
        }


        // 描画
        public void DrawGame(int highScore)
        {
            _scale = MathF.Min((float)Raylib.GetScreenWidth() / GameConfig.SCREEN_WIDTH, (float)Raylib.GetScreenHeight() / GameConfig.SCREEN_HEIGHT);
            //            X_SCALE = scale;
            //            Y_SCALE = scale;
            Rectangle destRec = new Rectangle(
            ((float)Raylib.GetScreenWidth() - ((float)GameConfig.SCREEN_WIDTH * _scale)) * 0.5f,
            ((float)Raylib.GetScreenHeight() - ((float)GameConfig.SCREEN_HEIGHT * _scale)) * 0.5f,
            (float)GameConfig.SCREEN_WIDTH * _scale,
            (float)GameConfig.SCREEN_HEIGHT * _scale
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
                foreach (var p in _particles) {
                    if (p.Life > 0) {
                        Raylib.DrawCircle((int)p.X * GameConfig.X_SCALE, (int)p.Y * GameConfig.Y_SCALE, 1.5f*2, Color.Yellow);
                    }
                }
//          }


            // チェインアイテム描画
            foreach (var item in _chainItems)
            {
                PutSprite(item.X, item.Y, 3);   // 3番パターンにチェインアイテムの画像を入れる
            }

            foreach(var i in _items)
            {
                if (i.Type == 1)
                    PutSprite(i.X, i.Y, 8);
                else if (i.Type == 2)
                    PutSprite(i.X, i.Y, 7);
                else if (i.Type == 3)
                    PutSprite(i.X, i.Y, 9);
            }

            // オプション描画
            foreach (var opt in _options)
            {
                PutSprite(opt.X, opt.Y, 10);   // 10 = オプションのパターン番号（要調整）
            }

            // 敵弾
            foreach (var eBullet in _eBullets)
            {
                PutSprite(eBullet.X, eBullet.Y, 0);
            }

            // 敵（赤い四角＋目）
            foreach (var enemy in _enemies)
            {
//                Raylib.DrawRectangleRec(enemy, Color.Red);
/*                Raylib.DrawCircle((int)(enemy.X + 15), (int)(enemy.Y + 15), 8, Color.Black);
                Raylib.DrawCircle((int)(enemy.X + 35), (int)(enemy.Y + 15), 8, Color.Black);*/
                PutSprite(enemy.X, enemy.Y, 2);
            }

            // 自機弾
            foreach (var bullet in _bullets)
            {
//                Raylib.DrawRectangleRec(bullet, Color.Yellow);
                PutSprite(bullet.X, bullet.Y, 4);
            }

            // シールド描画
            if (_shieldActive){ // && m_pSpriteBitmap) {
                    PutSprite(_player.X, _player.Y, 6);
            }

            // プレイヤー（簡易三角形風）
/*            Raylib.DrawRectangleRec(player, Color.Lime);
            Raylib.DrawTriangle(
                new Vector2(player.X + player.Width, player.Y + 10),
                new Vector2(player.X + player.Width, player.Y + player.Height - 10),
                new Vector2(player.X + player.Width + 30, player.Y + player.Height / 2),
                Color.Yellow);*/
                PutSprite(_player.X, _player.Y, 1);

            // UI
//            Raylib.DrawText($"SCORE: {score}", 20, 20, 30, Color.White);
            //            Raylib.DrawText("WASD / ↑↓移動   Spaceで射撃", 20, ScreenHeight - 40, 20, Color.Gray);

            if (score >= highScore)
                PutStringsNum(0, 0, "HIGH  ", score, 7);
            else
                PutStringsNum(0, 0, "SCORE ", score, 7);

            if (_easyMode == true)
                PutStringsNum(0, 2 * GameConfig.FONT_SIZE, "LIVES ", _lives, 1);

            PutStringsNum(0, 1 * GameConfig.FONT_SIZE, "BOMB  ", _bombStock, 1);

//            wchar_t text[128];
            PutStringsNum(16 * GameConfig.FONT_SIZE, 0, "COUNT ", (int)_gametime, 7);

            if (_chainCount > 0)
            {
                PutStringsNum(16 * GameConfig.FONT_SIZE, 1 * GameConfig.FONT_SIZE, "CHAIN ", _chainCount, 3);
            }


            if (gameOver != 0)
            {
                //                Raylib.DrawRectangle(0, 0, ScreenWidth, ScreenHeight, new Color(0, 0, 0, 180));
                //                Raylib.DrawText("GAME OVER", ScreenWidth / 2 - 180, ScreenHeight / 2 - 80, 70, Color.Red);
                //                Raylib.DrawText($"FINAL SCORE: {score}", ScreenWidth / 2 - 160, ScreenHeight / 2 + 20, 40, Color.White);
                //                Raylib.DrawText("RESTART R KEY", ScreenWidth / 2 - 140, ScreenHeight / 2 + 80, 30, Color.Yellow);
                PutStrings(11 * GameConfig.FONT_SIZE, 12 * GameConfig.FONT_SIZE, "GAME OVER");
                PutStringsNum(7 * GameConfig.FONT_SIZE, 15 * GameConfig.FONT_SIZE, "HIGH SCORE ", highScore, 7);

                PutStrings(7 * GameConfig.FONT_SIZE, 18 * GameConfig.FONT_SIZE, "PRESS A TO RESTART");
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
                _particles.Add(new Particle(x, y,
                    Raylib.GetRandomValue(-50, (100 - 50) - 1) * 0.12f,   // -6.0 ~ +6.0
                    Raylib.GetRandomValue(-50, 100 - 50 - 1) * 0.12f,
                    20 + Raylib.GetRandomValue(0, 25),
                    Raylib.GetRandomValue(0, 5),
                    type
                ));
            }
        }

        private void PutStrings(float x, float y, string text) { //, int mode) {
            int len = text.Count(); // wcslen(text);
            for (int i = 0; i < len; ++i)
            {
                if (text[i] != ' ')
                {
                    int patNo = text[i] - '0';
                    float rotation = 0.0f;

                    Rectangle destRect = new Rectangle(x, y, 16 * GameConfig.X_SCALE - 1, 16 * GameConfig.Y_SCALE - 1);
                    Rectangle sourceRect = new Rectangle(16.0f * (patNo % 16), 16.0f * (patNo / 16), 16.0f, 16.0f);
                    Vector2 origin = new Vector2(0, 0);

                    Raylib.DrawTexturePro(assets.fontTex, sourceRect, destRect, origin, rotation, Color.White);
                }
                x += 16 * GameConfig.X_SCALE;
            }
        }

        private void PutStringsNum(float x, float y, string str, int num, int digit){ //, int mode) {
            string text = ""; //[128];
            int len = str.Count(); // wcslen(str), i = digit, j = num;
            int i = digit;
            int j = num;
            PutStrings(x, y, str);

            while (--i >= 0) {
                //              text[i] = j % 10 + '0';
                text = (j % 10) + text; // + '0');
                j /= 10;
            }
//          text[digit] = '\0';
            PutStrings(x+len * GameConfig.FONT_SIZE, y, text);
        }

        void PutSprite(float x, float y, int pat_no)
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
        static int highScore = 5000;
        static List<Star> stars = new List<Star>();

        static RenderTexture2D target;


        static void Main(string[] args)
        {
            // 背景
            //        static float bgX = 0f;
            //        static Texture2D bgTexture;
            Raylib.SetConfigFlags(ConfigFlags.ResizableWindow | ConfigFlags.VSyncHint);
            Raylib.InitWindow(GameConfig.SCREEN_WIDTH, GameConfig.SCREEN_HEIGHT, "Raylib C# 横スクロールシューティング");
            //            Raylib.SetTargetFPS(60);

            target = Raylib.LoadRenderTexture(GameConfig.SCREEN_WIDTH, GameConfig.SCREEN_HEIGHT);
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
                s.x = Raylib.GetRandomValue(0, GameConfig.SCREEN_WIDTH / GameConfig.X_SCALE - 1);
                s.y = Raylib.GetRandomValue(0, GameConfig.SCREEN_HEIGHT / GameConfig.Y_SCALE - 1);
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
//                      ResetGame();
                        game = new GameWorld(stars, assets, target, useEasyMode, 0);
                        Raylib.StopMusicStream(assets.bgm);
                        Raylib.PlayMusicStream(assets.bgm);
                    }
                }

                game.UpdateGame(gamepad);
                //                }
                if ((game.gameOver != 0) && (game.score > highScore))
                    highScore = game.score;

                game.DrawGame(highScore);
            }

            //          Raylib.UnloadTexture(bgTexture);
            assets.UnloadAll();

            Raylib.CloseWindow();
        }
        //  }
    }
}