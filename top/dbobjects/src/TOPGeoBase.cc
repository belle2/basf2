
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPGeoBase.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <TSpline.h>
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

  double TOPGeoBase::getReflectivity(const GeoOpticalSurface& surface, double energy) const
  {
    energy *= Unit::eV;
    if (surface.hasProperties()) {
      for (const auto& property : surface.getProperties()) {
        if (property.getName() == "REFLECTIVITY") {
          auto energies = property.getEnergies();
          auto values = property.getValues();
          if (energies.size() < 2) return 0;
          if (energy < energies[0] or energy > energies.back()) {
            B2WARNING("TOPGeoBase::getReflectivity: photon energy out of range - return value not reliable");
          }
          auto spline = TSpline3("tmp", energies.data(), values.data(), energies.size());
          return spline.Eval(energy);
        }
      }
    }

    B2ERROR("Optical surface " << surface.getName() << " has no property REFLECTIVITY");
    return 0;
  }


} // end Belle2 namespace
