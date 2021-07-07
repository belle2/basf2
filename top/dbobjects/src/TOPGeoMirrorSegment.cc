/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPGeoMirrorSegment.h>
#include <math.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  double TOPGeoMirrorSegment::getZc() const
  {
    double dx = fabs(getXc()) - getWidth() / 2;
    double dy = fabs(getYc()) - getThickness() / 2;
    double z = getOuterRadius();
    if (dx > 0) z = sqrt(z * z - dx * dx);
    if (dy > 0) z = sqrt(z * z - dy * dy);

    return getFullLength() / 2 - z;
  }


  bool TOPGeoMirrorSegment::isConsistent() const
  {
    if (m_radius <= 0) return false;
    if (m_coatingThickness <= 0) return false;
    if (m_coatingMaterial.empty()) return false;
    if (m_coatingSurface.getName().empty() and !m_coatingSurface.hasProperties()) return false;
    if (!TOPGeoBarSegment::isConsistent()) return false;
    return true;
  }


  void TOPGeoMirrorSegment::print(const std::string& title) const
  {
    TOPGeoBarSegment::print(title);
    cout << " Radius of curvature: " << getRadius() << " " << s_unitName << endl;
    cout << " Center of curvature: (" << getXc() << ", " << getYc()
         << ") " << s_unitName << endl;
    cout << " Reflective coating: " << getCoatingMaterial() << ", thickness: " <<
         getCoatingThickness() << " " << s_unitName << endl;
    printSurface(m_coatingSurface);

  }

} // end Belle2 namespace
