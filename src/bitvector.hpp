#pragma once
#include <vector>

class BitVector final : public std::vector<bool> {
  public:
    BitVector () : std::vector<bool>() {};
    void append (const size_t &value, const size_t &length);
};
