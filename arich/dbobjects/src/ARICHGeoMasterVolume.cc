/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHGeoMasterVolume.h>
#include <iostream>

using namespace std;
using namespace Belle2;



ROOT::Math::XYZVector ARICHGeoMasterVolume::pointToGlobal(const ROOT::Math::XYZVector& point) const
{
  return (*m_rotation) * point + (*m_translation);
}

ROOT::Math::XYZVector ARICHGeoMasterVolume::momentumToGlobal(const ROOT::Math::XYZVector& momentum) const
{
  return (*m_rotation) * momentum;
}

ROOT::Math::XYZVector ARICHGeoMasterVolume::pointToLocal(const ROOT::Math::XYZVector& point) const
{
  return (*m_rotationInverse) * (point - (*m_translation));
}

ROOT::Math::XYZVector ARICHGeoMasterVolume::momentumToLocal(const ROOT::Math::XYZVector& momentum) const
{
  return (*m_rotationInverse) * momentum;
}

void ARICHGeoMasterVolume::setPlacement(double x, double y, double z, double rx, double ry, double rz)
{

  m_x = x; m_y = y; m_z = z; m_rx = rx; m_ry = ry; m_rz = rz;

  ROOT::Math::XYZVector translation(x, y, z);

  m_rotation =  new ROOT::Math::Rotation3D();
  ROOT::Math::RotationX rotX(m_rx);
  ROOT::Math::RotationY rotY(m_ry);
  ROOT::Math::RotationZ rotZ(m_rz);
  (*m_rotation) *= rotZ * rotY * rotX;

  m_rotationInverse = new ROOT::Math::Rotation3D(m_rotation->Inverse());
  m_translation = new ROOT::Math::XYZVector(x, y, z);

}



bool ARICHGeoMasterVolume::isConsistent() const
{
  if (m_innerR > m_outerR) return false;
  if (m_length <= 0 || m_innerR < 0) return false;
  if (m_material.empty()) return false;
  return true;
}


void ARICHGeoMasterVolume::print(const std::string& title) const
{
  ARICHGeoBase::print(title);

  cout << " Tube inner radius: " << getInnerRadius() << " " << s_unitName << endl;
  cout << " Tube outer radius: " << getOuterRadius() << " " << s_unitName << endl;
  cout << " Tube length:       " << getLength() << " "   << s_unitName << endl;

  cout << "Positioning parameters (in Belle II global frame)" << endl;
  ARICHGeoBase::printPlacement(m_x, m_y, m_z, m_rx, m_ry, m_rz);

}
