/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackExporter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackExporter::getDescription()
{
  return "Creates a RecoTrack from each CDCTrack.";
}

void TrackExporter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "RecoTracksStoreArrayName"),
                                m_param_exportTracksInto,
                                "Alias for exportTracksInto",
                                m_param_exportTracksInto);

  moduleParamList->addParameter(prefixed(prefix, "WriteRecoTracks"),
                                m_param_exportTracks,
                                "Alias for exportTracks",
                                m_param_exportTracks);

  moduleParamList->addParameter(prefixed(prefix, "exportTracks"),
                                m_param_exportTracks,
                                "Switch for the creation of reco tracks for each cdc track.",
                                m_param_exportTracks);

  moduleParamList->addParameter(prefixed(prefix, "exportTracksInto"),
                                m_param_exportTracksInto,
                                "Name of the output StoreArray of RecoTracks.",
                                m_param_exportTracksInto);
}

void TrackExporter::initialize()
{
  // Output StoreArray
  if (m_param_exportTracks) {
    StoreArray<RecoTrack> storedRecoTracks(m_param_exportTracksInto);
    storedRecoTracks.registerInDataStore();
    RecoTrack::registerRequiredRelations(storedRecoTracks);
  }
  Super::initialize();
}

void TrackExporter::apply(std::vector<CDCTrack>& tracks)
{
  // Put code to generate gf track cands here if requested.
  if (m_param_exportTracks) {
    StoreArray<RecoTrack> storedRecoTracks(m_param_exportTracksInto);
    for (const CDCTrack& track : tracks) {
      track.storeInto(storedRecoTracks);
    }
  }
}
