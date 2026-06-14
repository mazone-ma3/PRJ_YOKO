package main

import (
	//	"fmt"
	"math"
	"math/rand"
	"time"

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
	Pos   rl.Vector2
	Speed float32
}

type Bullet struct {
	Pos    rl.Vector2
	Active bool
	Speed  float32
}

type EnemyBullet struct {
	Pos    rl.Vector2
	vx     float32
	vy     float32
	Active bool
}

type Enemy struct {
	Pos rl.Vector2
	//	Speed  float32
	Active        bool
	Type          int
	HP            int
	count         float32
	count2        float32
	shootTimer    float32
	nextShootTime float32
	Speed         float32
}

type Option struct {
	Pos      rl.Vector2
	offset_y float32
	//	Active   bool
}

type Item struct {
	Pos    rl.Vector2
	timer  float32
	types  int
	Active bool
}

type ChainItem struct {
	Pos    rl.Vector2
	timer  float32
	Active bool
}

type Particle struct {
	Pos  rl.Vector2
	vx   float32
	vy   float32
	life float32
}

type Star struct {
	x, y, baseSpeed, speed, size float32
}

var (
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
)

func main() {
	rand.Seed(time.Now().UnixNano())

	rl.SetConfigFlags(rl.FlagWindowResizable | rl.FlagVsyncHint)

	rl.InitWindow(screenWidth, screenHeight, "Go-raylib 横スクロールシューティング")
	defer rl.CloseWindow()

	target := rl.LoadRenderTexture(screenWidth, screenHeight)
	rl.SetTextureFilter(target.Texture, rl.FilterPoint)

	chrTex = rl.LoadTexture("yokosht.png") // 画像がなければ後で矩形で代用
	fontTex = rl.LoadTexture("FONTYOKO.png")

	//    rl.SetTargetFPS(60)

	rl.InitAudioDevice()
	laserSound = rl.LoadSound("laser.wav")
	explosionSound = rl.LoadSound("explosion.wav")
	bgm = rl.LoadMusicStream("bgm.mp3")

	for i := 0; i < 80; i++ {
		star = append(star, Star{
			x:     float32(rl.GetRandomValue(0, screenWidth/X_SCALE-1)),
			y:     float32(rl.GetRandomValue(0, screenHeight/Y_SCALE-1)),
			speed: 0.5 + float32(rl.GetRandomValue(0, 99)/30), // static_cast<float>(rand() % 100) / 30.0f
		}) //    speed : baseSpeed   // もし個別速度も欲しい場合
		if star[i].speed > 2 {
			star[i].size = 2
		} else {
			star[i].size = 1
		}
		//        star[i] = s//.Add(new s)
	}

	ResetGame()
	gameOver = 1
	high_score = 5000

	//	rl.PlayMusicStream(bgm)

	for !rl.WindowShouldClose() {
		rl.UpdateMusicStream(bgm)

		if rl.IsKeyPressed(rl.KeyF11) {
			rl.ToggleFullscreen()
		}

		scale := float32(min(rl.GetScreenWidth()/screenWidth, rl.GetScreenHeight()/screenHeight))

		destRec := rl.NewRectangle((float32(rl.GetScreenWidth())-(screenWidth*scale))*0.5, (float32(rl.GetScreenHeight())-(screenHeight*scale))*0.5, screenWidth*scale, screenHeight*scale)

		delta = rl.GetFrameTime()
		//		if gameOver != 0 {
		update()
		//		}
		rl.BeginTextureMode(target)
		draw()
		rl.EndTextureMode()

		rl.BeginDrawing()
		rl.ClearBackground(rl.Black) // フルスクリーン時の「黒帯」になる部分の色

		// レンダーテクスチャは上下の座標が反転しているため、sourceのheightをマイナスにする必要があります
		sourceRec := rl.NewRectangle(0.0, 0.0, float32(target.Texture.Width), -float32(target.Texture.Height))
		origin := rl.NewVector2(0.0, 0.0)

		// 計算した位置・サイズ（destRec）で綺麗に拡大描画
		rl.DrawTexturePro(target.Texture, sourceRec, destRec, origin, 0.0, rl.White)

		// デバッグ情報（実際の現在のウィンドウサイズを表示）
		//            DrawFPS(10, 10);
		//            DrawText("F: Toggle Fullscreen", 10, 30, 20, GREEN);
		rl.EndDrawing()
	}
	rl.StopMusicStream(bgm)

	rl.UnloadTexture(fontTex)
	rl.UnloadTexture(chrTex)

	rl.UnloadSound(explosionSound)
	rl.UnloadSound(laserSound)

	rl.UnloadMusicStream(bgm)
}

