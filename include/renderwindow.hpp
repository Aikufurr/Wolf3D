/**
 * @file renderwindow.hpp
 * @author Aikufurr
 * @brief Contains the header functions for the RenderWindow Class
 * @version 1.0
 * @date 2021-07-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <array>

#include "global.hpp"

class RenderWindow {
   public:
    RenderWindow();

    /**
     * @brief Create the window and the renderer
     * 
     * @param title The window title
     * @param width The window width
     * @param height The window height
     */
    void create(const char *title, int width, int height);

    /**
     * @brief Draws a buffer of pixel colours to the screen
     * 
     * @param buffer Buffer of pixel colours
     */
    void draw_buffer(uint32_t buffer[][SCREEN_WIDTH]);

    /**
     * @brief 
     * 
     * @param x Offset from left
     * @param y Offset from top
     * @param w Width
     * @param h Height
     * @param r Red
     * @param g Green
     * @param b Blue
     * @param a Alpha
     */
    void draw_rectangle(int x, int y, int w, int h, int r, int g, int b, int a);

    /**
     * @brief 
     * 
     * @param file_path Path to the file
     * @return SDL_Texture* 
     */
    SDL_Texture *load_texture(const char *file_path);

    /**
     * @brief Render an SDL_Texture to the renderer
     * 
     * @param x Offset from left
     * @param y Offset from top
     * @param scale_factor How much to scale the image by, 1 is normal size
     * @param texture SDL_Texture to render
     */
    void render(int x, int y, float scale_factor, SDL_Texture *texture);

    /**
     * @brief Render string text to the renderer with an SDL_Colour for text colour
     * 
     * @param x Offset from left
     * @param y Offset from top
     * @param scale_factor How much to scale the image by, 1 is normal size
     * @param text Text to render
     * @param font Font to render the text in
     * @param text_colour Colour to render the text in
     * @param centre_text Centre the text at the x/y
     */
    void render(int x, int y, float scale_factor, std::string text, TTF_Font *font, SDL_Colour text_colour, bool centre_text = false);

    /**
     * @brief Draws the renderer to the window
     * 
     */
    void redraw();

    /**
     * @brief Used in while loops to tell if the window has been closed or not
     * 
     * @param quit_if_esc Quit the program is the ESC key is pressed
     * @param delay Delay per frame to save computation power
     * @return true The window has been closed
     * @return false The window is still open
     */
    bool done(bool quit_if_esc = true, bool delay = true);

    /**
     * @brief Get the current held down keys
     * 
     * @return const Uint8* 
     */
    const Uint8 *readKeys();

    /**
     * @brief If the key is held down
     * 
     * @param inkeys Uint8 of the keys on the keyboard which are held down
     * @param key Key to test
     * @return true Key is held down
     * @return false Key is now held down
     */
    bool keyDown(const Uint8 *inkeys, int key);

   private:
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *framebuffer;
};