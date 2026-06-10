#import pyxel
from pyray import *

import math
import random
import json
import os
#import x68k
from binascii import unhexlify

X_SCALE = 2
Y_SCALE = 2
screenwidth = 256*2*X_SCALE
screenheight = 212*2*X_SCALE
COUNT1S = 60
delta = 0

def rgb(r,g,b):
	return (g << 11) + (r << 6) + (b << 1)

def put_sprite(x, y, pat_no):
	rotation = 0.0
	destRect = (x * X_SCALE, y * Y_SCALE, 32 * X_SCALE - 1, 32 * Y_SCALE - 1)
	sourceRect = (32.0 * pat_no, 0, 32.0, 32.0)
	origin = ( 0, 0)
	raylib.DrawTexturePro(chrTex, sourceRect, destRect, origin, rotation, raylib.WHITE);
	return

class Particle:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.vx = random.uniform(-4, 4)*2
		self.vy = random.uniform(-4, 4)*2
		self.life = 35 + random.randint(0, 20)
		self.color = random.choice([8, 9, 10, 14])

	def update(self, delta):
		self.x += self.vx * COUNT1S * delta
		self.y += self.vy * COUNT1S * delta
#		self.vx *= 1 #0.92
#		self.vy *= 1 #0.92
		self.life -= 1

	def draw(self):
#		pyxel.blt(self.x, self.y,  2, 5*16, 0, 16, 16, 0)
		raylib.DrawCircle(int(self.x * X_SCALE), int(self.y * Y_SCALE), 1.5 * 2, raylib.YELLOW);

#		if self.life > 0:
#			pyxel.pset(int(self.x), int(self.y), self.color)
#			if self.life > 20:
#				pyxel.pset(int(self.x + 1), int(self.y), 7)

class Option:
	def __init__(self, offset_y):
		self.offset_y = offset_y
		self.x = 0
		self.y = 0

	def update(self, player_x, player_y, delta):
		self.x += int(((player_x + 8*2) - self.x) / 4) * COUNT1S * delta * 1 #* 0.25
		self.y += int(((player_y + self.offset_y) - self.y) / 4) * COUNT1S * delta * 1 #* 0.25

	def draw(self):
#		pyxel.tri(self.x-4, self.y, self.x+4, self.y-4, self.x+4, self.y+4, 11)
#		pyxel.tri(self.x-4, self.y, self.x-4, self.y-4, self.x-4, self.y+4, 11)
#		pyxel.blt(self.x, self.y,  2, 10*16, 0, 16, 16, 0)
		put_sprite(self.x, self.y, 10)

class ChainItem:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.timer = 240

	def update(self, delta):
		self.x -= 2 * COUNT1S * delta * 2 #1.6
		self.timer -= COUNT1S * delta

	def draw(self):
		c = 10 if self.timer % 8 < 4 else 9
#		pyxel.tri(self.x, self.y-7, self.x+7, self.y, self.x, self.y+7, c)
#		pyxel.tri(self.x-7, self.y, self.x, self.y-4, self.x, self.y+4, 7)
#		pyxel.blt(self.x, self.y,  2, 3*16, 0, 16, 16, 0)
		put_sprite(self.x, self.y, 3)

class OptionItem:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.timer = 300

	def update(self, delta):
		self.x -= 1 * COUNT1S * delta * 2 #1.4
		self.timer -= 1  * COUNT1S * delta

	def draw(self):
		c = 12 if self.timer % 10 < 5 else 6
#		pyxel.tri(self.x, self.y-8, self.x+8, self.y, self.x, self.y+8, c)
#		pyxel.tri(self.x-6, self.y, self.x+2, self.y-5, self.x+2, self.y+5, 7)
#		pyxel.blt(self.x, self.y,  2, 8*16, 0, 16, 16, 0)
		put_sprite(self.x, self.y, 8)

