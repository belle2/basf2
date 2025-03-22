/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/states/ExtrapolateAndUpdateCDCStateFilter.h>

#include <tracking/trackingUtilities/numerics/ERightLeft.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackingUtilities/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;

namespace {
  TrackingUtilities::ERightLeft setRLInfo(const genfit::MeasuredStateOnPlane& mSoP, CDCCKFState& state)
  {
    const auto& mom = TrackingUtilities::Vector3D(mSoP.getMom());
    const auto& wire = state.getWireHit()->getWire();

    const auto& trackPosition = TrackingUtilities::Vector3D(mSoP.getPos());
    const auto& hitPosition = wire.getWirePos3DAtZ(trackPosition.z());

    TrackingUtilities::Vector3D trackPosToWire{hitPosition - trackPosition};
    TrackingUtilities::ERightLeft rlInfo = trackPosToWire.xy().isRightOrLeftOf(mom.xy());

    state.setRLinfo(rlInfo);
    return rlInfo;
  }
}

ExtrapolateAndUpdateCDCStateFilter::ExtrapolateAndUpdateCDCStateFilter()
{
  addProcessingSignalListener(&m_extrapolator);
}

void ExtrapolateAndUpdateCDCStateFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_extrapolator.exposeParameters(moduleParamList, prefix);

  moduleParamList->getParameter<std::string>(TrackingUtilities::prefixed(prefix, "direction")).setDefaultValue("forward");
}

TrackingUtilities::Weight ExtrapolateAndUpdateCDCStateFilter::operator()(const BaseCDCStateFilter::Object& pair)
{
  const CDCCKFPath* path = pair.first;
  CDCCKFState& state = *(pair.second);
  const CDCCKFState& lastState = path->back();

  genfit::MeasuredStateOnPlane mSoP = lastState.getTrackState();

  const TrackingUtilities::CDCWireHit* wireHit = state.getWireHit();
  CDCRecoHit recoHit(wireHit->getHit(), nullptr);

  try {
    const auto& plane = recoHit.constructPlane(mSoP);
    if (std::isnan(m_extrapolator.extrapolateToPlane(mSoP, plane))) {
      return NAN;
    }

    const auto& measurements = recoHit.constructMeasurementsOnPlane(mSoP);
    B2ASSERT("Should be exactly two measurements", measurements.size() == 2);

    const auto& rightLeft = setRLInfo(mSoP, state);

    if (rightLeft == TrackingUtilities::ERightLeft::c_Right) {
      state.setChi2(m_updater.kalmanStep(mSoP, *(measurements[1])));
    } else {
      state.setChi2(m_updater.kalmanStep(mSoP, *(measurements[0])));
    }

    delete measurements[0];
    delete measurements[1];

    state.setTrackState(mSoP);
    setRLInfo(mSoP, state);

    return 1. / state.getChi2();
  } catch (const genfit::Exception& e) {
    return NAN;
  }
}
