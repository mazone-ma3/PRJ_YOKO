function setfullscreen() {

	// Chrome & Firefox v64以降
	if (canvas.requestFullscreen) {
		canvas.requestFullscreen();

	// Firefox v63以前
	} else if (canvas.mozRequestFullScreen) {
		canvas.mozRequestFullScreen();

	// Safari & Edge & Chrome v68以前
	} else if (canvas.webkitRequestFullscreen) {
		canvas.webkitRequestFullscreen();

	// IE11
	} else if (canvas.msRequestFullscreen) {
		canvas.msRequestFullscreen();
	}
}

function resetfullscreen() {
	// Chrome & Firefox v64以降
	if (document.exitFullscreen) {
		document.exitFullscreen();

	// Firefox v63以前
	} else if (document.mozCancelFullScreen) {
		document.mozCancelFullScreen();

	// Safari & Edge & Chrome v44以前
	} else if (document.webkitCancelFullScreen) {
		document.webkitCancelFullScreen();

	// IE11
	} else if (document.msExitFullscreen) {
		document.msExitFullscreen();
	}
}

window.addEventListener('load', function () {

	// フルスクリーン表示
	document.getElementById('button1').addEventListener('click', function () {

		setfullscreen();

	});

	// フルスクリーン解除
	document.getElementById('button2').addEventListener('click', function () {
		//		audio.mute = false;
		resetfullscreen();

	});
});

const fontSize = 16;
const count1S = 60;
const maxOptions = 2;

const canvas = document.getElementById('game');
const ctx = canvas.getContext('2d');
const scoreEl = document.getElementById('score');
const lifeEl = document.getElementById('life');

const audio = new Audio('bgm.mp3');
audio.loop = true;
//	audio.mute = false;
const wav = new Audio('explosion.wav');
const laser = new Audio('laser.wav');

sourceImage = document.getElementById("yokosht");
fontImage = document.getElementById("fontyoko");

let gameRunning = 1;
let score = 0;
let highScore = 5000;
let life = 3;

let easyMode = false;
let bombStock = 0;
let bombTimer = 0;
let bombActive = true;
let gameTime = 0;
let chainCount = 0;
let chainTimer = 0;

let shieldActive = false;

let lastTime = Date.now();
let rate = 0;

let spawnTimer = 0;
let etype = 0;
let ehp = 1;
let randNum = 0;
let optionCooldown = 10;
let optionNum = 0;

// プレイヤー
const player = {
	x: 60,
	y: 160,
	width: 32,
	height: 20,
	left: 0,
	top: 6,
	speed: 4
};

let bullets = [];
let enemies = [];
let enemyBullets = [];
let chainItems = [];
let items = [];
let options = [];
let particles = []; // 爆発エフェクト

let keys = {};
let lastShot = 0;

// 背景星
let stars = [];
for (let i = 0; i < 100; i++) {
	stars.push({
		x: Math.random() * canvas.width,
		y: Math.random() * canvas.height,
		size: Math.random() * 2 + 1,
		speed: Math.random() * 2 + 1
	});
}

function putStrings(x, y, text) {
	const len = text.length;
	for (let i = 0; i < len; i++) {
		if (text[i] !== ' ') {
			// 文字コードを取得して '0' の文字コードを引く
			const patNo = text.charCodeAt(i) - '0'.charCodeAt(0);
			ctx.drawImage(fontImage, (patNo % 16) * 16, Math.floor(patNo / 16) * 16, 16, 16, x, y, 16, 16);
		}
		x = x + 16;
	}
}

function putStringsNum(x, y, str, num, digit) {
	let text = "";
	const len = str.length;
	let i = digit;
	let j = num;

	// 1.	最初の文字列（例: "SCORE: "）を描画
	putStrings(x, y, str);

	// 2.	数値を下1桁から順に文字に変換して、文字列の先頭に結合していく
	while (i > 0) {
		i--;
		// j % 10 で一番右の桁の数値を取得し、文字に変換
		text = String.fromCharCode((j % 10) + '0'.charCodeAt(0)) + text;
		// JavaScriptの除算は浮動小数点になるため、Math.floorで整数化（10で割って1桁削る）
		j = Math.floor(j / 10);
	}

	// 3.	最初の文字列の長さ分だけ右にずらした位置に、変換した数値文字列を描画
	putStrings(x + (len * fontSize), y, text);
}

