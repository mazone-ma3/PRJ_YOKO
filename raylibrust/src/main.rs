use raylib::ffi::Rectangle;
use raylib::prelude::*;
use raylib::ffi;
use raylib::consts::ConfigFlags;
//use std::time::{Duration, Instant};

const X_SCALE: i32 = 2;
const Y_SCALE: i32 = 2;
const FONT_SIZE: i32 = 16;
const COUNT1S: f32 = 60.0;

const SCREEN_WIDTH: i32 = 256 * 2 * X_SCALE;
const SCREEN_HEIGHT: i32 = 192 * 2 * Y_SCALE;
const PLAYER_SPEED: f32 = 4.0 * COUNT1S * X_SCALE as f32;
const BULLET_SPEED: f32 = 12.0 * COUNT1S * X_SCALE as f32;
const ENEMY_SPEED: f32 = 4.0 * COUNT1S * X_SCALE as f32;

#[derive(Clone)]
struct Bullet {
    pos: Vector2,
    active: bool,
}

struct Enemy {
    pos: Vector2,
    active: bool,
}

struct Star {
    pos: Vector2,
    speed: f32,
}

//fn put_strings(font_tex: &Texture2D, d: &mut RaylibDrawHandle ,mut x: f32, y: f32, text: &str) {
fn put_strings(font_tex: &Texture2D, d: &mut RaylibTextureMode<'_, '_, RaylibHandle> ,mut x: i32, y: i32, text: &str) {
    // Rustの文字列はUTF-8なので、バイト列として走査するか、文字(char)として走査します。
    // 今回はフォントテクスチャのアスキー配置（0?255）を前提としているため、バイト列として処理します。
    for &byte in text.as_bytes() {
        if byte != b' ' {
            // 文字コードの引き算。結果をインデックス用に usize（またはi32）にキャスト
            let pat_no = (byte - b'0') as i32;

            let rotation = 0.0f32;

            // 各ライブラリのRectangleやVector2の構造体に合わせます
            let dest_rect = Rectangle::new(
                x as f32 * (X_SCALE as f32), 
                y as f32* (Y_SCALE as f32), 
                16.0 * (X_SCALE as f32) - 1.0, 
                16.0 * (Y_SCALE as f32) - 1.0
            );
            
            // Rustの整数同士の除算・剰余（pat_no % 16）は自動的に整数になります
            let source_rect = Rectangle::new(
                (FONT_SIZE * (pat_no % 16)) as f32,
                (FONT_SIZE * (pat_no / 16)) as f32,
                FONT_SIZE as f32,
                FONT_SIZE as f32,
            );
            
            let origin = Vector2::new(0.0, 0.0);

            // 描画関数の呼び出し
            d.draw_texture_pro(&font_tex, source_rect, dest_rect, origin, rotation, Color::WHITE);
        }
        x = x + FONT_SIZE;
    }
}

//fn put_strings_num(font_tex: &Texture2D, d: &mut RaylibDrawHandle, x: f32, y: f32, str_val: &str, num: i32, digit: i32) {
fn put_strings_num(font_tex: &Texture2D, d: &mut RaylibTextureMode<'_, '_, RaylibHandle>, x: i32, y: i32, str_val: &str, num: i32, digit: i32) {
    let mut text = String::new();
    let len = str_val.chars().count(); // 文字列の長さを取得
    let mut i = digit;
    let mut j = num;
    
    // 1. 最初の文字列を描画
//    let f = font_tex; //.clone() as Texture2D;
    put_strings(&font_tex, d, x, y, str_val);

    // 2. 数値を下1桁から順に文字に変換して結合
    while i > 0 {
        i -= 1;
        // (j % 10) で得た数値に '0' のバイト値を足し、char型に変換
        let digit_char = ((j % 10) as u8 + b'0') as char;
        // 文字列の先頭に挿入
        text.insert(0, digit_char);
        // 整数同士の割り算なので、自動的に小数点以下は切り捨てられます
        j /= 10;
    }

    // 3. 最初の文字列の長さ分だけ右にずらした位置に数値文字列を描画
    put_strings(&font_tex, d, x + (len as i32 * FONT_SIZE), y, &text);
}

