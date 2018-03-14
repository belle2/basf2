
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoBase.h>
#include <framework/gearbox/Unit.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  double TOPGeoBase::s_unit = Unit::cm;
  std::string TOPGeoBase::s_unitName("cm");


  void TOPGeoBase::print(const std::string& title) const
  {
    cout << title << ":" << endl;
    cout << " name: " << m_name << endl;
  }


  void TOPGeoBase::printUnderlined(const std::string& title) const
  {
    cout << title << ":" << endl;
    for (size_t i = 0; i <= title.length(); i++) cout << "-";
    cout << endl;
    cout << " name: " << m_name << endl;
  }


  void TOPGeoBase::printSurface(const GeoOpticalSurface& surface) const
  {
    cout << " Optical surface: ";
    if (surface.getName().empty() and !surface.hasProperties()) {
      cout << "not defined" << endl;
      return;
    }
    cout << surface.getName();
    cout << ", model: " << surface.getModel();
    cout << ", finish: " << surface.getFinish();
    cout << ", type: " << surface.getType();
    cout << ", value: " << surface.getValue();
    cout << endl;
    if (surface.hasProperties()) {
      for (const auto& property : surface.getProperties()) {
        cout << "  - property: ";
        cout << property.getName() << " [";
        for (const auto& value : property.getValues()) cout << value << ", ";
        cout << "], @[";
        for (const auto& value : property.getEnergies()) cout << value / Unit::eV << ", ";
        cout << "] eV" << endl;
      }
    } else {
      cout << "  - properties: None" << endl;
    }

  }

} // end Belle2 namespace
