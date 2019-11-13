#include <stdexcept>
#include "bitvector.hpp"

void BitVector::append (const unsigned int &value, const size_t &length) {
	if (length > 31 || value >> length != 0) {
    throw std::out_of_range("BitVector length argument");
  }

	for (int i = length - 1; i >= 0; i--) {
    this->push_back((value >> i) & 1);
  }
}