class ShieldItem:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.timer = 280

	def update(self, delta):
		self.x -= 2  * COUNT1S * delta * 2 #1.5
		self.timer -= 1 * COUNT1S * delta

	def draw(self):
		c = 7 if self.timer % 6 < 3 else 12
#		pyxel.circ(self.x + 4, self.y + 4, 7, c)
#		pyxel.circb(self.x + 4, self.y + 4, 7, 6)
#		pyxel.blt(self.x, self.y,  2, 7*16, 0, 16, 16, 0)
		put_sprite(self.x, self.y, 7)

class BombItem:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.timer = 270

	def update(self, delta):
		self.x -= 2  * COUNT1S * delta * 2 #1.5
		self.timer -= 1  * COUNT1S * delta

	def draw(self):
		c = 8 if self.timer % 7 < 4 else 9
#		pyxel.circ(self.x + 5, self.y + 5, 6, c)
#		pyxel.pset(self.x + 5, self.y + 2, 7)
#		pyxel.pset(self.x + 5, self.y + 8, 7)
#		pyxel.blt(self.x, self.y,  2, 9*16, 0, 16, 16, 0)
		put_sprite(self.x, self.y, 9)

class App:
	def __init__(self):


#		pyxel.init(256, 192, title="Simple Shmup - Plus", fps=60)
#		pyxel.load("yokosht.pyxres")

		# 効果音
#		pyxel.sounds[0].set("c3e3g3", tones="t", volumes="4", effects="f", speed=10)
#		pyxel.sounds[1].set("c2c2c1", tones="p", volumes="6", effects="n", speed=15)
#		pyxel.sounds[2].set("g2e2", tones="s", volumes="5", effects="f", speed=8)
#		pyxel.sounds[3].set("c2a1f1", tones="p", volumes="7", effects="n", speed=20)
#		pyxel.sounds[4].set("c2c2c3c2", tones="t", volumes="7", effects="n", speed=8)

#		pyxel.sounds[0].mml("t196l4efga8g8a8d2.r2e8f8g8a8g8a8d1r4.>d4c2. <a+4g2 r8e8f8g8a2 r8e8f8g8a8 r8efga8g8a8d2.r2e8f8g8a8g8a8d1r4.>d4c2&c4<a+4g2fed1r2r2 >d2efe2c2<a+2>cdc+8<a8r8>e2r8d2efe2c2<a+2>cdc+8<a8r8>e2r8d2efe2c2<a+2>cdc+8<a8r8>e2r8d2efe2c2g2r8e8f8g8a8r8< l4efga8g8a8d2.r2e8f8g8a8g8a8d1r4.>d4c2. <a+4g2 r8e8f8g8a2 r8e8f8g8a8 r8efga8g8a8d2.r2e8f8g8a8g8a8d1r4.>d4c2&c4<a+4g2fed1&d4")

#		pyxel.sounds[1].mml("t196l8r2. dddddddd dddddddd cccccccc<aaaaaaaa a+a+a+a+a+a+a+a+ gggggggg+ aaaaa r8a+a+ aaaaaaaa> dddddddd dddddddd cccccccc cccccccc <a+a+a+a+a+a+a+a+ aaaaar8>cc dddddddd d2r2 dddddddd cccc<aaaa a+a+a+a+gggg>c+c+c+c+eeee dddd<aaaa> cccc eeee <a+a+a+a+gggg>c+c+c+c+eeee dddddddd cccc<aaaa a+a+a+a+gggg>c+c+c+c+eeee dddd<aaaa> cccc eeee <a+a+a+a+> eegg ar8 l8r4r4r4 dddddddd dddddddd cccccccc<aaaaaaaa a+a+a+a+a+a+a+a+ gggggggg+ aaaaa r8a+a+ aaaaaaaa> dddddddd dddddddd cccccccc cccccccc <a+a+a+a+a+a+a+a+ aaaaar8>cc dddddddd r4")

