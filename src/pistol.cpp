#include "../include/pistol.hpp"


Pistol::Pistol() {
    this->set_audio(new Sample("res/sounds/Pistol.wav", MIX_MAX_VOLUME, true));
    this->set_capacity(20);
}

void Pistol::init_textures(RenderWindow *window) {
    std::vector<SDL_Texture *> textures;
    textures.push_back(window->load_texture("res/textures/pistol/01.png"));
    textures.push_back(window->load_texture("res/textures/pistol/02.png"));
    textures.push_back(window->load_texture("res/textures/pistol/03.png"));
    textures.push_back(window->load_texture("res/textures/pistol/04.png"));
    textures.push_back(window->load_texture("res/textures/pistol/05.png"));
    textures.push_back(window->load_texture("res/textures/pistol/06.png"));
    this->set_weapon_specifications(textures, 2, 1.25, 6);
}