/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoEndPlate.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  bool TOPGeoEndPlate::isConsistent() const
  {
    if (m_thickness <= 0) return false;
    if (m_height <= 0) return false;
    if (m_material.empty()) return false;
    return true;
  }


  void TOPGeoEndPlate::print(const std::string& title) const
  {
    TOPGeoBase::print(title);

    cout << " thickness = " << getThickness() << " " << s_unitName;
    cout << ", height = " << getHeight() << " " << s_unitName;
    cout << ", material = " << getMaterial() << endl;

  }

} // end Belle2 namespace
