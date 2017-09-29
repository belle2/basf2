/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoQBB.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  std::vector<std::pair<double, double> > TOPGeoQBB::getForwardContour() const
  {
    typedef std::pair<double, double> Pair;
    std::vector<Pair> contour;

    double x1 = getWidth() / 2;
    double x2 = x1 - m_sideRails.getThickness();
    double x3 = getWidth() / 6;
    double y1 = m_outerPanel.getY() + m_outerPanel.getMaxThickness();
    double y2 = m_outerPanel.getY() + m_outerPanel.getMinThickness();
    double y3 = y2 - m_sideRails.getHeight();
    double y4 = m_innerPanel.getY() - m_innerPanel.getMaxThickness();
    contour.push_back(Pair(-x1, y2));
    contour.push_back(Pair(-x3, y1));
    contour.push_back(Pair(x3, y1));
    contour.push_back(Pair(x1, y2));
    contour.push_back(Pair(x1, y3));
    contour.push_back(Pair(x2, y4));
    contour.push_back(Pair(-x2, y4));
    contour.push_back(Pair(-x1, y3));

    return contour;
  }

  void TOPGeoQBB::printForwardContour() const
  {
    auto contour = getForwardContour();
    cout << "Forward contour [" << s_unitName << "]:" << endl;
    for (auto& xy : contour) cout << xy.first << " " << xy.second << endl;
    cout << endl;
  }


  std::vector<std::pair<double, double> > TOPGeoQBB::getInnerPanelContour() const
  {
    typedef std::pair<double, double> Pair;
    std::vector<Pair> contour;

    double x1 = getPanelWidth() / 2;
    double y1 = m_innerPanel.getY();
    double y2 = y1 - m_innerPanel.getMaxThickness();
    contour.push_back(Pair(-x1, y1));
    contour.push_back(Pair(x1, y1));
    contour.push_back(Pair(x1, y2));
    m_innerPanel.appendContour(contour, y2, false);
    contour.push_back(Pair(-x1, y2));

    return contour;
  }

  void TOPGeoQBB::printInnerPanelContour() const
  {
    auto contour = getInnerPanelContour();
    cout << "Inner panel contour [" << s_unitName << "]:" << endl;
    for (auto& xy : contour) cout << xy.first << " " << xy.second << endl;
    cout << endl;
  }


  std::vector<std::pair<double, double> > TOPGeoQBB::getOuterPanelContour() const
  {
    typedef std::pair<double, double> Pair;
    std::vector<Pair> contour;

    double x1 = getPanelWidth() / 2;
    double y1 = m_outerPanel.getY();
    double y2 = y1 + m_outerPanel.getMinThickness();
    contour.push_back(Pair(-x1, y2));
    m_outerPanel.appendContour(contour, y2, true);
    contour.push_back(Pair(x1, y2));
    contour.push_back(Pair(x1, y1));
    contour.push_back(Pair(-x1, y1));

    return contour;
  }

  void TOPGeoQBB::printOuterPanelContour() const
  {
    auto contour = getOuterPanelContour();
    cout << "Outer panel contour [" << s_unitName << "]:" << endl;
    for (auto& xy : contour) cout << xy.first << " " << xy.second << endl;
    cout << endl;
  }


  std::vector<std::pair<double, double> > TOPGeoQBB::getBackwardContour() const
  {
    typedef std::pair<double, double> Pair;
    std::vector<Pair> contour;

    double x1 = getWidth() / 2;
    double x2 = x1 - m_prismEnclosure.getHeight() * tan(m_prismEnclosure.getAngle());
    double x3 = getWidth() / 6;
    double y1 = m_outerPanel.getY() + m_outerPanel.getMaxThickness();
    double y2 = m_outerPanel.getY() + m_outerPanel.getMinThickness();
    double y3 = y2 - m_sideRails.getHeight();
    double y4 = y3 - m_prismEnclosure.getHeight();
    contour.push_back(Pair(-x1, y2));
    contour.push_back(Pair(-x3, y1));
    contour.push_back(Pair(x3, y1));
    contour.push_back(Pair(x1, y2));
    contour.push_back(Pair(x1, y3));
    contour.push_back(Pair(x2, y4));
    contour.push_back(Pair(-x2, y4));
    contour.push_back(Pair(-x1, y3));

    return contour;
  }

  void TOPGeoQBB::printBackwardContour() const
  {
    auto contour = getBackwardContour();
    cout << "Backward contour [" << s_unitName << "]:" << endl;
    for (auto& xy : contour) cout << xy.first << " " << xy.second << endl;
    cout << endl;
  }


  std::vector<std::pair<double, double> > TOPGeoQBB::getPrismEnclosureContour() const
  {
    typedef std::pair<double, double> Pair;
    std::vector<Pair> contour;

    double tanAngle = tan(m_prismEnclosure.getAngle());
    double x1 = getWidth() / 2;
    double x2 = x1 - m_prismEnclosure.getHeight() * tanAngle;
    double x3 = x1 - m_prismEnclosure.getSideThickness();
    double y1 = m_outerPanel.getY() + m_outerPanel.getMinThickness()
                - m_sideRails.getHeight();
    double y2 = y1 - m_prismEnclosure.getHeight();
    double y3 = y2 + m_prismEnclosure.getBottomThickness();
    double x4 = x1 - m_prismEnclosure.getSideThickness() - (y1 - y3) * tanAngle;
    contour.push_back(Pair(x1, y1));
    contour.push_back(Pair(x2, y2));
    contour.push_back(Pair(-x2, y2));
    contour.push_back(Pair(-x1, y1));
    contour.push_back(Pair(-x3, y1));
    contour.push_back(Pair(-x4, y3));
    contour.push_back(Pair(x4, y3));
    contour.push_back(Pair(x3, y1));

    return contour;
  }

  void TOPGeoQBB::printPrismEnclosureContour() const
  {
    auto contour = getPrismEnclosureContour();
    cout << "Prism enclosure contour [" << s_unitName << "]:" << endl;
    for (auto& xy : contour) cout << xy.first << " " << xy.second << endl;
    cout << endl;
  }


  std::vector<std::pair<double, double> > TOPGeoQBB::getBackPlateContour() const
  {
    typedef std::pair<double, double> Pair;
    std::vector<Pair> contour;

    double x1 = getWidth() / 2;
    double x2 = x1 - m_prismEnclosure.getHeight() * tan(m_prismEnclosure.getAngle());
    double y1 = m_outerPanel.getY() + m_outerPanel.getMinThickness();
    double y2 = y1 - m_sideRails.getHeight();
    double y3 = y2 - m_prismEnclosure.getHeight();
    contour.push_back(Pair(-x1, y1));
    contour.push_back(Pair(x1, y1));
    contour.push_back(Pair(x1, y2));
    contour.push_back(Pair(x2, y3));
    contour.push_back(Pair(-x2, y3));
    contour.push_back(Pair(-x1, y2));

    return contour;
  }

  void TOPGeoQBB::printBackPlateContour() const
  {
    auto contour = getBackPlateContour();
    cout << "Prism enclosure back plate contour [" << s_unitName << "]:" << endl;
    for (auto& xy : contour) cout << xy.first << " " << xy.second << endl;
    cout << endl;
  }


  std::vector<std::pair<double, double> > TOPGeoQBB::getFrontPlateContour() const
  {
    typedef std::pair<double, double> Pair;
    std::vector<Pair> contour;

    double x1 = getWidth() / 2;
    double x2 = x1 - m_prismEnclosure.getHeight() * tan(m_prismEnclosure.getAngle());
    double y1 = m_outerPanel.getY() + m_outerPanel.getMinThickness();
    double y2 = y1 - m_sideRails.getHeight();
    double y3 = y2 - m_prismEnclosure.getHeight();
    contour.push_back(Pair(x1, y2));
    contour.push_back(Pair(x2, y3));
    contour.push_back(Pair(-x2, y3));
    contour.push_back(Pair(-x1, y2));

    return contour;
  }

  void TOPGeoQBB::printFrontPlateContour() const
  {
    auto contour = getFrontPlateContour();
    cout << "Prism enclosure front plate contour [" << s_unitName << "]:" << endl;
    for (auto& xy : contour) cout << xy.first << " " << xy.second << endl;
    cout << endl;
  }


  bool TOPGeoQBB::isConsistent() const
  {
    if (m_width <= 0) return false;
    if (m_length <= 0) return false;
    if (m_prismPosition <= 0) return false;
    if (m_material.empty()) return false;
    if (!m_innerPanel.isConsistent()) return false;
    if (!m_outerPanel.isConsistent()) return false;
    if (!m_sideRails.isConsistent()) return false;
    if (!m_prismEnclosure.isConsistent()) return false;
    if (!m_endPlate.isConsistent()) return false;
    if (!m_coldPlate.isConsistent()) return false;
    return true;
  }


  void TOPGeoQBB::print(const std::string& title) const
  {
    TOPGeoBase::printUnderlined(title);

    cout << " width = " << getWidth() << " " << s_unitName;
    cout << ", length = " << getLength() << " " << s_unitName;
    cout << ", prism-bar joint = " << getPrismPosition() << " " << s_unitName;
    cout << ", inside material = " << getMaterial() << endl;

    m_innerPanel.print("QBB inner honeycomb panel geometry parameters");
    m_outerPanel.print("QBB outer honeycomb panel geometry parameters");
    m_sideRails.print();
    m_prismEnclosure.print();
    m_endPlate.print();
    m_coldPlate.print();

  }

} // end Belle2 namespace
