#include "../include/sample.hpp"

Sample::Sample(const std::string &path, int volume, bool stack)
    : chunk(Mix_LoadWAV(path.c_str()), Mix_FreeChunk) {
    if (!chunk.get()) {
        // LOG("Couldn't load audio sample: ", path);
    }

    Mix_VolumeChunk(chunk.get(), volume);
    this->stack = stack;
}

// -1 here means we let SDL_mixer pick the first channel that is free
// If no channel is free it'll return an err code.
void Sample::play() {
    if (stack || this->playing_channel == -1) {
        this->playing_channel = Mix_PlayChannel(-1, chunk.get(), 0);
    } else {
        while (Mix_Playing(this->playing_channel) == 0) {
            this->playing_channel = Mix_PlayChannel(-1, chunk.get(), 0);
        }
    }
}

void Sample::play(int times) {
    if (stack || this->playing_channel == -1) {
        this->playing_channel = Mix_PlayChannel(-1, chunk.get(), times - 1);
    } else {
        while (Mix_Playing(this->playing_channel) == 0) {
            this->playing_channel = Mix_PlayChannel(-1, chunk.get(), times - 1);
        }
    }
}

void Sample::set_volume(int volume) { Mix_VolumeChunk(chunk.get(), volume); }
