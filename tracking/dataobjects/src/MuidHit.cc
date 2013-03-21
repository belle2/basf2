/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/MuidHit.h>

using namespace std;
using namespace Belle2;

ClassImp(MuidHit)

MuidHit::MuidHit() :
  m_PdgCode(0),
  m_ExtTime(0.0),
  m_HitTime(0.0),
  m_InBarrel(false),
  m_IsForward(false),
  m_Sector(0),
  m_Layer(0),
  m_ChiSquared(0.0)
{
  m_ExtPosition = TVector3(0.0, 0.0, 0.0);
  m_HitPosition = m_ExtPosition;
}

MuidHit::MuidHit(int pdgCode, bool inBarrel, bool isForward, int sector, int layer, const TVector3& extPosition, const TVector3& hitPosition, double extTime, double hitTime, double chiSquared)
{
  m_PdgCode = pdgCode;
  m_InBarrel = inBarrel;
  m_IsForward = isForward;
  m_Sector = sector;
  m_Layer = layer;
  m_ExtPosition = extPosition;
  m_HitPosition = hitPosition;
  m_ExtTime = extTime;
  m_HitTime = hitTime;
  m_ChiSquared = chiSquared;
}