function putSprite(x, y, patNo) {
	ctx.drawImage(sourceImage, patNo * 32, 0, 31, 31, x, y, 31, 31);
}

function drawPlayer() {
	/*	 ctx.fillStyle = '#0f0';
			ctx.beginPath();
			ctx.moveTo(player.x, player.y + player.height/2);
			ctx.lineTo(player.x + player.width, player.y);
			ctx.lineTo(player.x + player.width - 10, player.y + player.height/2);
			ctx.lineTo(player.x + player.width, player.y + player.height);
			ctx.closePath();
			ctx.fill();
	
			// コックピット
			ctx.fillStyle = '#ff0';
			ctx.fillRect(player.x + 25, player.y + 12, 8, 6);
	*/
	//		ctx.drawImage(sourceImage, 32, 0, 32, 32, player.x, player.y, 32, 32);
	putSprite(player.x, player.y, 1);
}

function drawEnemyBullet(eb) {
	putSprite(eb.x, eb.y, 0);
}

function drawBullet(b) {
	/*		ctx.fillStyle = '#ff0';
			ctx.fillRect(b.x, b.y - 3, 20, 6);*/
	//		ctx.drawImage(sourceImage, 32*4, 0, 32, 32, b.x, b.y, 32, 32);
	putSprite(b.x, b.y, 4);
}

function drawEnemy(e) {
	/*		ctx.fillStyle = '#f44';
			ctx.beginPath();
			ctx.moveTo(e.x + e.width, e.y + e.height/2);
			ctx.lineTo(e.x, e.y);
			ctx.lineTo(e.x + 10, e.y + e.height/2);
			ctx.lineTo(e.x, e.y + e.height);
			ctx.closePath();
			ctx.fill();*/
	//		ctx.drawImage(sourceImage, 32*2, 0, 32, 32, e.x, e.y, 32, 32);
	putSprite(e.x, e.y, 2);
}

function drawChainItem(i) {
	putSprite(i.x, i.y, 3);
}

function drawItem(i) {
	patNo = 0;
	switch (i.types) {
		case 1:
			patNo = 8;
			break;
		case 2:
			patNo = 7;
			break;
		case 3:
			patNo = 9;
			break;
	}
	putSprite(i.x, i.y, patNo);
}

function drawOption(o) {
	putSprite(o.x, o.y, 10);
}

function drawShield() {
	putSprite(player.x, player.y, 6);
}

function useBomb() {
	if (bombStock <= 0 || (bombActive == true)) {
		return;
	}

	bombStock--;
	bombActive = true;
	bombTimer = 1;//bombDuration;

	// 敵と敵弾を全滅
	enemies = [];
	enemyBullets = [];

	// 大量の破片を発生
	createExplosion(player.x + 16, player.y + 16, 0); // 大爆発

	// 画面全体に破片を散らす
	for (i = 0; i < 60; i++) {
		let rx = Math.random() * canvas.width;
		let ry = Math.random() * canvas.height;
		createExplosion(rx, ry)
	}

	score += 200
	wav.pause();
	wav.currentTime = 0;
	wav.play();
}

