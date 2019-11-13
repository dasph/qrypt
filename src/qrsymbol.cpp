#include <cstring>
#include <stdexcept>
#include "qrsymbol.hpp"

int QrSymbol::Mode::getChars (const int &ver) const {
  return this->data >> (((size_t)((ver & 0x3f) + 7) / 17) << 3) & 0x1f;
}

const QrSymbol::Mode QrSymbol::Mode::NUMERIC (0x10e0c0a);
const QrSymbol::Mode QrSymbol::Mode::ALPHANUMERIC (0x20d0b09);
const QrSymbol::Mode QrSymbol::Mode::BYTE (0x4101008);
const QrSymbol::Mode QrSymbol::Mode::KANJI (0x80c0a08);
const QrSymbol::Mode QrSymbol::Mode::ECI (0x7000000);

const char *QrSymbol::ALPHANUMERIC_CHARSET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

QrSymbol QrSymbol::makeNumeric (const char *data) {
  BitVector bv;
  size_t i, acc = 0;

  for (i = 1; *data != '\0'; i++, data++) {
    acc = 10 * acc + (*data - '0');
    if (!(i % 3)) {
      bv.append(acc, 10);
      acc = 0;
    }
  }
  if (--i % 3) bv.append(acc, 3 * (i % 3) + 1);

  return QrSymbol(QrSymbol::Mode::NUMERIC, i, std::move(bv));
}

QrSymbol QrSymbol::makeAlphanumeric (const char *data) {
  BitVector bv;
  size_t i, acc;

  for (i = acc = 0; *data != '\0'; i++, data++) {
    const char *ptr = std::strchr(ALPHANUMERIC_CHARSET, *data);
    acc = 45 * acc + (ptr - QrSymbol::ALPHANUMERIC_CHARSET);
    if (i & 1) {
      bv.append(acc, 11);
      acc = 0;
    }
  }
  if (acc) bv.append(acc, 6);

  return QrSymbol(QrSymbol::Mode::ALPHANUMERIC, i, std::move(bv));
}

QrSymbol QrSymbol::makeBytes (const char *data) {
  const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);
  BitVector bv;
  size_t i = 0;
  
  for (; *(ptr + i) != '\0'; i++) bv.append(*(ptr + i), 8);

  return QrSymbol(QrSymbol::Mode::BYTE, i, std::move(bv));
}

QrSymbol makeEci (const size_t &value) {
  BitVector bv;

  if (value < 0x80) {
    bv.append(value, 8);
  } else if (value < 0x4000) {
    bv.append(2, 2);
    bv.append(value, 14);
  } else if (value < 0xf4240) {
    bv.append(6, 3);
    bv.append(value, 21);
  } else throw std::out_of_range("ECI value out of range");

  return QrSymbol(QrSymbol::Mode::ECI, 0, std::move(bv));
}

QrSymbol QrSymbol::makeSymbol(const char *data) {
  if (QrSymbol::isNumeric(data)) return QrSymbol::makeNumeric(data);
  else if (QrSymbol::isAlphanumeric(data)) return QrSymbol::makeAlphanumeric(data);
  return QrSymbol::makeBytes(data);
}

size_t QrSymbol::getTotalBits (const QrSymbol &sym, const int &ver) {
  return 4 + sym.mode.getChars(ver) + sym.data.size();
}

bool QrSymbol::isNumeric (const char *data) {
  for (; *data != '\0'; data++) if (*data < '0' || *data > '9') return false;
  return true;
}

bool QrSymbol::isAlphanumeric (const char *data) {
  for (; *data != '\0'; data++) {
    if (std::strchr(QrSymbol::ALPHANUMERIC_CHARSET, *data) == nullptr) return false;
  }
  return true;
}
