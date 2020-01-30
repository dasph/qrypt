#include <utility>
#include <stdexcept>
#include "qrmodules.hpp"
#include "reedsolomon.hpp"

using namespace std;

void QrModules::reserve (int n) {
  this->_M_fill_assign(n, vector<pair<bool, bool>>(n));
}

void QrModules::set (size_t x, size_t y, bool state) {
  (*this)[y][x].first = state;
  (*this)[y][x].second = true;
}

void QrModules::QrHistory::add (int n) {
  copy_backward(this->cbegin(), this->cend() - 1, this->end());
  (*this)[0] = n;
}

int QrModules::QrHistory::count () {
  bool mid = (*this)[1] == 1 && (*this)[2] == 1 && (*this)[3] == 3 && (*this)[4] == 1 && (*this)[5] == 1;
  return ((mid && (*this)[0] > 3) ? 1 : 0) + ((mid && (*this)[6] > 3) ? 1 : 0);
}

long QrModules::getPenaltyScore () const {
  long score = 0;
  int size = this->size();

  // condition 1 and 3: adjacent modules in a row and finder-like patterns
  for (int y = 0; y < size; y++) {
    bool c = (*this)[y][0].first;
    QrHistory hst = {};
    int i = 1;

    for (int x = 1; x < size; x++) {
      if (c == (*this)[y][x].first) {
        i++;
        if (i == 5) score += QrModules::PENALTY[0];
        else if (i > 5) score++;
      } else {
        hst.add(i);
        if (!c) score += hst.count() * QrModules::PENALTY[2];

        c = (*this)[y][x].first;
        i = 1;
      }
    }
    
    if (c) hst.add(i);
    hst.add(i);
    score += hst.count() * QrModules::PENALTY[2];
  }

  // condition 1 and 3: adjacent modules in a column and finder-like patterns
  for (int x = 0; x < size; x++) {
    bool c = (*this)[0][x].first;
    QrHistory hst = {};
    int i = 1;

    for (int y = 1; y < size; y++) {
      if (c == (*this)[y][x].first) {
        i++;
        if (i == 5) score += QrModules::PENALTY[0];
        else if (i > 5) score++;
      } else {
        hst.add(i);
        if (!c) score += hst.count() * QrModules::PENALTY[2];

        c = (*this)[y][x].first;
        i = 1;
      }
    }

    if (c) hst.add(i);
    hst.add(i);
    score += hst.count() * QrModules::PENALTY[2];
  }

  // condition 2: 2x2 blocks
  for (int y = 0, n = size - 1; y < n; y++) {
    for (int x = 0; x < n; x++) {
      bool c = (*this)[y][x].first;
      if ((c == (*this)[y][x + 1].first) && (c == (*this)[y + 1][x].first) && (c == (*this)[y + 1][x + 1].first)) {
        score += QrModules::PENALTY[1];
      }
    }
  }

  // condition 4: balance of black and white
  int black = 0;
  int total = size * size;

  for (const vector<pair<bool, bool>> &row : *this) {
    for (const pair<bool, bool> &m : row) if (m.first) black++;
  }

  int k = (abs(black * 20 - total * 10) + total - 1) / total - 1;
  score += k * QrModules::PENALTY[3];

  return score;
}

int QrModules::dataModules (int ver) {
	int modules = (ver * (ver + 8) + 4) * 16;
	
	if (ver > 1) {
		int align = ver / 7 + 2;
		modules -= (25 * align - 10) * align - 55;

		if (ver > 6) modules -= 36;
	}
	
	return modules;
}

int QrModules::getCodewords (int ver, int ecl) {
	return QrModules::dataModules(ver) / 8
		- QrModules::EC_CODEWORDS[ecl][ver]
		* QrModules::NUM_OF_BLOCKS[ecl][ver];
}

