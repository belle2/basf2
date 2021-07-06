/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPGeoColdPlate.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  bool TOPGeoColdPlate::isConsistent() const
  {
    if (m_baseThickness <= 0) return false;
    if (m_baseMaterial.empty()) return false;
    if (m_coolThickness <= 0) return false;
    if (m_coolWidth <= 0) return false;
    if (m_coolMaterial.empty()) return false;
    return true;
  }


  void TOPGeoColdPlate::print(const std::string& title) const
  {
    TOPGeoBase::print(title);

    cout << " base plate: thickness = " << getBaseThickness() << " " << s_unitName;
    cout << ", material = " << getBaseMaterial() << endl;

    cout << " cooling plate: thickness = " << getCoolThickness() << " " << s_unitName;
    cout << ", width = " << getCoolWidth() << " " << s_unitName;
    cout << ", material = " << getCoolMaterial() << endl;

  }

} // end Belle2 namespace
