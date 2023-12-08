//
// Created by maxim on 12/6/23.
//

#ifndef MVC_LAB2_BMP_IO_H
#define MVC_LAB2_BMP_IO_H

#include <fstream>
#include <iostream>
#include "bmp.h"

class BMP_io{
private:
    const char *inputFileName;
    const char *outputFileName;
    std::ifstream inputFileStream;
    std::ofstream outputFileStream;
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER fileInfoHeader;
//    RGBQUAD **rgbInfo;
    RGBQUAD *rgbInfo;
    int colorsCount;
    int bitsOnColor;
    int maskValue;
    int linePadding;
public:
    BMP_io(const char *input, const char *output);
//    RGBQUAD **readBMP();
    RGBQUAD *readBMP();
    void writeBMP(RGBQUAD *rgbInfo);
//    void writeBMP(RGBQUAD **rgbInfo);
    BITMAPINFOHEADER getFileInfoHeader();
    void setOutputFileName(const char *fileName);
private:
    int open_input_file();
    int open_output_file();
    int read_file_header();
    void write_file_header();
    int read_fileinfo_header();
    int read_rgbInfo();
    void write_rgbInfo(RGBQUAD *rgbInfoN);
//    void write_rgbInfo(RGBQUAD **rgbInfoN);
    int close_input_file();
    int close_output_file();
    static int getMaskPadding(unsigned int mask);
    static unsigned int bitInsert(unsigned char color, unsigned int mask);
    static unsigned char bitExtract(unsigned int byte, unsigned int mask);
};

// read bytes
template <typename Type>
void read(std::ifstream &fp, Type &result, std::size_t size) {
    fp.read(reinterpret_cast<char*>(&result), size);
}

#endif //MVC_LAB2_BMP_IO_H
