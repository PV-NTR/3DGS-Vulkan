#pragma once

#include <memory>

#include "Object.hpp"
#include "resources/Data.hpp"

namespace X {

struct SplatData {
    uint32_t vertexCnt_;
    std::shared_ptr<Data> positions_;
    std::shared_ptr<Data> rotations_;
    std::shared_ptr<Data> scales_;
    std::shared_ptr<Data> colors_;
    std::shared_ptr<Data> shs_;
};

class Splat : public Object {
public:

private:
    SplatData data_;
};

} // namespace X