/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
