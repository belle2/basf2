/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoModule.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>

using namespace std;
using namespace Belle2;

bool TOPGeoModule::isConsistent() const
{
  if (m_moduleID <= 0) return false;
  if (!m_bar1.isConsistent()) return false;
  if (!m_bar2.isConsistent()) return false;
  if (!m_mirror.isConsistent()) return false;
  if (!m_prism.isConsistent()) return false;
  return true;
}


void TOPGeoModule::print(const std::string& title) const
{
  TOPGeoBase::print(title);

  cout << " moduleID: " << getModuleID();
  cout << " radius: " << getRadius() << " " << s_unitName;
  cout << " phi: " << getPhi();
  cout << " backwardZ: " << getBackwardZ() << " " << s_unitName;
  cout << " CNumber: " << getModuleCNumber() << endl;

  m_bar1.print("Bar segment 1 geometry parameters");
  m_bar2.print("Bar segment 2 geometry parameters");
  m_mirror.print();
  m_prism.print();
  m_arrayDisplacement.print();
  m_moduleDisplacement.print();

}