function update() {

	timestamp = Date.now();
	deltaTime = (timestamp - lastTime) / 1000;
	lastTime = timestamp;
	rate = deltaTime * count1S;
	for (let star of stars) {
		star.x -= star.speed * rate;
		if (star.x < 0) star.x = canvas.width;
	}

	key = 0;
	const gamepads = navigator.getGamepads();
	if (gamepads[0]) {
		const gp = gamepads[0];
		gp.buttons.forEach((button, index) => {
			if (button.pressed) {
				//					console.log(`ボタン ${index} が押されています`);
				if (index == 0)
					key |= 0x01;	// A
				if (index == 1)
					key |= 0x02;	// B
				if (index == 2)
					key |= 0x04;
				if (index == 3)
					key |= 0x08;
				if (index == 12)
					key |= 0x10;	// U
				if (index == 13)
					key |= 0x20;	// D
				if (index == 14)
					key |= 0x40;	// L
				if (index == 15)
					key |= 0x80;	// R
			}
		});
	}

	if (gameRunning == 1) {
		if (!(keys[' '] || keys['z'] || (key & 0x01))) {
			gameRunning = 2;
		} else {
			return;
		}
	}

	if (gameRunning == 2) {
		if (keys[' '] || keys['z']) { // || (key & 0x01))
			startGame();
			easyMode = false;
			life = 1;
		} else if (keys['x'] || keys['b']) { // || (key & 0x02))
			startGame();
			easyMode = true;
			life = 3;
		} else {
			return;
		}
	}

	gameTime += deltaTime;

	// プレイヤー移動
	if (keys['ArrowLeft'] || keys['a'] || keys['A'] || (key & 0x40)) player.x -= player.speed * rate;
	if (keys['ArrowRight'] || keys['d'] || keys['D'] || (key & 0x80)) player.x += player.speed * rate;
	if (keys['ArrowUp'] || keys['w'] || keys['W'] || (key & 0x10)) player.y -= player.speed * rate;
	if (keys['ArrowDown'] || keys['s'] || keys['S'] || (key & 0x20)) player.y += player.speed * rate;


	// 画面端制限
	player.x = Math.max(0, Math.min(canvas.width - 40, player.x));
	player.y = Math.max(0, Math.min(canvas.height - 32, player.y));


	// オプション更新
	for (let i = options.length - 1; i >= 0; i--) {
		const opt = options[i];
		// 滑らかに追従
		opt.x += ((player.x + 16) - opt.x) / 4 * rate;
		opt.y += ((player.y + opt.offsetY) - opt.y) / 4 * rate;
	}

	// 自動射撃（連射）
	const now = Date.now();
	if ((keys[' '] || keys['z'] || (key & 0x01)) && now - lastShot > 150) {
		bullets.push({
			x: player.x + player.width,
			y: player.y + player.height / 2,
			width: 16,
			height: 8,
			left: 0,
			top: 0,
		});
		for (let i = options.length - 1; i >= 0; i--) {
			const opt = options[i];
			bullets.push({
				x: opt.x + 8,
				y: opt.y + 12,
				width: 16,
				height: 8,
				left: 0,
				top: 0,
			});
		}

		lastShot = now;
	}

	if ((keys['x'] || keys['b'] || (key & 0x02)) && (bombStock > 0) && (bombActive == false)) {
		useBomb();
	}

	// 弾更新
	for (let i = bullets.length - 1; i >= 0; i--) {
		bullets[i].x += 12 * rate;
		if (bullets[i].x > canvas.width) bullets.splice(i, 1);
	}

	// 敵生成
	spawnTimer += deltaTime;
	let baseInterval = (50.0 - (score / 250.0)); // scoreが増えるほど短く
	let spawnInterval = Math.max((18.0) / count1S, baseInterval / count1S); // フレーム→秒に変換

	//		if (Math.random() < 0.03) {
	if (spawnTimer > spawnInterval) {
		randNum = Math.random() * 100;
		if (randNum < 60) {
			etype = 0;
		} else if (randNum < 85) {
			etype = 1;
		} else {
			etype = 2;
		}
		if (etype == 0) {
			ehp = 1;
		} else {
			ehp = 3;
		}

		enemies.push({
			x: canvas.width, //50,
			y: 32 + Math.random() * (canvas.height - 32 - 32 - 32),
			width: 32,
			height: 32,
			left: 0,
			top: 0,
			speed: 3 + Math.random() * 2,
			type: etype,
			life: ehp,
			count: 0,
			count2: Math.random() * (30 * 2 + canvas.width - 40 * 2) - 30 * 2,
			shootTimer: 0,
			nextShootTime: 5.0 / count1S
		});
		spawnTimer = 0;
	}

	// 敵更新
	let enemySpeed = 4.0 * rate;
	for (let i = enemies.length - 1; i >= 0; i--) {
		const e = enemies[i];
		//			e.x -= e.speed;

		e.count += rate;

		switch (e.type) {
			case 0:
				e.x -= enemySpeed;
				break;

			case 1:
				if (e.count < 24) {
					e.x -= 6 * 2 * rate;
					e.y += ((player.y + 8 - e.y) / 8) / 2 * rate;
				} else if (e.count < 49) {
					e.x -= 0;
				} else {
					e.x += 6 * 2 * rate;
				}
				break;

			case 2:
				e.x -= enemySpeed;
				e.y = (e.count2 + Math.sin(e.count * 0.12) * 55 * 2);
				break;
		}
		//			e.x -= e.speed * $rate;


		// 敵弾発射処理
		e.shootTimer += deltaTime;

		let difficulty = Math.min(1, Math.floor(gameTime / 180)); // * COUNT1S)))
		let enemyBulletSpeed = (4 + difficulty * 2);
		let shootInterval = ((82 - difficulty * 36) - 5) / count1S;
		//			console.log(`難易度 ${difficulty}`);

		if (e.shootTimer >= e.nextShootTime) {

			let dx = player.x - e.x;
			let dy = player.y - e.y;

			let dist = 0;
			if (Math.abs(dx) > Math.abs(dy)) {
				dist = Math.abs(dx);
			} else {
				dist = Math.abs(dy);
			}
			if (dist == 0) {
				dist = 1;
			}

			// 弾を発射
			let bulletSpeed = enemyBulletSpeed;

			dx = (dx * bulletSpeed / dist);
			dy = (dy * bulletSpeed / dist);
			dx = Math.max(-3 * 2.0, dx);
			dx = Math.min(dx, 4 * 2.0);
			dy = Math.max(-4 * 2.0, dy);
			dy = Math.min(dy, 4 * 2.0);

			enemyBullets.push({
				x: e.x + 16,
				y: e.y + 16,
				vx: dx, // * bulletSpeed - 1.0f*1, // vx
				vy: dy, // * bulletSpeed	 // vy
				width: 8,
				height: 8,
				left: 0,
				top: 0
			});

			// 次回の発射間隔を設定
			e.nextShootTime = shootInterval;

			e.shootTimer = 0.0;
		}


		// プレイヤーと衝突
		if (checkCollision(player, e)) {
			if (shieldActive) {
				shieldActive = false; // シールド消費
				createExplosion(player.x + 16, player.y + 16); // 大きな爆発
			} else {
				life--;
				lifeEl.textContent = life;
				createExplosion(e.x, e.y);
				enemies.splice(i, 1);

				if (life <= 0) {
					gameOver();
				}
			}
			continue;
		}

		if ((e.x < -32) || (e.x > canvas.width)) enemies.splice(i, 1);
	}

	// 衝突判定（弾 vs 敵）
	for (let i = bullets.length - 1; i >= 0; i--) {
		const b = bullets[i];
		for (let j = enemies.length - 1; j >= 0; j--) {
			const e = enemies[j];
			if (checkCollision(b, e)) {
				e.life--;
				createExplosion(e.x, e.y);
				bullets.splice(i, 1);
				if (e.life <= 0) {

					// オプションアイテム出現
					if (optionCooldown <= 0) {
						items.push({
							x: e.x,
							y: e.y,
							timer: 300, // 約5秒で消える
							types: 1,   // 1 = オプションアイテム
						});
						optionCooldown = 10;
					} else {
						optionCooldown--
					}

					// シールドアイテム出現（確率12%程度）
					if (Math.random() * 100 < 12 && !shieldActive) {
						items.push({
							x: e.x,
							y: e.y,
							timer: 280.0,
							types: 2, // 2 = シールド
						});
					}

					// ボムアイテム出現
					if (Math.random() * 100 < 10) { // 約10%の確率
						items.push({
							x: e.x,
							y: e.y,
							timer: 270.0,
							types: 3, // 3 = ボム
						});
					}

					// === チェインアイテム出現 ===
					if (Math.random() * 100 < 40) { // 40%くらいの確率で落とす
						chainItems.push({
							x: e.x,
							y: e.y,
							timer: 240.0,
						});
					}

					score += 100;
					scoreEl.textContent = score;
					enemies.splice(j, 1);
					wav.pause();
					wav.currentTime = 0;
					wav.play();
				}
				break;
			}
		}
	}

	// 敵弾 vs 自機
	for (let i = enemyBullets.length - 1; i >= 0; i--) {
		const it = enemyBullets[i];

		if (checkCollision(player, it)) {
			if (shieldActive) {
				shieldActive = false; // シールド消費
				createExplosion(player.x + 16, player.y + 16); // 大きな爆発
			} else {
				life--;
				lifeEl.textContent = life;
				createExplosion(player.x, player.y);

				if (life <= 0) {
					gameOver();
				}
			}
			enemyBullets.splice(i, 1);
			break;
		}
	}

	// 敵弾移動&画面範囲外判定
	for (let i = enemyBullets.length - 1; i >= 0; i--) {
		const it = enemyBullets[i];
		it.x += it.vx * rate;
		it.y += it.vy * rate;

		if ((it.x < -32) || (it.x > canvas.width) || (it.y < 32) || (it.y > canvas.height)) {
			enemyBullets.splice(i, 1);
		}
	}

	// アイテム更新
	for (let i = items.length - 1; i >= 0; i--) {
		const it = items[i];

		switch (it.types) {
			case 1:
				it.x -= 2.0 * rate; // 左に流れる
				break;

			case 2:
				it.x -= 4.0 * rate; // 左に流れる
				break;

			case 3:
				it.x -= 4.0 * rate; // 左に流れる
				break;
		}
		it.timer -= game.delta;


		// 自機とアイテムの当たり判定
		if (Math.abs(it.x - player.x) < 44 - 16 && Math.abs(it.y - player.y) < 44 - 16) {

			if (it.types == 1 && optionNum < maxOptions) { // オプションアイテム
				offset = 25;
				if (optionNum == 0) {
					offset = 25.0;
				} else {
					offset = -25.0;
				}
				options.push({
					offsetY: offset * 2,
					x: 0, //playerX + 20
					y: 0, //playerY + 16 + offset
					//                opt.angle = 0.0
				});
				optionNum++;
			} else if (it.types == 2) { // シールド
				shieldActive = true;
				//                shieldTimer = shieldDuration
			} else if (it.types == 3) { // 3 = ボムアイテム
				bombStock = Math.min(3, bombStock + 1);
			}

			laser.pause();
			laser.currentTime = 0;
			laser.play();

			items.splice(i, 1);
			continue;
		}

		// 画面外 or 時間切れ
		if (it.x < -40 || it.timer <= 0) {
			items.splice(i, 1);
		}
	}

	// パーティクル更新
	for (let i = particles.length - 1; i >= 0; i--) {
		const p = particles[i];
		p.x += p.vx;
		p.y += p.vy;
		p.life--;
		if (p.life <= 0) particles.splice(i, 1);
	}

	// ボム更新
	if (bombActive) {
		bombTimer -= deltaTime;
		if (bombTimer <= 0.0) {
			bombActive = false;
		}
	}

	// チェインアイテム更新
	for (let i = chainItems.length - 1; i >= 0; i--) {
		const it = chainItems[i];
		it.x -= 4.0 * rate; // 左に流れる
		it.timer -= deltaTime;

		// 自機取得判定
		if (Math.abs(it.x - player.x) < 44 - 16 && Math.abs(it.y - player.y) < 44 - 16) {
			chainCount++;
			chainTimer = 240 / count1S; // チェイン持続時間リセット
			score += chainCount * 100;  // チェイン数に応じたボーナス

			chainItems.splice(i, 1);
			laser.pause();
			laser.currentTime = 0;
			laser.play();
			continue;
		}

		// 時間切れ or 画面外
		if (it.timer <= 0.0 || it.x < -20) {
			chainCount = 0;
			chainItems.splice(i, 1);
		}
	}

	// チェインタイマー減少
	if (chainTimer > 0.0) {
		chainTimer -= deltaTime;
		if (chainTimer <= 0.0) {
			chainCount = 0;
		}
	}

	if (gameOver != 0 && score > highScore) {
		highScore = score;
	}

}