func ResetGame() {
	// プレイヤー初期化
	player = Player{
		Pos:   rl.NewVector2(60, 160), //float32(screenHeight)/2),
		Speed: 4,
	}
	for i := range bullets {
		bullets[i].Active = false
	}
	for i := range enemies {
		enemies[i].Active = false
	}

	for i := range enemybullets {
		enemybullets[i].Active = false
	}

	/*	for i := range options {
		options[i].Active = false
	}*/
	for i := range particles {
		particles[i].life = 0
	}

	for i := range items {
		items[i].Active = false
	}
	for i := range chainitems {
		chainitems[i].Active = false
	}

	optionnum = 0

	shootCooldown = 0
	score = 0

	enemySpawnTimer = 0.0
	gameOver = 0
	shield_active = false

	//	high_score = 5000
	lives = 0
	gameTime = 0
	chain_count = 0

	bomb_timer = 0
	option_cooldown = 10

	if easy_mode {
		lives = 3
	} else {
		lives = 1
	}
	bomb_stock = 0
}

func UseBomb() {
	if bomb_stock <= 0 || bomb_active {
		return
	}

	bomb_stock--
	bomb_active = true
	//    bomb_timer = BOMB_DURATION;

	// 敵と敵弾を全滅
	for i := range enemies {
		enemies[i].Active = false
	}
	for i := range enemybullets {
		enemybullets[i].Active = false
	}

	// 大量の破片を発生
	CreateParticles(player.Pos.X+16, player.Pos.Y+16, 45, 1) // 大爆発

	// 画面全体に破片を散らす
	for i := 0; i < 60; i++ {
		rx := float32(rand.Intn(screenWidth))
		ry := float32(rand.Intn(screenHeight))
		CreateParticles(rx, ry, 6, 1)
	}

	score += 200
	rl.PlaySound(explosionSound)
}

