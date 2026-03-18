#include "Weyl.hpp"
#include <iostream>
#include <vector>
#include <chrono> // Pour le chronométrage
#include <string>
#include <bits/stdc++.h> // Pour strcat
#include <filesystem>

namespace fs = std::filesystem;

struct BBox {
    int x;
    int y;
    int w;
    int h;
};

int main(int argc, char** argv) 
{
    if (argc < 4 || argc > 5) {
        std::cerr << "Usage : " << argv[0] << " <root_tracking_path> <video_name>"
            << " <frames_count> <optional:root_output_path>\n";
        return 1;
    }

    // Retrieving arguments
    std::string rootTrackingPath = argv[1];
    std::string videoName = argv[2];
    int framesCount = std::stoi(argv[3]);
    std::string outputPath = (argc == 5) ? argv[4] : ".";

    // Prepare output folder for writing
    fs::path outDir = fs::path(outputPath) / videoName / "frames";
    fs::create_directories(outDir);

    // Retrieving patch image
    fs::path patchPath = fs::path(rootTrackingPath) / videoName / (videoName + "_patch.jpg");
    Weyl::Image::Image patch;
    Weyl::Image::LoadImage(patch, patchPath.string());

    // Preparing Various global variables
    std::vector<BBox> boundingBoxes(framesCount);
    Weyl::Image::Image disparityBuffer;

    std::cout << "[INFO] Tracking informations :" << std::endl;;
    std::cout << "  - Frames count: " << framesCount << std::endl;
    std::cout << "  - Patch size:   " << patch.Width << "x" << patch.Height << std::endl;
    std::cout << "[INFO] Starting video object tracking..." << std::endl;

    // Lopping over each image
    for(int i = 0; i < framesCount; i++) {

        // Creating frame paths
        std::string frameFileName = videoName + "_frame__" + std::to_string(i) + ".jpg";
        std::string dispFileName = videoName + "_disp_frame__" + std::to_string(i) + ".png";

        fs::path framePath = fs::path(rootTrackingPath) / videoName / "frames" / frameFileName;
        fs::path frameOutPath = outDir / dispFileName;
        
        // Retrieving frame image
        Weyl::Image::Image frame;
        Weyl::Image::LoadImage(frame, framePath.string());

        // Calling patch matching
        int bestIndex = Weyl::PatchMatching(frame, patch, disparityBuffer);

        // Retrievin bounding box position
        int dispWidth = frame.Width - patch.Width + 1; 
        int matchY = bestIndex / dispWidth; 
        int matchX = bestIndex % dispWidth; 

        boundingBoxes[i] = {
            matchX,
            matchY,
            patch.Width,
            patch.Height
        };

        Weyl::Image::WriteImage(disparityBuffer, frameOutPath.string());
        
        std::cout << "  - Frame " << i+1 << "/" << framesCount << " done." << std::endl;
    }

    // Writing .csv file with bounding box positions
    std::cout << "[INFO] Saving Bounding boxes positions..." << std::endl;

    fs::path csvPath = outDir / (videoName + "_tracking.csv");
    std::ofstream csvFile(csvPath);

    if (!csvFile.is_open()) {
        std::cerr << "[ERREUR] Unable to create CSV file: " << csvPath << std::endl;
    } else {
        csvFile << "frame,x,y,w,h\n";

        for (int i = 0; i < framesCount; ++i) {
            csvFile << i << "," 
                    << boundingBoxes[i].x << "," 
                    << boundingBoxes[i].y << "," 
                    << boundingBoxes[i].w << "," 
                    << boundingBoxes[i].h << "\n";
        }

        csvFile.close();
        std::cout << "[SUCESS] Bounding boxes saved at: " << csvPath << std::endl;
    }

    return 0;
}