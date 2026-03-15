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
    std::vector<uint8_t> Img;
    int Width;
    int Height;
    int Channels;

    Image() {
        Img = std::vector<uint8_t>();
    }
};

void LoadImage(Image& image, std::string path);

uint32_t WeylDiscrepancy(const Image& image);

uint32_t WeylDiscrepancyAVX(const Image& image);

uint32_t hmin_epi32(__m256i v);

uint32_t hmax_epi32(__m256i v);