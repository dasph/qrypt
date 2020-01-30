#include "reedsolomon.hpp"

using std::vector;

uint8_t ReedSolomon::multiply (uint8_t a, uint8_t b) {
	int x = 0;

	for (int i = 7; i >= 0; i--) {
		x = (x << 1) ^ ((x >> 7) * 0x11d);
		x ^= ((b >> i) & 1) * a;
	}
  
	return static_cast<uint8_t>(x);
}

vector<uint8_t> ReedSolomon::divisor (uint8_t degree) {
	vector<uint8_t> div(degree);
	if (degree == 0) return div;
	
	div[degree - 1] = 1;
	uint8_t root = 1;

	for (int i = 0; i < degree; i++) {
		for (int j = 0; j < degree; j++) {
			div[j] = ReedSolomon::multiply(div[j], root);
			if (j + 1 < degree) div[j] ^= div[j + 1];
		}
		root = ReedSolomon::multiply(root, 2);
	}

	return div;
}

vector<uint8_t> ReedSolomon::remainder (const vector<uint8_t> &data, const vector<uint8_t> &divisor) {
	int size = divisor.size();
	vector<uint8_t> rem(size);
	if (size == 0) return rem;

	for (const uint8_t &b : data) {
		uint8_t factor = b ^ rem[0];
		rem.erase(rem.begin());
		rem.push_back(0);
		for (int i = 0; i < size; i++) rem[i] ^= ReedSolomon::multiply(divisor[i], factor);
	}
  
	return rem;
}
