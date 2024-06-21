#include "Splat.hpp"

#include <fstream>

#include "common/LogCommon.hpp"

namespace X {

// Load the Gaussians from the given file.
std::vector<RawGaussianPoint> loadPly(std::string fileName)
{
    std::ifstream infile(fileName.c_str(), std::ios_base::binary);

    if (!infile.good()) {
        XLOGE("Unable to find model's PLY file, fileName: %s", fileName);
    }

    // "Parse" header (it has to be a specific format anyway)
    std::string buff;
    std::getline(infile, buff);
    std::getline(infile, buff);

    std::string dummy;
    std::getline(infile, buff);
    std::stringstream ss(buff);
    int count;
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

    // TODO: z-order for tiling, no need for graphics pipeline?
    return points;
}

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
    data_ = loadPly(fileName);
}

} // namespace X