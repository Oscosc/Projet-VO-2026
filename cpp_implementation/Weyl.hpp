#pragma once

#include "stb_image.h"
#include "stb_image_write.h"

#include <cmath>
#include <iostream>
#include <cstdint> // For uint32_t, used for images integrals
#include <vector>
#include <algorithm> // For std::min and std::max
#include <string>
#include <immintrin.h> // For AVX optimizations

namespace Weyl
{
    namespace Image
    {
        struct Image {
            std::vector<uint8_t> Img;
            int Width;
            int Height;
            int Channels;

            Image() { Img = std::vector<uint8_t>(); }
            Image(int width, int height) : Width(width), Height(height) {
                Img = std::vector<uint8_t>(width * height);
            }
        };

        void LoadImage(Image& image, std::string path);

        void WriteImage(Image& image, std::string path);

        void NormalizeImageData(const std::vector<uint32_t>& data,
            const uint32_t min, const uint32_t max, Image& image);
        
        void NormalizeImageData(const std::vector<uint32_t>& data, Image& image);

        void ScaleImageData(Image& image, const uint8_t scaleFactor);
    }

    namespace Core
    {
        uint32_t WeylDiscrepancy(const Image::Image& image);

        uint32_t WeylDiscrepancyAVX(const Image::Image& image);

        uint32_t hmin_epi32(__m256i v);

        uint32_t hmax_epi32(__m256i v);
    }

    int PatchMatching(const Image::Image& image, const Image::Image& pattern, Image::Image& disparityMap);

    void DenseCorresponding(const Image::Image& imgLeft, const Image::Image& imgRight, Image::Image& dispLeft,
        Image::Image& dispRight, const int patchSize, const int maxDisparity);

    void LeftRightConsistency(const Image::Image& dispLeftIn, const Image::Image& dispRightIn,
        Image::Image& dispLeftOut, Image::Image& dispRightOut, const int threshold = 1);
}