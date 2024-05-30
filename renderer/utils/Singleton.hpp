#pragma once

template<typename T>
class Singleton {
public:
    static T& GetInstance() {
        static T instance_ {};
        return instance_;
    }

protected:
    Singleton() = default;

private:
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;
};