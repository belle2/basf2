/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <structure/dbobjects/COILGeometryPar.h>

using namespace Belle2;

COILGeometryPar::COILGeometryPar() : m_Version(0),
  m_GlobalRotAngle(0),
  m_GlobalOffsetZ(0),
  m_CryoRmin(0),
  m_CryoRmax(0),
  m_CryoLength(0),
  m_Cav1Rmin(0),
  m_Cav1Rmax(0),
  m_Cav1Length(0),
  m_Cav2Rmin(0),
  m_Cav2Rmax(0),
  m_Cav2Length(0),
  m_ShieldRmin(0),
  m_ShieldRmax(0),
  m_ShieldLength(0),
  m_CoilRmin(0),
  m_CoilRmax(0),
  m_CoilLength(0)

{
}

COILGeometryPar::~COILGeometryPar()
{
}





