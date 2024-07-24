#include "Splat.hpp"

#include <fstream>
#include <cmath>

#include "common/LogCommon.hpp"

#ifdef HOST_ANDROID
#include <android/asset_manager.h>
#include "AndroidMain.hpp"
#endif

namespace X {

// Load the Gaussians from the given file.
std::vector<RawGaussianPoint> LoadPly(std::string fileName)
{
    std::ifstream infile(fileName.c_str(), std::ios_base::binary);

    if (!infile.good()) {
        XLOGE("Unable to find model's PLY file, fileName: %s", fileName.c_str());
        return {};
    }

    // "Parse" header (it has to be a specific format anyway)
    std::string buff;
    std::getline(infile, buff);
    std::getline(infile, buff);

    std::string dummy;
    std::getline(infile, buff);
    std::stringstream ss(buff);
    int count = 0;
    ss >> dummy >> dummy >> count;

    // Output number of Gaussians contained
    XLOGI("Loading %d Gaussian splats...", count);

    while (std::getline(infile, buff)) {
        if (buff.compare("end_header") == 0) {
            break;
        }
    }

    // Read all Gaussians at once (AoS)
    std::vector<RawGaussianPoint> points(count);
    infile.read((char*)(points.data()), count * sizeof(RawGaussianPoint));
    for (auto& point : points) {
        point.scale[0] = std::exp(point.scale[0]);
        point.scale[1] = std::exp(point.scale[1]);
        point.scale[2] = std::exp(point.scale[2]);
        point.opacity = 1.0f / (1.0f + std::exp(-point.opacity));
    }

    // TODO: z-order for tiling, no need for graphics pipeline?
    return points;
}

#ifdef HOST_ANDROID
std::vector<RawGaussianPoint> LoadPlyAndroid(std::string fileName)
{
    assert(g_androidAppCtx);
    assert(g_androidAppCtx->activity->assetManager);
    AAsset* asset = AAssetManager_open(g_androidAppCtx->activity->assetManager, fileName.c_str(), AASSET_MODE_STREAMING);
    assert(asset);
    size_t size = AAsset_getLength(asset);
    assert(size > 0);

    // memory leak?
    char* data = new char[size];
    AAsset_read(asset, data, size);
    AAsset_close(asset);

    std::stringstream ss(data);
    // "Parse" header (it has to be a specific format anyway)
    std::string dummy;
    int count = 0;
    ss >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy >> count;

    // Output number of Gaussians contained
    XLOGI("Loading %d Gaussian splats...", count);

    while (ss >> dummy) {
        if (dummy.compare("end_header") == 0) {
            break;
        }
    }

    // Read all Gaussians at once (AoS)
    std::vector<RawGaussianPoint> points(count);
    uint32_t headerLength = ss.tellg();
    std::copy(data + headerLength + 1, data + 1 + headerLength + count * sizeof(RawGaussianPoint), (char*)points.data());
    for (auto& point : points) {
        point.scale[0] = std::exp(point.scale[0]);
        point.scale[1] = std::exp(point.scale[1]);
        point.scale[2] = std::exp(point.scale[2]);
        point.opacity = 1.0f / (1.0f + std::exp(-point.opacity));
    }

    // TODO: z-order for tiling, no need for graphics pipeline?
    return points;
}
#endif

std::unique_ptr<Splat> Splat::MakeUnique(std::string fileName)
{
    return std::unique_ptr<Splat>(new Splat(fileName));
}

std::shared_ptr<Splat> Splat::MakeShared(std::string fileName)
{
    return std::shared_ptr<Splat>(new Splat(fileName));
}

Splat::Splat(std::string fileName) : Object(Type::Splat)
{
    // TODO: load data from file
#ifdef HOST_ANDROID
    data_ = LoadPlyAndroid(fileName);
#elif defined HOST_WIN32
    data_ = LoadPly(fileName);
#endif
}

RawGaussianPoint Splat::GetPointData(uint32_t index) const
{
    return data_[index];
}

} // namespace X