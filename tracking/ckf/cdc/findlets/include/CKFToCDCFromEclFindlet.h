/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Simon Kurz, Nils Braun                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/ckf/general/findlets/ECLTrackCreator.h>
#include <tracking/ckf/cdc/findlets/CDCfromEclCKFSeedCreator.h>
#include <tracking/ckf/cdc/findlets/StackTreeSearcher.h>
#include <tracking/ckf/cdc/findlets/CDCCKFResultFinalizer.h>
#include <tracking/ckf/cdc/findlets/CDCCKFResultStorer.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <vector>

namespace Belle2 {
  class RecoTrack;

  class ModuleParamList;

  /// Main findlet of the ToCDCCKF module
  class CKFToCDCFromEclFindlet : public TrackFindingCDC::Findlet<const TrackFindingCDC::CDCWireHit> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const TrackFindingCDC::CDCWireHit>;

  public:
    /// Constructor, for setting module description and parameters.
    CKFToCDCFromEclFindlet();

    /// Default desctructor
    ~CKFToCDCFromEclFindlet() override;

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the track/hit finding/merging.
    void apply(const std::vector<TrackFindingCDC::CDCWireHit>& wireHits) override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    // Findlets
    /// Findlet for retrieving the ecl showers and creating recoTracks out of it
    ECLTrackCreator m_trackCreator;
    /// Seed Creator
    CDCfromEclCKFSeedCreator m_seedCreator;
    /// Tree Searcher
    StackTreeSearcher m_treeSearcher;
    /// Result Finalizer
    CDCCKFResultFinalizer m_resultFinalizer;
    /// Result Storer
    CDCCKFResultStorer m_resultStorer;

    // Object pools
    /// Pointers to the CDC Reco tracks as a vector
    std::vector<RecoTrack*> m_eclRecoTrackVector;
    /// Current list of paths
    std::vector<CDCCKFPath> m_paths;
    /// Current list of seeds
    std::vector<CDCCKFPath> m_seeds;
    /// Current list of results
    std::vector<CDCCKFResult> m_results;
  };
}
