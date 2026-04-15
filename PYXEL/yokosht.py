import pyxel
import math
import random
import json
import os
#import x68k
from binascii import unhexlify

def rgb(r,g,b):
	return (g << 11) + (r << 6) + (b << 1)

class Particle:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.vx = random.uniform(-4, 4)
		self.vy = random.uniform(-4, 4)
		self.life = 35 + random.randint(0, 20)
		self.color = random.choice([8, 9, 10, 14])

	def update(self):
		self.x += self.vx
		self.y += self.vy
#		self.vx *= 1 #0.92
#		self.vy *= 1 #0.92
		self.life -= 1

	def draw(self):
		pyxel.blt(self.x, self.y,  2, 5*16, 0, 16, 16, 0)
#		if self.life > 0:
#			pyxel.pset(int(self.x), int(self.y), self.color)
#			if self.life > 20:
#				pyxel.pset(int(self.x + 1), int(self.y), 7)

class Option:
	def __init__(self, offset_y):
		self.offset_y = offset_y
		self.x = 0
		self.y = 0

	def update(self, player_x, player_y):
		self.x += int(((player_x + 8) - self.x) / 4) #* 0.25
		self.y += int(((player_y + self.offset_y) - self.y) / 4) #* 0.25

	def draw(self):
#		pyxel.tri(self.x-4, self.y, self.x+4, self.y-4, self.x+4, self.y+4, 11)
#		pyxel.tri(self.x-4, self.y, self.x-4, self.y-4, self.x-4, self.y+4, 11)
		pyxel.blt(self.x, self.y,  2, 10*16, 0, 16, 16, 0)

class ChainItem:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.timer = 240

	def update(self):
		self.x -= 2 #1.6
		self.timer -= 1

	def draw(self):
		c = 10 if self.timer % 8 < 4 else 9
#		pyxel.tri(self.x, self.y-7, self.x+7, self.y, self.x, self.y+7, c)
#		pyxel.tri(self.x-7, self.y, self.x, self.y-4, self.x, self.y+4, 7)
		pyxel.blt(self.x, self.y,  2, 3*16, 0, 16, 16, 0)

class OptionItem:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.timer = 300

	def update(self):
		self.x -= 1 #1.4
		self.timer -= 1

	def draw(self):
		c = 12 if self.timer % 10 < 5 else 6
#		pyxel.tri(self.x, self.y-8, self.x+8, self.y, self.x, self.y+8, c)
#		pyxel.tri(self.x-6, self.y, self.x+2, self.y-5, self.x+2, self.y+5, 7)
		pyxel.blt(self.x, self.y,  2, 8*16, 0, 16, 16, 0)

class ShieldItem:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.timer = 280

	def update(self):
		self.x -= 2 #1.5
		self.timer -= 1

	def draw(self):
		c = 7 if self.timer % 6 < 3 else 12
#		pyxel.circ(self.x + 4, self.y + 4, 7, c)
#		pyxel.circb(self.x + 4, self.y + 4, 7, 6)
		pyxel.blt(self.x, self.y,  2, 7*16, 0, 16, 16, 0)

class BombItem:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.timer = 270

	def update(self):
		self.x -= 2 #1.5
		self.timer -= 1

	def draw(self):
		c = 8 if self.timer % 7 < 4 else 9
#		pyxel.circ(self.x + 5, self.y + 5, 6, c)
#		pyxel.pset(self.x + 5, self.y + 2, 7)
#		pyxel.pset(self.x + 5, self.y + 8, 7)
		pyxel.blt(self.x, self.y,  2, 9*16, 0, 16, 16, 0)

class App:
	def __init__(self):
		pyxel.init(256, 192, title="Simple Shmup - Time-based Difficulty", fps=60)
		pyxel.load("yokosht.pyxres")

		# 効果音