function checkCollision(a, b) {
	/*		if (!b.width) { // 弾の場合
				return (
					a.x < b.x + 20 &&
					a.x + (a.width || 40) > b.x &&
					a.y < b.y + 6 &&
					a.y + (a.height || 30) > b.y
				);
			}
			return !(
				a.x + (a.width || 40) < b.x ||
				a.x > b.x + b.width ||
				a.y + (a.height || 30) < b.y ||
				a.y > b.y + b.height
			);*/
	return !(
		a.x + a.left + a.width < b.x + b.left ||
		a.x + a.left > b.x + b.left + b.width ||
		a.y + a.top + a.height < b.y + b.top ||
		a.y + a.top > b.y + b.top + b.height
	);
}

function createExplosion(x, y) {
	for (let i = 0; i < 15; i++) {
		particles.push({
			x: x + 20,
			y: y + 15,
			vx: Math.random() * 8 - 4,
			vy: Math.random() * 8 - 4,
			life: 20 + Math.random() * 20,
			color: Math.random() > 0.5 ? '#ff0' : '#f80'
		});
	}
}

function draw() {
	// 背景
	ctx.fillStyle = '#000011';
	ctx.fillRect(0, 0, canvas.width, canvas.height);

	// 星
	ctx.fillStyle = '#fff';
	for (let star of stars) {
		ctx.globalAlpha = 0.3 + Math.random() * 0.7;
		ctx.fillRect(star.x, star.y, star.size, star.size);
	}

	ctx.globalAlpha = 1;

	// パーティクル
	for (let p of particles) {
		ctx.fillStyle = p.color;
		ctx.globalAlpha = p.life / 30;
		ctx.fillRect(p.x, p.y, 4, 4);
	}
	ctx.globalAlpha = 1;

	// オブジェクト描画

	chainItems.forEach(drawChainItem);
	items.forEach(drawItem);
	options.forEach(drawOption);
	enemyBullets.forEach(drawEnemyBullet);
	bullets.forEach(drawBullet);
	enemies.forEach(drawEnemy);
	if (shieldActive) {
		drawShield();
	}
	drawPlayer();

	//		putStrings(0,0,"0123");

	if (score >= highScore) {
		putStringsNum(0, 0, "HIGH  ", score, 7);
	} else {
		putStringsNum(0, 0, "SCORE ", score, 7);
	}
	if (easyMode == true) {
		putStringsNum(0, 2 * fontSize, "LIVES ", life, 1);
	}
	putStringsNum(0, 1 * fontSize, "BOMB  ", bombStock, 1);

	putStringsNum(16 * fontSize, 0, "COUNT ", Math.floor(gameTime), 7);

	if (chainCount > 0) {
		putStringsNum(16 * fontSize, 1 * fontSize, "CHAIN ", chainCount, 3);
	}

	if (gameRunning != 0) {
		putStrings(11 * fontSize, 12 * fontSize, "GAME OVER");
		putStringsNum(7 * fontSize, 15 * fontSize, "HIGH SCORE ", highScore, 7);

		putStrings(7 * fontSize, 18 * fontSize, "PRESS A TO RESTART");
	}

}

