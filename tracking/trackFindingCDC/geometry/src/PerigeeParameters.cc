/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>

#include <tracking/trackFindingCDC/geometry/UncertainParameters.icc.h>

#include <tracking/trackFindingCDC/numerics/Angle.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template struct TrackFindingCDC::UncertainParametersUtil<PerigeeUtil, EPerigeeParameter>;

double PerigeeUtil::average(const PerigeeParameters& fromPar,
                            const PerigeeCovariance& fromCov,
                            const PerigeeParameters& toPar,
                            const PerigeeCovariance& toCov,
                            PerigeeParameters& avgPar,
                            PerigeeCovariance& avgCov)
{
  using namespace NPerigeeParameterIndices;
  PerigeeParameters refPar = (fromPar + toPar) / 2.0;
  refPar(c_Phi0) = AngleUtil::average(fromPar(c_Phi0), toPar(c_Phi0));

  PerigeeParameters relFromPar = fromPar - refPar;
  AngleUtil::normalise(relFromPar(c_Phi0));

  PerigeeParameters relToPar = toPar - refPar;
  AngleUtil::normalise(relToPar(c_Phi0));

  PerigeeParameters relAvgPar;

  double chi2 =
    CovarianceMatrixUtil::average(relFromPar, fromCov, relToPar, toCov, relAvgPar, avgCov);

  avgPar = relAvgPar + refPar;
  AngleUtil::normalise(avgPar(c_Phi0));

  return chi2;
}
