#pragma once
#include <string>
#include "qrsymbol.hpp"
#include "qrmodules.hpp"

class QrCode final {
  public:
    enum class Ecc { U = -1, M, L, H, Q };

  private:
    int version;
    int size;
    int mask;
    Ecc ecl;
    QrModules modules;

  public:
    QrCode (const void *data, Ecc ecl = Ecc::M);

    std::string toSvg (int border) const;
    int getVersion () const;
    int getSize () const;
    int getMask () const;
    Ecc getEcl () const;
};
