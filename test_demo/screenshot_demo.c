#include <stdio.h>
#include <stdlib.h>
#include "../src/utils/bmp_writer.h"

/**
 * Simple demo program to test BMP writer functionality
 * This creates a test image and saves it as a BMP file
 */
int main(int argc, char *argv[])
{
    printf("CQAnalyzer Screenshot Demo\n");
    printf("==========================\n\n");

    // Create a simple test image (256x256 RGB gradient)
    const int width = 256;
    const int height = 256;
    unsigned char *image_data = (unsigned char *)malloc(width * height * 3);

    if (!image_data) {
        printf("Failed to allocate memory for test image\n");
        return 1;
    }

    // Generate a colorful gradient pattern
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * 3;

            // Red channel: horizontal gradient
            image_data[index] = (unsigned char)(x * 255 / width);

            // Green channel: vertical gradient
            image_data[index + 1] = (unsigned char)(y * 255 / height);

            // Blue channel: diagonal gradient
            image_data[index + 2] = (unsigned char)((x + y) * 255 / (width + height));
        }
    }

    // Save the image as BMP
    const char *filename = "demo_screenshot.bmp";
    printf("Creating test image (%dx%d)...\n", width, height);

    if (write_bmp(filename, width, height, image_data) == 0) {
        printf("✓ Successfully saved screenshot to: %s\n", filename);
        printf("✓ Image dimensions: %dx%d pixels\n", width, height);
        printf("✓ File format: BMP (24-bit RGB)\n");
    } else {
        printf("✗ Failed to save screenshot\n");
    }

    // Clean up
    free(image_data);

    printf("\nDemo completed!\n");
    printf("In the actual CQAnalyzer application:\n");
    printf("- Press 'S' to take a screenshot of the 3D visualization\n");
    printf("- Press 'V' to start/stop video recording\n");
    printf("- Screenshots are saved as BMP files\n");
    printf("- Video frames are saved as numbered BMP files\n");

    return 0;
}