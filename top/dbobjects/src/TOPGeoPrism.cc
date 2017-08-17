/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoPrism.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <math.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  bool TOPGeoPrism::isConsistent() const
  {
    if (m_exitThickness <= 0) return false;
    if (m_flatLength < 0) return false;
    if (!TOPGeoBarSegment::isConsistent()) return false;
    if (!m_peelOffRegions.empty()) {
      if (m_peelOffSize <= 0) return false;
      if (m_peelOffThickness <= 0) return false;
      if (m_peelOffMaterial.empty()) return false;
      double halfSize = (getWidth() - getPeelOffSize()) / 2;
      for (const auto& region : m_peelOffRegions) {
        if (fabs(getPeelOffCenter(region)) > halfSize) return false;
      }
    }
    return true;
  }


  void TOPGeoPrism::print(const std::string& title) const
  {
    TOPGeoBase::print(title);
    cout << " Vendor: " << getVendor() << ", serial number: " << getSerialNumber() << endl;
    cout << " Dimensions: " << getWidth() << " X " << getThickness() << " X " << getLength()
         << " " << s_unitName << endl;
    cout << " Exit window dimensions: " << getWidth() << " X "
         << getExitThickness() << " " << s_unitName << endl;
    cout << " Prism angle: " << getAngle() / Unit::deg << " deg";
    cout << ", flat surface length: " << getFlatLength() << " " << s_unitName << endl;
    cout << " Material: " << getMaterial() << endl;
    cout << " Wavelenght filter: " << getFilterMaterial()
         << ", thickness: " << getFilterThickness() << " " << s_unitName << endl;
    if (!m_peelOffRegions.empty()) {
      cout << " Peel-off cookie regions: ";
      cout << " size = " << getPeelOffSize() << " " << s_unitName;
      cout << " thickness = " << getPeelOffThickness() << " " << s_unitName;
      cout << " material = " << getPeelOffMaterial() << endl;
      for (const auto& region : m_peelOffRegions) {
        cout << "   ID = " << region.ID << ", fraction = " << region.fraction
             << ", angle = " << region.angle  / Unit::deg << " deg"
             << ", xc = " << getPeelOffCenter(region) << " " << s_unitName << endl;
      }
    } else {
      cout << " Peel-off regions: None" << endl;
    }
    printSurface(m_surface);
    cout << "  - sigmaAlpha: " << getSigmaAlpha() << endl;

  }


  void TOPGeoPrism::appendPeelOffRegion(unsigned ID, double fraction, double angle)
  {
    for (const auto& region : m_peelOffRegions) {
      if (region.ID == ID) {
        B2ERROR("TOPGeoPrism::appendPeelOffRegion: region ID = " << ID
                << " already appended");
        return;
      }
    }
    PeelOffRegion region;
    region.ID = ID;
    region.fraction = fraction;
    region.angle = angle;
    double halfSize = (getWidth() - getPeelOffSize()) / 2;
    if (fabs(getPeelOffCenter(region)) > halfSize) {
      B2ERROR("TOPGeoPrism::appendPeelOffRegion: region ID = " << ID
              << " doesn't fit into prism");
      return;
    }
    m_peelOffRegions.push_back(region);
  }

  typedef std::pair<double, double> Pair;

  std::vector<Pair> TOPGeoPrism::getPeelOffContour(const PeelOffRegion& region) const
  {
    std::vector<Pair> contour;
    constructContour(getPeelOffSize() / 2, getExitThickness() / 2,
                     region.fraction, region.angle,
                     contour);

    return contour;
  }

} // end Belle2 namespace
