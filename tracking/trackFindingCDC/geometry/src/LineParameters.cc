/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/LineParameters.h>

#include <tracking/trackFindingCDC/geometry/UncertainParameters.icc.h>

#include <tracking/trackFindingCDC/numerics/Angle.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template struct TrackFindingCDC::UncertainParametersUtil<LineUtil, ELineParameter>;

double LineUtil::average(const LineParameters& fromPar,
                         const LineCovariance& fromCov,
                         const LineParameters& toPar,
                         const LineCovariance& toCov,
                         LineParameters& avgPar,
                         LineCovariance& avgCov)
{
  using namespace NLineParameterIndices;
  LineParameters refPar;
  refPar(c_I) = (fromPar(c_I) + toPar(c_I)) / 2;
  refPar(c_Phi0) = AngleUtil::average(fromPar(c_Phi0), toPar(c_Phi0));

  LineParameters relFromPar;
  relFromPar(c_I) = fromPar(c_I) - refPar(c_I);
  relFromPar(c_Phi0) = AngleUtil::normalised(fromPar(c_Phi0) - refPar(c_Phi0));

  LineParameters relToPar;
  relToPar(c_I) = toPar(c_I) - refPar(c_I);
  relToPar(c_Phi0) = AngleUtil::normalised(toPar(c_Phi0) - refPar(c_Phi0));

  LineParameters relAvgPar;
  double chi2 =
    CovarianceMatrixUtil::average(relFromPar, fromCov, relToPar, toCov, relAvgPar, avgCov);

  avgPar(c_I) = relAvgPar(c_I) + refPar(c_I);
  avgPar(c_Phi0) = AngleUtil::normalised(relAvgPar(c_Phi0) + refPar(c_Phi0));

  return chi2;
}
