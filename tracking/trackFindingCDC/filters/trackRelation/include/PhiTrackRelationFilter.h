/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>

#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCTrack;

    /// Relation filter that lets only possibilities with small phi distance pass
    class PhiTrackRelationFilter : public BaseTrackRelationFilter {

    private:
      /// Type of the super class
      using Super = BaseTrackRelationFilter;

    public:
      /// Export all parameters
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Implementation of the phi calculation.
      Weight operator()(const CDCTrack& fromTrack, const CDCTrack& toTrack) final;

    private:
      /// Parameter : The maximal deviation in phi between the trajectories of the track.
      double m_param_maximalPhiDistance = 0.2;
    };
  }
}
