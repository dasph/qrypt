#pragma once
#include <vector>
#include <cstdint>

class ReedSolomon final {
  private:
    static uint8_t multiply (uint8_t a, uint8_t b);
  public:
    static std::vector<uint8_t> divisor (uint8_t degree);
    static std::vector<uint8_t> remainder (const std::vector<uint8_t> &data, const std::vector<uint8_t> &divisor);
};
