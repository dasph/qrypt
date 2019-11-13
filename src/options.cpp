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
  std::cerr << this->argv[0] << ": invalid " << reason  << " - \'" << optarg << "\'\n";
  exit(1);
}

void Options::help () const {
  std::cout << this->argv[0] << ": usage " << this->argv[0] << " <input data> "
            << "[-e L|M|Q|H] [-f png|jpg|svg] [-s 1-255] [-o name] [-w] [-h]\n\t"
            << "-e, --errorcorrection=M\t\terror correction level\n\t"
            << "-f, --format=png\t\toutput format\n\t"
            << "-s, --scale=1\t\t\tscale factor\n\t"
            << "-o, --output=qrcode.png\t\toutput name\n\t"
            << "-w, --wifi\t\t\twifi access point entry, input has to be [name:password]\n";
  exit(0);
}

Options::Ecc Options::parseEcc (const char *c) {
  if (*c > 47 && *c < 52) return Options::Ecc(atoi(c) - 48);
  return Options::Ecc(*c == 'L' ? 0 : *c == 'M' ? 1 : *c == 'Q' ? 2 : *c == 'H' ? 3 : -1);
}

Options::Format Options::parseExt (const char *ext) {
  if (*ext > 47 && *ext < 51) return Options::Format(atoi(ext) - 48);
  return Options::Format(!strcmp(ext, "png") ? 0 : !strcmp(ext, "jpg") ? 1 : !strcmp(ext, "svg") ? 2 : -1);
}

std::pair<std::string, std::string> Options::parseOutput (const char *output) {
  std::string str = output;
  size_t dot = str.find_last_of('.');
  if (dot == std::string::npos) return { str, "" };
  
  const char *ext = str.substr(dot + 1).c_str();
  const bool valid = Options::parseExt(ext) != Options::Format::undefined;
  return { str.substr(0, dot), (valid ? str.substr(dot + 1) : "png") };
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
        if (this->_ecc == Ecc::undefined) this->error("error correction level");
      } break;
      case 'f': {
        this->flags.format = true;
        if (Options::parseExt(optarg) == Format::undefined) this->error("file format");
        this->_output = Options::renameOutput(this->_output, nullptr, optarg);
      } break;
      case 's': {
        this->_scale = atoi(optarg);
        if (!this->_scale) this->error("scale factor");
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
    this->data += v[i];
    if (i + 1 < c) this->data += " ";
  }

  if (this->data.empty()) this->help();
  if (this->data == "-") this->flags.stream = true;
}
