#include "../include/weapon.hpp"

#include "../include/global.hpp"

Weapon::Weapon() {
}

int Weapon::get_capacity() {
    return this->capacity;
}

void Weapon::set_capacity(int value) {
    this->capacity = value;
}

void Weapon::set_weapon_specifications(std::vector<SDL_Texture *> texture, int fire_frame, int fire_cooldown, int max_fire_distance) {
    this->weapon_texture = texture;
    this->fire_frame = fire_frame;
    this->fire_cooldown = fire_cooldown;
    this->max_fire_distance = max_fire_distance;
}

void Weapon::set_audio(Sample *sample) {
    this->weapon_sample = sample;
}

void Weapon::render(RenderWindow *window) {
    this->previous_time = this->time;
    this->time = SDL_GetTicks();
    this->fire_timer += (this->time - this->previous_time) / 1000.0;
    // window->render((SCREEN_WIDTH / 2) - TEXTURE_WIDTH / 2, SCREEN_HEIGHT - TEXTURE_HEIGHT, 1, this->pistol_texture[0]);

    if (this->fired) {
        if (this->frame == this->fire_frame && this->weapon_sample_complete == false) {
            std::cout << "Playing sound" << std::endl;
            this->weapon_sample->play();
            this->weapon_sample_complete = true;
        }
        window->render(((SCREEN_WIDTH / 2) - TEXTURE_WIDTH / 2) / 3, ((SCREEN_HEIGHT - TEXTURE_HEIGHT) / 3) - 107, 7, this->weapon_texture.at(this->frame));

        if (this->frame == (int(this->weapon_texture.size()) - 1)) {
            this->frame = 0;
            this->fired = false;
            this->timer = 3;
            this->weapon_sample_complete = false;
        }
        if (this->timer <= 0) {
            frame++;
        } else {
            this->timer--;
        }
    } else {
        window->render(((SCREEN_WIDTH / 2) - TEXTURE_WIDTH / 2) / 3, ((SCREEN_HEIGHT - TEXTURE_HEIGHT) / 3) - 107, 7, this->weapon_texture.at(this->frame));
    }
}

