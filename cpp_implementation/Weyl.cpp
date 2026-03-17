#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Weyl.hpp"


void Weyl::Image::LoadImage(Image &image, std::string path)
{
    unsigned char* stbiImage = stbi_load(path.c_str(), &image.Width, &image.Height, &image.Channels, 0);
    
    if(stbiImage == NULL) {
        std::cout << "[ERROR] Unable to load the image." << std::endl;
        exit(1);
    }

    image.Img.resize(image.Width * image.Height);
    for (int i = 0; i < image.Width * image.Height; ++i) {
        image.Img[i] = stbiImage[i * image.Channels];
    }

    stbi_image_free(stbiImage);

    std::cout << "[INFO] Image loaded : " << image.Width << "x" << image.Height 
              << " (" << image.Channels << " channels)." << std::endl;
}


void Weyl::Image::WriteImage(Image& image, std::string path)
{
    int targetChannels = 1;
    int stride_in_bytes = image.Width * targetChannels;
    int success = stbi_write_png(path.c_str(), image.Width, image.Height, targetChannels, 
                                 image.Img.data(), stride_in_bytes);
    
    if (success == 0) {
        std::cerr << "[ERROR] Unable to write the image to " << path << std::endl;
    } else {
        std::cout << "[INFO] Image successfully saved to : " << path << std::endl;
    }
}


void Weyl::Image::NormalizeImageData(const std::vector<uint32_t> &data, const uint32_t min, const uint32_t max, Image &image)
{
    size_t size = data.size();
    image.Img.resize(size);

    float range = static_cast<float>(max - min);
    if (range == 0.0f) range = 1.0f; 

    for(size_t i = 0; i < size; i++)
    {
        float normalized = ((data[i] - min) / range) * 255.0f;
        image.Img[i] = static_cast<uint8_t>(normalized + 0.5f);
    }
}

void Weyl::Image::NormalizeImageData(const std::vector<uint32_t> &data, Image &image)
{
    const uint32_t min = *std::min_element(data.begin(), data.end());
    const uint32_t max = *std::max_element(data.begin(), data.end());

    NormalizeImageData(data, min, max, image);
}

