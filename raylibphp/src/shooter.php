<?php
require dirname(__DIR__) . "/vendor/autoload.php";
//$ffi = FFI::load("../vendor/Kingbes/Raylib/src/raylib.h");

use Kingbes\Raylib\Core;
use Kingbes\Raylib\Shapes;
use Kingbes\Raylib\Text;
use Kingbes\Raylib\Utils;
use Kingbes\Raylib\Audio;
use Kingbes\Raylib\Textures;

// ==================== 定数 ====================
const X_SCALE = 2;
const Y_SCALE = 2;

const SCREEN_WIDTH = 256 * 2 * X_SCALE;
const SCREEN_HEIGHT = 192 * 2 * Y_SCALE;
const PLAYER_WIDTH = 32;
const PLAYER_HEIGHT = 20;
const BULLET_WIDTH = 16;
const BULLET_HEIGHT = 8;
const ENEMY_WIDTH = 32;
const ENEMY_HEIGHT = 32;

const PLAYER_SPEED = 4;
const BULLET_SPEED = 12;
const ENEMY_SPEED = 4;
const SCROLL_SPEED = -3;

const FIRE_RATE = 8;         // 値が小さいほど連射が速くなる
const ENEMY_SPAWN_RATE = 25; // ★値を25にして敵を多く出やすくしました

const FLAG_WINDOW_RESIZABLE  = 0x4;
const FLAG_WINDOW_UNDECORATED = 0x08;
const FLAG_WINDOW_TOPMOST = 0x00001000;
const FLAG_FULLSCREEN_MODE    = 0x00000002;
const FLAG_WINDOW_HIGHDPI     = 0x00002000;
const FLAG_VSYNC_HINT = 0x40;
const TEXTURE_FILTER_POINT = 0;

const FONT_SIZE = 16;

const COUNT1S = 60;

// キー番号の定義
const KEY_RIGHT = 262;
const KEY_LEFT = 263;
const KEY_DOWN = 264;
const KEY_UP = 265;
const KEY_R = 82;
const KEY_SPACE = 32;
const KEY_Z = 90;
const KEY_X = 88;
const KEY_B = 66;
const KEY_W = 87;
const KEY_S = 83;
const KEY_A = 65;
const KEY_D = 68;
const KEY_F11 = 300;

enum GamePad:int {
    case GAMEPAD_BUTTON_UNKNOWN = 0;         // Unknown button, for error checking
    case GAMEPAD_BUTTON_LEFT_FACE_UP = 1;        // Gamepad left DPAD up button
    case GAMEPAD_BUTTON_LEFT_FACE_RIGHT = 2;     // Gamepad left DPAD right button
    case GAMEPAD_BUTTON_LEFT_FACE_DOWN = 3;      // Gamepad left DPAD down button
    case GAMEPAD_BUTTON_LEFT_FACE_LEFT = 4;      // Gamepad left DPAD left button
    case GAMEPAD_BUTTON_RIGHT_FACE_UP = 5;       // Gamepad right button up (i.e. PS3: Triangle, Xbox: Y)
    case GAMEPAD_BUTTON_RIGHT_FACE_RIGHT = 6;    // Gamepad right button right (i.e. PS3: Circle, Xbox: B)
    case GAMEPAD_BUTTON_RIGHT_FACE_DOWN = 7;     // Gamepad right button down (i.e. PS3: Cross, Xbox: A)
    case GAMEPAD_BUTTON_RIGHT_FACE_LEFT = 8;     // Gamepad right button left (i.e. PS3: Square, Xbox: X)
    case GAMEPAD_BUTTON_LEFT_TRIGGER_1 = 9;      // Gamepad top/back trigger left (first), it could be a trailing button
    case GAMEPAD_BUTTON_LEFT_TRIGGER_2 = 10;      // Gamepad top/back trigger left (second), it could be a trailing button
    case GAMEPAD_BUTTON_RIGHT_TRIGGER_1 = 11;     // Gamepad top/back trigger right (first), it could be a trailing button
    case GAMEPAD_BUTTON_RIGHT_TRIGGER_2 = 12;     // Gamepad top/back trigger right (second), it could be a trailing button
    case GAMEPAD_BUTTON_MIDDLE_LEFT = 13;         // Gamepad center buttons, left one (i.e. PS3: Select)
    case GAMEPAD_BUTTON_MIDDLE = 14;              // Gamepad center buttons, middle one (i.e. PS3: PS, Xbox: XBOX)
    case GAMEPAD_BUTTON_MIDDLE_RIGHT = 15;        // Gamepad center buttons, right one (i.e. PS3: Start)
    case GAMEPAD_BUTTON_LEFT_THUMB = 16;          // Gamepad joystick pressed button left
    case GAMEPAD_BUTTON_RIGHT_THUMB = 17;          // Gamepad joystick pressed button right
}
enum GamePadAxis:int {
    case GAMEPAD_AXIS_LEFT_X = 0;       // Gamepad left stick X axis
    case GAMEPAD_AXIS_LEFT_Y = 1;       // Gamepad left stick Y axis
    case GAMEPAD_AXIS_RIGHT_X = 2;      // Gamepad right stick X axis
    case GAMEPAD_AXIS_RIGHT_Y = 3;      // Gamepad right stick Y axis
    case GAMEPAD_AXIS_LEFT_TRIGGER = 4; // Gamepad back trigger left, pressure level: [1..-1]
    case GAMEPAD_AXIS_RIGHT_TRIGGER = 5; // Gamepad back trigger right, pressure level: [1..-1]
}

