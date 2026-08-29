#pragma once

// Minimal stand-in for Arduino.h, used only by the native simulator build. Implements just the two things
// display.cpp/oba_client.h actually need, not a full Arduino String reimplementation.

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>

class String : public std::string {
public:
    String() = default;
    String(const char *s) : std::string(s) {}
    String(const std::string &s) : std::string(s) {}

    String(int64_t value) { *this = std::to_string(value); }

    String operator+(const char *rhs) const { return String(*this) += rhs; }
    String operator+(int64_t rhs) const { return String(*this) += String(rhs); }
    String &operator+=(const char *rhs) {
        std::string::operator+=(rhs);
        return *this;
    }
    String &operator+=(const String &rhs) {
        std::string::operator+=(rhs);
        return *this;
    }
};

inline String operator+(const char *lhs, const String &rhs) {
    return String(lhs) += rhs;
}

inline unsigned long millis() {
    using namespace std::chrono;
    static const auto start = steady_clock::now();
    return duration_cast<milliseconds>(steady_clock::now() - start).count();
}
