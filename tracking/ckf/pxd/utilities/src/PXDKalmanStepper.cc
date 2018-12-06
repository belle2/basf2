/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/pxd/utilities/PXDKalmanStepper.h>

#include <tracking/ckf/pxd/entities/CKFToPXDState.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <pxd/dataobjects/PXDCluster.h>

using namespace Belle2;

double PXDKalmanStepper::kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const CKFToPXDState& state)
{
  double chi2 = 0;
  for (const PXDRecoHit& pxdRecoHit : state.getRecoHits()) {
    const std::vector<genfit::MeasurementOnPlane*>& measurementsOnPlane = pxdRecoHit.constructMeasurementsOnPlane(
          measuredStateOnPlane);

    B2ASSERT("I expect exactly one measurement here", measurementsOnPlane.size() == 1);
    const genfit::MeasurementOnPlane* measurementOnPlane = measurementsOnPlane.front();

    chi2 += m_kalmanStepper.kalmanStep(measuredStateOnPlane, *measurementOnPlane);

    delete measurementOnPlane;
  }
  return chi2;
}

double PXDKalmanStepper::calculateResidual(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const CKFToPXDState& state)
{
  double residual = 0;
  for (const PXDRecoHit& pxdRecoHit : state.getRecoHits()) {
    const std::vector<genfit::MeasurementOnPlane*>& measurementsOnPlane = pxdRecoHit.constructMeasurementsOnPlane(
          measuredStateOnPlane);

    B2ASSERT("I expect exactly one measurement here", measurementsOnPlane.size() == 1);
    const genfit::MeasurementOnPlane* measurementOnPlane = measurementsOnPlane.front();

    residual += m_kalmanStepper.calculateResidual(measuredStateOnPlane, *measurementOnPlane);

    delete measurementOnPlane;
  }
  return residual;
}