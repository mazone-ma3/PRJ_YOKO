use raylib::ffi::Rectangle;
use raylib::prelude::*;
use raylib::ffi;
use raylib::consts::ConfigFlags;

const X_SCALE: i32 = 2;
const Y_SCALE: i32 = 2;
const FONT_SIZE: i32 = 16;
const COUNT1S: f32 = 60.0;

const SCREEN_WIDTH: i32 = 256 * 2 * X_SCALE;
const SCREEN_HEIGHT: i32 = 192 * 2 * Y_SCALE;
const PLAYER_SPEED: f32 = 4.0;
const BULLET_SPEED: f32 = 12.0;
const ENEMY_SPEED: f32 = 4.0;

#[derive(Clone)]
struct Player {
    pos: Vector2,
    poswh: Vector2,
    poslt: Vector2,
}

struct Bullet {
    pos: Vector2,
    poswh: Vector2,
    poslt: Vector2,
    active: bool,
}

struct Enemy {
    pos: Vector2,
    poswh: Vector2,
    poslt: Vector2,
    lives: i32,
    etype: i32,
    count: f32,
    count2: f32,
    shoot_timer: f32,
    next_shoot_time: f32,
    active: bool,
}

struct EnemyBullet {
    pos: Vector2,
    poswh: Vector2,
    poslt: Vector2,
    v: Vector2,
    active: bool,
}

struct Option {
    pos :Vector2,
    offset_y :f32,
}

struct Item {
    pos: Vector2,
//    poswh: Vector2,
//    poslt: Vector2,
    timer: f32,
    types:  i32,
    active: bool,
}

struct ChainItem {
    pos: Vector2,
//    poswh: Vector2,
//    poslt: Vector2,
    timer: f32,
    active: bool,
}

struct Particle {
    pos: Vector2,
    vpos: Vector2,
    life: f32,
}

struct Star {
    pos: Vector2,
    speed: f32,
}

fn put_strings(font_tex: &Texture2D, d: &mut impl RaylibDraw, mut x: i32, y: i32, text: &str) {
    for &byte in text.as_bytes() {
        if byte != b' ' {
            let pat_no = (byte - b'0') as i32;
            let rotation = 0.0f32;

            let dest_rect = Rectangle::new(
                x as f32 * (X_SCALE as f32), 
                y as f32 * (Y_SCALE as f32), 
                16.0 * (X_SCALE as f32) - 1.0, 
                16.0 * (Y_SCALE as f32) - 1.0
            );
            
            let source_rect = Rectangle::new(
                (FONT_SIZE * (pat_no % 16)) as f32,
                (FONT_SIZE * (pat_no / 16)) as f32,
                FONT_SIZE as f32,
                FONT_SIZE as f32,
            );
            
            let origin = Vector2::new(0.0, 0.0);
            d.draw_texture_pro(&font_tex, source_rect, dest_rect, origin, rotation, Color::WHITE);
        }
        x = x + FONT_SIZE;
    }
}

fn put_strings_num(font_tex: &Texture2D, d: &mut impl RaylibDraw, x: i32, y: i32, str_val: &str, num: i32, digit: i32) {
    let mut text = String::new();
    let len = str_val.chars().count();
    let mut i = digit;
    let mut j = num;
    
    put_strings(&font_tex, d, x, y, str_val);

    while i > 0 {
        i -= 1;
        let digit_char = ((j % 10) as u8 + b'0') as char;
        text.insert(0, digit_char);
        j /= 10;
    }

    put_strings(&font_tex, d, x + (len as i32 * FONT_SIZE), y, &text);
}

fn put_sprite(chr_tex: &Texture2D, d: &mut impl RaylibDraw, x: f32, y: f32, pat_no: i32) {
    let rotation = 0.0f32;

    let dest_rect = Rectangle::new(
        x * X_SCALE as f32, 
        y * Y_SCALE as f32, 
        32.0 * (X_SCALE as f32) - 1.0, 
        32.0 * (Y_SCALE as f32) - 1.0
    );
            
    let source_rect = Rectangle::new(
        32.0 * pat_no as f32,
        0.0 as f32,
        32.0,
        32.0,
    );
            
    let origin = Vector2::new(0.0, 0.0);
    d.draw_texture_pro(&chr_tex, source_rect, dest_rect, origin, rotation, Color::WHITE);
}

