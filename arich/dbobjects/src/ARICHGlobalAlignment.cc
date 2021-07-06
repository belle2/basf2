/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHGlobalAlignment.h>
#include <iostream>

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
  m_alignPars.print();
}
