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
	XScale       = 2
	YScale       = 2
	ScreenWidth  = 256 * 2 * XScale
	ScreenHeight = 192 * 2 * YScale
	MaxOptions   = 2
	Count1S      = 60
	MaxStars     = 80
	FontSize     = 16 * XScale
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
	Vx     float32
	Vy     float32
	Active bool
}

type Enemy struct {
	Pos rl.Vector2
	//	speed  float32
	Active        bool
	Types         int
	HP            int
	Count         float32
	Count2        float32
	ShootTimer    float32
	NextShootTime float32
	Speed         float32
}

type Option struct {
	Pos     rl.Vector2
	OffsetY float32
	//	active   bool
}

type Item struct {
	Pos    rl.Vector2
	Timer  float32
	Types  int
	Active bool
}

type ChainItem struct {
	Pos    rl.Vector2
	Timer  float32
	Active bool
}

type Particle struct {
	Pos  rl.Vector2
	Vx   float32
	Vy   float32
	Life float32
}

type Star struct {
	X, Y, Basespeed, Speed, Size float32
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

	score    int
	gameOver int
	easyMode bool

	bombActive bool
	bombStock  int
	bombTimer  float32

	chainTimer float32

	enemySpawnTimer float32

	highScore  int
	lives      int
	chainCount int

	chrTex  rl.Texture2D
	fontTex rl.Texture2D

	laserSound     rl.Sound
	explosionSound rl.Sound
	bgm            rl.Music

	delta         float32
	shootCooldown float32
	gameTime      float32

	shieldActive   bool
	optionCooldown float32

	star []Star
}

func NewGame() *Game {
	game := &Game{
		highScore: 5000,
	}

	//	rand.Seed(time.Now().UnixNano())

	rl.SetConfigFlags(rl.FlagWindowResizable | rl.FlagVsyncHint)

	rl.InitWindow(ScreenWidth, ScreenHeight, "raylib-Go 横スクロールシューティング")
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
			X:     float32(rl.GetRandomValue(0, ScreenWidth/XScale-1)),
			Y:     float32(rl.GetRandomValue(0, ScreenHeight/YScale-1)),
			Speed: 0.5 + float32(rl.GetRandomValue(0, 99)/30), // static_cast<float>(rand() % 100) / 30.0f
		}) //    speed : basespeed   // もし個別速度も欲しい場合
		if game.star[i].Speed > 2 {
			game.star[i].Size = 2
		} else {
			game.star[i].Size = 1
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

	target := rl.LoadRenderTexture(ScreenWidth, ScreenHeight)
	rl.SetTextureFilter(target.Texture, rl.FilterPoint)

	game.Reset()
	game.gameOver = 1

	//	rl.PlayMusicStream(game.bgm)

	for !rl.WindowShouldClose() {
		rl.UpdateMusicStream(game.bgm)

		if rl.IsKeyPressed(rl.KeyF11) {
			rl.ToggleFullscreen()
		}

		scale := float32(min(float32(rl.GetScreenWidth())/ScreenWidth, float32(rl.GetScreenHeight())/ScreenHeight))

		destRec := rl.NewRectangle((float32(rl.GetScreenWidth())-(ScreenWidth*scale))*0.5, (float32(rl.GetScreenHeight())-(ScreenHeight*scale))*0.5, ScreenWidth*scale, ScreenHeight*scale)

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
		Pos:   rl.NewVector2(60, 160), //float32(screenHeight)/2),
		Speed: 4,
	}
	for i := range game.bullets {
		game.bullets[i].Active = false
	}
	for i := range game.enemies {
		game.enemies[i].Active = false
	}

	for i := range game.enemybullets {
		game.enemybullets[i].Active = false
	}

	/*	for i := range game.options {
		options[i].active = false
	}*/
	for i := range game.particles {
		game.particles[i].Life = 0
	}

	for i := range game.items {
		game.items[i].Active = false
	}
	for i := range game.chainitems {
		game.chainitems[i].Active = false
	}

	game.optionnum = 0

	game.shootCooldown = 0
	game.score = 0

	game.enemySpawnTimer = 0.0
	game.gameOver = 0
	game.shieldActive = false

	//	highScore = 5000
	game.lives = 0
	game.gameTime = 0
	game.chainCount = 0

	game.bombTimer = 0
	game.optionCooldown = 10

	if game.easyMode {
		game.lives = 3
	} else {
		game.lives = 1
	}
	game.bombStock = 0
}

