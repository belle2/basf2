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



TVector3 ARICHGeoMasterVolume::pointToGlobal(const TVector3& point) const
{
  return (*m_rotation) * point + (*m_translation);
}

TVector3 ARICHGeoMasterVolume::momentumToGlobal(const TVector3& momentum) const
{
  return (*m_rotation) * momentum;
}

TVector3 ARICHGeoMasterVolume::pointToLocal(const TVector3& point) const
{
  return (*m_rotationInverse) * (point - (*m_translation));
}

TVector3 ARICHGeoMasterVolume::momentumToLocal(const TVector3& momentum) const
{
  return (*m_rotationInverse) * momentum;
}

void ARICHGeoMasterVolume::setPlacement(double x, double y, double z, double rx, double ry, double rz)
{

  m_x = x; m_y = y; m_z = z; m_rx = rx; m_ry = ry; m_rz = rz;

  TVector3 translation(x, y, z);

  m_rotation =  new TRotation();
  m_rotation->RotateX(rx);
  m_rotation->RotateY(ry);
  m_rotation->RotateZ(rz);

  m_rotationInverse = new TRotation(m_rotation->Inverse());
  m_translation = new TVector3(x, y, z);

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
