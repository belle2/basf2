/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPGeoPrismEnclosure.h>
#include <framework/gearbox/Unit.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  bool TOPGeoPrismEnclosure::isConsistent() const
  {
    if (m_length <= 0) return false;
    if (m_height <= 0) return false;
    if (m_bottomThickness <= 0) return false;
    if (m_sideThickness <= 0) return false;
    if (m_backThickness <= 0) return false;
    if (m_frontThickness <= 0) return false;
    if (m_extensionThickness <= 0) return false;
    if (m_material.empty()) return false;
    return true;
  }


  void TOPGeoPrismEnclosure::print(const std::string& title) const
  {
    TOPGeoBase::print(title);

    cout << " length = " << getLength() << " " << s_unitName;
    cout << ", height = " << getHeight() << " " << s_unitName;
    cout << ", angle = " << getAngle() / Unit::deg << " deg";
    cout << ", material = " << getMaterial() << endl;

    cout << " thicknesses: bottom = " << getBottomThickness() << " " << s_unitName;
    cout << ", side = " << getSideThickness() << " " << s_unitName;
    cout << ", back = " << getBackThickness() << " " << s_unitName;
    cout << ", front = " << getFrontThickness() << " " << s_unitName;
    cout << ", extension plate = " << getExtensionThickness() << " " << s_unitName << endl;
  }

} // end Belle2 namespace
