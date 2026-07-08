package main

import (
	//	"fmt"
	"math"
	"math/rand"
	//	"time"

	rl "github.com/gen2brain/raylib-go/raylib"
)

// シンプルな横スクロールシューティング

const (
	X_SCALE      = 2
	Y_SCALE      = 2
	screenWidth  = 256 * 2 * X_SCALE
	screenHeight = 192 * 2 * Y_SCALE
	MAXOPTIONS   = 2
	COUNT1S      = 60
	MAXSTARS     = 80
	FONT_SIZE    = 16 * X_SCALE
)

type Player struct {
	pos   rl.Vector2
	speed float32
}

type Bullet struct {
	pos    rl.Vector2
	active bool
	speed  float32
}

type EnemyBullet struct {
	pos    rl.Vector2
	vx     float32
	vy     float32
	active bool
}

type Enemy struct {
	pos rl.Vector2
	//	speed  float32
	active        bool
	types         int
	hp            int
	count         float32
	count2        float32
	shootTimer    float32
	nextShootTime float32
	speed         float32
}

type Option struct {
	pos      rl.Vector2
	offset_y float32
	//	active   bool
}

type Item struct {
	pos    rl.Vector2
	timer  float32
	types  int
	active bool
}

type ChainItem struct {
	pos    rl.Vector2
	timer  float32
	active bool
}

type Particle struct {
	pos  rl.Vector2
	vx   float32
	vy   float32
	life float32
}

type Star struct {
	x, y, basespeed, speed, size float32
}

type Game struct {
	player       Player
	bullets      [32]Bullet
	enemies      [32]Enemy
	enemybullets [32]EnemyBullet

	options   [2]Option
	optionnum int

	items      [32]Item
	chainitems [32]ChainItem

	particles [2000]Particle

	score     int
	gameOver  int
	easy_mode bool

	bomb_active bool
	bomb_stock  int
	bomb_timer  float32

	chain_timer float32

	enemySpawnTimer float32

	high_score  int
	lives       int
	chain_count int

	chrTex  rl.Texture2D
	fontTex rl.Texture2D

	laserSound     rl.Sound
	explosionSound rl.Sound
	bgm            rl.Music

	delta         float32
	shootCooldown float32
	gameTime      float32

	shield_active   bool
	option_cooldown float32

	star []Star
}


func NewGame() *Game {
	game := &Game{
		high_score : 5000,
	}

//	rand.Seed(time.Now().UnixNano())

	rl.SetConfigFlags(rl.FlagWindowResizable | rl.FlagVsyncHint)

	rl.InitWindow(screenWidth, screenHeight, "raylib-Go 横スクロールシューティング")
//	defer rl.CloseWindow()

	game.chrTex = rl.LoadTexture("yokosht.png") // 画像がなければ後で矩形で代用
	game.fontTex = rl.LoadTexture("FONTYOKO.png")

	//    rl.SetTargetFPS(60)

	rl.InitAudioDevice()
	game.laserSound = rl.LoadSound("laser.wav")
	game.explosionSound = rl.LoadSound("explosion.wav")
	game.bgm = rl.LoadMusicStream("bgm.mp3")

	for i := 0; i < 80; i++ {
		game.star = append(game.star, Star{
			x:     float32(rl.GetRandomValue(0, screenWidth/X_SCALE-1)),
			y:     float32(rl.GetRandomValue(0, screenHeight/Y_SCALE-1)),
			speed: 0.5 + float32(rl.GetRandomValue(0, 99)/30), // static_cast<float>(rand() % 100) / 30.0f
		}) //    speed : basespeed   // もし個別速度も欲しい場合
		if game.star[i].speed > 2 {
			game.star[i].size = 2
		} else {
			game.star[i].size = 1
		}
		//        star[i] = s//.Add(new s)
	}

	return game
}

func (game *Game) Close() {
	rl.CloseWindow()

	rl.StopMusicStream(game.bgm)

	rl.UnloadTexture(game.fontTex)
	rl.UnloadTexture(game.chrTex)

	rl.UnloadSound(game.explosionSound)
	rl.UnloadSound(game.laserSound)

	rl.UnloadMusicStream(game.bgm)
}

