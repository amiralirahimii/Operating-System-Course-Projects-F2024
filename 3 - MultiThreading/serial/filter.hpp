#ifndef FILTER_HPP
#define FILTER_HPP

#include <vector>
#include <cmath>

typedef struct Pixel{
    int r;
    int g;
    int b;
}Pixel;

using Image = std::vector<std::vector<Pixel>>;

namespace filter{
    
    void mirrorImage(int rows, int cols, Image& img);
    void fixPixel(Pixel& pixel);
    void blurImage(int rows, int cols, Image& img);
    void purpleImage(int rows, int cols, Image& img);
    void hatchImage(int rows, int cols, Image& img);
}


#endif