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
BKLMSimHit::BKLMSimHit() :
  SimHitBase(),
  m_Position(TVector3(0.0, 0.0, 0.0)),
  m_Time(0.0),
  m_DeltaE(0.0),
  m_KE(0.0),
  m_Status(0),
  m_IsForward(false),
  m_Sector(0),
  m_Layer(0),
  m_Plane(0),
  m_IsPhiReadout(false),
  m_StripMin(0),
  m_StripMax(0)
{
}

//! Constructor with initial values
BKLMSimHit::BKLMSimHit(const TVector3& position, double time, double deltaE,
                       double KE, unsigned int status,
                       bool isForward, int sector, int layer, int plane,
                       bool isPhiReadout, int stripMin, int stripMax) :
  SimHitBase(),
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
