#include <iostream>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <vector>
#include <algorithm>

using namespace emscripten;

int randomInRange(int min, int max) {
    return min + static_cast<int>(emscripten_random() * (max - min + 1));
}

class Star {
    public:
        int star_width;
        int star_height;
        int x;
        int y;
        int vel_x;
        int vel_y;
        int DISPLAY_WIDTH;
        Star() {
            this->DISPLAY_WIDTH = 400;

            this->star_width = randomInRange(1, 4);
            this->star_height = this->star_width;

            this->x = randomInRange(0, this->DISPLAY_WIDTH);
            this->y = 0;
            this->vel_x = 0;
            this->vel_y = randomInRange(5, 15);
        }
        void update() {
            this->x += this->vel_x;
            this->y += this->vel_y;
        }
};

class Bg {
    public:
        int time;
        std::vector<Star> group_star;
        int DISPLAY_HEIGHT;
        Bg() {
            this->time = randomInRange(1, 7);
            this->DISPLAY_HEIGHT = 600;
        }
        void update() {
            for(int i = 0; i < this->group_star.size(); i++) {
                this->group_star[i].update();
            }

            this->group_star.erase(std::remove_if(this->group_star.begin(), this->group_star.end(), [this](Star s) {
                return s.y > this->DISPLAY_HEIGHT;
            }), this->group_star.end());

            if(this->time == 0) {
                this->group_star.push_back(Star());
                this->time = randomInRange(1, 7);
            }

            this->time--;
        }
};

class Bullet {
    public:
        int bullet_width;
        int bullet_height;
        int x;
        int y;
        int vel_x;
        int vel_y;
        int speed;
        Bullet(int x, int y) {
            this->bullet_width = 3;
            this->bullet_height = 16;

            this->x = x;
            this->y = y;

            this->vel_x = 0;
            this->vel_y = 0;

            this->speed = 20;
        }
        void update() {
            this->x += this->vel_x;
            this->y += this->vel_y;
        }
};

class Enemy {
    public:
        int DISPLAY_WIDTH;
        int anim_index;
        int frame_length_max;
        int frame_length;
        bool is_destroyed;
        bool is_invincible;
        int x;
        int y;
        int vel_x;
        int vel_y;
        int speed;
        int hp;
        int score_value;
        int enemyElementWidth;
        int enemyElementHeight;

        Enemy(int enemyElementWidth, int enemyElementHeight) {
            this->DISPLAY_WIDTH = 400;

            this->anim_index = 0;
            this->frame_length_max = 8; 
            this->frame_length = this->frame_length_max;
            this->is_destroyed = false;
            this->is_invincible = false;

            this->vel_x = 0;
            this->speed = randomInRange(3, 8);
            this->vel_y = this->speed;

            this->hp = 3;
            this->score_value = 5;

            this->enemyElementWidth = enemyElementWidth;
            this->enemyElementHeight = enemyElementHeight;
            this->x = randomInRange(0, this->DISPLAY_WIDTH - this->enemyElementWidth);
            this->y = -this->enemyElementHeight;
        }

        void update() {
            this->x += this->vel_x;
            this->y += this->vel_y;

            if(is_destroyed) {
                int max_index = 5;          // anim_explosion.length - 1
                if(this->frame_length == 0) {
                    this->anim_index++;
                    this->frame_length = this->frame_length_max;
                } else {
                    this->frame_length--;
                }
            }
        }

        void get_hit() {
            if(!this->is_invincible) {
                this->hp--;
                if(this->hp <= 0) {
                    this->is_destroyed = true;
                    this->is_invincible = true;
                    this->vel_x = 0;
                    this->vel_y = 0;
                    this->x -= 20;
                    this->y -= 20;
                }
            }
        }
};

class Enemyspawners {
    public:
        std::vector<Enemy> enemy_group;
        int spawn_timer;
        bool stop;
        int DISPLAY_HEIGHT;
        int enemyElementWidth;
        int enemyElementHeight;

        Enemyspawners(int enemyElementWidth, int enemyElementHeight) {
            this->enemyElementWidth = enemyElementWidth;
            this->enemyElementHeight = enemyElementHeight;

            this->spawn_timer = randomInRange(40, 60);
            this->stop = false;
            this->DISPLAY_HEIGHT = 600;
        }

