/*************************************************************************
 *  BASF2 (Belle Analysis Framework 2)                                    *
 *  Copyright(C) 2010 - Belle II Collaboration                            *
 *                                                                        *
 *  Author: The Belle II Collaboration                                    *
 *  Contributors:  Timofey Uglov                                          *
 *                                                                        *
 *  This software is provided "as is" without any warranty.               *
 * ***********************************************************************/
#include <eklm/geoeklm/EKLMTransformationFactory.h>
#include <iostream>
#include <fstream>

using namespace Belle2;

EKLMTransformationFactory*
EKLMTransformationFactory::EKLMTransformationFactory_instance = 0;


EKLMTransformationFactory:: EKLMTransformationFactory()
{
  clear();
}
// return pointer to object
EKLMTransformationFactory* EKLMTransformationFactory::getInstance()
{
  if (EKLMTransformationFactory_instance == 0)
    EKLMTransformationFactory_instance = new EKLMTransformationFactory();
  return EKLMTransformationFactory_instance;
}

void EKLMTransformationFactory::addMatrixEntry(int endcap, int layer,
                                               int sector, int plane, int strip, G4Transform3D matrix)
{
  if (endcap < 1 || endcap > 2) {
    B2FATAL("Invalid endcap number!");
    return;
  }
  if (layer < 1 || layer > 14) {
    B2FATAL("Invalid layer number!");
    return;
  }
  if (sector < 1 || sector > 4) {
    B2FATAL("Invalid sector number!");
    return;
  }
  if (plane < 1 || plane > 2) {
    B2FATAL("Invalid plane number!");
    return;
  }
  if (strip < 1 || strip > 75) {
    B2FATAL("Invalid strip number!");
    return;
  }
  stripMatrixArray[endcap - 1][layer - 1][sector - 1][plane - 1][strip - 1] =
    matrix;
}

void EKLMTransformationFactory::addLengthEntry(int strip, double length)
{
  if (strip < 1 || strip > 75) {
    B2FATAL("Invalid strip number!");
    return ;
  }
  //  std::cout << strip << " " << length << " " << stripLengthArray[strip - 1] << std::endl;
  if (stripLengthArray[strip - 1] != 0 &&
      (int)stripLengthArray[strip - 1] != (int)length) {
    B2FATAL("Strip length mismatch!");
    return ;
  }
  stripLengthArray[strip - 1] = length;
}

double EKLMTransformationFactory::getStripLength(int strip)
{
  if (strip < 1 || strip > 75) {
    B2FATAL("Invalid strip number!");
    return 0;
  }
  return stripLengthArray[strip - 1];
}


G4Transform3D EKLMTransformationFactory::getTransformation(int endcap,
                                                           int layer,
                                                           int sector,
                                                           int plane,
                                                           int strip)
{
  if (endcap < 1 || endcap > 2) {
    B2FATAL("Invalid endcap number!");
    return G4Transform3D();
  }
  if (layer < 1 || layer > 14) {
    B2FATAL("Invalid layer number!");
    return G4Transform3D();
  }
  if (sector < 1 || sector > 4) {
    B2FATAL("Invalid sector number!");
    return G4Transform3D();
  }
  if (plane < 1 || plane > 2) {
    B2FATAL("Invalid plane number!");
    return G4Transform3D();
  }
  if (strip < 1 || strip > 75) {
    B2FATAL("Invalid strip number!");
    return G4Transform3D();
  }
  return
    stripMatrixArray[endcap - 1][layer - 1][sector - 1][plane - 1][strip - 1];
}