#		pyxel.sounds[2].mml("t196 l8r2. dfegfed<a> dfegfed<a>  cfegfec<g> cfegfec<g> <a+g>d<g>g<g>e<g> <a+g>d<g>g<g>e<g> ec+g<g>egfe c+<a>e<a>gfec+ dfegfed<a> dfegfed<a>  cfegfec<g> cfegfec<g> <a+g>d<g>g<g>e<g> ec+g<g>egfe dfegfed<a> dfefdfef d<a>d<a>fdfd ece<a> c<g>c<g a+fa+g a+ga+>d c+<a>c+ <g> egfe d<a>d<a>fdfd ece<a> c<g>c<g a+fa+g a+ga+>d c+<a>c+ <g> egfe d<a>d<a>fdfd ece<a> c<g>c<g a+fa+g a+ga+>d c+<a>c+ <g> egfe d<a>d<a>fdfd ece<a> c<g>c<g a+ga+g >d<a+>ef c+r8 l8r2. dfegfed<a> dfegfed<a>  cfegfec<g> cfegfec<g> <a+g>d<g>g<g>e<g> <a+g>d<g>g<g>e<g> ec+g<g>egfe c+<a>e<a>gfec+ dfegfed<a> dfegfed<a> cfegfec<g> cfegfec<g> <a+g>d<g>g<g>e<g> ec+g<g>egfe  dfegfed<a> r4")

#		pyxel.play(0, 0, 0,True)
#		pyxel.play(1, 1, 0,True)
#		pyxel.play(2, 2, 0,True)

#		pyxel.stop()

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

#		pyxel.run(self.update, self.draw)

		target = raylib.LoadRenderTexture(screenwidth, screenheight);

		while not window_should_close():
			raylib.UpdateMusicStream(bgm);
			delta = raylib.GetFrameTime()
			self.delta = delta
			self.update()

			if (raylib.IsKeyPressed(KEY_F11)):
				raylib.ToggleFullscreen()

			# 描画開始
			scale = min(raylib.GetScreenWidth() / screenwidth, raylib.GetScreenHeight() / screenheight)
			destRec = ((raylib.GetScreenWidth() - (screenwidth * scale)) * 0.5, (raylib.GetScreenHeight() - (screenheight * scale)) * 0.5, screenwidth * scale, screenheight * scale)
			raylib.BeginTextureMode(target)
#			begin_drawing()
			clear_background(BLACK)

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

	# 文字列表示
	def put_strings(self, x, y, str):
#		y = 28-y
		chr = str.encode("UTF-8")
		for i in range(len(str)):
			a = chr[i]
			if(a < 0x30):
				a = 0x40
			a = a - 0x30
#			pyxel.blt((x + i) * 8, y * 8, 1, (a % 16) * 8, int(a / 16) * 8, 8, 8, 0)
			rotation = 0.0
			destRect = ((x + i) * 16 * X_SCALE, y * 16 * Y_SCALE, 16 * X_SCALE - 1, 16 * Y_SCALE - 1)
			sourceRect = (16.0 * (a % 16), 16.0 * int(a / 16), 16.0, 16.0)
			origin = (0, 0)
			raylib.DrawTexturePro(fontTex, sourceRect, destRect, origin, rotation, raylib.WHITE);

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
		gamepad = 0
		if self.game_over:
			if raylib.IsKeyPressed(KEY_R) or raylib.IsKeyPressed(KEY_Z) or raylib.IsKeyPressed(KEY_SPACE) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)):
				self.reset()
				self.game_over = False
