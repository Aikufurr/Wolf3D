#include "../include/player.hpp"

Player::Player(RenderWindow *window, double *position_x, double *position_y, double *direction_x, double *direction_y, double *plane_x, double *plane_y, std::vector<std::vector<int>> *world_map, json *sprite, std::vector<Enemy *> *enemies) {
    this->position_x = position_x;
    this->position_y = position_y;
    this->starting_position_x = *position_x;
    this->starting_position_y = *position_y;
    this->direction_x = direction_x;
    this->direction_y = direction_y;
    this->starting_direction_x = *direction_x;
    this->starting_direction_y = *direction_y;
    this->plane_x = plane_x;
    this->plane_y = plane_y;
    this->starting_plane_x = *plane_x;
    this->starting_plane_y = *plane_y;
    this->world_map = world_map;
    this->sprite = sprite;
    this->enemies = enemies;
    this->window = window;

    // Sound
    this->sound_player_die = new Sample("res/sounds/Player Dies.wav", MIX_MAX_VOLUME);
    this->sound_player_pain.push_back(new Sample("res/sounds/Player Pain 1.wav", MIX_MAX_VOLUME));
    this->sound_player_pain.push_back(new Sample("res/sounds/Player Pain 2.wav", MIX_MAX_VOLUME));

    this->pistol = new Pistol();
    this->pistol->init_textures(window);

    this->arch = window->load_texture("res/textures/arch.png");

    // Controller
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            this->controller = SDL_GameControllerOpen(i);
            if (this->controller) {
                this->haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(this->controller));
                if (this->haptic) {
                    std::cout << ("Haptic Device (i.e. Rumble) Opened") << std::endl;
                    if (SDL_HapticRumbleInit(this->haptic) < 0) {
                        printf("Warning: Unable to initialize rumble! SDL Error: %s\n", SDL_GetError());
                    }
                } else {
                    std::cout << ("Error in opening Haptic Device (i.e. Rumble) ") << SDL_GetError() << std::endl;
                    std::cout << "Number of Haptic Devices: " << SDL_NumHaptics() << std::endl;
                    std::cout << "Controller is Haptic?: " << SDL_JoystickIsHaptic(SDL_GameControllerGetJoystick(this->controller)) << std::endl;
                }
                break;
            } else {
                fprintf(stderr, "Could not open gamecontroller %i: %s\n", i, SDL_GetError());
            }
        }
    }

    // Fonts
    TTF_Init();
    font64 = TTF_OpenFont("res/fonts/wolfenstein.ttf", 64);
    font128 = TTF_OpenFont("res/fonts/wolfenstein.ttf", 128);

    // Init buffer
    for (int y = 0; y < SCREEN_HEIGHT + ui_height; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            // Clearing the buffer
            this->buffer[y][x] = Uint32(-1);
        }
    }
}

Player::~Player() {
    delete this->position_x;
    delete this->position_y;
    delete this->direction_x;
    delete this->direction_y;
    delete this->plane_x;
    delete this->plane_y;
    delete this->world_map;
    delete this->sprite;
    delete this->enemies;
    delete this->window;
    delete this->enemies;
    delete this->sound_player_die;
    for (int i = 0; i < int(this->sound_player_pain.size()); i++) {
        delete this->sound_player_pain.at(i);
    }
    delete this->pistol;
    SDL_HapticClose(this->haptic);
    SDL_GameControllerClose(this->controller);
}