// ==================== 初期化 ====================

class Player
{
	public $x;
	public $y;
	public $lives;
	public $width;
	public $height;
	public $top;
	public $left;

    public function __construct($x, $y, $lives, $width, $height, $left, $top) {
		$this->x = $x;
		$this->y = $y;
		$this->lives = $lives;
		$this->width = $width;
		$this->height = $height;
		$this->left = $left;
		$this->top = $top;
	}
}

class Star
{
	public $x;
	public $y;
	public $speed;

    public function __construct($x, $y, $speed) {
		$this->x = $x;
		$this->y = $y;
		$this->speed = $speed;
	}
}

class Bullet
{
	public $x;
	public $y;
	public $width;
	public $height;
	public $top;
	public $left;

    public function __construct($x, $y, $width, $height, $left, $top) {
		$this->x = $x;
		$this->y = $y;
		$this->width = $width;
		$this->height = $height;
		$this->left = $left;
		$this->top = $top;
	}
}


class Enemy
{
	public $x;
	public $y;
	public $type;
	public $lives;
	public $count;
	public $count2;
	public $shootTimer;
	public $nextShootTime;
	public $speed;
	public $width;
	public $height;
	public $top;
	public $left;

    public function __construct($x, $y, $etype, $lives, $count, $count2, $shootTimer, $nextShootTime, $speed, $width, $height, $left, $top) {
		$this->x = $x;
		$this->y = $y;
		$this->type = $etype;
		$this->lives = $lives;

		$this->$count = $count;
		$this->$count2 = $count2;
		$this->$shootTimer = $shootTimer;
		$this->$nextShootTime = $nextShootTime;
		$this->$speed = $speed;

		$this->width = $width;
		$this->height = $height;
		$this->left = $left;
		$this->top = $top;
		$this->speed = $speed;
	}
}

class Particle
{
	public $x;
	public $y;
	public $vx;
	public $vy;

	public $lives;

    public function __construct($x, $y, $vx, $vy, $lives) {
		$this->x = $x;
		$this->y = $y;
		$this->vx = $vx;
		$this->vy = $vy;
		$this->lives = $lives;
	}
}


class Game
{
    public $se;
    public $bgm;
    public $target;
    public $chrTex;
    public $fontTex;

    public $bgColor;
    public $playerColor;
    public $bulletColor;
    public $enemyColor;
    public $textColor;
    public $whiteColor;
    public $blackColor;

        // ゲーム全体で使う変数
    public $player;

    public $bullets;
    public $enemies;
    public $enemybullets;

    public $score;
    public $high_score;

    public $easy_mode; //false;

    public $bomb_stock;
	public $bomb_timer;
	public $bomb_active;

    public $gameTime;
    public $chain_count;


    public $scrollOffset;
    public $stars;

    public $shootTimer;
    public $spawnTimer; // ★ staticをやめて、ここで通常の変数として管理します
    public $gameOver; // 起動時はゲームオーバー（タイトル画面代わり）

    public $isFullscreen;


    public function __construct() {

        Core::SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT); // | FLAG_WINDOW_HIGHDPI);
//Core::SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_VSYNC_HINT);

        Core::initWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PHP-Raylib 横スクロールシューティング");

        $this->target = Textures::LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
        Textures::SetTextureFilter($this->target->texture, TEXTURE_FILTER_POINT);

        $this->chrTex = Textures::LoadTexture(__DIR__ . "/yokosht.png"); // 画像がなければ後で矩形で代用
        $this->fontTex = Textures::LoadTexture(__DIR__ . "/FONTYOKO.png");

        Audio::InitAudioDevice();
//      Core::setTargetFPS(60);


        $this->se = Audio::LoadSound(__DIR__ . "/explosion.wav");
        $this->bgm = Audio::LoadMusicStream(__DIR__ . "/bgm.mp3");

