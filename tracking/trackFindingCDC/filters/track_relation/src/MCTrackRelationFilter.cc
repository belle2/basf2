/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/track_relation/MCTrackRelationFilter.h>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


void MCTrackRelationFilter::setParameter(const std::string& key, const std::string& value)
{
  if (key == "symmetric") {
    if (value == "true") {
      m_param_allowReverse = true;
      B2INFO("Filter received parameter '" << key << "' " << value);
    } else if (value == "false") {
      m_param_allowReverse = false;
      B2INFO("Filter received parameter '" << key << "' " << value);
    } else {
      Super::setParameter(key, value);
    }
  } else {
    Super::setParameter(key, value);
  }
}

std::map<std::string, std::string> MCTrackRelationFilter::getParameterDescription()
{
  std::map<std::string, std::string> des = Super::getParameterDescription();
  des["symmetric"] =  "Accept the track relation if the reverse track relation is correct "
                      "preserving the progagation reversal symmetry on this level of detail."
                      "Allowed values 'true', 'false'. Default is 'true'.";
  return des;
}

bool MCTrackRelationFilter::needsTruthInformation()
{
  return true;
}


NeighborWeight MCTrackRelationFilter::operator()(const CDCTrack& fromTrack,
                                                 const CDCTrack& toTrack)
{
  const CDCMCTrackLookUp& mcTrackLookUp = CDCMCTrackLookUp::getInstance();

  // Check if the tracks are aligned correctly along the Monte Carlo track
  ForwardBackwardInfo pairFBInfo = mcTrackLookUp.areAlignedInMCTrack(&fromTrack, &toTrack);
  if (pairFBInfo == INVALID_INFO) return NOT_A_CELL;

  if (pairFBInfo == FORWARD or (m_param_allowReverse and pairFBInfo == BACKWARD)) {
    // Final check for the distance between the track
    Index fromNPassedSuperLayers = mcTrackLookUp.getLastNPassedSuperLayers(&fromTrack);
    if (fromNPassedSuperLayers == INVALID_INDEX) return NOT_A_CELL;

    Index toNPassedSuperLayers = mcTrackLookUp.getFirstNPassedSuperLayers(&toTrack);
    if (toNPassedSuperLayers == INVALID_INDEX) return NOT_A_CELL;

    if (fromNPassedSuperLayers == toNPassedSuperLayers) return NOT_A_CELL;

    return fromTrack.size() + toTrack.size();
  }

  return NOT_A_CELL;
}
