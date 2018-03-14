/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Malwin Weiler, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

namespace Belle2 {
  class RecoTrack;
  class ModuleParamList;

  /**
   * Selector doing an extrapolation of the two tracks in the relations
   * onto a common radius (e.g. the CDC radius) and calculating the distance
   * on this plane. If it is below the cutValue, the relation will be updated
   * with 1 / distance (calculated on the same plane). If the distance is
   * too large, the relation is deleted from the list.
   */
  class ExtrapolationDetectorTrackCombinationSelector :
    public TrackFindingCDC::Findlet<TrackFindingCDC::WeightedRelation<RecoTrack*, RecoTrack* const>> {
  public:
    /// Shortcut class name for a weighted relation between a collector item and a collection item.
    using WeightedRelationItem = TrackFindingCDC::WeightedRelation<RecoTrack*, RecoTrack* const>;

    /// The parent class
    using Super = TrackFindingCDC::Findlet<WeightedRelationItem>;

    /// Expose the cut value to the module.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Do the extrapolation.
    void apply(std::vector<WeightedRelationItem>& weightedRelations) override;

  private:
    /// The cut value to use.
    TrackFindingCDC::Weight m_param_cutValue = 2.0;

    /**
     * Radius to which the two tracks in each relation should be extrapolated.
     * This can be for example the CDC inner wall radius.
     **/
    double m_param_radius = 16.25;
  };
}
