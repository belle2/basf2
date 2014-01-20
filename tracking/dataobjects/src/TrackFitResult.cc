/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/dataobjects/TrackFitResult.h>
#include <cmath>
#include <TMath.h>
#include <genfit/AbsTrackRep.h>

using namespace Belle2;

ClassImp(TrackFitResult);

TrackFitResult::TrackFitResult() : m_perigeeParams(5, 0), m_perigeeUncertainties(15, 0), m_pdg(0), m_pValue(0), m_charge(0), m_storeAsPerigee(false)
{
}

TrackFitResult::TrackFitResult(const TVector3& position, const TVector3& momentum, const short int& charge, const TMatrixF& covariance,
                               const unsigned int& pdg, const float& pValue, bool storeAsPerigee)
{
  m_storeAsPerigee = storeAsPerigee;
  m_pdg = pdg;
  m_pValue = pValue;

  if (storeAsPerigee == true) {
    // Calculate helix parameters
    cartesianToHelix(position, momentum, charge);
    cartesianCovToHelixCov(position, momentum, charge, covariance);
    // Set cartesian to zero
    //m_position = (0,0,0);
    // m_momentum = (0,0,0);
    m_charge = 0;
    m_cov6.ResizeTo(6, 6);
    for (unsigned int rr(0); rr < 6; ++rr) { // fill rows
      for (unsigned int cc(rr); cc < 6; ++cc) { // fill columns
        m_cov6(rr, cc) = m_cov6(cc, rr) = 0;
      } // column
    } // row
  } else if (storeAsPerigee == false) {
    // Set cartesian parameters
    m_position = position;
    m_momentum = momentum;
    m_charge = charge;
    m_cov6.ResizeTo(6, 6);
    m_cov6 = covariance;
    // helix parameter vectors are size 0
  }

}

TMatrixF TrackFitResult::getCovariance6() const
{
  if (m_storeAsPerigee == true) {
    // nice names
    double d0(m_perigeeParams[0]);
    double phi0(m_perigeeParams[1]);
    double omega(m_perigeeParams[2]);
    //double z0(m_perigeeParams[3]);
    double cotTheta(m_perigeeParams[4]);
    // with alpha = 1/(1.5 * 0.00299792458), obtained from c and the magnetic field strength of 1.5T.
    float alpha(222.3760634654347);

    TMatrixF transformation(6, 5);
    // Fill transformation matrix with zero and do the non-zero entries later
    for (unsigned int rr(0); rr < 6; ++rr) { // fill rows
      for (unsigned int cc(0); cc < 5; ++cc) { // fill columns
        transformation(rr, cc) = 0;
      } // column
    } // row
    // Calculate non zero elements
    transformation(0, 0) = std::sin(phi0);
    transformation(0, 1) = d0 * std::cos(phi0);
    transformation(1, 0) = -std::cos(phi0);
    transformation(1, 1) = d0 * std::sin(phi0);
    transformation(2, 3) = 1;
    transformation(3, 1) = -std::sin(phi0) / (alpha * omega);
    transformation(3, 2) = -std::cos(phi0) / (alpha * std::pow(omega, 2));
    transformation(4, 1) = std::cos(phi0) / (alpha * omega);
    transformation(4, 2) = -std::sin(phi0) / (alpha * std::pow(omega, 2));
    transformation(5, 2) = -cotTheta / (alpha * std::pow(omega, 2));
    transformation(5, 4) = 1.0 / (alpha * omega);

    // Transposed transformation matrix
    TMatrixF transformationT(5, 6);
    transformationT.Transpose(transformation);

    // Build perigee covariance matrix
    TMatrixF covPerigee(5, 5);
    unsigned int ii(0); // perigee uncertainties entry
    for (unsigned int rr(0); rr < 5; ++rr) { // fill rows
      for (unsigned int cc(rr); cc < 5; ++cc) { // fill columns
        covPerigee(rr, cc) = covPerigee(cc, rr) = m_perigeeUncertainties[ii];
        ++ii;
      } // column
    } // row

    // Calculate cartesian covariance matrix.
    // cov6 = A*cov5*A^T
    TMatrixF covCartesian(6, 6);
    covCartesian = transformation * covPerigee * transformationT;

    return covCartesian;
  } else {
    return m_cov6;
  }

}

/** Getter for the x value of the poca */
float TrackFitResult::getX0() const
{
  return m_perigeeParams[0] * std::sin(m_perigeeParams[1]);
}
/** Getter for the y value of the poca */
float TrackFitResult::getY0() const
{
  return -m_perigeeParams[0] * std::cos(m_perigeeParams[1]);
}
/** Getter for the px value at the poca */
float TrackFitResult::getPx0() const
{
  return std::cos(m_perigeeParams[1]) / std::fabs(m_perigeeParams[2] * 222.3760634654347);
}
/** Getter for the py value at the poca */
float TrackFitResult::getPy0() const
{
  return std::sin(m_perigeeParams[1]) / std::fabs(m_perigeeParams[2] * 222.3760634654347);
}
/** Getter for the pz value at the poca */
float TrackFitResult::getPz0() const
{
  return m_perigeeParams[4] / std::fabs(m_perigeeParams[2] * 222.3760634654347);
}
/** Getter for the poca vector */
TVector3 TrackFitResult::getPosition() const
{
  if (m_storeAsPerigee == true) {
    return TVector3(m_perigeeParams[0] * std::sin(m_perigeeParams[1]), -m_perigeeParams[0] * std::cos(m_perigeeParams[1]), m_perigeeParams[3]);
  } else {
    return m_position;
  }
}
/** Getter for the momentum vector at the poca */
TVector3 TrackFitResult::getMomentum() const
{
  if (m_storeAsPerigee == true) {
    float pt(std::fabs(m_perigeeParams[2] * 222.3760634654347));
    return TVector3(std::cos(m_perigeeParams[1]) / pt, std::sin(m_perigeeParams[1]) / pt, m_perigeeParams[4] / pt);
  } else {
    return m_momentum;
  }

}