vector<uint8_t> QrModules::encodeData (const vector<uint8_t> &data, int ver, int ecl) {
  int n = QrModules::NUM_OF_BLOCKS[ecl][ver];
  int eccnum = QrModules::EC_CODEWORDS[ecl][ver];
  int bytes = QrModules::dataModules(ver) / 8;
  int shortblocks = n - bytes % n;
  int shortblocklen = bytes / n;

  const vector<uint8_t> div = ReedSolomon::divisor(eccnum);
  vector<vector<uint8_t>> blocks;

  for (int i, o = i = 0; i < n; i++) {
    vector<uint8_t> tmp(data.cbegin() + o, data.cbegin() + (o + shortblocklen - eccnum + (i < shortblocks ? 0 : 1)));
    o += tmp.size();
    vector<uint8_t> ecc = ReedSolomon::remainder(tmp, div);
    if (i < shortblocks) tmp.push_back(0);
    tmp.insert(tmp.end(), ecc.cbegin(), ecc.cend());
    blocks.push_back(move(tmp));
  }

  vector<uint8_t> encoded;

  for (int i = 0, k = blocks[0].size(); i < k; i++) {
		for (int j = 0, n = blocks.size(); j < n; j++)
			if (i != shortblocklen - eccnum || j >= shortblocks) {
        encoded.push_back(blocks[j][i]);
      }
	}

  return encoded;
}

vector<int> QrModules::getAlignmentPos (int ver) {
	vector<int> vec;
	if (ver < 2) return vec;

	int size = ver * 4 + 17;
	int num = ver / 7 + 2;
	int step = (ver == 32) ? 26 : (ver * 4 + num * 2 + 1) / (num * 2 - 2) * 2;
	
	vec.assign(num, 6);
	for (int i = num - 1, pos = size - 7; i > 0; i--, pos -= step) vec[i] = pos;
	
	return vec;
}

void QrModules::setFormat (int ecl, int mask) {
  int payload = (ecl << 3) | mask;

  int end = this->size() - 1;
  int ecb = payload;
  for (int i = 0; i < 10; i++) {
    ecb = ((ecb >> 9) * 0x537) ^ (ecb << 1);
  }

  int data = ((payload << 10) | ecb) ^ 0x5412;

  for (int i = 0; i < 6; i++) this->set(8, i, (data >> i) & 1);
  for (int i = 9; i < 15; i++) this->set(14 - i, 8, (data >> i) & 1);

  this->set(8, 7, (data >> 6) & 1);
  this->set(8, 8, (data >> 7) & 1);
  this->set(7, 8, (data >> 8) & 1);

  for (int i = 0; i < 8; i++) this->set(end - i, 8, (data >> i) & 1);
  for (int i = 0; i < 7; i++) this->set(8, end - i, (data >> (14 - i)) & 1);

  // always black
  this->set(8, end - 7, true);
}

void QrModules::setVersion (int ver) {
  if (ver < 7) return;

  int offset = this->size() - 11;
  int ecb = ver;
  for (int i = 0; i < 12; i++) {
    ecb = ((ecb >> 11) * 0x1f25) ^ (ecb << 1);
  }

  int data = ver << 12 | ecb;

  for (int i = 0; i < 18; i++) {
    bool bit = (data >> i) & 1;
    int x = offset + i % 3;
    int y = i / 3;

    this->set(x, y, bit);
    this->set(y, x, bit);
  }
}

void QrModules::setData (const vector<uint8_t> &data) {
  int size = this->size();
  size_t i = 0;

  for (int r = size - 1; r > 0; r -= 2) {
    if (r == 6) r = 5;
    for (int v = 0; v < size; v++) {
      for (int k = 0; k < 2; k++) {
        bool dir = !((r + 1) & 2);
        int x = r - k;
        int y = dir ? size - v - 1 : v;
        if (!(*this)[y][x].second && i < data.size() * 8) {
          (*this)[y][x].first = (data[i >> 3] >> (7 - (i & 7))) & 1;
          i++;
        }
      }
    }
  }

  if (i != data.size() * 8) {
    throw logic_error("data was not correctly set");
  }
}

