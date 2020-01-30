#pragma once
#include <array>
#include <vector>
#include <cstdint>

class QrModules final : public std::vector<std::vector<std::pair<bool, bool>>> {
  private:
    void set (size_t x, size_t y, bool state = true);
    
    class QrHistory final : public std::array<int, 7> {
      public:
        void add (int n);
        int count ();
    };

    static constexpr int PENALTY[] = { 3, 3, 40, 10 };

  public:
    void reserve (int n);

    static std::vector<int> getAlignmentPos (int ver);

    static int dataModules (int ver);
    static int getCodewords (int ver, int ecl);

    static std::vector<uint8_t> encodeData (const std::vector<uint8_t> &data, int ver, int ecl);

    long getPenaltyScore () const;

    void setFormat (int ecl, int mask);
    void setVersion (int ver);
    void setData (const std::vector<uint8_t> &data);
    void setFinder (int x, int y);
    void setAlignment (int x, int y);
    void setFunctions (int ver, int ecl, int mask);

    void applyMask (int mask);

    static constexpr int MIN_VERSION = 1;
    static constexpr int MAX_VERSION = 40;
    static const char EC_CODEWORDS[4][41];
    static const char NUM_OF_BLOCKS[4][41];
};
