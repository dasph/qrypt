#include <iostream>
#include <string.h>
#include "options.hpp"

char Options::readOption () {
  return getopt_long(this->argc, this->argv, Options::shortOpts, Options::longOpts, nullptr);
}

Options::Format Options::_format () const {
  return Options::parseExt(this->_output.substr(this->_output.find_last_of('.') + 1).c_str());
}

void Options::error (const char *reason) const {
  std::cerr << this->argv[0] << ": invalid " << reason  << " - \'" << (optarg ? optarg : this->_data) << "\'\n";
  exit(1);
}

void Options::help () const {
  std::cout << this->argv[0] << ": usage " << this->argv[0] << " <input data> "
            << "[-e L|M|Q|H] [-f png|jpg|svg] [-s 1-255] [-o name] [-w] [-h]\n\t"
            << "-e, --errorcorrection=M\t\terror correction level\n\t"
            << "-f, --format=svg\t\toutput format\n\t"
            << "-s, --scale=1\t\t\tpixels per module\n\t"
            << "-b, --border=0\t\t\tamount of border modules\n\t"
            << "-o, --output=qrcode.png\t\toutput name\n\t"
            << "-w, --wifi\t\t\twifi access point entry <\"name;password\">\n";
  exit(0);
}

QrCode::Ecc Options::parseEcc (const char *c) {
  // if (*c > 47 && *c < 52) return QrCode::Ecc(atoi(c) - 48);
  return QrCode::Ecc(*c == 'L' ? 1 : *c == 'M' ? 0 : *c == 'Q' ? 3 : *c == 'H' ? 2 : -1);
}

Options::Format Options::parseExt (const char *ext) {
  if (*ext > 47 && *ext < 51) return Options::Format(atoi(ext) - 48);
  return Options::Format(!strcmp(ext, "svg") ? 0 : !strcmp(ext, "png") ? 1 : !strcmp(ext, "jpg") ? 2 : -1);
}

std::pair<std::string, std::string> Options::parseOutput (const char *output) {
  std::string str = output;
  size_t dot = str.find_last_of('.');
  if (dot == std::string::npos) return { str, "" };
  
  const char *ext = str.substr(dot + 1).c_str();
  const bool valid = Options::parseExt(ext) != Options::Format::undefined;
  return { str.substr(0, dot), (valid ? str.substr(dot + 1) : "svg") };
}

std::string Options::renameOutput (std::string str, const char *name, const char *ext) {
  if (name) str = name + str.substr(str.find_last_of('.'));
  if (ext) str = str.substr(0, str.find_last_of('.') + 1) + ext;
  return str;
}

Options::Options (const int &c, char *const *v) : argc(c), argv(v) {
  char ch;
  while (ch = this->readOption(), ch != -1) {
    switch (ch) {
      case 'e': {
        this->_ecc = Options::parseEcc(optarg);
        if (this->_ecc == QrCode::Ecc::U) this->error("error correction level");
      } break;
      case 'f': {
        this->flags.format = true;
        if (Options::parseExt(optarg) == Format::undefined) this->error("file format");
        this->_output = Options::renameOutput(this->_output, nullptr, optarg);
      } break;
      case 's': {
        this->_scale = static_cast<unsigned char>(atoi(optarg));
        if (!this->_scale) this->error("scale factor");
      } break;
      case 'b': {
        this->_border = atoi(optarg);
      } break;
      case 'o': {
        auto [name, ext] = Options::parseOutput(optarg);
        const char *format = ext.empty() ? nullptr : this->flags.format ? nullptr : ext.c_str();
        this->_output = Options::renameOutput(this->_output, name.c_str(), format);
      } break;
      case 'w': this->flags.wifi = true; break;
      case 'h': this->help();
      case ':': case '?': default: exit(1);
    }
  }

  for (int i = optind; i < c; i++) {
    this->_data += v[i];
    if (i + 1 < c) this->_data += " ";
  }

  if (this->_data == "-") {
    char c;
    this->_data.clear();

    while (std::cin.read(&c, 1)) this->_data += c;
  }

  if (this->flags.wifi) {
    size_t p = this->_data.find(';');
    if (p == std::string::npos) this->error("wifi syntax");

    this->_data = "WIFI:S:" + this->_data.substr(0, p) + ";T:WPA;P:" + this->_data.substr(p + 1) + ";;";
  }

  if (this->_data.empty()) this->help();
}
