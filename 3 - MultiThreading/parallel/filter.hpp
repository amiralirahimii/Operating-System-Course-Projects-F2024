#ifndef FILTER_HPP
#define FILTER_HPP

#include <vector>
#include <cmath>
#include <pthread.h>

const int THREAD_NUMBERS = 8;

typedef struct Pixel{
    int r;
    int g;
    int b;
}Pixel;


using Image = std::vector<std::vector<Pixel>>;

typedef struct Args{
    int rows;
    int cols;
    int tid;
    Image* img;
    char* fileBuffer;
    int bufferSize;
}Args;

namespace filter{
    void* mirrorImage(void* arg);
    void* blurImage(void* arg);
    void* purpleImage(void* arg);
    void* hatchImage(void* arg);
}


#endif