function gameLoop() {
	update();
	draw();
	requestAnimationFrame(gameLoop);
}

function startGame() {
	//		document.getElementById('start-screen').style.display = 'none';
	gameRunning = 0;
	score = 0;
	scoreEl.textContent = score;
	lifeEl.textContent = life;
	bullets = [];
	enemies = [];
	enemyBullets = [];
	options = [];
	items = [];
	chainItems = [];
	particles = [];

	player.x = 60;
	player.y = 160;

	bombStock = 0;
	bombTimer = 0;
	bombActive = true;
	gameTime = 0;
	chainCount = 0;
	chainTimer = 0;
	optionCooldown = 10;
	optionNum = 0;

	lastTime = Date.now();

	easyMode = true;
	life = 3;
	shieldActive = false;

	audio.play();
}

function gameOver() {
	gameRunning = 1;
	audio.pause();			// ① 止める
	audio.currentTime = 0;	// ② 頭出し（0秒に戻す）
	laser.pause();
	laser.currentTime = 0;
	wav.pause();
	wav.currentTime = 0;

	//		alert(`ゲームオーバー！\n最終スコア:	 ${score}`);
	//		document.getElementById('start-screen').style.display = 'block';
	//		startGame();
}

// キー入力
window.addEventListener('keydown', e => {
	keys[e.key] = true;
});

window.addEventListener('keyup', e => {
	keys[e.key] = false;
});

// ゲーム開始
gameLoop();
