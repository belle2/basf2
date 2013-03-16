/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/ExtHit.h>

#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(ExtHit)

//! empty constructor
ExtHit::ExtHit() : RelationsObject()
{
}

//! Constructor with initial values
ExtHit::ExtHit(int pdgCode, ExtDetectorID detectorID, int copyID, ExtHitStatus status, double tof,
               const TVector3& position, const TVector3& momentum, const TMatrixD& covariance) :
  RelationsObject(),
  m_PdgCode(pdgCode),
  m_DetectorID(detectorID),
  m_CopyID(copyID),
  m_Status(status),
  m_TOF(tof),
  m_Position(position),
  m_Momentum(momentum),
  m_Covariance(covariance)
{
}

//! Copy constructor
ExtHit::ExtHit(const ExtHit& h) :
  RelationsObject(h),
  m_PdgCode(h.m_PdgCode),
  m_DetectorID(h.m_DetectorID),
  m_CopyID(h.m_CopyID),
  m_Status(h.m_Status),
  m_TOF(h.m_TOF),
  m_Position(h.m_Position),
  m_Momentum(h.m_Momentum),
  m_Covariance(h.m_Covariance)
{
}