fn create_particles(particles: &mut Vec<Particle>, rl: &RaylibHandle, x: f32, y: f32, count: i32) {
    for _ in 0..count {
        particles.push(Particle {
            pos: Vector2::new(x, y),
            vpos: Vector2::new((rl.get_random_value::<i32>(0..=100) as f32 - 50.0) * 0.12, (rl.get_random_value::<i32>(0..=100) as f32 - 50.0) * 0.12),
            life: 20.0 + rl.get_random_value::<i32>(0..=25) as f32,
        });
    }
}

fn check_collision(a_pos: Vector2, a_poswh: Vector2, a_poslt: Vector2, b_pos: Vector2, b_poswh: Vector2, b_poslt: Vector2) -> bool {
    return !(a_pos.x + a_poslt.x + a_poswh.x < b_pos.x + b_poslt.x ||
             a_pos.x + a_poslt.x > b_pos.x + b_poslt.x + b_poswh.x ||
             a_pos.y + a_poslt.y + a_poswh.y < b_pos.y + b_poslt.y ||
             a_pos.y + a_poslt.y > b_pos.y + b_poslt.y + b_poswh.y);
}

fn use_bomb(particles: &mut Vec<Particle>, rl: &RaylibHandle, explosion_sound: &Sound, score: &mut i32, bomb_stock: &mut i32, bomb_active: &mut bool, enemies: &mut Vec<Enemy>, enemy_bullets: &mut Vec<EnemyBullet>, player: &mut Player) {
    if *bomb_stock <= 0 || *bomb_active {
        return;
    }

    *bomb_stock -= 1;
    *bomb_active = true;

    enemies.clear();
    enemy_bullets.clear();

    create_particles(particles, &rl, player.pos.x + 16.0, player.pos.y + 16.0, 45);

    for _ in 0..60 {
        let rx = rl.get_random_value::<i32>(0..= SCREEN_WIDTH / X_SCALE) as f32;
        let ry = rl.get_random_value::<i32>(0..= SCREEN_HEIGHT / Y_SCALE) as f32;
        create_particles(particles, &rl, rx, ry, 6);
    }

    *score += 200;
    explosion_sound.play();
}