func main() {
	game := NewGame()
	defer game.Close()

	target := rl.LoadRenderTexture(screenWidth, screenHeight)
	rl.SetTextureFilter(target.Texture, rl.FilterPoint)

	game.Reset()
	game.gameOver = 1

	//	rl.PlayMusicStream(game.bgm)

	for !rl.WindowShouldClose() {
		rl.UpdateMusicStream(game.bgm)

		if rl.IsKeyPressed(rl.KeyF11) {
			rl.ToggleFullscreen()
		}

		scale := float32(min(float32(rl.GetScreenWidth())/screenWidth, float32(rl.GetScreenHeight())/screenHeight))

		destRec := rl.NewRectangle((float32(rl.GetScreenWidth())-(screenWidth*scale))*0.5, (float32(rl.GetScreenHeight())-(screenHeight*scale))*0.5, screenWidth*scale, screenHeight*scale)

		game.delta = rl.GetFrameTime()
		//		if game.gameOver != 0 {
		game.Update()
		//		}
		rl.BeginTextureMode(target)
		game.Draw()
		rl.EndTextureMode()

		rl.BeginDrawing()
		rl.ClearBackground(rl.Black) // フルスクリーン時の「黒帯」になる部分の色

		// レンダーテクスチャは上下の座標が反転しているため、sourceのheightをマイナスにする必要があります
		sourceRec := rl.NewRectangle(0.0, 0.0, float32(target.Texture.Width), -float32(target.Texture.Height))
		origin := rl.NewVector2(0.0, 0.0)

		// 計算した位置・サイズ（destRec）で綺麗に拡大描画
		rl.DrawTexturePro(target.Texture, sourceRec, destRec, origin, 0.0, rl.White)

		// デバッグ情報（実際の現在のウィンドウサイズを表示）
		//            DrawFPS(10, 10)
		//            DrawText("F: Toggle Fullscreen", 10, 30, 20, GREEN)
		rl.EndDrawing()
	}
//	game.Close()
}

func (game *Game) Reset() {
	// プレイヤー初期化
	game.player = Player{
		pos:   rl.NewVector2(60, 160), //float32(screenHeight)/2),
		speed: 4,
	}
	for i := range game.bullets {
		game.bullets[i].active = false
	}
	for i := range game.enemies {
		game.enemies[i].active = false
	}

	for i := range game.enemybullets {
		game.enemybullets[i].active = false
	}

	/*	for i := range game.options {
		options[i].active = false
	}*/
	for i := range game.particles {
		game.particles[i].life = 0
	}

	for i := range game.items {
		game.items[i].active = false
	}
	for i := range game.chainitems {
		game.chainitems[i].active = false
	}

	game.optionnum = 0

	game.shootCooldown = 0
	game.score = 0

	game.enemySpawnTimer = 0.0
	game.gameOver = 0
	game.shield_active = false

	//	high_score = 5000
	game.lives = 0
	game.gameTime = 0
	game.chain_count = 0

	game.bomb_timer = 0
	game.option_cooldown = 10

	if game.easy_mode {
		game.lives = 3
	} else {
		game.lives = 1
	}
	game.bomb_stock = 0
}

func (game *Game) UseBomb() {
	if game.bomb_stock <= 0 || game.bomb_active {
		return
	}

	game.bomb_stock--
	game.bomb_active = true
	//   game. bomb_timer = BOMB_DURATION

	// 敵と敵弾を全滅
	for i := range game.enemies {
		game.enemies[i].active = false
	}
	for i := range game.enemybullets {
		game.enemybullets[i].active = false
	}

	// 大量の破片を発生
	game.CreateParticles(game.player.pos.X+16, game.player.pos.Y+16, 45, 1) // 大爆発

	// 画面全体に破片を散らす
	for i := 0; i < 60; i++ {
		rx := float32(rand.Intn(screenWidth/X_SCALE))
		ry := float32(rand.Intn(screenHeight/Y_SCALE))
		game.CreateParticles(rx, ry, 6, 1)
	}

	game.score += 200
	rl.PlaySound(game.explosionSound)
}

