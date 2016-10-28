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
#include <TVector2.h>

using namespace std;
using namespace Belle2;


bool ARICHGeoAerogelPlane::isConsistent() const
{

  if (m_layers.size() == 0 || m_r.size() == 0 || m_dPhi.size() == 0) return false;
  if (m_tileGap < 0 || m_innerR < 0) return false;
  if (m_innerR > m_outerR) return false;
  if (m_thickness < 0 || m_wallThickness < 0 || m_wallHeight < 0) return false;
  return true;
}


void ARICHGeoAerogelPlane::print(const std::string& title) const
{
  ARICHGeoBase::print(title);

  cout << "Aerogel layers " << endl;

  int nLayer = getNLayers();
  for (int iLayer = 1; iLayer < nLayer + 1; iLayer++) {
    cout << " layer " << iLayer  << endl;
    cout << "  thickness: " << getLayerThickness(iLayer) << " " << s_unitName << ", ref. index: " << getLayerRefIndex(
           iLayer) << ", transm. length: " << getLayerTrLength(iLayer) << " " << s_unitName << ", material: " << getLayerMaterial(
           iLayer)  << endl;
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
