#pragma once

#include <memory>

namespace X {

class Data {
public:
    std::shared_ptr<Data> MakeLength(uint32_t size, bool init = false);
    ~Data();

    void* Addr() { return data_; }
    uint32_t Size() { return size_; }

private:
    Data() = default;
    Data(uint32_t size, bool init = false);
    uint32_t size_;
    void* data_;
};

} // namespace X