void EKLMTransformationFactory::readFromFile(const char* filename)
{
  //  std::cout << " ------------ Q" << std::endl;
  std::ifstream in(filename);
  for (int endcap = 1; endcap <= 2; endcap++)
    for (int layer = 1; layer <= 14; layer++)
      for (int sector = 1; sector <= 4; sector++)
        for (int plane = 1; plane <= 2; plane++)
          for (int strip = 1; strip <= 75; strip++) {
            CLHEP::HepRotation rot;
            double xx = 0;
            double xy = 0;
            double xz = 0;

            double yx = 0;
            double yy = 0;
            double yz = 0;

            double zx = 0;
            double zy = 0;
            double zz = 0;

            double dx = 0;
            double dy = 0;
            double dz = 0;

            in >> xx >> xy >> xz;
            in >> yx >> yy >> yz;
            in >> zx >> zy >> zz;

            in >> dx >> dy >> dz;


            rot.set(CLHEP::HepRep3x3(xx, xy, xz, yx, yy, yz, zx, zy, zz));
            CLHEP::Hep3Vector vec(dx, dy, dz);

            int endcap1 = 0;
            int layer1 = 0;
            int sector1 = 0;
            int plane1 = 0;
            int strip1 = 0;

            in >> endcap1 >> layer1 >> sector1 >> plane1 >> strip1;
            addMatrixEntry(endcap1, layer1, sector1, plane1, strip1,
                           G4Transform3D(rot, vec));

          }

  for (int i = 1; i <= 75; i++) {
    double length;
    int strip;
    in >> strip >> length;
    addLengthEntry(strip, length);
  }

}

void EKLMTransformationFactory::readFromXMLFile(
  const GearDir& stripTransformationDir)
{
  GearDir dir(stripTransformationDir);
  dir.append("/StripTransformationMatrixAndLengthDatabase");
  for (int i = 0; i < 2 * 14 * 4 * 2 * 75; i++) {
    GearDir StripMatrixContent(dir);
    StripMatrixContent.append((boost::format("/StripMatrix[%1%]") %
                               (i + 1)).str());

    CLHEP::HepRotation rot;
    rot.set(CLHEP::HepRep3x3(StripMatrixContent.getDouble("XX"),
                             StripMatrixContent.getDouble("XY"),
                             StripMatrixContent.getDouble("XZ"),
                             StripMatrixContent.getDouble("YX"),
                             StripMatrixContent.getDouble("YY"),
                             StripMatrixContent.getDouble("YZ"),
                             StripMatrixContent.getDouble("ZX"),
                             StripMatrixContent.getDouble("ZY"),
                             StripMatrixContent.getDouble("ZZ")));

    CLHEP::Hep3Vector vec(StripMatrixContent.getDouble("DX"),
                          StripMatrixContent.getDouble("DY"),
                          StripMatrixContent.getDouble("DZ"));


    addMatrixEntry(
      StripMatrixContent.getInt("Endcap"),
      StripMatrixContent.getInt("Layer"),
      StripMatrixContent.getInt("Sector"),
      StripMatrixContent.getInt("Plane"),
      StripMatrixContent.getInt("Strip"),
      G4Transform3D(rot, vec)
    );
  }

  for (int i = 1; i <= 75; i++) {
    GearDir StripLengthContent(dir);
    StripLengthContent.append((boost::format("/StripLength[%1%]") %
                               (i)).str());

    addLengthEntry(i, StripLengthContent.getInt("Length"));
  }
}


void EKLMTransformationFactory::writeToXMLFile(std::string filename)
{
  writeToXMLFile(filename.c_str());
}

void EKLMTransformationFactory::writeToFile(const char* filename)const
{
  //  std::cout << " ------------ Q " << filename << std::endl;
  std::ofstream out(filename, std::ios_base::trunc);
  for (int endcap = 1; endcap <= 2; endcap++)
    for (int layer = 1; layer <= 14; layer++)
      for (int sector = 1; sector <= 4; sector++)
        for (int plane = 1; plane <= 2; plane++)
          for (int strip = 1; strip <= 75; strip++) {
            G4Transform3D matrix = stripMatrixArray[endcap - 1][layer - 1]
                                   [sector - 1][plane - 1][strip - 1];
            out << matrix.xx() << " ";
            out << matrix.xy() << " ";
            out << matrix.xz() << " ";
            out << matrix.yx() << " ";
            out << matrix.yy() << " ";
            out << matrix.yz() << " ";
            out << matrix.zx() << " ";
            out << matrix.zy() << " ";
            out << matrix.zz() << " ";
            out << matrix.dx() << " ";
            out << matrix.dy() << " ";
            out << matrix.dz() << " ";

            out << " " << endcap;
            out << " " << layer;
            out << " " << sector;
            out << " " << plane;
            out << " " << strip << std::endl;
          }

  for (int strip = 1; strip <= 75; strip++)
    out << strip << " " << stripLengthArray[strip - 1] << std::endl;
}


