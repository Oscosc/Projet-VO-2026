#include "Weyl.hpp"
#include <iostream>
#include <vector>
#include <chrono> // Pour le chronométrage
#include <string>

#define DEFAULT_NB_ITERATIONS 1000

int main(int argc, char** argv) 
{
    if(argc < 2 || argc > 3) {
        std::cerr << "Usage : " << argv[0] << " <path_to_image> <optional:nb_iterations>" << std::endl;
        return 1;
    }

    int nbIterations = DEFAULT_NB_ITERATIONS;
    if(argc == 3) {
        nbIterations = std::stoi(argv[2]);
    }

    Image image;
    LoadImage(image, argv[1]);

    std::cout << "[INFO] Starting benchmark with " << nbIterations << " iterations..." << std::endl;

    auto startScalar = std::chrono::high_resolution_clock::now();
    uint32_t resScalar = 0;
    for(int i = 0; i < nbIterations; i++)
        resScalar = WeylDiscrepancy(image);
    auto endScalar = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> timeScalar = endScalar - startScalar;

    auto startAVX = std::chrono::high_resolution_clock::now();
    uint32_t resAVX = 0;
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

    return 0;
}