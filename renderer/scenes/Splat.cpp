#include "Splat.hpp"

#include <fstream>
#include <cmath>

#include "common/LogCommon.hpp"

#ifdef HOST_ANDROID
#include <android/asset_manager.h>
#include "AndroidMain.hpp"
#endif

namespace X {

struct RawGaussianPoint {
    float pos[3];
    float n[3];
    float shs[48];
    float opacity;
    float scale[3];
    float rot[4];
};

static void QuatToMatrix3(float* q, float* mat3)
{
    float norm = sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    q[0] = q[0] / norm;
    q[1] = q[1] / norm;
    q[2] = q[2] / norm;
    q[3] = q[3] / norm;

    mat3[0] = 1 - 2 * (q[2] * q[2] + q[3] * q[3]);
    mat3[1] = 2 * (q[1] * q[2] + q[0] * q[3]);
    mat3[2] = 2 * (q[1] * q[3] - q[0] * q[2]);

    mat3[3] = 2 * (q[1] * q[2] - q[0] * q[3]);
    mat3[4] = 1 - 2 * (q[1] * q[1] + q[3] * q[3]);
    mat3[5] = 2 * (q[2] * q[3] + q[0] * q[1]);

    mat3[6] = 2 * (q[1] * q[3] + q[0] * q[2]);
    mat3[7] = 2 * (q[2] * q[3] - q[0] * q[1]);
    mat3[8] = 1 - 2 * (q[1] * q[1] + q[2] * q[2]);
}

static void CalculateCov(float* scale, float* rotate, float* cov)
{
    float sr[9];
    QuatToMatrix3(rotate, sr);

    for (size_t i = 0; i < 3; i++) {
        float s = std::exp(scale[i]);
        sr[3 * i] = s * sr[3 * i];
        sr[3 * i + 1] = s * sr[3 * i + 1];
        sr[3 * i + 2] = s * sr[3 * i + 2];
    }

    size_t idx = 0;
    for (size_t i = 0; i < 3; i++) {
        for (size_t j = i; j < 3; j++) {
            cov[idx++] = 4 * (sr[i] * sr[j] + sr[i + 3] * sr[j + 3] + sr[i + 6] * sr[j + 6]);
        }
    }
}

// Load the Gaussians from the given file.
std::vector<GaussianPoint> LoadPly(std::string fileName)
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
    std::vector<RawGaussianPoint> rawPoints(count);
    infile.read((char*)(rawPoints.data()), count * sizeof(RawGaussianPoint));

    // simple preprocess
    std::vector<GaussianPoint> points(count);
    for (size_t i = 0; i < count; i++) {
        points[i].pos[0] = rawPoints[i].pos[0];
        points[i].pos[1] = rawPoints[i].pos[1];
        points[i].pos[2] = rawPoints[i].pos[2];

        points[i].opacity = 1.0f / (1.0f + std::exp(-rawPoints[i].opacity));
        for (size_t j = 0; j < 48; j++) {
            points[i].shs[j] = rawPoints[i].shs[j];
        }

        CalculateCov(rawPoints[i].scale, rawPoints[i].rot, points[i].cov);
    }

    // TODO: z-order for tiling, no need for graphics pipeline?
    return points;
}

#ifdef HOST_ANDROID
std::vector<GaussianPoint> LoadPlyAndroid(std::string fileName)
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
    std::vector<RawGaussianPoint> rawPoints(count);
    uint32_t headerLength = ss.tellg();
    std::copy(data + headerLength + 1, data + 1 + headerLength + count * sizeof(RawGaussianPoint), (char*)rawPoints.data());

    // simple preprocess
    std::vector<GaussianPoint> points(count);
    for (size_t i = 0; i < count; i++) {
        points[i].pos[0] = rawPoints[i].pos[0];
        points[i].pos[1] = rawPoints[i].pos[1];
        points[i].pos[2] = rawPoints[i].pos[2];

        points[i].opacity = 1.0f / (1.0f + std::exp(-rawPoints[i].opacity));
        for (size_t j = 0; j < 16; j++) {
            points[i].shs[j] = rawPoints[i].shs[j];
        }

        CalculateCov(rawPoints[i].scale, rawPoints[i].rot, points[i].cov);
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

GaussianPoint Splat::GetPointData(uint32_t index) const
{
    return data_[index];
}

} // namespace X