#		pyxel.sounds[0].set("c3e3g3", tones="t", volumes="4", effects="f", speed=10)
#		pyxel.sounds[1].set("c2c2c1", tones="p", volumes="6", effects="n", speed=15)
#		pyxel.sounds[2].set("g2e2", tones="s", volumes="5", effects="f", speed=8)
#		pyxel.sounds[3].set("c2a1f1", tones="p", volumes="7", effects="n", speed=20)
#		pyxel.sounds[4].set("c2c2c3c2", tones="t", volumes="7", effects="n", speed=8)

		self.high_score = self.load_high_score()
		self.particles = []
		self.reset()

		self.sin_table = []

		for i in range(0, 256, 1):
			self.sin_table.append(int(math.sin(i * 0.12) * 55))

		pyxel.run(self.update, self.draw)

#		t = 0
#		s = x68k.Sprite()
#		while (t := t + 1) < 20000:
#			k = x68k.iocs(x68k.i.B_SFTSNS)
#			if k & 0x01:
#				break
#			k5 = x68k.iocs(x68k.i.BITSNS, 5)
#			k6 = x68k.iocs(x68k.i.BITSNS, 6)
#			k7 = x68k.iocs(x68k.i.BITSNS, 7)
#			k8 = x68k.iocs(x68k.i.BITSNS, 8)
#			k9 = x68k.iocs(x68k.i.BITSNS, 9)
#			self.update()
#			self.draw()
#			x68k.vsync()

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
		self.player_speed = 2 #.0

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
		self.game_over = False

		self.stars = []
		for _ in range(80):
			self.stars.append([random.randint(0, pyxel.width),
								random.randint(0, pyxel.height),
								random.uniform(0.8, 1.8),
								random.randint(2,16)])

	# 文字列表示
	def put_strings(self, x, y, str):
#		y = 28-y
		chr = str.encode("UTF-8")
		for i in range(len(str)):
			a = chr[i]
			if(a < 0x30):
				a = 0x40
			a = a - 0x30
			pyxel.blt((x + i) * 8, y * 8, 1, (a % 16) * 8, int(a / 16) * 8, 8, 8, 0)

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

	def update(self):
		if self.game_over:
			if pyxel.btnp(pyxel.KEY_R) or pyxel.btnp(pyxel.KEY_Z) or pyxel.btnp(pyxel.KEY_SPACE) or pyxel.btnp(pyxel.GAMEPAD1_BUTTON_A):
				self.reset()
			return

		self.play_time += 1

		# 移動
#	if((k8 & 0x10) || (k7 & 0x10) || !(st & 0x01)) /* 8 */
#		keycode |= KEY_UP1;
#	if((k9 & 0x10) || (k7 & 0x40) || !(st & 0x02)) /* 2 */
#		keycode |= KEY_DOWN1;

#	if(!(st & 0x0c)){ /* RL */
#		keycode |= KEY_START;
#	}else{
#		if((k8 & 0x80) || (k7 & 0x08) || !(st & 0x04)) /* 4 */
#			keycode |= KEY_LEFT1;
#		if((k9 & 0x02) || (k7 & 0x20) || !(st & 0x08)) /* 6 */
#			keycode |= KEY_RIGHT1;
#	}


		if pyxel.btn(pyxel.KEY_LEFT) or pyxel.btn(pyxel.GAMEPAD1_BUTTON_DPAD_LEFT): self.player_x -= self.player_speed

#		k5 = x68k.iocs(x68k.i.BITSNS, 5)
#		k6 = x68k.iocs(x68k.i.BITSNS, 6)
#		k7 = x68k.iocs(x68k.i.BITSNS, 7)
#		k8 = x68k.iocs(x68k.i.BITSNS, 8)
#		k9 = x68k.iocs(x68k.i.BITSNS, 9)

#		if (k8 & 0x10) or (k7 & 0x10): self.player_y -= self.player_speed
#		if (k9 & 0x10) or (k7 & 0x40): self.player_y += self.player_speed
#		if (k8 & 0x80) or (k7 & 0x08): self.player_x -= self.player_speed
#		if (k9 & 0x02) or (k7 & 0x20): self.player_x += self.player_speed

		if pyxel.btn(pyxel.KEY_RIGHT) or pyxel.btn(pyxel.GAMEPAD1_BUTTON_DPAD_RIGHT): self.player_x += self.player_speed
		if pyxel.btn(pyxel.KEY_UP) or pyxel.btn(pyxel.GAMEPAD1_BUTTON_DPAD_UP): self.player_y -= self.player_speed
		if pyxel.btn(pyxel.KEY_DOWN) or pyxel.btn(pyxel.GAMEPAD1_BUTTON_DPAD_DOWN): self.player_y += self.player_speed
		if pyxel.btn(pyxel.KEY_A): self.player_x -= self.player_speed
		if pyxel.btn(pyxel.KEY_D): self.player_x += self.player_speed
		if pyxel.btn(pyxel.KEY_W): self.player_y -= self.player_speed
		if pyxel.btn(pyxel.KEY_S): self.player_y += self.player_speed

		self.player_x = max(0, min(self.player_x, pyxel.width - 20))
		self.player_y = max(0, min(self.player_y, pyxel.height - 16))
