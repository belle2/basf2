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
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/StoreArray.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackExporter::getDescription()
{
  return "Creates reconstruction track candidates from each individual tracks.";
}

/** Add the parameters of the filter to the module */
void TrackExporter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "RecoTracksStoreArrayName"),
                                m_param_exportTracksInto,
                                "Alias for exportTracksInto",
                                std::string(m_param_exportTracksInto));

  moduleParamList->addParameter(prefixed(prefix, "WriteGFTrackCands"),
                                m_param_exportTracks,
                                "Alias for exportTracks",
                                bool(m_param_exportTracks));

  moduleParamList->addParameter(prefixed(prefix, "exportTracks"),
                                m_param_exportTracks,
                                "Switch for the creation of reco tracks for each cdc track.",
                                bool(m_param_exportTracks));

  moduleParamList->addParameter(prefixed(prefix, "exportTracksInto"),
                                m_param_exportTracksInto,
                                "Name of the output StoreArray of RecoTracks.",
                                std::string(m_param_exportTracksInto));
}

/// Signal initialisation phase to register store array for export
void TrackExporter::initialize()
{
  // Output StoreArray
  if (m_param_exportTracks) {
    StoreArray <RecoTrack> storedRecoTracks(m_param_exportTracksInto);
    storedRecoTracks.registerInDataStore();

    StoreArray <RecoHitInformation> storedRecoHitInformation;
    storedRecoHitInformation.registerInDataStore();
    storedRecoTracks.registerRelationTo(storedRecoHitInformation);

    StoreArray <CDCHit> storedCDCHits;
    storedRecoHitInformation.registerRelationTo(storedCDCHits);
    storedCDCHits.registerRelationTo(storedRecoTracks);
  }
  Super::initialize();
}

/// Signal new event recreate the store array
void TrackExporter::beginEvent()
{
  if (m_param_exportTracks) {
    StoreArray <RecoTrack> storedRecoTracks(m_param_exportTracksInto);
    if (not storedRecoTracks.isValid()) {
      storedRecoTracks.create();
    }

    StoreArray <RecoHitInformation> storedRecoHitInformation;
    if (not storedRecoHitInformation.isValid()) {
      storedRecoHitInformation.create();
    }
  }
  Super::beginEvent();
}

/// Write give tracks into track store array
void TrackExporter::apply(std::vector<CDCTrack>& tracks)
{
  // Put code to generate gf track cands here if requested.
  if (m_param_exportTracks) {
    StoreArray <RecoTrack> storedRecoTracks(m_param_exportTracksInto);
    for (CDCTrack& track : tracks) {
      track.storeInto(storedRecoTracks);
    }
  }
}