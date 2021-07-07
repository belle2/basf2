/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/ckf/vtx/utilities/VTXKalmanStepper.h>

#include <tracking/ckf/vtx/entities/CKFToVTXState.h>
#include <vtx/reconstruction/VTXRecoHit.h>

using namespace Belle2;

double VTXKalmanStepper::kalmanStep(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const CKFToVTXState& state)
{
  double chi2 = 0;
  for (const VTXRecoHit& vtxRecoHit : state.getRecoHits()) {
    const std::vector<genfit::MeasurementOnPlane*>& measurementsOnPlane = vtxRecoHit.constructMeasurementsOnPlane(
          measuredStateOnPlane);

    B2ASSERT("I expect exactly one measurement here", measurementsOnPlane.size() == 1);
    const genfit::MeasurementOnPlane* measurementOnPlane = measurementsOnPlane.front();

    chi2 += m_kalmanStepper.kalmanStep(measuredStateOnPlane, *measurementOnPlane);

    delete measurementOnPlane;
  }
  return chi2;
}

double VTXKalmanStepper::calculateResidual(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const CKFToVTXState& state)
{
  double residual = 0;
  for (const VTXRecoHit& vtxRecoHit : state.getRecoHits()) {
    const std::vector<genfit::MeasurementOnPlane*>& measurementsOnPlane = vtxRecoHit.constructMeasurementsOnPlane(
          measuredStateOnPlane);

    B2ASSERT("I expect exactly one measurement here", measurementsOnPlane.size() == 1);
    const genfit::MeasurementOnPlane* measurementOnPlane = measurementsOnPlane.front();

    residual += m_kalmanStepper.calculateResidual(measuredStateOnPlane, *measurementOnPlane);

    delete measurementOnPlane;
  }
  return residual;
}
