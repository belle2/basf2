/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHGeoAerogelPlane.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <TVector2.h>

using namespace std;
using namespace Belle2;

bool ARICHGeoAerogelPlane::isConsistent() const
{

  if (m_layers.size() == 0 || m_r.size() == 0 || m_dPhi.size() == 0) return false;
  if (m_tileGap < 0 || m_innerR < 0) return false;
  if (m_innerR > m_outerR) return false;
  if (m_thickness < 0 || m_wallThickness < 0 || m_wallHeight < 0) return false;

  if (abs(getWallHeight() - getLayerThickness(1) - getLayerThickness(2)) > 1.0e-10) {
    cout << "getWallHeight()      = " << getWallHeight() << endl
         << "getLayerThickness(1) = " << getLayerThickness(1) << endl
         << "getLayerThickness(2) = " << getLayerThickness(2) << endl;
    cout << "getWallHeight() - getLayerThickness(1) - getLayerThickness(2) = "
         << getWallHeight() - getLayerThickness(1) - getLayerThickness(2) << endl;
    B2ERROR("Data of the ARICHGeoAerogelPlane is inconsisten : --> abs (getWallHeight() - getLayerThickness(1) - getLayerThickness(2)) > 1.0e-10");
  }

  if (getFullAerogelMaterialDescriptionKey() == 1) {
    const int nn = 10;// Please note it can be any number bigger than number of aero ringth (now it is 4)
    int nAeroSlotsPerRing[nn];
    // Define the array with zeros
    for (int i = 0; i < nn; i++)
      nAeroSlotsPerRing[i] = 0;
    for (unsigned i = 0; i < m_tiles.size(); i++)
      if (m_tiles.at(i).layer == 0)
        nAeroSlotsPerRing[m_tiles.at(i).ring - 1] += 1;
    //Check is number of aerogel slots is consistent with number of entries for tiles properties.
    for (unsigned i = 0; i < getNAeroSlotsIndividualRing().size(); i++) {
      //cout<<"getNAeroSlotsIndividualRing().at(i) = "<<getNAeroSlotsIndividualRing().at(i)<<endl
      //  <<"nAeroSlotsPerRing[i]                = "<<nAeroSlotsPerRing[i]<<endl;
      B2ASSERT("Data of the ARICHGeoAerogelPlane is inconsisten : --> getNAeroSlotsIndividualRing().at(i) != nAeroSlotsPerRing[i]",
               getNAeroSlotsIndividualRing().at(i) == nAeroSlotsPerRing[i]);
    }
  }

  return true;
}

void ARICHGeoAerogelPlane::print(const std::string& title) const
{
  ARICHGeoBase::print(title);

  cout << "Full aerogel material description key : " << getFullAerogelMaterialDescriptionKey() << endl;

  cout << "Nume of aerogel rings and  number of aerogel slots in individual ring " << endl;

  cout << setw(5) << "Ring"
       << setw(15) << "Aerogel slots" << endl;
  for (unsigned i = 0; i < getNAeroSlotsIndividualRing().size(); i++) {
    cout << setw(5) << i + 1
         << setw(15) << getNAeroSlotsIndividualRing().at(i) << endl;
  }

  cout << "Imaginary tube thikness just after aerogel layers : " << getImgTubeThickness() << endl;
  cout << "Minimum thickness of the compensation volume with ARICH air : " << getCompensationARICHairVolumeThick_min() << endl;

  cout << "Aerogel layers " << endl;

  int nLayer = getNLayers();
  for (int iLayer = 1; iLayer < nLayer + 1; iLayer++) {
    cout << " layer " << iLayer  << endl;
    cout << "  thickness: " << getLayerThickness(iLayer) << " " << s_unitName << ", ref. index: " << getLayerRefIndex(
           iLayer) << ", transm. length: " << getLayerTrLength(iLayer) << " " << s_unitName << ", material: " << getLayerMaterial(
           iLayer)  << endl;
  }
  if (getFullAerogelMaterialDescriptionKey() == 1) {
    cout << " Please note full aerogel material description key is set to 1 " << endl;
    cout << " The name of each particular tile have this form : Aero1_serA090_rin4_col26 " << endl;
    cout << " serialID = A090; ring = 4; column = 26; layer = up; " << endl;
    cout << " The name of each particular tile have this form : Aero2_serB162_rin4_col14 " << endl;
    cout << " serialID = B162; ring = 4  column = 15; layer = down; " << endl;
  }

  int nRing = getNRings();
  cout << " Aluminum walls between tiles parameters" << endl;
  cout << "  thickness: " << getWallThickness()  << " " << s_unitName << endl;
  cout << "  height: " << getWallHeight() << " " << s_unitName << endl;
  for (int iRing = 1; iRing < nRing + 1; iRing++) {

    if (iRing < nRing) cout << "  ring: " << iRing << " has inner radius of " <<  getRingRadius(iRing) << " " << s_unitName << " and "
                              << static_cast<int>(2 * M_PI / getRingDPhi(iRing) + 0.5) << " aerogel tile slots." << endl;
    else  cout << "  ring: " << iRing << " has inner radius of " <<  getRingRadius(iRing) << " " << s_unitName <<
                 " (this is the outer ring)" << endl;
  }

  cout << " gap between tile and aluminum wall: " << getTileGap() << " " << s_unitName << endl;
  cout << " inner radius of support plate:      " <<  getSupportInnerR() << " " << s_unitName << endl;
  cout << " outer radius of support plate:      " <<  getSupportOuterR() << " " << s_unitName << endl;
  cout << " thickness of support plate:         " <<  getSupportThickness() << " " << s_unitName << endl;
  cout << " material of support plate:          " <<  getSupportMaterial() << endl;

  cout << "Positioning parameters (in local ARICH frame)" << endl;
  ARICHGeoBase::printPlacement(m_x, m_y, m_z, m_rx, m_ry, m_rz);

  if (isSimple()) {
    cout << endl << "Simple aerogel configuration is set!" << endl;
    cout << " Parameters:" << endl;
    cout << "  xSize: " << m_simpleParams.at(0) << ", ySize: " <<   m_simpleParams.at(1) << ", xPos: " << m_simpleParams.at(
           2) << ", xPos: " << m_simpleParams.at(3) << ", zRot: " << m_simpleParams.at(4) << endl;
  }

}

