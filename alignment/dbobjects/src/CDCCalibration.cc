#include <alignment/dbobjects/CDCCalibration.h>

using namespace Belle2;

ClassImp(CDCCalibration);
/*
void CDCCalibration::readWireAlignmentTextFile(std::string fileName) {
  fileName = FileSystem::findFile(fileName);
  std::ifstream ifs(fileName);

  int iL(0), iC(0);
  const int np = 3;
  double back[np], fwrd[np], tension;
  unsigned nRead = 0;

  while (true) {
    ifs >> iL >> iC;
    for (int i = 0; i < np; ++i) {
      ifs >> back[i];
    }
    for (int i = 0; i < np; ++i) {
      ifs >> fwrd[i];
    }
    ifs >> tension;

    if (ifs.eof()) break;

    ++nRead;
    WireID wire(iL, iC);
    set(wire, wireBwdX, back[0]);
    set(wire, wireBwdY, back[1]);
    set(wire, wireBwdZ, back[2]);
    set(wire, wireFwdDx, fwrd[0]);
    set(wire, wireFwdDy, fwrd[1]);
    set(wire, wireFwdDz, fwrd[2]);

    set(wire, wireTension, tension);

    //TODO: This is temporary until we have separete layer alignment
    WireID layer(wire.getISuperLayer(), wire.getILayer(), 511);
    set(layer, layerX, back[0]);
    set(layer, layerY, back[1]);

  }
}

void CDCCalibration::writeWireAlignmentTextFile(std::string fileName) {
  std::ofstream file(fileName);
  file.close();
}
*/