uint32_t Weyl::Core::WeylDiscrepancy(const Image::Image& image)
{
    int width = image.Width, height = image.Height;

    // Using a linearized image for memory alignment
    std::vector<uint32_t> integralImage(width * height);

    uint32_t globalMinP1 = UINT32_MAX, globalMaxP1 = 0;
    uint32_t globalMinP2 = UINT32_MAX, globalMaxP2 = 0;
    uint32_t globalMinP3 = UINT32_MAX, globalMaxP3 = 0;
    int32_t  globalMinP4 = INT32_MAX,  globalMaxP4 = INT32_MIN; // P4 values are int32 and not uint32 because they can be negative
    
    uint32_t rowSum = image.Img[0];
    integralImage[0] = rowSum;
    uint32_t localMinP1 = image.Img[0];
    uint32_t localMaxP1 = image.Img[0];


    // Pass 1 : P1 and P2 -------------------------------------------------------------------------

    // First loop for topmost row
    for(int x = 1; x < width; x++)
    {
        rowSum += image.Img[x];
        integralImage[x] = rowSum;

        localMinP1 = std::min(localMinP1, integralImage[x]);
        localMaxP1 = std::max(localMaxP1, integralImage[x]);
    }

    globalMinP1 = std::min(globalMinP1, localMinP1);
    globalMaxP1 = std::max(globalMaxP1, localMaxP1);

    unsigned int c = width - 1;
    globalMinP2 = std::min(globalMinP2, std::min(integralImage[c], integralImage[c] - localMaxP1));
    globalMaxP2 = std::max(globalMaxP2, std::max(integralImage[c], integralImage[c] - localMinP1));

    // Main loop of first pass
    for(int y = 1; y < height; y++)
    {
        localMinP1 = UINT32_MAX;
        localMaxP1 = 0;
        rowSum = 0;

        for(int x = 0; x < width; x++)
        {
            unsigned int current = y * width + x;
            rowSum += image.Img[current];
            integralImage[current] = integralImage[current - width] + rowSum;

            localMinP1 = std::min(localMinP1, integralImage[current]);
            localMaxP1 = std::max(localMaxP1, integralImage[current]);
        }

        globalMinP1 = std::min(globalMinP1, localMinP1);
        globalMaxP1 = std::max(globalMaxP1, localMaxP1);

        c = (y + 1) * width - 1;
        globalMinP2 = std::min(globalMinP2, std::min(integralImage[c], integralImage[c] - localMaxP1));
        globalMaxP2 = std::max(globalMaxP2, std::max(integralImage[c], integralImage[c] - localMinP1));
    }


    // Pass 2 : P3 and P4 -------------------------------------------------------------------------

    int32_t integralValueP4;
    
    localMinP1 = image.Img[0];
    localMaxP1 = image.Img[0];

    // First loop for leftmost column
    for(int y = 0; y < height; y++)
    {
        unsigned int current = y * width;
        unsigned int maxWidth = (y + 1) * width - 1;
        unsigned int maxHeight = (height - 1) * width;

        // P3
        localMinP1 = std::min(localMinP1, integralImage[current]);
        localMaxP1 = std::max(localMaxP1, integralImage[current]);

        // P4
        integralValueP4 = integralImage[y * width] - integralImage[maxWidth] - integralImage[maxHeight];
        globalMinP4 = std::min(globalMinP4, integralValueP4);
        globalMaxP4 = std::max(globalMaxP4, integralValueP4);
    }

    c = (height - 1) * width;
    globalMinP3 = std::min(globalMinP3, std::min(integralImage[c], integralImage[c] - localMaxP1));
    globalMaxP3 = std::max(globalMaxP3, std::max(integralImage[c], integralImage[c] - localMinP1));

    // Main loop of second pass
    for(int x = 1; x < width; x++)
    {
        localMinP1 = UINT32_MAX;
        localMaxP1 = 0;

        unsigned int maxHeight = (height - 1) * width + x;

        for(int y = 0; y < height; y++)
        {
            unsigned int current = y * width + x;
            unsigned int maxWidth = (y + 1) * width - 1;

            // P3
            localMinP1 = std::min(localMinP1, integralImage[current]);
            localMaxP1 = std::max(localMaxP1, integralImage[current]);

            // P4
            integralValueP4 = integralImage[current] - integralImage[maxWidth] - integralImage[maxHeight];
            globalMinP4 = std::min(globalMinP4, integralValueP4);
            globalMaxP4 = std::max(globalMaxP4, integralValueP4);
        }

        c = (height - 1) * width + x;
        globalMinP3 = std::min(globalMinP3, std::min(integralImage[c], integralImage[c] - localMaxP1));
        globalMaxP3 = std::max(globalMaxP3, std::max(integralImage[c], integralImage[c] - localMinP1));
    }


    // Return the maximum of the differences ------------------------------------------------------

    return std::max({
        globalMaxP1 - globalMinP1,
        globalMaxP2 - globalMinP2,
        globalMaxP3 - globalMinP3,
        static_cast<uint32_t>(globalMaxP4 - globalMinP4)
    });
}