func (game *Game) Update() {

	gamepad := int32(0)
	rate := COUNT1S * game.delta

	if game.gameOver == 1 {
		if !(rl.IsKeyDown(rl.KeySpace) || rl.IsKeyDown(rl.KeyZ) || rl.IsKeyDown(rl.KeyR) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonRightFaceDown)) || rl.IsKeyDown(rl.KeySpace) || rl.IsKeyDown(rl.KeyZ) || rl.IsKeyDown(rl.KeyR) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonRightFaceDown))) {
			game.gameOver = 2
		}
	}
	if game.gameOver == 2 {
		if rl.IsKeyPressed(rl.KeySpace) || rl.IsKeyPressed(rl.KeyZ) || rl.IsKeyPressed(rl.KeyR) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonPressed(gamepad, rl.GamepadButtonRightFaceDown)) {
			game.easy_mode = false
			rl.PlayMusicStream(game.bgm)
			game.Reset()
		} else if rl.IsKeyPressed(rl.KeyX) || rl.IsKeyPressed(rl.KeyB) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonPressed(gamepad, rl.GamepadButtonRightFaceRight)) {
			game.easy_mode = true
			rl.PlayMusicStream(game.bgm)
			game.Reset()
		}
	}

	// 星移動
	for i := range game.star {
		s := &game.star[i]
		s.x -= s.speed * rate // これでFPSが60の時に元の速度と同じになる
		if s.x < 0 {
			s.x = screenWidth / X_SCALE
		}
	}

	if game.gameOver != 0 {
		return
	}

	game.gameTime += game.delta

	//movespeed := 4.0 * rate
	enemyspeed := 4.0 * rate
	//enemyspeed2 := 5.0 * rate

	// 1. ゲームパッドが接続されているかチェック
	axisX := float32(0)
	axisY := float32(0)
	if rl.IsGamepadAvailable(gamepad) {
		// 2. アナログスティック（左スティック）の入力を取得
		// 戻り値は -1.0f から 1.0f の間
		axisX = rl.GetGamepadAxisMovement(gamepad, rl.GamepadAxisLeftX)
		axisY = rl.GetGamepadAxisMovement(gamepad, rl.GamepadAxisLeftY)
	}

	// プレイヤー移動 (WASD or Arrow keys)
	if rl.IsKeyDown(rl.KeyUp) || rl.IsKeyDown(rl.KeyW) || (axisY < -0.2) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonLeftFaceUp)) {
		game.player.pos.Y -= game.player.speed * rate
	}
	if rl.IsKeyDown(rl.KeyDown) || rl.IsKeyDown(rl.KeyS) || (axisY > 0.2) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonLeftFaceDown)) {
		game.player.pos.Y += game.player.speed * rate
	}
	if rl.IsKeyDown(rl.KeyLeft) || rl.IsKeyDown(rl.KeyA) || (axisX < -0.2) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonLeftFaceLeft)) {
		game.player.pos.X -= game.player.speed * rate
	}
	if rl.IsKeyDown(rl.KeyRight) || rl.IsKeyDown(rl.KeyD) || (axisX > 0.2) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonLeftFaceRight)) {
		game.player.pos.X += game.player.speed * rate
	}

	// 画面端制限
	if game.player.pos.X < 0 {
		game.player.pos.X = 0
	}
	if game.player.pos.X > screenWidth/X_SCALE-40 {
		game.player.pos.X = screenWidth/X_SCALE - 40
	}
	if game.player.pos.Y < 0 {
		game.player.pos.Y = 0
	}
	if game.player.pos.Y > screenHeight/Y_SCALE-32 {
		game.player.pos.Y = screenHeight/Y_SCALE - 32
	}

	// オプション更新
	for i := 0; i < game.optionnum; i++ {
		//        game.opt.angle += 0.08f * rate;   // 回転速度
		opt := &game.options[i]
		// 滑らかに追従
		opt.pos.X += ((game.player.pos.X + 16) - opt.pos.X) / 4 * rate
		opt.pos.Y += ((game.player.pos.Y + opt.offset_y) - opt.pos.Y) / 4 * rate
		//		float t = 1.0f - pow(1.0f - 0.25f, m_deltaTime * COUNT1S)
		//		opt.x = std::lerp(opt.x, playerX + 16, t)
		//		opt.y = std::lerp(opt.y, playerY + opt.offset_y, t)
	}

	// 射撃 (Spaceキー)
	game.shootCooldown += game.delta
	if (rl.IsKeyDown(rl.KeySpace) || rl.IsKeyDown(rl.KeyZ) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonRightFaceDown))) && (game.shootCooldown >= float32(8)/COUNT1S) {
		/*		bullets = append(bullets, Bullet{
				pos:    rl.NewVector2(player.pos.X+32, player.pos.Y+12),
				speed:  12,
				active: true,
			})*/
		for i := range game.bullets {
			if game.bullets[i].active == false {
				game.bullets[i] = Bullet{
					pos:    rl.NewVector2(game.player.pos.X+32, game.player.pos.Y+12),
					speed:  12,
					active: true,
				}
				break
			}
		}
		for j := 0; j < game.optionnum; j++ {
			for i := range game.bullets {
				if game.bullets[i].active == false {
					game.bullets[i] = Bullet{
						pos:    rl.NewVector2(game.options[j].pos.X+8, game.options[j].pos.Y+12),
						speed:  12,
						active: true,
					}
					break
				}
			}
		}
		game.shootCooldown = 0 //(8 / COUNT1S) // 連射速度
	}

	if (rl.IsKeyPressed(rl.KeyX) || rl.IsKeyPressed(rl.KeyB) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonPressed(gamepad, rl.GamepadButtonRightFaceRight))) && game.bomb_stock > 0 && !game.bomb_active {
		game.UseBomb()
	}

	// 自機弾更新
	for i := range game.bullets {
		if game.bullets[i].active {
			game.bullets[i].pos.X += game.bullets[i].speed * rate
			if game.bullets[i].pos.X > screenWidth/X_SCALE {
				game.bullets[i].active = false
			}
		}
	}

	game.enemySpawnTimer += game.delta
	baseInterval := float32(50.0 - (game.score / 250.0))                   // scoreが増えるほど短く
	spawnInterval := max(float32(18.0)/COUNT1S, baseInterval/COUNT1S) // フレーム→秒に変換

	// 敵生成(スポーン) (ランダム)
	//	if rand.Intn(40) == 0 { // 調整可能
	if game.enemySpawnTimer >= spawnInterval {
		/*		enemies = append(enemies, Enemy{
				pos:    rl.NewVector2(screenWidth+30, float32(rand.Intn(screenHeight-40))),
				speed:  -4 - float32(rand.Intn(3)),
				active: true,
				types:	0,
				hp:     1,
			})*/
		game.enemySpawnTimer = 0

		for i := range game.enemies {
			if game.enemies[i].active == false {

				rand_num := rand.Intn(100)
				var etype int
				if rand_num < 60 {
					etype = 0
				} else if rand_num < 85 {
					etype = 1
				} else {
					etype = 2
				}
				var ehp int
				if etype == 0 {
					ehp = 1
				} else {
					ehp = 3
				}

				game.enemies[i] = Enemy{
					pos:           rl.NewVector2(screenWidth/X_SCALE, 32+float32(rand.Intn(screenHeight/Y_SCALE-32-32-32))),
					speed:         -4 - float32(rand.Intn(3)),
					active:        true,
					types:         etype,
					hp:            ehp,
					count:         0,
					count2:        float32(rand.Intn(30*2+screenHeight/Y_SCALE-40*2) - 30*2),
					shootTimer:    0,
					nextShootTime: 5.0 / COUNT1S,
				}
				break
			}
		}

	}

	// 敵更新
	for i := range game.enemies {
		if !game.enemies[i].active {
			continue
		}
		//			var e Enemy
		e := &game.enemies[i]
		e.count += rate

		switch {
		case e.types == 0: // 通常敵
			e.pos.X -= enemyspeed

		case e.types == 1: // ヘリザコ - 勢いよく突っ込む
			//				static float dist_x = e.x - player_x
			if e.count < 24 { // 1段階：超急接近
				e.pos.X -= 6 * 2 * rate
				e.pos.Y += ((game.player.pos.Y + 8 - e.pos.Y) / 8) / 2 * rate
			} else if e.count < 49 { // 2段階：短くホバリング
				e.pos.X -= 0
			} else { // 3段階：右へ全力逃走
				e.pos.X += 6 * 2 * rate
			}

		case e.types == 2: // サインカーブ
			e.pos.X -= enemyspeed
			e.pos.Y = (e.count2 + float32(math.Sin(float64(e.count*0.12)))*55*2)
		}

		// 敵弾発射処理
		e.shootTimer += game.delta

		difficulty := float32(int(min(1, game.gameTime/180))) // * COUNT1S)))
		enemy_bullet_speed := (4 + difficulty*2)
		shoot_interval := ((82 - difficulty*36) - 5) / COUNT1S

		if e.shootTimer >= e.nextShootTime {

			dx := game.player.pos.X - e.pos.X
			dy := game.player.pos.Y - e.pos.Y

			//	            dx -= 4.0f

			var dist float32
			if math.Abs(float64(dx)) > math.Abs(float64(dy)) {
				dist = float32(math.Abs(float64(dx)))
			} else {
				dist = float32(math.Abs(float64(dy)))
			}
			if dist == 0 {
				dist = 1
			}

			// 弾を発射
			bulletspeed := float32(enemy_bullet_speed)

			dx = (dx * bulletspeed / dist)
			dy = (dy * bulletspeed / dist)
			dx = max(-3*2.0, dx)
			dx = min(dx, 4*2.0)
			dy = max(-4*2.0, dy)
			dy = min(dy, 4*2.0)

			for j := range game.enemybullets {
				if game.enemybullets[j].active == false {

					game.enemybullets[j] = EnemyBullet{
						pos: rl.NewVector2(e.pos.X+16,
							e.pos.Y+16),
						vx:     dx, // * bulletspeed - 1.0f*1,   // vx
						vy:     dy, // * bulletspeed     // vy
						active: true,
					}
					break
				}
			}

			// 次回の発射間隔を設定
			e.nextShootTime = float32(shoot_interval)

			e.shootTimer = 0.0
			//				e.count += rate
			//	            e.count++
		}

		//			e.pos.X += e.speed * rate
		if (e.pos.X < -32) || (e.pos.X > screenWidth/X_SCALE) {
			e.active = false
		}
	}

	// 自機弾・敵 衝突判定
	for i := range game.bullets {
		if !game.bullets[i].active {
			continue
		}
		bRect := rl.NewRectangle(game.bullets[i].pos.X, game.bullets[i].pos.Y, 16, 8)
		for j := range game.enemies {
			e := &game.enemies[j]
			if !e.active {
				continue
			}
			eRect := rl.NewRectangle(e.pos.X, e.pos.Y, 32, 32)
			if rl.CheckCollisionRecs(bRect, eRect) {
				game.bullets[i].active = false

				game.CreateParticles(e.pos.X+16, e.pos.Y+16, 8, 0) // 通常爆発

				e.hp--
				if e.hp <= 0 {

					// オプションアイテム出現（確率20%くらい）
					//				    if (rand.Intn(100) < 22 && Options.size() < MAX_OPTIONS) {
					if game.option_cooldown <= 0 {
						for i := range game.items {
							item := &game.items[i]
							if item.active {
								continue
							}
							item.pos.X = e.pos.X
							item.pos.Y = e.pos.Y
							item.timer = 300 // 約5秒で消える
							item.types = 1   // 1 = オプションアイテム
							item.active = true
							game.option_cooldown = 10
							break
						}
					} else {
						game.option_cooldown--
					}

					// シールドアイテム出現（確率12%程度）
					if rand.Intn(100) < 12 && !game.shield_active {
						for i := range game.items {
							item := &game.items[i]
							if item.active {
								continue
							}
							item.pos.X = e.pos.X
							item.pos.Y = e.pos.Y
							item.timer = 280.0
							item.types = 2 // 2 = シールド
							item.active = true
							break
						}
					}

					// ボムアイテム出現
					if rand.Intn(100) < 10 { // 約10%の確率
						for i := range game.items {
							item := &game.items[i]
							if item.active {
								continue
							}
							item.pos.X = e.pos.X
							item.pos.Y = e.pos.Y
							item.timer = 270.0
							item.types = 3 // 3 = ボム
							item.active = true
							break
						}
					}
					// === チェインアイテム出現 ===
					if rand.Intn(100) < 40 { // 40%くらいの確率で落とす
						for i := range game.chainitems {
							item := &game.chainitems[i]
							if item.active {
								continue
							}
							item.pos.X = e.pos.X
							item.pos.Y = e.pos.Y
							item.timer = 240.0
							item.active = true
							break
						}
					}

					e.active = false
					rl.PlaySound(game.explosionSound)
					game.score += 100
				}
				break
			}
		}
	}

	// 敵弾 vs 自機
	pRect := rl.NewRectangle(game.player.pos.X, game.player.pos.Y+6, 32, 32-6*2)
	for i := range game.enemybullets {
		it := &game.enemybullets[i]
		if !it.active {
			continue
		}
		eRect := rl.NewRectangle(it.pos.X, it.pos.Y, 8, 8)

		if rl.CheckCollisionRecs(pRect, eRect) {
			//        if (it.pos.X > player.pos.X && it.pos.X + 8 < player.pos.X + 32 &&
			//            it.pos.Y > player.pos.Y + 6 && it.pos.Y + 8 < player.pos.Y + 6 + 20) {

			if game.shield_active {
				game.shield_active = false                                    // シールド消費
				game.CreateParticles(game.player.pos.X+16, game.player.pos.Y+16, 18, 1) // 大きな爆発
			} else {
				game.lives--
				if game.lives <= 0 {
					game.gameOver = 1
					rl.StopMusicStream(game.bgm)
				}
			}
			it.active = false
			break
		}
	}

	// 敵弾移動&画面範囲外判定
	for i := range game.enemybullets {
		it := &game.enemybullets[i]
		if it.active {
			it.pos.X += it.vx * rate
			it.pos.Y += it.vy * rate

			if (it.pos.X < -32) || (it.pos.X > screenWidth/X_SCALE) || (it.pos.Y < 32) || (it.pos.Y > screenHeight/Y_SCALE) {
				it.active = false
			}
		}
	}

	// プレイヤーと敵の衝突
	pRect = rl.NewRectangle(game.player.pos.X, game.player.pos.Y+6, 32, 32-6*2)
	for j := range game.enemies {
		if !game.enemies[j].active {
			continue
		}
		eRect := rl.NewRectangle(game.enemies[j].pos.X, game.enemies[j].pos.Y, 32, 32)
		if rl.CheckCollisionRecs(pRect, eRect) {
			if game.shield_active {
				game.shield_active = false                                    // シールド消費
				game.CreateParticles(game.player.pos.X+16, game.player.pos.Y+16, 18, 1) // 大きな爆発
			} else {
				game.lives--
				if game.lives <= 0 {
					game.gameOver = 1
					rl.StopMusicStream(game.bgm)
				}
			}
			game.enemies[j].active = false
		}
		break
	}

	// アイテム更新
	for i := range game.items {
		it := &game.items[i]
		if !it.active {
			continue
		}
		switch {
		case it.types == 1:
			it.pos.X -= 2.0 * rate // 左に流れる

		case it.types == 2:
			it.pos.X -= 4.0 * rate // 左に流れる

		case it.types == 3:
			it.pos.X -= 4.0 * rate // 左に流れる
		}
		it.timer -= game.delta

		// 自機との当たり判定
		if math.Abs(float64(it.pos.X-game.player.pos.X)) < 44-16 && math.Abs(float64(it.pos.Y-game.player.pos.Y)) < 44-16 {

			if it.types == 1 && game.optionnum < MAXOPTIONS { // オプションアイテム
				var offset float32
				if game.optionnum == 0 {
					offset = 25.0
				} else {
					offset = -25.0
				}
				opt := &game.options[game.optionnum]
				opt.offset_y = offset * 2
				opt.pos.X = 0 //playerX + 20
				opt.pos.Y = 0 //playerY + 16 + offset
				//                opt.angle = 0.0
				game.optionnum++
			} else if it.types == 2 { // シールド
				game.shield_active = true
				//                shield_timer = SHIELD_DURATION
			} else if it.types == 3 { // 3 = ボムアイテム
				game.bomb_stock = min(3, game.bomb_stock+1)
			}

			rl.PlaySound(game.laserSound)

			it.active = false
			continue
		}

		// 画面外 or 時間切れ
		if it.pos.X < -40 || it.timer <= 0 {
			it.active = false
		}
	}

	// パーティクル更新
	for i := range game.particles {
		it := &game.particles[i]
		if it.life <= 0 {
			continue
		}
		it.pos.X += it.vx * rate
		it.pos.Y += it.vy * rate
		//        it->vx *= 0.96f      // 少し減速（空気抵抗）
		//        it->vy *= 0.96f

		damping := float32(math.Pow(0.96, float64(rate)))
		it.vx *= damping
		it.vy *= damping

		it.life = it.life - rate
	}

	// ボム更新
	if game.bomb_active {
		game.bomb_timer -= game.delta
		if game.bomb_timer <= 0.0 {
			game.bomb_active = false
		}
	}

	// チェインアイテム更新
	for i := range game.chainitems {
		it := &game.chainitems[i]
		if !it.active {
			continue
		}
		it.pos.X -= 4.0 * rate // 左に流れる
		it.timer -= game.delta

		// 自機取得判定
		if math.Abs(float64(it.pos.X-game.player.pos.X)) < 44-16 && math.Abs(float64(it.pos.Y-game.player.pos.Y)) < 44-16 {
			game.chain_count++
			game.chain_timer = 240 / COUNT1S // チェイン持続時間リセット
			game.score += game.chain_count * 100  // チェイン数に応じたボーナス

			it.active = false
			rl.PlaySound(game.laserSound)
			continue
		}

		// 時間切れ or 画面外
		if it.timer <= 0.0 || it.pos.X < -20 {
			game.chain_count = 0
			it.active = false
		}
	}

	// チェインタイマー減少
	if game.chain_timer > 0.0 {
		game.chain_timer -= game.delta
		if game.chain_timer <= 0.0 {
			game.chain_count = 0
		}
	}

	if game.gameOver != 0 && game.score > game.high_score {
		game.high_score = game.score
	}
	// 不要なオブジェクトを削除 (リストをクリーンアップ)
	//	bullets = removeInactiveBullets(bullets)
	//	enemies = removeInactiveEnemies(enemies)
}

