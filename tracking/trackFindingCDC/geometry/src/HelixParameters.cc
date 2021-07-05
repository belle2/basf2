/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/UncertainParameters.icc.h>

#include <tracking/trackFindingCDC/geometry/HelixParameters.h>

#include <tracking/trackFindingCDC/numerics/CovarianceMatrixUtil.h>
#include <tracking/trackFindingCDC/numerics/ParameterVectorUtil.h>

#include <tracking/trackFindingCDC/numerics/Angle.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template struct TrackFindingCDC::UncertainParametersUtil<HelixUtil, EHelixParameter>;

HelixUtil::ParameterVector HelixUtil::reversalSigns()
{
  return ParameterVectorUtil::stack(PerigeeUtil::reversalSigns(), SZUtil::reversalSigns());
}

PerigeeUtil::ParameterVector HelixUtil::getPerigeeParameters(const ParameterVector& helixPar)
{
  using namespace NHelixParameterIndices;
  return getSubParameterVector<PerigeeUtil::ParameterVector, c_Curv>(helixPar);
}

SZUtil::ParameterVector HelixUtil::getSZParameters(const ParameterVector& helixPar)
{
  using namespace NHelixParameterIndices;
  return getSubParameterVector<SZUtil::ParameterVector, c_TanL>(helixPar);
}

HelixUtil::ParameterVector HelixUtil::stack(const PerigeeUtil::ParameterVector& perigeePar,
                                            const SZUtil::ParameterVector& szPar)
{
  ParameterVector result = ParameterVectorUtil::stack(perigeePar, szPar);
  return result;
}

PerigeeUtil::CovarianceMatrix HelixUtil::getPerigeeCovariance(const CovarianceMatrix& helixCov)
{
  using namespace NHelixParameterIndices;
  return getSubCovarianceMatrix<PerigeeUtil::CovarianceMatrix, c_Curv>(helixCov);
}

SZUtil::CovarianceMatrix HelixUtil::getSZCovariance(const CovarianceMatrix& helixCov)
{
  using namespace NHelixParameterIndices;
  return getSubCovarianceMatrix<SZUtil::CovarianceMatrix, c_TanL>(helixCov);
}

HelixUtil::CovarianceMatrix HelixUtil::stackBlocks(const PerigeeUtil::CovarianceMatrix& perigeeCov,
                                                   const SZUtil::CovarianceMatrix& szCov)
{
  CovarianceMatrix result = CovarianceMatrixUtil::stackBlocks(perigeeCov, szCov);
  return result;
}

HelixUtil::PerigeeAmbiguity HelixUtil::defaultPerigeeAmbiguity()
{
  return PerigeeAmbiguity::Zero();
}

double HelixUtil::average(const HelixParameters& fromPar,
                          const HelixCovariance& fromCov,
                          const HelixParameters& toPar,
                          const HelixCovariance& toCov,
                          HelixParameters& avgPar,
                          HelixCovariance& avgCov)
{
  using namespace NHelixParameterIndices;
  HelixParameters refPar = (fromPar + toPar) / 2.0;
  refPar[c_Phi0] = AngleUtil::average(fromPar[c_Phi0], toPar[c_Phi0]);

  HelixParameters relFromPar = fromPar - refPar;
  AngleUtil::normalise(relFromPar[c_Phi0]);

  HelixParameters relToPar = toPar - refPar;
  AngleUtil::normalise(relToPar[c_Phi0]);

  HelixParameters relAvgPar;
  double chi2 =
    CovarianceMatrixUtil::average(relFromPar, fromCov, relToPar, toCov, relAvgPar, avgCov);

  avgPar = relAvgPar + refPar;
  AngleUtil::normalise(avgPar[c_Phi0]);

  return chi2;
}

double HelixUtil::average(const PerigeeParameters& fromPar,
                          const PerigeeCovariance& fromCov,
                          const PerigeeHelixAmbiguity& fromAmbiguity,
                          const HelixParameters& toPar,
                          const HelixCovariance& toCov,
                          HelixParameters& avgPar,
                          HelixCovariance& avgCov)
{
  auto toAmbiguity = JacobianMatrixUtil::identity<5>();

  using namespace NPerigeeParameterIndices;
  PerigeeParameters refPar = (fromPar + toPar.head<3>()) / 2.0;
  refPar[c_Phi0] = AngleUtil::average(fromPar[c_Phi0], toPar[c_Phi0]);

  HelixParameters refHelixPar = stack(refPar, toPar.tail<2>());

  PerigeeParameters relFromPar = fromPar - refPar;
  AngleUtil::normalise(relFromPar[c_Phi0]);

  HelixParameters relToPar = toPar - refHelixPar;
  AngleUtil::normalise(relToPar[c_Phi0]);

  HelixParameters relAvgPar;

  // Chi2 value
  double chi2 = CovarianceMatrixUtil::average(relFromPar,
                                              fromCov,
                                              fromAmbiguity,
                                              relToPar,
                                              toCov,
                                              toAmbiguity,
                                              relAvgPar,
                                              avgCov);

  avgPar = relAvgPar + refHelixPar;
  AngleUtil::normalise(avgPar[c_Phi0]);

  return chi2;
}

double HelixUtil::average(const PerigeeParameters& fromPar,
                          const PerigeeCovariance& fromCov,
                          const PerigeeHelixAmbiguity& fromAmbiguity,
                          const PerigeeParameters& toPar,
                          const PerigeeCovariance& toCov,
                          const PerigeeHelixAmbiguity& toAmbiguity,
                          const SZParameters& szParameters,
                          HelixParameters& avgPar,
                          HelixCovariance& avgCov)
{
  using namespace NPerigeeParameterIndices;
  PerigeeParameters refPar = (fromPar + toPar) / 2.0;
  refPar[c_Phi0] = AngleUtil::average(fromPar[c_Phi0], toPar[c_Phi0]);

  HelixParameters refHelixPar = stack(refPar, szParameters);

  PerigeeParameters relFromPar = fromPar - refPar;
  AngleUtil::normalise(relFromPar[c_Phi0]);

  PerigeeParameters relToPar = toPar - refPar;
  AngleUtil::normalise(relToPar[c_Phi0]);

  HelixParameters relAvgPar;

  // Chi2 value
  double chi2 = CovarianceMatrixUtil::average(relFromPar,
                                              fromCov,
                                              fromAmbiguity,
                                              relToPar,
                                              toCov,
                                              toAmbiguity,
                                              relAvgPar,
                                              avgCov);

  avgPar = relAvgPar + refHelixPar;
  AngleUtil::normalise(avgPar[c_Phi0]);

  return chi2;
}
