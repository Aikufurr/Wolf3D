/**
 * @file enemy.hpp
 * @author Aikufurr
 * @brief Contains the header functions for the Enemy Class
 * @version 1.0
 * @date 2021-07-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once
#include "astar.hpp"
#include "global.hpp"
#include "renderwindow.hpp"
#include "sample.hpp"

// Forward (Class) Declaration
// https://en.cppreference.com/w/cpp/language/class#Forward_declaration
class Player;

class Enemy {
   public:
    //
    /**
     * @brief States that the Enemy's animation could be in
     * 
     */
    enum state {
        DIE = 0,
        PAIN = 4,
        IDLE = 6,
        SHOOT = 14,
        WALK = 17
    };

    /**
     * @brief Construct the Enemy object
     * 
     * @param id Index of the Enemy
     * @param type Type of the Enemy
     * @param position_x The Enemy's Position X
     * @param position_y The Enemy's Position Y
     * @param direction The Enemy's Direction (0-359)
     * @param player The Player Class
     * @param world_map The vector of the world map
     */
    Enemy(int id, std::string type, double position_x, double position_y, double direction, Player *player, std::vector<std::vector<int>> *world_map);

    /**
     * @brief Resets the Enemy back to its defaults
     * 
     * @param sprite The "vector" of the sprites
     */
    void reset(json *sprite);

    /**
     * @brief Calculate the rotation of the Enemy relative to a target's Postion X/Y
     * 
     * @param position_x The target's Position X
     * @param position_y The target's Position Y
     * @param angle If false, return the rotation (1-8). If true, return the angle (0-359)
     * @return double 
     */
    double calculate_rotation(double position_x, double position_y, bool angle = false);

    /**
     * @brief Perform the Enemy's Artificial Intelligence
     * 
     * @param position_x The Player's Position X
     * @param position_y The Player's Position Y
     * @param world_map The vector of the world map
     * @param sprite The "vector" of the sprites
     */
    void ai(double *position_x, double *position_y, std::vector<std::vector<int>> *world_map, json *sprite);

    /**
     * @brief A subset of the Enemy's Artificial Intelligence for when the Player shoots within a certain range to the Enemy
     * 
     * @param position_x The Player's Position X
     * @param position_y The Player's Position Y
     */
    void handle_shot(double *position_x, double *position_y);

    /**
     * @brief Calculating if the Enemy has Line of Sight with the provided coordinates and wall collisions with the world map
     * 
     * @param position_x The Player's Position X
     * @param position_y The Player's Position Y
     * @param world_map The vector of the world map
     * @return std::pair<bool, double> Bool = If there is Line Of Sight
     * Double = Perpendicular distance from the Enemy to the target
     */
    std::pair<bool, double> line_of_sight(double *position_x, double *position_y, std::vector<std::vector<int>> *world_map);

    /**
     * @brief Get the Enemy's id
     * 
     * @return int 
     */
    int get_id();

    /**
     * @brief Get the Enemy's type
     * 
     * @return std::string 
     */
    std::string get_type();

    /**
     * @brief Get the Enemy's position x
     * 
     * @return double 
     */
    double get_position_x();

    /**
     * @brief Get the Enemy's position y
     * 
     * @return double 
     */
    double get_position_y();

    /**
     * @brief Get the Enemy's direction
     * 
     * @return int 
     */
    int get_direction();

    /**
     * @brief Set the Enemy's rotation
     * 
     * @param rotation Rotation of the Enemy (1-8)
     */
    void set_rotation(int rotation);

    /**
     * @brief Get the Enemy's health
     * 
     * @return double
     */
    double get_health();

    /**
     * @brief Remove health from the Enemy's health
     * 
     * @param health Heath to remove from the Enemy's health
     * @return double The new health of the Enemy
     */
    double remove_health(double health);

    /**
     * @brief Set the Enemy's health
     * 
     * @param health Heath to set the Enemy's health to
     */
    void set_health(double health);

    /**
     * @brief If the Enemy is alive
     * 
     * @return true The Enemy is alive
     * @return false The Enemy is not alive
     */
    bool is_alive();

    /**
     * @brief Get the Enemy's animation state
     * 
     * @return int 
     */
    int get_state();

    /**
     * @brief Set the Enemy's animation state
     * 
     * @param state The #state to set
     */
    void set_state(int state);

    /**
     * @brief Play a sound effect
     * 
     * @param sound The corresponding @ref ::state of the sound effect to play
     * @param arg0 Argument 0 for sound effect playback
     */
    void play_sound(int sound, double arg0);

    /**
     * @brief Set if the Enemy is tracking the Player
     * 
     * @param tracking_player 
     */
    void set_tracking_player(bool tracking_player);

    /**
     * @brief Load the textures into memory
     * 
     * @param window RenderWindow Renderer to use to load the textures from
     */
    void init_texture(RenderWindow *window);

    /**
     * @brief Get the Enemy's current texture accounting for their #state
     * 
     * @return std::vector<Uint32> 
     */
    std::vector<Uint32> get_texture();

    /**
     * @brief Update the timer 
     * 
     */
    void update();

   private:
    // Field of View
    std::map<std::string, int> fov{
        {"mguard", 60}};
    // Hit chance 1/x
    std::map<std::string, int> hit_chance{
        {"mguard", 2}};
    // Weapon damage (before debuff)
    std::map<std::string, int> weapon_damage{
        {"mguard", 30}};    

    // Maximum distance the Enemy can fire
    int max_fire_distance = 6;
    // Maximum distance the Enemy can see
    int max_view_distance = 15;
    // Used to ensure the sound effect and fire rate don't stack
    bool is_shooting = false;
    // Sample for the Enemy's pain sound effect
    Sample *sound_enemy_pain;
    // Vector of Samples for the Enemy's death sound effects
    std::vector<Sample *> sound_enemy_die;
    // The sounds the Enemy's Weapon can make
    std::map<std::string, Sample *> sound_enemy_gun;

    // The Enemy's id
    int id;
    // The Enemy's type
    std::string type;
    // The Enemy's Position
    double position_x;
    double position_y;
    // The Enemy's Starting Position
    double staring_x;
    double staring_y;
    // The last seen Player's Position from the Enemy's Perspective
    double last_player_x;
    double last_player_y;
    // The Enemy's direction (0-359)
    int direction = 0;
    int starting_direction = 0;

    // The Player Class for their Position and dealing damage, @see Player::hit(double)
    Player *player = nullptr;
    // Used to prevent stacking of damage applied to the Player
    bool damaged_player = false;

    // If the Enemy is tracking the Player's movement
    bool tracking_player = false;
    // If the Enemy is actively moving towards the Player's last seen Position
    bool hunting_player = false;
    // Pathfinding
    AStar::Generator *generator;
    AStar::CoordinateList hunt_path;
    int hunt_offset = 0;

    // Enemy's animation state, used for texture index
    int state = state::IDLE;
    // Previous state, used to prevent stacking and revertion
    int previous_state = 0;
    // Offset of texture index for frame
    int animation_frame = 0;
    // Frame for walking
    int walk_frame = 1;
    // Rotation for texture 1-8
    int rotation = 1;
    int starting_rotation = 1;

    // Vector of the Enemy's textures
    std::vector<Uint32> enemy_texture[49];
    // Used to map the walking texture file names to their index in the enemy_texture vector
    std::map<std::string, int> enemy_shoot_textures;

    // Enemy health
    double health = 100;
    bool alive = true;

    // Timer variables
    double previous_time = 0.0;
    double time = 0.0;
    double timer = 0.0;
    double shoot_timer = 0.0;
    double hunt_timer = 0.0;
    double walk_timer = 0.0;
};