/*
func removeInactiveBullets(bullets []Bullet) []Bullet {
	n := 0
	for i := range bullets {
		if bullets[i].active {
			bullets[n] = bullets[i]
			n++
		}
	}
	return bullets[:n]
}

func removeInactiveEnemies(list []Enemy) []Enemy {
	newList := make([]Enemy, 0, len(list))
	for _, e := range list {
		if e.active {
			newList = append(newList, e)
		}
	}
	return newList
}*/

func (game *Game) put_strings(x float32, y float32, text string) {
	len := len(text)
	for i := 0; i < len; i++ {
		if text[i] != ' ' {
			pat_no := text[i] - '0'

			rotation := float32(0)

			destRect := rl.NewRectangle(x, y, 16*X_SCALE-1, 16*Y_SCALE-1)
			sourceRect := rl.NewRectangle(16.0*float32(pat_no%16), 16.0*float32(pat_no/16), 16.0, 16.0)
			origin := rl.NewVector2(0, 0)

			rl.DrawTexturePro(game.fontTex, sourceRect, destRect, origin, rotation, rl.White)
		}
		x += FONT_SIZE
	}
}

func (game *Game) put_strings_num(x float32, y float32, str string, num int, digit int) {
	var text string
	len := len(str)
	i := digit
	j := num
	game.put_strings(x, y, str)

	for i > 0 {
		i--
		text = string(rune(j%10+'0')) + text
		j /= 10
	}
	//    text[digit] = null
	game.put_strings(x+float32(len*FONT_SIZE), y, text)
}

