#pragma once
#include <string>
#include <getopt.h>

class Options final {
  public:
    enum class Ecc { undefined = -1, L, M, Q, H };
    enum class Format { undefined = -1, png, jpg, svg };

  private:
    const int argc;
    char *const *argv;
    std::string data {};
    struct { bool format, wifi, stream; } flags {};
    unsigned char _scale = 1;
    Ecc _ecc = Ecc::M;
    std::string _output = "qrcode.png";

    static constexpr const char *shortOpts = "e:f:s:o:wh";
    static constexpr const option longOpts[] = {
      { "errorcorrection", 1, 0, 'e'},
      { "format", 1, 0, 'f' },
      { "scale", 1, 0, 's' },
      { "output", 1, 0, 'o' },
      { "wifi", 0, 0, 'w' },
      { "help", 0, 0, 'h' },
      { 0, 0, 0, 0 }
    };
    
    char readOption ();
    Format _format () const;
    void error (const char *reason) const;
    void help () const;
    
    static Ecc parseEcc (const char *ecc);
    static Format parseExt (const char *ext);
    static std::pair<std::string, std::string> parseOutput (const char *name);
    static std::string renameOutput (std::string str, const char *name, const char *ext);

  public:
    Options (const int &argc, char *const *argv);
    Format format () const { return this->_format(); };
    const Ecc &ecc () const { return this->_ecc; };
    const unsigned char &scale () const { return this->_scale; };
    const char *output () const { return this->_output.c_str(); };
    const bool &wifi () const { return this->flags.wifi; };
    const bool &stream () const { return this->flags.stream; };
};
