/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/dataobjects/SiSensorPlane.h>
#include <framework/logging/Logger.h>


using namespace Belle2;

ClassImp(SiSensorPlane)


void SiSensorPlane::Print() const
{
  B2INFO("SiSensorPlane object:")
  B2INFO("- Center: (" << m_u << ", " << m_v << ")");
  B2INFO("- Size:   (" << 2*m_du << " +/- " << 2*m_dv*m_dudv << ", " << 2*m_dv << ")")
}

