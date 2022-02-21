#include <SDL2/SDL.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

class picoPNG {
   public:
    void loadFile(std::vector<unsigned char>& buffer, const std::string& filename);
    int loadImage(std::vector<Uint32>& out, unsigned long& w, unsigned long& h, const std::string& filename);
    int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32 = true);
    int decodePNG(std::vector<unsigned char>& out_image_32bit, unsigned long& image_width, unsigned long& image_height, const std::vector<unsigned char>& in_png);
};