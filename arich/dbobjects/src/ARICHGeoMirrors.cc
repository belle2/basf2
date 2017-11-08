/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHGeoMirrors.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>
#include <TVector2.h>
#include <iomanip>

using namespace std;
using namespace Belle2;


void ARICHGeoMirrors::initializeDefault()
{
  double rmir = m_radius - m_plateThickness / 2.;
  for (unsigned i = 0; i < m_nMirrors; i++) {
    TVector3 norm(cos(i * 2.*M_PI / m_nMirrors + m_startAngle),
                  sin(i * 2.*M_PI / m_nMirrors + m_startAngle), 0);
    m_normVector.push_back(norm);
    m_point.push_back(rmir * norm);
    m_installed.push_back(true);
  }
}

bool ARICHGeoMirrors::isConsistent() const
{
  if (m_nMirrors == 0) { B2INFO("ARICHGeoMirrors: no mirrors will be placed"); return true;}

  if (m_radius <= 0) return false;
  if (m_plateThickness <= 0 || m_plateWidth <= 0 || m_plateLength <= 0) return false;
  if (m_plateWidth == 0) return false;
  if (m_radius == 0) return false;
  if (m_radius == 0) return false;
  if (m_point.size() != m_nMirrors) return false;
  if (m_normVector.size() != m_nMirrors) return false;
  if (m_installed.size() != m_nMirrors) return false;
  if (m_material.empty()) return false;
  if (!m_surface.hasProperties())
    B2INFO("ARICHGeoMirrors: no optical properties are specified for mirrors! (no photons will be reflected)");
  return true;
}


void ARICHGeoMirrors::print(const std::string& title) const
{
  ARICHGeoBase::print(title);
  cout << " mirror plate:" << endl;
  cout << "  thickness: " <<  getPlateThickness() << " " << s_unitName << ", width: " << getPlateWidth() << " " << s_unitName <<
       ", length: "
       << getPlateLength() <<  " " << s_unitName << endl;
  cout << "  material: " << getMaterial() << endl;
  cout << " number of plates:                          " <<  getNMirrors() << endl;
  cout << " nominal phi angle of 1st plate:            " <<  getStartAngle() << endl;
  cout << " nominal radius at which plates are placed: " << getRadius() <<  " " << s_unitName << endl;
  cout << " nominal Z position of plates:              " << getZPosition() <<  " " << s_unitName << endl;

  cout << " installed plates " << endl;

  cout << " ID:   ";
  for (unsigned imir = 1; imir < getNMirrors() + 1; imir++) {
    cout << setw(3) << imir << " ";
  }
  cout << endl;
  cout << " flag: ";
  for (unsigned imir = 1; imir < getNMirrors() + 1; imir++) {
    cout << setw(3) << isInstalled(imir) << " ";
  }
  cout << endl;

  ARICHGeoBase::printSurface(m_surface);
}