        $this->bgColor = Utils::color(5, 5, 25, 255);
        $this->playerColor = Utils::color(0, 200, 255, 255);
        $this->bulletColor = Utils::color(255, 220, 0, 255);
        $this->enemyColor = Utils::color(255, 60, 60, 255);
        $this->textColor = Utils::color(255, 255, 255, 255);
        $this->whiteColor = Utils::color(255, 255, 255, 255);
        $this->blackColor = Utils::color(0, 0, 0, 0);

        $this->isFullscreen = false;
        $this->easy_mode = true; //false;

        $this->scrollOffset = 0;
        $this->stars = [];
/*
        for ($i = 0; $i < 80; $i++) {
            $this->stars[] = [
                'x' => rand(0, SCREEN_WIDTH),
                'y' => rand(0, SCREEN_HEIGHT),
                'speed' => rand(1, 3) * X_SCALE
            ];
        }*/

        for ($i = 0; $i < 80; $i++) {
			$this->stars[] = new Star(
				rand(0, SCREEN_WIDTH),
            	rand(0, SCREEN_HEIGHT),
            	rand(1, 3) * X_SCALE
			);
		}
/*        foreach ($this->stars as $star) {
			$star->x = rand(0, SCREEN_WIDTH);
            $star->y =rand(0, SCREEN_HEIGHT);
            $star->speed = rand(1, 3) * X_SCALE;
		}*/

        $this->high_score = 5000;
        $this->reset();
    }

    function reset() {
        // ゲーム全体で使う変数
/*        $this->player = [
            'x' => 60,
            'y' => SCREEN_HEIGHT / Y_SCALE / 2 - PLAYER_HEIGHT / 2,
            'lives' => 3
        ];*/

		$this->player = new Player(
			60,
			(SCREEN_HEIGHT / Y_SCALE / 2 - PLAYER_HEIGHT / 2),
			3,
			PLAYER_WIDTH,
			PLAYER_HEIGHT,
			0, 6
		);
/*		$this->player->x = 60;
		$this->player->y =  SCREEN_HEIGHT / Y_SCALE / 2 - PLAYER_HEIGHT / 2;
		$this->player->lives = 3;
		$this->player->width = PLAYER_WIDTH;
		$this->player->height = PLAYER_HEIGHT;
*/
        $this->bullets = [];
        $this->enemies = [];
        $this->enemybullets = [];

		$this->particles  = [];

        $this->score = 0;

        $this->bomb_stock = 0;
		$this->bomb_timer = 0;
		$this->bomb_active = false;

        $this->gameTime = 0;
        $this->chain_count = 0;

        $this->shootTimer = 0;
        $this->spawnTimer = 0; // ★ staticをやめて、ここで通常の変数として管理します
        $this->gameOver = true; // 起動時はゲームオーバー（タイトル画面代わり）
    }

    public function main() {
        while (!Core::windowShouldClose()) {

    //    $delta = 0.17;
        Audio::UpdateMusicStream($this->bgm);

        if (Core::isKeyPressed(KEY_F11)) {
/*        Core::ToggleFullscreen();
        $isFullscreen = !$isFullscreen;
        if ($isFullscreen) {
            Core::SetWindowState(FLAG_FULLSCREEN_MODE | FLAG_VSYNC_HINT);
        }else{
            Core::SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
        }
    }*/
            $this->isFullscreen = !$this->isFullscreen;

            if ($this->isFullscreen) {
                // 1. 現在ウィンドウがあるモニターの番号を取得
                $monitor = Core::getCurrentMonitor();

                // 2. そのモニターの「本来の最大解像度」を取得
                $w = Core::getMonitorWidth($monitor);
                $h = Core::getMonitorHeight($monitor);

                // 3. ウィンドウの枠（ボーダー）を消す
                Core::setWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST);

                // 4. ウィンドウのサイズをモニター最大に合わせる
                Core::setWindowSize($w, $h);

                // 5. ウィンドウの位置を左上（0, 0）に移動させて画面を覆う
                Core::setWindowPosition(0, 0);
            } else {
                // ウィンドウモードに戻す処理
                Core::clearWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST);

                Core::setWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

                // 画面中央に戻す（お好みで）
                $monitor = Core::getCurrentMonitor();
                $mw = Core::getMonitorWidth($monitor);
                $mh = Core::getMonitorHeight($monitor);
                Core::setWindowPosition(($mw - SCREEN_WIDTH) / 2, ($mh - SCREEN_HEIGHT) / 2);
            }
        }

/*    if (Core::isWindowFullscreen()) {
            // フルスクリーンになったら、現在のモニターの最大解像度にウィンドウサイズを合わせる
        $monitor = Core::getCurrentMonitor();
        Core::setWindowSize(Core::getMonitorWidth($monitor), Core::getMonitorHeight($monitor));
    } else {
        // ウィンドウモードに戻ったら、元の設定サイズに戻す
        Core::setWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    }*/


        $scale = (float)min((float)Core::getScreenWidth() / (float)SCREEN_WIDTH, (float)Core::getScreenHeight() / (float)SCREEN_HEIGHT);
