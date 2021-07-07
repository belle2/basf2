/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/ckf/cdc/findlets/CDCCKFEclSeedCreator.h>
#include <tracking/ckf/cdc/findlets/StackTreeSearcher.h>
#include <tracking/ckf/cdc/findlets/CDCCKFResultFinalizer.h>
#include <tracking/ckf/cdc/findlets/CDCCKFResultStorer.h>
#include <tracking/ckf/cdc/findlets/CDCCKFDuplicateRemover.h>

#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <vector>

namespace Belle2 {
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
    CDCCKFEclSeedCreator m_seedCreator;
    /// Tree Searcher
    StackTreeSearcher m_treeSearcher;
    /// Result Finalizer
    CDCCKFResultFinalizer m_resultFinalizer;
    /// Result Storer
    CDCCKFResultStorer m_resultStorer;
    /// Showers from Bremsstrahlung might lead to duplicate tracks
    CDCCKFDuplicateRemover m_duplicateRemover;

    // Object pools
    /// Current list of paths
    std::vector<CDCCKFPath> m_paths;
    /// Current list of seeds
    std::vector<CDCCKFPath> m_seeds;
    /// Current list of results
    std::vector<CDCCKFResult> m_results;
  };
}
