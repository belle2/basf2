/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/pxd/filters/states/PXDStateTruthVarSet.h>

#include <framework/core/ModuleParamList.h>
#include <tracking/trackingUtilities/utilities/StringManipulation.h>
#include <framework/dataobjects/EventMetaData.h>

#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <TRandom.h>

using namespace Belle2;
using namespace TrackingUtilities;

bool PXDStateTruthVarSet::extract(const BasePXDStateFilter::Object* pair)
{
  const std::vector<TrackingUtilities::WithWeight<const CKFToPXDState*>>& previousStates = pair->first;
  const CKFToPXDState* state = pair->second;

  std::vector<TrackingUtilities::WithWeight<const CKFToPXDState*>> allStates = previousStates;
  allStates.emplace_back(state, 0);

  const RecoTrack* seedTrack = previousStates.front()->getSeed();
  B2ASSERT("Path without seed?", seedTrack);

  var<named("truth_event_id")>() = m_eventMetaData->getEvent();
  var<named("truth_seed_number")>() = seedTrack->getArrayIndex();

  // Default to 0
  var<named("truth_position_x")>() = 0;
  var<named("truth_position_y")>() = 0;
  var<named("truth_position_z")>() = 0;
  var<named("truth_momentum_x")>() = 0;
  var<named("truth_momentum_y")>() = 0;
  var<named("truth_momentum_z")>() = 0;
  var<named("truth")>() = 0;

  if (not m_mcUtil.allStatesCorrect(allStates)) {
    if (m_UseFractionOfBackground and gRandom->Uniform() > m_BackgroundFraction) {
      return false;
    }
    // Keep all variables set to false and return.
    return true;
  }

  const std::string& seedTrackStoreArrayName = seedTrack->getArrayName();

  TrackMatchLookUp mcCDCMatchLookUp("MCRecoTracks", seedTrackStoreArrayName);
  const RecoTrack* cdcMCTrack = mcCDCMatchLookUp.getRelatedMCRecoTrack(*seedTrack);

  var<named("truth")>() = 1;

  var<named("truth_position_x")>() = cdcMCTrack->getPositionSeed().X();
  var<named("truth_position_y")>() = cdcMCTrack->getPositionSeed().Y();
  var<named("truth_position_z")>() = cdcMCTrack->getPositionSeed().Z();
  var<named("truth_momentum_x")>() = cdcMCTrack->getMomentumSeed().X();
  var<named("truth_momentum_y")>() = cdcMCTrack->getMomentumSeed().Y();
  var<named("truth_momentum_z")>() = cdcMCTrack->getMomentumSeed().Z();

  return true;
}

void PXDStateTruthVarSet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "useFractionOfBackground"), m_UseFractionOfBackground,
                                "Use only a fraction of background hits and combinations in recording to improve the signal fraction in the recording.",
                                m_UseFractionOfBackground);

  moduleParamList->addParameter(TrackingUtilities::prefixed(prefix, "backgroundFraction"), m_BackgroundFraction,
                                "Fraction of background hits and combinations to be used in recording to improve the signal fraction in the recording if " + \
                                TrackingUtilities::prefixed(prefix, "useFractionOfBackground") + " is true.",
                                m_BackgroundFraction);
}
