#include "bmp_io.h"


BMP_io::BMP_io(const char *input, const char *output) {
    inputFileName = input;
    outputFileName = output;
}

int BMP_io::getMaskPadding(unsigned int mask) {
    unsigned int maskBuffer = mask;
    int maskPadding = 0;

    while (!(maskBuffer & 1)) {
        maskBuffer >>= 1;
        maskPadding++;
    }

    return maskPadding;
}

unsigned int BMP_io::bitInsert(const unsigned char color, const unsigned int mask) {
    if (mask == 0) {
        return 0;
    }

    int maskPadding = getMaskPadding(mask);

    return (color << maskPadding) & mask;
}

unsigned char BMP_io::bitExtract(const unsigned int byte, const unsigned int mask) {
    if (mask == 0) {
        return 0;
    }

    // определение количества нулевых бит справа от маски
    int maskPadding = getMaskPadding(mask);

    // применение маски и смещение
    return (byte & mask) >> maskPadding;
}


RGBQUAD *BMP_io::readBMP() {
    if (!open_input_file()) {
        exit(1);
    }
    if (!read_file_header()) {
        exit(1);
    }
    if (!read_fileinfo_header()) {
        exit(1);
    }
    read_rgbInfo();
    close_input_file();
    return rgbInfo;
}


void BMP_io::writeBMP(RGBQUAD *rgbInfoN) {
    open_output_file();
    write_file_header();
    write_rgbInfo(rgbInfoN);
    close_output_file();
}

int BMP_io::open_input_file() {
    inputFileStream.open(inputFileName, std::ifstream::binary);
    if (!inputFileStream) {
        std::cout << "Error opening file '" << inputFileName << "'." << std::endl;
        return 0;
    }
    return 1;
}

int BMP_io::open_output_file() {
    outputFileStream.open(outputFileName, std::ios_base::binary);
    if (!outputFileStream) {
        std::cout << "Error opening file '" << outputFileName << "'." << std::endl;
        return 0;
    }
    return 1;
}

int BMP_io::read_file_header() {
    read(inputFileStream, fileHeader.bfType, sizeof(fileHeader.bfType));
    read(inputFileStream, fileHeader.bfSize, sizeof(fileHeader.bfSize));
    read(inputFileStream, fileHeader.bfReserved1, sizeof(fileHeader.bfReserved1));
    read(inputFileStream, fileHeader.bfReserved2, sizeof(fileHeader.bfReserved2));
    read(inputFileStream, fileHeader.bfOffBits, sizeof(fileHeader.bfOffBits));

    if (fileHeader.bfType != 0x4D42) {
        std::cout << "Error: '" << inputFileName << "' is not BMP file." << std::endl;
        return 0;
    }
    return 1;
}

