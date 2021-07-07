/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPGeoSideRails.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  bool TOPGeoSideRails::isConsistent() const
  {
    if (m_thickness <= 0) return false;
    if (m_reducedThickness <= 0) return false;
    if (m_height <= 0) return false;
    if (m_material.empty()) return false;
    return true;
  }


  void TOPGeoSideRails::print(const std::string& title) const
  {
    TOPGeoBase::print(title);

    cout << " thickness = " << getThickness() << " " << s_unitName;
    cout << ", reduced thickness = " << getReducedThickness() << " " << s_unitName;
    cout << ", height = " << getHeight() << " " << s_unitName;
    cout << ", material = " << getMaterial() << endl;

  }

} // end Belle2 namespace
