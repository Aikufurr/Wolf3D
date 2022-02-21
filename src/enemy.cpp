#include "../include/enemy.hpp"

#include "../include/player.hpp"

Enemy::Enemy(int id, std::string type, double position_x, double position_y, double direction, Player *player, std::vector<std::vector<int>> *world_map) {
    this->id = id;
    this->type = type;
    this->position_x = position_x;
    this->position_y = position_y;
    this->staring_x = position_x;
    this->staring_y = position_y;
    while (direction >= 360) {
        direction -= 360;
    }
    while (direction < 0) {
        direction += 360;
    }
    this->direction = direction;
    this->starting_direction = direction;

    this->sound_enemy_pain = new Sample("res/sounds/Enemy Pain.wav", MIX_MAX_VOLUME);
    this->sound_enemy_die.push_back(new Sample("res/sounds/Death 1.wav", MIX_MAX_VOLUME));
    this->sound_enemy_die.push_back(new Sample("res/sounds/Death 2.wav", MIX_MAX_VOLUME));
    this->sound_enemy_die.push_back(new Sample("res/sounds/Mein Leben.wav", MIX_MAX_VOLUME));

    this->sound_enemy_gun[this->type] = new Sample("res/sounds/Machine Gun.wav", MIX_MAX_VOLUME);

    this->player = player;
    this->last_player_x = *player->get_position_x();
    this->last_player_y = *player->get_position_y();

    this->generator = new AStar::Generator();
    this->generator->setWorldSize({int(world_map->size()), int(world_map->size())});
    this->generator->setHeuristic(AStar::Heuristic::euclidean);
    this->generator->setDiagonalMovement(true);

    for (int x = 0; x < int(world_map->size()); x++) {
        for (int y = 0; y < int((*world_map)[x].size()); y++) {
            if ((*world_map)[x][y] != 0) {
                this->generator->addCollision({x, y});
            }
        }
    }
}

void Enemy::reset(json *sprite) {
    this->position_x = this->staring_x;
    this->position_y = this->staring_y;
    sprite->at(this->id)["x"] = this->staring_x;
    sprite->at(this->id)["y"] = this->staring_y;
    this->direction = this->starting_direction;
    this->rotation = this->starting_rotation;
    this->health = 100;
    this->tracking_player = false;
    this->hunting_player = false;
    this->alive = true;
    this->state = this->state::IDLE;
    this->previous_state = 0;
    this->damaged_player = false;
    this->hunt_offset = 0;
    this->walk_frame = 1;
    this->animation_frame = 0;
    this->is_shooting = 0;

    this->previous_time = 0.0;
    this->time = 0.0;
    this->timer = 0.0;
    this->shoot_timer = 0.0;
    this->hunt_timer = 0.0;
    this->walk_timer = 0.0;
}

double Enemy::calculate_rotation(double position_x, double position_y, bool return_angle) {
    double angle = std::atan2(position_x - this->position_x, position_y - this->position_y) * (180 / M_PI);

    // Enemy rotation
    angle += (-this->direction) + 90;

    while (angle < 0.0) {
        angle += 360;
    }
    while (angle >= 360.0) {
        angle -= 360;
    }
    if (angle > 320) {
        angle = 0;
    }

    // std::cout << enemy_position_x << " " << enemy_position_y << " " <<
    // enemy_direction_x << " " << enemy_direction_y  << " " << side_a  << " "
    // << side_b  << " " << side_c  << std::endl;

    /*
                1-4:
                1 = Frame 1
                2 = Frame 2
                3 = Frame 3
                4 = Frame 4

                1-8:
                1 = o\/
                2 = /o
                3 = -o
                4 = \o
                5 = o^
                6 = o/
                7 = o-
                8 = o\
                */

    double rotation = (range(angle, 360, 0, 1, 8));
    if (rotation >= 4) {
        rotation += 1;
        if (rotation > 5 && rotation < 6) {
            rotation = 5;
        } else if (std::round(rotation) == 9) {
            rotation = 1;
        }
    }

    return return_angle ? angle : rotation;
}