void Player::handle_input(const Uint8 *inkeys, double frameTime) {
    // Movement speed modifiers
    double movement_speed = frameTime * 5.0 > 1 ? 1 : frameTime * 5.0;  // The constant value is in squares/second
    double rotation_speed = frameTime * 3.0;                            // The constant value is in radians/second
    // const double wall_gap = 0.5;

    double controller_direction_x = 0;
    double controller_direction_y = 0;
    double controller_rotate_x = 0;
    double controller_trigger_right = 0;
    int controller_left_button = 0;

    // Handle controller
    if (this->controller) {
        // Left Stick Button
        controller_left_button = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK);
        controller_left_button = range(controller_left_button, 0, 1, 1, speed_multiplier);

        // Lext Axis X
        controller_direction_x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        controller_direction_x = range(controller_direction_x, -32768, 32767, 1, -1);
        controller_direction_x = ceil(controller_direction_x * 10);
        controller_direction_x = controller_direction_x / 10;

        // Lext Axis Y
        controller_direction_y = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
        controller_direction_y = range(controller_direction_y, -32768, 32767, 1, -1);
        controller_direction_y = ceil(controller_direction_y * 10);
        controller_direction_y = controller_direction_y / 10;

        // Right Axis X
        controller_rotate_x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
        controller_rotate_x = range(controller_rotate_x, -32768, 32767, 1, -1);
        controller_rotate_x = ceil(controller_rotate_x * 10);
        controller_rotate_x = controller_rotate_x / 10;

        // Right Trigger
        controller_trigger_right = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
        controller_trigger_right = range(controller_trigger_right, 0, 32767, 0, 1);
        controller_trigger_right = ceil(controller_trigger_right * 10);
        controller_trigger_right = controller_trigger_right / 10;

        // std::cout << "CONTROLLER: " << controller_direction_x << "," << controller_direction_y << " " << controller_rotate_x << " " << controller_trigger_right << " " << controller_left_button<< std::endl;
    }

    if (this->health > 0) {
        // If any controller is above tolerance use, else keyboard
        if (controller_direction_x >= controller_tolerance ||
            controller_direction_x <= -controller_tolerance ||
            controller_direction_y >= controller_tolerance ||
            controller_direction_y <= -controller_tolerance ||
            controller_rotate_x >= controller_tolerance ||
            controller_rotate_x <= -controller_tolerance ||
            controller_trigger_right >= controller_trigger_tolerance) {
            //
            // Up
            if (controller_direction_y >= controller_tolerance) {
                if ((*world_map)[int((*position_x) + (*direction_x) * movement_speed * (controller_direction_y * controller_left_button))][int((*position_y))] == 0) (*position_x) += (*direction_x) * movement_speed * (controller_direction_y * controller_left_button);
                if ((*world_map)[int((*position_x))][int((*position_y) + (*direction_y) * movement_speed * (controller_direction_y * controller_left_button))] == 0) (*position_y) += (*direction_y) * movement_speed * (controller_direction_y * controller_left_button);
            }
            // Down
            if (controller_direction_y <= -controller_tolerance) {
                if ((*world_map)[int((*position_x) - (*direction_x) * movement_speed * -(controller_direction_y * controller_left_button))][int((*position_y))] == 0) (*position_x) -= (*direction_x) * movement_speed * -(controller_direction_y * controller_left_button);
                if ((*world_map)[int((*position_x))][int((*position_y) - (*direction_y) * movement_speed * -(controller_direction_y * controller_left_button))] == 0) (*position_y) -= (*direction_y) * movement_speed * -(controller_direction_y * controller_left_button);
            }
            // Left
            if (controller_direction_x >= controller_tolerance) {
                // Strafe to the left
                // Left direction
                double local_direction_x = (*direction_x) * std::cos(1.5708) - (*direction_y) * std::sin(1.5708);
                double local_direction_y = (*direction_x) * std::sin(1.5708) + (*direction_y) * std::cos(1.5708);

                if ((*world_map)[int((*position_x) + local_direction_x * movement_speed * (controller_direction_x * controller_left_button))][int((*position_y))] == 0) (*position_x) += local_direction_x * movement_speed * (controller_direction_x * controller_left_button);
                if ((*world_map)[int((*position_x))][int((*position_y) + local_direction_y * movement_speed * (controller_direction_x * controller_left_button))] == 0) (*position_y) += local_direction_y * movement_speed * (controller_direction_x * controller_left_button);
            }
            // Right
            if (controller_direction_x <= -controller_tolerance) {
                // Right direction
                double local_direction_x = (*direction_x) * std::cos(-1.5708) - (*direction_y) * std::sin(-1.5708);
                double local_direction_y = (*direction_x) * std::sin(-1.5708) + (*direction_y) * std::cos(-1.5708);

                if ((*world_map)[int((*position_x) + local_direction_x * movement_speed * -(controller_direction_x * controller_left_button))][int((*position_y))] == 0) (*position_x) += local_direction_x * movement_speed * -(controller_direction_x * controller_left_button);
                if ((*world_map)[int((*position_x))][int((*position_y) + local_direction_y * movement_speed * -(controller_direction_x * controller_left_button))] == 0) (*position_y) += local_direction_y * movement_speed * -(controller_direction_x * controller_left_button);
            }

            // Rotate Left
            if (controller_rotate_x >= controller_tolerance) {
                // Rotate to the left
                // Both the camera direction and camera plane must be rotated
                double old_direction_x = (*direction_x);
                (*direction_x) = (*direction_x) * cos(rotation_speed * controller_rotate_x) - (*direction_y) * sin(rotation_speed * controller_rotate_x);
                (*direction_y) = old_direction_x * sin(rotation_speed * controller_rotate_x) + (*direction_y) * cos(rotation_speed * controller_rotate_x);
                double old_plane_x = (*plane_x);
                (*plane_x) = (*plane_x) * cos(rotation_speed * controller_rotate_x) - (*plane_y) * sin(rotation_speed * controller_rotate_x);
                (*plane_y) = old_plane_x * sin(rotation_speed * controller_rotate_x) + (*plane_y) * cos(rotation_speed * controller_rotate_x);
            }
            // Rotate Right
            if (controller_rotate_x <= -controller_tolerance) {
                // Rotate to the right
                // Both the camera direction and camera plane must be rotated
                double old_direction_x = (*direction_x);
                (*direction_x) = (*direction_x) * cos(-rotation_speed * -controller_rotate_x) - (*direction_y) * sin(-rotation_speed * -controller_rotate_x);
                (*direction_y) = old_direction_x * sin(-rotation_speed * -controller_rotate_x) + (*direction_y) * cos(-rotation_speed * -controller_rotate_x);
                double old_plane_x = (*plane_x);
                (*plane_x) = (*plane_x) * cos(-rotation_speed * -controller_rotate_x) - (*plane_y) * sin(-rotation_speed * -controller_rotate_x);
                (*plane_y) = old_plane_x * sin(-rotation_speed * -controller_rotate_x) + (*plane_y) * cos(-rotation_speed * -controller_rotate_x);
            }
            // Fire
            if (controller_trigger_right >= controller_trigger_tolerance) {
                this->score += pistol->fire(position_x, position_y, direction_x, direction_y, plane_x, plane_y, world_map, sprite, enemies, this->haptic);
            }
        } else {
            // If UP arrow is down and there is no wall in front (Moves if the grid that is going to be moved into is a '0', '0' being walkable space)
            if (this->window->keyDown(inkeys, SDL_SCANCODE_UP)) {
                if ((*world_map)[int((*position_x) + (*direction_x) * movement_speed)][int((*position_y))] == 0) (*position_x) += (*direction_x) * movement_speed;
                if ((*world_map)[int((*position_x))][int((*position_y) + (*direction_y) * movement_speed)] == 0) (*position_y) += (*direction_y) * movement_speed;
            }
            // If DOWN arrow is down and there is no wall behind (Moves if the grid that is going to be moved into is a '0', '0' being walkable space)
            if (this->window->keyDown(inkeys, SDL_SCANCODE_DOWN)) {
                if ((*world_map)[int((*position_x) - (*direction_x) * movement_speed)][int((*position_y))] == 0) (*position_x) -= (*direction_x) * movement_speed;
                if ((*world_map)[int((*position_x))][int((*position_y) - (*direction_y) * movement_speed)] == 0) (*position_y) -= (*direction_y) * movement_speed;
            }
            if (this->window->keyDown(inkeys, SDL_SCANCODE_LEFT)) {
                if (this->window->keyDown(inkeys, SDL_SCANCODE_LALT)) {
                    // Strafe to the left
                    // Left direction
                    double local_direction_x = (*direction_x) * std::cos(1.5708) - (*direction_y) * std::sin(1.5708);
                    double local_direction_y = (*direction_x) * std::sin(1.5708) + (*direction_y) * std::cos(1.5708);

                    if ((*world_map)[int((*position_x) + local_direction_x * movement_speed)][int((*position_y))] == 0) (*position_x) += local_direction_x * movement_speed;
                    if ((*world_map)[int((*position_x))][int((*position_y) + local_direction_y * movement_speed)] == 0) (*position_y) += local_direction_y * movement_speed;
                } else {
                    // Rotate to the left
                    // Both the camera direction and camera plane must be rotated
                    double old_direction_x = (*direction_x);
                    (*direction_x) = (*direction_x) * cos(rotation_speed) - (*direction_y) * sin(rotation_speed);
                    (*direction_y) = old_direction_x * sin(rotation_speed) + (*direction_y) * cos(rotation_speed);
                    double old_plane_x = (*plane_x);
                    (*plane_x) = (*plane_x) * cos(rotation_speed) - (*plane_y) * sin(rotation_speed);
                    (*plane_y) = old_plane_x * sin(rotation_speed) + (*plane_y) * cos(rotation_speed);
                }
            }
            if (this->window->keyDown(inkeys, SDL_SCANCODE_RIGHT)) {
                if (this->window->keyDown(inkeys, SDL_SCANCODE_LALT)) {
                    // Right direction
                    double local_direction_x = (*direction_x) * std::cos(-1.5708) - (*direction_y) * std::sin(-1.5708);
                    double local_direction_y = (*direction_x) * std::sin(-1.5708) + (*direction_y) * std::cos(-1.5708);

                    if ((*world_map)[int((*position_x) + local_direction_x * movement_speed)][int((*position_y))] == 0) (*position_x) += local_direction_x * movement_speed;
                    if ((*world_map)[int((*position_x))][int((*position_y) + local_direction_y * movement_speed)] == 0) (*position_y) += local_direction_y * movement_speed;
                } else {
                    // Rotate to the right
                    // Both the camera direction and camera plane must be rotated
                    double old_direction_x = (*direction_x);
                    (*direction_x) = (*direction_x) * cos(-rotation_speed) - (*direction_y) * sin(-rotation_speed);
                    (*direction_y) = old_direction_x * sin(-rotation_speed) + (*direction_y) * cos(-rotation_speed);
                    double old_plane_x = (*plane_x);
                    (*plane_x) = (*plane_x) * cos(-rotation_speed) - (*plane_y) * sin(-rotation_speed);
                    (*plane_y) = old_plane_x * sin(-rotation_speed) + (*plane_y) * cos(-rotation_speed);
                }
            }
            if (this->window->keyDown(inkeys, SDL_SCANCODE_SPACE)) {
                this->score += pistol->fire(position_x, position_y, direction_x, direction_y, plane_x, plane_y, world_map, sprite, enemies);
            }
        }
    }
}

