/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/filters/segment/BaseSegmentFilter.h>
#include <tracking/trackFindingCDC/filters/segment/SegmentFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.dcl.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCSegment2D;

    // Guard to prevent repeated instantiations
    extern template class TrackFindingCDC::Chooseable<BaseSegmentFilter>;
    extern template class TrackFindingCDC::ChooseableFilter<SegmentFilterFactory>;

    /// Deletes fake segments that have been rejected by a filter
    class SegmentRejecter : public Findlet<CDCSegment2D&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCSegment2D&>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      explicit SegmentRejecter(const std::string& defaultFilterName = "all");

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main algorithm
      void apply(std::vector<CDCSegment2D>& segment2Ds) final;

    private:
      /// Parameter : Switch to delete the segments instead of marking them as background.
      bool m_param_deleteRejected = false;

      /// Reference to the filter to be used to filter
      ChooseableFilter<SegmentFilterFactory> m_segmentFilter;
    };
  }
}
