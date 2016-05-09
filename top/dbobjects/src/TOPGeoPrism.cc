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
#include <math.h>
#include <iostream>

using namespace std;
using namespace Belle2;

bool TOPGeoPrism::isConsistent() const
{
  if (m_exitThickness <= 0) return false;
  if (m_flatLength < 0) return false;
  if (!TOPGeoBarSegment::isConsistent()) return false;
  return true;
}


void TOPGeoPrism::print(const std::string& title) const
{
  TOPGeoBase::print(title);
  cout << " Dimensions: " << getWidth() << " X " << getThickness() << " X " << getLength()
       << " " << s_unitName << endl;
  cout << " Exit window dimensions: " << getWidth() << " X "
       << getExitThickness() << " " << s_unitName << endl;
  cout << " Bottom flat surface length: " << getFlatLength() << " " << s_unitName << endl;
  cout << " Material: " << getMaterial() << endl;
  cout << " Wavelenght filter: " << getFilterMaterial()
       << ", thickness: " << getFilterThickness() << " " << s_unitName << endl;
  printSurface(m_surface);
  cout << "  - sigmaAlpha: " << getSigmaAlpha() << endl;

}