void BMP_io::write_file_header() {
    outputFileStream.write((const char *) &fileHeader.bfType, sizeof(fileHeader.bfType));
    outputFileStream.write((const char *) &fileHeader.bfSize, sizeof(fileHeader.bfSize));
    outputFileStream.write((const char *) &fileHeader.bfReserved1, sizeof(fileHeader.bfReserved1));
    outputFileStream.write((const char *) &fileHeader.bfReserved2, sizeof(fileHeader.bfReserved2));
    outputFileStream.write((const char *) &fileHeader.bfOffBits, sizeof(fileHeader.bfOffBits));

    outputFileStream.write((const char *) &fileInfoHeader.biSize, sizeof(fileInfoHeader.biSize));

    outputFileStream.write((const char *) &fileInfoHeader.biWidth, sizeof(fileInfoHeader.biWidth));
    outputFileStream.write((const char *) &fileInfoHeader.biHeight, sizeof(fileInfoHeader.biHeight));
    outputFileStream.write((const char *) &fileInfoHeader.biPlanes, sizeof(fileInfoHeader.biPlanes));
    outputFileStream.write((const char *) &fileInfoHeader.biBitCount, sizeof(fileInfoHeader.biBitCount));

    // bmp v1
    if (fileInfoHeader.biSize >= 40) {
        outputFileStream.write((const char *) &fileInfoHeader.biCompression, sizeof(fileInfoHeader.biCompression));
        outputFileStream.write((const char *) &fileInfoHeader.biSizeImage, sizeof(fileInfoHeader.biSizeImage));
        outputFileStream.write((const char *) &fileInfoHeader.biXPelsPerMeter, sizeof(fileInfoHeader.biXPelsPerMeter));
        outputFileStream.write((const char *) &fileInfoHeader.biYPelsPerMeter, sizeof(fileInfoHeader.biYPelsPerMeter));
        outputFileStream.write((const char *) &fileInfoHeader.biClrUsed, sizeof(fileInfoHeader.biClrUsed));
        outputFileStream.write((const char *) &fileInfoHeader.biClrImportant, sizeof(fileInfoHeader.biClrImportant));
    }

    // bmp v2
    if (fileInfoHeader.biSize >= 52) {
        outputFileStream.write((const char *) &fileInfoHeader.biRedMask, sizeof(fileInfoHeader.biRedMask));
        outputFileStream.write((const char *) &fileInfoHeader.biGreenMask, sizeof(fileInfoHeader.biGreenMask));
        outputFileStream.write((const char *) &fileInfoHeader.biBlueMask, sizeof(fileInfoHeader.biBlueMask));
    }

    // bmp v3
    if (fileInfoHeader.biSize >= 56) {
        outputFileStream.write((const char *) &fileInfoHeader.biAlphaMask, sizeof(fileInfoHeader.biAlphaMask));
    }

    // bmp v4
    if (fileInfoHeader.biSize >= 108) {
        outputFileStream.write((const char *) &fileInfoHeader.biCSType, sizeof(fileInfoHeader.biCSType));
        outputFileStream.write((const char *) &fileInfoHeader.biEndpoints, sizeof(fileInfoHeader.biEndpoints));
        outputFileStream.write((const char *) &fileInfoHeader.biGammaRed, sizeof(fileInfoHeader.biGammaRed));
        outputFileStream.write((const char *) &fileInfoHeader.biGammaGreen, sizeof(fileInfoHeader.biGammaGreen));
        outputFileStream.write((const char *) &fileInfoHeader.biGammaBlue, sizeof(fileInfoHeader.biGammaBlue));
    }

    // bmp v5
    if (fileInfoHeader.biSize >= 124) {
        outputFileStream.write((const char *) &fileInfoHeader.biIntent, sizeof(fileInfoHeader.biIntent));
        outputFileStream.write((const char *) &fileInfoHeader.biProfileData, sizeof(fileInfoHeader.biProfileData));
        outputFileStream.write((const char *) &fileInfoHeader.biProfileSize, sizeof(fileInfoHeader.biProfileSize));
        outputFileStream.write((const char *) &fileInfoHeader.biReserved, sizeof(fileInfoHeader.biReserved));
    }
}