#				pyxel.play(0, 0, 0,True)
#				pyxel.play(1, 1, 0,True)
#				pyxel.play(2, 2, 0,True)
				raylib.PlayMusicStream(bgm)
			return

		self.play_time += COUNT1S * self.delta

		# 1. ゲームパッドが接続されているかチェック
		axisX = 0
		axisY = 0
		if raylib.IsGamepadAvailable(gamepad):
			# 2. アナログスティック（左スティック）の入力を取得
			# 戻り値は -1.0f から 1.0f の間
			axisX = raylib.GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X);
			axisY = raylib.GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y);

		# 移動
		if raylib.IsKeyDown(KEY_LEFT) or  (axisX < -0.2) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_LEFT)):
			 self.player_x -= self.player_speed

		if raylib.IsKeyDown(KEY_RIGHT) or  (axisX > 0.2) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)):
			self.player_x += self.player_speed * COUNT1S * self.delta
		if raylib.IsKeyDown(KEY_UP) or  (axisY < -0.2) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_UP)):
			self.player_y -= self.player_speed * COUNT1S * self.delta
		if raylib.IsKeyDown(KEY_DOWN) or  (axisY > 0.2) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_DOWN)):
			self.player_y += self.player_speed * COUNT1S * self.delta
		if raylib.IsKeyDown(KEY_A): self.player_x -= self.player_speed * COUNT1S * self.delta
		if raylib.IsKeyDown(KEY_D): self.player_x += self.player_speed * COUNT1S * self.delta
		if raylib.IsKeyDown(KEY_W): self.player_y -= self.player_speed * COUNT1S * self.delta
		if raylib.IsKeyDown(KEY_S): self.player_y += self.player_speed * COUNT1S * self.delta

		self.player_x = max(0, min(self.player_x, screenwidth / X_SCALE - 20*2))
		self.player_y = max(0, min(self.player_y, screenheight / Y_SCALE - 16*2))

		# 自機射撃
		self.shoot_timer += 1
		if (raylib.IsKeyDown(KEY_SPACE) or raylib.IsKeyDown(KEY_Z) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))) and self.shoot_timer > 8:
			self.bullets.append([self.player_x + 16*2, self.player_y + 6*2])
			for opt in self.options:
				self.bullets.append([opt.x + 4*2, opt.y + 6*2])
			self.shoot_timer = 0
#			pyxel.play(0, 0)

		# ボム使用
		if (raylib.IsKeyPressed(KEY_B) or raylib.IsKeyPressed(KEY_X) or (raylib.IsGamepadAvailable(gamepad) and raylib.IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))) and self.bomb_stock > 0:
			# or pyxel.btnp(pyxel.GAMEPAD1_BUTTON_B))
			self.use_bomb()

		for b in self.bullets[:]:
			b[0] += 6 * COUNT1S * self.delta * 2
			if b[0] > screenwidth / X_SCALE:
