#pragma once
#include <string>
#include <getopt.h>
#include "./qrcode.hpp"

class Options final {
  public:
    enum class Format { undefined = -1, svg, png };

  private:
    const int argc;
    char *const *argv;
    std::string _data {};
    struct { bool format, wifi; } flags {};
    int _border = 0;
    QrCode::Ecc _ecc = QrCode::Ecc::M;
    std::string _output = "qrcode.svg";

    static constexpr const char *shortOpts = "e:f:s:b:o:wh";
    static constexpr const option longOpts[] = {
      { "errorcorrection", 1, 0, 'e'},
      { "format", 1, 0, 'f' },
      { "border", 1, 0, 'b' },
      { "output", 1, 0, 'o' },
      { "wifi", 0, 0, 'w' },
      { "help", 0, 0, 'h' },
      { 0, 0, 0, 0 }
    };
    
    char readOption ();
    Format _format () const;
    void error (const char *reason) const;
    void help () const;
    
    static QrCode::Ecc parseEcc (const char *ecc);
    static Format parseExt (const char *ext);
    static std::pair<std::string, std::string> parseOutput (const char *name);
    static std::string renameOutput (std::string str, const char *name, const char *ext);

  public:
    Options (const int &argc, char *const *argv);
    
    std::string data () const { return this->_data; };
    Format format () const { return this->_format(); };
    QrCode::Ecc ecc () const { return this->_ecc; };
    int border () const { return this->_border; };
    const char *output () const { return this->_output.c_str(); };
};
