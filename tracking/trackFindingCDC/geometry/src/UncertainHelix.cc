/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/UncertainHelix.h>

#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>

#include <tracking/trackFindingCDC/geometry/Helix.h>

#include <tracking/trackFindingCDC/geometry/HelixParameters.h>
#include <tracking/trackFindingCDC/geometry/PerigeeParameters.h>
#include <tracking/trackFindingCDC/geometry/SZParameters.h>

#include <ostream>

using namespace Belle2;
using namespace TrackFindingCDC;

UncertainHelix UncertainHelix::average(const UncertainHelix& fromHelix,
                                       const UncertainHelix& toHelix)
{
  HelixParameters fromPar = fromHelix.helixParameters();
  HelixCovariance fromCov = fromHelix.helixCovariance();

  HelixParameters toPar = toHelix.helixParameters();
  HelixCovariance toCov = toHelix.helixCovariance();

  HelixParameters avgPar;
  HelixCovariance avgCov;

  double chi2 = HelixUtil::average(fromPar, fromCov, toPar, toCov, avgPar, avgCov);

  // Calculating 5 parameters from 10 input parameters. 5 NDF remaining.
  size_t ndf = 5;

  return UncertainHelix(avgPar, avgCov, chi2, ndf);
}

UncertainHelix UncertainHelix::average(const UncertainPerigeeCircle& fromPerigeeCircle,
                                       const PerigeeHelixAmbiguity& fromAmbiguity,
                                       const UncertainHelix& toHelix)
{
  PerigeeParameters fromPar = fromPerigeeCircle.perigeeParameters();
  PerigeeCovariance fromCov = fromPerigeeCircle.perigeeCovariance();

  HelixParameters toPar = toHelix.helixParameters();
  HelixCovariance toCov = toHelix.helixCovariance();

  HelixParameters avgPar;
  HelixCovariance avgCov;

  double chi2 = HelixUtil::average(fromPar, fromCov, fromAmbiguity, toPar, toCov, avgPar, avgCov);

  // Calculating 5 parameters from 8 input parameters. 3 NDF remaining.
  size_t ndf = 3;

  return UncertainHelix(avgPar, avgCov, chi2, ndf);
}

UncertainHelix UncertainHelix::average(const UncertainPerigeeCircle& fromPerigeeCircle,
                                       const PerigeeHelixAmbiguity& fromAmbiguity,
                                       const UncertainPerigeeCircle& toPerigeeCircle,
                                       const PerigeeHelixAmbiguity& toAmbiguity,
                                       const SZParameters& szParameters)
{
  const PerigeeParameters& fromPar = fromPerigeeCircle.perigeeParameters();
  const PerigeeCovariance& fromCov = fromPerigeeCircle.perigeeCovariance();

  const PerigeeParameters& toPar = toPerigeeCircle.perigeeParameters();
  const PerigeeCovariance& toCov = toPerigeeCircle.perigeeCovariance();

  HelixParameters avgPar;
  HelixCovariance avgCov;

  double chi2 = HelixUtil::average(fromPar,
                                   fromCov,
                                   fromAmbiguity,
                                   toPar,
                                   toCov,
                                   toAmbiguity,
                                   szParameters,
                                   avgPar,
                                   avgCov);

  // Calculating 5 parameters from 6 input parameters. 1 NDF remaining.
  size_t ndf = 1;

  return UncertainHelix(avgPar, avgCov, chi2, ndf);
}

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const UncertainHelix& uncertainHelix)
{
  return output << "Uncertain" << uncertainHelix.helix();
}
