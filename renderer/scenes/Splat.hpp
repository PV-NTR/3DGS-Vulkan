#pragma once

#include <memory>
#include <string>

#include "Object.hpp"
#include "resources/Data.hpp"

namespace X {

struct GaussianPoint {
    float pos[3];
    float shs[48];
    float opacity;
    float cov[6];
};

// TODO: add other compressed formats
// simply replace float with half float
struct CompressedGaussianPoint {
    uint32_t posAndOpacity[2];
    uint32_t shs[24];
    uint32_t cov[3];
};

class Splat : public Object {
public:
    [[nodiscard]] static std::unique_ptr<Splat> MakeUnique(std::string fileName, bool compress = false);
    [[nodiscard]] static std::shared_ptr<Splat> MakeShared(std::string fileName, bool compress = false);
    uint32_t GetPointCount() const { return data_.size(); }
    const void* GetPointData() const;
    GaussianPoint GetPointData(uint32_t index) const;
    bool IsCompressed() const { return compressed_; }

protected:
    Splat() : Object(Type::Splat) {}
    explicit Splat(std::string fileName, bool compresse);
    void CompressData();

private:
    std::vector<GaussianPoint> data_;
    std::vector<CompressedGaussianPoint> compressedData_;
    bool compressed_ = false;
};

} // namespace X