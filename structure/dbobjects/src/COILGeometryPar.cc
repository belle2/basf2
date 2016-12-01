/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexandre BEAULIEU                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <structure/dbobjects/COILGeometryPar.h>
#include <framework/logging/Logger.h>

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