//    if (Core::isWindowFullscreen()) {
//       $monitor = Core::getCurrentMonitor();
//       var_dump(Core::getMonitorWidth($monitor));
//       var_dump(Core::getMonitorHeight($monitor));
//    }

        $destRec = Utils::rectangle((Core::getScreenWidth() - SCREEN_WIDTH * $scale)*0.5, (Core::getScreenHeight() - SCREEN_HEIGHT * $scale) * 0.5, SCREEN_WIDTH * $scale, SCREEN_HEIGHT * $scale);

            $this->update();

        Core::beginTextureMode($this->target);

            $this->draw();

        Core::endTextureMode($this->target);

        Core::beginDrawing();
        Core::clearBackground($this->blackColor);
        $sourceRec = Utils::rectangle(0.0, 0.0, $this->target->texture->width, -$this->target->texture->height);
        $origin = Utils::vector2(0.0, 0.0);
        Textures::DrawTexturePro($this->target->texture, $sourceRec, $destRec, $origin, 0.0, $this->whiteColor);

        Core::endDrawing();

        }

//      $this->__destruct();
    }

	function createParticles($x, $y, $count) {
		for ($i = 0; $i < $count; $i++) {
			$this->particles[] = new Particle(
				$x, $y,
				rand(-50, 50) * 0.12,
				rand(-50, 50) * 0.12,
				 20.0 + rand(0, 25)
			);
		}
	}

    function put_strings(float $x, float $y, string $text): void 
    {
//      global $fontTex; // Raylibのテクスチャポインタなどをグローバルから参照する場合

        $len = strlen($text);
        for ($i = 0; $i < $len; $i++) {
            if ($text[$i] !== ' ') {
                // ord() で文字コード（ASCII値）を取得して計算します
                $pat_no = ord($text[$i]) - ord('0');

                $rotation = 0.0;

                // PHPのRaylibバインディングを想定したオブジェクト（または配列）の生成
                // ここでは仮にクラスのインスタンス、または stdClass としています
/*            $destRect = (object)[
                'x' => $x, 
                'y' => $y, 
                'width' => 16 * X_SCALE - 1, 
                'height' => 16 * Y_SCALE - 1
            ];*/
                $destRect = Utils::rectangle($x * X_SCALE, $y * Y_SCALE, 16 * X_SCALE - 1, 16 * Y_SCALE - 1);
            
            // PHPの除算は浮動小数点になるため、(int)でキャストするか intdiv() を使います
/*            $sourceRect = (object)[
                'x' => 16.0 * ($pat_no % 16), 
                'y' => 16.0 * (int)($pat_no / 16), 
                'width' => 16.0, 
                'height' => 16.0
            ];*/
                $sourceRect = Utils::rectangle(16.0 * ($pat_no % 16), 16.0 * (int)($pat_no / 16), 16.0, 16.0);

//            $origin = (object)['x' => 0, 'y' => 0];
                $origin = Utils::vector2(0, 0); 
                $whiteColor = Utils::color(255, 255, 255, 255);

            // 描画関数の呼び出し（名前空間やクラス名は環境に合わせて調整してください）
                Textures::DrawTexturePro($this->fontTex, $sourceRect, $destRect, $origin, $rotation, $this->whiteColor);
            }
            $x += FONT_SIZE;
        }
    }

    function put_strings_num(float $x, float $y, string $str, int $num, int $digit): void 
{
        $text = "";
        $len = strlen($str);
        $i = $digit;
        $j = $num;

        // 1. 最初の文字列を描画
        $this->put_strings($x, $y, $str);

        // 2. 数値を下1桁から順に文字に変換して結合
        while ($i > 0) {
            $i--;
            // chr() で文字コードから文字（文字列）に変換し、ドット(.)で結合します
            $text = chr(($j % 10) + ord('0')) . $text;
            // 10で割ってintにキャストすることで、下の桁を切り捨てます
            $j = (int)($j / 10);
        }

        // 3. 最初の文字列の長さ分だけ右にずらした位置に数値文字列を描画
        $this->put_strings($x + (float)($len * FONT_SIZE), $y, $text);
    }


    function put_sprite($x, $y, $pat_no) {
    //  $destRect = Utils::rectangle($x*X_SCALE, $y*Y_SCALE, 32*X_SCALE-1, 32*Y_SCALE-1);
        $destRect = Utils::rectangle($x * X_SCALE, $y * Y_SCALE, 32*X_SCALE-1, 32*Y_SCALE-1);
        $sourceRect = Utils::rectangle(32*$pat_no, 0, 32, 32);
        $origin = Utils::vector2(0, 0); //destRect.width/2, destRect.height/2 }
        $rotation = 0;
        $whiteColor = Utils::color(255, 255, 255, 255);

        Textures::DrawTexturePro($this->chrTex, $sourceRect, $destRect, $origin, $rotation, $whiteColor);
    }

    // ==================== 敵生成関数 ====================
    function spawnEnemy(&$enemies, &$etype, &$ehp) {
        // 1回につき1?2体を同時に生成して配列に追加する
        $count = 1; //rand(1, 2);
        for ($i = 0; $i < $count; $i++) {
            $enemies[] = new Enemy(
                SCREEN_WIDTH / X_SCALE, // + rand(10, 100),
                rand(32, SCREEN_HEIGHT / Y_SCALE - 64),
				$etype,
				$ehp,
				0,
				rand(0, 30*2+SCREEN_HEIGHT/Y_SCALE-40*2) - 30*2,
				0,
				5.0 / COUNT1S,
                ENEMY_SPEED + rand(-1, 3),
                ENEMY_WIDTH,
                ENEMY_HEIGHT,
				0, 0
			);

/*			[
                'x' => SCREEN_WIDTH / X_SCALE + rand(10, 100),
                'y' => rand(40, SCREEN_HEIGHT / Y_SCALE - 80),
                'width' => ENEMY_WIDTH,
                'height' => ENEMY_HEIGHT,
                'speed' => ENEMY_SPEED + rand(-1, 3)
            ];*/
        }
    }

    // ==================== 衝突判定関数 ====================
