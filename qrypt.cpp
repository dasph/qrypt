#include <fstream>
#include "src/options.hpp"

using namespace std;

int main (const int argc, char *const *argv) {
  Options opts(argc, argv);

  QrCode qr = QrCode(opts.data().c_str(), opts.ecc());

  ofstream output(opts.output(), ofstream::out);

  output << qr.toSvg(opts.border());

  output.close();

  return 0;
}
