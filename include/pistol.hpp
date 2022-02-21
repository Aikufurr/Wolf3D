/**
 * @file pistol.hpp
 * @author Aikufurr
 * @brief Contains the header functions for the Pistol Class
 * @version 1.0
 * @date 2021-07-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once
#include "global.hpp"
#include "renderwindow.hpp"
#include "sample.hpp"
#include "weapon.hpp"

class Pistol : public Weapon {
   public:
    // 
    /**
     * @brief Construct the Pistol object
     * 
     */
    Pistol();

    /**
     * @brief Initilise the Pistol's textures and specifications
     * 
     * @param window RenderWindow Renderer to render too
     */
    void init_textures(RenderWindow *window);

   private:
};