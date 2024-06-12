#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>
#include <pthread.h>

#include "filter.hpp"

const std::string OUTPUT_FILE = "output.bmp";

using myTime = std::chrono::time_point<std::chrono::high_resolution_clock>;

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack(pop)

int rows;
int cols;

bool fillAndAllocate(char*& buffer, const char* fileName, int& rows, int& cols, int& bufferSize) {
    std::ifstream file(fileName);
    if (!file) {
        std::cout << "File" << fileName << " doesn't exist!" << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    return true;
}

void* getPixelsFromBMP24(void* arg) {
    Args* thread_arg =(Args*) arg;
    int end = thread_arg->bufferSize;
    int rows = thread_arg->rows;
    int cols = thread_arg->cols;
    char* fileReadBuffer = thread_arg->fileBuffer;
    Image* img = thread_arg->img;
    int tid = thread_arg->tid;

    int startRow = rows / THREAD_NUMBERS * tid;
    int endRow = (tid == THREAD_NUMBERS - 1) ? rows : rows / THREAD_NUMBERS * (tid + 1);
    int extra = cols % 4;
    int count = (tid * (rows/THREAD_NUMBERS)) * (cols * 3) + 1;

    for (int i = startRow; i < endRow; i++) {
        count += extra;
        for (int j = cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                unsigned char thisRGB = fileReadBuffer[end - count];
                switch (k) {
                case 0:
                    (*img)[i][j].r = int(thisRGB);
                    break;
                case 1:
                    (*img)[i][j].g = int(thisRGB);
                    break;
                case 2:
                    (*img)[i][j].b = int(thisRGB);
                    break;
                }
                count++;
            }
        }
    }
    pthread_exit(NULL); 
}

void* writeOutBmp24(void* arg) {
    Args* thread_arg =(Args*) arg;
    int bufferSize = thread_arg->bufferSize;
    int rows = thread_arg->rows;
    int cols = thread_arg->cols;
    char* fileBuffer = thread_arg->fileBuffer;
    Image* img = thread_arg->img;
    int tid = thread_arg->tid;

    std::ofstream write(OUTPUT_FILE);
    if (!write) {
        std::cout << "Failed to write " << OUTPUT_FILE << std::endl;
    }

    int startRow = rows/THREAD_NUMBERS * tid;
    int endRow = (tid == THREAD_NUMBERS - 1) ? rows : rows / THREAD_NUMBERS * (tid + 1);
    int extra = cols % 4;
    int count = (tid * (rows/THREAD_NUMBERS)) * (cols * 3 + extra) + 1;

    for (int i = startRow; i < endRow; i++) {
        count += extra;
        for (int j = cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++) {
                switch (k) {
                case 0:
                    fileBuffer[bufferSize - count] = char((*img)[i][j].r);
                    break;
                case 1:
                    fileBuffer[bufferSize - count] = char((*img)[i][j].g);
                    break;
                case 2:
                    fileBuffer[bufferSize - count] = char((*img)[i][j].b);
                    break;
                }
                count++;
            }
        }
    }
    write.write(fileBuffer, bufferSize);
    pthread_exit(NULL); 
}

void creadteAndJoinThreads(void* (*func)(void*), Args arg)
{
    pthread_t threads[THREAD_NUMBERS];
    std::vector<Args*> args;
    for (int i = 0; i < THREAD_NUMBERS; i++){
        Args* thread_arg = (Args*) malloc(sizeof(Args));
        *thread_arg = arg;
        thread_arg->tid = i;
        args.push_back(thread_arg);
        int result = pthread_create(&threads[i], NULL, func, (void *)thread_arg);
        if (result){
            std::cout << "Error Creating Thread" << std::endl;
            exit(-1);
        }
    }
    for (int i = 0; i < THREAD_NUMBERS; i++){
        int result = pthread_join(threads[i], NULL);
        free(args[i]);
        if (result){
            std::cout << "Error Joining Thread" << std::endl;
            exit(-1);
        }
    }
}

void printExecTimes(myTime start, myTime readStart, myTime readEnd, myTime flipEnd, myTime blurEnd, myTime purpleEnd, myTime linesEnd, myTime end){
    std::cout << "Read: " <<  std::fixed << std::setprecision(3) << std::chrono::duration<double, std::milli> (readEnd - readStart).count() << " ms" << std::endl;
    std::cout << "Flip: " <<  std::fixed << std::setprecision(3) << std::chrono::duration<double, std::milli> (flipEnd - readEnd).count() << " ms" << std::endl;
    std::cout << "Blur: " <<  std::fixed << std::setprecision(3) << std::chrono::duration<double, std::milli> (blurEnd - flipEnd).count() << " ms" << std::endl;
    std::cout << "Purple: " <<  std::fixed << std::setprecision(3) << std::chrono::duration<double, std::milli> (purpleEnd - blurEnd).count() << " ms" << std::endl;
    std::cout << "Lines: " <<  std::fixed << std::setprecision(3) << std::chrono::duration<double, std::milli> (linesEnd - purpleEnd).count() << " ms" << std::endl;
    std::cout << "Write: " <<  std::fixed << std::setprecision(3) << std::chrono::duration<double, std::milli> (end - linesEnd).count() << " ms" << std::endl;
    std::cout << "Execution: " <<  std::fixed << std::setprecision(3) << std::chrono::duration<double, std::milli> (end - start).count() << " ms" << std::endl;
}

int main(int argc, char* argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    char* fileBuffer;
    int bufferSize;
    if (!fillAndAllocate(fileBuffer, argv[1], rows, cols, bufferSize)) {
        std::cout << "File read error" << std::endl;
        return 1;
    }
    Image img(rows, std::vector<Pixel> (cols));
    Args arg = {rows, cols, 0, &img, fileBuffer, bufferSize};

    auto readStart = std::chrono::high_resolution_clock::now();
    creadteAndJoinThreads(getPixelsFromBMP24, arg);
    auto readEnd = std::chrono::high_resolution_clock::now();
    creadteAndJoinThreads(filter::mirrorImage, arg);
    auto flipEnd = std::chrono::high_resolution_clock::now();
    creadteAndJoinThreads(filter::blurImage, arg);
    auto blurEnd = std::chrono::high_resolution_clock::now();
    creadteAndJoinThreads(filter::purpleImage, arg);
    auto purpleEnd = std::chrono::high_resolution_clock::now();
    creadteAndJoinThreads(filter::hatchImage, arg);
    auto linesEnd = std::chrono::high_resolution_clock::now();
    creadteAndJoinThreads(writeOutBmp24, arg);
    auto end = std::chrono::high_resolution_clock::now();
    printExecTimes(start, readStart, readEnd, flipEnd, blurEnd, purpleEnd, linesEnd, end);
    return 0;
}