/*    function checkCollision($a, $b) {
        return !($a['x'] + ($a['width'] ?? PLAYER_WIDTH) < $b['x'] ||
                 $a['x'] > $b['x'] + ($b['width'] ?? ENEMY_WIDTH) ||
                 $a['y'] + ($a['height'] ?? PLAYER_HEIGHT) < $b['y'] ||
                 $a['y'] > $b['y'] + ($b['height'] ?? ENEMY_HEIGHT));
    }
*/
    function checkCollision($a, $b) {
        return !($a->x + $a->left + $a->width < $b->x + $b->left ||
                 $a->x + $a->left > $b->x + $b->left + $b->width ||
                 $a->y + $a->top + $a->height < $b->y + $b->top ||
                 $a->y + $a->top > $b->y + $b->top + $b->height);

/*        return !($a->x + $a->width < $b['x'] ||
                 $a->x > $b['x'] + ($b['width'] ?? ENEMY_WIDTH) ||
                 $a->y + $a->height < $b['y'] ||
                 $a->y > $b['y'] + ($b['height'] ?? ENEMY_HEIGHT));*/
    }

	function usebomb() {
		if ($this->bomb_stock <= 0 || $this->bomb_active) {
			return;
		}

		$this->bomb_stock--;
		$this->bomb_active = true;
		//   game. bomb_timer = BOMB_DURATION

		// 敵と敵弾を全滅
/*		foreach( i = range game.enemies {
			game.enemies[i].Active = false
		}
		for i = range game.enemybullets {
			game.enemybullets[i].Active = false
		}
*/
        $this->enemies = [];
        $this->enemybullets = [];

	// 大量の破片を発生
		$this->CreateParticles($this->player->x + 16, $this->player->y + 16, 45, 1); // 大爆発

	// 画面全体に破片を散らす
		for($i = 0; $i < 60; $i++) {
			$rx = rand(0, SCREEN_WIDTH / X_SCALE);
			$ry = rand(0, SCREEN_HEIGHT / Y_SCALE);
			$this->CreateParticles($rx, $ry, 6, 1);
		}

		$this->score += 200;
		Audio::PlaySound($this->se);
	}

    // ==================== メインループ ====================
    function update() {
        $delta = Core::GetFrameTime();
        $rate = COUNT1S * $delta;

		$gamepad = 0;

        // 背景スクロール
//        $this->scrollOffset = ($this->scrollOffset + SCROLL_SPEED) % SCREEN_WIDTH;

        if (!$this->gameOver) {
            $this->gameTime += $delta;

			// 1. ゲームパッドが接続されているかチェック
			$axisX = 0;
			$axisY = 0;
			if (Core::isGamepadAvailable($gamepad)) {
				// 2. アナログスティック（左スティック）の入力を取得
				// 戻り値は -1.0f から 1.0f の間
				$axisX = Core::getGamepadAxisMovement($gamepad, GamePadAxis::GAMEPAD_AXIS_LEFT_X->value);
				$axisY = Core::getGamepadAxisMovement($gamepad, GamePadAxis::GAMEPAD_AXIS_LEFT_Y->value);
			}

            // --- プレイヤー移動 ---
            if (Core::isKeyDown(KEY_UP) || Core::isKeyDown(KEY_W) || ($axisY < -0.2) || (Core::isGamepadAvailable($gamepad) && Core::isGamePadButtonDown($gamepad, GamePad::GAMEPAD_BUTTON_LEFT_FACE_UP->value))) $this->player->y -= PLAYER_SPEED * $rate;
            if (Core::isKeyDown(KEY_DOWN) || Core::isKeyDown(KEY_S) || ($axisY > 0.2) || (Core::isGamepadAvailable($gamepad) && Core::isGamePadButtonDown($gamepad, GamePad::GAMEPAD_BUTTON_LEFT_FACE_DOWN->value))) $this->player->y += PLAYER_SPEED * $rate;
            if (Core::isKeyDown(KEY_LEFT) || Core::isKeyDown(KEY_A) || ($axisX < -0.2) || (Core::isGamepadAvailable($gamepad) && Core::isGamePadButtonDown($gamepad, GamePad::GAMEPAD_BUTTON_LEFT_FACE_LEFT->value))) $this->player->x -= PLAYER_SPEED * $rate;
            if (Core::isKeyDown(KEY_RIGHT) || Core::isKeyDown(KEY_D) || ($axisX > 0.2) || (Core::isGamepadAvailable($gamepad) && Core::isGamePadButtonDown($gamepad, GamePad::GAMEPAD_BUTTON_LEFT_FACE_RIGHT->value))) $this->player->x += PLAYER_SPEED * $rate;

            // 画面端制限
            $this->player->x = max(0, min($this->player->x, SCREEN_WIDTH / X_SCALE - 40));
            $this->player->y = max(0, min($this->player->y, SCREEN_HEIGHT / Y_SCALE  - 32));

            // --- 射撃（連射） ---
            $this->shootTimer += $rate;
            if ((Core::isKeyDown(KEY_SPACE) || Core::isKeyDown(KEY_Z) || (Core::isGamepadAvailable($gamepad) && Core::isGamepadButtonDown($gamepad, GamePad::GAMEPAD_BUTTON_RIGHT_FACE_DOWN->value))) && $this->shootTimer > FIRE_RATE) {
                // 弾を配列に「追加（push）」していく（複数表示を可能にする）
/*                $this->bullets[] = [
                    'x' => $this->player->x + 32,
                    'y' => $this->player->y + 12,
                    'width' => BULLET_SIZE * 2,
                    'height' => BULLET_SIZE
                ];*/
               $this->bullets[] = new Bullet(
					$this->player->x + 32, 
					$this->player->y + 12, 
					BULLET_WIDTH,
					BULLET_HEIGHT,
					0, 0
				);
/*                    'x' => $this->player->x + 32,
                    'y' => $this->player->y + 12,
                    'width' => BULLET_SIZE * 2,
                    'height' => BULLET_SIZE
                ];*/
  
               $this->shootTimer = 0;
            }

			if ((Core::isKeyPressed(KEY_X) || Core::isKeyPressed(KEY_B) || (Core::isGamepadAvailable($gamepad) && Core::isGamepadButtonPressed($gamepad, GamePad::GAMEPAD_BUTTON_RIGHT_FACE_RIGHT->value))) && $this->bomb_stock > 0 && !$this->bomb_active) {
				$this->usebomb();
			}

            // --- 敵生成タイムカウント ---
            $this->spawnTimer += $delta;

			$baseInterval = (50.0 - ($this->score / 250.0)); // scoreが増えるほど短く
			$spawnInterval = max((18.0)/COUNT1S, $baseInterval/COUNT1S); // フレーム→秒に変換

//            if ($this->spawnTimer > ENEMY_SPAWN_RATE) {
            if ($this->spawnTimer > $spawnInterval) {

				$rand_num = rand(0, 100);
				if ($rand_num < 60) {
					$etype = 0;
				} else if ($rand_num < 85) {
					$etype = 1;
				} else {
					$etype = 2;
				}
				if ($etype == 0) {
					$ehp = 1;
				} else {
					$ehp = 3;
				}

                $this->spawnEnemy($this->enemies, $etype, $ehp);
                $this->spawnTimer = 0;
            }

            // --- 子弾の移動と画面外削除 ---
/*            foreach ($this->bullets as $i => $b) {
                $this->bullets[$i]['x'] += BULLET_SPEED * $rate;
                if ($this->bullets[$i]['x'] > SCREEN_WIDTH + 20) {
                    unset($this->bullets[$i]);
                }
            }*/
            foreach ($this->bullets as $i => $b) {
                $b->x += BULLET_SPEED * $rate;
                if ($b->x > SCREEN_WIDTH + 20) {
                    unset($this->bullets[$i]);
                }
            }

            $this->bullets = array_values($this->bullets);

			$enemySpeed = 4.0 * $rate;
            // --- 敵の移動と画面外削除 ---
            foreach ($this->enemies as $i => $e) {
//                $this->enemies[$i]['x'] -= $e['speed'] * $rate;

				$e->count += $rate;

				switch($e->type) {
				case 0:
					$e->x -= $enemySpeed;
					break;

				case 1:
					if($e->count < 24){
						$e->x -= 6 * 2 * $rate;
						$e->y += (($this->player->y + 8 - $e->y) / 8) / 2 * $rate;
					} else if ($e->count < 49) {
						$e->x -= 0;
					} else {
						$e->x += 6 * 2 * $rate;
					}
					break;

				case 2:
					$e->x -= $enemySpeed;
					$e->y = ($e->count2 + sin($e->count * 0.12) * 55 * 2);
					break;
				}
//				$e->x -= $e->speed * $rate;

//                if ($this->enemies[$i]['x'] < -60) {
                if ($e->x < -60) {
                    unset($this->enemies[$i]);
                }else{
//					$this->enemies[$i]->x = $e->x;
				}
            }
            $this->enemies = array_values($this->enemies);

            // --- 衝突判定（子弾 と 敵）---
            $hitBullets = [];
            $hitEnemies = [];
            foreach ($this->bullets as $bi => $b) {
                foreach ($this->enemies as $ei => $e) {
                    if (in_array($ei, $hitEnemies)) continue;
                    if ($this->checkCollision($b, $e)) {
						$e->lives--;
                        $hitBullets[] = $bi;
						$this->createParticles($e->x+16, $e->y+16, 8, 0); // 通常爆発
						if($e->lives <= 0) {
	                        $hitEnemies[] = $ei;
	                        $this->score += 100;	
	                        Audio::PlaySound($this->se);
						}
                        break; // この弾のチェックを終えて次の弾へ
                    }
                }
            }
            // 当たったものを一括削除
            foreach ($hitBullets as $bi) unset($this->bullets[$bi]);
            foreach ($hitEnemies as $ei) unset($this->enemies[$ei]);
            $this->bullets = array_values($this->bullets);
            $this->enemies = array_values($this->enemies);

            // --- 衝突判定（プレイヤー と 敵）---
            foreach ($this->enemies as $ei => $e) {
                if ($this->checkCollision($this->player, $e)) {
                    unset($this->enemies[$ei]);
                    $this->player->lives--;
                    if ($this->player->lives <= 0) {
                        $this->gameOver = true;
                        Audio::StopMusicStream($this->bgm);
                    }
                    break;
                }
            }
            $this->enemies = array_values($this->enemies);

			// パーティクル更新
            foreach ($this->particles as $i => $p) {
				if($p->lives <= 0) {
                    unset($this->particles[$i]);
				} else {
	                $p->x += $p->vx * $rate;
	                $p->y += $p->vy * $rate;
					$damping = Pow(0.96, $rate);
					$p->vx *= $damping;
					$p->vy *= $damping;
					$p->lives -= $rate;
				}
            }
            $this->particlees = array_values($this->particles);

			// ボム更新
			if ($this->bomb_active) {
				$this->bomb_timer -= $delta;
				if ($this->bomb_timer <= 0.0) {
					$this->bomb_active = false;
				}
			}

			if ($this->gameOver != 0 && $this->score > $this->high_score) {
				$this->high_score = $this->score;
			}

        } else {
            // --- ゲームオーバー状態（Rキーでリセット） ---
            if (Core::isKeyPressed(KEY_R) || Core::isKeyPressed(KEY_Z) || Core::isKeyPressed(KEY_SPACE) || (Core::isGamepadAvailable($gamepad) && Core::isGamepadButtonPressed($gamepad, GamePad::GAMEPAD_BUTTON_RIGHT_FACE_DOWN->value))) {
                $this->reset();
                $this->player->lives = 1;
                $this->easy_mode = false;
                $this->gameOver = false;
                Audio::PlayMusicStream($this->bgm);
            } else if (Core::isKeyPressed(KEY_X) || Core::isKeyPressed(KEY_B) || (Core::isGamepadAvailable($gamepad) && Core::isGamepadButtonPressed($gamepad, GamePad::GAMEPAD_BUTTON_RIGHT_FACE_RIGHT->value))) {
                $this->reset();
                $this->player->lives = 3;
                $this->easy_mode = true;
                $this->gameOver = false;
                Audio::PlayMusicStream($this->bgm);
			}
        }
    }

    function draw() {
        // ==================== 描画処理 ====================
        Core::clearBackground($this->bgColor);

        // 星空背景
        foreach ($this->stars as $star) {
            Shapes::drawCircle($star->x, $star->y, 1.5, Utils::color(255, 255, 255, 255));
			$star->x -= $star->speed;
			if ($star->x < 0) {
				$star->x = SCREEN_WIDTH;
			}
		}

/*
        for ($i = 0; $i < 80; $i++) {
            $x = $this->stars[$i]['x'];//($i * 37 + $scrollOffset) % (SCREEN_WIDTH + 100) - 50;
            $y = $this->stars[$i]['y'];//($i * 23) % SCREEN_HEIGHT;
            Shapes::drawCircle($x, $y, 1.5, Utils::color(255, 255, 255, 255));
            $x -= $this->stars[$i]['speed'];
            if ($x < 0) {
                $x = SCREEN_WIDTH;
            }
            $this->stars[$i]['x'] = $x;
        }*/

		// パーティクル描画
        foreach ($this->particles as $p) {
            Shapes::drawCircle($p->x * X_SCALE, $p->y * Y_SCALE, 1.5*2, Utils::color( 253, 249, 0, 255));
		}

        // 子弾の描画（配列にある分だけすべてループ描画）
        foreach ($this->bullets as $b) {
//          Shapes::drawRectangle($b['x'], $b['y'], BULLET_SIZE * 2, BULLET_SIZE, $bulletColor);
//            $this->put_sprite($b['x'], $b['y'], 4);
            $this->put_sprite($b->x, $b->y, 4);
        }

        // 敵の描画（配列にある分だけすべてループ描画）
        foreach ($this->enemies as $e) {
//          Shapes::drawRectangle($e['x'], $e['y'], $e['width'], $e['height'], $enemyColor);
//            $this->put_sprite($e['x'], $e['y'], 2);
            $this->put_sprite($e->x, $e->y, 2);
        }

        // プレイヤー機体
/*    Shapes::drawRectangle($player['x'], $player['y'], PLAYER_SIZE, PLAYER_SIZE, $playerColor);
    Shapes::drawTriangle(
        Utils::vector2($player['x'] + PLAYER_SIZE + 10, $player['y'] + PLAYER_SIZE / 2),
        Utils::vector2($player['x'] + 10, $player['y']),
        Utils::vector2($player['x'] + 10, $player['y'] + PLAYER_SIZE),
        $playerColor
    );*/
//        $this->put_sprite($this->player->x, $this->player->y, 1);
        $this->put_sprite($this->player->x, $this->player->y, 1);


        // UI表示
//    Text::drawText("Score: " . $score, 20, 20, 24, $textColor);

//    Text::drawText("Rest: " . $player['lives'], SCREEN_WIDTH - 180, 20, 24, $textColor);
//    Text::drawText("Cursor Move / Space shoot", 20, SCREEN_HEIGHT - 40, 18, Utils::color(180,180,180,255));


        if ($this->score >= $this->high_score) {
            $this->put_strings_num(0, 0, "HIGH  ", $this->score, 7);
        } else {
            $this->put_strings_num(0, 0, "SCORE ", $this->score, 7);
        }
        if ($this->easy_mode == true) {
//            $this->put_strings_num(0, 2*FONT_SIZE, "LIVES ", $this->player->lives, 1);
            $this->put_strings_num(0, 2*FONT_SIZE, "LIVES ", $this->player->lives, 1);
        }
        $this->put_strings_num(0, 1*FONT_SIZE, "BOMB  ", $this->bomb_stock, 1);

        $this->put_strings_num(16*FONT_SIZE, 0, "COUNT ", floor($this->gameTime), 7 );
            if ($this->chain_count > 0) {
            $this->put_strings_num(16*FONT_SIZE, 1*FONT_SIZE, "CHAIN ", $this->chain_count, 3);
        }

        if ($this->gameOver) {
//        Shapes::drawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Utils::color(0,0,0,180));
//        Text::drawText("GAME OVER / START", SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 60, 45, Utils::color(255,50,50,255));
//        Text::drawText("FINAL SCORE " . $score, SCREEN_WIDTH/2 - 110, SCREEN_HEIGHT/2, 28, $textColor);
//        Text::drawText("PUSH R KEY TO START", SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 + 60, 24, $textColor);

            $this->put_strings(11*FONT_SIZE, 12*FONT_SIZE, "GAME OVER");
            $this->put_strings_num(7*FONT_SIZE, 15*FONT_SIZE, "HIGH SCORE ", $this->high_score, 7);

            $this->put_strings(7*FONT_SIZE, 18*FONT_SIZE, "PRESS A TO RESTART");
        }
    }

    // ==================== 後片付け ====================

    public function __destruct() {
        Textures::UnloadTexture($this->fontTex);
        Textures::UnloadTexture($this->chrTex);

        Audio::UnloadSound($this->se);
        Audio::UnloadMusicStream($this->bgm);
        Audio::CloseAudioDevice();
        Core::closeWindow();
    }

}

$game = new Game();
$game->main();

?>

