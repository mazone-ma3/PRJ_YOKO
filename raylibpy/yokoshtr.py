#from pyray import *
import pyray
import raylib

import math
import random
import json
import os
from binascii import unhexlify

X_SCALE = 2
Y_SCALE = 2
screenwidth = 256*2*X_SCALE
screenheight = 212*2*X_SCALE
COUNT1S = 60

class Particle:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.vx = random.uniform(-4, 4)*2
		self.vy = random.uniform(-4, 4)*2
		self.life = 35 + random.randint(0, 20)
		self.color = random.choice([8, 9, 10, 14])

	def update(self, rate):
		self.x += self.vx * rate
		self.y += self.vy * rate
		damping = math.pow(0.96, rate);
		self.vx *= damping
		self.vy *= damping
		self.life -= rate

	def draw(self):
		raylib.DrawCircle(int(self.x * X_SCALE), int(self.y * Y_SCALE), 1.5 * 2, raylib.YELLOW)

#		if self.life > 0:
#			pyxel.pset(int(self.x), int(self.y), self.color)
#			if self.life > 20:
#				pyxel.pset(int(self.x + 1), int(self.y), 7)

class Option:
	def __init__(self, offset_y, app):
		self.offset_y = offset_y
		self.x = 0
		self.y = 0
		self.app = app

	def update(self, player_x, player_y, rate):
		self.x += int(((player_x + 8*2) - self.x) / 4) * rate * 1 #* 0.25
		self.y += int(((player_y + self.offset_y) - self.y) / 4) * rate * 1 #* 0.25

	def draw(self):
		self.app.put_sprite(self.x, self.y, 10)

class ChainItem:
	def __init__(self, x, y, app):
		self.x = x
		self.y = y
		self.timer = 240
		self.app = app

	def update(self, rate):
		self.x -= 2 * rate * 2 #1.6
		self.timer -= rate

	def draw(self):
		c = 10 if self.timer % 8 < 4 else 9
		self.app.put_sprite(self.x, self.y, 3)

class OptionItem:
	def __init__(self, x, y, app):
		self.x = x
		self.y = y
		self.timer = 300
		self.app = app

	def update(self, rate):
		self.x -= 1 * rate * 2 #1.4
		self.timer -= 1  * rate

	def draw(self):
		c = 12 if self.timer % 10 < 5 else 6
		self.app.put_sprite(self.x, self.y, 8)

class ShieldItem:
	def __init__(self, x, y, app):
		self.x = x
		self.y = y
		self.timer = 280
		self.app = app

	def update(self, rate):
		self.x -= 2  * rate * 2 #1.5
		self.timer -= 1 * rate

	def draw(self):
		c = 7 if self.timer % 6 < 3 else 12
		self.app.put_sprite(self.x, self.y, 7)

class BombItem:
	def __init__(self, x, y, app):
		self.x = x
		self.y = y
		self.timer = 270
		self.app = app

	def update(self, rate):
		self.x -= 2  * rate * 2 #1.5
		self.timer -= 1  * rate

	def draw(self):
		c = 8 if self.timer % 7 < 4 else 9
		self.app.put_sprite(self.x, self.y, 9)

class App:
	def __init__(self):

		raylib.SetConfigFlags(raylib.FLAG_WINDOW_RESIZABLE | raylib.FLAG_VSYNC_HINT )
		pyray.init_window(screenwidth, screenheight, "Raylib Python 横スクロールシューティング")
