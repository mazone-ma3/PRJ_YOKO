	function setfullscreen() {

		//   Chrome  &	Firefox	v64以降
		if(	canvas.requestFullscreen )   {
				 canvas.requestFullscreen();
		  
		//   Firefox   v63以前
		}  else   if(   canvas.mozRequestFullScreen   ) {
		  canvas.mozRequestFullScreen();

		//   Safari  &	Edge &   Chrome  v68以前
		}  else   if(   canvas.webkitRequestFullscreen  )	{
		  canvas.webkitRequestFullscreen();
		  
		//   IE11
		}  else   if(   canvas.msRequestFullscreen  )	{
		  canvas.msRequestFullscreen();
		}		 

	}

	function resetfullscreen()   {
		//   Chrome  &	Firefox	v64以降
		if(	document.exitFullscreen	)  {
		  document.exitFullscreen();

		//   Firefox   v63以前
		}  else   if(   document.mozCancelFullScreen	)  {
		  document.mozCancelFullScreen();

		//   Safari  &	Edge &   Chrome  v44以前
		}  else   if(   document.webkitCancelFullScreen   ) {
		  document.webkitCancelFullScreen();

		//   IE11
		}  else   if(   document.msExitFullscreen )   {
		  document.msExitFullscreen();
		}
	}

	window.addEventListener('load',	function(){

		//   フルスクリーン表示
		document.getElementById('button1').addEventListener('click', function(){

			setfullscreen();

		 });

		 //	フルスクリーン解除
		 document.getElementById('button2').addEventListener('click',  function(){
//			audio.mute   = false;
			resetfullscreen();

		 });
	});

	const  FONT_SIZE	=  16;
	const  COUNT1S  =	60;

	const  canvas =   document.getElementById('game');
	const  ctx  =	canvas.getContext('2d');
	const  scoreEl  =	document.getElementById('score');
	const  lifeEl =   document.getElementById('life');

	const  audio	=  new  Audio('bgm.mp3');
	audio.loop   = true;
	audio.mute   = false;
	const  wav  =	new	Audio('explosion.wav');

	sourceImage	=  document.getElementById("yokosht");
	fontImage  =	document.getElementById("fontyoko");

	let	gameRunning	=  1;
	let	score  =	0;
	let	high_score   = 5000;
	let	life =   3;

	let	easy_mode  =	false;
	let	bomb_stock   = 0;
	let	gameTime =   0;
	let	chain_count	=  0;

	let	lastTime =   0;
	let	rate =   0;

	//   プレイヤー
	const  player =   {
		x:   60,
		y:   160,
		width:   32,
		height:	20,
		left:  0,
		top: 6,
		speed:   4
	};

	let	bullets	=  [];
	let	enemies	=  [];
	let	particles  =	[];	//   爆発エフェクト

	let	keys =   {};
	let	lastShot =   0;

	//   背景星
	let	stars  =	[];
	for	(let i   = 0;	i  <	100; i++)  {
		stars.push({
			x:   Math.random() *   canvas.width,
			y:   Math.random() *   canvas.height,
			size:  Math.random()	*  2	+  1,
			speed:   Math.random() *   2 +   1
		});
	}

	function put_strings(x,	y,   text) {
		const  len  =	text.length;
		for	(let i   = 0;	i  <	len; i++)  {
			if (text[i]	!==	'  ') {
			//   文字コードを取得して	'0'	の文字コードを引く
				const  pat_no =   text.charCodeAt(i)  -	'0'.charCodeAt(0);
				ctx.drawImage(fontImage, (pat_no %   16)   * 16, Math.floor(pat_no   / 16) *   16,   16,   16,   x,  y,  16,  16);
			}
			x  =	x  +	16;
		}
	}

	function put_strings_num(x,	y,   str,	num, digit)	{
		let	text =   "";
		const  len  =	str.length;
		let	i  =	digit;
		let	j  =	num;

		//   1.  最初の文字列（例:	"SCORE:	"）を描画
		put_strings(x,   y,  str);

		//   2.  数値を下1桁から順に文字に変換して、文字列の先頭に結合していく
		while  (i >   0)  {
			i--;
			//   j %   10  で一番右の桁の数値を取得し、文字に変換
			text =   String.fromCharCode((j  %	10)	+  '0'.charCodeAt(0)) +   text;
			//   JavaScriptの除算は浮動小数点になるため、Math.floorで整数化（10で割って1桁削る）
			j  =	Math.floor(j /   10);
		}

		//   3.  最初の文字列の長さ分だけ右にずらした位置に、変換した数値文字列を描画
		put_strings(x  +	(len *   FONT_SIZE),   y,  text);
	}

	function put_sprite(x,   y,  pat_no)  {
		ctx.drawImage(sourceImage,   pat_no  *	32,	0,   31,   31,   x,  y,  31,  31);
	}

	function drawPlayer()  {
/*		  ctx.fillStyle	=  '#0f0';
		ctx.beginPath();
		ctx.moveTo(player.x, player.y  +	player.height/2);
		ctx.lineTo(player.x	+  player.width,	player.y);
		ctx.lineTo(player.x	+  player.width   - 10, player.y  +	player.height/2);
		ctx.lineTo(player.x	+  player.width,	player.y +   player.height);
		ctx.closePath();
		ctx.fill();

		//   コックピット
		ctx.fillStyle  =	'#ff0';
		ctx.fillRect(player.x  +	25,	player.y +   12,   8,  6);
*/
//		  ctx.drawImage(sourceImage, 32, 0,	32,	32,	player.x,  player.y,	32,	32);
		put_sprite(player.x,   player.y, 1);
	}

	function drawBullet(b)   {
/*		  ctx.fillStyle	=  '#ff0';
		ctx.fillRect(b.x,  b.y  -	3,   20,   6);*/
//		  ctx.drawImage(sourceImage, 32*4,   0,  32,  32,  b.x,   b.y,	32,	32);
		put_sprite(b.x,  b.y,   4);
	}

	function drawEnemy(e)  {
/*		  ctx.fillStyle	=  '#f44';
		ctx.beginPath();
		ctx.moveTo(e.x   + e.width,  e.y  +	e.height/2);
		ctx.lineTo(e.x,	e.y);
		ctx.lineTo(e.x   + 10, e.y +   e.height/2);
		ctx.lineTo(e.x,	e.y	+  e.height);
		ctx.closePath();
		ctx.fill();*/
//		  ctx.drawImage(sourceImage, 32*2,   0,  32,  32,  e.x,   e.y,	32,	32);
		put_sprite(e.x,  e.y,   2);
	}

	function update()  {
		key	=  0;
		const  gamepads   = navigator.getGamepads();
		if(gamepads[0])	{
			const  gp =   gamepads[0];
			gp.buttons.forEach((button,	index)   =>  {
				if (button.pressed)	{
//					console.log(`ボタン	${index} が押されています`);
					if(index ==	0)
						key	|=   0x01;	//   A
					if(index ==	1)
						key	|=   0x02;	//   B
					if(index ==	2)
						key	|=   0x04;
					if(index ==	3)
						key	|=   0x08;
					if(index ==	12)
						key	|=   0x10;	//   U
					if(index ==	13)
						key	|=   0x20;	//   D
					if(index ==	14)
						key	|=   0x40;	//   L
					if(index ==	15)
						key	|=   0x80;	//   R
				}
			});
		}

		if (gameRunning	==   1)  {
			if (!(keys[' ']	||   keys['z'] ||	(key &   0x01))){
				gameRunning	=  2;
			}else{
				return;
			}
		}

		if (gameRunning	==   2)  {
			if (keys['   ']  || keys['z'])	{  // ||	(key &   0x01))
				startGame();
				easy_mode  =	false;
				life =   1;
			}else  if (keys['x']	||   keys['b'])  {	//   ||  (key   & 0x02))
				startGame();
				easy_mode  =	true;
				life =   3;
			}else{
				return;
			}
		}

		timestamp  =	Date.now();
		deltaTime  =	(timestamp   - lastTime)   / 1000;
		lastTime =   timestamp;
		gameTime +=	deltaTime;
		rate =   deltaTime *   COUNT1S;


			//   プレイヤー移動
		if (keys['ArrowLeft']  || keys['a']   ||  keys['A']	||   (key	&  0x40)) player.x  -= player.speed  *	rate;
		if (keys['ArrowRight']   ||  keys['d']	||   keys['D'] ||	(key &   0x80))  player.x   +=  player.speed   * rate;
		if (keys['ArrowUp']	||   keys['w'] ||	keys['W']  || (key  &	0x10))   player.y	-=   player.speed	*  rate;
		if (keys['ArrowDown']  || keys['s']   ||  keys['S']	||   (key	&  0x20)) player.y  += player.speed  *	rate;


		//   画面端制限
		player.x =   Math.max(0,   Math.min(canvas.width -   player.width - 6, player.x));
		player.y =   Math.max(0,   Math.min(canvas.height  -	player.height - 6,   player.y));

		//   自動射撃（連射）
		const  now  =	Date.now();
		if ((keys['	']   ||  keys['z']	||   (key	&  0x01)) &&	now	-  lastShot   > 150)  {
			bullets.push({
				x:   player.x	+  player.width,
				y:   player.y	+  player.height/2,
				width:   16,
				height:	8,
				left:  0,
				top: 0,
			});
			lastShot =   now;
		}

		//   弾更新
		for	(let i   = bullets.length	-  1; i   >=  0; i--)  {
			bullets[i].x +=	12   * rate;
			if (bullets[i].x >   canvas.width) bullets.splice(i,   1);
		}

		//   敵生成
		if (Math.random()  <	0.03)  {
			enemies.push({
				x:   canvas.width	+  50,
				y:   Math.random() *   (canvas.height  -	40),
				width:   32,
				height:	32,
				left:  0,
				top: 0,
				speed:   3 +   Math.random() *   2
			});
		}

		//   敵更新
		for	(let i   = enemies.length	-  1; i   >=  0; i--)  {
			const  e	=  enemies[i];
			e.x	-=   e.speed;

			//   プレイヤーと衝突
			if (checkCollision(player,   e))   {
				life--;
				lifeEl.textContent   = life;
				createExplosion(e.x, e.y);
				enemies.splice(i,  1);

				if (life <=	0)   {
					gameOver();
				}
				continue;
			}

			if (e.x	<  -50)   enemies.splice(i, 1);
		}

		//   衝突判定（弾	vs   敵）
		for	(let i   = bullets.length	-  1; i   >=  0; i--)  {
			const  b	=  bullets[i];
			for	(let j   = enemies.length	-  1; j   >=  0; j--)  {
				const  e	=  enemies[j];
				if (checkCollision(b,  e))  {
					score  += 100;
					scoreEl.textContent	=  score;
					createExplosion(e.x, e.y);
					bullets.splice(i,  1);
					enemies.splice(j,  1);
					wav.pause();
					wav.currentTime	=  0;
					wav.play();
					break;
				}
			}
		}

		//   パーティクル更新
		for	(let i   = particles.length  -	1;   i >=	0;   i--)	{
			const  p	=  particles[i];
			p.x	+=   p.vx;
			p.y	+=   p.vy;
			p.life--;
			if (p.life   <=  0) particles.splice(i, 1);
		}
	}

	function checkCollision(a,   b)  {
/*		  if (!b.width)	{  // 弾の場合
			return   (
				a.x	<  b.x  +	20   &&
				a.x	+  (a.width   ||  40)  >	b.x	&&
				a.y	<  b.y  +	6  &&
				a.y	+  (a.height	||   30)   > b.y
			);
		}
		return   !(
			a.x	+  (a.width   ||  40)  <	b.x	||
			a.x	>  b.x  +	b.width	||
			a.y	+  (a.height	||   30)   < b.y ||
			a.y	>  b.y  +	b.height
		);*/
		return   !(
			a.x	+  a.left +   a.width   < b.x +   b.left  ||
			a.x	+  a.left >   b.x   + b.left	+  b.width  ||
			a.y	+  a.top	+  a.height   < b.y +   b.top ||
			a.y	+  a.top	>  b.y  +	b.top  +	b.height
		);
	}

	function createExplosion(x,	y)   {
		for	(let i   = 0;	i  <	15;	i++) {
			particles.push({
				x:   x +   20,
				y:   y +   15,
				vx:	Math.random()  *	8  -	4,
				vy:	Math.random()  *	8  -	4,
				life:  20 +   Math.random() *   20,
				color:   Math.random() >   0.5   ? '#ff0'	:  '#f80'
			});
		}
	}

	function draw()	{
		//   背景
		ctx.fillStyle  =	'#000011';
		ctx.fillRect(0,	0,   canvas.width, canvas.height);

		//   星
		ctx.fillStyle  =	'#fff';
		for	(let star  of stars)	{
			ctx.globalAlpha	=  0.3  +	Math.random()  *	0.7;
			ctx.fillRect(star.x, star.y, star.size,	star.size);
			star.x   -=  star.speed;
			if (star.x   < 0)	star.x   = canvas.width;
		}
		ctx.globalAlpha	=  1;

		//   オブジェクト描画
		drawPlayer();

		bullets.forEach(drawBullet);
		enemies.forEach(drawEnemy);

		//   パーティクル
		for	(let p   of  particles) {
			ctx.fillStyle  =	p.color;
			ctx.globalAlpha	=  p.life /   30;
			ctx.fillRect(p.x,  p.y,   4,  4);
		}

		ctx.globalAlpha	=  1;

//		put_strings(0,0,"0123");

		if (score  >= high_score) {
			put_strings_num(0,   0,  "HIGH	 ",	score,   7);
		}  else   {
			put_strings_num(0,   0,  "SCORE ",	score,   7);
		}
		if (easy_mode  == true)   {
			put_strings_num(0,   2*FONT_SIZE,	"LIVES   ",  life,	1);
		}
		put_strings_num(0,   1*FONT_SIZE,	"BOMB   ",  bomb_stock,  1);

		put_strings_num(16*FONT_SIZE,  0, "COUNT	",   Math.floor(gameTime), 7);

		if (chain_count	>  0) {
			put_strings_num(16*FONT_SIZE,  1*FONT_SIZE,   "CHAIN  ", chain_count,  3);
		}

		if (gameRunning	!=   0)  {
			put_strings(11*FONT_SIZE,  12*FONT_SIZE,	"GAME  OVER");
			put_strings_num(7*FONT_SIZE, 15*FONT_SIZE,   "HIGH SCORE   ",  high_score,  7);

			put_strings(7*FONT_SIZE, 18*FONT_SIZE,   "PRESS  A	TO   RESTART");
		}

	}

	function gameLoop()	{
		update();
		draw();
		requestAnimationFrame(gameLoop);
	}

	function startGame() {
//		  document.getElementById('start-screen').style.display	=  'none';
		gameRunning	=  0;
		score  =	0;
		scoreEl.textContent	=  score;
		lifeEl.textContent   = life;
		bullets	=  [];
		enemies	=  [];
		particles  =	[];
		player.x =   60;
		player.y =   160;

		bomb_stock   = 0;
		gameTime =   0;
		chain_count	=  0;

		lastTime =   Date.now();

		easy_mode  =	true;
		life =   3;

		audio.play();
	}

	function gameOver()	{
		gameRunning	=  1;
		audio.pause();		   //  ① 止める
		audio.currentTime  =	0;   //  ② 頭出し（0秒に戻す）
//		  alert(`ゲームオーバー！\n最終スコア:   ${score}`);
//		  document.getElementById('start-screen').style.display	=  'block';
//		startGame();
	}

	//   キー入力
	window.addEventListener('keydown',   e =>	{
		keys[e.key]	=  true;
	});

	window.addEventListener('keyup', e   =>  {
		keys[e.key]	=  false;
	});

	//   ゲーム開始
	gameLoop();
