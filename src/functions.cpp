#include "../include/global.hpp"

double range (double x, double in_min, double in_max, double out_min, double out_max) {
    return ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

double constrain (double x, double min, double max){
    return (std::min(std::max(x, min), max));
}