func update() {

	gamepad := int32(0)

	if gameOver == 1 {
		gameOver = 2
	}
	if gameOver == 2 {
		if rl.IsKeyPressed(rl.KeySpace) || rl.IsKeyPressed(rl.KeyZ) || rl.IsKeyPressed(rl.KeyR) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonPressed(gamepad, rl.GamepadButtonRightFaceDown)) {
			easy_mode = false
			rl.PlayMusicStream(bgm)
			ResetGame()
		} else if rl.IsKeyPressed(rl.KeyX) || rl.IsKeyPressed(rl.KeyB) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonPressed(gamepad, rl.GamepadButtonRightFaceRight)) {
			easy_mode = true
			rl.PlayMusicStream(bgm)
			ResetGame()
		}
	}

	// 星移動
	for i := range star {
		s := &star[i]
		s.x -= s.speed * delta * COUNT1S // これでFPSが60の時に元の速度と同じになる
		if s.x < 0 {
			s.x = screenWidth / X_SCALE
		}
	}

	if gameOver != 0 {
		return
	}

	gameTime += delta

	//moveSpeed := 4.0 * COUNT1S * delta
	enemySpeed := 4.0 * COUNT1S * delta
	//enemySpeed2 := 5.0 * COUNT1S * delta

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
		player.Pos.Y -= player.Speed * COUNT1S * delta
	}
	if rl.IsKeyDown(rl.KeyDown) || rl.IsKeyDown(rl.KeyS) || (axisY > 0.2) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonLeftFaceDown)) {
		player.Pos.Y += player.Speed * COUNT1S * delta
	}
	if rl.IsKeyDown(rl.KeyLeft) || rl.IsKeyDown(rl.KeyA) || (axisX < -0.2) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonLeftFaceLeft)) {
		player.Pos.X -= player.Speed * COUNT1S * delta
	}
	if rl.IsKeyDown(rl.KeyRight) || rl.IsKeyDown(rl.KeyD) || (axisX > 0.2) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonLeftFaceRight)) {
		player.Pos.X += player.Speed * COUNT1S * delta
	}

	// 画面端制限
	if player.Pos.X < 0 {
		player.Pos.X = 0
	}
	if player.Pos.X > screenWidth/X_SCALE-40 {
		player.Pos.X = screenWidth/X_SCALE - 40
	}
	if player.Pos.Y < 0 {
		player.Pos.Y = 0
	}
	if player.Pos.Y > screenHeight/Y_SCALE-32 {
		player.Pos.Y = screenHeight/Y_SCALE - 32
	}

	// オプション更新
	for i := 0; i < optionnum; i++ {
		//        opt.angle += 0.08f * COUNT1S * m_deltaTime;   // 回転速度
		opt := &options[i]
		// 滑らかに追従
		opt.Pos.X += ((player.Pos.X + 16) - opt.Pos.X) / 4 * delta * COUNT1S
		opt.Pos.Y += ((player.Pos.Y + opt.offset_y) - opt.Pos.Y) / 4 * delta * COUNT1S
		//		float t = 1.0f - pow(1.0f - 0.25f, m_deltaTime * COUNT1S);
		//		opt.x = std::lerp(opt.x, playerX + 16, t);
		//		opt.y = std::lerp(opt.y, playerY + opt.offset_y, t);
	}

	// 射撃 (Spaceキー)
	shootCooldown += delta
	if (rl.IsKeyDown(rl.KeySpace) || rl.IsKeyDown(rl.KeyZ) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonRightFaceDown))) && (shootCooldown >= float32(8)/COUNT1S) {
		/*		bullets = append(bullets, Bullet{
				Pos:    rl.NewVector2(player.Pos.X+32, player.Pos.Y+12),
				Speed:  12,
				Active: true,
			})*/
		for i := range bullets {
			if bullets[i].Active == false {
				bullets[i] = Bullet{
					Pos:    rl.NewVector2(player.Pos.X+32, player.Pos.Y+12),
					Speed:  12,
					Active: true,
				}
				break
			}
		}
		for j := 0; j < optionnum; j++ {
			for i := range bullets {
				if bullets[i].Active == false {
					bullets[i] = Bullet{
						Pos:    rl.NewVector2(options[j].Pos.X+8, options[j].Pos.Y+12),
						Speed:  12,
						Active: true,
					}
					break
				}
			}
		}
		shootCooldown = 0 //(8 / COUNT1S) // 連射速度
	}

	if (rl.IsKeyPressed(rl.KeyX) || rl.IsKeyPressed(rl.KeyB) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonPressed(gamepad, rl.GamepadButtonRightFaceRight))) && bomb_stock > 0 && !bomb_active {
		UseBomb()
	}

	// 自機弾更新
	for i := range bullets {
		if bullets[i].Active {
			bullets[i].Pos.X += bullets[i].Speed * COUNT1S * delta
			if bullets[i].Pos.X > screenWidth {
				bullets[i].Active = false
			}
		}
	}

	enemySpawnTimer += delta
	baseInterval := float32(50.0 - (score / 250.0))                   // scoreが増えるほど短く
	spawnInterval := max(float32(18.0)/COUNT1S, baseInterval/COUNT1S) // フレーム→秒に変換

	// 敵生成(スポーン) (ランダム)
	//	if rand.Intn(40) == 0 { // 調整可能
	if enemySpawnTimer >= spawnInterval {
		/*		enemies = append(enemies, Enemy{
				Pos:    rl.NewVector2(screenWidth+30, float32(rand.Intn(screenHeight-40))),
				Speed:  -4 - float32(rand.Intn(3)),
				Active: true,
				Type:	0,
				HP:     1,
			})*/
		enemySpawnTimer = 0

		for i := range enemies {
			if enemies[i].Active == false {

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

				enemies[i] = Enemy{
					Pos:           rl.NewVector2(screenWidth/X_SCALE, 32+float32(rand.Intn(screenHeight/Y_SCALE-32-32-32))),
					Speed:         -4 - float32(rand.Intn(3)),
					Active:        true,
					Type:          etype,
					HP:            ehp,
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
	for i := range enemies {
		if !enemies[i].Active {
			continue
		}
		//			var e Enemy
		e := &enemies[i]
		e.count += delta * COUNT1S

		switch {
		case e.Type == 0: // 通常敵
			e.Pos.X -= enemySpeed

		case e.Type == 1: // ヘリザコ - 勢いよく突っ込む
			//				static float dist_x = e.x - player_x
			if e.count < 24 { // 1段階：超急接近
				e.Pos.X -= 6 * 2 * delta * COUNT1S
				e.Pos.Y += ((player.Pos.Y + 8 - e.Pos.Y) / 8) / 2 * delta * COUNT1S
			} else if e.count < 49 { // 2段階：短くホバリング
				e.Pos.X -= 0
			} else { // 3段階：右へ全力逃走
				e.Pos.X += 6 * 2 * delta * COUNT1S
			}

		case e.Type == 2: // サインカーブ
			e.Pos.X -= enemySpeed
			e.Pos.Y = (e.count2 + float32(math.Sin(float64(e.count*0.12)))*55*2)
		}

		// 敵弾発射処理
		e.shootTimer += delta

		difficulty := min(1, gameTime/180) // * COUNT1S)))
		enemy_bullet_speed := (4 + difficulty*2)
		shoot_interval := ((82 - difficulty*36) - 5) / COUNT1S

		if e.shootTimer >= e.nextShootTime {

			dx := player.Pos.X - e.Pos.X
			dy := player.Pos.Y - e.Pos.Y

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
			bulletSpeed := float32(enemy_bullet_speed)

			dx = (dx * bulletSpeed / dist)
			dy = (dy * bulletSpeed / dist)
			dx = max(-3*2.0, dx)
			dx = min(dx, 4*2.0)
			dy = max(-4*2.0, dy)
			dy = min(dy, 4*2.0)

			for j := range enemybullets {
				if enemybullets[j].Active == false {

					enemybullets[j] = EnemyBullet{
						Pos: rl.NewVector2(e.Pos.X+16,
							e.Pos.Y+16),
						vx:     dx, // * bulletSpeed - 1.0f*1,   // vx
						vy:     dy, // * bulletSpeed     // vy
						Active: true,
					}
					break
				}
			}

			// 次回の発射間隔を設定
			e.nextShootTime = float32(shoot_interval)

			e.shootTimer = 0.0
			//				e.count += delta * COUNT1S
			//	            e.count++
		}

		//			e.Pos.X += e.Speed * COUNT1S * delta
		if (e.Pos.X < -32) || (e.Pos.X > screenWidth) {
			e.Active = false
		}
	}

	// 自機弾・敵 衝突判定
	for i := range bullets {
		if !bullets[i].Active {
			continue
		}
		bRect := rl.NewRectangle(bullets[i].Pos.X, bullets[i].Pos.Y, 16, 8)
		for j := range enemies {
			e := &enemies[j]
			if !e.Active {
				continue
			}
			eRect := rl.NewRectangle(e.Pos.X, e.Pos.Y, 32, 32)
			if rl.CheckCollisionRecs(bRect, eRect) {
				bullets[i].Active = false

				CreateParticles(e.Pos.X+16, e.Pos.Y+16, 8, 0) // 通常爆発

				e.HP--
				if e.HP <= 0 {

					// オプションアイテム出現（確率20%くらい）
					//				    if (rand.Intn(100) < 22 && Options.size() < MAX_OPTIONS) {
					if option_cooldown <= 0 {
						for i := range items {
							item := &items[i]
							if item.Active {
								continue
							}
							item.Pos.X = e.Pos.X
							item.Pos.Y = e.Pos.Y
							item.timer = 300 // 約5秒で消える
							item.types = 1   // 1 = オプションアイテム
							item.Active = true
							option_cooldown = 10
							break
						}
					} else {
						option_cooldown--
					}

					// シールドアイテム出現（確率12%程度）
					if rand.Intn(100) < 12 && !shield_active {
						for i := range items {
							item := &items[i]
							if item.Active {
								continue
							}
							item.Pos.X = e.Pos.X
							item.Pos.Y = e.Pos.Y
							item.timer = 280.0
							item.types = 2 // 2 = シールド
							item.Active = true
							break
						}
					}

					// ボムアイテム出現
					if rand.Intn(100) < 10 { // 約10%の確率
						for i := range items {
							item := &items[i]
							if item.Active {
								continue
							}
							item.Pos.X = e.Pos.X
							item.Pos.Y = e.Pos.Y
							item.timer = 270.0
							item.types = 3 // 3 = ボム
							item.Active = true
							break
						}
					}
					// === チェインアイテム出現 ===
					if rand.Intn(100) < 40 { // 40%くらいの確率で落とす
						for i := range chainitems {
							item := &chainitems[i]
							if item.Active {
								continue
							}
							item.Pos.X = e.Pos.X
							item.Pos.Y = e.Pos.Y
							item.timer = 240.0
							item.Active = true
							break
						}
					}

					e.Active = false
					rl.PlaySound(explosionSound)
					score += 100
				}
				break
			}
		}
	}

	// 敵弾 vs 自機
	pRect := rl.NewRectangle(player.Pos.X, player.Pos.Y+6, 32, 32-6*2)
	for i := range enemybullets {
		it := &enemybullets[i]
		if !it.Active {
			continue
		}
		eRect := rl.NewRectangle(it.Pos.X, it.Pos.Y, 8, 8)

		if rl.CheckCollisionRecs(pRect, eRect) {
			//        if (it.Pos.X > player.Pos.X && it.Pos.X + 8 < player.Pos.X + 32 &&
			//            it.Pos.Y > player.Pos.Y + 6 && it.Pos.Y + 8 < player.Pos.Y + 6 + 20) {

			if shield_active {
				shield_active = false                                    // シールド消費
				CreateParticles(player.Pos.X+16, player.Pos.Y+16, 18, 1) // 大きな爆発
			} else {
				lives--
				if lives <= 0 {
					gameOver = 1
					rl.StopMusicStream(bgm)
				}
			}
			it.Active = false
			break
		}
	}

	// 敵弾移動&画面範囲外判定
	for i := range enemybullets {
		it := &enemybullets[i]
		if it.Active {
			it.Pos.X += it.vx * delta * COUNT1S
			it.Pos.Y += it.vy * delta * COUNT1S

			if (it.Pos.X < -32) || (it.Pos.X > screenWidth) || (it.Pos.Y < 32) || (it.Pos.Y > screenHeight) {
				it.Active = false
			}
		}
	}

	// プレイヤーと敵の衝突
	pRect = rl.NewRectangle(player.Pos.X, player.Pos.Y+6, 32, 32-6*2)
	for j := range enemies {
		if !enemies[j].Active {
			continue
		}
		eRect := rl.NewRectangle(enemies[j].Pos.X, enemies[j].Pos.Y, 32, 32)
		if rl.CheckCollisionRecs(pRect, eRect) {
			if shield_active {
				shield_active = false                                    // シールド消費
				CreateParticles(player.Pos.X+16, player.Pos.Y+16, 18, 1) // 大きな爆発
			} else {
				lives--
				if lives <= 0 {
					gameOver = 1
					rl.StopMusicStream(bgm)
				}
			}
			enemies[j].Active = false
		}
		break
	}

	// アイテム更新
	for i := range items {
		it := &items[i]
		if !it.Active {
			continue
		}
		switch {
		case it.types == 1:
			it.Pos.X -= 2.0 * COUNT1S * delta // 左に流れる

		case it.types == 2:
			it.Pos.X -= 4.0 * COUNT1S * delta // 左に流れる

		case it.types == 3:
			it.Pos.X -= 4.0 * COUNT1S * delta // 左に流れる
		}
		it.timer -= delta

		// 自機との当たり判定
		if math.Abs(float64(it.Pos.X-player.Pos.X)) < 44-16 && math.Abs(float64(it.Pos.Y-player.Pos.Y)) < 44-16 {

			if it.types == 1 && optionnum < MAXOPTIONS { // オプションアイテム
				var offset float32
				if optionnum == 0 {
					offset = 25.0
				} else {
					offset = -25.0
				}
				opt := &options[optionnum]
				opt.offset_y = offset * 2
				opt.Pos.X = 0 //playerX + 20;
				opt.Pos.Y = 0 //playerY + 16 + offset;
				//                opt.angle = 0.0f;
				optionnum++
			} else if it.types == 2 { // シールド
				shield_active = true
				//                shield_timer = SHIELD_DURATION;
			} else if it.types == 3 { // 3 = ボムアイテム
				bomb_stock = min(3, bomb_stock+1)
			}

			rl.PlaySound(laserSound)

			it.Active = false
			continue
		}

		// 画面外 or 時間切れ
		if it.Pos.X < -40 || it.timer <= 0 {
			it.Active = false
		}
	}

	// パーティクル更新
	for i := range particles {
		it := &particles[i]
		if it.life <= 0 {
			continue
		}
		it.Pos.X += it.vx * delta * COUNT1S
		it.Pos.Y += it.vy * delta * COUNT1S
		//        it->vx *= 0.96f      // 少し減速（空気抵抗）
		//        it->vy *= 0.96f

		damping := float32(math.Pow(0.96, float64(delta*COUNT1S)))
		it.vx *= damping
		it.vy *= damping

		it.life = it.life - delta*COUNT1S
	}

	// ボム更新
	if bomb_active {
		bomb_timer -= delta
		if bomb_timer <= 0.0 {
			bomb_active = false
		}
	}

	// チェインアイテム更新
	for i := range chainitems {
		it := &chainitems[i]
		if !it.Active {
			continue
		}
		it.Pos.X -= 4.0 * delta * COUNT1S // 左に流れる
		it.timer -= delta

		// 自機取得判定
		if math.Abs(float64(it.Pos.X-player.Pos.X)) < 44-16 && math.Abs(float64(it.Pos.Y-player.Pos.Y)) < 44-16 {
			chain_count++
			chain_timer = 240 / COUNT1S // チェイン持続時間リセット
			score += chain_count * 100  // チェイン数に応じたボーナス

			it.Active = false
			rl.PlaySound(laserSound)
			continue
		}

		// 時間切れ or 画面外
		if it.timer <= 0.0 || it.Pos.X < -20 {
			chain_count = 0
			it.Active = false
		}
	}

	// チェインタイマー減少
	if chain_timer > 0.0 {
		chain_timer -= delta
		if chain_timer <= 0.0 {
			chain_count = 0
		}
	}

	if gameOver != 0 && score > high_score {
		high_score = score
	}
	// 不要なオブジェクトを削除 (リストをクリーンアップ)
	//	bullets = removeInactiveBullets(bullets)
	//	enemies = removeInactiveEnemies(enemies)
}

/*
func removeInactiveBullets(bullets []Bullet) []Bullet {
	n := 0
	for i := range bullets {
		if bullets[i].Active {
			bullets[n] = bullets[i]
			n++
		}
	}
	return bullets[:n]
}

func removeInactiveEnemies(list []Enemy) []Enemy {
	newList := make([]Enemy, 0, len(list))
	for _, e := range list {
		if e.Active {
			newList = append(newList, e)
		}
	}
	return newList
}*/

func put_strings(x float32, y float32, text string) {
	len := len(text)
	for i := 0; i < len; i++ {
		if text[i] != ' ' {
			pat_no := text[i] - '0'

			rotation := float32(0)

			destRect := rl.NewRectangle(x, y, 16*X_SCALE-1, 16*Y_SCALE-1)
			sourceRect := rl.NewRectangle(16.0*float32(pat_no%16), 16.0*float32(pat_no/16), 16.0, 16.0)
			origin := rl.NewVector2(0, 0)

			rl.DrawTexturePro(fontTex, sourceRect, destRect, origin, rotation, rl.White)
		}
		x += FONT_SIZE
	}
}

func put_strings_num(x float32, y float32, str string, num int, digit int) {
	var text string
	len := len(str)
	i := digit
	j := num
	put_strings(x, y, str)

	for i > 0 {
		i--
		text = string(rune(j%10+'0')) + text
		j /= 10
	}
	//    text[digit] = null
	put_strings(x+float32(len*FONT_SIZE), y, text)
}

func CreateParticles(x float32, y float32, count int, types int) {
	for i := 0; i < count; i++ {
		for j := range particles {
			p := &particles[j]
			if p.life > 0 {
				continue
			}
			p.Pos.X = x
			p.Pos.Y = y
			p.vx = float32(rand.Intn(100)-50) * 0.12 // -6.0 ~ +6.0
			p.vy = float32(rand.Intn(100)-50) * 0.12
			p.life = 20.0 + float32(rand.Intn(25))
			//        p.color_index = rand.Intn(5)
			break
		}
	}
}

func put_sprite(x float32, y float32, pat_no float32) {
	var rotation float32 = 0.0
	//    var destRect rl.Rectangle

	destRect := rl.NewRectangle(x*X_SCALE, y*Y_SCALE, 32*X_SCALE-1, 32*Y_SCALE-1)
	sourceRect := rl.NewRectangle(32*pat_no, 0, 32, 32)
	origin := rl.NewVector2(0, 0) //destRect.width/2, destRect.height/2 }

	rl.DrawTexturePro(chrTex, sourceRect, destRect, origin, rotation, rl.White)

}

func draw() {
	//	rl.BeginDrawing()
	rl.ClearBackground(rl.Black) //NewColor(10, 10, 30, 255)) // 暗い宇宙背景

	// 背景スクロール風 (シンプルな線)
	/*	for i := 0; i < 20; i++ {
		x := float32((int(rl.GetTime()*80)+i*50)%(screenWidth+100)) - 50
		rl.DrawLine(int32(x), 0, int32(x), screenHeight, rl.NewColor(50, 50, 80, 100))
	}*/

	for i := range star {
		rl.DrawCircle(int32(star[i].x)*X_SCALE, int32(star[i].y)*Y_SCALE, 1.5, rl.White)
	}

	// パーティクル描画
	for i := range particles {
		p := &particles[i]
		if p.life > 0 {
			rl.DrawCircle(int32(p.Pos.X*X_SCALE), int32(p.Pos.Y*Y_SCALE), 1.5*2, rl.Yellow)
			//            put_sprite(p.Pos.X, p.Pos.Y, 5)
		}
	}

	// チェインアイテム描画
	for i := range chainitems {
		if chainitems[i].Active {
			put_sprite(chainitems[i].Pos.X, chainitems[i].Pos.Y, 3) // 3番パターンにチェインアイテムの画像を入れる
		}
	}

	for i := range items {
		if !items[i].Active {
			continue
		}
		switch {
		case items[i].types == 1:
			put_sprite(items[i].Pos.X, items[i].Pos.Y, 8)
		case items[i].types == 2:
			put_sprite(items[i].Pos.X, items[i].Pos.Y, 7)
		case items[i].types == 3:
			put_sprite(items[i].Pos.X, items[i].Pos.Y, 9)
		}
	}

	// オプション描画
	for i := 0; i < optionnum; i++ { //range options {
		opt := &options[i]
		//		if opt.Active {
		put_sprite(opt.Pos.X, opt.Pos.Y, 10) // 10 = オプションのパターン番号
		//		}
	}

	// 敵弾
	for _, b := range enemybullets {
		if b.Active {
			put_sprite(b.Pos.X, b.Pos.Y, 0)
		}
	}

	// 敵
	for _, e := range enemies {
		if e.Active {
			//			rl.DrawRectangleV(e.Pos, rl.NewVector2(50, 30), rl.Red)
			//			rl.DrawRectangle(int32(e.Pos.X+10), int32(e.Pos.Y+8), 30, 14, rl.Maroon)
			put_sprite(e.Pos.X, e.Pos.Y, 2)
		}
	}

	// 自機弾
	for _, b := range bullets {
		if b.Active {
			//			rl.DrawRectangleV(b.Pos, rl.NewVector2(22, 6), rl.Yellow)
			//			rl.DrawRectangle(int32(b.Pos.X+18), int32(b.Pos.Y-2), 8, 10, rl.Orange)
			put_sprite(b.Pos.X, b.Pos.Y, 4)
		}
	}

	// シールド描画
	if shield_active {
		put_sprite(player.Pos.X, player.Pos.Y, 6)
	}

	// プレイヤー (三角形っぽく)
	/*	rl.DrawTriangle(
			rl.NewVector2(player.Pos.X+50, player.Pos.Y+15),
			rl.NewVector2(player.Pos.X, player.Pos.Y),
			rl.NewVector2(player.Pos.X, player.Pos.Y+30),
			rl.SkyBlue,
		)
		rl.DrawRectangle(int32(player.Pos.X+10), int32(player.Pos.Y+10), 30, 10, rl.Blue) // 本体*/
	put_sprite(player.Pos.X, player.Pos.Y, 1)

	// UI
	//	rl.DrawText(fmt.Sprintf("SCORE: %d", score), 20, 20, 30, rl.White)
	//	rl.DrawText("WASD / Arrow: Move    Space: Shoot", 20, screenHeight-40, 20, rl.LightGray)

	if score >= high_score {
		put_strings_num(0, 0, "HIGH  ", score, 7)
	} else {
		put_strings_num(0, 0, "SCORE ", score, 7)
	}
	if easy_mode == true {
		put_strings_num(0, 2*FONT_SIZE, "LIVES ", lives, 1)
	}
	put_strings_num(0, 1*FONT_SIZE, "BOMB  ", bomb_stock, 1)

	put_strings_num(16*FONT_SIZE, 0, "COUNT ", int(gameTime), 7)

	if chain_count > 0 {
		put_strings_num(16*FONT_SIZE, 1*FONT_SIZE, "CHAIN ", chain_count, 3)
	}

	if gameOver != 0 {
		//		rl.DrawText("GAME OVER", screenWidth/2-150, screenHeight/2-50, 60, rl.Red)
		//		rl.DrawText("ESC to End", screenWidth/2-80, screenHeight/2+20, 30, rl.White)
		put_strings(11*FONT_SIZE, 12*FONT_SIZE, "GAME OVER")
		put_strings_num(7*FONT_SIZE, 15*FONT_SIZE, "HIGH SCORE ", high_score, 7)

		put_strings(7*FONT_SIZE, 18*FONT_SIZE, "PRESS A TO RESTART")
	}

	//	rl.EndDrawing()
}