int BMP_io::read_fileinfo_header() {
    read(inputFileStream, fileInfoHeader.biSize, sizeof(fileInfoHeader.biSize));

    // bmp core
    if (fileInfoHeader.biSize >= 12) {
        read(inputFileStream, fileInfoHeader.biWidth, sizeof(fileInfoHeader.biWidth));
        read(inputFileStream, fileInfoHeader.biHeight, sizeof(fileInfoHeader.biHeight));
        read(inputFileStream, fileInfoHeader.biPlanes, sizeof(fileInfoHeader.biPlanes));
        read(inputFileStream, fileInfoHeader.biBitCount, sizeof(fileInfoHeader.biBitCount));
    }

    // получаем информацию о битности
    colorsCount = fileInfoHeader.biBitCount >> 3;
    if (colorsCount < 3) {
        colorsCount = 3;
    }

    bitsOnColor = fileInfoHeader.biBitCount / colorsCount;
    maskValue = (1 << bitsOnColor) - 1;

    // bmp v1
    if (fileInfoHeader.biSize >= 40) {
        read(inputFileStream, fileInfoHeader.biCompression, sizeof(fileInfoHeader.biCompression));
        read(inputFileStream, fileInfoHeader.biSizeImage, sizeof(fileInfoHeader.biSizeImage));
        read(inputFileStream, fileInfoHeader.biXPelsPerMeter, sizeof(fileInfoHeader.biXPelsPerMeter));
        read(inputFileStream, fileInfoHeader.biYPelsPerMeter, sizeof(fileInfoHeader.biYPelsPerMeter));
        read(inputFileStream, fileInfoHeader.biClrUsed, sizeof(fileInfoHeader.biClrUsed));
        read(inputFileStream, fileInfoHeader.biClrImportant, sizeof(fileInfoHeader.biClrImportant));
    }

    // bmp v2
    fileInfoHeader.biRedMask = 0;
    fileInfoHeader.biGreenMask = 0;
    fileInfoHeader.biBlueMask = 0;

    if (fileInfoHeader.biSize >= 52) {
        read(inputFileStream, fileInfoHeader.biRedMask, sizeof(fileInfoHeader.biRedMask));
        read(inputFileStream, fileInfoHeader.biGreenMask, sizeof(fileInfoHeader.biGreenMask));
        read(inputFileStream, fileInfoHeader.biBlueMask, sizeof(fileInfoHeader.biBlueMask));
    }

    // если маска не задана, то ставим маску по умолчанию
    if (fileInfoHeader.biRedMask == 0 || fileInfoHeader.biGreenMask == 0 || fileInfoHeader.biBlueMask == 0) {
        fileInfoHeader.biRedMask = maskValue << (bitsOnColor * 2);
        fileInfoHeader.biGreenMask = maskValue << bitsOnColor;
        fileInfoHeader.biBlueMask = maskValue;
    }

    // bmp v3
    if (fileInfoHeader.biSize >= 56) {
        read(inputFileStream, fileInfoHeader.biAlphaMask, sizeof(fileInfoHeader.biAlphaMask));
    } else {
        fileInfoHeader.biAlphaMask = maskValue << (bitsOnColor * 3);
    }

    // bmp v4
    if (fileInfoHeader.biSize >= 108) {
        read(inputFileStream, fileInfoHeader.biCSType, sizeof(fileInfoHeader.biCSType));
        read(inputFileStream, fileInfoHeader.biEndpoints, sizeof(fileInfoHeader.biEndpoints));
        read(inputFileStream, fileInfoHeader.biGammaRed, sizeof(fileInfoHeader.biGammaRed));
        read(inputFileStream, fileInfoHeader.biGammaGreen, sizeof(fileInfoHeader.biGammaGreen));
        read(inputFileStream, fileInfoHeader.biGammaBlue, sizeof(fileInfoHeader.biGammaBlue));
    }

    // bmp v5
    if (fileInfoHeader.biSize >= 124) {
        read(inputFileStream, fileInfoHeader.biIntent, sizeof(fileInfoHeader.biIntent));
        read(inputFileStream, fileInfoHeader.biProfileData, sizeof(fileInfoHeader.biProfileData));
        read(inputFileStream, fileInfoHeader.biProfileSize, sizeof(fileInfoHeader.biProfileSize));
        read(inputFileStream, fileInfoHeader.biReserved, sizeof(fileInfoHeader.biReserved));
    }

    // проверка на поддерку этой версии формата
    if (fileInfoHeader.biSize != 12 && fileInfoHeader.biSize != 40 && fileInfoHeader.biSize != 52 &&
        fileInfoHeader.biSize != 56 && fileInfoHeader.biSize != 108 && fileInfoHeader.biSize != 124) {
        std::cout << "Error: Unsupported BMP format." << std::endl;
        return 0;
    }

    if (fileInfoHeader.biBitCount != 16 && fileInfoHeader.biBitCount != 24 && fileInfoHeader.biBitCount != 32) {
        std::cout << fileInfoHeader.biBitCount << std::endl;
        std::cout << "Error: Unsupported BMP bit count." << std::endl;
        return 0;
    }

    if (fileInfoHeader.biCompression != 0 && fileInfoHeader.biCompression != 3) {
        std::cout << "Error: Unsupported BMP compression." << std::endl;
        return 0;
    }
    return 1;
}