uint32_t Weyl::Core::WeylDiscrepancyAVX(const Image::Image& image)
{
    int width = image.Width, height = image.Height;

    // Defining the same variables as the normal version
    std::vector<uint32_t> integralImage(width * height);

    uint32_t globalMinP1 = UINT32_MAX, globalMaxP1 = 0;
    uint32_t globalMinP2 = UINT32_MAX, globalMaxP2 = 0;
    uint32_t globalMinP3 = UINT32_MAX, globalMaxP3 = 0;
    int32_t  globalMinP4 = INT32_MAX,  globalMaxP4 = INT32_MIN;
    
    uint32_t firstRowSum = image.Img[0];
    integralImage[0] = firstRowSum;
    uint32_t localMinP1 = image.Img[0];
    uint32_t localMaxP1 = image.Img[0];

    
    // Pass 1 : P1 and P2 -------------------------------------------------------------------------
    
    // First row is computed in linear way for lisibility (no performance loose)
    for(int x = 1; x < width; x++) {
        firstRowSum += image.Img[x];
        integralImage[x] = firstRowSum;

        localMinP1 = std::min(localMinP1, integralImage[x]);
        localMaxP1 = std::max(localMaxP1, integralImage[x]);
    }
    
    globalMinP1 = std::min(globalMinP1, localMinP1);
    globalMaxP1 = std::max(globalMaxP1, localMaxP1);

    unsigned int c1 = width - 1;
    globalMinP2 = std::min(globalMinP2, std::min(integralImage[c1], integralImage[c1] - localMaxP1));
    globalMaxP2 = std::max(globalMaxP2, std::max(integralImage[c1], integralImage[c1] - localMinP1));

    // Next rows using AVX2
    for(int y = 1; y < height; y++)
    {
        // Declaring 256 bits int (8 x 32 bits ints) for localMin and localMax
        __m256i localMinP1_vec = _mm256_set1_epi32(UINT32_MAX);
        __m256i localMaxP1_vec = _mm256_set1_epi32(0);
        uint32_t rowSum = 0;

        // Reusing x id for multiple of 8 loop and 'finish' loop
        int x = 0;
        for(; x <= width - 8; x += 8)
        {
            unsigned int current = y * width + x;

            // Loading 8 next image pixels
            __m128i pixels_8bit = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&image.Img[current]));
            __m256i vec = _mm256_cvtepu8_epi32(pixels_8bit);

            // Next 4 blocks : calculating integral using paper method
            vec = _mm256_add_epi32(vec, _mm256_slli_si256(vec, 4));
            vec = _mm256_add_epi32(vec, _mm256_slli_si256(vec, 8));

            __m256i shift_lane = _mm256_permute2x128_si256(vec, vec, 0x08);
            __m256i broadcast_high = _mm256_shuffle_epi32(shift_lane, 0xFF);
            vec = _mm256_add_epi32(vec, broadcast_high);

            __m256i rowSum_vec = _mm256_set1_epi32(rowSum);
            vec = _mm256_add_epi32(vec, rowSum_vec);

            __m256i vec_above = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&integralImage[current - width]));
            __m256i integral_vec = _mm256_add_epi32(vec, vec_above);

            // Storing values in integral image
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(&integralImage[current]), integral_vec);

            // Computing locals min/max (8 differents locals min/max)
            localMinP1_vec = _mm256_min_epu32(localMinP1_vec, integral_vec);
            localMaxP1_vec = _mm256_max_epu32(localMaxP1_vec, integral_vec);

            __m128i top_half = _mm256_extracti128_si256(vec, 1);
            rowSum = static_cast<uint32_t>(_mm_cvtsi128_si32(_mm_shuffle_epi32(top_half, 0xFF)));
        }

        // Reducing locals min/max
        localMinP1 = hmin_epi32(localMinP1_vec);
        localMaxP1 = hmax_epi32(localMaxP1_vec);

        // Computing the rest of the image (not multiple of 8)
        for(; x < width; x++)
        {
            unsigned int current = y * width + x;
            rowSum += image.Img[current];
            integralImage[current] = integralImage[current - width] + rowSum;

            localMinP1 = std::min(localMinP1, integralImage[current]);
            localMaxP1 = std::max(localMaxP1, integralImage[current]);
        }

        globalMinP1 = std::min(globalMinP1, localMinP1);
        globalMaxP1 = std::max(globalMaxP1, localMaxP1);

        unsigned int c = (y + 1) * width - 1;
        globalMinP2 = std::min(globalMinP2, std::min(integralImage[c], integralImage[c] - localMaxP1));
        globalMaxP2 = std::max(globalMaxP2, std::max(integralImage[c], integralImage[c] - localMinP1));
    }


    // Pass 2 : P3 and P4 -------------------------------------------------------------------------
    // Sequential mode because columns aren't aligned in memory
    
    int32_t integralValueP4;
    localMinP1 = integralImage[0];
    localMaxP1 = integralImage[0];

    for(int y = 0; y < height; y++)
    {
        unsigned int current = y * width;
        unsigned int maxWidth = (y + 1) * width - 1;
        unsigned int maxHeight = (height - 1) * width;

        localMinP1 = std::min(localMinP1, integralImage[current]);
        localMaxP1 = std::max(localMaxP1, integralImage[current]);

        integralValueP4 = integralImage[current] - integralImage[maxWidth] - integralImage[maxHeight];
        globalMinP4 = std::min(globalMinP4, integralValueP4);
        globalMaxP4 = std::max(globalMaxP4, integralValueP4);
    }

    unsigned int c2 = (height - 1) * width;
    globalMinP3 = std::min(globalMinP3, std::min(integralImage[c2], integralImage[c2] - localMaxP1));
    globalMaxP3 = std::max(globalMaxP3, std::max(integralImage[c2], integralImage[c2] - localMinP1));

    for(int x = 1; x < width; x++)
    {
        localMinP1 = UINT32_MAX;
        localMaxP1 = 0;
        unsigned int maxHeight = (height - 1) * width + x;

        for(int y = 0; y < height; y++)
        {
            unsigned int current = y * width + x;
            unsigned int maxWidth = (y + 1) * width - 1;

            localMinP1 = std::min(localMinP1, integralImage[current]);
            localMaxP1 = std::max(localMaxP1, integralImage[current]);

            integralValueP4 = integralImage[current] - integralImage[maxWidth] - integralImage[maxHeight];
            globalMinP4 = std::min(globalMinP4, integralValueP4);
            globalMaxP4 = std::max(globalMaxP4, integralValueP4);
        }

        unsigned int c3 = (height - 1) * width + x;
        globalMinP3 = std::min(globalMinP3, std::min(integralImage[c3], integralImage[c3] - localMaxP1));
        globalMaxP3 = std::max(globalMaxP3, std::max(integralImage[c3], integralImage[c3] - localMinP1));
    }


    // Return the maximum of the differences ------------------------------------------------------

    return std::max({
        globalMaxP1 - globalMinP1,
        globalMaxP2 - globalMinP2,
        globalMaxP3 - globalMinP3,
        static_cast<uint32_t>(globalMaxP4 - globalMinP4)
    });
}


