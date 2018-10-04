/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <tracking/ckf/cdc/findlets/StackTreeSearcher.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/ckf/general/findlets/TrackLoader.h>

#include <vector>

namespace Belle2 {
  class RecoTrack;
  class ModuleParamList;

  class CKFToCDCFindlet : public TrackFindingCDC::Findlet<const TrackFindingCDC::CDCWireHit> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const TrackFindingCDC::CDCWireHit>;

  public:
    /// Constructor, for setting module description and parameters.
    CKFToCDCFindlet();

    /// Default desctructor
    ~CKFToCDCFindlet() override;

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the track/hit finding/merging.
    void apply(const std::vector<TrackFindingCDC::CDCWireHit>& wireHits) override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    // Findlets
    /// Findlet for retrieving the vxd tracks and writing the result out
    TrackLoader m_trackHandler;
    /// Tree Searcher
    StackTreeSearcher m_treeSearcher;

    // Object pools
    /// Pointers to the CDC Reco tracks as a vector
    std::vector<RecoTrack*> m_vxdRecoTrackVector;
  };
}
