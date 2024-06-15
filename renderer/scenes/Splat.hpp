#pragma once

#include <memory>
#include <string>

#include "Object.hpp"
#include "resources/Data.hpp"

namespace X {

struct SplatData {
    uint32_t count_;
    std::shared_ptr<Data> positions_;
    std::shared_ptr<Data> rotations_;
    std::shared_ptr<Data> scales_;
    std::shared_ptr<Data> colors_;
    std::shared_ptr<Data> shs_;
};

class Splat : public Object {
public:
    [[nodiscard]] static std::unique_ptr<Splat> MakeUnique(std::string fileName);
    [[nodiscard]] static std::shared_ptr<Splat> MakeShared(std::string fileName);
    uint32_t GetPointCount() const { return data_.count_; }

protected:
    Splat() : Object(Type::Splat) {}
    explicit Splat(std::string fileName);

private:
    SplatData data_;
};

} // namespace X