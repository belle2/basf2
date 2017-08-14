/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoHoneycombPanel.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  void TOPGeoHoneycombPanel::appendContour(std::vector<std::pair<double, double> >& contour,
                                           double y0,
                                           bool fromLeftToRight) const
  {
    if (m_N <= 0) return;

    typedef std::pair<double, double> Pair;

    double R = getRadius();
    double yc = getMaxThickness() - getMinThickness() - R;
    double x0 = sqrt(R * R - yc * yc);
    if (!fromLeftToRight) x0 = -x0;
    double dx = 2.0 * x0 / m_N;
    double x = -x0;
    contour.push_back(Pair(x, y0));
    for (int i = 1; i < m_N; i++) {
      x += dx;
      double y = y0 + sqrt(R * R - x * x) + yc;
      contour.push_back(Pair(x, y));
    }
    contour.push_back(Pair(x0, y0));

  }


  bool TOPGeoHoneycombPanel::isConsistent() const
  {
    if (m_width <= 0) return false;
    if (m_length <= 0) return false;
    if (m_minThickness <= 0) return false;
    if (m_maxThickness < m_minThickness) return false;
    if (m_radius <= 0) return false;
    if (m_edgeWidth <= 0) return false;
    if (m_material.empty()) return false;
    if (m_edgeMaterial.empty()) return false;
    return true;
  }


  void TOPGeoHoneycombPanel::print(const std::string& title) const
  {
    TOPGeoBase::print(title);

    cout << " width = " << getWidth() << " " << s_unitName;
    cout << ", length = " << getLength() << " " << s_unitName;
    cout << ", thickness: min = " << getMinThickness() << " " << s_unitName;
    cout << ", max = " << getMaxThickness() << " " << s_unitName;
    cout << ", radius = " << getRadius() << " " << s_unitName << endl;

    cout << " edge width = " << getEdgeWidth() << " " << s_unitName;
    cout << ", y = " << getY() << " " << s_unitName;
    cout << ", N = " << m_N << endl;

    cout << " materials: body = " << getMaterial();
    cout << ", edge = " << getEdgeMaterial() << endl;
  }

} // end Belle2 namespace
