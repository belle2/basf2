/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/CDCRobustSZFitter.h>

#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <tracking/trackFindingCDC/fitting/CDCSZObservations.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace Eigen;

CDCTrajectorySZ CDCRobustSZFitter::fitUsingSimplifiedTheilSen(const CDCSZObservations& observationsSZ) const
{
  CDCTrajectorySZ trajectorySZ;
  CDCSZFitter szFitter;

  if (observationsSZ.size() > 4) {
    CDCSZObservations observationsSZFiltered;

    double meanTanLambda = 0;
    double meanStartZ = 0;

    for (unsigned int i = 0; i < observationsSZ.size(); i++) {
      for (unsigned int j = 0; j < observationsSZ.size(); j++) {
        if (i != j) {
          observationsSZFiltered.fill(observationsSZ.getS(j),
                                      observationsSZ.getZ(j),
                                      observationsSZ.getWeight(j));
        }
      } // for j

      szFitter.update(trajectorySZ, observationsSZFiltered);
      meanTanLambda += trajectorySZ.getTanLambda();
      meanStartZ += trajectorySZ.getZ0();
    } // for i

    return CDCTrajectorySZ(meanTanLambda / observationsSZ.size(),
                           meanStartZ / observationsSZ.size());
  } else {
    return CDCTrajectorySZ::basicAssumption();
  }
}
