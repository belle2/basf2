/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/reconstruction/LinSignalFitter.h>

#include <boost/foreach.hpp>

#include <TArrayI.h>
#include <TVectorD.h>
#include <TMatrixD.h>
#include <TDecompSVD.h>

using namespace std;
using namespace Belle2::SVD;
using namespace boost;


int LinSignalFitter::doFit()
{
  // 1. Make data vectors from the map
  int nData = m_data.size();
  // Exit with failure if not enough data.
  if (nData < 3) {
    m_charge = 0;
    m_chargeSigma = 1.0e3;
    m_initTime = 0;
    m_initTimeSigma = m_samplingTime;
    return bad_fit;
  }
  // 1c. Create the vectors.
  TArrayI times(nData);
  TVectorD signals(nData);
  int index = 0;
  double firstSampleTime = m_data.begin()->first;
  int iStartTime = int(firstSampleTime / m_samplingTime + 0.5);
  BOOST_FOREACH(signal_type::value_type sample, m_data) {
    // iTime is the time in terms of sampling periods.
    int iTime = - iStartTime + int(sample.first / m_samplingTime + 0.5);
    times[index] = iTime;
    signals[index] = sample.second;
    index++;
  }
  // 2. Form the regression matrices.
  // The regression model is
  // y_i = q^i * beta_0 + i * sampleTime/tau * q^i * beta_1,
  // with
  // q      = exp(-sampleTime/tau),
  // beta_0 = y_0 = C*(startTime - initTime)/tau * exp(-(startTime-initTime)/tau),
  // beta_1 = C * exp(-(startTime-initTime)/tau).
  // From there,
  // (startTime - initTime)/tau = beta_0/beta_1,
  // initTime = startTime - tau * beta_0/beta_1,
  // d(initTime)/d(beta_0) = -tau / beta_1,
  // d(initTime)/d(beta_1) = +tau * beta_0 / beta_1^2.
  // C = beta_1 * exp(beta_0/beta_1),
  // dC/d(beta_0) = C / beta_1,
  // dC/d(beta_1) = C / beta_1 * (1 - beta_0 / beta_1)
  // 2a. The factor matrix.
  TMatrixD F(nData, 2);
  double q = exp(-m_samplingTime / m_tau);
  double c1 = 1.0;
  double c2 = m_samplingTime / m_tau;
  for (int iRow = 0; iRow < nData; iRow++) {
    F(iRow, 0) = c1;
    c1 *= q;
    F(iRow, 1) = c2 * iRow;
    c2 *= q;
  }
  // 2b. The information matrix
  TMatrixD I(TMatrixD::kAtA, F);
  // 2c. The error covariance
  TDecompSVD decompI(I);
  TMatrixD covBeta(I);
  decompI.Invert(covBeta);
  // 2d. Fit coefficients beta
  TMatrixD S(covBeta, TMatrixD::kMultTranspose, F);
  TVectorD beta(S * signals);
  // 2e. Predictions and residual variance
  TVectorD predictions(F * beta);
  signal_type::iterator it_signal = m_data.begin();
  for (int i = 0; i < nData; ++i) {
    m_predictions.insert(make_pair(it_signal->first, predictions[i]));
    ++it_signal;
  }
  double sigma_res = (signals - predictions).Norm2Sqr() / (nData - 2.);
  // 2f. Estimates of actual parameters
  m_initTime = + firstSampleTime - beta(0) / beta(1) * m_tau;
  m_charge = beta(1) * exp(beta(0) / beta(1));
  // 2g. Covariance of actual parameters
  TMatrixD J(2, 2);
  J(0, 0) = -m_tau / beta(1);
  J(0, 1) = m_tau * beta(0) / beta(1) / beta(1);
  J(1, 0) = exp(beta(0) / beta(1));
  J(1, 1) = (1 - beta(0) / beta(1)) * J(1, 0);
  TMatrixD cov(J, TMatrixD::kMult, TMatrixD(covBeta, TMatrixD::kMultTranspose, J));
  m_initTimeSigma = sqrt(sigma_res * cov(0, 0));
  m_chargeSigma = sqrt(sigma_res * cov(1, 1));
  return good_fit;
}
