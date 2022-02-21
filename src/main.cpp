/**
 * @file main.cpp
 * @author Aikufurr
 * @brief Wolf3D is a Wolfenstein 3D inspried "game".
 * The game is written in C++ and used the SDL2 framework for rendering to a window
 * @version 1.0
 * @date 2021-07-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/*! \mainpage Wolf3D
 *
 * \htmlinclude "./README.html"
 */

#include "../include/enemy.hpp"
#include "../include/global.hpp"
#include "../include/player.hpp"
#include "../include/renderwindow.hpp"
#include "../include/utils.hpp"

#include <array>

TTF_Font *font64;

std::ifstream ifs("res/maps/map0.json");
std::string content((std::istreambuf_iterator<char>(ifs)),
                    (std::istreambuf_iterator<char>()));

json j3 = json::parse(content);

std::vector<std::vector<int>> world_map = j3["grid"];
int MAP_WIDTH = world_map.size();
int MAP_HEIGHT = world_map.size();

// The player's position vector
double position_x = j3["player_position"][0], position_y = j3["player_position"][1];
// The player's direction vector
double direction_x = std::stod(std::string(j3["direction_x"])), direction_y = std::stod(std::string(j3["direction_y"]));
// The player's camera plane
double plane_x = std::stod(std::string(j3["plane_x"])), plane_y = std::stod(std::string(j3["plane_y"]));

int floor_texture = j3["floor_texture"], ceiling_texture = j3["ceiling_texture"];

json sprite = j3["sprites"];