uint32_t Weyl::Core::hmin_epi32(__m256i v) {

    __m128i low128  = _mm256_castsi256_si128(v);
    __m128i high128 = _mm256_extracti128_si256(v, 1);
    __m128i min128  = _mm_min_epu32(low128, high128);

    __m128i high64  = _mm_shuffle_epi32(min128, _MM_SHUFFLE(1, 0, 3, 2));
    __m128i min64   = _mm_min_epu32(min128, high64);

    __m128i high32  = _mm_shuffle_epi32(min64, _MM_SHUFFLE(2, 3, 0, 1));
    __m128i min32   = _mm_min_epu32(min64, high32);

    return static_cast<uint32_t>(_mm_cvtsi128_si32(min32));
}


uint32_t Weyl::Core::hmax_epi32(__m256i v) {
    __m128i low128  = _mm256_castsi256_si128(v);
    __m128i high128 = _mm256_extracti128_si256(v, 1);
    __m128i max128  = _mm_max_epu32(low128, high128);

    __m128i high64  = _mm_shuffle_epi32(max128, _MM_SHUFFLE(1, 0, 3, 2));
    __m128i max64   = _mm_max_epu32(max128, high64);

    __m128i high32  = _mm_shuffle_epi32(max64, _MM_SHUFFLE(2, 3, 0, 1));
    __m128i max32   = _mm_max_epu32(max64, high32);

    return static_cast<uint32_t>(_mm_cvtsi128_si32(max32));
}