void Enemy::ai(double *position_x, double *position_y, std::vector<std::vector<int>> *world_map, json *sprite) {
    if (this->health < 0 || this->alive == false) {
        return;
    }
    std::pair<bool, double> has_LOS = this->line_of_sight(position_x, position_y, world_map);
    if (this->hunting_player == true) {
        this->hunt_timer += (this->time - this->previous_time) / 1000.0;
        if (int(this->hunt_path.size()) > 0) {
            if ((*this->player->get_position_x()) != this->last_player_x &&
                (*this->player->get_position_y()) != this->last_player_y) {
                // std::cout << "Generate path inline ... \n"
                //           << std::endl;
                this->hunt_path = this->generator->findPath({int(this->position_x), int(this->position_y)}, {int(this->last_player_x), int(this->last_player_y)});
                this->hunt_offset = int(this->hunt_path.size()) - 2;
            }
            if (this->hunt_timer > 0.5) {
                this->hunt_offset--;
                this->hunt_timer = 0;
            }

            if (this->hunt_offset < 0) {
                this->hunt_path.clear();
                this->hunting_player = false;
                this->tracking_player = true;
            } else {
                // double angle = std::atan2(this->hunt_path.at(this->hunt_offset).x + 0.5 - this->position_x, this->hunt_path.at(this->hunt_offset).y + 0.5 - this->position_y) * (180 / M_PI) + 180;
                // angle == 360 ? angle = 0 : angle = angle;
                // angle += 180 - 90;
                // while (angle >= 360) {
                //     angle -= 360;
                // }

                this->walk_timer += (this->time - this->previous_time) / 1000.0;
                if (this->walk_timer > 0.5) {
                    this->walk_timer = 0;

                    int angle = 0;
                    int hunt_x = this->hunt_path.at(this->hunt_offset).x;
                    int hunt_y = this->hunt_path.at(this->hunt_offset).y;
                    int this_x = std::floor(this->position_x);
                    int this_y = std::floor(this->position_y);
                    // std::cout << "--------------------- " << hunt_x << " " << hunt_y << " @ " << this_x << " " << this_y << std::endl;
                    // std::cout << "===================== " << (hunt_x < this_x) << " " << (hunt_x > this_x) << " " << (hunt_y < this_y) << " " << (hunt_y > this->position_y) << std::endl;

                    if (hunt_x < this_x) {
                        // North -x 0
                        angle = 0;
                    } else if (hunt_x > this_x) {
                        // South +x 180
                        angle = 180;
                    } else if (hunt_y < this_y) {
                        // West -y 270
                        angle = 270;
                    } else if (hunt_y > this_y) {
                        // East +y 90
                        angle = 90;
                    } else if (hunt_x < this_x && hunt_y > this_y) {
                        // North West +x +y 135
                        angle = 135;
                    } else if (hunt_x > this_x && hunt_y > this_y) {
                        // South West +x +y 135
                        angle = 135;
                    } else if (hunt_x < this_x && hunt_y > this_y) {
                        // North East -x +y 45
                        angle = 315;
                    } else if (hunt_x > this_x && hunt_y > this_y) {
                        // South East +x +y 135
                        angle = 225;
                    } else {
                        // Fallback to calculation
                        angle = this->calculate_rotation(*this->player->get_position_x(), *this->player->get_position_y(), true);
                    }
                    this->direction = angle;
                    this->rotation = this->calculate_rotation(*this->player->get_position_x(), *this->player->get_position_y());

                    // std::cout << "Walk Dir:" << this->direction << " Rot: " << this->rotation << std::endl;

                    this->position_x = this->hunt_path.at(this->hunt_offset).x + 0.5;
                    this->position_y = this->hunt_path.at(this->hunt_offset).y + 0.5;
                    sprite->at(this->id)["x"] = this->hunt_path.at(this->hunt_offset).x + 0.5;
                    sprite->at(this->id)["y"] = this->hunt_path.at(this->hunt_offset).y + 0.5;

                    this->walk_frame++;
                    if (this->walk_frame == 5) {
                        this->walk_frame = 1;
                    }
                    this->set_state(this->state::WALK);
                }
            }
        } else {
            // std::cout << "Generate path ... \n"
            //           << std::endl;
            this->hunt_path = this->generator->findPath({int(this->position_x), int(this->position_y)}, {int(this->last_player_x), int(this->last_player_y)});
            this->hunt_offset = int(this->hunt_path.size()) - 1;
        }
    }
    if (has_LOS.first) {
        // If has line of sight
        /*

            if random number threashold

                rotation:
                0 = facing player
                1 = facing left from player
                2 = facing away from player
                3 = facing right from player
                if enemy rotation = 0
                    enemy shoot
                else if enemy rotation = 1
                    enemy translate 0
                else if enemy rotation = 2
                    enemy translate random of 1 or 3
                else if enemy rotaion = 3
                    enemy translate 0

            endif

            Enemy gets alerted if:
        X   - The player is in front of enemy (distance?)
        |   - If the enemy is facing away from the player and either:
        X   -- The player shoots the enemy
        X   -- The player shoots withing a certian distance to the enemy
        // -   -- The player moves within a certain distance to the enemy (closer than shoot distance ^)
        -   -- (Stretch goal) If alerted enemy runs past unalerted enemy

        X   - On out of range OR out of LOS, move to last seen location

        */
        double angle = this->calculate_rotation(*player->get_position_x(), *player->get_position_y(), true);
        // If in enemy's Field of View
        if (angle >= 360 - this->fov[this->type] || angle <= this->fov[this->type] || this->tracking_player == true) {
            // Set enemy's last looking direction
            this->direction = angle;
            // Set player's last seen location
            this->last_player_x = *player->get_position_x();
            this->last_player_y = *player->get_position_y();
            if (has_LOS.second > 6) {
                this->is_shooting = false;
                this->hunting_player = true;
                this->animation_frame = 0;
            } else
                // If the enemy is tracking the player or if a rand
                if (((rand() <= RAND_MAX * 0.05) || this->tracking_player == true)) {
                this->shoot_timer += (this->time - this->previous_time) / 1000.0;
                this->tracking_player = true;
                if (this->is_shooting == false && shoot_timer > 0.5) {
                    this->hunting_player = false;
                    shoot_timer = 0;
                    // std::cout << "Enemy: Take this! *fucking shoots*" << std::endl;
                    this->is_shooting = true;
                    this->animation_frame = 0;
                    this->sound_enemy_gun[this->type]->set_volume(MIX_MAX_VOLUME - (has_LOS.second * 15) > 0 ? MIX_MAX_VOLUME - (has_LOS.second * 15) : 0);
                    this->set_state(this->state::SHOOT);
                    // this->play_sound(this->state::SHOOT, has_LOS.second);
                }
            }
        }
    }
}

