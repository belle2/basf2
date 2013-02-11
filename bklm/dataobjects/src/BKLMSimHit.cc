/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/dataobjects/BKLMSimHit.h>

using namespace Belle2;

ClassImp(BKLMSimHit)

//! empty constructor for ROOT
BKLMSimHit::BKLMSimHit()
{
}

//! Constructor with initial values
BKLMSimHit::BKLMSimHit(const TVector3& position, double time, double deltaE,
                       double KE, unsigned int status,
                       bool isForward, int sector, int layer, int plane,
                       bool isPhiReadout, int stripMin, int stripMax) :
  m_Position(position),
  m_Time(time),
  m_DeltaE(deltaE),
  m_KE(KE),
  m_Status(status),
  m_IsForward(isForward),
  m_Sector(sector),
  m_Layer(layer),
  m_Plane(plane),
  m_IsPhiReadout(isPhiReadout),
  m_StripMin(stripMin),
  m_StripMax(stripMax)
{
}
