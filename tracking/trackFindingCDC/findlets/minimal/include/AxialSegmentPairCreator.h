/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/ChooseableAxialSegmentPairFilter.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <vector>
#include <array>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCAxialSegmentPair;
    class CDCSegment2D;

    /// Class providing construction combinatorics for the axial stereo segment pairs.
    class AxialSegmentPairCreator : public Findlet<const CDCSegment2D, CDCAxialSegmentPair> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegment2D, CDCAxialSegmentPair>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      AxialSegmentPairCreator();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main method constructing pairs in adjacent super layers
      void apply(const std::vector<CDCSegment2D>& inputSegments,
                 std::vector<CDCAxialSegmentPair>& axialSegmentPairs) final;

    private:
      /// Creates segment pairs from a combination of start segments and end segments.
      void create(const std::vector<const CDCSegment2D*>& startSegments,
                  const std::vector<const CDCSegment2D*>& endSegments,
                  std::vector<CDCAxialSegmentPair>& axialSegmentPairs);

    private:
      /// Structure for the segments grouped by super layer id.
      std::array<std::vector<const CDCSegment2D*>, ISuperLayerUtil::c_N> m_segmentsBySuperLayer;

      /// The filter to be used for the segment pair generation.
      ChooseableAxialSegmentPairFilter m_axialSegmentPairFilter;
    };
  }
}
