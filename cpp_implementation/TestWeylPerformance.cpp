// compil : g++ TestWeylPerformance.cpp Weyl.cpp -O3 -mavx2 -o weyl_benchmark
// usage  : ./weyl_benchmark ../datasets/tests_images/tree.png <optional:nb_iterations>

#include "Weyl.hpp"
#include <iostream>
#include <vector>
#include <chrono> // Pour le chronométrage
#include <string>

#define DEFAULT_NB_ITERATIONS 1000

int main(int argc, char** argv) 
{
    if (argc < 3) {
        std::cerr << "Usage : " << argv[0] << " <path_to_image> <path_to_patch> <optional:path_to_output>" << std::endl;
        return 1;
    }

    const char* outputPath = (argc == 4) ? argv[3] : "patch_matching_output.png";

    Image image, patch, output;

    LoadImage(image, argv[1]);
    LoadImage(patch, argv[2]);

    if (patch.Width > image.Width || patch.Height > image.Height) {
        std::cerr << "[ERREUR] Le patch est plus grand que l'image globale !" << std::endl;
        return 1;
    }

    std::cout << "[INFO] Patch matching : Image " << image.Width << "x" << image.Height << " | "
        << "Patch " << patch.Width << "x" << patch.Height << std::endl;
    std::cout << "[INFO] Starting patch matching...\n";
    auto start = std::chrono::high_resolution_clock::now();
    PatchMatching(image, patch, output);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> execTime = end - start;
    std::cout << "[PERFORMANCE] Execution time : " << execTime.count() << " ms\n";

    WriteImage(output, outputPath);

    return 0;
}

/*
    std::cout << "[INFO] Starting benchmark with " << nbIterations << " iterations..." << std::endl;

    auto startScalar = std::chrono::high_resolution_clock::now();
    uint32_t resScalar;
    for(int i = 0; i < nbIterations; i++)
        resScalar = WeylDiscrepancy(image);
    auto endScalar = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> timeScalar = endScalar - startScalar;

    auto startAVX = std::chrono::high_resolution_clock::now();
    uint32_t resAVX;
    for(int i = 0; i < nbIterations; i++)
        resAVX = WeylDiscrepancyAVX(image);
    auto endAVX = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> timeAVX = endAVX - startAVX;

    std::cout << "----------------------------------------\n";
    std::cout << "Result Scalar version : " << resScalar << " | Time : " << timeScalar.count() << " ms"
        << " | Average/image : " << timeScalar.count() / nbIterations << " ms"
        << " | Average/pixel : " << timeScalar.count() / (nbIterations * image.Height * image.Width) << " ms\n";
    std::cout << "Result AVX2   version : " << resAVX << " | Time : " << timeAVX.count() << " ms"
        << " | Average/image : " << timeAVX.count() / nbIterations << " ms"
        << " | Average/pixel : " << timeAVX.count() / (nbIterations * image.Height * image.Width) << " ms\n";
    std::cout << "----------------------------------------\n";
    
    if (resScalar == resAVX) {
        std::cout << "[SUCESS] Both algorithms give the same result !\n";
        std::cout << "[PERFORMANCE] Acceleration AVX2 : x" << (timeScalar.count() / timeAVX.count()) << "\n";
    } else {
        std::cerr << "[ERROR] Results are differents. Somthing wrong happend in the algorithm.\n";
    }
*/