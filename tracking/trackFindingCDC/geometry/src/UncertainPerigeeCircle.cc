/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>

#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>

#include <ostream>

using namespace Belle2;
using namespace TrackFindingCDC;

UncertainPerigeeCircle
UncertainPerigeeCircle::average(const UncertainPerigeeCircle& fromPerigeeCircle,
                                const UncertainPerigeeCircle& toPerigeeCircle)
{
  const PerigeeParameters& fromPar = fromPerigeeCircle.perigeeParameters();
  const PerigeeCovariance& fromCov = fromPerigeeCircle.perigeeCovariance();

  const PerigeeParameters& toPar = toPerigeeCircle.perigeeParameters();
  const PerigeeCovariance& toCov = toPerigeeCircle.perigeeCovariance();

  PerigeeParameters avgPar;
  PerigeeCovariance avgCov;

  double chi2 = PerigeeUtil::average(fromPar, fromCov, toPar, toCov, avgPar, avgCov);

  // Calculating 3 parameters from 6 input parameters. 3 NDF remaining.
  size_t ndf = 3;

  return UncertainPerigeeCircle(avgPar, avgCov, chi2, ndf);
}

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const UncertainPerigeeCircle& circle)
{
  return output << "UncertainPerigeeCircle("
         << "curvature=" << circle->curvature() << ","
         << "phi0=" << circle->phi0() << ","
         << "impact=" << circle->impact() << ")";
}