        void update() {
            if(!this->stop) {
                for(int i = 0; i < this->enemy_group.size(); i++) {
                    this->enemy_group[i].update();
                }

                this->enemy_group.erase(std::remove_if(this->enemy_group.begin(), this->enemy_group.end(), [&](Enemy &enemy) {
                    return enemy.y > this->DISPLAY_HEIGHT;
                }), this->enemy_group.end());

                this->enemy_group.erase(std::remove_if(this->enemy_group.begin(), this->enemy_group.end(), [&](Enemy &enemy) {
                    return enemy.anim_index > 5;
                }), this->enemy_group.end());

                if(this->spawn_timer == 0) {
                    this->enemy_group.push_back(Enemy(this->enemyElementWidth, this->enemyElementHeight));
                    this->spawn_timer = randomInRange(40, 60);
                }

                this->spawn_timer--;
            } else {
                this->enemy_group.clear();
            }
        }

        void clear_enemies() {
            this->enemy_group.clear();
        }
        
        void stop_spawning() {
            this->stop = true;
        }
};

class HealthBar {
    public:
        int max_hp;
        int hp;
        int vel_x;
        int vel_y;
        int x;
        int y;
        int DISPLAY_HEIGHT;
        int healthBarElementHeight;

        HealthBar(int hp, int healthBarElementHeight) {
            this->DISPLAY_HEIGHT = 600;
            this->healthBarElementHeight = healthBarElementHeight;

            this->max_hp = hp;
            this->hp = this->max_hp;

            this->vel_x = 0;
            this->vel_y = 0;

            this->x = 10;
            this->y = this->DISPLAY_HEIGHT - this->healthBarElementHeight - 25;
        }

        void update() {
            this->x += this->vel_x;
            this->y += this->vel_y;
        }

        void decrease_hp_value() {
            this->hp--;
        };

        void reset_health_to_max() {
            this->hp = this->max_hp;
        };
};

class Lives {
    public:
        int num_lives;
        int width;
        int height;
        int x;
        int y;
        int DISPLAY_HEIGHT;
        int vel_x;
        int vel_y;

        Lives(int num_lives) {
            this->num_lives = num_lives;

            this->DISPLAY_HEIGHT = 600;

            this->width = 80;
            this->height = 40;

            this->x = 200;
            this->y = this->DISPLAY_HEIGHT-40;

            this->vel_x = 0;
            this->vel_y = 0;
        }

        void update() {

        }

        void decrement_life() {
            this->num_lives--;
            if(this->num_lives < 0) {
                this->num_lives = 0;
            }
        }
};

class Score {
    public:
        int DISPLAY_WIDTH;
        int DISPLAY_HEIGHT;
        int value;
        int font_size;
        int x_pad;
        int y_pad;
        int x;
        int y;

        Score() {
            this->DISPLAY_WIDTH = 400;
            this->DISPLAY_HEIGHT = 600;

            this->value = 0;
            this->font_size = 20;

            this->x_pad = 20;
            this->y_pad = 17;

            this->x = this->DISPLAY_WIDTH - 100;
            this->y = this->DISPLAY_HEIGHT - 15;
        }

        void update() {
            
        }

        void update_score(int value) {
            this->value += value;
        }
};

class HUD {
    public:
        int DISPLAY_HEIGHT;
        int x;
        int y;
        int hudElementHeight;
        int vel_x;
        int vel_y;

        HealthBar health_bar;
        Score score;
        Lives lives;

        HUD(int hp, int num_lives, int hudElementHeight, int healthBarElementHeight)
            : DISPLAY_HEIGHT(600),
            hudElementHeight(hudElementHeight),
            x(0),
            y(DISPLAY_HEIGHT - hudElementHeight),
            vel_x(0),
            vel_y(0),
            health_bar(hp, healthBarElementHeight),
            lives(num_lives),
            score()
        { }

        void update() {
            this->health_bar.update();
            this->score.update();
            this->lives.update();
            this->x += this->vel_x;
            this->y += this->vel_y;
        }
};

class Particle {
    public:
        int particle_width;
        int particle_height;
        int vel_x;
        int vel_y;
        int kill_timer;
        int x;
        int y;
        int DISPLAY_WIDTH;
        int DISPLAY_HEIGHT;

        Particle(int x, int y) {
            this->DISPLAY_WIDTH = 400;
            this->DISPLAY_HEIGHT = 600;

            this->particle_width = randomInRange(1, 6);
            this->particle_height = this->particle_width;

            this->vel_x = randomInRange(-16, 16);
            this->vel_y = randomInRange(-16, 16);

            this->kill_timer = 45;

            this->x = x;
            this->y = y;
        }

        void update() {
            this->x += this->vel_x;
            this->y += this->vel_y;

            if(this->x < 0 || this->x > this->DISPLAY_WIDTH || this->y < 0 || this->y > this->DISPLAY_HEIGHT) {
                this->kill_timer = 0;
            }

            this->kill_timer--;
        }
};

