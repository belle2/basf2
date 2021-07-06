/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHGeoDetectorPlane.h>
#include <math.h>
#include <iostream>

using namespace std;
using namespace Belle2;

void ARICHGeoDetectorPlane::setRingPar(const std::vector<std::pair<double, double>>& ringPar)
{
  m_ringPar = ringPar;

  for (auto ring : m_ringPar) {
    double iPhi = ring.second / 2.;
    while (iPhi < 2 * M_PI) {
      m_slotPar.push_back({ring.first, iPhi});
      iPhi += ring.second;
    }
  }

  m_nRings = m_ringPar.size();
  m_nSlots = m_slotPar.size();

}

unsigned ARICHGeoDetectorPlane::getSlotRing(unsigned slotID) const
{

  if (slotID > m_nSlots) B2ERROR("ARICHGeoDetectorPlane: invalid module slot ID number!");
  unsigned iRing = 1;
  unsigned nSlots = 0;
  for (auto dphi : m_ringPar) {
    nSlots += static_cast<int>(2 * M_PI / dphi.second + 0.5);
    if (slotID < nSlots + 1) return iRing;
    iRing++;
  }
  return 0;
}


unsigned ARICHGeoDetectorPlane::pointSlotID(double x, double y) const
{
  double r = sqrt(x * x + y * y);
  double phi = atan2(y, x);
  if (phi < 0) phi += 2 * M_PI;
  if (r < (m_ringPar[0].first - (m_ringPar[1].first - m_ringPar[0].first) / 2.)) return 0;

  if (r > (m_ringPar[m_nRings - 1].first + (m_ringPar[m_nRings - 1].first - m_ringPar[m_nRings - 2].first) / 2.)) return 0;

  unsigned nslot = 0;
  for (unsigned i = 1; i < m_nRings; i++) {
    if (r < (m_ringPar[i - 1].first + m_ringPar[i].first) / 2.) {
      nslot += phi / m_ringPar[i - 1].second;
      return nslot + 1;
    }
    nslot += static_cast<int>(2 * M_PI / m_ringPar[i - 1].second + 0.5);
  }
  nslot += phi / m_ringPar[m_nRings - 1].second;
  return nslot + 1;
}

bool ARICHGeoDetectorPlane::isConsistent() const
{
  if (m_outerR < m_innerR) return false;
  if (m_thickness <= 0) return false;
  if (m_moduleHoleSize < 0) return false;
  if (m_supportBackWallHeight < 0) return false;
  if (m_supportBackWallThickness < 0) return false;
  if (m_supportZPosition < 0) return false;
  if (m_ringPar.size() == 0 || m_slotPar.size() == 0) return false;
  return true;
}

unsigned ARICHGeoDetectorPlane::getSlotIDFromSRF(unsigned sector, unsigned ring, unsigned azimuth) const
{

  if (azimuth > static_cast<unsigned>(2 * M_PI / m_ringPar[ring - 1].second + 0.5) / 6 || ring > getNRings() || sector > 6)
  { B2ERROR("ARICHGeoDetectorPlane::getSlotIDFromSRF: invalid slot position parameters!"); return 0;}

  unsigned moduleID = 0;
  for (unsigned i = 1; i < ring; i++) {
    moduleID += static_cast<int>(2 * M_PI / m_ringPar[i - 1].second + 0.5);
  }
  moduleID += (sector - 1) * static_cast<int>(2 * M_PI / m_ringPar[ring - 1].second + 0.5) / 6 + (azimuth - 1);
  if (moduleID + 1 > m_nSlots) {B2ERROR("ARICHGeoDetectorPlane::getSlotIDFromSRF: invalid slot position parameters!"); return 0;}
  return moduleID + 1;
}


void ARICHGeoDetectorPlane::print(const std::string& title) const
{
  ARICHGeoBase::print(title);
  cout << "Configuration of module slots:" << endl;
  int i = 0;
  cout << " There are " << getNRings() << " module rings." << endl;
  for (auto ring : m_ringPar) {
    cout << " ring: " << i + 1 << " has " <<  static_cast<int>(2 * M_PI / ring.second + 0.5) << " module slots, at radius: " <<
         ring.first << " cm" << endl;
    i++;
  }

  cout << " Support plate inner radius:       " << getSupportInnerR() << " " << s_unitName << endl;
  cout << " Support plate outer radius:       " << getSupportOuterR() << " " << s_unitName << endl;
  cout << " Support plate thickness:          " << getSupportThickness() << " " << s_unitName << endl;
  cout << " Support plate Z position:         " << getSupportZPosition() << " " << s_unitName << endl;
  cout << " Support plate backwalls height:   " << getSupportBackWallHeight() << " " << s_unitName << endl;
  cout << " Support plate backwall thickness: " << getSupportBackWallThickness() << " " << s_unitName << endl;
  cout << " Support plate module hole size:   " << getModuleHoleSize() << " X " << getModuleHoleSize()  << " " << s_unitName << endl;
  cout << " Support plate material:           " << getSupportMaterial() << endl;

  cout << "Positioning parameters (in local ARICH frame)" << endl;
  ARICHGeoBase::printPlacement(m_x, m_y, m_z, m_rx, m_ry, m_rz);

}
