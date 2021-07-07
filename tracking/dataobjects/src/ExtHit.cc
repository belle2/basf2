/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/dataobjects/ExtHit.h>

#include <TMatrixD.h>
#include <TMatrixDSym.h>

#include <cmath>

using namespace Belle2;

// empty constructor for ROOT - do not use this
ExtHit::ExtHit() :
  RelationsObject(),
  m_PdgCode(0),
  m_DetectorID(Const::EDetector::invalidDetector),
  m_CopyID(0),
  m_Status(EXT_FIRST),
  m_BackwardPropagation(false),
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
ExtHit::ExtHit(int pdgCode, Const::EDetector detectorID, int copyID, ExtHitStatus status, bool backwardPropagation, double tof,
               const TVector3& position, const TVector3& momentum, const TMatrixDSym& covariance) :
  RelationsObject(),
  m_PdgCode(pdgCode),
  m_DetectorID(detectorID),
  m_CopyID(copyID),
  m_Status(status),
  m_BackwardPropagation(backwardPropagation),
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
ExtHit::ExtHit(int pdgCode, Const::EDetector detectorID, int copyID, ExtHitStatus status, bool backwardPropagation, double tof,
               const G4ThreeVector& position, const G4ThreeVector& momentum, const G4ErrorSymMatrix& covariance) :
  RelationsObject(),
  m_PdgCode(pdgCode),
  m_DetectorID(detectorID),
  m_CopyID(copyID),
  m_Status(status),
  m_BackwardPropagation(backwardPropagation),
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

// Assignment operator
ExtHit& ExtHit::operator=(const ExtHit& h)
{
  m_PdgCode = h.m_PdgCode;
  m_DetectorID = h.m_DetectorID;
  m_CopyID = h.m_CopyID;
  m_Status = h.m_Status;
  m_TOF = h.m_TOF;
  m_Position[0] = h.m_Position[0];
  m_Position[1] = h.m_Position[1];
  m_Position[2] = h.m_Position[2];
  m_Momentum[0] = h.m_Momentum[0];
  m_Momentum[1] = h.m_Momentum[1];
  m_Momentum[2] = h.m_Momentum[2];
  for (int k = 0; k < 21; ++k) {
    m_Covariance[k] = h.m_Covariance[k];
  }
  return *this;
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

// Get the uncertainty in the polar angle theta
double ExtHit::getErrorTheta() const
{
  return sqrt(getPolarCovariance(1));
}

// Get the uncertainty in the azimuthal angle phi
double ExtHit::getErrorPhi() const
{
  return sqrt(getPolarCovariance(2));
}

double ExtHit::getPolarCovariance(int i) const
{
  TMatrixDSym covariance(3);
  covariance[0][0] = m_Covariance[0];
  covariance[0][1] = covariance[1][0] = m_Covariance[1];
  covariance[1][1] = m_Covariance[2];
  covariance[0][2] = covariance[2][0] = m_Covariance[3];
  covariance[1][2] = covariance[2][1] = m_Covariance[4];
  covariance[2][2] = m_Covariance[5];
  double perpSq = m_Position[0] * m_Position[0] + m_Position[1] * m_Position[1];
  double perp = sqrt(perpSq);
  double rSq = perpSq + m_Position[2] * m_Position[2];
  double r = sqrt(rSq);
  TMatrixD jacobian(3, 3);
  jacobian[0][0] =  m_Position[0] / r;
  jacobian[0][1] =  m_Position[1] / r;
  jacobian[0][2] =  m_Position[2] / r;
  jacobian[1][0] =  m_Position[0] * m_Position[2] / (perp * rSq);
  jacobian[1][1] =  m_Position[1] * m_Position[2] / (perp * rSq);
  jacobian[1][2] = -perp / rSq;
  jacobian[2][0] = -m_Position[1] / perpSq;
  jacobian[2][1] =  m_Position[0] / perpSq;
  jacobian[2][2] =  0.0;
  return (covariance.Similarity(jacobian))[i][i];
}