class Particlespawners {
    public:
        std::vector<Particle> particle_group;

        Particlespawners() {
        }

        void update() {
            for(int i = 0; i < this->particle_group.size(); i++) {
                this->particle_group[i].update();
            }

            for(int i = 0; i < this->particle_group.size(); i++) {
                if(this->particle_group[i].kill_timer <= 0) {
                    this->particle_group.erase(this->particle_group.begin() + i);
                }
            }
        }

        void spawn_particles(int x, int y) {
            int random_number = randomInRange(3, 20);
            for(int i = 0; i < random_number; i++) {
                this->particle_group.push_back(Particle(x, y));
            }
        }
};

class Ship {
    public:
        int x;
        int y;
        int DISPLAY_WIDTH;
        int DISPLAY_HEIGHT;
        int shipElementWidth;
        int shipElementHeight;
        int max_hp;
        int hp;
        int lives;
        bool is_alive;

        HUD hud;

        bool is_invincible;
        int max_invincible_timer;
        int invincible_timer;

        int vel_x;
        int vel_y;
        int speed;

        std::vector<Bullet> bullet_group;

		Ship(int shipElementWidth, int shipElementHeight, int hudElementHeight, int healthBarElementHeight)
			: DISPLAY_WIDTH(400), DISPLAY_HEIGHT(600), shipElementWidth(shipElementWidth), shipElementHeight(shipElementHeight),
			max_hp(3), hp(max_hp), lives(3), is_alive(true), 
			hud(hp, lives, hudElementHeight, healthBarElementHeight),
			is_invincible(false), max_invincible_timer(60), invincible_timer(max_invincible_timer),
			vel_x(0), vel_y(0), speed(5)
		{
			this->x = this->DISPLAY_WIDTH / 2 - 40;
			this->y = this->DISPLAY_HEIGHT - this->shipElementHeight * 1.5;
		}

        void update() {
            for(int i = 0; i < this->bullet_group.size(); i++) {
                this->bullet_group[i].update();
            }

            this->bullet_group.erase(std::remove_if(this->bullet_group.begin(), this->bullet_group.end(), [](Bullet bullet) {
                return bullet.y < 0;
            }), this->bullet_group.end());

            if(this->x > this->DISPLAY_WIDTH - this->shipElementWidth + 20) {
                this->x = this->DISPLAY_WIDTH - this->shipElementWidth + 20;
            }

            if(this->x < -20) {
                this->x = -20;
            }

            this->x += this->vel_x;
            this->y += this->vel_y;

            if(this->invincible_timer > 0) {
                this->invincible_timer--;
            } else {
                this->is_invincible = false;
            }
        }

        void shoot() {
            if(this->is_alive) {
                Bullet new_bullet = Bullet(this->x + 32, this->y);
                new_bullet.vel_y = -new_bullet.speed;
                this->bullet_group.push_back(new_bullet);
            }
        }

        void get_hit() {
            if(this->is_alive) {
                this->hp--;
                this->hud.health_bar.decrease_hp_value();
                if(this->hp <= 0) {
                    this->hp = 0;
                    this->death();
                }
            }
        }

        void death() {
            this->lives--;
            if(this->lives <= 0) {
                this->lives = 0;
                this->is_alive = false;
            }
            this->hp = this->max_hp;
            this->hud.health_bar.reset_health_to_max();
            this->hud.lives.decrement_life();
            this->x = this->DISPLAY_WIDTH / 2 - 40;
            this->is_invincible = true;
            this->invincible_timer = this->max_invincible_timer;
        }

        void left() {
            this->vel_x = -this->speed;
        }

        void right() {
            this->vel_x = this->speed;
        }

        void zero_x() {
            this->vel_x = 0;
        }
};

