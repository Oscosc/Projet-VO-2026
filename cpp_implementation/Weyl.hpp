#pragma once

#include <string>

struct Image {
    unsigned char *Image;
    int Width;
    int Height;
    int Channels;
};

void LoadImage(Image& image, std::string path);

uint32_t WeylDiscrepancy(const std::vector<uint8_t>& image, int width, int height);