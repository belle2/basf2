/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