#set_target_fps(60)

		self.target = raylib.LoadRenderTexture(screenwidth*2, screenheight*2)
		raylib.SetTextureFilter(self.target.texture, raylib.ICON_FILTER_POINT)

		self.chrTex = raylib.LoadTexture(b"yokosht.png") #// 画像がなければ後で矩形で代用
		self.fontTex = raylib.LoadTexture(b"FONTYOKO.png")

		raylib.InitAudioDevice()
		self.laserSound = raylib.LoadSound(b"laser.wav")
		self.explosionSound = raylib.LoadSound(b"explosion.wav")
		self.bgm = raylib.LoadMusicStream(b"bgm.mp3")

		# 効果音

		self.high_score = self.load_high_score()
		self.particles = []
		self.reset()
		self.game_over = True

		self.sin_table = []

		for i in range(0, 256, 1):
			self.sin_table.append(int(math.sin(i * 0.12) * 55))

		self.stars = []
		for _ in range(80):
			self.stars.append([random.randint(0, screenwidth // X_SCALE),
								random.randint(0, screenheight // Y_SCALE),
								random.uniform(0.8, 1.8),
								random.randint(2,15)])

		target = raylib.LoadRenderTexture(screenwidth, screenheight)

		while not pyray.window_should_close():
			raylib.UpdateMusicStream(self.bgm)
			self.delta = raylib.GetFrameTime()
			self.update()

			if (raylib.IsKeyPressed(raylib.KEY_F11)):
				raylib.ToggleFullscreen()

			# 描画開始
			scale = min(raylib.GetScreenWidth() / screenwidth, raylib.GetScreenHeight() / screenheight)
			destRec = ((raylib.GetScreenWidth() - (screenwidth * scale)) * 0.5, (raylib.GetScreenHeight() - (screenheight * scale)) * 0.5, screenwidth * scale, screenheight * scale)
			raylib.BeginTextureMode(target)
#			begin_drawing()
			pyray.clear_background(raylib.BLACK)

			self.draw()

			# 画面にテキストを表示 (文字列の前に b をつけてバイト列にするのがコツです)
#			draw_text(b"Python is running Raylib!", 190, 200, 40, LIGHTGRAY)

			# 描画終了
#			end_drawing()
			raylib.EndTextureMode()

			raylib.BeginDrawing()
			raylib.ClearBackground(raylib.BLACK) #// フルスクリーン時の「黒帯」になる部分の色

			#// レンダーテクスチャは上下の座標が反転しているため、sourceのheightをマイナスにする必要があります
			sourceRec = ( 0.0, 0.0, target.texture.width, - target.texture.height )
			origin = ( 0, 0)

			#// 計算した位置・サイズ（destRec）で綺麗に拡大描画
			raylib.DrawTexturePro(target.texture, sourceRec, destRec, origin, 0.0, raylib.WHITE)
			raylib.EndDrawing()

		# ウィンドウを閉じる
		pyray.close_window()


	def load_high_score(self):
		if os.path.exists("highscore.json"):
			try:
				with open("highscore.json", "r") as f:
					return json.load(f).get("high_score", 0)
			except:
				return 5000
		return 5000

	def save_high_score(self):
		try:
			with open("highscore.json", "w") as f:
				json.dump({"high_score": self.high_score}, f)
		except:
			pass

	def reset(self):
		self.player_x = 30
		self.player_y = 80
		self.player_speed = 2*2 #.0

		self.bullets = []
		self.enemy_bullets = []
		self.particles = []
		self.options = []
		self.chain_items = []
		self.option_items = []
		self.shield_items = []
		self.bomb_items = []
		self.bomb_stock = 0
		self.shield_active = False

		self.chain_count = 0
		self.chain_timer = 0
		self.option_cooldown = 10

		self.enemies = []
		self.enemy_spawn_timer = 0
		self.kill_count = 0

		self.shoot_timer = 0
		self.score = 0
		self.play_time = 0		  # 経過時間（フレーム）
#		self.game_over = False

	def put_sprite(self, x, y, pat_no):
		rotation = 0.0
		destRect = (x * X_SCALE, y * Y_SCALE, 32 * X_SCALE - 1, 32 * Y_SCALE - 1)
		sourceRect = (32.0 * pat_no, 0, 32.0, 32.0)
		origin = ( 0, 0)
		raylib.DrawTexturePro(self.chrTex, sourceRect, destRect, origin, rotation, raylib.WHITE)
		return


	# 文字列表示
	def put_strings(self, x, y, str):
#		y = 28-y
		chr = str.encode("UTF-8")
		for i in range(len(str)):
			a = chr[i]
			if(a < 0x30):
				a = 0x40
			a = a - 0x30
			rotation = 0.0
			destRect = ((x + i) * 16 * X_SCALE, y * 16 * Y_SCALE, 16 * X_SCALE - 1, 16 * Y_SCALE - 1)
			sourceRect = (16.0 * (a % 16), 16.0 * int(a / 16), 16.0, 16.0)
			origin = (0, 0)
			raylib.DrawTexturePro(self.fontTex, sourceRect, destRect, origin, rotation, raylib.WHITE)

	# 数字表示
	def put_numd(self, x, y, j, digit):
		i = 0 #digit
		self.str_temp = ""
		k = 0
		l = b""
		while (i < digit):
			i = i + 1
			k = int(j / (10**(digit-i))) % 10
			l += str(k).encode('UTF-8')
#			self.str_temp[i] = k.decode(UTF-8)
#			j /= 10
#		self.str_temp[digit] = '\0'
#		k = str(j).encode('UTF-8')
		self.str_temp = l.decode("UTF-8")
		self.put_strings(x, y, self.str_temp)


	# メイン処理
	def update(self):
		rate = self.delta * COUNT1S
		gamepad = 0
		if self.game_over:
			if raylib.IsKeyPressed(raylib.KEY_R) or raylib.IsKeyPressed(raylib.KEY_Z) or raylib.IsKeyPressed(raylib.KEY_SPACE) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonPressed(gamepad, raylib.GAMEPAD_BUTTON_RIGHT_FACE_DOWN)):
				self.reset()
				self.game_over = False
				raylib.PlayMusicStream(self.bgm)
			return

		self.play_time += self.delta #rate

		# 1. ゲームパッドが接続されているかチェック
		axisX = 0
		axisY = 0
		if raylib.IsGamepadAvailable(gamepad):
			# 2. アナログスティック（左スティック）の入力を取得
			# 戻り値は -1.0f から 1.0f の間
			axisX = raylib.GetGamepadAxisMovement(gamepad, raylib.GAMEPAD_AXIS_LEFT_X)
			axisY = raylib.GetGamepadAxisMovement(gamepad, raylib.GAMEPAD_AXIS_LEFT_Y)

		# 移動
		movespeed = self.player_speed * rate
		if raylib.IsKeyDown(raylib.KEY_A) or raylib.IsKeyDown(raylib.KEY_LEFT) or  (axisX < -0.2) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonDown(gamepad, raylib.GAMEPAD_BUTTON_LEFT_FACE_LEFT)):
			self.player_x -= movespeed

		if raylib.IsKeyDown(raylib.KEY_D) or raylib.IsKeyDown(raylib.KEY_RIGHT) or  (axisX > 0.2) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonDown(gamepad, raylib.GAMEPAD_BUTTON_LEFT_FACE_RIGHT)):
			self.player_x += movespeed
		if raylib.IsKeyDown(raylib.KEY_W) or raylib.IsKeyDown(raylib.KEY_UP) or  (axisY < -0.2) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonDown(gamepad, raylib.GAMEPAD_BUTTON_LEFT_FACE_UP)):
			self.player_y -= movespeed
		if raylib.IsKeyDown(raylib.KEY_S) or raylib.IsKeyDown(raylib.KEY_DOWN) or  (axisY > 0.2) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonDown(gamepad, raylib.GAMEPAD_BUTTON_LEFT_FACE_DOWN)):
			self.player_y += movespeed

		self.player_x = max(0, min(self.player_x, screenwidth / X_SCALE - 20*2))
		self.player_y = max(0, min(self.player_y, screenheight / Y_SCALE - 16*2))

		# 自機射撃
		self.shoot_timer += self.delta
		if (raylib.IsKeyDown(raylib.KEY_SPACE) or raylib.IsKeyDown(raylib.KEY_Z) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonDown(gamepad, raylib.GAMEPAD_BUTTON_RIGHT_FACE_DOWN))) and self.shoot_timer >= (8 / COUNT1S):
			self.bullets.append([self.player_x + 16*2, self.player_y + 6*2])
			for opt in self.options:
				self.bullets.append([opt.x + 4*2, opt.y + 6*2])
			self.shoot_timer = 0

		# ボム使用
		if (raylib.IsKeyPressed(raylib.KEY_B) or raylib.IsKeyPressed(raylib.KEY_X) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonPressed(gamepad, raylib.GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))) and self.bomb_stock > 0:
			self.use_bomb()

		for b in self.bullets[:]:
			b[0] += 6 * rate * 2
			if b[0] > screenwidth / X_SCALE:
#			if b[0] > 256:
				self.bullets.remove(b)

		# 敵出現
		self.enemy_spawn_timer += self.delta
		baseinterval = (50 - (self.score / 250))
		spawninterval = max(18 / COUNT1S, baseinterval / COUNT1S)

		if self.enemy_spawn_timer >= spawninterval:
			rand = random.randint(0,100) #random()
			if rand < 60: enemy_type = 0
			elif rand < 85: enemy_type = 1
			else: enemy_type = 2

			hp = 1 if enemy_type == 0 else 3
			self.enemies.append([screenwidth / X_SCALE, random.randint(32, screenheight // Y_SCALE - 64), enemy_type, 0, random.randint(0, 30*2 + screenheight // Y_SCALE - 40 * 2) - 30 * 2, hp, 0, 5 / COUNT1S])

			self.enemy_spawn_timer = 0

		# 敵更新
		for e in self.enemies[:]:
			e[3] += 1 * rate

			if e[2] == 0:		# 通常敵
				e[0] -= 2 * rate * 2 #2.2

			elif e[2] == 1:	  # ヘリザコ - 勢いよく突っ込む
				dist_x = e[0] - self.player_x

				if e[3] < 24:								# 1段階：超急接近
					e[0] -= 6 * rate * 2#5.7
					e[1] += int((self.player_y + 8 - e[1]) / 8) * rate * 2# * 0.13
				elif e[3] < 49:							  # 2段階：短くホバリング
					e[0] -= 0 #0.25
				else:										# 3段階：右へ全力逃走
					e[0] += 6 * rate * 2#5.5

			elif e[2] == 2:	  # サインカーブ
				e[0] -= 2 * rate * 2#1.9
#				e[1] = e[4] + int(math.sin(e[3] * 0.12) * 55)
				if(e[3] < 0):
					e[3] = 0
#				e[1] = e[4] + self.sin_table[e[3]]
				e[1] = e[4] + int(math.sin(e[3] * 0.12) * 55)*2

			# 難易度計算（時間経過）
			e[6] += self.delta

			difficulty = int(min(1, self.play_time / 180)) # * COUNT1S))) # 10800))
#			enemy_bullet_speed = 2.4 + difficulty * 1.2
			enemy_bullet_speed = (4 + difficulty * 2)
			shoot_interval = ((82 - difficulty * 36) - 5) / COUNT1S

			# 敵射撃
			if e[6] >= e[7]: # and random.randint(0,100) < 60:
				sx = e[0] + 8*2
				sy = e[1] + 8*2
				dx = self.player_x + 8*2 - sx
				dy = self.player_y + 8*2 - sy
#				dist = math.hypot(dx, dy) or 1
#				dist = int(abs(dx + dy))

				if abs(dx) > abs(dy):
					dist = abs(dx)
				else:
					dist = abs(dy)

				if dist == 0 : dist = 1
				base_speed = enemy_bullet_speed
				direction_factor = dx / dist
				speed = base_speed #* (1.0 - 0.22 * direction_factor)
				dx = int(1024 * dx*speed/dist)
				dy = int(1024 * dy*speed/dist)
				dx = max(-3*2 * 1024, dx)
				dx = min(dx, 4*2 * 1024)
				dy = max(-4*2 * 1024, dy)
				dy = min(dy, 1024 * 4*2)
				self.enemy_bullets.append([sx * 1024, sy * 1024, dx, dy])
				e[6] = 0
				e[7] = shoot_interval

			if e[0] < -40 or e[0] > screenwidth / X_SCALE + 60:
				self.enemies.remove(e)

		# 敵弾更新
		for eb in self.enemy_bullets[:]:
			eb[0] += eb[2] * rate
			eb[1] += eb[3] * rate
			if not (-10 < eb[0] / 1024 < screenwidth / X_SCALE + 10 and -10 < eb[1] / 1024 < screenheight / Y_SCALE + 10):
				self.enemy_bullets.remove(eb)

		# 自機弾 vs 敵
		for b_idx in range(len(self.bullets)-1, -1, -1):
			b = self.bullets[b_idx]
			for e_idx in range(len(self.enemies)-1, -1, -1):
				e = self.enemies[e_idx]
				if (b[0] < e[0] + 16*2 and b[0] + 8*2 > e[0] and
					b[1] < e[1] + 16*2 and b[1] + 4*2 > e[1]):

					e[5] -= 1
					del self.bullets[b_idx]

					if e[5] <= 0:
						ex = e[0] # + 8
						ey = e[1] # + 8
						for _ in range(8):
							self.particles.append(Particle(ex, ey))

						del self.enemies[e_idx]

						self.score += 100
						self.kill_count += 1
						raylib.PlaySound(self.explosionSound)

						if random.randint(0,100) < 40:
							self.chain_items.append(ChainItem(ex, ey, self))

						if self.option_cooldown <= 0:
							self.option_items.append(OptionItem(ex, ey, self))
							self.option_cooldown = 10
						else:
							self.option_cooldown -= 1

						if random.randint(0, 100) < 12 and not self.shield_active:
							self.shield_items.append(ShieldItem(ex, ey, self))
						if random.randint(0, 100) < 10 and self.bomb_stock < 3:
							self.bomb_items.append(BombItem(ex, ey, self))

					else:
						for _ in range(8):
							self.particles.append(Particle(e[0] + 8, e[1] + 8))

					break

		# アイテム処理
		for item in self.chain_items[:]:
			item.update(rate)
			if abs(self.player_x + 8*2 - item.x) < 20*2 and abs(self.player_y + 8*2 - item.y) < 20*2:
				self.chain_count += 1
				self.chain_timer = 240
				self.score += 100 * self.chain_count
				raylib.PlaySound(self.laserSound)
				self.chain_items.remove(item)
				continue
			if item.x < -20 or item.timer <= 0:
				self.chain_count = 0
				self.chain_items.remove(item)

		if self.chain_count > 0:
			self.chain_timer -= rate
			if self.chain_timer <= 0:
				self.chain_count = 0

		for item in self.option_items[:]:
			item.update(rate)
			if abs(self.player_x + 8*2 - item.x) < 22*2 and abs(self.player_y + 8*2 - item.y) < 22*2:
				if len(self.options) < 2:
					offset = 25*2 if len(self.options) == 0 else -25*2
					self.options.append(Option(offset, self))
				self.option_items.remove(item)
				raylib.PlaySound(self.laserSound)
				continue
			if item.x < -20 or item.timer <= 0:
				self.option_items.remove(item)

		for item in self.shield_items[:]:
			item.update(rate)
			if abs(self.player_x + 8*2 - item.x) < 22*2 and abs(self.player_y + 8*2 - item.y) < 22*2:
				self.shield_active = True
				self.shield_items.remove(item)
				raylib.PlaySound(self.laserSound)
				continue
			if item.x < -20 or item.timer <= 0:
				self.shield_items.remove(item)

		for item in self.bomb_items[:]:
			item.update(rate)
			if abs(self.player_x + 8*2 - item.x) < 22*2 and abs(self.player_y + 8*2 - item.y) < 22*2:
				self.bomb_stock = min(3, self.bomb_stock + 1)
				self.bomb_items.remove(item)
				raylib.PlaySound(self.laserSound)
				continue
			if item.x < -20 or item.timer <= 0:
				self.bomb_items.remove(item)

		for opt in self.options:
			opt.update(self.player_x, self.player_y, rate)

		# 当たり判定
		ph_x = self.player_x
		ph_y = self.player_y + 3*2
		ph_w = 16*2
		ph_h = 10*2

		for eb_idx in range(len(self.enemy_bullets)-1, -1, -1):
			eb = self.enemy_bullets[eb_idx]
			if (ph_x < eb[0] / 1024 + 4*2 and ph_x + ph_w > eb[0] / 1024 and
				ph_y < eb[1] / 1024 + 4*2 and ph_y + ph_h > eb[1] / 1024):
				if self.shield_active:
					self.shield_active = False
					for _ in range(4):
						self.particles.append(Particle(self.player_x + 8, self.player_y + 8))
				else:
					self.game_over = True
					raylib.StopMusicStream(self.bgm)
				self.enemy_bullets.pop(eb_idx)
				break

		for e_idx in range(len(self.enemies)-1, -1, -1):
			e = self.enemies[e_idx]
			if (ph_x < e[0] + 16*2 and ph_x + ph_w > e[0] and
				ph_y < e[1] + 16*2 and ph_y + ph_h > e[1]):
				if self.shield_active:
					self.shield_active = False
					for _ in range(4):
						self.particles.append(Particle(self.player_x + 8, self.player_y + 8))
				else:
					self.game_over = True
					raylib.StopMusicStream(self.bgm)
				self.enemies.pop(e_idx)
				break

		# パーティクル更新
		for p in self.particles[:]:
			p.update(rate)
			if p.life <= 0:
				self.particles.remove(p)

		if self.game_over and self.score > self.high_score:
			self.high_score = self.score
			self.save_high_score()

	def use_bomb(self):
		if self.bomb_stock <= 0: return
		self.bomb_stock -= 1
		for _ in range(45):
			self.particles.append(Particle(self.player_x + 16, self.player_y + 16))
		for _ in range(360):
			self.particles.append(Particle(random.randint(20*2, 236*2), random.randint(20*2, 172*2)))
		self.enemies.clear()
		self.enemy_bullets.clear()
		self.score += 200

	def draw(self):
		for star in self.stars:
			star[0] -= star[2]*2
			if star[0] < 0: star[0] = screenwidth / X_SCALE
			raylib.DrawCircle(int(star[0] * X_SCALE), int(star[1] * Y_SCALE), 1.5/2, raylib.WHITE)

#		i = 1

		for p in self.particles:
			p.draw()

		for item in self.chain_items: item.draw()

		for item in self.bomb_items: item.draw()
		for item in self.shield_items: item.draw()
		for item in self.option_items: item.draw()

		for opt in self.options:
			opt.draw()

		for eb in self.enemy_bullets:
			self.put_sprite(eb[0] / 1024, eb[1] / 1024, 0)

		for e in self.enemies:
			if e[2] == 0: col = 8
			elif e[2] == 1: col = 10 if e[5] == 3 else (9 if e[5] == 2 else 4)
			else: col = 11 if e[5] == 3 else (9 if e[5] == 2 else 4)
			self.put_sprite(e[0], e[1], 2)

		for b in self.bullets:
			self.put_sprite(b[0] , b[1], 4)

		if self.shield_active:
			self.put_sprite(self.player_x, self.player_y, 6)

		self.put_sprite(self.player_x, self.player_y, 1)


		# UI
		self.put_strings(0, 1, "BOMB: ")
		self.put_strings(16, 0, "COUNT: ")

		self.put_numd(7, 0, self.score, 7)
		if self.score >= self.high_score:
			if self.score % 10 == 0:
				self.put_strings(0, 0, "HIGH: ")
		else:
			self.put_strings(0, 0, "SCORE:")

		self.put_numd(7, 1, self.bomb_stock, 1)
		self.put_numd(7+16, 0, self.play_time // 1, 7)

		if self.chain_count > 0:
			self.put_strings(16, 1, "CHAIN")
			self.put_numd(7+16, 1, self.chain_count, 3)

		if self.game_over:
			self.put_strings(11, 12, "GAME OVER")
			self.put_strings(7, 15, "HIGH SCORE: ")
			self.put_numd(7+11, 15, self.high_score, 7)
			self.put_strings(7, 18, "PRESS A TO RESTART")


App()

#yokoshtr.py By maZone(m@3) with Grok 2026.