void QrModules::setFinder (int x, int y) {
  int size = this->size();
  if (x < 3 || y < 3 || x > size - 4 || y > size - 4) {
    throw out_of_range("bad position for a finder pattern");
  }

  for (int dy = -4; dy < 5; dy++) {
    for (int dx = -4; dx < 5; dx++) {
      int m = max(abs(dx), abs(dy));
      int a = x + dx;
      int b = y + dy;

      if (0 <= a && a < size && 0 <= b && b < size) {
        this->set(a, b, m != 2 && m != 4);
      }
    }
  }
}

void QrModules::setAlignment (int x, int y) {
  int end = this->size() - 3;
  if (x < 2 || y < 2 || x > end || y > end) {
    throw out_of_range("bad position for an alignment pattern");
  }

  for (int dy = -2; dy < 3; dy++) {
    for (int dx = -2; dx < 3; dx++) {
      this->set(x + dx, y + dy, max(abs(dx), abs(dy)) != 1);
    }
  }
}

void QrModules::setFunctions (int ver, int ecl, int mask) {
  int size = this->size();

  for (int i = 0; i < size; i++) {
    bool bit = !(i % 2);
    this->set(6, i, bit);
    this->set(i, 6, bit);
  }

  this->setFinder(3, 3);
  this->setFinder(3, size - 4);
  this->setFinder(size - 4, 3);

  const vector<int> pos = QrModules::getAlignmentPos(ver);
  int n = pos.size();

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++)
      if (!((i == 0 && j == 0) || (i == 0 && j == n - 1) || (i == n - 1 && j == 0))) {
        this->setAlignment(pos[i], pos[j]);
      }
  }

  this->setFormat(ecl, mask);
  this->setVersion(ver);
}

void QrModules::applyMask (int mask) {
  if (mask < 0 || mask > 7) {
    throw out_of_range("mask value");
  }

  bool inv;
  int size = this->size();

  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      if ((*this)[y][x].second) continue;

      switch (mask) {
        case 0: inv = (x + y) % 2;                    break;
        case 1: inv = y % 2;                          break;
        case 2: inv = x % 3;                          break;
        case 3: inv = (x + y) % 3;                    break;
        case 4: inv = (x / 3 + y / 2) % 2;            break;
        case 5: inv = x * y % 2 + x * y % 3;          break;
        case 6: inv = (x * y % 2 + x * y % 3) % 2;    break;
        case 7: inv = ((x + y) % 2 + x * y % 3) % 2;  break;
      }

      (*this)[y][x].first ^= !inv;
    }
  }
}

const char QrModules::EC_CODEWORDS[4][41] = {
	{ -1, 10, 16, 26, 18, 24, 16, 18, 22, 22, 26, 30, 22, 22, 24, 24, 28, 28, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28 },
	{ -1,  7, 10, 15, 20, 26, 18, 20, 24, 30, 18, 20, 24, 26, 30, 22, 24, 28, 30, 28, 28, 28, 28, 30, 30, 26, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
	{ -1, 17, 28, 22, 16, 22, 28, 26, 26, 24, 28, 24, 28, 22, 24, 24, 30, 28, 28, 26, 28, 30, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
	{ -1, 13, 22, 18, 26, 18, 24, 18, 22, 20, 24, 28, 26, 24, 20, 30, 24, 28, 28, 26, 30, 28, 30, 30, 30, 30, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }
};

const char QrModules::NUM_OF_BLOCKS[4][41] = {
	{ -1, 1, 1, 1, 2, 2, 4, 4, 4, 5, 5,  5,  8,  9,  9, 10, 10, 11, 13, 14, 16, 17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49 },
	{ -1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4,  4,  4,  4,  4,  6,  6,  6,  6,  7,  8,  8,  9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25 },
	{ -1, 1, 1, 2, 4, 4, 4, 5, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25, 25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81 },
	{ -1, 1, 1, 2, 2, 4, 4, 6, 6, 8, 8,  8, 10, 12, 16, 12, 17, 16, 18, 21, 20, 23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68 }
};
