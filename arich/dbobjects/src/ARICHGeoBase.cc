
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHGeoBase.h>
#include <framework/gearbox/Unit.h>
#include <iostream>

using namespace std;
using namespace Belle2;

double ARICHGeoBase::s_unit = Unit::cm;
std::string ARICHGeoBase::s_unitName("cm");

void ARICHGeoBase::print(const std::string& title) const
{
  cout << title << ":" << endl;
}

void ARICHGeoBase::printPlacement(double x, double y, double z, double rx, double ry, double rz) const
{
//  cout << "Volume positioning information (inside local ARICH frame)" << endl;
  cout << " Center position; x: " << x << " " << s_unitName << ", y: " << y << " " << s_unitName << ", z: " << z << " " << s_unitName
       << endl;
  cout << " Rotations; x-axis: " << rx << " y-axis: " << ry << " z-axis: " << rz << " (in rad)"  << endl;
}
void ARICHGeoBase::printSurface(const GeoOpticalSurface& surface) const
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

