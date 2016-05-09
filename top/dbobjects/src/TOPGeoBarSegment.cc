/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoBarSegment.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>

using namespace std;
using namespace Belle2;

bool TOPGeoBarSegment::isConsistent() const
{
  if (m_width <= 0) return false;
  if (m_thickness <= 0) return false;
  if (m_length <= 0) return false;
  if (m_material.empty()) return false;
  if (m_glueThickness <= 0) return false;
  if (m_glueMaterial.empty()) return false;
  if (m_surface.getName().empty() and !m_surface.hasProperties()) return false;
  if (m_sigmaAlpha < 0) return false;
  return true;
}


void TOPGeoBarSegment::print(const std::string& title) const
{
  TOPGeoBase::print(title);
  cout << " Dimensions: " << getWidth() << " X " << getThickness() << " X " << getLength()
       << " " << s_unitName << endl;
  cout << " Material: " << getMaterial() << endl;
  cout << " Glue: " << getGlueMaterial() << ", thickness: " << getGlueThickness()
       << " " << s_unitName << endl;
  printSurface(m_surface);
  cout << "  - sigmaAlpha: " << getSigmaAlpha() << endl;
}

