#include "bmp_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int write_bmp(const char *filename, int width, int height, const unsigned char *data)
{
    if (!filename || !data || width <= 0 || height <= 0) {
        return -1;
    }

    FILE *file = fopen(filename, "wb");
    if (!file) {
        return -1;
    }

    // Calculate padding for each row (BMP rows must be aligned to 4 bytes)
    int row_padding = (4 - (width * 3) % 4) % 4;
    int row_size = width * 3 + row_padding;
    int image_size = row_size * height;

    // BMP file header
    BMPFileHeader file_header;
    file_header.bfType = 0x4D42; // "BM"
    file_header.bfSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + image_size;
    file_header.bfReserved1 = 0;
    file_header.bfReserved2 = 0;
    file_header.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

    // BMP info header
    BMPInfoHeader info_header;
    info_header.biSize = sizeof(BMPInfoHeader);
    info_header.biWidth = width;
    info_header.biHeight = height; // Negative height means top-to-bottom
    info_header.biPlanes = 1;
    info_header.biBitCount = 24; // 24-bit RGB
    info_header.biCompression = 0; // BI_RGB
    info_header.biSizeImage = image_size;
    info_header.biXPelsPerMeter = 0;
    info_header.biYPelsPerMeter = 0;
    info_header.biClrUsed = 0;
    info_header.biClrImportant = 0;

    // Write headers
    if (fwrite(&file_header, sizeof(BMPFileHeader), 1, file) != 1 ||
        fwrite(&info_header, sizeof(BMPInfoHeader), 1, file) != 1) {
        fclose(file);
        return -1;
    }

    // Write image data (BMP stores rows bottom-to-top, so we need to flip)
    unsigned char *row_buffer = (unsigned char *)malloc(row_size);
    if (!row_buffer) {
        fclose(file);
        return -1;
    }

    for (int y = height - 1; y >= 0; --y) {
        const unsigned char *src_row = data + (y * width * 3);
        unsigned char *dst_row = row_buffer;

        // Copy RGB data (BMP uses BGR order)
        for (int x = 0; x < width; ++x) {
            dst_row[0] = src_row[2]; // B
            dst_row[1] = src_row[1]; // G
            dst_row[2] = src_row[0]; // R
            dst_row += 3;
            src_row += 3;
        }

        // Add padding
        memset(dst_row, 0, row_padding);

        // Write row
        if (fwrite(row_buffer, row_size, 1, file) != 1) {
            free(row_buffer);
            fclose(file);
            return -1;
        }
    }

    free(row_buffer);
    fclose(file);
    return 0;
}