//fn put_sprite(chr_tex: &Texture2D, d: &mut RaylibDrawHandle ,x: f32, y: f32, pat_no: i32) {
fn put_sprite(chr_tex: &Texture2D, d: &mut RaylibTextureMode<'_, '_, RaylibHandle> ,x: f32, y: f32, pat_no: i32) {
    let rotation = 0.0f32;

    // 各ライブラリのRectangleやVector2の構造体に合わせます
    let dest_rect = Rectangle::new(
        x, 
        y, 
        32.0 * (X_SCALE as f32) - 1.0, 
        32.0 * (Y_SCALE as f32) - 1.0
    );
            
    // Rustの整数同士の除算・剰余（pat_no % 16）は自動的に整数になります
    let source_rect = Rectangle::new(
        32.0 * pat_no as f32,
        0.0 as f32,
        32.0,
        32.0,
    );
            
    let origin = Vector2::new(0.0, 0.0);

    // 描画関数の呼び出し
    d.draw_texture_pro(&chr_tex, source_rect, dest_rect, origin, rotation, Color::WHITE);
}

fn main() {
    println!("DEBUG: 初期化を開始します...");

    unsafe {
        // 例: フルスクリーンモードとVSyncを有効にする場合
        let flags = ConfigFlags::FLAG_WINDOW_RESIZABLE as u32 | ConfigFlags::FLAG_VSYNC_HINT as u32;
        ffi::SetConfigFlags(flags);
    }

    let (mut rl, thread) = raylib::init()
        .size(SCREEN_WIDTH as i32, SCREEN_HEIGHT as i32)
        .title("Rust 横スクロールシューティング")
        .build();

//	rl.set_target_fps(60);

	let mut target = rl.load_render_texture(&thread, SCREEN_WIDTH as u32, SCREEN_HEIGHT as u32).unwrap();
	target.set_texture_filter(&thread, TextureFilter::TEXTURE_FILTER_POINT);

	let chr_tex = rl.load_texture(&thread, "yokosht.png").expect("load_render_texture"); // 画像がなければ後で矩形で代用
	let font_tex = rl.load_texture(&thread, "FONTYOKO.png").expect("load_render_texture");

    let audio = RaylibAudio::init_audio_device().expect("audio init failed");

	//let laser_sound = audio.new_sound( "laser.wav").expect("sound load failed");
	let explosion_sound = audio.new_sound( "explosion.wav").expect("sound load failed");
	let bgm = audio.new_music( "bgm.mp3").expect("music load failed");


    // ★Raylib側のFPS制限はバグの原因になるため使わない
    println!("DEBUG: 初期化が完了しました。ループに入ります。");

    let mut player_pos = Vector2::new(100.0, (SCREEN_HEIGHT / 2) as f32);
    let mut bullets: Vec<Bullet> = Vec::new();
    let mut enemies: Vec<Enemy> = Vec::new();
    let mut stars: Vec<Star> = Vec::new();
    let mut score = 0;
    let mut high_score = 5000;
    let mut lives = 3;

    let mut easy_mode = true;
    let mut bomb_stock = 0;
    let mut game_time: f32 = 0.0;
    let mut chain_count = 0;

    let mut game_over = true;

    let mut last_shot = rl.get_time();
    let shot_cooldown = 0.15;

    for _ in 0..80 {
        stars.push(Star {
            pos: Vector2::new(
                rl.get_random_value::<i32>(0..=(SCREEN_WIDTH)) as f32,
                rl.get_random_value::<i32>(0..=(SCREEN_HEIGHT)) as f32,
            ),
            speed: rl.get_random_value::<i32>(1..=3) as f32 * X_SCALE as f32,
        })           
    }

    // 手動FPS管理用のタイマー（60FPS = 1フレーム約16.6ミリ秒）
//    let target_frame_time = Duration::from_micros(16666); 

    while !rl.window_should_close() {
        bgm.update_stream();

		if rl.is_key_pressed(KeyboardKey::KEY_F11) {
			rl.toggle_fullscreen();
		}

		let scale = std::cmp::min(rl.get_screen_width() / SCREEN_WIDTH, rl.get_screen_height() / SCREEN_HEIGHT);
		let dest_rec = Rectangle::new((rl.get_screen_width() as f32 - SCREEN_WIDTH as f32 * scale as f32) * 0.5, (rl.get_screen_height() as f32  - SCREEN_HEIGHT as f32 * scale as f32) * 0.5, SCREEN_WIDTH as f32 * scale as f32, SCREEN_HEIGHT as f32 * scale as f32);

//        let frame_start = Instant::now();

        // ★重要: Windowsの「応答なし」を防ぐため、明示的にイベントを回収
//        rl.poll_input_events();

        // 固定デルタタイム（手動で60FPSに制御するため、dtは1/60秒に固定すると挙動が安定します）
        let dt = rl.get_frame_time(); //1.0 / 60.0;
        let now = rl.get_time();
        let rate = dt * COUNT1S;

        if !game_over {
            game_time += dt;

            // --- プレイヤー操作 ---
            if rl.is_key_down(KeyboardKey::KEY_UP) || rl.is_key_down(KeyboardKey::KEY_W) {
                player_pos.y -= PLAYER_SPEED * dt;
            }
            if rl.is_key_down(KeyboardKey::KEY_DOWN) || rl.is_key_down(KeyboardKey::KEY_S) {
                player_pos.y += PLAYER_SPEED * dt;
            }
            if rl.is_key_down(KeyboardKey::KEY_LEFT) || rl.is_key_down(KeyboardKey::KEY_A) {
                player_pos.x -= PLAYER_SPEED * dt;
            }
            if rl.is_key_down(KeyboardKey::KEY_RIGHT) || rl.is_key_down(KeyboardKey::KEY_D) {
                player_pos.x += PLAYER_SPEED * dt;
            }

            player_pos.y = player_pos.y.clamp(0.0, (SCREEN_HEIGHT - 32) as f32);
            player_pos.x = player_pos.x.clamp(0.0, (SCREEN_WIDTH - 40) as f32);

            // --- 射撃 ---
            if (rl.is_key_down(KeyboardKey::KEY_SPACE) || rl.is_key_down(KeyboardKey::KEY_Z))
                && now - last_shot > shot_cooldown
            {
                bullets.push(Bullet {
                    pos: Vector2::new(player_pos.x + 32.0*2.0, player_pos.y + 12.0*2.0),
                    active: true,
                });
                last_shot = now;
            }

            // --- 敵生成（確率を少し調整）---
            if rl.get_random_value::<i32>(0..=40) == 0 {
                enemies.push(Enemy {
                    pos: Vector2::new(
                        (SCREEN_WIDTH + 50) as f32,
                        rl.get_random_value::<i32>(30..=(SCREEN_HEIGHT - 30)) as f32,
                    ),
                    active: true,
                });
            }

            // --- 更新 ---
            for bullet in &mut bullets {
                if bullet.active {
                    bullet.pos.x += BULLET_SPEED * dt;
                    if bullet.pos.x > SCREEN_WIDTH as f32 + 20.0 {
                        bullet.active = false;
                    }
                }
            }

            for enemy in &mut enemies {
                if enemy.active {
                    enemy.pos.x -= ENEMY_SPEED * dt;
                    if enemy.pos.x < -30.0 {
                        enemy.active = false;
                    }
                }
            }

            // 当たり判定
            for bullet in &mut bullets {
                if !bullet.active { continue; }
                for enemy in &mut enemies {
                    if !enemy.active { continue; }
                    if bullet.pos.distance(enemy.pos) < 32.0 {
                        bullet.active = false;
                        enemy.active = false;
                        score += 100;
                        explosion_sound.play();
                    }
                }
            }

            // 衝突
            for enemy in &enemies {
                if enemy.active && player_pos.distance(enemy.pos) < 32.0 {
                    game_over = true;
                    bgm.stop_stream();
                }
            }

            bullets.retain(|b| b.active);
            enemies.retain(|e| e.active);
        } else {
            if rl.is_key_pressed(KeyboardKey::KEY_R) || rl.is_key_pressed(KeyboardKey::KEY_Z)  || rl.is_key_pressed(KeyboardKey::KEY_SPACE){
                player_pos = Vector2::new(100.0, (SCREEN_HEIGHT / 2) as f32);
                bullets.clear();
                enemies.clear();
                score = 0;
                lives = 3;

                easy_mode = true;
                bomb_stock = 0;
                game_time = 0.0;
                chain_count = 0;
                game_over = false;
                bgm.play_stream();
            }
        }

        // --- 描画開始 ---
//        let mut d = rl.begin_drawing(&thread);

//        let mut d = 
        let mut d = rl.begin_texture_mode(&thread, &mut target);

        d.clear_background(Color::BLACK);

        // 背景（シンプルな星）
/*         for i in 0..50 {
            let x = ((i * 37) % SCREEN_WIDTH as usize) as f32;
            let y = ((i * 23) % SCREEN_HEIGHT as usize) as f32;
            d.draw_pixel(x as i32, y as i32, Color::WHITE);
        }*/
        for star in &mut stars {
//            let x = stars.x as f32;
//            let y = stars.y as f32;
            d.draw_pixel(star.pos.x as i32, star.pos.y as i32, Color::WHITE);
//            x += stars.speed;
            star.pos.x -= star.speed * rate;
            if star.pos.x < 0.0 {
                star.pos.x = SCREEN_WIDTH as f32;
            }
        }

        // プレイヤー（三角機体）
        if !game_over {
/*             d.draw_triangle(
                Vector2::new(player_pos.x + 30.0, player_pos.y),
                Vector2::new(player_pos.x - 20.0, player_pos.y - 20.0),
                Vector2::new(player_pos.x - 20.0, player_pos.y + 20.0),
                Color::LIME,
            );
*/
            put_sprite(&chr_tex, &mut d, player_pos.x, player_pos.y, 1);
        }

        // 弾
        for bullet in &bullets {
            if bullet.active {
//                d.draw_circle_v(bullet.pos, 6.0, Color::YELLOW);
                put_sprite(&chr_tex, &mut d, bullet.pos.x, bullet.pos.y, 4);
            }
        }

        // 敵
        for enemy in &enemies {
            if enemy.active {
/*                 d.draw_rectangle_v(
                    Vector2::new(enemy.pos.x - 20.0, enemy.pos.y - 15.0),
                    Vector2::new(40.0, 30.0),
                    Color::RED,
                );
                d.draw_triangle(
                    Vector2::new(enemy.pos.x + 25.0, enemy.pos.y),
                    Vector2::new(enemy.pos.x - 15.0, enemy.pos.y - 18.0),
                    Vector2::new(enemy.pos.x - 15.0, enemy.pos.y + 18.0),
                    Color::MAROON,
                );*/
                put_sprite(&chr_tex, &mut d, enemy.pos.x, enemy.pos.y, 2);
            }
        }

        // UI
//        d.draw_text(&format!("SCORE: {}", score), 20, 20, 30, Color::WHITE);
//        d.draw_text("Cursor Move  SPACE/Z Shot", 20, SCREEN_HEIGHT - 40, 20, Color::GRAY);

//        put_strings_num(&font_tex, &mut d, 0.0, 0.0, "SCORE  ", score, 7);
		if  score >= high_score {
			put_strings_num(&font_tex, &mut d,0, 0, "HIGH  ", score, 7);
		} else {
			put_strings_num(&font_tex, &mut d,0, 0, "SCORE ", score, 7);
		}
		if easy_mode == true {
			put_strings_num(&font_tex, &mut d,0, 2*FONT_SIZE, "LIVES ", lives, 1);
		}
		put_strings_num(&font_tex, &mut d,0, 1*FONT_SIZE, "BOMB  ", bomb_stock, 1);

		put_strings_num(&font_tex, &mut d,16*FONT_SIZE, 0, "COUNT ", game_time as i32, 7);

		if chain_count > 0 {
			put_strings_num(&font_tex, &mut d,16*FONT_SIZE, 1*FONT_SIZE, "CHAIN ", chain_count, 3);
		}

        if game_over {
//            d.draw_text("GAME OVER", SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 - 30, 50, Color::RED);
//            d.draw_text("PUSH R KEY TO RESTART", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 30, 25, Color::WHITE);

			put_strings(&font_tex, &mut d, 11 *FONT_SIZE , 12 *FONT_SIZE, "GAME OVER");
			put_strings_num(&font_tex, &mut d, 7 *FONT_SIZE, 15 *FONT_SIZE, "HIGH SCORE ", high_score, 7);

			put_strings(&font_tex, &mut d, 7 *FONT_SIZE, 18 *FONT_SIZE, "PRESS A TO RESTART");

        }
        drop(d); // 描画終了 (EndTextureMode)

        let mut d = rl.begin_drawing(&thread);
		d.clear_background(Color::BLACK); // フルスクリーン時の「黒帯」になる部分の色

    	let source_rec = Rectangle::new(0.0 as f32, 0.0 as f32, target.texture.width as f32, -target.texture.height as f32);
		let origin = (0.0, 0.0);

		// 計算した位置・サイズ（destRec）で綺麗に拡大描画
		d.draw_texture_pro(&target, source_rec, dest_rec, origin, 0.0, Color::WHITE);
    
        drop(d); // 描画終了 (EndDrawing / ここで画面が更新されます)

        // ★重要: 手動FPS制御（16.6msに満たない時間をスリープしてCPU負荷を下げる）
/*        let elapsed = frame_start.elapsed();
        if elapsed < target_frame_time {
            std::thread::sleep(target_frame_time - elapsed);
        }*/
    }
}