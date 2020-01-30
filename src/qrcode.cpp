#include <sstream>
#include <stdexcept>
#include "qrcode.hpp"
#include "qrsymbol.hpp"

using namespace std;

QrCode::QrCode (const void *data, Ecc ecl) {
	this->ecl = ecl;

	// 1) convert raw input data into bits
	QrSymbol symb = QrSymbol::makeSymbol(static_cast<const char *>(data));
	
	// 2) find the minimal version
	int used;
	for (this->version = QrModules::MIN_VERSION; ; this->version++) {
		used = QrSymbol::getTotalBits(symb, this->version);
		int cap = QrModules::getCodewords(this->version, static_cast<int>(this->ecl)) * 8;

		if (used <= cap) break;
		if (this->version > QrModules::MAX_VERSION) throw length_error("input data is too big");
	}

	// 3) boost ecc
	for (Ecc n : vector<Ecc>{ Ecc::M, Ecc::Q, Ecc::H }) {
		if (used <= QrModules::getCodewords(this->version, static_cast<int>(n)) * 8) {
			this->ecl = n;
		}
	}

	// 4) convert symbol into bit string
	BitVector bv;
	bv.append(symb.getMode().getMode(), 4);
	bv.append(symb.getChars(), symb.getMode().getChars(this->version));
	bv.insert(bv.end(), symb.getData().begin(), symb.getData().end());

	// 5) terminator, padding
	size_t cap = QrModules::getCodewords(this->version, static_cast<int>(this->ecl)) * 8;
	bv.append(0, min(4, static_cast<int>(cap - bv.size())));
	bv.append(0, (8 - static_cast<int>(bv.size() % 8)) % 8);

	// 6) fill the left space with alternating bits
	for (int pad = 0xec; bv.size() < cap; pad ^= 0xec ^ 0x11) {
		bv.append(pad, 8);
	}

	// 7) pack data into big endian bytes
	vector<uint8_t> codewords(bv.size() / 8);
	for (size_t i = 0; i < bv.size(); i++) {
		codewords[i >> 3] |= (bv[i] ? 1 : 0) << (7 - (i & 7));
	}

	// -------- CONSTRUCTOR ------------
	this->size = this->version * 4 + 17;
	
	this->modules.reserve(this->size);
	this->modules.setFunctions(this->version, static_cast<int>(this->ecl), 0);

	const vector<uint8_t> encoded = QrModules::encodeData(codewords, this->version, static_cast<int>(this->ecl));
	this->modules.setData(encoded);

	// mask selection
	long min = __LONG_MAX__;
	for (int i = 0; i < 8; i++) {
		this->modules.applyMask(i);
		this->modules.setFormat(static_cast<int>(this->ecl), i);
		long penalty = this->modules.getPenaltyScore();
		if (penalty < min) {
			this->mask = i;
			min = penalty;
		}

		this->modules.applyMask(i);
	}
	this->modules.applyMask(this->mask);
	this->modules.setFormat(static_cast<int>(this->ecl), this->mask);
}

string QrCode::toSvg (int border) const {
	if (border < 0) {
		throw invalid_argument("border can not be negative");
	}

	if (border > INT32_MAX / 2 || border * 2 > INT32_MAX - this->size) {
		throw invalid_argument("border is too big");
	}

	ostringstream stream;
	stream << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 "
				 << (this->size + border * 2) << ' ' << (this->size + border * 2) << "\">\n"
				 << "\t<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n\t<path d=\"";

	for (int y = 0; y < this->size; y++) {
		for (int x = 0; x < this->size; x++)
			if (this->modules[y][x].first) {
				if (x != 0 || y != 0) stream << " ";
				stream << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
			}
	}

	stream << "\" fill=\"black\"/>\n</svg>\n";

	return stream.str();
}

int QrCode::getVersion () const {
	return this->version;
}

int QrCode::getSize () const {
	return this->size;
}

int QrCode::getMask () const {
	return this->mask;
}

QrCode::Ecc QrCode::getEcl () const {
	return this->ecl;
}
