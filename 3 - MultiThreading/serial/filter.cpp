#include "filter.hpp"

namespace filter{
    void mirrorImage(int rows, int cols, Image& img){
        for(int i=0; i<rows/2; i++){
            for(int j=0; j<cols; j++){
                Pixel temp = img[i][j];
                img[i][j] = img[rows - i - 1][j];
                img[rows - i - 1][j] = temp;
            }
        }
    }
    bool isInBoard(int row, int col, int rows, int cols){
        return row<rows && row>=0 && col<cols && col>=0;
    }

    void fixPixel(Pixel& pixel){
        pixel.r = pixel.r<256 ? (pixel.r>=0 ? pixel.r : 0) : 255;
        pixel.g = pixel.g<256 ? (pixel.g>=0 ? pixel.g : 0) : 255;
        pixel.b = pixel.b<256 ? (pixel.b>=0 ? pixel.b : 0) : 255;
    }
    
    Pixel calcForThisPixel(int row, int col, int rows, int cols, const Image& img){
        Pixel result = {0, 0, 0};
        int kernel[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
        for(int i=-1; i<=1; i++){
            for(int j=-1; j<=1; j++){
                if(isInBoard(row+i, col+j, rows, cols)){
                    result.r += img[row+i][col+j].r * kernel[i+1][j+1];
                    result.g += img[row+i][col+j].g * kernel[i+1][j+1];
                    result.b += img[row+i][col+j].b * kernel[i+1][j+1];
                }
                else{
                    result.r += img[row][col].r * kernel[i+1][j+1];
                    result.g += img[row][col].g * kernel[i+1][j+1];
                    result.b += img[row][col].b * kernel[i+1][j+1];
                }
            }
        }
        result.r /= 16, result.g /= 16, result.b /= 16;
        fixPixel(result);
        return result;
    }
    void blurImage(int rows, int cols, Image& img){
        Image copyImage = img;
        for(int i=0; i<rows; i++){
            for(int j=0; j<cols; j++){
                img[i][j] = calcForThisPixel(i, j, rows, cols, copyImage);
            }
        }
    }
    void purpleImage(int rows, int cols, Image& img){
        for(int i=0; i<rows; i++){
            for(int j=0; j<cols; j++){
                Pixel temp = img[i][j];
                img[i][j].r = int(0.5*temp.r + 0.3*temp.g + 0.5*temp.b);
                img[i][j].g = int(0.16*temp.r + 0.5*temp.g + 0.16*temp.b);
                img[i][j].b = int(0.6*temp.r + 0.2*temp.g + 0.8*temp.b);
                fixPixel(img[i][j]);
            }
        }
    }
    bool isEqual(float a, float b){
        return abs(a-b) < 0.00001;
    }
    void hatchImage(int rows, int cols, Image& img){
        float shib = float(rows) / float(cols);
        for(int i=0; i<rows; i++){
            for(int j=0; j<cols; j++){
                if(isEqual(j*shib + rows/2, i) || isEqual(j*shib, i) || isEqual(j*shib - rows/2, i)){
                    img[rows - i - 1][j].r = 255;
                    img[rows - i - 1][j].g = 255;
                    img[rows - i - 1][j].b = 255;
                }
            }
        }
    }

}