int Weyl::PatchMatching(const Image::Image &image, const Image::Image &patch, Image::Image &disparityMap)
{
    int iw = image.Width;
    int ih = image.Height;
    int pw = patch.Width;
    int ph = patch.Height;

    Image::Image diffImage(pw, ph);

    uint32_t minDiscrepancy = UINT32_MAX;
    uint32_t maxDiscrepancy = 0;
    int bestMatchIndex = 0;

    int dispWidth = iw - pw + 1;
    int dispHeight = ih - ph + 1;
    std::vector<uint32_t> rawDisparities(dispWidth * dispHeight);

    for(int y = 0; y <= ih - ph; y++) {
        for(int x = 0; x <= iw - pw; x++) {

            // Creating the difference image
            for(int j = 0; j < ph; j++) {
                for(int i = 0; i < pw; i++) {

                    int imgIdx = (y + j) * iw + (x + i);
                    int patchIdx = j * pw + i;

                    int diff = std::abs(image.Img[imgIdx] - patch.Img[patchIdx]);
                    diffImage.Img[patchIdx] = static_cast<uint8_t>(diff);
                }
            }

            // Computing disparity
            int dispIdx = y * dispWidth + x;
            uint32_t currentDiscrepancy = Core::WeylDiscrepancyAVX(diffImage);
            rawDisparities[dispIdx] = currentDiscrepancy;

            // Storing extrem values
            if(currentDiscrepancy < minDiscrepancy) {
                minDiscrepancy = currentDiscrepancy;
                bestMatchIndex = dispIdx;
            }
            if(currentDiscrepancy > maxDiscrepancy) {
                maxDiscrepancy = currentDiscrepancy;
            }
        }
    }

    disparityMap.Width = dispWidth;
    disparityMap.Height = dispHeight;
    disparityMap.Channels = 1;
    disparityMap.Img.resize(dispWidth * dispHeight);

    Image::NormalizeImageData(rawDisparities, minDiscrepancy, maxDiscrepancy, disparityMap);

    return bestMatchIndex;
}


void Weyl::DenseCorresponding(const Image::Image& imgLeft, const Image::Image& imgRight, 
                         std::vector<uint32_t>& dispLeft, std::vector<uint32_t>& dispRight, 
                         const int patchSize, const int maxDisparity) 
{
    int width = imgLeft.Width;
    int height = imgLeft.Height;
    int radius = patchSize / 2;

    dispLeft.assign(width * height, 0);
    dispRight.assign(width * height, 0);

    Image::Image diffBuffer(patchSize, patchSize);

    for (int y = radius; y < height - radius; ++y) 
    {
        for (int x = radius; x < width - radius; ++x) 
        {
            uint32_t minWeyl = UINT32_MAX;
            int best_d = 0;

            for (int d = 0; d <= maxDisparity; ++d) 
            {
                if (x - d - radius < 0) continue;

                for (int j = -radius; j <= radius; ++j) {
                    for (int i = -radius; i <= radius; ++i) {
                        int idxL = (y + j) * width + (x + i);
                        int idxR = (y + j) * width + (x - d + i); 
                        
                        int diff = std::abs(imgLeft.Img[idxL] - imgRight.Img[idxR]);
                        diffBuffer.Img[(j + radius) * patchSize + (i + radius)] = static_cast<uint8_t>(diff);
                    }
                }

                uint32_t currentWeyl = Core::WeylDiscrepancyAVX(diffBuffer);

                if (currentWeyl < minWeyl) {
                    minWeyl = currentWeyl;
                    best_d = d;
                }
            }
            dispLeft[y * width + x] = best_d;
        }
    }

    for (int y = radius; y < height - radius; ++y) 
    {
        for (int x = radius; x < width - radius; ++x) 
        {
            uint32_t minWeyl = UINT32_MAX;
            int best_d = 0;

            for (int d = 0; d <= maxDisparity; ++d) 
            {
                if (x + d + radius >= width) continue;

                for (int j = -radius; j <= radius; ++j) {
                    for (int i = -radius; i <= radius; ++i) {
                        int idxR = (y + j) * width + (x + i);
                        int idxL = (y + j) * width + (x + d + i); 
                        
                        int diff = std::abs(imgRight.Img[idxR] - imgLeft.Img[idxL]);
                        diffBuffer.Img[(j + radius) * patchSize + (i + radius)] = static_cast<uint8_t>(diff);
                    }
                }

                uint32_t currentWeyl = Core::WeylDiscrepancyAVX(diffBuffer);

                if (currentWeyl < minWeyl) {
                    minWeyl = currentWeyl;
                    best_d = d;
                }
            }
            dispRight[y * width + x] = best_d * 255.0f;
        }
    }
}