#include <chrono>
#include "main.h"
#include "bmp_io.h"
#include <arm_neon.h>

const unsigned char greyLvl = 50;

int REPEAT = 10;

uint8x16_t binLevel = {
        greyLvl, greyLvl, greyLvl, greyLvl, greyLvl, greyLvl, greyLvl, greyLvl,
        greyLvl, greyLvl, greyLvl, greyLvl, greyLvl, greyLvl, greyLvl, greyLvl
};

uint8x16_t ones = vdupq_n_u8(0), twos = vdupq_n_u8(255); // the conditional branches: if condition is true returns 0, else returns 255

void binaryV1(RGBQUAD *rgbInfo, unsigned int pixelCount) {
    for (unsigned int i = 0; i < pixelCount; i++) {
        RGBQUAD &pixel = rgbInfo[i];
        if (pixel < greyLvl) {
            pixel = 0;
        } else {
            pixel= 255;
        }
    }
}

void binaryV2(RGBQUAD *rgbInfo, unsigned int pixelCount) {
    for (unsigned int i = 0; i < pixelCount; i += 16) {
        uint8x16_t rColor = vld1q_u8(rgbInfo + i);

        uint8x16_t mask = vcltq_u8(rColor, binLevel); // rColor < binLevel
        uint8x16_t v3 = vbslq_u8(mask, ones, twos);  // will select first if mask 0, second if mask 1

        vst1q_u8(rgbInfo + i, v3);
    }
}

RGBQUAD *bin1_time(BMP_io &bmpIo, double &time) {
    RGBQUAD *rgbInfo = bmpIo.readBMP();
    BITMAPINFOHEADER fileInfoHeader = bmpIo.getFileInfoHeader();
    unsigned int pixelCount = fileInfoHeader.biHeight * fileInfoHeader.biWidth;

    auto b1_t1 = std::chrono::system_clock::now();
    binaryV1(rgbInfo, pixelCount);
    auto b1_t2 = std::chrono::system_clock::now();

    std::chrono::duration<double, std::milli> ms_double = b1_t2 - b1_t1;
    (*time) = ms_double.count();
    std::cout << "BIN 1: REPEAT = " << *time << " ms" << std::endl;
    return rgbInfo;
}

RGBQUAD *bin2_time(BMP_io &bmpIo, double &time) {
    RGBQUAD *rgbInfo = bmpIo.readBMP();
    BITMAPINFOHEADER fileInfoHeader = bmpIo.getFileInfoHeader();
    unsigned int pixelCount = fileInfoHeader.biHeight * fileInfoHeader.biWidth;
    auto b2_t1 = std::chrono::system_clock::now();
    binaryV2(rgbInfo, pixelCount);
    auto b2_t2 = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> ms_double = b2_t2 - b2_t1;
    (*time) = ms_double.count();
    std::cout << "BIN 2: REPEAT = " << *time << " ms" << std::endl;
    return rgbInfo;
}

int main(int argc, char *argv[]) {
    const char *inputFileName = "./files/inputG.bmp";
    const char *outputFileName1 = "./output.bmp";
    const char *outputFileName2 = "./outputNeon.bmp";

    BMP_io bmpIo = BMP_io(inputFileName, outputFileName1);

    bmpIo.readBMP();
    BITMAPINFOHEADER fileInfoHeader = bmpIo.getFileInfoHeader();
    std::cout << "PIX w= " << fileInfoHeader.biWidth << " h=" << fileInfoHeader.biHeight << std::endl;

    //бинаризация 1

    RGBQUAD *rgbInfo1;

    double times_b1;
    for (int i = 0; i < REPEAT; i++) {
        double ti = 0;
        rgbInfo1 = bin1_time(bmpIo, &(ti));
        times_b1 += ti;
    }
    times_b1 /= REPEAT;
    std::cout << "BIN 1: MEAN = " << times_b1 << " ms" << std::endl;

    bmpIo.writeBMP(rgbInfo1);

    //бинаризация 2
    bmpIo.setOutputFileName(outputFileName2);

    RGBQUAD *rgbInfo2;
    double times_b2;
    for (int i = 0; i < REPEAT; i++) {
        double ti = 0;
        rgbInfo2 = bin2_time(bmpIo, &(ti));
        times_b2 += ti;
    }
    times_b2 /= REPEAT;
    std::cout << "BIN 2: MEAN = " << times_b2 << " ms" << std::endl;

    //запись
    bmpIo.writeBMP(rgbInfo2);


    return 0;
}