void Enemy::handle_shot(double *position_x, double *position_y) {
    // Use Euclidian distance
    double distance = std::sqrt(std::pow(*position_x - this->position_x, 2) + std::pow(*position_y - this->position_y, 2));
    // std::cout << "distance " << distance << std::endl;
    if (distance <= 6) {
        this->tracking_player = true;
    }
}

double angleBetween(std::pair<int, int> a, std::pair<int, int> b) {
    std::pair<int, int> p(-b.second, b.first);
    float b_coord = a.first * a.second + b.first * b.second;
    float p_coord = a.first * a.second + p.first * p.second;
    return std::atan2(p_coord, b_coord);
}

std::pair<bool, double> Enemy::line_of_sight(double *position_x, double *position_y, std::vector<std::vector<int>> *world_map) {
    double angle = std::atan2(*position_x - this->position_x, *position_y - this->position_y) * (180 / M_PI) + 180;
    angle == 360 ? angle = 0 : angle = angle;

    double ray_direction_x = std::sin(angle * (M_PI / 180));  // Vector of the ray on the camera plane
    double ray_direction_y = std::cos(angle * (M_PI / 180));

    // DDA Algorithm

    // The ray's grid location on the map
    int map_x = int(*position_x);
    int map_y = int(*position_y);

    // The length the ray has to travel to get to the next x-side or y-side
    // https://lodev.org/cgtutor/images/raycastdelta.gif
    double side_distance_x, side_distance_y;

    // The length the ray has to travel to get to the 1 x-side or y-side
    double delta_distance_x = std::abs(1 / ray_direction_x);
    double delta_distance_y = std::abs(1 / ray_direction_y);
    // Length of the ray to the wall
    double perpendicular_wall_distance;

    // What direction to step in
    int step_x, step_y;
    // Was it a NS or a WE wall hit?
    int side;

    // Calculate the step and side_distance
    if (ray_direction_x < 0) {
        // If the ray is left side
        step_x = -1;
        side_distance_x = ((*position_x) - map_x) * delta_distance_x;
    } else {
        // If the ray is right side
        step_x = 1;
        side_distance_x = (map_x + 1.0 - (*position_x)) * delta_distance_x;
    }
    if (ray_direction_y < 0) {
        // If the ray is down side
        step_y = -1;
        side_distance_y = ((*position_y) - map_y) * delta_distance_y;
    } else {
        // If the ray is up side
        step_y = 1;
        side_distance_y = (map_y + 1.0 - (*position_y)) * delta_distance_y;
    }

    int hit_index = -1;
    int hit_wall = -1;

    // Perform the DDA
    // While the ray has not hit a wall
    while (hit_index == -1 && hit_wall == -1 &&
           map_x < max_view_distance + this->position_x && map_x > -max_view_distance + this->position_x &&
           map_y < max_view_distance + this->position_y && map_y > -max_view_distance + this->position_y) {
        if (side_distance_x < side_distance_y) {
            // Jump one square along the x
            side_distance_x += delta_distance_x;
            map_x += step_x;
            side = 0;
        } else {
            // Jump one square along the y
            side_distance_y += delta_distance_y;
            map_y += step_y;
            side = 1;
        }

        // If the ray has collided with a wall, 0 being walkable space, anything
        // higher being a wall std::cout << (abs(double((indexed_sprite["x"]) -
        // map_x) <= 1) << ", " << (abs(double((sprite[sprite_order[i]]["y"]) -
        // map_y) <= 1) << " # " << (sprite[sprite_order[i]]["x"] << ", " <<
        // (sprite[sprite_order[i]]["y"] << " @ " << map_x << ", " << map_y <<
        // ": " << (sprite[sprite_order[i]]["texture"] << std::endl;
        if ((*world_map)[map_x][map_y] > 0) {
            hit_wall = (*world_map)[map_x][map_y];
            break;
        } else if (std::floor(this->position_x) == map_x &&
                   std::floor(this->position_y) == map_y) {
            if (side == 0) {
                // If the side of the wall hit was along the x-axis
                perpendicular_wall_distance =
                    (map_x - (*position_x) + (1 - step_x) / 2) /
                    ray_direction_x;
            } else {
                // If the side of the wall hit was along the y-axis
                perpendicular_wall_distance =
                    (map_y - (*position_y) + (1 - step_y) / 2) /
                    ray_direction_y;
            }

            // std::cout << "IN LINE OF SIGHT " <<
            // this->calculate_rotation(*position_x, *position_y, true) << " "
            // << map_x << " " << map_y << " : " << this->position_x << " " <<
            // this->position_y << std::endl;
            return {true, perpendicular_wall_distance};
        }
    }
    // std::cout << "OUT OF LINE OF SIGHT " <<
    // this->calculate_rotation(*position_x, *position_y, true) << " " << map_x
    // << " " << map_y << " : " << this->position_x << " " << this->position_y
    // << std::endl;
    return std::pair<bool, double>{false, -1};
}

