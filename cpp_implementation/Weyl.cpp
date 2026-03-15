#include "Weyl.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <cstdint> // For uint32_t, used for images integrals
#include <vector>
#include <algorithm> // For std::min and std::max


void LoadImage(Image &image, std::string path)
{
    image.Image = stbi_load(path.c_str(), &image.Width, &image.Height, &image.Channels, 0);
    if(image.Image == NULL) {
        std::cout << "[ERROR] Unable to load the image." << std::endl;
        exit(1);
    }
}

uint32_t WeylDiscrepancy(const std::vector<uint8_t>& image, int width, int height)
{
    // Using a linearized image for memory alignment
    std::vector<uint32_t> integralImage(width * height);

    uint32_t globalMinP1 = UINT32_MAX;
    uint32_t globalMaxP1 = 0;
    uint32_t localMinP1 = image[0];
    uint32_t localMaxP1 = image[0];

    uint32_t globalMinP2 = UINT32_MAX;
    uint32_t globalMaxP2 = 0;

    uint32_t globalMinP3 = UINT32_MAX;
    uint32_t globalMaxP3 = 0;

    // P4 values are int32 and not uint32 because they can be negative
    int32_t globalMinP4 = INT32_MAX;
    int32_t globalMaxP4 = 0;


    // Pass 1 : P1 and P2 -------------------------------------------------------------------------

    uint32_t rowSum = image[0];

    // First (top-left) cell
    integralImage[0] = image[0];

    // First loop for topmost row
    for(int x = 1; x < width; x++)
    {
        rowSum += image[x];
        integralImage[x] = rowSum;

        localMinP1 = std::min(localMinP1, integralImage[x]);
        localMaxP1 = std::max(localMaxP1, integralImage[x]);
    }

    globalMinP1 = std::min(globalMinP1, localMinP1);
    globalMaxP1 = std::max(globalMaxP1, localMaxP1);

    unsigned int c = width - 1;
    globalMinP2 = std::min(globalMinP2, std::min(integralImage[c], integralImage[c] - localMaxP1));
    globalMaxP2 = std::max(globalMaxP2, std::max(integralImage[c], integralImage[c] - localMinP1));

    // Main loop of first pass
    for(int y = 1; y < height; y++)
    {
        localMinP1 = UINT32_MAX;
        localMaxP1 = 0;
        rowSum = 0;

        for(int x = 0; x < width; x++)
        {
            unsigned int current = y * width + x;
            rowSum += image[current];
            integralImage[current] = integralImage[current - width] + rowSum;

            localMinP1 = std::min(localMinP1, integralImage[current]);
            localMaxP1 = std::max(localMaxP1, integralImage[current]);
        }

        globalMinP1 = std::min(globalMinP1, localMinP1);
        globalMaxP1 = std::max(globalMaxP1, localMaxP1);

        unsigned int c = (y + 1) * width - 1;
        globalMinP2 = std::min(globalMinP2, std::min(integralImage[c], integralImage[c] - localMaxP1));
        globalMaxP2 = std::max(globalMaxP2, std::max(integralImage[c], integralImage[c] - localMinP1));
    }


    // Pass 2 : P3 and P4 -------------------------------------------------------------------------
    int32_t integralValueP4;
    
    localMinP1 = image[0];
    localMaxP1 = image[0];

    // First loop for leftmost column
    for(int y = 0; y < height; y++)
    {
        unsigned int current = y * width;
        unsigned int maxWidth = (y + 1) * width - 1;
        unsigned int maxHeight = (height - 1) * width;

        // P3
        localMinP1 = std::min(localMinP1, integralImage[current]);
        localMaxP1 = std::max(localMaxP1, integralImage[current]);

        // P4
        integralValueP4 = integralImage[y * width] - integralImage[maxWidth] - integralImage[maxHeight];
        globalMinP4 = std::min(globalMinP4, integralValueP4);
        globalMaxP4 = std::max(globalMaxP4, integralValueP4);
    }

    unsigned int c = (height - 1) * width;
    globalMinP3 = std::min(globalMinP3, std::min(integralImage[c], integralImage[c] - localMaxP1));
    globalMaxP3 = std::max(globalMaxP3, std::max(integralImage[c], integralImage[c] - localMinP1));

    // Main loop of second pass
    for(int x = 1; x < width; x++)
    {
        localMinP1 = UINT32_MAX;
        localMaxP1 = 0;

        unsigned int maxHeight = (height - 1) * width + x;

        for(int y = 0; y < height; y++)
        {
            unsigned int current = y * width + x;
            unsigned int maxWidth = (y + 1) * width - 1;

            // P3
            localMinP1 = std::min(localMinP1, integralImage[current]);
            localMaxP1 = std::max(localMaxP1, integralImage[current]);

            // P4
            integralValueP4 = integralImage[current] - integralImage[maxWidth] - integralImage[maxHeight];
            globalMinP4 = std::min(globalMinP4, integralValueP4);
            globalMaxP4 = std::max(globalMaxP4, integralValueP4);
        }

        unsigned int c = (height - 1) * width + x;
        globalMinP3 = std::min(globalMinP3, std::min(integralImage[c], integralImage[c] - localMaxP1));
        globalMaxP3 = std::max(globalMaxP3, std::max(integralImage[c], integralImage[c] - localMinP1));
    }


    // Return the maximum of the differences ------------------------------------------------------

    return std::max({
        globalMaxP1 - globalMinP1,
        globalMaxP2 - globalMinP2,
        globalMaxP3 - globalMinP3,
        static_cast<uint32_t>(globalMaxP4 - globalMinP4)
    });
}