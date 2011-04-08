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

using namespace std;
using namespace Belle2;

ClassImp(SiSensorPlane)

SiSensorPlane::SiSensorPlane(double u, double v, double du, double dv, double dv2):
    m_u(u), m_v(v), m_du(du), m_dv(dv), m_dv2(dv2)
{
  m_du *= 0.5;
  m_dv *= 0.5;
  if (m_dv2 <= 0) // rectangular sensor
    dv2 = dv;
  else           // trapezoid
    dv2 *= 0.5;
  // Mean width in v and slope
  m_dv0 = 0.5 * (m_dv + m_dv2);
  m_dvSlope = 0.5 * (m_dv - m_dv2);
  if (m_du > 0)
    m_dvSlope = m_dvSlope / m_du;
  else {
    m_du = 0;
    m_dvSlope = 0;
  }
}


SiSensorPlane::SiSensorPlane(): m_u(0), m_v(0), m_du(0), m_dv(0), m_dv2(0)
{
  ;
}

void SiSensorPlane::Print() const
{
  B2INFO("SiDetectorPlane object:")
  B2INFO("- Center: (" << m_u << "," << m_v << ")");
  B2INFO("- Size:   (" << m_du << "," << m_dv << "," << m_dv2 << ")" << endl)
}

