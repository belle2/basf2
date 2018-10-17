/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/ExtrapolateAndUpdateCDCStateFilter.h>

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;

ExtrapolateAndUpdateCDCStateFilter::ExtrapolateAndUpdateCDCStateFilter()
{
  addProcessingSignalListener(&m_extrapolator);
}

void ExtrapolateAndUpdateCDCStateFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_extrapolator.exposeParameters(moduleParamList, prefix);

  moduleParamList->getParameter<std::string>(TrackFindingCDC::prefixed(prefix, "direction")).setDefaultValue("forward");
}

TrackFindingCDC::Weight ExtrapolateAndUpdateCDCStateFilter::operator()(const BaseCDCStateFilter::Object& pair)
{
  const CDCCKFPath* path = pair.first;
  CDCCKFState& state = *(pair.second);
  const CDCCKFState& lastState = path->back();

  genfit::MeasuredStateOnPlane mSoP = lastState.getTrackState();

  const TrackFindingCDC::CDCWireHit* wireHit = state.getWireHit();
  CDCRecoHit recoHit(wireHit->getHit(), nullptr);

  try {
    const auto& plane = recoHit.constructPlane(mSoP);
    if (std::isnan(m_extrapolator.extrapolateToPlane(mSoP, plane))) {
      return NAN;
    }

    const auto& measurements = recoHit.constructMeasurementsOnPlane(mSoP);
    B2ASSERT("Should be exactly two measurements", measurements.size() == 2);

    const auto rightLeft = static_cast<TrackFindingCDC::ERightLeft>(TrackFindingCDC::sign(
                             state.getHitDistance()));

    state.setRLinfo(rightLeft);
    //double residual = 0;

    if (rightLeft == TrackFindingCDC::ERightLeft::c_Right) {
      state.setChi2(m_updater.kalmanStep(mSoP, *(measurements[1])));
      //      residual = m_updater.calculateResidual(mSoP, *(measurements[1])) ;
    } else {
      state.setChi2(m_updater.kalmanStep(mSoP, *(measurements[0])));
      // residual = m_updater.calculateResidual(mSoP, *(measurements[0])) ;
    }

    //std::cout << " Residual = " << residual <<"\n";

    delete measurements[0];
    delete measurements[1];

    state.setTrackState(mSoP);

    return 1. / state.getChi2();
  } catch (genfit::Exception) {
    return NAN;
  }
}