EMSCRIPTEN_BINDINGS(my_module) {

    register_vector<Star>("VectorStar");
    register_vector<Enemy>("VectorEnemy");
    register_vector<Bullet>("VectorBullet");
    register_vector<Particle>("VectorParticle");

    class_<Star>("Star")
        .constructor<>()
        .property("star_width", &Star::star_width)
        .property("star_height", &Star::star_height)
        .property("x", &Star::x)
        .property("y", &Star::y)
        .property("vel_x", &Star::vel_x)
        .property("vel_y", &Star::vel_y)
        .property("DISPLAY_WIDTH", &Star::DISPLAY_WIDTH)
        .function("update", &Star::update);

    class_<Bg>("Bg")
        .constructor<>()
        .property("time", &Bg::time)
        .property("group_star", &Bg::group_star)
        .property("DISPLAY_HEIGHT", &Bg::DISPLAY_HEIGHT)
        .function("update", &Bg::update);

    class_<Bullet>("Bullet")
        .constructor<int, int>()
        .property("bullet_width", &Bullet::bullet_width)
        .property("bullet_height", &Bullet::bullet_height)
        .property("x", &Bullet::x)
        .property("y", &Bullet::y)
        .property("vel_x", &Bullet::vel_x)
        .property("vel_y", &Bullet::vel_y)
        .property("speed", &Bullet::speed)
        .function("update", &Bullet::update);
    
    class_<Enemy>("Enemy")
        .constructor<int, int>()
        .property("DISPLAY_WIDTH", &Enemy::DISPLAY_WIDTH)
        .property("anim_index", &Enemy::anim_index)
        .property("frame_length_max", &Enemy::frame_length_max)
        .property("frame_length", &Enemy::frame_length)
        .property("is_destroyed", &Enemy::is_destroyed)
        .property("is_invincible", &Enemy::is_invincible)
        .property("x", &Enemy::x)
        .property("y", &Enemy::y)
        .property("vel_x", &Enemy::vel_x)
        .property("vel_y", &Enemy::vel_y)
        .property("speed", &Enemy::speed)
        .property("hp", &Enemy::hp)
        .property("score_value", &Enemy::score_value)
        .property("enemyElementWidth", &Enemy::enemyElementWidth)
        .property("enemyElementHeight", &Enemy::enemyElementHeight)
        .function("update", &Enemy::update)
        .function("get_hit", &Enemy::get_hit);
    
    class_<Enemyspawners>("Enemyspawners")
        .constructor<int, int>()
        .property("enemy_group", &Enemyspawners::enemy_group)
        .property("spawn_timer", &Enemyspawners::spawn_timer)
        .property("stop", &Enemyspawners::stop)
        .property("DISPLAY_HEIGHT", &Enemyspawners::DISPLAY_HEIGHT)
        .property("enemyElementWidth", &Enemyspawners::enemyElementWidth)
        .property("enemyElementHeight", &Enemyspawners::enemyElementHeight)
        .function("update", &Enemyspawners::update)
        .function("clear_enemies", &Enemyspawners::clear_enemies)
        .function("stop_spawning", &Enemyspawners::stop_spawning);

    class_<HealthBar>("HealthBar")
        .constructor<int, int>()
        .property("max_hp", &HealthBar::max_hp)
        .property("hp", &HealthBar::hp)
        .property("vel_x", &HealthBar::vel_x)
        .property("vel_y", &HealthBar::vel_y)
        .property("x", &HealthBar::x)
        .property("y", &HealthBar::y)
        .property("DISPLAY_HEIGHT", &HealthBar::DISPLAY_HEIGHT)
        .property("healthBarElementHeight", &HealthBar::healthBarElementHeight)
        .function("update", &HealthBar::update)
        .function("decrease_hp_value", &HealthBar::decrease_hp_value)
        .function("reset_health_to_max", &HealthBar::reset_health_to_max);

    class_<Lives>("Lives")
        .constructor<int>()
        .property("num_lives", &Lives::num_lives)
        .property("width", &Lives::width)
        .property("height", &Lives::height)
        .property("x", &Lives::x)
        .property("y", &Lives::y)
        .property("DISPLAY_HEIGHT", &Lives::DISPLAY_HEIGHT)
        .property("vel_x", &Lives::vel_x)
        .property("vel_y", &Lives::vel_y)
        .function("update", &Lives::update)
        .function("decrement_life", &Lives::decrement_life);

    class_<Score>("Score")
        .constructor<>()
        .property("DISPLAY_WIDTH", &Score::DISPLAY_WIDTH)
        .property("DISPLAY_HEIGHT", &Score::DISPLAY_HEIGHT)
        .property("value", &Score::value)
        .property("font_size", &Score::font_size)
        .property("x_pad", &Score::x_pad)
        .property("y_pad", &Score::y_pad)
        .property("x", &Score::x)
        .property("y", &Score::y)
        .function("update", &Score::update)
        .function("update_score", &Score::update_score);

    class_<HUD>("HUD")
        .constructor<int, int, int, int>()
        .property("x", &HUD::x)
        .property("y", &HUD::y)
        .property("hudElementHeight", &HUD::hudElementHeight)
        .property("DISPLAY_HEIGHT", &HUD::DISPLAY_HEIGHT)
        .property("vel_x", &HUD::vel_x)
        .property("vel_y", &HUD::vel_y)
        .property("health_bar", &HUD::health_bar)
        .property("score", &HUD::score)
        .property("lives", &HUD::lives)
        .function("update", &HUD::update);

    class_<Particle>("Particle")
        .constructor<int, int>()
        .property("particle_width", &Particle::particle_width)
        .property("particle_height", &Particle::particle_height)
        .property("vel_x", &Particle::vel_x)
        .property("vel_y", &Particle::vel_y)
        .property("kill_timer", &Particle::kill_timer)
        .property("x", &Particle::x)
        .property("y", &Particle::y)
        .property("DISPLAY_WIDTH", &Particle::DISPLAY_WIDTH)
        .property("DISPLAY_HEIGHT", &Particle::DISPLAY_HEIGHT)
        .function("update", &Particle::update);

    class_<Particlespawners>("Particlespawners")
        .constructor<>()
        .property("particle_group", &Particlespawners::particle_group)
        .function("update", &Particlespawners::update)
        .function("spawn_particles", &Particlespawners::spawn_particles);

    class_<Ship>("Ship")
        .constructor<int, int, int, int>()
        .property("x", &Ship::x)
        .property("y", &Ship::y)
        .property("DISPLAY_WIDTH", &Ship::DISPLAY_WIDTH)
        .property("DISPLAY_HEIGHT", &Ship::DISPLAY_HEIGHT)
        .property("shipElementWidth", &Ship::shipElementWidth)
        .property("shipElementHeight", &Ship::shipElementHeight)
        .property("max_hp", &Ship::max_hp)
        .property("hp", &Ship::hp)
        .property("lives", &Ship::lives)
        .property("is_alive", &Ship::is_alive)
        .property("hud", &Ship::hud)
        .property("is_invincible", &Ship::is_invincible)
        .property("max_invincible_timer", &Ship::max_invincible_timer)
        .property("invincible_timer", &Ship::invincible_timer)
        .property("vel_x", &Ship::vel_x)
        .property("vel_y", &Ship::vel_y)
        .property("speed", &Ship::speed)
        .property("bullet_group", &Ship::bullet_group)
        .function("update", &Ship::update)
        .function("shoot", &Ship::shoot)
        .function("get_hit", &Ship::get_hit)
        .function("death", &Ship::death)
        .function("left", &Ship::left)
        .function("right", &Ship::right)
        .function("zero_x", &Ship::zero_x);
}