int Weapon::fire(double *position_x, double *position_y, double *direction_x, double *direction_y, double *plane_x, double *plane_y, std::vector<std::vector<int>> *world_map, json *sprite, std::vector<Enemy *> *enemies, SDL_Haptic *haptic) {
    while (this->fire_timer > this->fire_cooldown && this->capacity > 0) {
        std::cout << "Bang Bang" << std::endl;
        this->fired = true;
        this->weapon_sample_complete = false;
        this->fire_timer = 0;
        this->capacity--;

        // Used for enemy AI, testing for if the player shot close to the enemy
        for (int i = 0; i < int(enemies->size()); i++) {
            enemies->at(i)->handle_shot(position_x, position_y);
        }

        // Calculate the x-coordinate in camera space
        double camera_x = 2 * (SCREEN_WIDTH / 2) / double(SCREEN_WIDTH) - 1;
        double ray_direction_x = (*direction_x) + (*plane_x) * camera_x;  // Vector of the ray on the camera plane
        double ray_direction_y = (*direction_y) + (*plane_y) * camera_x;

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

        std::map<int, std::string> textures = {
            {0, "Eagle"},
            {1, "Redbrick"},
            {2, "Purplestone"},
            {3, "Greystone"},
            {4, "Bluestone"},
            {5, "Mossy"},
            {6, "Wood"},
            {7, "Colorstone"},
            {8, "Barrel"},
            {9, "Pillar"},
            {10, "Greenlight"}};

        // Perform the DDA
        // While the ray has not hit a wall
        while (hit_index == -1 && hit_wall == -1 &&
               map_x < this->max_fire_distance + *position_x && map_x > -this->max_fire_distance + *position_x &&
               map_y < this->max_fire_distance + *position_y && map_y > -this->max_fire_distance + *position_y) {
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

            // If the ray has collided with a wall, 0 being walkable space, anything higher being a wall

            for (int i = 0; i < int((*sprite).size()); i++) {
                double sprite_x = double((*sprite)[i]["x"]);
                double sprite_y = double((*sprite)[i]["y"]);

                // std::cout << (abs(double((indexed_sprite["x"]) - map_x) <= 1) << ", " << (abs(double((sprite[sprite_order[i]]["y"]) - map_y) <= 1) << " # " << (sprite[sprite_order[i]]["x"] << ", " << (sprite[sprite_order[i]]["y"] << " @ " << map_x << ", " << map_y << ": " << (sprite[sprite_order[i]]["texture"] << std::endl;

                if ((*world_map)[map_x][map_y] > 0) {
                    hit_wall = (*world_map)[map_x][map_y];
                } else if (std::floor(sprite_x) == map_x && std::floor(sprite_y) == map_y) {
                    hit_index = i;
                }

                // Hit something
                if (hit_index != -1 || hit_wall != -1) {
                    if (side == 0) {
                        // If the side of the wall hit was along the x-axis
                        perpendicular_wall_distance = (map_x - (*position_x) + (1 - step_x) / 2) / ray_direction_x;
                    } else {
                        // If the side of the wall hit was along the y-axis
                        perpendicular_wall_distance = (map_y - (*position_y) + (1 - step_y) / 2) / ray_direction_y;
                    }

                    if (hit_index != -1) {
                        json hit_sprite = (*sprite)[hit_index];
                        if (hit_sprite.contains("id")) {
                            std::cout << std::endl
                                      << std::endl
                                      << "HIT ENEMY" << std::endl
                                      << hit_sprite["x"] << ", " << hit_sprite["y"] << " @ " << perpendicular_wall_distance << " : " << enemies->at(hit_sprite["id"])->get_type() << std::endl;
                            if (!enemies->at(hit_sprite["id"])->is_alive()) {
                                hit_index = -1;
                                continue;
                            }
                            if (perpendicular_wall_distance <= this->max_fire_distance) {
                                double damage_dealt = this->damage - (perpendicular_wall_distance * 2);
                                if (damage_dealt < 0) {
                                    damage_dealt = 0;
                                }
                                double health = enemies->at(hit_sprite["id"])->remove_health(damage_dealt);
                                std::cout << "Dealt " << damage_dealt << " damage to " << enemies->at(hit_sprite["id"])->get_type() << ". Now at " << health << " health" << std::endl;
                                if (health > 0) {
                                    enemies->at(hit_sprite["id"])->set_state(Enemy::state::PAIN);
                                    enemies->at(hit_sprite["id"])->play_sound(Enemy::state::PAIN, perpendicular_wall_distance);
                                    enemies->at(hit_sprite["id"])->set_tracking_player(true);
                                    
                                    return int(range(perpendicular_wall_distance, 0, this->max_fire_distance, 50, 0));
                                } else {
                                    enemies->at(hit_sprite["id"])->set_state(Enemy::state::DIE);
                                    enemies->at(hit_sprite["id"])->play_sound(Enemy::state::DIE, perpendicular_wall_distance);
                                    return 100;
                                }
                            }
                        } else {
                            std::cout << std::endl
                                      << std::endl
                                      << "HIT SPRITE" << std::endl
                                      << map_x << ", " << map_y << " @ " << perpendicular_wall_distance << " : " << textures[hit_sprite["texture"]] << std::endl;
                            if (textures[hit_sprite["texture"]] != "greenlight") {
                                hit_index = -1;
                                continue;
                            }
                        }
                    }

                    if (hit_wall != -1) {
                        std::cout << std::endl
                                  << std::endl
                                  << "HIT WALL" << std::endl
                                  << map_x << ", " << map_y << " @ " << perpendicular_wall_distance << " : " << textures[hit_wall - 1] << std::endl;
                        break;
                    }
                }
            }
        }

        if (haptic) {
            if (SDL_HapticRumblePlay(haptic, 1, 100) != 0) {
                printf("Warning: Unable to play rumble! %s\n", SDL_GetError());
            }
        }
    }

    return 0;
}