void Player::render() {
    this->previous_time = this->time;
    this->time = SDL_GetTicks();

    // Render Player's elements

    // Black border
    this->window->draw_rectangle(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH, SCREEN_HEIGHT + ui_height, 0, 0, 0, 255);
    // Lighter blue border
    this->window->draw_rectangle(3, SCREEN_HEIGHT + 2, SCREEN_WIDTH - 6, ui_height - 5, 92, 96, 255, 255);
    // Blue background
    this->window->draw_rectangle(6, SCREEN_HEIGHT + 5, SCREEN_WIDTH - 12, ui_height - 11, 0, 0, 166, 255);
    // Group lines
    this->window->draw_rectangle(70, SCREEN_HEIGHT + 2, 2, ui_height - 5, 92, 96, 255, 255);
    this->window->draw_rectangle(200, SCREEN_HEIGHT + 2, 2, ui_height - 5, 92, 96, 255, 255);
    this->window->draw_rectangle(415, SCREEN_HEIGHT + 2, 2, ui_height - 5, 92, 96, 255, 255);
    this->window->draw_rectangle(490, SCREEN_HEIGHT + 2, 2, ui_height - 5, 92, 96, 255, 255);
    this->window->draw_rectangle(490, SCREEN_HEIGHT + 2 + ui_height / 2 - 5, 20, 6, 92, 96, 255, 255);
    this->window->draw_rectangle(510, SCREEN_HEIGHT + 2, 2, ui_height - 5, 92, 96, 255, 255);

    // Text
    // FLOOR TEXT
    window->render(15, SCREEN_HEIGHT + 2 - 5, 0.5, "FLOOR", font64, SDL_Colour{255, 255, 255});
    // FLOOR VALUE
    window->render(40, SCREEN_HEIGHT + 2 + 13, 0.75, std::to_string(this->world_floor + 1), font64, SDL_Colour{255, 255, 255}, true);
    // SCORE TEXT
    window->render(115, SCREEN_HEIGHT + 2 - 5, 0.5, "SCORE", font64, SDL_Colour{255, 255, 255});
    // SCORE VALUE
    window->render(139, SCREEN_HEIGHT + 2 + 13, 0.75, std::to_string(this->score), font64, SDL_Colour{255, 255, 255}, true);
    // LIVES TEXT
    window->render(214, SCREEN_HEIGHT + 2 - 5, 0.5, "LIVES", font64, SDL_Colour{255, 255, 255});
    // LIVES VALUE
    window->render(234, SCREEN_HEIGHT + 2 + 13, 0.75, std::to_string(this->lives), font64, SDL_Colour{255, 255, 255}, true);
    // HEAD?
    window->render(260, SCREEN_HEIGHT + 4, 0.06, this->arch);
    this->window->draw_rectangle(260, SCREEN_HEIGHT + 2, 2, ui_height - 5, 92, 96, 255, 255);
    this->window->draw_rectangle(330, SCREEN_HEIGHT + 2, 2, ui_height - 5, 92, 96, 255, 255);
    // HEALTH TEXT
    window->render(347, SCREEN_HEIGHT + 2 - 5, 0.5, "HEALTH", font64, SDL_Colour{255, 255, 255});
    // HEALTH VALUE
    std::ostringstream sstream;
    sstream << std::fixed << std::setprecision(0) << (this->health > 0 ? this->health : 0);
    window->render(380, SCREEN_HEIGHT + 2 + 13, 0.75, sstream.str(), font64, SDL_Colour{255, 255, 255}, true);
    // AMMO TEXT
    window->render(428, SCREEN_HEIGHT + 2 - 5, 0.5, "AMMO", font64, SDL_Colour{255, 255, 255});
    // AMMO VALUE
    window->render(458, SCREEN_HEIGHT + 2 + 13, 0.75, std::to_string(this->pistol->get_capacity()), font64, SDL_Colour{255, 255, 255}, true);
    // WOLF3D TEXT
    window->render(517, SCREEN_HEIGHT + 2 - 13, 0.5, "WOLF3D", font128, SDL_Colour{255, 255, 255});

    // Render children elements

    // Render pistol sprite[s]
    if (this->health > 0) {
        pistol->render(window);
    }

    // For displaying pain overlay texture
    if (this->taking_damage == true) {
        this->pain_timer += (this->time - this->previous_time) / 1000.0;

        if (this->health <= 0) {
            if (this->pain_timer < 3) {
                for (int i = 0; i < 100; i += 10) {
                    if (this->pain_timer > 3) {
                        break;
                    }
                    int x = rand() % (SCREEN_WIDTH / 10);
                    int y = rand() % (SCREEN_HEIGHT / 10);
                    while (this->buffer[y * 10][x * 10] != Uint32(-1)) {
                        if (this->pain_timer > 3) {
                            break;
                        }
                        x = rand() % (SCREEN_WIDTH / 10);
                        y = rand() % (SCREEN_HEIGHT / 10);
                    }
                    x = x + 10 > SCREEN_WIDTH ? x - 10 : x;
                    y = y + 10 > SCREEN_HEIGHT ? y - 10 : y;
                    for (int w = 0; w < 10; w++) {
                        for (int h = 0; h < 10; h++) {
                            this->buffer[y * 10 + w][x * 10 + h] = 16711680;
                        }
                    }
                }
            } else {
                if (this->on_death_screen == false) {
                    for (int y = 0; y < SCREEN_HEIGHT; y++) {
                        for (int x = 0; x < SCREEN_WIDTH; x++) {
                            this->buffer[y][x] = 16711680;
                        }
                    }
                    this->on_death_screen = true;
                }
                this->death_timer += (this->time - this->previous_time) / 1000.0;
                if (this->death_timer > 3) {
                    this->on_death_screen = false;
                    for (int y = 0; y < SCREEN_HEIGHT; y++) {
                        for (int x = 0; x < SCREEN_WIDTH; x++) {
                            this->buffer[y][x] = -1;
                        }
                    }
                    this->health = 100;
                    this->damage_buffer = 0;
                    this->score = 0;
                    this->lives--;
                    // Handle out of lives
                    this->taking_damage = false;
                    *this->position_x = this->starting_position_x;
                    *this->position_y = this->starting_position_y;
                    *this->direction_x = this->starting_direction_x;
                    *this->direction_y = this->starting_direction_y;
                    *this->plane_x = this->starting_plane_x;
                    *this->plane_y = this->starting_plane_y;
                    this->pistol->set_capacity(20);

                    this->previous_time = 0.0;
                    this->time = 0.0;
                    this->pain_timer = 0.0;
                    this->death_timer = 0.0;

                    for (int i = 0; i < int(this->enemies->size()); i++) {
                        this->enemies->at(i)->reset(this->sprite);
                    }
                }
            }
        } else {
            if (this->pain_timer > 0.2) {
                this->pain_timer = 0;
                this->taking_damage = false;
            } else {
                SDL_Texture *texture = this->window->load_texture("res/textures/pain.png");
                SDL_SetTextureAlphaMod(texture, range(constrain(this->damage_buffer, 0, 60), 0, 60, 0, 255));
                this->window->render(0, 0, 1, texture);
            }
        }
    }
}

