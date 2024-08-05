#pragma once

#include <memory>

namespace X {

class Data {
public:
    [[nodiscard]] static std::shared_ptr<Data> MakeLength(uint32_t size, bool init = false);
    virtual ~Data();

    void* Addr() { return data_; }
    uint32_t Size() { return size_; }

private:
    Data() = default;
    explicit Data(uint32_t size, bool init = false);
    uint32_t size_;
    void* data_;
};

} // namespace X