/** Getter for helix parameter d0 */
float TrackFitResult::getD0() const
{
  return m_perigeeParams[0];
}
/** Getter for helix parameter phi */
float TrackFitResult::getPhi0() const
{
  return m_perigeeParams[1];
}
/** Getter for helix parameter omega */
float TrackFitResult::getOmega() const
{
  return m_perigeeParams[2];
}
/** Getter for helix parameter z0 */
float TrackFitResult::getZ0() const
{
  return m_perigeeParams[3];
}
/** Getter for helix parameter cotTheta */
float TrackFitResult::getCotTheta() const
{
  return m_perigeeParams[4];
}
/** Getter for the helix parameter covariance matrix */
TMatrixF TrackFitResult::getCovariance5() const
{
  TMatrixF cov(5, 5);
  unsigned int ii(0); // iterate over perigee uncertainties
  for (unsigned int rr(0); rr < 5; ++rr) { // fill rows
    for (unsigned int cc(rr); cc < 5; ++cc) { // fill columns
      cov(rr, cc) = cov(cc, rr) = m_perigeeUncertainties[ii];
      ++ii;
    } // column
  } // row

  return cov;
}


void TrackFitResult::cartesianToHelix(const TVector3& position, const TVector3& momentum, const short int& charge)
{
  // with alpha = 1/(1.5 * 0.00299792458), obtained from c and the magnetic field strength of 1.5T. See phd thesis from Oksana Lutz.
  float alpha(222.3760634654347);

  // Calculate the d0 sign
  // see: Fast vertex fitting with a local parametrization of tracks - Billoir, Pierre et al. Nucl.Instrum.Meth. A311 (1992) 139-150\
  // positive (negative) if angle between d0 and pt > (<) pi/2
  float positivePocaAngle(position.Phi() > 0 ? position.Phi() : 2 * TMath::Pi() + position.Phi());
  float positivePtAngle(momentum.Phi() > 0 ? momentum.Phi() : 2 * TMath::Pi() + momentum.Phi());
  float phiDiff(positivePtAngle - positivePocaAngle);
  if (phiDiff < 0.0) phiDiff += 2 * TMath::Pi();
  short int d0sign(phiDiff > TMath::Pi() ? -1 : 1);

  //d0
  m_perigeeParams.push_back(d0sign * position.Perp());
  //phi0
  m_perigeeParams.push_back(momentum.Phi());
  //omega
  m_perigeeParams.push_back(charge / (alpha * momentum.Perp()));
  //z0
  m_perigeeParams.push_back(position.z());
  //cotTheta
  m_perigeeParams.push_back(momentum.Pz() / momentum.Perp());
}

void TrackFitResult::cartesianCovToHelixCov(const TVector3& position, const TVector3& momentum, const short int& charge, const TMatrixF& cov)
{
  // rename variables for readability
  // transverse momentum
  double pt(momentum.Perp());
  // position
  double px(momentum.Px());
  double py(momentum.Py());
  double pz(momentum.Pz());
  // position
  double x(position.X());
  double y(position.Y());
  // with alpha = 1/(1.5 * 0.00299792458), obtained from c and the magnetic field strength of 1.5T.
  float alpha(222.3760634654347);


  TMatrixF transformation(5, 6);
  // Fill transformation matrix with zero and do the non-zero entries later
  for (unsigned int rr(0); rr < 5; ++rr) { // fill rows
    for (unsigned int cc(0); cc < 6; ++cc) { // fill columns
      transformation(rr, cc) = 0;
    } // column
  } // row
  transformation(0, 0) = x / std::sqrt(x * x + y * y);
  transformation(0, 1) = y / std::sqrt(x * x + y * y);
  transformation(1, 3) = -py / std::pow(pt, 2);
  transformation(1, 4) = px / std::pow(pt, 2);
  transformation(2, 3) = -px * charge / (alpha * std::pow(pt, 3));
  transformation(2, 4) = -py * charge / (alpha * std::pow(pt, 3));
  transformation(3, 2) = 1;
  transformation(4, 3) = -px * pz / std::pow(pt, 3);
  transformation(4, 4) = -py * pz / std::pow(pt, 3);
  transformation(4, 5) = 1 / pt;


  TMatrixF transformationT(6, 5);
  transformationT.Transpose(transformation);

  TMatrixF covPerigee(transformation * cov * transformationT);

  m_perigeeUncertainties.resize(15);
  unsigned int ii(0); // perigee uncertainties entry
  for (unsigned int rr(0); rr < 5; ++rr) { // fill rows
    for (unsigned int cc(rr); cc < 5; ++cc) { // fill columns
      m_perigeeUncertainties[ii] = covPerigee(rr, cc);
      ++ii;
    } // column
  } // row
}




//bool TrackFitResult:: hitInSuperLayer(unsigned int iSuperLayer) const {}



// Implementation note: Typically we will either have PXD or SVD hits,
// so it makes a lot of sense to simply start inside and test going further outside.
// Case without hits should never happen.
/*unsigned short TrackFitResult::getIInnermostLayer() const
{
  unsigned short ii = 0;
  while (!m_hitPattern[ii++]) {}
  return ii;
}
*/