int Enemy::get_id() { return this->id; }

std::string Enemy::get_type() { return this->type; }

double Enemy::get_position_x() { return this->position_x; }
double Enemy::get_position_y() { return this->position_y; }

int Enemy::get_direction() { return this->direction; }

void Enemy::set_rotation(int rotation) {
    if (this->hunting_player == false) {
        this->rotation = rotation;
    }
}

double Enemy::get_health() { return this->health; }
double Enemy::remove_health(double health) {
    this->health -= health;
    return this->health;
}
void Enemy::set_health(double health) { this->health = health; }
bool Enemy::is_alive() { return this->alive; }

int Enemy::get_state() { return this->state; }
void Enemy::set_state(int state) {
    if (!this->alive) {
        return;
    }
    if (this->previous_state != state) {
        this->timer = 0;
        this->animation_frame = 0;
        this->previous_state = this->state;
    }
    this->state = state;
}

void Enemy::play_sound(int sound, double arg0) {
    switch (this->state) {
        case state::DIE:
            if (this->health <= 0 && this->alive) {
                // std::cout << "Enemy: *ded* "
                //           << (MIX_MAX_VOLUME - (arg0 * 15) > 0 ? MIX_MAX_VOLUME - (arg0 * 15) : 0)
                //           << std::endl;
                Sample *sample = this->sound_enemy_die.at(int(rand() % this->sound_enemy_die.size()));
                sample->set_volume(MIX_MAX_VOLUME - (arg0 * 15) > 0 ? MIX_MAX_VOLUME - (arg0 * 15) : 0);
                sample->play();
                this->alive = false;
            }
            break;
        case state::PAIN:
            if (this->health > 0) {
                // std::cout << "Enemy: Ow! "
                //           << (MIX_MAX_VOLUME - (arg0 * 15) > 0 ? MIX_MAX_VOLUME - (arg0 * 15) : 0)
                //           << std::endl;
                this->sound_enemy_pain->set_volume(MIX_MAX_VOLUME - (arg0 * 15) > 0 ? MIX_MAX_VOLUME - (arg0 * 15) : 0);
                this->sound_enemy_pain->play();
            }
            break;
        case state::IDLE:
            if (this->health > 0) {
            }
            break;
        case state::SHOOT:
            if (this->health > 0 && this->is_shooting) {
                // std::cout << "Enemy: Pow!! "
                //           << (MIX_MAX_VOLUME - (arg0 * 15) > 0 ? MIX_MAX_VOLUME - (arg0 * 15) : 0)
                //           << std::endl;
                this->sound_enemy_gun.at(this->type)
                    ->set_volume(MIX_MAX_VOLUME - (arg0 * 15) > 0 ? MIX_MAX_VOLUME - (arg0 * 15) : 0);
                this->sound_enemy_gun.at(this->type)->play();
            }
            break;
        case state::WALK:
            if (this->health > 0) {
            }
            break;
    }
}