#		self.player_x = max(0, min(self.player_x, 256 - 20))
#		self.player_y = max(0, min(self.player_y, 192 - 16))

		# 射撃
		self.shoot_timer += 1
		if (pyxel.btn(pyxel.KEY_SPACE) or pyxel.btn(pyxel.KEY_Z) or pyxel.btn(pyxel.GAMEPAD1_BUTTON_A)) and self.shoot_timer > 8:
#		if ((k5 & 0x04) or (k6 & 0x20)) and self.shoot_timer > 8:
			self.bullets.append([self.player_x + 16, self.player_y + 6])
			for opt in self.options:
				self.bullets.append([opt.x + 4, opt.y + 6])
			self.shoot_timer = 0
#			pyxel.play(0, 0)

		# ボム使用
#		if (k5 & 0x08) and self.bomb_stock > 0:
		if (pyxel.btnp(pyxel.KEY_B) or pyxel.btnp(pyxel.KEY_X) or pyxel.btnp(pyxel.GAMEPAD1_BUTTON_B)) and self.bomb_stock > 0:
			self.use_bomb()

		for b in self.bullets[:]:
			b[0] += 6
			if b[0] > pyxel.width:
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
			base_y = random.randint(30, 192 - 40)
			hp = 1 if enemy_type == 0 else 3
			self.enemies.append([pyxel.width, base_y, enemy_type, 0, base_y, hp, False])
#			self.enemies.append([256, base_y, enemy_type, 0, base_y, hp, False])
			self.enemy_spawn_timer = 0

		# 敵更新
		for e in self.enemies[:]:
			e[3] += 1

			if e[2] == 0:		# 通常敵
				e[0] -= 2 #2.2

			elif e[2] == 1:	  # ヘリザコ - 勢いよく突っ込む
				dist_x = e[0] - self.player_x

				if e[3] < 24:								# 1段階：超急接近
					e[0] -= 6 #5.7
					e[1] += int((self.player_y + 8 - e[1]) / 8) # * 0.13
				elif e[3] < 49:							  # 2段階：短くホバリング
					e[0] -= 0 #0.25
				else:										# 3段階：右へ全力逃走
					e[0] += 6 #5.5

			elif e[2] == 2:	  # サインカーブ
				e[0] -= 2 #1.9
#				e[1] = e[4] + int(math.sin(e[3] * 0.12) * 55)
				if(e[3] < 0):
					e[3] = 0
				e[1] = e[4] + self.sin_table[e[3]]

			# 難易度計算（時間経過）
			difficulty = int(min(1, self.play_time / 10800))
#			enemy_bullet_speed = 2.4 + difficulty * 1.2
			enemy_bullet_speed = 4 + difficulty
			shoot_interval = int(82 - difficulty * 36)

			# 射撃
			if not e[6] and 18 <= e[3] <= 40:
				sx = e[0] + 8
				sy = e[1] + 8
				dx = self.player_x + 8 - sx
				dy = self.player_y + 8 - sy
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
				dx = max(-3 * 1024, dx)
				dx = min(dx, 4 * 1024)
				dy = max(-4 * 1024, dy)
				dy = min(dy, 4 * 1024)
				self.enemy_bullets.append([sx * 1024, sy * 1024, dx, dy])