unsigned ARICHGeoAerogelPlane::getAerogelTileID(double x, double y) const
{

  double r = sqrt(x * x + y * y);

  int nTile = 0;
  for (unsigned iRing = 0; iRing < getNRings(); iRing++) {
    if (r > m_r[iRing] + m_wallThickness + m_tileGap) {
      if (iRing == 0) continue;
      nTile += static_cast<int>(2 * M_PI / m_dPhi[iRing - 1] + 0.5);
    } else {
      if (iRing == 0) return 0;
      if (r >  m_r[iRing] - m_tileGap) return 0;
      double phi = atan2(y, x);
      if (phi < 0) phi += 2 * M_PI;
      int nphi = static_cast<int>(phi / m_dPhi[iRing - 1]);
      double gapPhi = (m_tileGap + m_wallThickness / 2.) / m_r[iRing];
      if (phi < nphi * m_dPhi[iRing - 1] + gapPhi || phi > (nphi + 1)* m_dPhi[iRing - 1] - gapPhi) return 0;
      nTile += nphi;
      return nTile + 1;
    }

  }
  return 0;
}

unsigned ARICHGeoAerogelPlane::getTileParameters(int ring, int column, int layerN, double& n, double& transmL, double& thick,
                                                 std::string& material) const
{
  n = 0.0;
  transmL = 0.0;
  thick = 0.0;
  material = (char*)"NULL";
  for (unsigned i = 0; i < m_tiles.size(); i++) {
    if (m_tiles.at(i).ring == ring) {
      if (m_tiles.at(i).column == column) {
        if (m_tiles.at(i).layer == layerN) {
          n = m_tiles.at(i).n;
          transmL = m_tiles.at(i).transmL;
          thick = m_tiles.at(i).thick;
          material = m_tiles.at(i).material;
          return i;
        }
      }
    }
  }
  B2WARNING("ARICHGeoAerogelPlane::getTileParameters --> tile position does not exist " << endl
            << "ring   = " << ring << endl
            << "column = " << column << endl
            << "layerN = " << layerN << endl);
  B2ASSERT("ARICHGeoAerogelPlane::getTileParameters --> tile position does not exist", false);
  return 0;
}

double ARICHGeoAerogelPlane::getTileThickness(int ring, int column, int layerN) const
{
  for (unsigned i = 0; i < m_tiles.size(); i++)
    if (m_tiles.at(i).ring == ring)
      if (m_tiles.at(i).column == column)
        if (m_tiles.at(i).layer == layerN)
          return m_tiles.at(i).thick;
  B2WARNING("ARICHGeoAerogelPlane::getTileThickness --> tile position does not exist " << endl
            << "ring   = " << ring << endl
            << "column = " << column << endl
            << "layerN = " << layerN << endl);
  B2ASSERT("ARICHGeoAerogelPlane::getTileParameters --> tile position does not exist", false);
  return 0;
}

