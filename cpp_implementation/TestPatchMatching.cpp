#include "Weyl.hpp"
#include <iostream>
#include <vector>
#include <chrono> // Pour le chronométrage
#include <string>

int main(int argc, char** argv) 
{
    if (argc < 3) {
        std::cerr << "Usage : " << argv[0] << " <path_to_image> <path_to_patch> <optional:path_to_output>" << std::endl;
        return 1;
    }

    const char* outputPath = (argc == 4) ? argv[3] : "patch_matching_output.png";

    Weyl::Image::Image image, patch, output;

    Weyl::Image::LoadImage(image, argv[1]);
    Weyl::Image::LoadImage(patch, argv[2]);

    if (patch.Width > image.Width || patch.Height > image.Height) {
        std::cerr << "[ERREUR] Le patch est plus grand que l'image globale !" << std::endl;
        return 1;
    }

    std::cout << "[INFO] Patch matching : Image " << image.Width << "x" << image.Height << " | "
        << "Patch " << patch.Width << "x" << patch.Height << std::endl;
    std::cout << "[INFO] Starting patch matching...\n";
    auto start = std::chrono::high_resolution_clock::now();
    Weyl::PatchMatching(image, patch, output);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> execTime = end - start;
    std::cout << "[PERFORMANCE] Execution time : " << execTime.count() << " ms\n";

    Weyl::Image::WriteImage(output, outputPath);
    Weyl::Image::WriteImage(image, "gray_nuance_image.png");

    return 0;
}