#				print(dx*5*speed/dist)
#				pyxel.play(2, 2)
				e[6] = True

			elif e[6] and e[3] % shoot_interval == 0 and random.randint(0,100) < 60:
				sx = e[0] + 8
				sy = e[1] + 8
				dx = self.player_x + 8 - sx
				dy = self.player_y + 8 - sy
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
				dx = max(-3 * 1024, dx)
				dx = min(dx, 4 * 1024)
				dy = max(-4 * 1024, dy)
				dy = min(dy * 1024, 4)
				self.enemy_bullets.append([sx * 1024, sy * 1024, dx, dy])
#				pyxel.play(2, 2)

			if e[0] < -40 or e[0] > pyxel.width + 60:
				self.enemies.remove(e)

		# 敵弾更新
		for eb in self.enemy_bullets[:]:
			eb[0] += eb[2]
			eb[1] += eb[3]
			if not (-10 < eb[0] / 1024 < pyxel.width + 10 and -10 < eb[1] / 1024 < pyxel.height + 10):
				self.enemy_bullets.remove(eb)

		# 自機弾 vs 敵
		for b_idx in range(len(self.bullets)-1, -1, -1):
			b = self.bullets[b_idx]
			for e_idx in range(len(self.enemies)-1, -1, -1):
				e = self.enemies[e_idx]
				if (b[0] < e[0] + 16 and b[0] + 8 > e[0] and
					b[1] < e[1] + 16 and b[1] + 4 > e[1]):

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
						pyxel.play(3,16,0,False,True)

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
			item.update()
			if abs(self.player_x + 8 - item.x) < 20 and abs(self.player_y + 8 - item.y) < 20:
				self.chain_count += 1
				self.chain_timer = 240
				self.score += 100 * self.chain_count
#				pyxel.play(1, 1)
				pyxel.play(3,17,0,False,True)
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
			item.update()
			if abs(self.player_x + 8 - item.x) < 22 and abs(self.player_y + 8 - item.y) < 22:
				if len(self.options) < 2:
					offset = 25 if len(self.options) == 0 else -25
					self.options.append(Option(offset))
				self.option_items.remove(item)
#				pyxel.play(1, 1)
				pyxel.play(3,17,0,False,True)
				continue
			if item.x < -20 or item.timer <= 0:
				self.option_items.remove(item)

		for item in self.shield_items[:]:
			item.update()
			if abs(self.player_x + 8 - item.x) < 22 and abs(self.player_y + 8 - item.y) < 22:
				self.shield_active = True
				self.shield_items.remove(item)
#				pyxel.play(1, 1)
				pyxel.play(3,17,0,False,True)
				continue
			if item.x < -20 or item.timer <= 0:
				self.shield_items.remove(item)

		for item in self.bomb_items[:]:
			item.update()
			if abs(self.player_x + 8 - item.x) < 22 and abs(self.player_y + 8 - item.y) < 22:
				self.bomb_stock = min(3, self.bomb_stock + 1)
				self.bomb_items.remove(item)
#				pyxel.play(1, 1)
				pyxel.play(3,17,0,False,True)
				continue
			if item.x < -20 or item.timer <= 0:
				self.bomb_items.remove(item)

		for opt in self.options:
			opt.update(self.player_x, self.player_y)

		# 当たり判定
		ph_x = self.player_x
		ph_y = self.player_y + 3
		ph_w = 16
		ph_h = 10

		for eb_idx in range(len(self.enemy_bullets)-1, -1, -1):
			eb = self.enemy_bullets[eb_idx]
			if (ph_x < eb[0] / 1024 + 4 and ph_x + ph_w > eb[0] / 1024 and
				ph_y < eb[1] / 1024 + 4 and ph_y + ph_h > eb[1] / 1024):
				if self.shield_active:
					self.shield_active = False
					for _ in range(4):
						self.particles.append(Particle(self.player_x + 8, self.player_y + 8))
				else:
					self.game_over = True
#					pyxel.play(3, 3)
				self.enemy_bullets.pop(eb_idx)
				break

		for e_idx in range(len(self.enemies)-1, -1, -1):
			e = self.enemies[e_idx]
			if (ph_x < e[0] + 16 and ph_x + ph_w > e[0] and
				ph_y < e[1] + 16 and ph_y + ph_h > e[1]):
				if self.shield_active:
					self.shield_active = False
					for _ in range(4):
						self.particles.append(Particle(self.player_x + 8, self.player_y + 8))
				else:
					self.game_over = True
