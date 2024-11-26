/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/KLMMuidHit.h>

using namespace Belle2;

KLMMuidHit::KLMMuidHit() :
  m_PdgCode(0),
  m_ExtPosition{0.0, 0.0, 0.0},
  m_ExtTime(0.0),
  m_HitPosition{0.0, 0.0, 0.0},
  m_HitTime(0.0),
  m_InBarrel(false),
  m_IsForward(false),
  m_Sector(0),
  m_Layer(0),
  m_ChiSquared(0.0)
{
}

KLMMuidHit::KLMMuidHit(int pdgCode, bool inBarrel, bool isForward, int sector, int layer,
                       const ROOT::Math::XYZVector& extPosition, const ROOT::Math::XYZVector& hitPosition,
                       double extTime, double hitTime, double chiSquared) :
  m_PdgCode(pdgCode),
  m_ExtTime(extTime),
  m_HitTime(hitTime),
  m_InBarrel(inBarrel),
  m_IsForward(isForward),
  m_Sector(sector),
  m_Layer(layer),
  m_ChiSquared(chiSquared)
{
  m_ExtPosition[0] = extPosition.X();
  m_ExtPosition[1] = extPosition.Y();
  m_ExtPosition[2] = extPosition.Z();
  m_HitPosition[0] = hitPosition.X();
  m_HitPosition[1] = hitPosition.Y();
  m_HitPosition[2] = hitPosition.Z();
}