void EKLMTransformationFactory::writeToXMLFile(char* filename)
{
  std::ofstream out(filename, std::ios_base::trunc);
  out
      << "<!--  this file is authomatically created by EKLM geometry creator-->"
      << std::endl;
  out << "<!--  do not manuallly change this file -->" << std::endl;
  out << "<StripTransformationMatrixAndLengthDatabase>" << std::endl;

  int i = 0;
  for (int endcap = 1; endcap <= 2; endcap++)
    for (int layer = 1; layer <= 14; layer++)
      for (int sector = 1; sector <= 4; sector++)
        for (int plane = 1; plane <= 2; plane++)
          for (int strip = 1; strip <= 75; strip++) {
            G4Transform3D matrix = stripMatrixArray[endcap - 1][layer - 1]
                                   [sector - 1][plane - 1][strip - 1];
            out << "<StripMatrix id=\"" << i++ << "\">" << std::endl;
            out << "        <XX  desc=\"XX matrix element\">"
                << matrix.xx() << "</XX>" << std::endl;
            out << "        <XY  desc=\"XY matrix element\">"
                << matrix.xy() << "</XY>" << std::endl;
            out << "        <XZ  desc=\"XZ matrix element\">"
                << matrix.xz() << "</XZ>" << std::endl;

            out << "        <YX  desc=\"YX matrix element\">"
                << matrix.yx() << "</YX>" << std::endl;
            out << "        <YY  desc=\"YY matrix element\">"
                << matrix.yy() << "</YY>" << std::endl;
            out << "        <YZ  desc=\"YZ matrix element\">"
                << matrix.yz() << "</YZ>" << std::endl;

            out << "        <ZX  desc=\"ZX matrix element\">"
                << matrix.zx() << "</ZX>" << std::endl;
            out << "        <ZY  desc=\"ZY matrix element\">"
                << matrix.zy() << "</ZY>" << std::endl;
            out << "        <ZZ  desc=\"ZZ matrix element\">"
                << matrix.zz() << "</ZZ>" << std::endl;

            out << "        <DX  desc=\"DX matrix element\">"
                << matrix.dx() << "</DX>" << std::endl;
            out << "        <DY  desc=\"DY matrix element\">"
                << matrix.dy() << "</DY>" << std::endl;
            out << "        <DZ  desc=\"DZ matrix element\">"
                << matrix.dz() << "</DZ>" << std::endl;


            out << "        <Endcap  desc=\" Endcap # \">"
                << endcap << "</Endcap>" << std::endl;
            out << "        <Layer  desc=\" Layer # \">"
                << layer << "</Layer>" << std::endl;
            out << "        <Sector  desc=\" Sector # \">"
                << sector << "</Sector>" << std::endl;
            out << "        <Plane  desc=\" Plane # \">"
                << plane << "</Plane>" << std::endl;
            out << "        <Strip  desc=\" Strip # \">"
                << strip << "</Strip>" << std::endl;

            out << "</StripMatrix>" << std::endl;
          }

  for (int strip = 1; strip <= 75; strip++) {
    out << "<StripLength id=\"" << strip << "\">" << std::endl;
    out << "        <Length  desc=\"Strip Length\" unit=\"cm\">"
        << stripLengthArray[strip - 1] << "</Length>" << std::endl;
    out << "</StripLength>" << std::endl;
  }
  out << "</StripTransformationMatrixAndLengthDatabase>" << std::endl;
}


void EKLMTransformationFactory::clear()
{
  for (int strip = 1; strip <= 75; strip++) {
    stripLengthArray[strip - 1] = 0;
    for (int endcap = 1; endcap <= 2; endcap++)
      for (int layer = 1; layer <= 14; layer++)
        for (int sector = 1; sector <= 4; sector++)
          for (int plane = 1; plane <= 2; plane++)
            stripMatrixArray[endcap - 1][layer - 1][sector - 1][plane - 1]
            [strip - 1] = G4Transform3D();
  }
}




