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

TVector3 TOPGeoModule::pointToGlobal(const TVector3& point) const
{
  if (!m_rotation) setTransformation();
  return (*m_rotation) * point + (*m_translation);
}

TVector3 TOPGeoModule::momentumToGlobal(const TVector3& momentum) const
{
  if (!m_rotation) setTransformation();
  return (*m_rotation) * momentum;
}

TVector3 TOPGeoModule::pointToLocal(const TVector3& point) const
{
  if (!m_rotation) setTransformation();
  return (*m_rotationInverse) * (point - (*m_translation));
}

TVector3 TOPGeoModule::momentumToLocal(const TVector3& momentum) const
{
  if (!m_rotation) setTransformation();
  return (*m_rotationInverse) * momentum;
}

void TOPGeoModule::setTransformation() const
{

  TRotation Rphi;
  Rphi.RotateZ(m_phi - M_PI / 2);
  TVector3 translation(0, m_radius, getZc() * s_unit);

  TRotation Rot = Rphi * m_moduleDisplacement.getRotation();
  translation += m_moduleDisplacement.getTranslation();
  m_rotation =  new TRotation(Rot);
  m_rotationInverse = new TRotation(Rot.Inverse());
  m_translation = new TVector3(Rphi * translation);
}


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