#			if b[0] > 256:
				self.bullets.remove(b)

		# 敵出現
		self.enemy_spawn_timer += 1
		if self.enemy_spawn_timer > max(18, 50 - (self.score // 250)):
			rand = random.randint(0,100) #random()
			if rand < 60: enemy_type = 0
			elif rand < 85: enemy_type = 1
			else: enemy_type = 2

#			base_y = pyxel.rndi(30, pyxel.height - 40)
			base_y = random.randint(30*2, 192*2 - 40*2)
			hp = 1 if enemy_type == 0 else 3
			self.enemies.append([screenwidth / X_SCALE, base_y, enemy_type, 0, base_y, hp, False])
#			self.enemies.append([256*2, base_y, enemy_type, 0, base_y, hp, False])
			self.enemy_spawn_timer = 0

		# 敵更新
		for e in self.enemies[:]:
			e[3] += 1 * COUNT1S * self.delta

			if e[2] == 0:		# 通常敵
				e[0] -= 2 * COUNT1S * self.delta * 2 #2.2

			elif e[2] == 1:	  # ヘリザコ - 勢いよく突っ込む
				dist_x = e[0] - self.player_x

				if e[3] < 24:								# 1段階：超急接近
					e[0] -= 6 * COUNT1S * self.delta * 2#5.7
					e[1] += int((self.player_y + 8 - e[1]) / 8) * COUNT1S * self.delta * 2# * 0.13
				elif e[3] < 49:							  # 2段階：短くホバリング
					e[0] -= 0 #0.25
				else:										# 3段階：右へ全力逃走
					e[0] += 6 * COUNT1S * self.delta * 2#5.5

			elif e[2] == 2:	  # サインカーブ
				e[0] -= 2 * COUNT1S * self.delta * 2#1.9
#				e[1] = e[4] + int(math.sin(e[3] * 0.12) * 55)
				if(e[3] < 0):
					e[3] = 0
#				e[1] = e[4] + self.sin_table[e[3]]
				e[1] = e[4] + int(math.sin(e[3] * 0.12) * 55)*2

			# 難易度計算（時間経過）
			difficulty = int(min(1, self.play_time / 10800))
#			enemy_bullet_speed = 2.4 + difficulty * 1.2
			enemy_bullet_speed = (2 + difficulty)*2
			shoot_interval = int(82 - difficulty * 36) #/ COUNT1S

			# 敵射撃
			if not e[6] and 18 <= e[3] <= 40:
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
				speed = enemy_bullet_speed
				dx = int(dx * 1024 * speed/dist)
				dy = int(dy * 1024 * speed/dist)
				dx = max(-3*2 * 1024, dx)
				dx = min(dx, 4*2 * 1024)
				dy = max(-4*2 * 1024, dy)
				dy = min(dy, 4*2 * 1024)
				self.enemy_bullets.append([sx * 1024, sy * 1024, dx, dy])
#				print(dx*5*speed/dist)
#				pyxel.play(2, 2)
				e[6] = True

			elif e[6] and int(e[3]) % shoot_interval == 0 and random.randint(0,100) < 60:
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
#				pyxel.play(2, 2)

			if e[0] < -40 or e[0] > screenwidth / X_SCALE + 60:
				self.enemies.remove(e)

		# 敵弾更新
		for eb in self.enemy_bullets[:]:
			eb[0] += eb[2] * COUNT1S * self.delta
			eb[1] += eb[3] * COUNT1S * self.delta
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
						for _ in range(4):
							self.particles.append(Particle(ex, ey))

						del self.enemies[e_idx]

						self.score += 100
						self.kill_count += 1
#						pyxel.play(1, 1)
#						pyxel.play(3,16,0,False,True)
						raylib.PlaySound(explosionSound)

						if random.randint(0,100) < 40:
							self.chain_items.append(ChainItem(ex, ey))

						if self.option_cooldown <= 0:
							self.option_items.append(OptionItem(ex, ey))
							self.option_cooldown = 10
						else:
							self.option_cooldown -= 1

						if random.randint(0, 100) < 12 and not self.shield_active:
							self.shield_items.append(ShieldItem(ex, ey))
						if random.randint(0, 100) < 10 and self.bomb_stock < 3:
							self.bomb_items.append(BombItem(ex, ey))

					else:
						for _ in range(4):
							self.particles.append(Particle(e[0] + 8, e[1] + 8))

					break

		# アイテム処理
		for item in self.chain_items[:]:
			item.update(self.delta)
			if abs(self.player_x + 8*2 - item.x) < 20*2 and abs(self.player_y + 8*2 - item.y) < 20*2:
				self.chain_count += 1
				self.chain_timer = 240
				self.score += 100 * self.chain_count
#				pyxel.play(1, 1)
#				pyxel.play(3,17,0,False,True)
				raylib.PlaySound(laserSound)
				self.chain_items.remove(item)
				continue
			if item.x < -20 or item.timer <= 0:
				self.chain_count = 0
				self.chain_items.remove(item)

		if self.chain_count > 0:
			self.chain_timer -= 1
			if self.chain_timer <= 0:
				self.chain_count = 0

		for item in self.option_items[:]:
			item.update(self.delta)
			if abs(self.player_x + 8*2 - item.x) < 22*2 and abs(self.player_y + 8*2 - item.y) < 22*2:
				if len(self.options) < 2:
					offset = 25*2 if len(self.options) == 0 else -25*2
					self.options.append(Option(offset))
				self.option_items.remove(item)
#				pyxel.play(1, 1)
#				pyxel.play(3,17,0,False,True)
				raylib.PlaySound(laserSound)
				continue
			if item.x < -20 or item.timer <= 0:
				self.option_items.remove(item)

		for item in self.shield_items[:]:
			item.update(self.delta)
			if abs(self.player_x + 8*2 - item.x) < 22*2 and abs(self.player_y + 8*2 - item.y) < 22*2:
				self.shield_active = True
				self.shield_items.remove(item)
#				pyxel.play(1, 1)
#				pyxel.play(3,17,0,False,True)
				raylib.PlaySound(laserSound)
				continue
			if item.x < -20 or item.timer <= 0:
				self.shield_items.remove(item)

		for item in self.bomb_items[:]:
			item.update(self.delta)
			if abs(self.player_x + 8*2 - item.x) < 22*2 and abs(self.player_y + 8*2 - item.y) < 22*2:
				self.bomb_stock = min(3, self.bomb_stock + 1)
				self.bomb_items.remove(item)
#				pyxel.play(1, 1)
#				pyxel.play(3,17,0,False,True)
				raylib.PlaySound(laserSound)
				continue
			if item.x < -20 or item.timer <= 0:
				self.bomb_items.remove(item)

		for opt in self.options:
			opt.update(self.player_x, self.player_y, self.delta)

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
#					pyxel.stop()
					raylib.StopMusicStream(bgm);
#					pyxel.play(3, 3)
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
#					pyxel.stop()
					raylib.StopMusicStream(bgm);
#					pyxel.play(3, 3)
				self.enemies.pop(e_idx)
				break

		# パーティクル更新
		for p in self.particles[:]:
			p.update(self.delta)
			if p.life <= 0:
				self.particles.remove(p)

		if self.game_over and self.score > self.high_score:
			self.high_score = self.score
			self.save_high_score()

	def use_bomb(self):
		if self.bomb_stock <= 0: return
		self.bomb_stock -= 1
#		pyxel.play(3, 4)
		for _ in range(30):
			self.particles.append(Particle(random.randint(20*2, 236*2), random.randint(20*2, 172*2)))
		self.enemies.clear()
		self.enemy_bullets.clear()
		self.score += 200

	def draw(self):
#		pyxel.cls(1)

		for star in self.stars:
			star[0] -= star[2]*2
			if star[0] < 0: star[0] = screenwidth / X_SCALE
#			pyxel.pset(int(star[0]), int(star[1]), star[3])
			raylib.DrawCircle(int(star[0] * X_SCALE), int(star[1] * Y_SCALE), 1.5/2, raylib.WHITE);

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
#			pyxel.blt(eb[0] / 1024, eb[1] / 1024,  2, 0*16, 0, 16, 16, 0)
			put_sprite(eb[0] / 1024, eb[1] / 1024, 0)
#			s.set(i,eb[0], eb[1],0,1, 0)
#			i = i + 1
#			pyxel.rect(eb[0] / 1024, eb[1] / 1024, 5, 5, 8)

		for e in self.enemies:
			if e[2] == 0: col = 8
			elif e[2] == 1: col = 10 if e[5] == 3 else (9 if e[5] == 2 else 4)
			else: col = 11 if e[5] == 3 else (9 if e[5] == 2 else 4)
#			s.set(i,e[0], e[1],0,1, 0)
#			i = i + 1
#			pyxel.rect(e[0], int(e[1]), 16, 16, col)
#			pyxel.rect(e[0] + 4, int(e[1]) + 4, 8, 8, 7)
#			pyxel.blt(e[0], e[1],  2, 2*16, 0, 16, 16, 0)
			put_sprite(e[0], e[1], 2)

		for b in self.bullets:
#			s.set(i,b[0], b[1],0,1, 0)
#			i = i + 1
#			pyxel.rect(b[0], b[1], 8, 4, 9)
#			pyxel.blt(b[0], b[1],  2, 4*16, 0, 16, 16, 0)
			put_sprite(b[0] , b[1], 4)

		if self.shield_active:
#			alpha = 7 if pyxel.frame_count % 8 < 4 else 12
#			pyxel.circb(self.player_x + 8, self.player_y + 8, 13, alpha)
#			pyxel.blt(self.player_x, self.player_y, 2, 6*16, 0, 16, 16, 0)
			put_sprite(self.player_x, self.player_y, 6)

#		pyxel.tri(self.player_x + 16, self.player_y + 8,
#				  self.player_x, self.player_y + 4,
#				  self.player_x, self.player_y + 12, 10)
#		s.set(i,spx[i]+16,spy[i]+16,sppat[i],sppri[i], 0)
#		s.set(0,self.player_x+16,self.player_y+16,0,1, 0)
#		pyxel.blt(self.player_x, self.player_y, 2, 1*16, 0, 16, 16, 0)
		put_sprite(self.player_x, self.player_y, 1)

#		for idx in range(i, 128, 1):
#			s.set(idx, 0, 256, 0, 0 , 0)

		# UI
		self.put_strings(0, 1, "BOMB: ")
		self.put_strings(16, 0, "COUNT: ")

		self.put_numd(7, 0, self.score, 7)
		if self.score >= self.high_score:
			if self.score % 10 == 0:
				self.put_strings(0, 0, "HIGH: ")
		else:
			self.put_strings(0, 0, "SCORE:")

		self.put_numd(7, 1, self.bomb_stock, 1);
		self.put_numd(7+16, 0, self.play_time // 60, 7)

		if self.chain_count > 0:
			self.put_strings(16, 1, "CHAIN")
			self.put_numd(7+16, 1, self.chain_count, 3)

		if self.game_over:
			self.put_strings(11, 12, "GAME OVER")
			self.put_strings(7, 15, "HIGH SCORE: ")
			self.put_numd(7+11, 15, self.high_score, 7)
			self.put_strings(7, 18, "PRESS A TO RESTART")

#		pyxel.text(4, 4, f"SCORE: {self.score}", 7)
#		pyxel.text(4, 14, f"HIGH: {self.high_score}", 7)
#		pyxel.text(180, 4, f"OPTIONS: {len(self.options)}", 7)
#		pyxel.text(200, 14, f"BOMB: {self.bomb_stock}", 8)

		# タイムカウント表示（復活）
#		pyxel.text(4, 34, f"TIME: {self.play_time // 60}s", 7)

#		if self.option_cooldown > 0:
#			pyxel.text(4, 24, f"NEXT OPTION: {self.option_cooldown}", 10)

#		if self.chain_count > 0:
#			 pyxel.text(95, 8, f"CHAIN x{self.chain_count}", 10)

#		if self.game_over:
#			pyxel.text(72, 70, "GAME OVER", 8)
#			pyxel.text(55, 85, f"FINAL SCORE: {self.score}", 7)
#			if self.score == self.high_score and self.score > 0:
#				pyxel.text(65, 100, "NEW HIGH SCORE!", 10)
#			pyxel.text(55, 120, "PRESS R or A TO RESTART", 7)


raylib.SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT )
init_window(screenwidth, screenheight, b"Raylib Python Simple Shmup")
#set_target_fps(60)

target = raylib.LoadRenderTexture(screenwidth*2, screenheight*2)
raylib.SetTextureFilter(target.texture, raylib.ICON_FILTER_POINT)

chrTex = raylib.LoadTexture(b"yokosht.png") #// 画像がなければ後で矩形で代用
fontTex = raylib.LoadTexture(b"FONTYOKO.png")

raylib.InitAudioDevice()
laserSound = raylib.LoadSound(b"laser.wav")
explosionSound = raylib.LoadSound(b"explosion.wav")
bgm = raylib.LoadMusicStream(b"bgm.mp3")
#delta = 0

App()

# ウィンドウを閉じる
close_window()


#yokoshtr.py By maZone(m@3) with Grok 2026.
