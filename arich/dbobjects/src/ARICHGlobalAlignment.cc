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



ROOT::Math::XYZVector ARICHGlobalAlignment::pointToGlobal(const ROOT::Math::XYZVector& point) const
{
  if (!m_rotation) setTransformation();
  return (*m_rotation) * point + (*m_translation);
}

ROOT::Math::XYZVector ARICHGlobalAlignment::momentumToGlobal(const ROOT::Math::XYZVector& momentum) const
{
  if (!m_rotation) setTransformation();
  return (*m_rotation) * momentum;
}

ROOT::Math::XYZVector ARICHGlobalAlignment::pointToLocal(const ROOT::Math::XYZVector& point) const
{
  if (!m_translation) setTransformation();
  return (*m_rotationInverse) * (point - (*m_translation));
}

ROOT::Math::XYZVector ARICHGlobalAlignment::momentumToLocal(const ROOT::Math::XYZVector& momentum) const
{
  if (!m_rotationInverse) setTransformation();
  return (*m_rotationInverse) * momentum;
}

void ARICHGlobalAlignment::setTransformation() const
{
  ROOT::Math::Rotation3D Rot = m_alignPars.getRotation();
  m_rotation =  new ROOT::Math::Rotation3D(Rot);
  m_rotationInverse = new ROOT::Math::Rotation3D(Rot.Inverse());
  m_translation = new ROOT::Math::XYZVector(m_alignPars.getTranslation());
}

void ARICHGlobalAlignment::print(const std::string& title) const
{
  ARICHGeoBase::print(title);
  m_alignPars.print();
}
