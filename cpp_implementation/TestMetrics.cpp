#include "Weyl.hpp"
#include <iostream>
#include <vector>
#include <string>


#define DEFAULT_BPR_TOLERANCE 1


int main(int argc, char** argv) 
{
    if (argc < 3) {
        std::cerr << "Usage : " << argv[0] << " <path_to_reference> <path_to_estimation> <optional:BPR_tolerance>" << std::endl;
        return 1;
    }

    const int bprTolerance = (argc == 4) ? std::stoi(argv[3]) : DEFAULT_BPR_TOLERANCE;

    Weyl::Image::Image ref, estim;
    Weyl::Image::LoadImage(ref, argv[1]);
    Weyl::Image::LoadImage(estim, argv[2]);

    std::cout << "[INFO] Metrics results :\n";
    std::cout << "  - Mean Squared Error (MSE): " << Weyl::Metrics::MeanSquaredError(ref, estim) << std::endl;
    std::cout << "  - Bad Pixel Rate (BPR):     " << Weyl::Metrics::BadPixelRate(ref, estim, bprTolerance) << std::endl;

    return 0;
}