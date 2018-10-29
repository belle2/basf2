/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHGlobalAlignment.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>
#include <TVector2.h>

using namespace std;
using namespace Belle2;



TVector3 ARICHGlobalAlignment::pointToGlobal(const TVector3& point) const
{
  if (!m_rotation) setTransformation();
  return (*m_rotation) * point + (*m_translation);
}

TVector3 ARICHGlobalAlignment::momentumToGlobal(const TVector3& momentum) const
{
  if (!m_rotation) setTransformation();
  return (*m_rotation) * momentum;
}

TVector3 ARICHGlobalAlignment::pointToLocal(const TVector3& point) const
{
  if (!m_translation) setTransformation();
  return (*m_rotationInverse) * (point - (*m_translation));
}

TVector3 ARICHGlobalAlignment::momentumToLocal(const TVector3& momentum) const
{
  if (!m_rotationInverse) setTransformation();
  return (*m_rotationInverse) * momentum;
}

void ARICHGlobalAlignment::setTransformation() const
{
  TRotation Rot = m_alignPars.getRotation();
  m_rotation =  new TRotation(Rot);
  m_rotationInverse = new TRotation(Rot.Inverse());
  m_translation = new TVector3(m_alignPars.getTranslation());
}

void ARICHGlobalAlignment::print(const std::string& title) const
{
  ARICHGeoBase::print(title);

  cout << "Positioning parameters (in Belle II global frame)" << endl;
  //  ARICHGeoBase::printPlacement(m_dx, m_dy, m_dz, m_drx, m_dry, m_drz);

}