func (game *Game) CreateParticles(x float32, y float32, count int, types int) {
	for i := 0; i < count; i++ {
		for j := range game.particles {
			p := &game.particles[j]
			if p.life > 0 {
				continue
			}
			p.pos.X = x
			p.pos.Y = y
			p.vx = float32(rand.Intn(100)-50) * 0.12 // -6.0 ~ +6.0
			p.vy = float32(rand.Intn(100)-50) * 0.12
			p.life = 20.0 + float32(rand.Intn(25))
			//        p.color_index = rand.Intn(5)
			break
		}
	}
}

func (game *Game) put_sprite(x float32, y float32, pat_no float32) {
	var rotation float32 = 0.0
	//    var destRect rl.Rectangle

	destRect := rl.NewRectangle(x*X_SCALE, y*Y_SCALE, 32*X_SCALE-1, 32*Y_SCALE-1)
	sourceRect := rl.NewRectangle(32*pat_no, 0, 32, 32)
	origin := rl.NewVector2(0, 0) //destRect.width/2, destRect.height/2 }

	rl.DrawTexturePro(game.chrTex, sourceRect, destRect, origin, rotation, rl.White)

}

func (game *Game) Draw() {
	//	rl.BeginDrawing()
	rl.ClearBackground(rl.Black) //NewColor(10, 10, 30, 255)) // 暗い宇宙背景

	// 背景スクロール風 (シンプルな線)
	/*	for i := 0; i < 20; i++ {
		x := float32((int(rl.GetTime()*80)+i*50)%(screenWidth+100)) - 50
		rl.DrawLine(int32(x), 0, int32(x), screenHeight, rl.NewColor(50, 50, 80, 100))
	}*/

	for i := range game.star {
		rl.DrawCircle(int32(game.star[i].x)*X_SCALE, int32(game.star[i].y)*Y_SCALE, 1.5, rl.White)
	}

	// パーティクル描画
	for i := range game.particles {
		p := &game.particles[i]
		if p.life > 0 {
			rl.DrawCircle(int32(p.pos.X*X_SCALE), int32(p.pos.Y*Y_SCALE), 1.5*2, rl.Yellow)
			//            game.put_sprite(p.pos.X, p.pos.Y, 5)
		}
	}

	// チェインアイテム描画
	for i := range game.chainitems {
		if game.chainitems[i].active {
			game.put_sprite(game.chainitems[i].pos.X, game.chainitems[i].pos.Y, 3) // 3番パターンにチェインアイテムの画像を入れる
		}
	}

	for i := range game.items {
		if !game.items[i].active {
			continue
		}
		switch {
		case game.items[i].types == 1:
			game.put_sprite(game.items[i].pos.X, game.items[i].pos.Y, 8)
		case game.items[i].types == 2:
			game.put_sprite(game.items[i].pos.X, game.items[i].pos.Y, 7)
		case game.items[i].types == 3:
			game.put_sprite(game.items[i].pos.X, game.items[i].pos.Y, 9)
		}
	}

	// オプション描画
	for i := 0; i < game.optionnum; i++ { //range game.options {
		opt := &game.options[i]
		//		if opt.active {
		game.put_sprite(opt.pos.X, opt.pos.Y, 10) // 10 = オプションのパターン番号
		//		}
	}

	// 敵弾
	for _, b := range game.enemybullets {
		if b.active {
			game.put_sprite(b.pos.X, b.pos.Y, 0)
		}
	}

	// 敵
	for _, e := range game.enemies {
		if e.active {
			//			rl.DrawRectangleV(e.pos, rl.NewVector2(50, 30), rl.Red)
			//			rl.DrawRectangle(int32(e.pos.X+10), int32(e.pos.Y+8), 30, 14, rl.Maroon)
			game.put_sprite(e.pos.X, e.pos.Y, 2)
		}
	}

	// 自機弾
	for _, b := range game.bullets {
		if b.active {
			//			rl.DrawRectangleV(b.pos, rl.NewVector2(22, 6), rl.Yellow)
			//			rl.DrawRectangle(int32(b.pos.X+18), int32(b.pos.Y-2), 8, 10, rl.Orange)
			game.put_sprite(b.pos.X, b.pos.Y, 4)
		}
	}

	// シールド描画
	if game.shield_active {
		game.put_sprite(game.player.pos.X, game.player.pos.Y, 6)
	}

	// プレイヤー (三角形っぽく)
	/*	rl.DrawTriangle(
			rl.NewVector2(player.pos.X+50, player.pos.Y+15),
			rl.NewVector2(player.pos.X, player.pos.Y),
			rl.NewVector2(player.pos.X, player.pos.Y+30),
			rl.SkyBlue,
		)
		rl.DrawRectangle(int32(player.pos.X+10), int32(player.pos.Y+10), 30, 10, rl.Blue) // 本体*/
	game.put_sprite(game.player.pos.X, game.player.pos.Y, 1)

	// UI
	//	rl.DrawText(fmt.Sprintf("SCORE: %d", score), 20, 20, 30, rl.White)
	//	rl.DrawText("WASD / Arrow: Move    Space: Shoot", 20, screenHeight-40, 20, rl.LightGray)

	if game.score >= game.high_score {
		game.put_strings_num( 0, 0, "HIGH  ", game.score, 7)
	} else {
		game.put_strings_num( 0, 0, "SCORE ", game.score, 7)
	}
	if game.easy_mode == true {
		game.put_strings_num( 0, 2*FONT_SIZE, "LIVES ", game.lives, 1)
	}
	game.put_strings_num( 0, 1*FONT_SIZE, "BOMB  ", game.bomb_stock, 1)

	game.put_strings_num( 16*FONT_SIZE, 0, "COUNT ", int(game.gameTime), 7)

	if game.chain_count > 0 {
		game.put_strings_num( 16*FONT_SIZE, 1*FONT_SIZE, "CHAIN ", game.chain_count, 3)
	}

	if game.gameOver != 0 {
		//		rl.DrawText("GAME OVER", screenWidth/2-150, screenHeight/2-50, 60, rl.Red)
		//		rl.DrawText("ESC to End", screenWidth/2-80, screenHeight/2+20, 30, rl.White)
		game.put_strings(11*FONT_SIZE, 12*FONT_SIZE, "GAME OVER")
		game.put_strings_num( 7*FONT_SIZE, 15*FONT_SIZE, "HIGH SCORE ",game. high_score, 7)

		game.put_strings(7*FONT_SIZE, 18*FONT_SIZE, "PRESS A TO RESTART")
	}

	//	rl.EndDrawing()
}