func (game *Game) UseBomb() {
	if game.bombStock <= 0 || game.bombActive {
		return
	}

	game.bombStock--
	game.bombActive = true
	//   game. bombTimer = BombDuration

	// 敵と敵弾を全滅
	for i := range game.enemies {
		game.enemies[i].Active = false
	}
	for i := range game.enemybullets {
		game.enemybullets[i].Active = false
	}

	// 大量の破片を発生
	game.CreateParticles(game.player.Pos.X+16, game.player.Pos.Y+16, 45, 1) // 大爆発

	// 画面全体に破片を散らす
	for i := 0; i < 60; i++ {
		rx := float32(rand.Intn(ScreenWidth / XScale))
		ry := float32(rand.Intn(ScreenHeight / YScale))
		game.CreateParticles(rx, ry, 6, 1)
	}

	game.score += 200
	rl.PlaySound(game.explosionSound)
}

func (game *Game) Update() {

	gamepad := int32(0)
	rate := Count1S * game.delta

	if game.gameOver == 1 {
		if !(rl.IsKeyDown(rl.KeySpace) || rl.IsKeyDown(rl.KeyZ) || rl.IsKeyDown(rl.KeyR) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonRightFaceDown)) || rl.IsKeyDown(rl.KeySpace) || rl.IsKeyDown(rl.KeyZ) || rl.IsKeyDown(rl.KeyR) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonRightFaceDown))) {
			game.gameOver = 2
		}
	}
	if game.gameOver == 2 {
		if rl.IsKeyPressed(rl.KeySpace) || rl.IsKeyPressed(rl.KeyZ) || rl.IsKeyPressed(rl.KeyR) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonPressed(gamepad, rl.GamepadButtonRightFaceDown)) {
			game.easyMode = false
			rl.PlayMusicStream(game.bgm)
			game.Reset()
		} else if rl.IsKeyPressed(rl.KeyX) || rl.IsKeyPressed(rl.KeyB) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonPressed(gamepad, rl.GamepadButtonRightFaceRight)) {
			game.easyMode = true
			rl.PlayMusicStream(game.bgm)
			game.Reset()
		}
	}

	// 星移動
	for i := range game.star {
		s := &game.star[i]
		s.X -= s.Speed * rate // これでFPSが60の時に元の速度と同じになる
		if s.X < 0 {
			s.X = ScreenWidth / XScale
		}
	}

	if game.gameOver != 0 {
		return
	}

	game.gameTime += game.delta

	//moveSpeed := 4.0 * rate
	enemySpeed := 4.0 * rate
	//enemySpeed2 := 5.0 * rate

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
		game.player.Pos.Y -= game.player.Speed * rate
	}
	if rl.IsKeyDown(rl.KeyDown) || rl.IsKeyDown(rl.KeyS) || (axisY > 0.2) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonLeftFaceDown)) {
		game.player.Pos.Y += game.player.Speed * rate
	}
	if rl.IsKeyDown(rl.KeyLeft) || rl.IsKeyDown(rl.KeyA) || (axisX < -0.2) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonLeftFaceLeft)) {
		game.player.Pos.X -= game.player.Speed * rate
	}
	if rl.IsKeyDown(rl.KeyRight) || rl.IsKeyDown(rl.KeyD) || (axisX > 0.2) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonLeftFaceRight)) {
		game.player.Pos.X += game.player.Speed * rate
	}

	// 画面端制限
	if game.player.Pos.X < 0 {
		game.player.Pos.X = 0
	}
	if game.player.Pos.X > ScreenWidth/XScale-40 {
		game.player.Pos.X = ScreenWidth/XScale - 40
	}
	if game.player.Pos.Y < 0 {
		game.player.Pos.Y = 0
	}
	if game.player.Pos.Y > ScreenHeight/YScale-32 {
		game.player.Pos.Y = ScreenHeight/YScale - 32
	}

	// オプション更新
	for i := 0; i < game.optionnum; i++ {
		//        game.opt.angle += 0.08f * rate;   // 回転速度
		opt := &game.options[i]
		// 滑らかに追従
		opt.Pos.X += ((game.player.Pos.X + 16) - opt.Pos.X) / 4 * rate
		opt.Pos.Y += ((game.player.Pos.Y + opt.OffsetY) - opt.Pos.Y) / 4 * rate
		//		float t = 1.0f - pow(1.0f - 0.25f, m_deltaTime * COUNT1S)
		//		opt.x = std::lerp(opt.x, playerX + 16, t)
		//		opt.y = std::lerp(opt.y, playerY + opt.offsetY, t)
	}

	// 射撃 (Spaceキー)
	game.shootCooldown += game.delta
	if (rl.IsKeyDown(rl.KeySpace) || rl.IsKeyDown(rl.KeyZ) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonDown(gamepad, rl.GamepadButtonRightFaceDown))) && (game.shootCooldown >= float32(8)/Count1S) {
		/*		bullets = append(bullets, Bullet{
				pos:    rl.NewVector2(player.pos.X+32, player.pos.Y+12),
				speed:  12,
				active: true,
			})*/
		for i := range game.bullets {
			if game.bullets[i].Active == false {
				game.bullets[i] = Bullet{
					Pos:    rl.NewVector2(game.player.Pos.X+32, game.player.Pos.Y+12),
					Speed:  12,
					Active: true,
				}
				break
			}
		}
		for j := 0; j < game.optionnum; j++ {
			for i := range game.bullets {
				if game.bullets[i].Active == false {
					game.bullets[i] = Bullet{
						Pos:    rl.NewVector2(game.options[j].Pos.X+8, game.options[j].Pos.Y+12),
						Speed:  12,
						Active: true,
					}
					break
				}
			}
		}
		game.shootCooldown = 0 //(8 / COUNT1S) // 連射速度
	}

	if (rl.IsKeyPressed(rl.KeyX) || rl.IsKeyPressed(rl.KeyB) || (rl.IsGamepadAvailable(gamepad) && rl.IsGamepadButtonPressed(gamepad, rl.GamepadButtonRightFaceRight))) && game.bombStock > 0 && !game.bombActive {
		game.UseBomb()
	}

	// 自機弾更新
	for i := range game.bullets {
		if game.bullets[i].Active {
			game.bullets[i].Pos.X += game.bullets[i].Speed * rate
			if game.bullets[i].Pos.X > ScreenWidth/XScale {
				game.bullets[i].Active = false
			}
		}
	}

	game.enemySpawnTimer += game.delta
	baseInterval := float32(50.0 - (game.score / 250.0))              // scoreが増えるほど短く
	spawnInterval := max(float32(18.0)/Count1S, baseInterval/Count1S) // フレーム→秒に変換

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
			if game.enemies[i].Active == false {

				randNum := rand.Intn(100)
				var etype int
				if randNum < 60 {
					etype = 0
				} else if randNum < 85 {
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
					Pos:           rl.NewVector2(ScreenWidth/XScale, 32+float32(rand.Intn(ScreenHeight/YScale-32-32-32))),
					Speed:         -4 - float32(rand.Intn(3)),
					Active:        true,
					Types:         etype,
					HP:            ehp,
					Count:         0,
					Count2:        float32(rand.Intn(30*2+ScreenHeight/YScale-40*2) - 30*2),
					ShootTimer:    0,
					NextShootTime: 5.0 / Count1S,
				}
				break
			}
		}

	}

	// 敵更新
	for i := range game.enemies {
		if !game.enemies[i].Active {
			continue
		}
		//			var e Enemy
		e := &game.enemies[i]
		e.Count += rate

		switch {
		case e.Types == 0: // 通常敵
			e.Pos.X -= enemySpeed

		case e.Types == 1: // ヘリザコ - 勢いよく突っ込む
			//				static float dist_x = e.x - player_x
			if e.Count < 24 { // 1段階：超急接近
				e.Pos.X -= 6 * 2 * rate
				e.Pos.Y += ((game.player.Pos.Y + 8 - e.Pos.Y) / 8) / 2 * rate
			} else if e.Count < 49 { // 2段階：短くホバリング
				e.Pos.X -= 0
			} else { // 3段階：右へ全力逃走
				e.Pos.X += 6 * 2 * rate
			}

		case e.Types == 2: // サインカーブ
			e.Pos.X -= enemySpeed
			e.Pos.Y = (e.Count2 + float32(math.Sin(float64(e.Count*0.12)))*55*2)
		}

		// 敵弾発射処理
		e.ShootTimer += game.delta

		difficulty := float32(int(min(1, game.gameTime/180))) // * COUNT1S)))
		enemyBulletSpeed := (4 + difficulty*2)
		shootInterval := ((82 - difficulty*36) - 5) / Count1S

		if e.ShootTimer >= e.NextShootTime {

			dx := game.player.Pos.X - e.Pos.X
			dy := game.player.Pos.Y - e.Pos.Y

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
			bulletSpeed := float32(enemyBulletSpeed)

			dx = (dx * bulletSpeed / dist)
			dy = (dy * bulletSpeed / dist)
			dx = max(-3*2.0, dx)
			dx = min(dx, 4*2.0)
			dy = max(-4*2.0, dy)
			dy = min(dy, 4*2.0)

			for j := range game.enemybullets {
				if game.enemybullets[j].Active == false {

					game.enemybullets[j] = EnemyBullet{
						Pos: rl.NewVector2(e.Pos.X+16,
							e.Pos.Y+16),
						Vx:     dx, // * bulletSpeed - 1.0f*1,   // vx
						Vy:     dy, // * bulletSpeed     // vy
						Active: true,
					}
					break
				}
			}

			// 次回の発射間隔を設定
			e.NextShootTime = float32(shootInterval)

			e.ShootTimer = 0.0
			//				e.count += rate
			//	            e.count++
		}

		//			e.pos.X += e.speed * rate
		if (e.Pos.X < -32) || (e.Pos.X > ScreenWidth/XScale) {
			e.Active = false
		}
	}

	// 自機弾・敵 衝突判定
	for i := range game.bullets {
		if !game.bullets[i].Active {
			continue
		}
		bRect := rl.NewRectangle(game.bullets[i].Pos.X, game.bullets[i].Pos.Y, 16, 8)
		for j := range game.enemies {
			e := &game.enemies[j]
			if !e.Active {
				continue
			}
			eRect := rl.NewRectangle(e.Pos.X, e.Pos.Y, 32, 32)
			if rl.CheckCollisionRecs(bRect, eRect) {
				game.bullets[i].Active = false

				game.CreateParticles(e.Pos.X+16, e.Pos.Y+16, 8, 0) // 通常爆発

				e.HP--
				if e.HP <= 0 {

					// オプションアイテム出現（確率20%くらい）
					//				    if (rand.Intn(100) < 22 && Options.size() < MaxOptions) {
					if game.optionCooldown <= 0 {
						for i := range game.items {
							item := &game.items[i]
							if item.Active {
								continue
							}
							item.Pos.X = e.Pos.X
							item.Pos.Y = e.Pos.Y
							item.Timer = 300 // 約5秒で消える
							item.Types = 1   // 1 = オプションアイテム
							item.Active = true
							game.optionCooldown = 10
							break
						}
					} else {
						game.optionCooldown--
					}

					// シールドアイテム出現（確率12%程度）
					if rand.Intn(100) < 12 && !game.shieldActive {
						for i := range game.items {
							item := &game.items[i]
							if item.Active {
								continue
							}
							item.Pos.X = e.Pos.X
							item.Pos.Y = e.Pos.Y
							item.Timer = 280.0
							item.Types = 2 // 2 = シールド
							item.Active = true
							break
						}
					}

					// ボムアイテム出現
					if rand.Intn(100) < 10 { // 約10%の確率
						for i := range game.items {
							item := &game.items[i]
							if item.Active {
								continue
							}
							item.Pos.X = e.Pos.X
							item.Pos.Y = e.Pos.Y
							item.Timer = 270.0
							item.Types = 3 // 3 = ボム
							item.Active = true
							break
						}
					}
					// === チェインアイテム出現 ===
					if rand.Intn(100) < 40 { // 40%くらいの確率で落とす
						for i := range game.chainitems {
							item := &game.chainitems[i]
							if item.Active {
								continue
							}
							item.Pos.X = e.Pos.X
							item.Pos.Y = e.Pos.Y
							item.Timer = 240.0
							item.Active = true
							break
						}
					}

					e.Active = false
					rl.PlaySound(game.explosionSound)
					game.score += 100
				}
				break
			}
		}
	}

	// 敵弾 vs 自機
	pRect := rl.NewRectangle(game.player.Pos.X, game.player.Pos.Y+6, 32, 32-6*2)
	for i := range game.enemybullets {
		it := &game.enemybullets[i]
		if !it.Active {
			continue
		}
		eRect := rl.NewRectangle(it.Pos.X, it.Pos.Y, 8, 8)

		if rl.CheckCollisionRecs(pRect, eRect) {
			//        if (it.pos.X > player.pos.X && it.pos.X + 8 < player.pos.X + 32 &&
			//            it.pos.Y > player.pos.Y + 6 && it.pos.Y + 8 < player.pos.Y + 6 + 20) {

			if game.shieldActive {
				game.shieldActive = false                                               // シールド消費
				game.CreateParticles(game.player.Pos.X+16, game.player.Pos.Y+16, 18, 1) // 大きな爆発
			} else {
				game.lives--
				if game.lives <= 0 {
					game.gameOver = 1
					rl.StopMusicStream(game.bgm)
				}
			}
			it.Active = false
			break
		}
	}

	// 敵弾移動&画面範囲外判定
	for i := range game.enemybullets {
		it := &game.enemybullets[i]
		if it.Active {
			it.Pos.X += it.Vx * rate
			it.Pos.Y += it.Vy * rate

			if (it.Pos.X < -32) || (it.Pos.X > ScreenWidth/XScale) || (it.Pos.Y < 32) || (it.Pos.Y > ScreenHeight/YScale) {
				it.Active = false
			}
		}
	}

	// プレイヤーと敵の衝突
	pRect = rl.NewRectangle(game.player.Pos.X, game.player.Pos.Y+6, 32, 32-6*2)
	for j := range game.enemies {
		if !game.enemies[j].Active {
			continue
		}
		eRect := rl.NewRectangle(game.enemies[j].Pos.X, game.enemies[j].Pos.Y, 32, 32)
		if rl.CheckCollisionRecs(pRect, eRect) {
			if game.shieldActive {
				game.shieldActive = false                                               // シールド消費
				game.CreateParticles(game.player.Pos.X+16, game.player.Pos.Y+16, 18, 1) // 大きな爆発
			} else {
				game.lives--
				if game.lives <= 0 {
					game.gameOver = 1
					rl.StopMusicStream(game.bgm)
				}
			}
			game.enemies[j].Active = false
		}
		break
	}

	// アイテム更新
	for i := range game.items {
		it := &game.items[i]
		if !it.Active {
			continue
		}
		switch {
		case it.Types == 1:
			it.Pos.X -= 2.0 * rate // 左に流れる

		case it.Types == 2:
			it.Pos.X -= 4.0 * rate // 左に流れる

		case it.Types == 3:
			it.Pos.X -= 4.0 * rate // 左に流れる
		}
		it.Timer -= game.delta

		// 自機との当たり判定
		if math.Abs(float64(it.Pos.X-game.player.Pos.X)) < 44-16 && math.Abs(float64(it.Pos.Y-game.player.Pos.Y)) < 44-16 {

			if it.Types == 1 && game.optionnum < MaxOptions { // オプションアイテム
				var offset float32
				if game.optionnum == 0 {
					offset = 25.0
				} else {
					offset = -25.0
				}
				opt := &game.options[game.optionnum]
				opt.OffsetY = offset * 2
				opt.Pos.X = 0 //playerX + 20
				opt.Pos.Y = 0 //playerY + 16 + offset
				//                opt.angle = 0.0
				game.optionnum++
			} else if it.Types == 2 { // シールド
				game.shieldActive = true
				//                shieldTimer = SheldDuration
			} else if it.Types == 3 { // 3 = ボムアイテム
				game.bombStock = min(3, game.bombStock+1)
			}

			rl.PlaySound(game.laserSound)

			it.Active = false
			continue
		}

		// 画面外 or 時間切れ
		if it.Pos.X < -40 || it.Timer <= 0 {
			it.Active = false
		}
	}

	// パーティクル更新
	for i := range game.particles {
		it := &game.particles[i]
		if it.Life <= 0 {
			continue
		}
		it.Pos.X += it.Vx * rate
		it.Pos.Y += it.Vy * rate
		//        it->vx *= 0.96f      // 少し減速（空気抵抗）
		//        it->vy *= 0.96f

		damping := float32(math.Pow(0.96, float64(rate)))
		it.Vx *= damping
		it.Vy *= damping

		it.Life = it.Life - rate
	}

	// ボム更新
	if game.bombActive {
		game.bombTimer -= game.delta
		if game.bombTimer <= 0.0 {
			game.bombActive = false
		}
	}

	// チェインアイテム更新
	for i := range game.chainitems {
		it := &game.chainitems[i]
		if !it.Active {
			continue
		}
		it.Pos.X -= 4.0 * rate // 左に流れる
		it.Timer -= game.delta

		// 自機取得判定
		if math.Abs(float64(it.Pos.X-game.player.Pos.X)) < 44-16 && math.Abs(float64(it.Pos.Y-game.player.Pos.Y)) < 44-16 {
			game.chainCount++
			game.chainTimer = 240 / Count1S     // チェイン持続時間リセット
			game.score += game.chainCount * 100 // チェイン数に応じたボーナス

			it.Active = false
			rl.PlaySound(game.laserSound)
			continue
		}

		// 時間切れ or 画面外
		if it.Timer <= 0.0 || it.Pos.X < -20 {
			game.chainCount = 0
			it.Active = false
		}
	}

	// チェインタイマー減少
	if game.chainTimer > 0.0 {
		game.chainTimer -= game.delta
		if game.chainTimer <= 0.0 {
			game.chainCount = 0
		}
	}

	if game.gameOver != 0 && game.score > game.highScore {
		game.highScore = game.score
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

func (game *Game) PutStrings(x float32, y float32, text string) {
	len := len(text)
	for i := 0; i < len; i++ {
		if text[i] != ' ' {
			patNo := text[i] - '0'

			rotation := float32(0)

			destRect := rl.NewRectangle(x, y, 16*XScale-1, 16*YScale-1)
			sourceRect := rl.NewRectangle(16.0*float32(patNo%16), 16.0*float32(patNo/16), 16.0, 16.0)
			origin := rl.NewVector2(0, 0)

			rl.DrawTexturePro(game.fontTex, sourceRect, destRect, origin, rotation, rl.White)
		}
		x += FontSize
	}
}

func (game *Game) PutStringsNum(x float32, y float32, str string, num int, digit int) {
	var text string
	len := len(str)
	i := digit
	j := num
	game.PutStrings(x, y, str)

	for i > 0 {
		i--
		text = string(rune(j%10+'0')) + text
		j /= 10
	}
	//    text[digit] = null
	game.PutStrings(x+float32(len*FontSize), y, text)
}

func (game *Game) CreateParticles(x float32, y float32, count int, types int) {
	for i := 0; i < count; i++ {
		for j := range game.particles {
			p := &game.particles[j]
			if p.Life > 0 {
				continue
			}
			p.Pos.X = x
			p.Pos.Y = y
			p.Vx = float32(rand.Intn(100)-50) * 0.12 // -6.0 ~ +6.0
			p.Vy = float32(rand.Intn(100)-50) * 0.12
			p.Life = 20.0 + float32(rand.Intn(25))
			//        p.colorIndex = rand.Intn(5)
			break
		}
	}
}

func (game *Game) PutSprite(x float32, y float32, patNo float32) {
	var rotation float32 = 0.0
	//    var destRect rl.Rectangle

	destRect := rl.NewRectangle(x*XScale, y*YScale, 32*XScale-1, 32*YScale-1)
	sourceRect := rl.NewRectangle(32*patNo, 0, 32, 32)
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
		rl.DrawCircle(int32(game.star[i].X)*XScale, int32(game.star[i].Y)*YScale, 1.5, rl.White)
	}

	// パーティクル描画
	for i := range game.particles {
		p := &game.particles[i]
		if p.Life > 0 {
			rl.DrawCircle(int32(p.Pos.X*XScale), int32(p.Pos.Y*YScale), 1.5*2, rl.Yellow)
			//            game.PutSprite(p.pos.X, p.pos.Y, 5)
		}
	}

	// チェインアイテム描画
	for i := range game.chainitems {
		if game.chainitems[i].Active {
			game.PutSprite(game.chainitems[i].Pos.X, game.chainitems[i].Pos.Y, 3) // 3番パターンにチェインアイテムの画像を入れる
		}
	}

	for i := range game.items {
		if !game.items[i].Active {
			continue
		}
		switch {
		case game.items[i].Types == 1:
			game.PutSprite(game.items[i].Pos.X, game.items[i].Pos.Y, 8)
		case game.items[i].Types == 2:
			game.PutSprite(game.items[i].Pos.X, game.items[i].Pos.Y, 7)
		case game.items[i].Types == 3:
			game.PutSprite(game.items[i].Pos.X, game.items[i].Pos.Y, 9)
		}
	}

	// オプション描画
	for i := 0; i < game.optionnum; i++ { //range game.options {
		opt := &game.options[i]
		//		if opt.active {
		game.PutSprite(opt.Pos.X, opt.Pos.Y, 10) // 10 = オプションのパターン番号
		//		}
	}

	// 敵弾
	for _, b := range game.enemybullets {
		if b.Active {
			game.PutSprite(b.Pos.X, b.Pos.Y, 0)
		}
	}

	// 敵
	for _, e := range game.enemies {
		if e.Active {
			//			rl.DrawRectangleV(e.pos, rl.NewVector2(50, 30), rl.Red)
			//			rl.DrawRectangle(int32(e.pos.X+10), int32(e.pos.Y+8), 30, 14, rl.Maroon)
			game.PutSprite(e.Pos.X, e.Pos.Y, 2)
		}
	}

	// 自機弾
	for _, b := range game.bullets {
		if b.Active {
			//			rl.DrawRectangleV(b.pos, rl.NewVector2(22, 6), rl.Yellow)
			//			rl.DrawRectangle(int32(b.pos.X+18), int32(b.pos.Y-2), 8, 10, rl.Orange)
			game.PutSprite(b.Pos.X, b.Pos.Y, 4)
		}
	}

	// シールド描画
	if game.shieldActive {
		game.PutSprite(game.player.Pos.X, game.player.Pos.Y, 6)
	}

	// プレイヤー (三角形っぽく)
	/*	rl.DrawTriangle(
			rl.NewVector2(player.pos.X+50, player.pos.Y+15),
			rl.NewVector2(player.pos.X, player.pos.Y),
			rl.NewVector2(player.pos.X, player.pos.Y+30),
			rl.SkyBlue,
		)
		rl.DrawRectangle(int32(player.pos.X+10), int32(player.pos.Y+10), 30, 10, rl.Blue) // 本体*/
	game.PutSprite(game.player.Pos.X, game.player.Pos.Y, 1)

	// UI
	//	rl.DrawText(fmt.Sprintf("SCORE: %d", score), 20, 20, 30, rl.White)
	//	rl.DrawText("WASD / Arrow: Move    Space: Shoot", 20, screenHeight-40, 20, rl.LightGray)

	if game.score >= game.highScore {
		game.PutStringsNum(0, 0, "HIGH  ", game.score, 7)
	} else {
		game.PutStringsNum(0, 0, "SCORE ", game.score, 7)
	}
	if game.easyMode == true {
		game.PutStringsNum(0, 2*FontSize, "LIVES ", game.lives, 1)
	}
	game.PutStringsNum(0, 1*FontSize, "BOMB  ", game.bombStock, 1)

	game.PutStringsNum(16*FontSize, 0, "COUNT ", int(game.gameTime), 7)

	if game.chainCount > 0 {
		game.PutStringsNum(16*FontSize, 1*FontSize, "CHAIN ", game.chainCount, 3)
	}

	if game.gameOver != 0 {
		//		rl.DrawText("GAME OVER", screenWidth/2-150, screenHeight/2-50, 60, rl.Red)
		//		rl.DrawText("ESC to End", screenWidth/2-80, screenHeight/2+20, 30, rl.White)
		game.PutStrings(11*FontSize, 12*FontSize, "GAME OVER")
		game.PutStringsNum(7*FontSize, 15*FontSize, "HIGH SCORE ", game.highScore, 7)

		game.PutStrings(7*FontSize, 18*FontSize, "PRESS A TO RESTART")
	}

	//	rl.EndDrawing()
}