int BMP_io::read_rgbInfo() {
    rgbInfo = new RGBQUAD[fileInfoHeader.biHeight * fileInfoHeader.biWidth];
//    rgbInfo = new RGBQUAD *[fileInfoHeader.biHeight];

//    for (unsigned int i = 0; i < fileInfoHeader.biHeight; i++) {
//        rgbInfo[i] = new RGBQUAD[fileInfoHeader.biWidth];
//    }

    // определение размера отступа в конце каждой строки
    linePadding = ((fileInfoHeader.biWidth * (fileInfoHeader.biBitCount / 8)) % 4) & 3;

    // чтение
    unsigned int bufer;


    for (unsigned int i = 0; i < fileInfoHeader.biHeight; i++) {
        for (unsigned int j = 0; j < fileInfoHeader.biWidth; j++) {
            read(inputFileStream, bufer, fileInfoHeader.biBitCount / 8);
            RGBQUAD &pixel = rgbInfo[i * fileInfoHeader.biWidth + j];
            pixel = bitExtract(bufer, fileInfoHeader.biRedMask);
//            pixel.rgbGreen = bitExtract(bufer, fileInfoHeader.biGreenMask);
//            pixel.rgbBlue = bitExtract(bufer, fileInfoHeader.biBlueMask);
//            pixel.rgbReserved = bitExtract(bufer, fileInfoHeader.biAlphaMask);
//            rgbInfo[i][j].rgbRed = bitExtract(bufer, fileInfoHeader.biRedMask);
//            rgbInfo[i][j].rgbGreen = bitExtract(bufer, fileInfoHeader.biGreenMask);
//            rgbInfo[i][j].rgbBlue = bitExtract(bufer, fileInfoHeader.biBlueMask);
//            rgbInfo[i][j].rgbReserved = bitExtract(bufer, fileInfoHeader.biAlphaMask);
        }
        inputFileStream.seekg(linePadding, std::ios_base::cur);
    }
    return 1;
}

void BMP_io::write_rgbInfo(RGBQUAD *rgbInfoN) {
    for (unsigned int i = 0; i < fileInfoHeader.biHeight; i++) {
        for (unsigned int j = 0; j < fileInfoHeader.biWidth; j++) {
            unsigned int buffer = 0;
            RGBQUAD &pixel = rgbInfoN[i * fileInfoHeader.biWidth + j];
            buffer += bitInsert(pixel, fileInfoHeader.biRedMask);
            buffer += bitInsert(pixel, fileInfoHeader.biGreenMask);
            buffer += bitInsert(pixel, fileInfoHeader.biBlueMask);
            //buffer += bitInsert(pixel.rgbReserved, fileInfoHeader.biAlphaMask);

            outputFileStream.write((const char *) &buffer, fileInfoHeader.biBitCount / 8);
        }
        outputFileStream.seekp(linePadding, std::ios_base::cur);
    }
}

int BMP_io::close_input_file() {
    inputFileStream.close();
    return 1;
}

int BMP_io::close_output_file() {
    outputFileStream.close();
    return 1;
}


BITMAPINFOHEADER BMP_io::getFileInfoHeader() {
    return fileInfoHeader;
}

void BMP_io::setOutputFileName(const char *fileName) {
    outputFileName = fileName;
}


