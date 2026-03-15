#pragma once

#include "stb_image.h"
#include "stb_image_write.h"

#include <iostream>
#include <cstdint> // For uint32_t, used for images integrals
#include <vector>
#include <algorithm> // For std::min and std::max
#include <string>

#include <immintrin.h> // For AVX optimizations

struct Image {
    unsigned char *Image;
    int Width;
    int Height;
    int Channels;
};

void LoadImage(Image& image, std::string path);

void VectorizeImage(const Image& image, std::vector<uint8_t>& vectorImage);

uint32_t WeylDiscrepancy(const std::vector<uint8_t>& image, int width, int height);

uint32_t WeylDiscrepancyAVX(const std::vector<uint8_t>& image, int width, int height);

uint32_t hmin_epi32(__m256i v);

uint32_t hmax_epi32(__m256i v);