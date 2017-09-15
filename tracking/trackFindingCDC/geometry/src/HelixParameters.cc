/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/UncertainParameters.icc.h>

#include <tracking/trackFindingCDC/geometry/HelixParameters.h>

#include <tracking/trackFindingCDC/numerics/CovarianceMatrixUtil.h>
#include <tracking/trackFindingCDC/numerics/ParameterVectorUtil.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template class TrackFindingCDC::UncertainParametersUtil<HelixUtil, EHelixParameter>;

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
