#include <fstream>
#include <iostream>
#include "src/options.hpp"

using namespace std;

// DEBUG ----------------------------
// void printOpts (const Options &opts);
// ----------------------------------

int main (const int argc, char *const *argv) {
  Options opts(argc, argv);

  QrCode qr = QrCode(opts.data().c_str(), opts.ecc());

  ofstream svg(opts.output(), ofstream::out);
  svg << qr.toSvg(opts.border());

  svg.close();

  return 0;
}





























// void printOpts (const Options &opts) {
//   cout << "---------OPTIONS---------"
//        << "\nEcc:\t" << (int)opts.ecc() << "\nFormat:\t" << (int)opts.format()
//        << "\nOutput:\t" << opts.output() << "\nScale:\t" << (int)opts.scale()
//        << "\nWiFi:\t" << opts.wifi() << "\nStream:\t" << opts.stream()
//        << "\n-------------------------\n";
// }
