#include "Weyl.hpp"
#include <iostream>
#include <vector>
#include <chrono> // Pour le chronométrage
#include <string>

#define DEFAULT_MAX_DISPARITY 60;
#define DEFAULT_PATCH_SIZE 9;

int main(int argc, char** argv) 
{
    if (argc < 3 || argc > 7) {
        std::cerr << "Usage : " << argv[0] << " <path_to_left_image> <path_to_right_image> "
            << "<optional:patch_size> <optional:max_disparity> "
            << "<optional:path_to_left_output> <optional:path_to_right_output>" << std::endl;
        return 1;
    }

    const int patchSize = (argc >= 4) ? std::stoi(argv[3]) : DEFAULT_PATCH_SIZE;
    const int maxDisparity = (argc >= 5) ? std::stoi(argv[4]) : DEFAULT_MAX_DISPARITY;
    const char* outputPathLeft = (argc >= 6) ? argv[5] : "left_disparity_output.png";
    const char* outputPathRight = (argc == 7) ? argv[6] : "right_disparity_output.png";

    Weyl::Image::Image imageLeft, imageRight, outputLeft, outputRight;
    std::vector<uint32_t> rawDispLeft, rawDispRight;

    Weyl::Image::LoadImage(imageLeft, argv[1]);
    Weyl::Image::LoadImage(imageRight, argv[2]);

    std::cout << "[INFO] Dense corresponding :\n";
    std::cout << "  -> Image left:    " << imageLeft.Width << "x" << imageLeft.Height << std::endl;
    std::cout << "  -> Image right:   " << imageRight.Width << "x" << imageRight.Height << std::endl;
    std::cout << "  -> Max disparity: " << maxDisparity << std::endl;
    std::cout << "  -> Patch size:    " << patchSize << std::endl;
    std::cout << "[INFO] Starting Dense corresponding...\n";

    auto start = std::chrono::high_resolution_clock::now();
    Weyl::DenseCorresponding(imageLeft, imageRight, rawDispLeft, rawDispRight, patchSize, maxDisparity);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> execTime = end - start;
    std::cout << "[PERFORMANCE] Execution time : " << execTime.count() << " ms\n";

    outputLeft.Width = imageLeft.Width; outputLeft.Height = imageLeft.Height; outputLeft.Channels = imageLeft.Channels;
    outputRight.Width = imageRight.Width; outputRight.Height = imageRight.Height; outputRight.Channels = imageRight.Channels;
    Weyl::Image::NormalizeImageData(rawDispLeft, outputLeft);
    Weyl::Image::NormalizeImageData(rawDispRight, outputRight);
    Weyl::Image::WriteImage(outputLeft, outputPathLeft);
    Weyl::Image::WriteImage(outputRight, outputPathRight);

    return 0;
}