/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHGeoHAPD.h>
#include <iostream>

using namespace std;
using namespace Belle2;

void ARICHGeoHAPD::getXYChannelPos(int chX, int chY, double& x, double& y) const
{

  if (chX < m_nPadX / 2.) x = -m_chipGap / 2. + (chX - m_nPadX / 2.) * m_padSize + m_padSize / 2.;
  else x = m_chipGap / 2. + (chX - m_nPadX / 2.) * m_padSize + m_padSize / 2.;

  if (chY < m_nPadY / 2.) y = -m_chipGap / 2. + (chY - m_nPadY / 2.) * m_padSize + m_padSize / 2.;
  else y = m_chipGap / 2. + (chY - m_nPadY / 2.) * m_padSize + m_padSize / 2.;

}


void ARICHGeoHAPD::getXYChannel(double x, double y, int& chX, int& chY) const
{

  if (x < - m_chipGap / 2.) chX = int((x + m_chipGap / 2.) / m_padSize) + m_nPadX / 2. - 1;
  else if (x > m_chipGap / 2.) chX = (x - m_chipGap / 2.) / m_padSize + m_nPadX / 2.;
  else {chX = -1; chY = -1; return;}

  if (y < - m_chipGap / 2.) chY = int((y + m_chipGap / 2.) / m_padSize) + m_nPadY / 2. - 1;
  else if (y > m_chipGap / 2.) chY = (y - m_chipGap / 2.) / m_padSize + m_nPadY / 2.;
  else {chX = -1; chY = -1; return;}

  if (chX + 1 > int(m_nPadX) || chY + 1 > int(m_nPadY) || chX < 0 || chY < 0) {
    chX = -1; chY = -1;
  }

}


unsigned ARICHGeoHAPD::getChipID(double x, double y) const
{
  if (x > 0) {
    if (y > 0) return 0;
    return 1;
  }
  if (y > 0) return 2;
  return 3;
}


bool ARICHGeoHAPD::isConsistent() const
{
  if (m_HAPDSizeX <= 0) return false;
  if (m_HAPDSizeY <= 0) return false;
  if (m_HAPDSizeZ <= 0) return false;
  if (m_wallThickness <= 0) return false;
  if (m_winThickness <= 0) return false;
  if (m_padSize <= 0) return false;
  if (m_nPadX == 0) return false;
  if (m_nPadY == 0) return false;
  if (m_APDSizeZ <= 0) return false;
  if (m_APDSizeX <= 0) return false;
  if (m_APDSizeY <= 0) return false;
  if (m_FEBSizeX <= 0) return false;
  if (m_FEBSizeY <= 0) return false;
  if (m_FEBSizeZ <= 0) return false;
  if (m_moduleSizeZ <= 0) return false;
  if (m_winRefIndex < 1.) return false;

  if (m_wallMaterial.empty()) return false;
  if (m_winMaterial.empty()) return false;
  if (m_apdMaterial.empty()) return false;
  if (m_febMaterial.empty()) return false;
  if (m_fillMaterial.empty()) return false;

  return true;
}


void ARICHGeoHAPD::print(const std::string& title) const
{
  ARICHGeoBase::print(title);

  cout << " outer HAPD dimensions: " << getSizeX() << " X " << getSizeY() << " X " << getSizeZ() << " " << s_unitName << endl;
  cout << " wall thickness:        " << getWallThickness() << " " << s_unitName << endl;
  cout << " window thickness:      " << getWinThickness() << " " << s_unitName << endl;
  cout << " pad size:              " << getPadSize() << " " << s_unitName << endl;
  cout << " number of pixels:      " << getNumPadsX() << " X " << getNumPadsY() << endl;
  cout << " gap between APD chips: " << getChipGap() << " " << s_unitName << endl;
  cout << " APD dimensions:        " << getAPDSizeX() << " X " << getAPDSizeY() << " X " << getAPDSizeZ() << " " << s_unitName <<
       endl;
  cout << " FEB dimensions:        " << getFEBSizeX() << " X " << getFEBSizeY()  << " X " << getFEBSizeZ() << " " << s_unitName <<
       endl;
  cout << " window ref. index:     " << getWinRefIndex() << endl;
  cout << " module height (Z):     " << getModuleSizeZ() << " " << s_unitName << endl;
  cout << " wall material:         " << getWallMaterial() << endl;
  cout << " window material:       " << getWinMaterial() << endl;
  cout << " HAPD fill material:    " << getFillMaterial() << endl;
  cout << " FEB material:          " << getFEBMaterial() << endl;
  cout << " APD material:          " << getAPDMaterial() << endl;

  // add material names!
  printSurface(m_apdSurface);
}
