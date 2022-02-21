/**
 * @file weapon.hpp
 * @author Aikufurr
 * @brief Contains the header functions for the Weapon Class
 * @version 1.0
 * @date 2021-07-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once
#include "enemy.hpp"
#include "global.hpp"
#include "renderwindow.hpp"
#include "sample.hpp"

class Weapon {
   public:
    //
    /**
     * @brief Construct the Weapon object
     * 
     */
    Weapon();

    /**
     * @brief Get the Weapon's capacity (clip)
     * 
     * @return int 
     */
    int get_capacity();

    /**
     * @brief Set the Weapon's capacity (clip)
     * 
     * @param value Value to set the Weapon's capacity (clip) to
     */
    void set_capacity(int value);

    /**
     * @brief Set the Weapon's specifications
     * 
     * @param texture Vector of the frames of the Weapon
     * @param fire_frame The frame (0,1,2,..) of the texture of when the Weapon has "fired", example of when the muzzle flash is showing
     * @param fire_cooldown Weapon's fire cooldown in seconds before it can shoot again
     * @param max_fire_distance The Weapon's maximum fire range
     */
    void set_weapon_specifications(std::vector<SDL_Texture *> texture, int fire_frame = 3, int fire_cooldown = 0.75, int max_fire_distance = 6);

    /**
     * @brief Set the Weapon's fire sound effect
     * 
     * @param sample Sample for the Weapon's fire sound effect
     */
    void set_audio(Sample *sample);

    /**
     * @brief Fire the Weapon
     * 
     * @param position_x The Player's Position X
     * @param position_y The Player's Position Y
     * @param direction_x The Player's Direction X 
     * @param direction_y The Player's Direction Y
     * @param plane_x The Camera's Plane X
     * @param plane_y The Camera'S Plane Y
     * @param world_map The vector of the world map
     * @param sprite The "vector" of sprites
     * @param enemies The vector of enemies
     * @param haptic The controller's haptic feedback for when the Weapon fires
     * 
     * @return int Score 
     */
    int fire(double *position_x, double *position_y, double *direction_x, double *direction_y, double *plane_x, double *plane_y, std::vector<std::vector<int>> *world_map, json *sprite, std::vector<Enemy *> *enemies, SDL_Haptic *haptic = NULL);

    /**
     * @brief Render the weapon to the renderer
     * 
     * @param window RenderWindow Renderer to render too
     */
    void render(RenderWindow *window);

   private:
    // Sound effect for when the Weapon fires
    Sample *weapon_sample = NULL;
    // The vector of the frames of the Weapon
    std::vector<SDL_Texture *> weapon_texture;
    // Frame for controlling the weapon_texture index
    int frame = 0;
    // Frame for when the Weapon has "fired", example for when the muzzle flash appears
    int fire_frame = 3;
    // How long in seconds before the Weapon can fire again
    int fire_cooldown = 0.75;
    // The Weapon's maximum fire range
    int max_fire_distance = 6;
    // Timer to control the Weapon's fire cooldown
    double fire_timer = 0.0;
    // Decremental timer for displaying the Weapon frames on the window 
    int timer = 3;
    // Used to control whether to start the Weapon's fire frame sequence
    bool fired = false;
    // Used to prevent the sample playing multiple times per frame
    bool weapon_sample_complete = true;
    // Timer variables
    double time = 0;
    double previous_time = 0;
    // The Weapon's damage
    double damage = 40;

    int capacity = 0;
};