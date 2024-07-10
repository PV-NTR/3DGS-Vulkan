#pragma once

#include <memory>
#include <string>

#include "Object.hpp"
#include "resources/Data.hpp"

namespace X {

struct RawGaussianPoint {
    float pos[3];
    float n[3];
    float shs[48];
    float opacity;
    float scale[3];
    float rot[4];
};

class Splat : public Object {
public:
    [[nodiscard]] static std::unique_ptr<Splat> MakeUnique(std::string fileName);
    [[nodiscard]] static std::shared_ptr<Splat> MakeShared(std::string fileName);
    uint32_t GetPointCount() const { return data_.size(); }
    const void* GetPointData() const { return data_.data(); }
    RawGaussianPoint GetPointData(uint32_t index) const;

protected:
    Splat() : Object(Type::Splat) {}
    explicit Splat(std::string fileName);

private:
    std::vector<RawGaussianPoint> data_;
};

} // namespace X