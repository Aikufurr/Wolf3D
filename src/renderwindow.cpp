#include "../include/renderwindow.hpp"

#include <numeric>

std::map<int, bool> keypressed;
SDL_Surface *scr;
SDL_Event event = {0};

RenderWindow::RenderWindow() : window(NULL), renderer(NULL) {
}

void RenderWindow::create(const char *title, int width, int height) {
    // Create the window
    this->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

    // Initialise the Game Controller and Haptic Feedback Sub-System
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);

    // If the window is NULL it failed to create, log reason and quit
    if (this->window == NULL) {
        std::cout << "Failed to create window. SDL2 Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        std::exit(1);
    }

    // On program exit, run SDL_Quit()
    std::atexit(SDL_Quit);

    // Create the renderer
    // SDL_RENDERER_ACCELERATED  - Hardware Acceleration
    // SDL_RENDERER_PRESENTVSYNC - Present is synchronized with the refresh rate
    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Create a frame buffer, this is used when creating a vector of Uint32 colour values then rendering at once
    this->framebuffer = SDL_CreateTexture(this->renderer,
                                          SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          width, height);

    // Open the audio mixer with 2 stereo channels
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer failed to initialize. SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
    // Allocate 32 playing channels for multiple audio sources
    Mix_AllocateChannels(32);
}

void RenderWindow::draw_buffer(uint32_t buffer[][SCREEN_WIDTH]) {
    // Converts a buffer of pixel colour values to a texture
    SDL_UpdateTexture(this->framebuffer, NULL, buffer, SCREEN_WIDTH * sizeof(uint32_t));
    // Clear the renderer
    SDL_RenderClear(this->renderer);
    // Copy the texture to the renderer
    SDL_RenderCopy(this->renderer, this->framebuffer, NULL, NULL);
}

void RenderWindow::draw_rectangle(int x, int y, int w, int h, int r, int g, int b, int a) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_SetRenderDrawColor(this->renderer, r, g, b, a);
    SDL_RenderFillRect(this->renderer, &rect);
}

SDL_Texture *RenderWindow::load_texture(const char *file_path) {
    SDL_Texture *texture = NULL;
    // Load the image into the SDL_Texture
    texture = IMG_LoadTexture(this->renderer, file_path);

    if (texture == NULL) {
        std::cout << "Failed to load texture. SDL2 Error: " << SDL_GetError() << std::endl;
    }

    return texture;
}

void RenderWindow::render(int x, int y, float scale_factor, SDL_Texture *texture) {
    SDL_Rect src;
    src.x = 0;
    src.y = 0;
    SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);

    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = src.w * scale_factor;
    dst.h = src.h * scale_factor;

    SDL_RenderCopy(renderer, texture, &src, &dst);
}

void RenderWindow::render(int x, int y, float scale_factor, std::string text, TTF_Font *font, SDL_Colour text_colour, bool centre_text) {
    // Create an SDL_Surface from the TTF rendered text
    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), text_colour);
    // Create an SDL_Texture from the SDL_Surface
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect src;
    src.x = 0;
    src.y = 0;
    src.w = surface->w;
    src.h = surface->h;

    SDL_Rect dst;
    if (centre_text) {
        int z = 0;
        z -= surface->w / 2;
        z += x;
        dst.x = z;
    } else {
        dst.x = x;
    }
    dst.y = y;
    dst.w = src.w * scale_factor;
    dst.h = src.h * scale_factor;

    SDL_RenderCopy(renderer, texture, &src, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void RenderWindow::redraw() {
    SDL_RenderPresent(this->renderer);
    //SDL_Flip(scr); // this could potentially be faster than SDL_UpdateRect if double buffering is used
}

bool RenderWindow::done(bool quit_if_esc, bool delay) {
    if (delay) SDL_Delay(5);  //so it consumes less processing power
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) return true;
    }
    return false;
}

const Uint8 *RenderWindow::readKeys() {
    return SDL_GetKeyboardState(NULL);
}

bool RenderWindow::keyDown(const Uint8 *inkeys, int key) {
    if (!inkeys) return false;
    return (inkeys[key] != 0);
}