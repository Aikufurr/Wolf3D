/**
 * @file player.hpp
 * @author Aikufurr
 * @brief Contains the header functions for the Player Class
 * @version 1.0
 * @date 2021-07-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once
#include "enemy.hpp"
#include "global.hpp"
#include "pistol.hpp"
#include "renderwindow.hpp"
#include "sample.hpp"

class Player {
   public:
    //
    /**
     * @brief Construct the Player object
     * 
     * @param window The RenderWindow's Renderer
     * @param buffer The screen buffer
     * @param position_x The Player's Position X
     * @param position_y The Player's Position Y
     * @param direction_x The Player's Direction X
     * @param direction_y The Player's Direction Y
     * @param plane_x The Camera's Plane X
     * @param plane_y The Camera's Plane Y
     * @param world_map The vector of the world map
     * @param sprite The "vector" of sprites
     * @param enemies The vector of enemies
     */

    Player(RenderWindow *window, double *position_x, double *position_y, double *direction_x, double *direction_y, double *plane_x, double *plane_y, std::vector<std::vector<int>> *world_map, json *sprite, std::vector<Enemy *> *enemies);
    ~Player();

    /**
     * @brief Handles the input for the Player, uses both keyboard and a controller
     * 
     * @param inkeys Uint8 of the keys on the keyboard which are held down
     * @param frameTime The frame time of the program to move/rotate the Player at a contant speed
     */
    void handle_input(const Uint8 *inkeys, double frameTime);

    /**
     * @brief Renders the Player's elements and it's children's elements
     * 
     */
    void render();

    /**
     * @brief Renders the Player's elements the require a buffer
     * 
     * @param buffer Uint32 buffer to render onto
     */
    // void render_buffer(std::vector<std::vector<Uint32>> *buffer);

    /**
     * @brief Get the Player's health
     * 
     * @return double 
     */
    double get_health();

    /**
     * @brief Removes the buffered damage from the Player's health, plays the hurt sound effect and displays a red flash 
     * 
     */
    void hit();

    /**
     * @brief Removes the provided damage from the Player's health, plays the hurt sound effect and displays a red flash 
     * 
     * @param damage Damage to deal to the Player
     */
    void hit(double damage);

    /**
     * @brief Accumulates damage which can be then dealt with @ref hit()
     * 
     * @param damage Damage to accumulate which would be dealt to the Player with @ref hit()
     */
    void buffer_damage(double damage);

    /**
     * @brief Clear the accumulated damage
     * 
     */
    void clear_buffer_damage();

    /**
     * @brief Get the Player's position x
     * 
     * @return double* 
     */
    double *get_position_x();

    /**
     * @brief Get the Player's position y
     * 
     * @return double* 
     */
    double *get_position_y();

    /**
     * @brief Get the world map
     * 
     * @return std::vector<std::vector<int>>* 
     */
    std::vector<std::vector<int>> *get_world_map();


   private:
    // The controller and it's haptic feedback
    SDL_GameController *controller = NULL;
    SDL_Haptic *haptic = NULL;
    // How much the stick needs to move before it's considered an input
    double controller_tolerance = 0.3;
    // How much the trigger needs to be pressed before it's considered a click 0-1
    double controller_trigger_tolerance = 0.5;
    // When left stick clicked, how much of a speed multiplier
    double speed_multiplier = 2;

    // Player's Position, Direction, and Camera Plane
    double *position_x;
    double *position_y;
    double starting_position_x;
    double starting_position_y;
    double *direction_x;
    double *direction_y;
    double starting_direction_x;
    double starting_direction_y;
    double *plane_x;
    double *plane_y;
    double starting_plane_x;
    double starting_plane_y;

    int world_floor = 0;
    int score = 0;
    int lives = 8;

    // Player health
    double health = 100;
    double damage_buffer = 0;
    // Sample for death sound effect
    Sample *sound_player_die;
    // Vector of Samples for pain sound effects
    std::vector<Sample *> sound_player_pain;
    // Arch logo
    SDL_Texture *arch;

    // For filling the screen when dying
    Uint32 buffer[SCREEN_HEIGHT + ui_height][SCREEN_WIDTH];
    bool on_death_screen = false;

    // Used for timing for displaying the border and sound effect
    bool taking_damage = false;

    // Timer variables
    double previous_time = 0.0;
    double time = 0.0;
    double pain_timer = 0.0;
    double death_timer = 0.0;

    // The world map vector
    std::vector<std::vector<int>> *world_map;
    // "Vector" of the sprites
    json *sprite;
    // Vector of the enemies
    std::vector<Enemy *> *enemies;
    // The renderer
    RenderWindow *window;
    // Pistol Weapon
    Pistol *pistol = nullptr;

    // Font
    TTF_Font *font64;
    TTF_Font *font128;
};