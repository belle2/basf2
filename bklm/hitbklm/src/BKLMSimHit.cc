/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/hitbklm/BKLMSimHit.h>

using namespace Belle2;

ClassImp(BKLMSimHit)

//! empty constructor
BKLMSimHit::BKLMSimHit()
{
}

//! Constructor with initial values
BKLMSimHit::BKLMSimHit(TVector3 hitPosition, double hitTime, double deltaE,
                       bool inRPC, bool decayed,
                       int frontBack, int sector, int layer, int plane) :
    m_HitPosition(hitPosition),
    m_HitTime(hitTime),
    m_DeltaE(deltaE),
    m_InRPC(inRPC),
    m_Decayed(decayed),
    m_FrontBack(frontBack),
    m_Sector(sector),
    m_Layer(layer),
    m_Plane(plane)
{
  m_PhiStrips.clear();
  m_ZStrips.clear();
}
