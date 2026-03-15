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
    if (argc < 2) {
        std::cerr << "Usage : " << argv[0] << " <path_to_image>" << std::endl;
        return 1;
    }

    int nbIterations = DEFAULT_NB_ITERATIONS;
    if (argc == 3) {
        nbIterations = std::stoi(argv[2]);
    }

    Image image;
    LoadImage(image, argv[1]);
    std::vector<uint8_t> imageVec;
    VectorizeImage(image, imageVec);

    std::cout << "[INFO] Starting benchmark with " << nbIterations << " iterations..." << std::endl;

    auto startScalar = std::chrono::high_resolution_clock::now();
    uint32_t resScalar;
    for(int i = 0; i < nbIterations; i++)
        resScalar = WeylDiscrepancy(imageVec, image.Width, image.Height);
    auto endScalar = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> timeScalar = endScalar - startScalar;

    auto startAVX = std::chrono::high_resolution_clock::now();
    uint32_t resAVX;
    for(int i = 0; i < nbIterations; i++)
        resAVX = WeylDiscrepancyAVX(imageVec, image.Width, image.Height);
    auto endAVX = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> timeAVX = endAVX - startAVX;

    std::cout << "----------------------------------------\n";
    std::cout << "Results Scalar version : " << resScalar << " | Time : " << timeScalar.count() << " ms"
        << " | Average : " << timeScalar.count() / nbIterations << " ms\n";
    std::cout << "Results AVX2   version : " << resAVX << " | Time : " << timeAVX.count() << " ms"
        << " | Average : " << timeAVX.count() / nbIterations << " ms\n";
    std::cout << "----------------------------------------\n";
    
    if (resScalar == resAVX) {
        std::cout << "[SUCESS] Both results give the same result !\n";
        std::cout << "[BILAN] Acceleration AVX2 : x" << (timeScalar.count() / timeAVX.count()) << "\n";
    } else {
        std::cerr << "[ERREUR] Results are differents. Somthing wrong happend in the algorithm.\n";
    }

    stbi_image_free(image.Image);
    return 0;
}