fn main() {
//    println!("DEBUG: 初期化を開始します...");

    unsafe {
        let flags = ConfigFlags::FLAG_WINDOW_RESIZABLE as u32 | ConfigFlags::FLAG_VSYNC_HINT as u32;
        ffi::SetConfigFlags(flags);
    }

    let (mut rl, thread) = raylib::init()
        .size(SCREEN_WIDTH as i32, SCREEN_HEIGHT as i32)
        .title("raylib Rust 横スクロールシューティング")
        .build();

    let mut target = rl.load_render_texture(&thread, SCREEN_WIDTH as u32, SCREEN_HEIGHT as u32).unwrap();
    target.set_texture_filter(&thread, TextureFilter::TEXTURE_FILTER_POINT);

    let chr_tex = rl.load_texture(&thread, "yokosht.png").expect("load texture failed");
    let font_tex = rl.load_texture(&thread, "FONTYOKO.png").expect("load font failed");

    let audio = RaylibAudio::init_audio_device().expect("audio init failed");
    let explosion_sound = audio.new_sound("explosion.wav").expect("sound load failed");
    let laser_sound = audio.new_sound("laser.wav").expect("sound load failed");
    let bgm = audio.new_music("bgm.mp3").expect("music load failed");

//    println!("DEBUG: 初期化が完了しました。ループに入ります。");

    let mut player = Player {
        pos: Vector2::new(60.0, 160.0),
        poswh: Vector2::new(32.0, 20.0),
        poslt: Vector2::new(0.0, 6.0),
    };
    let mut bullets: Vec<Bullet> = Vec::new();
    let mut enemies: Vec<Enemy> = Vec::new();
    let mut enemy_bullets: Vec<EnemyBullet> = Vec::new();
    let mut options: Vec<Option> = Vec::new();
    let mut items: Vec<Item> = Vec::new();
    let mut chain_items: Vec<ChainItem> = Vec::new();
    let mut particles: Vec<Particle> = Vec::new();
    let mut stars: Vec<Star> = Vec::new();

    let mut score = 0;
    let mut high_score = 5000;
    let mut lives = 3;

    let mut easy_mode = false;

    let mut bomb_stock = 0;
    let mut bomb_active = false;
    let mut bomb_timer = 0.0;

    let mut game_time: f32 = 0.0;
    let mut chain_count = 0;

    let mut game_over = 1;

    let mut last_shot = rl.get_time();
    let shot_cooldown = 0.15;
    let mut enemy_spawn_timer = 0.0;

    let mut shield_active = false;
    let mut option_cooldown = 10;
    let mut chain_timer = 0.0;

    for _ in 0..80 {
        stars.push(Star {
            pos: Vector2::new(
                rl.get_random_value::<i32>(0..=(SCREEN_WIDTH)) as f32,
                rl.get_random_value::<i32>(0..=(SCREEN_HEIGHT)) as f32,
            ),
            speed: rl.get_random_value::<i32>(1..=3) as f32 * X_SCALE as f32,
        })           
    }

    while !rl.window_should_close() {
        bgm.update_stream();

        if rl.is_key_pressed(KeyboardKey::KEY_F11) {
            rl.toggle_fullscreen();
        }

        // スケール計算
        let a = rl.get_screen_width() as f32 / SCREEN_WIDTH as f32;
        let b = rl.get_screen_height() as f32 / SCREEN_HEIGHT as f32;
        let scale = a.min(b);//std::f32::min((), ());
        let dest_rec = Rectangle::new((rl.get_screen_width() as f32 - SCREEN_WIDTH as f32 * scale as f32) * 0.5, (rl.get_screen_height() as f32  - SCREEN_HEIGHT as f32 * scale as f32) * 0.5, SCREEN_WIDTH as f32 * scale as f32, SCREEN_HEIGHT as f32 * scale as f32);

        let delta = rl.get_frame_time();
        let now = rl.get_time();
        let rate = delta * COUNT1S;
        let gamepad: i32 = 0;

        for star in &mut stars {
            star.pos.x -= star.speed * rate;
            if star.pos.x < 0.0 {
                star.pos.x = SCREEN_WIDTH as f32;
            }
        }

        if game_over == 0 {
            // 通常更新
            game_time += delta;

            let mut axisx: f32 = 0.0;
            let mut axisy: f32 = 0.0;
            if rl.is_gamepad_available(gamepad) {
                axisx = rl.get_gamepad_axis_movement(gamepad, GamepadAxis::GAMEPAD_AXIS_LEFT_X);
                axisy = rl.get_gamepad_axis_movement(gamepad, GamepadAxis::GAMEPAD_AXIS_LEFT_Y);
            }

            // 自機移動
            if rl.is_key_down(KeyboardKey::KEY_UP) || rl.is_key_down(KeyboardKey::KEY_W) || (axisy < -0.2) || (rl.is_gamepad_available(gamepad) && rl.is_gamepad_button_down(gamepad, GamepadButton::GAMEPAD_BUTTON_LEFT_FACE_UP)) {
                player.pos.y -= PLAYER_SPEED * rate;
            }
            if rl.is_key_down(KeyboardKey::KEY_DOWN) || rl.is_key_down(KeyboardKey::KEY_S) || (axisy > 0.2) || (rl.is_gamepad_available(gamepad) && rl.is_gamepad_button_down(gamepad, GamepadButton::GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
                player.pos.y += PLAYER_SPEED * rate;
            }
            if rl.is_key_down(KeyboardKey::KEY_LEFT) || rl.is_key_down(KeyboardKey::KEY_A) || (axisx < -0.2) || (rl.is_gamepad_available(gamepad) && rl.is_gamepad_button_down(gamepad, GamepadButton::GAMEPAD_BUTTON_LEFT_FACE_LEFT)){
                player.pos.x -= PLAYER_SPEED * rate;
            }
            if rl.is_key_down(KeyboardKey::KEY_RIGHT) || rl.is_key_down(KeyboardKey::KEY_D) || (axisx > 0.2) || (rl.is_gamepad_available(gamepad) && rl.is_gamepad_button_down(gamepad, GamepadButton::GAMEPAD_BUTTON_LEFT_FACE_RIGHT)){
                player.pos.x += PLAYER_SPEED * rate;
            }

            player.pos.x = player.pos.x.clamp(0.0, (SCREEN_WIDTH / X_SCALE - 40) as f32);
            player.pos.y = player.pos.y.clamp(0.0, (SCREEN_HEIGHT / Y_SCALE - 32) as f32);

            // オプション更新
            for opt in &mut options {
                opt.pos.x += ((player.pos.x + 16.0) - opt.pos.x) / 4.0 * rate;
                opt.pos.y += ((player.pos.y + opt.offset_y) - opt.pos.y) / 4.0 * rate;    
            }

            // 自機射撃
            if (rl.is_key_down(KeyboardKey::KEY_SPACE) || rl.is_key_down(KeyboardKey::KEY_Z) || (rl.is_gamepad_available(gamepad) && rl.is_gamepad_button_down(gamepad, GamepadButton::GAMEPAD_BUTTON_RIGHT_FACE_DOWN)))
                && now - last_shot > shot_cooldown
            {
                bullets.push(Bullet {
                    pos: Vector2::new(player.pos.x + 32.0, player.pos.y + 12.0),
                    poswh: Vector2::new(16.0, 8.0),
                    poslt:  Vector2::new(0.0, 0.0),

                    active: true,
                });

                for j in &options {
                    bullets.push(Bullet {
                        pos: Vector2::new(j.pos.x + 8.0, j.pos.y + 12.0),
                        poswh: Vector2::new(16.0, 8.0),
                        poslt:  Vector2::new(0.0, 0.0),

                        active: true,
                    });
                }
                last_shot = now;
            }

            enemy_spawn_timer += delta;
            let base_interval = (50.0 - (score as f32 / 250.0)) / COUNT1S;                   // scoreが増えるほど短く
            let spawn_interval = base_interval.max(18.0/COUNT1S); // フレーム→秒に変換

            // 敵生成
//            if rl.get_random_value::<i32>(0..=40) == 0 {
            if enemy_spawn_timer >= spawn_interval {
                let rand_num = rl.get_random_value::<i32>(0..=100);
                let &mut etype;
                if rand_num < 60 {
                    etype = 0;
                } else if rand_num < 85 {
                    etype = 1;
                } else {
                    etype = 2;
                }
                let &mut ehp;
                if etype == 0 {
                    ehp = 1;
                } else {
                    ehp = 3;
                }

                enemies.push(Enemy {
                    pos: Vector2::new(
                        (SCREEN_WIDTH / X_SCALE) as f32,
                        rl.get_random_value::<i32>(32..=(SCREEN_HEIGHT / Y_SCALE - 64)) as f32),
                    poswh: Vector2::new(32.0, 32.0),
                    poslt: Vector2::new(0.0, 0.0),
                    lives: ehp,
                    etype: etype,
                    count: 0.0,
                    count2: (rl.get_random_value::<i32>(30*2..=30*2+SCREEN_HEIGHT/Y_SCALE-40*2) - 30 * 2) as f32,
                    shoot_timer: 0.0,
                    next_shoot_time: 5.0 / COUNT1S,

                    active: true,
                });
                enemy_spawn_timer = 0.0;
            }

            // ボム使用
            if (rl.is_key_pressed(KeyboardKey::KEY_X) || rl.is_key_pressed(KeyboardKey::KEY_B) || (rl.is_gamepad_available(gamepad) && rl.is_gamepad_button_pressed(gamepad, GamepadButton::GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))) && bomb_stock > 0 && !bomb_active {
                use_bomb(&mut particles, &rl, &explosion_sound, &mut score, &mut bomb_stock, &mut bomb_active, &mut enemies, &mut enemy_bullets, &mut player);
            }

            // 自弾移動
            for bullet in &mut bullets {
                if bullet.active {
                    bullet.pos.x += BULLET_SPEED * rate;
                    if bullet.pos.x > SCREEN_WIDTH as f32 + 20.0 {
                        bullet.active = false;
                    }
                }
            }

            let enemyspeed = ENEMY_SPEED * rate;

            // 敵機移動
            for e in &mut enemies {
                if e.active {

                    e.count += rate;

                    match e.etype {
                        0 => { // 通常敵
                            e.pos.x -= enemyspeed;
                        },

                        1 => { // ヘリザコ - 勢いよく突っ込む
                           //               static float dist_x = e.x - player_x
                            if e.count < 24.0 { // 1段階：超急接近
                                e.pos.x -= 6.0 * 2.0 * rate;
                                e.pos.y += ((player.pos.y + 8.0 - e.pos.y) / 8.0) / 2.0 * rate;
                            } else if e.count < 49.0 { // 2段階：短くホバリング
                                e.pos.x -= 0.0;
                            } else { // 3段階：右へ全力逃走
                                e.pos.x += 6.0 * 2.0 * rate;
                            }
                        },
                        2 => { // サインカーブ
                            e.pos.x -= enemyspeed;
                            let rad = e.count*0.12;
                            e.pos.y = e.count2 + rad.sin() * 55.0 * 2.0;
                        },
                        _=> {
                        },
                    }


                    // 敵弾発射処理
                    e.shoot_timer += delta;

                    let difficulty = ((game_time/180.0).min(1.0) as i32) as f32; // * COUNT1S)))
                    let enemy_bullet_speed = 4.0 + difficulty * 2.0;
                    let shoot_interval = ((82.0 - difficulty * 36.0) - 5.0) / COUNT1S;

                    if e.shoot_timer >= e.next_shoot_time {

                        let dx = player.pos.x - e.pos.x;
                        let dy = player.pos.y - e.pos.y;

                    //              dx -= 4.0f

                        let mut dist:f32;
                        if dx.abs() > dy.abs() {
                            dist = dx.abs();
                        } else {
                            dist = dy.abs();
                        }
                        if dist == 0.0 {
                            dist = 1.0;
                        }

                        // 弾を発射
                        let bullet_speed = enemy_bullet_speed;

                        let mut dx = dx * bullet_speed / dist;
                        let mut dy = dy * bullet_speed / dist;
                        dx = dx.max(-3.0*2.0);
                        dx = dx.min(4.0*2.0);
                        dy = dy.max(-4.0*2.0);
                        dy = dy.min(4.0*2.0);

//                        for bullet in &mut enemy_bullets {
//                      for j := range game.enemybullets {
//                          if bullet.active == false {

                                enemy_bullets.push( EnemyBullet{
                                    pos: Vector2::new(e.pos.x+16.0,
                                        e.pos.y+16.0),
                                    poswh: Vector2::new(8.0, 8.0),
                                    poslt: Vector2::new(0.0, 0.0),
                                    v:   Vector2::new(dx, // * bulletSpeed - 1.0f*1,   // vx
                                         dy,), // * bulletSpeed     // vy
                                    active: true,
                                });
//                              break;
//                          }
//                      }

                        // 次回の発射間隔を設定
                        e.next_shoot_time = shoot_interval;

                        e.shoot_timer = 0.0;
                    //              e.count += rate
                    //              e.count++
                    }


//                    enemy.pos.x -= ENEMY_SPEED * rate;
                    if e.pos.x < -32.0 {
                        e.active = false;
                    }
                }
            }

            // 敵弾 vs 自機
            for e in &mut enemy_bullets {
                if !e.active {
                    continue;
                }
                if check_collision(player.pos, player.poswh, player.poslt, e.pos, e.poswh, e.poslt) {
                    if shield_active {
                        shield_active = false;                                    // シールド消費
                        create_particles(&mut particles, &rl, player.pos.x, player.pos.y, 8);
                    } else {
                        lives -= 1;
                        if lives <= 0 {
                            game_over = 1;
                            bgm.stop_stream();
                        }
                    }
                    e.active = false;
                    break
                }
            }

            // 敵弾移動&画面範囲外判定
            for e in &mut enemy_bullets {
                if e.active {
                    e.pos.x += e.v.x * rate;
                    e.pos.y += e.v.y * rate;

                    if (e.pos.x < -32.0) || (e.pos.x > SCREEN_WIDTH as f32/X_SCALE as f32) || (e.pos.y < 32.0) || (e.pos.y > SCREEN_HEIGHT as f32/Y_SCALE as f32) {
                        e.active = false;
                    }
                }
            }

            enemy_bullets.retain(|it| it.active);

            // 当たり判定 自弾&敵
            for bullet in &mut bullets {
                if !bullet.active { continue; }
                for enemy in &mut enemies {
                    if !enemy.active { continue; }
                    if check_collision(bullet.pos, bullet.poswh, bullet.poslt, enemy.pos, enemy.poswh, enemy.poslt) {
                        bullet.active = false;
                        enemy.lives -= 1;
                        create_particles(&mut particles, &rl, enemy.pos.x, enemy.pos.y, 8);
                        if enemy.lives <= 0 {
                            // アイテム生成
                            if option_cooldown == 0 {
                                items.push( Item{
                                    pos: Vector2 { x: (enemy.pos.x), y: (enemy.pos.y) },
//                                    poswh: Vector2 { x: (28.0), y: (28.0) },
//                                    poslt: Vector2 { x: (0.0), y: (0.0) },
                                    timer: 300.0,
                                    types: 1,
                                    active: true,
                                });
                                option_cooldown = 10;
                            }else{
                                option_cooldown-=1;
                            }
                            if rl.get_random_value::<i32>(0..=100)  < 12 && !shield_active {
                                items.push( Item{
                                    pos: Vector2 { x: (enemy.pos.x), y: (enemy.pos.y) },
//                                    poswh: Vector2 { x: (28.0), y: (28.0) },
//                                    poslt: Vector2 { x: (0.0), y: (0.0) },
                                    timer: 280.0,
                                    types: 2,
                                    active: true,
                                });                                
                            }
                            if rl.get_random_value::<i32>(0..=100)  < 10 {
                                items.push( Item{
                                    pos: Vector2 { x: (enemy.pos.x), y: (enemy.pos.y) },
//                                    poswh: Vector2 { x: (28.0), y: (28.0) },
//                                    poslt: Vector2 { x: (0.0), y: (0.0) },
                                    timer: 270.0,
                                    types: 3,
                                    active: true,
                                });                                
                            }
                            if rl.get_random_value::<i32>(0..=100)  < 40 {
                                chain_items.push( ChainItem{
                                    pos: Vector2 { x: (enemy.pos.x), y: (enemy.pos.y) },
//                                    poswh: Vector2 { x: (28.0), y: (28.0) },
//                                    poslt: Vector2 { x: (0.0), y: (0.0) },
                                    timer: 240.0,
                                    active: true,
                                });                                
                            }

                            enemy.active = false;
                            score += 100;
                            explosion_sound.play();
                        }
                    }
                }
            }
            enemies.retain(|e| e.active);

            // 当たり判定 自機&敵
            for enemy in &mut enemies {
                if enemy.active && check_collision(player.pos, player.poswh, player.poslt, enemy.pos, enemy.poswh, enemy.poslt) {
                    enemy.active = false;
                    if shield_active {
                        shield_active = false;                                    // シールド消費
                        create_particles(&mut particles, &rl, player.pos.x, player.pos.y, 8);
                    } else {
                        lives -= 1;
                        if lives <= 0 {
                            game_over = 1;
                            bgm.stop_stream();
                        }
                    }
                }
            }

            bullets.retain(|b| b.active);
            enemies.retain(|e| e.active);

            // アイテム更新
            for it in &mut items {
                match it.types {
                    1 => {
                        it.pos.x -= 2.0 * rate;
                    },
                    2 => {
                        it.pos.x -= 4.0 * rate;
                    },
                    3 => {
                        it.pos.x -= 4.0 * rate;
                    },
                    _ => {
                    },
                }
                it.timer -= delta;       
                // 自機との当たり判定
                if (it.pos.x - player.pos.x.abs() < 44.0-16.0) && ((it.pos.y - player.pos.y).abs() < 44.0-16.0) {
                    match it.types {
                        1 => {
                            if options.len() < 2 {
                                let mut offset = -25;
                                if options.len() < 1  {
                                    offset = 25;
                                }
                                options.push( Option{
                                    pos: Vector2 {x:0.0, y:0.0},
                                    offset_y: offset as f32 * 2.0 as f32,
                                });
                            }
                        },
                        2 => {
                            shield_active = true;
                        },
                        3 => {
                            bomb_stock = (bomb_stock + 1).min(3);
                        },
                        _ => {
                        },
                    }        
                    laser_sound.play();
                    it.active = false;
                }
            }
            items.retain(|it| it.active);

            // ボムタイマー
            if bomb_active {
                bomb_timer -= delta;
                if bomb_timer <= 0.0 {
                    bomb_active = false;
                }
            }

            // チェインアイテム更新
            for it in &mut chain_items {
                it.pos.x -= 4.0 * rate;
                it.timer -= delta;
                if (it.pos.x - player.pos.x.abs() < 44.0-16.0) && ((it.pos.y - player.pos.y).abs() < 44.0-16.0) {
                    chain_count += 1;
                    chain_timer = 240.0 / COUNT1S;
                    score += chain_count * 100;
                    it.active = false;
                    laser_sound.play();
                    continue;
                }
                if (it.timer <= 0.0) || (it.pos.x < -20.0) {
                    chain_count = 0;
                    it.active = false;    
                }
            }
            chain_items.retain(|it| it.active);

            // チェインタイマー減少
            if chain_timer > 0.0{
                chain_timer -= delta;
                if chain_timer <= 0.0 {
                    chain_count = 0; 
                }
            }


            for particle in &mut particles {
                particle.pos.x += particle.vpos.x * rate;
                particle.pos.y += particle.vpos.y * rate;
                let dumping = f32::powf(0.96, rate);
                particle.vpos.x *= dumping;
                particle.vpos.y *= dumping;
                particle.life -= rate;
            }
            particles.retain(|particle| particle.life > 0.0);


            if game_over != 0 && score > high_score {
                high_score = score;
            }
        } else {
            // ゲームオーバー
            if game_over == 1 {
                if !(rl.is_key_down(KeyboardKey::KEY_R) || rl.is_key_down(KeyboardKey::KEY_Z) || rl.is_key_down(KeyboardKey::KEY_SPACE) || (rl.is_gamepad_available(gamepad) && rl.is_gamepad_button_down(gamepad, GamepadButton::GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) ||
                    rl.is_key_down(KeyboardKey::KEY_X) || rl.is_key_down(KeyboardKey::KEY_B)  || (rl.is_gamepad_available(gamepad) && rl.is_gamepad_button_down(gamepad, GamepadButton::GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))){
                    game_over = 2;
                }
            }
            if game_over == 2 {
                if rl.is_key_pressed(KeyboardKey::KEY_R) || rl.is_key_pressed(KeyboardKey::KEY_Z) || rl.is_key_pressed(KeyboardKey::KEY_SPACE) || (rl.is_gamepad_available(gamepad) && rl.is_gamepad_button_pressed(gamepad, GamepadButton::GAMEPAD_BUTTON_RIGHT_FACE_DOWN)){
                    lives = 1;
                    easy_mode = false;
                    game_over = 3;
                } else if rl.is_key_pressed(KeyboardKey::KEY_X) || rl.is_key_pressed(KeyboardKey::KEY_B)  || (rl.is_gamepad_available(gamepad) && rl.is_gamepad_button_pressed(gamepad, GamepadButton::GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)){
                    lives = 3;
                    easy_mode = true;
                    game_over = 3;
                }
            }
            if game_over == 3 {
                player.pos.x = 60.0;
                player.pos.y = 160.0;
                bullets.clear();
                enemies.clear();
                enemy_bullets.clear();
                options.clear();
                items.clear();
                chain_items.clear();
                particles.clear();
                score = 0;
                bomb_stock = 0;
                game_time = 0.0;
                chain_count = 0;
                bgm.play_stream();
                game_over = 0;
                shield_active = false;
                option_cooldown = 10;
                chain_timer = 0.0;
            }
        }

        // --- 描画開始 ---
        let mut d = rl.begin_texture_mode(&thread, &mut target);
        d.clear_background(Color::BLACK);

        for star in &mut stars {
            d.draw_circle(star.pos.x as i32, star.pos.y as i32, 1.5, Color::WHITE);
        }

        for particle in &mut particles {
            if particle.life <= 0.0 {
                continue;
            }
            d.draw_circle(particle.pos.x as i32 * X_SCALE, particle.pos.y as i32 * Y_SCALE, 1.5 * 2.0, Color::YELLOW);
        }

        for chainitem in &chain_items {
            put_sprite(&chr_tex, &mut d, chainitem.pos.x, chainitem.pos.y, 3);
        }

        for i in &items {
            let mut pat_no = 0;
            match i.types {
                1 => {
                    pat_no = 8;
                },
                2 => {
                    pat_no = 7;
                },
                3 => {
                    pat_no = 9;
                },
                _=> {
                },
            }
            put_sprite(&chr_tex, &mut d, i.pos.x, i.pos.y, pat_no);
        }

        for option in &options {
            put_sprite(&chr_tex, &mut d, option.pos.x, option.pos.y, 10);
        }

        for enemy_bullets in &enemy_bullets {
            if enemy_bullets.active {
                put_sprite(&chr_tex, &mut d, enemy_bullets.pos.x, enemy_bullets.pos.y, 0);
            }
        }

        for enemy in &enemies {
            if enemy.active {
                put_sprite(&chr_tex, &mut d, enemy.pos.x, enemy.pos.y, 2);
            }
        }

        for bullet in &bullets {
            if bullet.active {
                put_sprite(&chr_tex, &mut d, bullet.pos.x, bullet.pos.y, 4);
            }
        }

        if shield_active {
            put_sprite(&chr_tex, &mut d, player.pos.x, player.pos.y, 6);
        }

        put_sprite(&chr_tex, &mut d, player.pos.x, player.pos.y, 1);

        // UI
        if score >= high_score {
            put_strings_num(&font_tex, &mut d, 0, 0, "HIGH  ", score, 7);
        } else {
            put_strings_num(&font_tex, &mut d, 0, 0, "SCORE ", score, 7);
        }
        if easy_mode == true {
            put_strings_num(&font_tex, &mut d, 0, 2 * FONT_SIZE, "LIVES ", lives, 1);
        }
        put_strings_num(&font_tex, &mut d, 0, 1 * FONT_SIZE, "BOMB  ", bomb_stock, 1);
        put_strings_num(&font_tex, &mut d, 16 * FONT_SIZE, 0, "COUNT ", game_time as i32, 7);

        if chain_count > 0 {
            put_strings_num(&font_tex, &mut d, 16 * FONT_SIZE, 1 * FONT_SIZE, "CHAIN ", chain_count, 3);
        }

//        put_strings_num(&font_tex, &mut d, 0, 10 * FONT_SIZE, "CHAINTIMER ", chain_timer as i32, 3);

        if game_over != 0 {
            put_strings(&font_tex, &mut d, 11 * FONT_SIZE, 12 * FONT_SIZE, "GAME OVER");
            put_strings_num(&font_tex, &mut d, 7 * FONT_SIZE, 15 * FONT_SIZE, "HIGH SCORE ", high_score, 7);
            put_strings(&font_tex, &mut d, 7 * FONT_SIZE, 18 * FONT_SIZE, "PRESS A TO RESTART");
        }
        drop(d); // 描画終了 (EndTextureMode)

        let mut d = rl.begin_drawing(&thread);
        d.clear_background(Color::BLACK);

        let source_rec = Rectangle::new(0.0 as f32, 0.0 as f32, target.texture.width as f32, -target.texture.height as f32);
        let origin = (0.0, 0.0);

        d.draw_texture_pro(&target, source_rec, dest_rec, origin, 0.0, Color::WHITE);
        drop(d); // 画面更新
    }
}