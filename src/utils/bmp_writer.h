#ifndef BMP_WRITER_H
#define BMP_WRITER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple BMP writer for screenshot functionality
// This is a minimal implementation that writes 24-bit BMP files

#pragma pack(push, 1)
typedef struct {
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
} BMPFileHeader;

typedef struct {
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} BMPInfoHeader;
#pragma pack(pop)

/**
 * Write RGB data to a BMP file
 * @param filename Output filename
 * @param width Image width
 * @param height Image height
 * @param data RGB data (3 bytes per pixel, row-major order)
 * @return 0 on success, -1 on failure
 */
int write_bmp(const char *filename, int width, int height, const unsigned char *data);

#endif // BMP_WRITER_H