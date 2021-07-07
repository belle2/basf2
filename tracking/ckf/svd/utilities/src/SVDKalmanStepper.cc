/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/ckf/svd/utilities/SVDKalmanStepper.h>

#include <tracking/ckf/svd/entities/CKFToSVDState.h>
#include <svd/reconstruction/SVDRecoHit.h>

using namespace Belle2;

double SVDKalmanStepper::kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const CKFToSVDState& state)
{
  double chi2 = 0;
  for (const SVDRecoHit& svdRecoHit : state.getRecoHits()) {
    const std::vector<genfit::MeasurementOnPlane*>& measurementsOnPlane = svdRecoHit.constructMeasurementsOnPlane(
          measuredStateOnPlane);

    B2ASSERT("I expect exactly one measurement here", measurementsOnPlane.size() == 1);
    const genfit::MeasurementOnPlane* measurementOnPlane = measurementsOnPlane.front();

    chi2 += m_kalmanStepper.kalmanStep(measuredStateOnPlane, *measurementOnPlane);

    delete measurementOnPlane;
  }
  return chi2;
}

double SVDKalmanStepper::calculateResidual(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const CKFToSVDState& state)
{
  double residual = 0;
  for (const SVDRecoHit& svdRecoHit : state.getRecoHits()) {
    const std::vector<genfit::MeasurementOnPlane*>& measurementsOnPlane = svdRecoHit.constructMeasurementsOnPlane(
          measuredStateOnPlane);

    B2ASSERT("I expect exactly one measurement here", measurementsOnPlane.size() == 1);
    const genfit::MeasurementOnPlane* measurementOnPlane = measurementsOnPlane.front();

    residual += m_kalmanStepper.calculateResidual(measuredStateOnPlane, *measurementOnPlane);

    delete measurementOnPlane;
  }
  return residual;
}
