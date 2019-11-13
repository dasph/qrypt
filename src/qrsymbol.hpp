#pragma once
#include <vector>
#include "bitvector.hpp"

class QrSymbol final {
  public:
    class Mode final {
      private:
        const int data;
        Mode (int payload) : data(payload) {};
      public:
        static const Mode NUMERIC;
        static const Mode ALPHANUMERIC;
        static const Mode BYTE;
        static const Mode KANJI;
        static const Mode ECI;

        int getMode () const { return this->data >> 24; };
        int getChars (const int &ver) const;
    };

  private:
    const Mode mode;
    const size_t length;
    const std::vector<bool> data;

    static const char *ALPHANUMERIC_CHARSET;
  
  public:
    static QrSymbol makeNumeric (const char *data);
    static QrSymbol makeAlphanumeric (const char *data);
    static QrSymbol makeBytes (const char *data);
    static QrSymbol makeEci (const size_t &value);
    static QrSymbol makeSymbol(const char *data);
    static size_t getTotalBits (const QrSymbol &sym, const int &ver);

    static bool isNumeric (const char *data);
    static bool isAlphanumeric (const char *data);

    QrSymbol (const Mode &mode, const size_t &length, const BitVector &data) :
      mode(mode), length(length), data(data) {};
    QrSymbol (const Mode &mode, const size_t &length, BitVector &&data) :
      mode(mode), length(length), data(std::move(data)) {};

    const Mode &getMode() const { return this->mode; };
    const size_t &getChars() const { return this->length; };
    const std::vector<bool> &getData() const { return this->data; };
};