#					pyxel.play(3, 3)
				self.enemies.pop(e_idx)
				break

		# パーティクル更新
		for p in self.particles[:]:
			p.update()
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
			self.particles.append(Particle(random.randint(20, 236), random.randint(20, 172)))
		self.enemies.clear()
		self.enemy_bullets.clear()
		self.score += 200

	def draw(self):
		pyxel.cls(1)

		for star in self.stars:
			star[0] -= star[2]
			if star[0] < 0: star[0] = pyxel.width
			pyxel.pset(int(star[0]), int(star[1]), star[3])

#		pyxel.tri(self.player_x + 16, self.player_y + 8,
#				  self.player_x, self.player_y + 4,
#				  self.player_x, self.player_y + 12, 10)
#		s.set(i,spx[i]+16,spy[i]+16,sppat[i],sppri[i], 0)
#		s.set(0,self.player_x+16,self.player_y+16,0,1, 0)
		pyxel.blt(self.player_x, self.player_y, 2, 1*16, 0, 16, 16, 0)

		if self.shield_active:
			alpha = 7 if pyxel.frame_count % 8 < 4 else 12
#			pyxel.circb(self.player_x + 8, self.player_y + 8, 13, alpha)
			pyxel.blt(self.player_x, self.player_y, 2, 6*16, 0, 16, 16, 0)

		i = 1
		for b in self.bullets:
#			s.set(i,b[0], b[1],0,1, 0)
#			i = i + 1
#			pyxel.rect(b[0], b[1], 8, 4, 9)
			pyxel.blt(b[0], b[1],  2, 4*16, 0, 16, 16, 0)

		for e in self.enemies:
			if e[2] == 0: col = 8
			elif e[2] == 1: col = 10 if e[5] == 3 else (9 if e[5] == 2 else 4)
			else: col = 11 if e[5] == 3 else (9 if e[5] == 2 else 4)
#			s.set(i,e[0], e[1],0,1, 0)
#			i = i + 1
#			pyxel.rect(e[0], int(e[1]), 16, 16, col)
#			pyxel.rect(e[0] + 4, int(e[1]) + 4, 8, 8, 7)
			pyxel.blt(e[0], e[1],  2, 2*16, 0, 16, 16, 0)

		for eb in self.enemy_bullets:
			pyxel.blt(eb[0] / 1024, eb[1] / 1024,  2, 0*16, 0, 16, 16, 0)
#			s.set(i,eb[0], eb[1],0,1, 0)
#			i = i + 1
#			pyxel.rect(eb[0] / 1024, eb[1] / 1024, 5, 5, 8)

#		for idx in range(i, 128, 1):
#			s.set(idx, 0, 256, 0, 0 , 0)

		for opt in self.options:
			opt.draw()

		for item in self.chain_items: item.draw()
		for item in self.option_items: item.draw()
		for item in self.shield_items: item.draw()
		for item in self.bomb_items: item.draw()

		for p in self.particles:
			p.draw()

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


#k5 = 0
#k6 = 0
#k7 = 0
#k8 = 0
#k9 = 0

#x68k.crtmod(6)
#s = x68k.Sprite()
#s.init()
#s.clr()
#s.disp()

#s.defcg(0, unhexlify(\
#"0000001111000000" +\
#"0000111111110000" +\
#"0001131111112000" +\
#"0011331111111200" +\
#"0113311111111120" +\
#"0131111111111120" +\
#"1111111111111122" +\
#"1111111111111122" +\
#"1111111111111122" +\
#"1111111111111122" +\
#"0111111111111220" +\
#"0111111111111220" +\
#"0011111111122200" +\
#"0001111112222000" +\
#"0000111122220000" +\
#"0000002222000000"))

#s.palet(14, (rgb(28,0,0),rgb(31,0,0)))
#s.palet(1, (rgb(28,28,28),rgb(15,28,28),rgb(31,31,31)))
#s.palet(1, (rgb(10,28,28),rgb(10,15,28),rgb(31,31,31)),2)

#x68k.curoff()
App()
#x68k.curon()
#x68k.crtmod(16)