extern "C" {
    
    EMSCRIPTEN_KEEPALIVE
    bool isColliding(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2) {
        return !(x1 + width1 < x2 ||      // Bullet is to the left of the enemy
                x1 > x2 + width2 ||      // Bullet is to the right of the enemy
                y1 + height1 < y2 ||     // Bullet is above the enemy
                y1 > y2 + height2);      // Bullet is below the enemy
    }

    EMSCRIPTEN_KEEPALIVE
    bool checkBulletEnemyCollisions(Ship* player, Enemyspawners* enemySpawner, Particlespawners* particleSpawner) {
        for (int i = 0; i < (*player).bullet_group.size(); i++) {
            Bullet bullet = (*player).bullet_group[i];

            for (int j = 0; j < (*enemySpawner).enemy_group.size(); j++) {
                Enemy& enemy = (*enemySpawner).enemy_group[j];

                if (!enemy.is_invincible && 
                    isColliding(bullet.x, bullet.y, bullet.bullet_width, bullet.bullet_height, 
                                enemy.x, enemy.y, enemy.enemyElementWidth, enemy.enemyElementHeight)) {
                    
                    enemy.get_hit();
                    (*player).hud.score.update_score(enemy.score_value);
                    (*particleSpawner).spawn_particles(bullet.x, bullet.y);

                    // Remove the bullet from the list
                    (*player).bullet_group.erase((*player).bullet_group.begin() + i);
                    i--;  // Adjust index after removal
                    return true;  // Break out of enemy loop once a bullet hits
                }
            }
        }
        return false;
    }

    EMSCRIPTEN_KEEPALIVE
    bool checkShipEnemyCollisions(Ship* player, Enemyspawners* enemySpawner) {
        for (int j = 0; j < (*enemySpawner).enemy_group.size(); j++) {
            Enemy& enemy = (*enemySpawner).enemy_group[j];

            if (!enemy.is_invincible && !(*player).is_invincible && 
                isColliding((*player).x, (*player).y, (*player).shipElementWidth, (*player).shipElementHeight, 
                            enemy.x, enemy.y, enemy.enemyElementWidth, enemy.enemyElementHeight)) {
                (*player).get_hit();
                enemy.hp = 0;  // Destroy the enemy
                enemy.get_hit();  // Apply explosion or destruction logic
                return true;  // Break once the collision is handled
            }
        }
        return false;
    }

}
