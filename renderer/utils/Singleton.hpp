#pragma once

template<typename T>
class Singleton {
public:
    Singleton() = default;
    ~Singleton() = default;
    static T& GetInstance() {
        static T instance_;
        return instance_;
    }

private:
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;
};