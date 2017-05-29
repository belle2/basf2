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

// empty constructor for ROOT - do not use this
ExtHit::ExtHit() :
  RelationsObject(),
  m_PdgCode(0),
  m_DetectorID(Const::EDetector::invalidDetector),
  m_CopyID(0),
  m_Status(EXT_FIRST),
  m_TOF(0.0)
{
  m_Position[0] = 0.0;
  m_Position[1] = 0.0;
  m_Position[2] = 0.0;
  m_Momentum[0] = 0.0;
  m_Momentum[1] = 0.0;
  m_Momentum[2] = 0.0;
  for (int k = 0; k < 21; ++k) {
    m_Covariance[k] = 0.0;
  }
}

// Constructor with initial values
ExtHit::ExtHit(int pdgCode, Const::EDetector detectorID, int copyID, ExtHitStatus status, double tof,
               const TVector3& position, const TVector3& momentum, const TMatrixDSym& covariance) :
  RelationsObject(),
  m_PdgCode(pdgCode),
  m_DetectorID(detectorID),
  m_CopyID(copyID),
  m_Status(status),
  m_TOF(tof)
{
  m_Position[0] = position.X();
  m_Position[1] = position.Y();
  m_Position[2] = position.Z();
  m_Momentum[0] = momentum.X();
  m_Momentum[1] = momentum.Y();
  m_Momentum[2] = momentum.Z();
  int k = 0;
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j <= i; ++j) {
      m_Covariance[k++] = covariance[i][j];
    }
  }
}

// Constructor with initial values
ExtHit::ExtHit(int pdgCode, Const::EDetector detectorID, int copyID, ExtHitStatus status, double tof,
               const G4ThreeVector& position, const G4ThreeVector& momentum, const G4ErrorSymMatrix& covariance) :
  RelationsObject(),
  m_PdgCode(pdgCode),
  m_DetectorID(detectorID),
  m_CopyID(copyID),
  m_Status(status),
  m_TOF(tof)
{
  m_Position[0] = position.x();
  m_Position[1] = position.y();
  m_Position[2] = position.z();
  m_Momentum[0] = momentum.x();
  m_Momentum[1] = momentum.y();
  m_Momentum[2] = momentum.z();
  int k = 0;
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j <= i; ++j) {
      m_Covariance[k++] = covariance[i][j];
    }
  }
}

// Copy constructor
ExtHit::ExtHit(const ExtHit& h) :
  RelationsObject(h),
  m_PdgCode(h.m_PdgCode),
  m_DetectorID(h.m_DetectorID),
  m_CopyID(h.m_CopyID),
  m_Status(h.m_Status),
  m_TOF(h.m_TOF)
{
  m_Position[0] = h.m_Position[0];
  m_Position[1] = h.m_Position[1];
  m_Position[2] = h.m_Position[2];
  m_Momentum[0] = h.m_Momentum[0];
  m_Momentum[1] = h.m_Momentum[1];
  m_Momentum[2] = h.m_Momentum[2];
  for (int k = 0; k < 21; ++k) {
    m_Covariance[k] = h.m_Covariance[k];
  }
}

// Get phase-space covariance at this extrapolation hit
TMatrixDSym ExtHit::getCovariance() const
{
  TMatrixDSym t(6);
  int k = 0;
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j <= i; ++j) {
      t[j][i] = t[i][j] = m_Covariance[k++];
    }
  }
  return t;
}

// Update the information for this extrapolation hit
void ExtHit::update(ExtHitStatus status, double tof, const G4ThreeVector& position,
                    const G4ThreeVector& momentum, const G4ErrorSymMatrix& covariance)
{
  m_Status = status;
  m_TOF = tof;
  m_Position[0] = position.x();
  m_Position[1] = position.y();
  m_Position[2] = position.z();
  m_Momentum[0] = momentum.x();
  m_Momentum[1] = momentum.y();
  m_Momentum[2] = momentum.z();
  int k = 0;
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j <= i; ++j) {
      m_Covariance[k++] = covariance[i][j];
    }
  }
}
