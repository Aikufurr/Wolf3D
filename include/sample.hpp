/**
 * @file sample.hpp
 * @author Aikufurr
 * @brief Contains the header functions for the Enemy Class
 * @version 1.0
 * @date 2021-07-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once
#include <SDL2/SDL_mixer.h>

#include <memory>
#include <string>

class Sample {
   public:
    //
    /**
     * @brief Construct the Sample object
     * 
     * @param path Path to the sound effect
     * @param volume Volume to play the sound effect at
     * @param stack If the audio can stack on top of itself
     */
    Sample(const std::string &path, int volume, bool stack = false);

    /**
     * @brief Play the Sample on a free mixer channel
     * 
     */
    void play();

    /**
     * @brief Play the Sample on a free mixer channel with a loop of an amount
     * 
     * @param times Repeat for this many times
     */
    void play(int times);

    /**
     * @brief Set the volume of the Sample
     * 
     * @param volume 
     */
    void set_volume(int volume);

   private:
    // The chunk for storing the sound effect in
    std::unique_ptr<Mix_Chunk, void (*)(Mix_Chunk *)> chunk;
    // The channel being used for playing back the audio, used to prevent stacking
    int playing_channel = -1;
    // If stacking is allowed
    bool stack = false;
};