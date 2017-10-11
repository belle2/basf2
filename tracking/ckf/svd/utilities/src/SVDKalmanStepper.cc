/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/svd/utilities/SVDKalmanStepper.h>

#include <tracking/ckf/svd/entities/CKFToSVDState.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/dataobjects/SVDCluster.h>

using namespace Belle2;

double SVDKalmanStepper::kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane, CKFToSVDState& state)
{
  double chi2 = 0;
  for (const std::unique_ptr<SVDRecoHit>& svdRecoHit : state.getRecoHits()) {
    const std::vector<genfit::MeasurementOnPlane*>& measurementsOnPlane = svdRecoHit->constructMeasurementsOnPlane(
          measuredStateOnPlane);

    B2ASSERT("I expect exactly one measurement here", measurementsOnPlane.size() == 1);
    const genfit::MeasurementOnPlane* measurementOnPlane = measurementsOnPlane.front();

    chi2 += this->kalmanStepImplementation<1>(measuredStateOnPlane, *measurementOnPlane);
  }
  return chi2;
}