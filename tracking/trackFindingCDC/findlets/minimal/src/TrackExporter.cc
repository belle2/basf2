/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackExporter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/utils/RecoTrackUtil.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParamList.templateDetails.h>

#include <TMatrixDSym.h>

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

  moduleParamList->addParameter(prefixed(prefix, "discardCovarianceMatrix"),
                                m_param_discardCovarianceMatrix,
                                "Discard covariance matrix in favour of a hand written one.",
                                m_param_discardCovarianceMatrix);
  moduleParamList->addParameter(prefixed(prefix, "monopoleMomSeed"),
                                m_param_monopoleMomSeed,
                                "If non-zero, estimate seeds as for monopoles and set the momentum magnitude as this value.",
                                m_param_monopoleMomSeed);
}

void TrackExporter::initialize()
{
  // Output StoreArray
  if (m_param_exportTracks) {
    StoreArray<RecoTrack> storedRecoTracks(m_param_exportTracksInto);
    storedRecoTracks.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);
    RecoTrack::registerRequiredRelations(storedRecoTracks);
  }
  Super::initialize();
}

void TrackExporter::apply(std::vector<CDCTrack>& tracks)
{
  TMatrixDSym defaultCovSeed(6);
  defaultCovSeed(0, 0) = 1e-3;
  defaultCovSeed(1, 1) = 1e-3;
  defaultCovSeed(2, 2) = 4e-3;
  defaultCovSeed(3, 3) = 0.01e-3;
  defaultCovSeed(4, 4) = 0.01e-3;
  defaultCovSeed(5, 5) = 0.04e-3;

  // Put code to generate gf track cands here if requested.
  if (m_param_exportTracks) {
    StoreArray<RecoTrack> storedRecoTracks(m_param_exportTracksInto);
    for (const CDCTrack& track : tracks) {
      RecoTrack* newRecoTrack;
      if (m_param_monopoleMomSeed != 0.0)
        newRecoTrack = RecoTrackUtil::storeInto(track, storedRecoTracks, m_param_monopoleMomSeed);
      else
        newRecoTrack = RecoTrackUtil::storeInto(track, storedRecoTracks);
      if (newRecoTrack) {
        newRecoTrack->setQualityIndicator(track.getQualityIndicator());
      }
      if (newRecoTrack and m_param_discardCovarianceMatrix) {
        newRecoTrack->setSeedCovariance(defaultCovSeed);
      }
    }
  }
}