std::string ARICHGeoAerogelPlane::getTileMaterialName(int ring, int column, int layerN) const
{
  for (unsigned i = 0; i < m_tiles.size(); i++)
    if (m_tiles.at(i).ring == ring)
      if (m_tiles.at(i).column == column)
        if (m_tiles.at(i).layer == layerN)
          return m_tiles.at(i).material;
  B2WARNING("ARICHGeoAerogelPlane::getTileThickness --> tile position does not exist " << endl
            << "ring   = " << ring << endl
            << "column = " << column << endl
            << "layerN = " << layerN << endl);
  B2ASSERT("ARICHGeoAerogelPlane::getTileParameters --> tile position does not exist", false);
  return NULL;
}

double ARICHGeoAerogelPlane::getTotalTileThickness(int ring, int column) const
{
  double n;
  double transmL;
  double thickUp;
  double thickDown;
  std::string materialName;
  getTileParameters(ring, column, 0, n, transmL, thickUp, materialName);
  getTileParameters(ring, column, 1, n, transmL, thickDown, materialName);
  //cout << setw(5) << "QWER"
  //   << setw(10) << ring
  //   << setw(10) << column
  //   << setw(10) << thickUp
  //   << setw(10) << thickDown <<endl;
  return thickUp + thickDown;
}

double ARICHGeoAerogelPlane::getMaximumTotalTileThickness() const
{
  int iRi = 1;
  double maxThick = 0.0;
  double athick = 0.0;
  // ring ID   (range : [   1,     2,     3,     4])
  // column ID (range : [1;22] [1;28] [1;34] [1;40])
  for (unsigned iR = 0; iR < getNAeroSlotsIndividualRing().size(); iR++) {
    iRi = iR;
    for (int iS = 0; iS < getNAeroSlotsIndividualRing().at(iR); iS++) {
      athick = getTotalTileThickness(iRi + 1, iS + 1);
      if (maxThick < athick)
        maxThick = athick;
    }
  }
  return maxThick;
}

void ARICHGeoAerogelPlane::printTileParameters(const std::string& title) const
{
  ARICHGeoBase::print(title);
  cout << setw(5) << "i"
       << setw(10) << "ring"
       << setw(10) << "column"
       << setw(10) << "layer"
       << setw(12) << "n"
       << setw(12) << "transmL"
       << setw(12) << "thick"
       << setw(30) << "material" << endl;
  for (unsigned i = 0; i < m_tiles.size(); i++) {
    cout << setw(5) << i
         << setw(10) << m_tiles.at(i).ring
         << setw(10) << m_tiles.at(i).column
         << setw(10) << m_tiles.at(i).layer
         << setw(12) << m_tiles.at(i).n
         << setw(12) << m_tiles.at(i).transmL
         << setw(12) << m_tiles.at(i).thick
         << setw(30) << m_tiles.at(i).material << endl;
  }
}

void ARICHGeoAerogelPlane::printSingleTileParameters(unsigned i) const
{
  cout << "ARICHGeoAerogelPlane::printSingleTileParameters" << endl;
  B2ASSERT("Aerogel tile property entry does not exist i >= m_tiles.size() ", i < m_tiles.size());
  cout << setw(5) << "i"
       << setw(10) << "ring"
       << setw(10) << "column"
       << setw(10) << "layer"
       << setw(12) << "n"
       << setw(12) << "transmL"
       << setw(12) << "thick"
       << setw(30) << "material" << endl;
  cout << setw(5) << i
       << setw(10) << m_tiles.at(i).ring
       << setw(10) << m_tiles.at(i).column
       << setw(10) << m_tiles.at(i).layer
       << setw(12) << m_tiles.at(i).n
       << setw(12) << m_tiles.at(i).transmL
       << setw(12) << m_tiles.at(i).thick
       << setw(30) << m_tiles.at(i).material << endl;
}

void ARICHGeoAerogelPlane::testGetTileParametersFunction() const
{
  int ring = 1;
  int column = 1;
  int layerN = 0;
  double n;
  double transmL;
  double thick;
  std::string materialName;
  printSingleTileParameters(getTileParameters(ring, column, layerN, n, transmL, thick, materialName));
  ring = 1;
  column = 1;
  layerN = 1;
  printSingleTileParameters(getTileParameters(ring, column, layerN, n, transmL, thick, materialName));
  ring = 3;
  column = 20;
  layerN = 0;
  printSingleTileParameters(getTileParameters(ring, column, layerN, n, transmL, thick, materialName));
  ring = 3;
  column = 20;
  layerN = 1;
  printSingleTileParameters(getTileParameters(ring, column, layerN, n, transmL, thick, materialName));
  ring = -3;
  column = 20;
  layerN = 1;
  printSingleTileParameters(getTileParameters(ring, column, layerN, n, transmL, thick, materialName));
}
