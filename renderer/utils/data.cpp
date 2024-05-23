#include "data.hpp"

std::shared_ptr<Data> Data::MakeLength(uint32_t size, bool init)
{
    return std::shared_ptr<Data>(new Data(size, init));
}

Data::Data(uint32_t size, bool init) : size_(size)
{
    data_ = new uint32_t[size];
    if (init) {
        memset(data_, 0, size_);
    }
}

Data::~Data()
{
    if (data_) {
        delete reinterpret_cast<uint32_t*>(data_);
    }
}