// Sort algorithm for sorting the sprites based on their distance
void sort_sprites(int *order, double *dist, int amount) {
    std::vector<std::pair<double, int>> sprites(amount);
    for (int i = 0; i < amount; i++) {
        sprites[i].first = dist[i];
        sprites[i].second = order[i];
    }
    std::sort(sprites.begin(), sprites.end());
    // Restore in reverse order to go from farthest to nearest
    for (int i = 0; i < amount; i++) {
        dist[i] = sprites[amount - i - 1].first;
        order[i] = sprites[amount - i - 1].second;
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    double low_fps = 999;
    double max_fps = -1;
    std::vector<double> fps_stats;
    double fps_timer = 0;

    // Testing position
    if (0) {
        position_x = 22.9162, position_y = 13.4644;
        direction_x = -0.999942, direction_y = 0.0108145;
        plane_x = 0.00713756, plane_y = 0.659961;
    }

    if (0) {
        position_x = 4.5, position_y = 4.5;
        direction_x = 0, direction_y = 1;
        plane_x = 0.6599999999955475, plane_y = 0.000002424315368208739;
    }

    // Time of the current frame
    double time = 0;
    // Time of the previous frame
    double previous_time = 0;

    // The y-axis goes first because it works per scanline
    // std::vector<std::vector<Uint32>> buffer(SCREEN_HEIGHT + ui_height, std::vector<Uint32>(SCREEN_WIDTH)); 
    // std::array<std::array<uint32_t, SCREEN_WIDTH>, SCREEN_HEIGHT + ui_height> buffer;
    uint32_t buffer[SCREEN_HEIGHT + ui_height][SCREEN_WIDTH];
    // 1D buffer storing the perpendicular distance of each vertical stripe
    double ZBuffer[SCREEN_WIDTH];

    std::vector<Uint32> texture[11];
    for (int i = 0; i < 11; i++) {
        texture[i].resize(TEXTURE_WIDTH * TEXTURE_HEIGHT);
    }
    picoPNG pico;
    RenderWindow window;

    unsigned long tw, th, error = 0;
    error |= pico.loadImage(texture[0], tw, th, "res/textures/eagle.png");
    error |= pico.loadImage(texture[1], tw, th, "res/textures/redbrick.png");
    error |= pico.loadImage(texture[2], tw, th, "res/textures/purplestone.png");
    error |= pico.loadImage(texture[3], tw, th, "res/textures/greystone.png");
    error |= pico.loadImage(texture[4], tw, th, "res/textures/bluestone.png");
    error |= pico.loadImage(texture[5], tw, th, "res/textures/mossy.png");
    error |= pico.loadImage(texture[6], tw, th, "res/textures/wood.png");
    error |= pico.loadImage(texture[7], tw, th, "res/textures/colorstone.png");
    error |= pico.loadImage(texture[8], tw, th, "res/textures/barrel.png");
    error |= pico.loadImage(texture[9], tw, th, "res/textures/pillar.png");
    error |= pico.loadImage(texture[10], tw, th, "res/textures/greenlight.png");
    if (error) {
        std::cout << "Error loading textures" << std::endl;
        return EXIT_FAILURE;
    }

    TTF_Init();

    font64 = TTF_OpenFont("res/fonts/wolfenstein.ttf", 64);

    window.create("Raycaster", SCREEN_WIDTH, SCREEN_HEIGHT + ui_height);

    // array of \/
    // {x, y, type}
    // x/y - double of position
    // type - string of type
    json j_enemies = j3["enemies"];

    // vector of all enemies
    std::vector<Enemy *> enemies;

    // Initiate player
    // We can create the enemy vector and pass it to the player now because the pointer will get populated the line below
    Player *player = new Player(&window, &position_x, &position_y, &direction_x, &direction_y, &plane_x, &plane_y, &world_map, &sprite, &enemies);

    // Enemies

    for (int i = 0; i < int(j_enemies.size()); i++) {
        // (id, type, their x, their y)
        Enemy *enemy = new Enemy(sprite.size(), std::string(j_enemies[i]["type"]), double(j_enemies[i]["x"]), double(j_enemies[i]["y"]), double(j_enemies[i]["rotation"]), player, &world_map);
        enemy->init_texture(&window);

        enemies.push_back(enemy);
        sprite.push_back({{"x", enemy->get_position_x()}, {"y", enemy->get_position_y()}, {"id", i}});
        std::cout << "Pushing enemy: " << enemy->get_position_x() << " " << enemy->get_position_y() << " " << sprite.size() << std::endl;
    }

    int sprite_order[sprite.size()];
    double sprite_distance[sprite.size()];

    while (!window.done()) {
        // AI
        if (player->get_health() > 0) {
            for (int i = 0; i < int(enemies.size()); i++) {
                enemies.at(i)->update();
                enemies.at(i)->ai(&position_x, &position_y, &world_map, &sprite);
            }
        }

        // FLOOR CASTING
        // For every horizontal scanline on the screen
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            // Ray distance for leftmost ray (x=0) and rightmost ray (x=w)
            float ray_distance_x0 = direction_x - plane_x;
            float ray_distance_y0 = direction_y - plane_y;
            float ray_distance_x1 = direction_x + plane_x;
            float ray_distance_y1 = direction_y + plane_y;

            // The current y position compared to the centre of the screen (AKA the horizon)
            int p = y - SCREEN_HEIGHT / 2;

            // Vertical postion of the camera
            float position_z = 0.5 * SCREEN_HEIGHT;

            // The horizontal distance from the camera to the floor for the current row/scanline
            // 0.5 is the z position exactly on the horizon
            float row_distance = position_z / p;

            // Calculate the real world step vector we have to add for each x (parallel to the camera plane)
            // Adding step by step avoids multiplications with a weight in the inner loop
            float floor_step_x = row_distance * (ray_distance_x1 - ray_distance_x0) / SCREEN_WIDTH;
            float floor_step_y = row_distance * (ray_distance_y1 - ray_distance_y0) / SCREEN_WIDTH;

            // Real world coordinates of the leftmost column. This will be updated as it steps to the right
            float floor_x = position_x + row_distance * ray_distance_x0;
            float floor_y = position_y + row_distance * ray_distance_y0;

            for (int x = 0; x < SCREEN_WIDTH; ++x) {
                // The cell coordinate is the integer part of floor_x/y
                int cell_x = (int)floor_x;
                int cell_y = (int)floor_y;

                // Get the texture coordinate from the fractional part
                int texture_x = (int)(TEXTURE_WIDTH * (floor_x - cell_x)) & (TEXTURE_WIDTH - 1);
                int texture_y = (int)(TEXTURE_HEIGHT * (floor_y - cell_y)) & (TEXTURE_HEIGHT - 1);

                // Updates the floor_x/y
                floor_x += floor_step_x;
                floor_y += floor_step_y;

                // Choose the texture for the floor and ceiling
                Uint32 colour;

                // Add the floor to the buffer
                colour = texture[floor_texture][TEXTURE_WIDTH * texture_y + texture_x];
                colour = (colour >> 1) & 8355711;
                buffer[y][x] = colour;

                // Add the ceiling to the buffer
                colour = texture[ceiling_texture][TEXTURE_WIDTH * texture_y + texture_x];
                colour = (colour >> 1) & 8355711;
                buffer[SCREEN_HEIGHT - y - 1][x] = colour;
            }
        }

        // WALL CASTING
        // For every vertical stripe on the screen
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            // Calculate the x-coordinate in camera space
            double camera_x = 2 * x / double(SCREEN_WIDTH) - 1;
            double ray_direction_x = direction_x + plane_x * camera_x;  // Vector of the ray on the camera plane
            double ray_direction_y = direction_y + plane_y * camera_x;

            // DDA Algorithm

            // The ray's grid location on the map
            int map_x = int(position_x);
            int map_y = int(position_y);

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

            // Was a wall hit?
            bool hit_wall = false;
            // Was it a NS or a WE wall hit?
            int side;

            // Calculate the step and side_distance
            if (ray_direction_x < 0) {
                // If the ray is left side
                step_x = -1;
                side_distance_x = (position_x - map_x) * delta_distance_x;
            } else {
                // If the ray is right side
                step_x = 1;
                side_distance_x = (map_x + 1.0 - position_x) * delta_distance_x;
            }
            if (ray_direction_y < 0) {
                // If the ray is down side
                step_y = -1;
                side_distance_y = (position_y - map_y) * delta_distance_y;
            } else {
                // If the ray is up side
                step_y = 1;
                side_distance_y = (map_y + 1.0 - position_y) * delta_distance_y;
            }

            // Perform the DDA
            // While the ray has not hit a wall
            while (!hit_wall) {
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
                if (world_map[map_x][map_y] > 0) {
                    hit_wall = true;
                }
            }

            if (side == 0) {
                // If the side of the wall hit was along the x-axis
                perpendicular_wall_distance = (map_x - position_x + (1 - step_x) / 2) / ray_direction_x;
            } else {
                // If the side of the wall hit was along the y-axis
                perpendicular_wall_distance = (map_y - position_y + (1 - step_y) / 2) / ray_direction_y;
            }

            // Calculate the height of the line to be drawn on the screen
            int line_height = (int)(SCREEN_HEIGHT / perpendicular_wall_distance);

            // Calculate the lowest and highest pixel to fill in the current stripe
            int draw_start = -line_height / 2 + SCREEN_HEIGHT / 2;
            if (draw_start < 0) {
                draw_start = 0;
            }
            int draw_end = line_height / 2 + SCREEN_HEIGHT / 2;
            if (draw_end >= SCREEN_HEIGHT) {
                draw_end = SCREEN_HEIGHT - 1;
            }

            // Texturing
            int texture_index = int(world_map[map_x][map_y]) - 1;

            // Calculate the value of wall_x
            // Exactly where the ray hit the wall for the x-coordinate of the texture
            double wall_x;
            if (side == 0) {
                // If the side of the wall hit was along the x-axis
                wall_x = position_y + perpendicular_wall_distance * ray_direction_y;
            } else {
                // If the side of the wall hit was along the y-axis
                wall_x = position_x + perpendicular_wall_distance * ray_direction_x;
            }
            wall_x -= floor((wall_x));

            // Get the x-coordinate off the texture
            int texture_x = int(wall_x * double(TEXTURE_WIDTH));
            if ((side == 0 && ray_direction_x > 0) || (side == 1 && ray_direction_y < 0)) {
                texture_x = TEXTURE_WIDTH - texture_x - 1;
            }

            // How much to increase the texture coordinate per pixel
            double step = 1.0 * TEXTURE_HEIGHT / line_height;

            // THe starting texture coordinate
            double texture_position = (draw_start - SCREEN_HEIGHT / 2 + line_height / 2) * step;

            for (int y = draw_start; y < draw_end; y++) {
                // Cast the texture coordinate to integer, and mask with (tex_height - 1) in event of overflow
                int texture_y = (int)texture_position & (TEXTURE_HEIGHT - 1);
                texture_position += step;
                Uint32 colour = texture[texture_index][TEXTURE_HEIGHT * texture_y + texture_x];
                // Make the colour darker for the y-sides.
                if (side == 1) {
                    // The RGB byte are each divided by two by shifting and an and
                    colour = (colour >> 1) & 8355711;
                }
                buffer[y][x] = colour;
            }

            // After raycasting the wall, the ZBuffer has to be set.
            // This ZBuffer is 1D, because it only contains the distance to the wall of every vertical stripe,
            // instead of having this for every pixel.
            ZBuffer[x] = perpendicular_wall_distance;
        }

        // SPRITE CASTING
        // Sort the sprites from far to close
        for (int i = 0; i < int(sprite.size()); i++) {
            sprite_order[i] = i;
            sprite_distance[i] = ((position_x - double(sprite[i]["x"])) * (position_x - double(sprite[i]["x"])) + (position_y - double(sprite[i]["y"])) * (position_y - double(sprite[i]["y"])));
        }

        sort_sprites(sprite_order, sprite_distance, sprite.size());

        // Projection of the sprites
        for (int i = 0; i < int(sprite.size()); i++) {
            // Texture index
            bool is_sprite = true;
            int sprite_texture = 0;
            std::vector<Uint32> enemy_texture;

            double uDiv = 1;
            double vDiv = 1;
            double vMove = 0.0;

            // Location of sprite relative to camera
            double sprite_x = double(sprite[sprite_order[i]]["x"]) - position_x;
            double sprite_y = double(sprite[sprite_order[i]]["y"]) - position_y;

            if (sprite[sprite_order[i]].contains("id")) {
                is_sprite = false;
                // uDiv = 1.025;
                // vDiv = 1.025;
                // vMove = TEXTURE_HEIGHT;

                // int enemy_direction = enemies[sprite[sprite_order[i]]["id"]]->get_direction();

                // double enemy_position_x = enemies[sprite[sprite_order[i]]["id"]]->get_position_x();
                // double enemy_position_y = enemies[sprite[sprite_order[i]]["id"]]->get_position_y();

                // double enemy_direction_x = enemies[sprite[sprite_order[i]]["id"]]->get_direction_x() + enemy_position_x;
                // double enemy_direction_y = enemies[sprite[sprite_order[i]]["id"]]->get_direction_y() + enemy_position_y;

                // double side_a = std::sqrt(std::pow(position_x - enemy_position_x, 2) + std::pow(position_y - enemy_position_y, 2));
                // double side_b = std::sqrt(std::pow(enemy_position_x - enemy_direction_x, 2) + std::pow(enemy_position_y - enemy_direction_y, 2));
                // double side_c = std::sqrt(std::pow(position_x - enemy_direction_x, 2) + std::pow(position_y - enemy_direction_y, 2));

                // double semi_perimeter = (side_a + side_b + side_c) / 2;
                // double area = std::sqrt(semi_perimeter * (semi_perimeter - side_a) * (semi_perimeter - side_b) * (semi_perimeter - side_c));

                // double gamma = std::asin((2 * area) / (side_b * side_a)) * (180 / M_PI);

                // if (position_y > enemy_position_y) {
                //     gamma = 90 + (90 - gamma);
                // }

                double rotation = enemies.at(sprite[sprite_order[i]]["id"])->calculate_rotation(position_x, position_y);

                // std::cout << "Angle A: " << rotation << " " << std::round(rotation) << " " << std::endl;
                enemies.at(sprite[sprite_order[i]]["id"])->set_rotation(std::round(rotation));
                enemy_texture = enemies.at(sprite[sprite_order[i]]["id"])->get_texture();
            } else {
                sprite_texture = sprite[sprite_order[i]]["texture"];
            }

            double invert_det = 1.0 / (plane_x * direction_y - direction_x * plane_y);

            double transform_x = invert_det * (direction_y * sprite_x - direction_x * sprite_y);
            double transform_y = invert_det * (-plane_y * sprite_x + plane_x * sprite_y);  // Depth

            int sprite_screen_x = int((SCREEN_WIDTH / 2) * (1 + transform_x / transform_y));

            int vertical_move_screen = int(vMove / transform_y);

            // The height of the sprite
            int sprite_height = abs(int(SCREEN_HEIGHT / transform_y)) / vDiv;
            // Calculate lowest and highest pixel to fill in the current stripe
            int draw_start_y = -sprite_height / 2 + SCREEN_HEIGHT / 2 + vertical_move_screen;
            if (draw_start_y < 0) {
                draw_start_y = 0;
            }
            int draw_end_y = sprite_height / 2 + SCREEN_HEIGHT / 2 + vertical_move_screen;
            if (draw_end_y >= SCREEN_HEIGHT) {
                draw_end_y = SCREEN_HEIGHT - 1;
            }

            // Calculate width of the sprite
            int sprite_width = abs(int(SCREEN_HEIGHT / transform_y)) / uDiv;
            int draw_start_x = -sprite_width / 2 + sprite_screen_x;
            if (draw_start_x < 0) {
                draw_start_x = 0;
            }
            int draw_end_x = sprite_width / 2 + sprite_screen_x;
            if (draw_end_x >= SCREEN_WIDTH) {
                draw_end_x = SCREEN_WIDTH - 1;
            }

            // Loop through every vertical stripe on the screen
            for (int stripe = draw_start_x; stripe < draw_end_x; stripe++) {
                int texture_x = int((stripe - (-sprite_width / 2 + sprite_screen_x)) * TEXTURE_WIDTH / sprite_width);

                if (transform_y > 0 && stripe > 0 && stripe < SCREEN_WIDTH && transform_y < ZBuffer[stripe]) {
                    for (int y = draw_start_y; y < draw_end_y; y++) {
                        int d = (y - vertical_move_screen) * 256 - SCREEN_HEIGHT * 128 + sprite_height * 128;
                        int texture_y = ((d * TEXTURE_HEIGHT) / sprite_height) / 256;
                        Uint32 colour;
                        if (is_sprite) {
                            colour = texture[sprite_texture][TEXTURE_WIDTH * texture_y + texture_x];
                            if ((colour & 0x00FFFFFF) != 0) {
                                buffer[y][stripe] = colour;
                            }
                        } else {
                            colour = enemy_texture[TEXTURE_WIDTH * texture_y + texture_x];
                            if (colour != 4288151688) {
                                buffer[y][stripe] = colour;
                            }
                        }
                    }
                }
            }
        }

        // Rendering

        for (int y = SCREEN_HEIGHT - 1; y < SCREEN_HEIGHT + ui_height; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                // Clearing the buffer
                buffer[y][x] = 0;
            }
        }

        // Draw the pixel buffer
        window.draw_buffer(buffer);

        // player->render_buffer(&buffer);

        for (int y = 0; y < SCREEN_HEIGHT + ui_height; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                // Clearing the buffer
                buffer[y][x] = 0;
            }
        }

        player->render();

        // Input timing and FPS counter
        previous_time = time;
        time = SDL_GetTicks();
        double frameTime = (time - previous_time) / 1000.0;
        fps_timer += (time - previous_time) / 1000.0;
        SDL_Colour white = {255, 255, 255};
        // Render the FPS string
        window.render(0, -10, 1 / 2.5, utils::valtostr(1 / frameTime, 0), font64, white);

        double fps_average = 0;

        if (SDL_GetTicks() > 3000) {
            if (fps_timer > 10) {
                low_fps = 999;
                max_fps = -1;
                fps_stats.clear();
                fps_timer = 0;
            }
            if (low_fps > frameTime) {
                low_fps = frameTime;
            }
            if (max_fps < frameTime) {
                max_fps = frameTime;
            }
            fps_stats.push_back(frameTime);
            fps_average = std::accumulate(fps_stats.begin(), fps_stats.end(), 0.0) / fps_stats.size();
        }

        window.render(30, -10, 1 / 2.5, utils::valtostr(1 / low_fps, 0), font64, white);
        window.render(60, -10, 1 / 2.5, utils::valtostr(1 / max_fps, 0), font64, white);
        window.render(90, -10, 1 / 2.5, utils::valtostr(1 / fps_average, 0), font64, white);

        // Render the Player health (debug)
        window.render(0, 10, 1 / 2.5, utils::valtostr(player->get_health(), 0), font64, white);

        // for (int i = 0; i < SCREEN_HEIGHT; i += 10) {
        //     window.render(rand() % 59 + (-69), i-5, 1 / 2.5, "all work and no play makes jack a dull boy all work and no play makes jack a dull boy all work and no play makes jack a dull boy all work and no play makes jack a dull boy", font32, white);
        // }

        // Render to the screen
        window.redraw();

        const Uint8 *inkeys = window.readKeys();
        player->handle_input(inkeys, frameTime);

        // If ESC is pressed, stop
        if (window.keyDown(inkeys, SDL_SCANCODE_ESCAPE)) {
            break;
        }

        // std::cout << position_x << ", " << position_y << " : " << direction_x << ", " << direction_y << " @ " << plane_x << ", " << plane_y << std::endl;
    }

    return EXIT_SUCCESS;
}