void Enemy::set_tracking_player(bool tracking_player) {
    this->tracking_player = tracking_player;
}

void Enemy::init_texture(RenderWindow *window) {
    picoPNG pico;

    // this->enemy_texture.reserve(48);

    for (int i = 0; i < 49; i++) {
        this->enemy_texture[i].resize(TEXTURE_WIDTH * TEXTURE_HEIGHT);
    }

    unsigned long tw, th, error = 0;

    /*
    1-4:
    1 = Frame 1
    2 = Frame 2
    3 = Frame 3
    4 = Frame 4

    1-8:
    1 = o\/
    2 = /o
    3 = -o
    4 = \o
    5 = o^
    6 = o/
    7 = o-
    8 = o\
    */

    // Death animation
    error |= pico.loadImage(this->enemy_texture[0], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_die1.png");
    error |= pico.loadImage(this->enemy_texture[1], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_die2.png");
    error |= pico.loadImage(this->enemy_texture[2], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_die3.png");
    error |= pico.loadImage(this->enemy_texture[3], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_die4.png");
    // Hit/Pain animation
    error |= pico.loadImage(this->enemy_texture[4], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_pain1.png");
    error |= pico.loadImage(this->enemy_texture[5], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_pain2.png");
    // Idle animation Rotation 1-8
    error |= pico.loadImage(this->enemy_texture[6], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_s_1.png");
    error |= pico.loadImage(this->enemy_texture[7], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_s_2.png");
    error |= pico.loadImage(this->enemy_texture[8], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_s_3.png");
    error |= pico.loadImage(this->enemy_texture[9], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_s_4.png");
    error |= pico.loadImage(this->enemy_texture[10], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_s_5.png");
    error |= pico.loadImage(this->enemy_texture[11], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_s_6.png");
    error |= pico.loadImage(this->enemy_texture[12], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_s_7.png");
    error |= pico.loadImage(this->enemy_texture[13], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_s_8.png");
    // Shoot Frame 1-3
    error |= pico.loadImage(this->enemy_texture[14], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_shoot1.png");
    error |= pico.loadImage(this->enemy_texture[15], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_shoot2.png");
    error |= pico.loadImage(this->enemy_texture[16], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_shoot3.png");

    // Walking Frame 1-4 Rotation 1-8
    int walk_offset = 17;
    for (int j = 1; j < 5; j++) {
        for (int k = 1; k < 9; k++) {
            error |= pico.loadImage(this->enemy_texture[walk_offset], tw, th, "res/textures/enemies/" + this->type + "/" + this->type + "_w" + std::to_string(j) + "_" + std::to_string(k) + ".png");
            this->enemy_shoot_textures[this->type + "_w" + std::to_string(j) + "_" + std::to_string(k) + ".png"] = walk_offset;
            //std::cout << "Setting " << (this->type + "_w" + std::to_string(j) + "_" + std::to_string(k) + ".png") << " to " << walk_offset << std::endl;
            walk_offset++;
        }
    }
    if (error) {
        std::cout << "Error loading textures" << std::endl;
        return std::exit(EXIT_FAILURE);
    }
}

std::vector<Uint32> Enemy::get_texture() {
    this->timer += (this->time - this->previous_time) / 1000.0;

    if (this->tracking_player && this->hunting_player == false) {
        this->rotation = 1;
    }

    switch (this->state) {
        case state::DIE:
            if (this->health <= 0) {
                if (this->animation_frame < 4 && this->timer > 0.2) {
                    this->animation_frame++;
                    this->timer = 0;
                }
                if (this->animation_frame == 0) {
                    animation_frame++;
                }
                return this->enemy_texture[this->state::DIE +
                                           this->animation_frame - 1];
            }
            break;
        case state::PAIN:
            if (this->health > 0) {
                if (this->animation_frame < 3 && this->timer > 0.3) {
                    this->animation_frame++;
                    this->timer = 0;
                    if (this->animation_frame == 3) {
                        this->set_state(this->tracking_player ? this->state::SHOOT : this->previous_state);
                    }
                }
                if (this->animation_frame == 0) {
                    this->animation_frame++;
                } else if (this->animation_frame == 3) {
                    this->animation_frame--;
                }
                return this->enemy_texture[this->state::PAIN +
                                           this->animation_frame - 1];
            }
            break;
        case state::IDLE:
            if (this->health > 0) {
                return this
                    ->enemy_texture[this->state::IDLE + this->rotation - 1];
            }
            break;
        case state::SHOOT:
            if (this->health > 0) {
                if (this->animation_frame < 4 && this->timer > 0.3) {
                    this->animation_frame++;
                    // std::cout << "Frame at " << this->animation_frame
                    //           << std::endl;
                    if (this->animation_frame == 4) {
                        this->animation_frame = 3;
                        // std::cout << "Timer at " << this->timer << std::endl;
                        this->is_shooting = false;
                        this->damaged_player = false;
                        if (this->timer > 2) {
                            // std::cout << "Returning to " << this->previous_state
                            //           << std::endl;
                            this->timer = 0;
                            this->animation_frame = 0;
                            // Player tracking
                            this->tracking_player = false;
                            this->hunt_timer = 0;
                            this->hunting_player = true;
                            this->set_state(this->state::IDLE);
                        } else {
                            // std::cout << "SHOOT STATIC" << std::endl;
                            return this->enemy_texture[this->state::SHOOT];
                        }
                    } else {
                        this->timer = 0;
                    }
                }
                if (this->animation_frame == 0) {
                    this->animation_frame++;
                } else if (this->animation_frame == 3) {
                    // Muzzle Flash
                    this->sound_enemy_gun[this->type]->play();

                    // Check if the player is still in LOS before taking damage
                    std::pair<bool, double> has_LOS =
                        this->line_of_sight(this->player->get_position_x(),
                                            this->player->get_position_y(),
                                            this->player->get_world_map());
                    if (has_LOS.first) {
                        if (has_LOS.second <= this->max_fire_distance) {
                            double damage_dealt = this->weapon_damage.at(this->type) - (has_LOS.second * 2);
                            if (damage_dealt < 0) {
                                damage_dealt = 0;
                            }
                            if (int(rand() % this->hit_chance.at(this->type)) == 0 && this->damaged_player == false) {
                                this->player->hit(damage_dealt);
                                damaged_player = true;
                            }
                        }
                    }
                }
                return this->enemy_texture[this->state::SHOOT + this->animation_frame - 1];
            }
            break;
        case state::WALK:
            if (this->health > 0) {
                // std::cout << this->type << "_w" << this->walk_frame << "_" << this->rotation << " TEXTURE " << this->enemy_shoot_textures.at(this->type + "_w" + std::to_string(this->walk_frame) + "_" + std::to_string(this->rotation) + ".png") << std::endl;
                return this->enemy_texture[this->enemy_shoot_textures.at(this->type + "_w" + std::to_string(this->walk_frame) + "_" + std::to_string(this->rotation) + ".png")];
            }
            break;
    }

    return this->enemy_texture[this->state::IDLE + this->rotation - 1];
}

void Enemy::update() {
    this->previous_time = this->time;
    this->time = SDL_GetTicks();
}