// void Player::render_buffer(std::vector<std::vector<Uint32>> *buffer) {
//     if (this->health <= 0) {
//         for (int y = 0; y < SCREEN_HEIGHT + ui_height; y++) {
//             for (int x = 0; x < SCREEN_WIDTH; x++) {
//                 if (this->buffer[y][x] != Uint32(-1)) {
//                     // Copy the local buffer to the main buffer
//                     (*buffer)[y][x] = 16711680;
//                 }
//             }
//         }
//     }
//     // this->window->draw_buffer(buffer);
// }

double Player::get_health() {
    return this->health;
}

void Player::hit() {
    if (damage_buffer > 0) {
        this->health -= damage_buffer;
        this->sound_player_pain.at(rand() % this->sound_player_pain.size())->play();
        this->taking_damage = true;
    }
}
void Player::hit(double damage) {
    if (damage > 0) {
        this->damage_buffer = damage;
        this->health -= damage;
        this->sound_player_pain.at(rand() % this->sound_player_pain.size())->play();
        this->taking_damage = true;
    }
}
void Player::buffer_damage(double damage) {
    this->damage_buffer += damage;
}
void Player::clear_buffer_damage() {
    this->damage_buffer = 0;
}

double *Player::get_position_x() {
    return this->position_x;
}
double *Player::get_position_y() {
    return this->position_y;
}
std::vector<std::vector<int>> *Player::get_world_map() {
    return this->world_map;
}