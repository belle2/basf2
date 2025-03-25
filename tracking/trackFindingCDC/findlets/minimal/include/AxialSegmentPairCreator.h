/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/ChooseableAxialSegmentPairFilter.h>

#include <tracking/trackingUtilities/topology/ISuperLayer.h>

#include <vector>
#include <array>
#include <string>

namespace Belle2 {

  namespace TrackingUtilities {
    class CDCAxialSegmentPair;
    class CDCSegment2D;
  }
  namespace TrackFindingCDC {

    /// Class providing construction combinatorics for the axial stereo segment pairs.
    class AxialSegmentPairCreator : public
      TrackingUtilities::Findlet<const TrackingUtilities::CDCSegment2D, TrackingUtilities::CDCAxialSegmentPair> {

    private:
      /// Type of the base class
      using Super = TrackingUtilities::Findlet<const TrackingUtilities::CDCSegment2D, TrackingUtilities::CDCAxialSegmentPair>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      AxialSegmentPairCreator();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main method constructing pairs in adjacent super layers
      void apply(const std::vector<TrackingUtilities::CDCSegment2D>& inputSegments,
                 std::vector<TrackingUtilities::CDCAxialSegmentPair>& axialSegmentPairs) final;

    private:
      /// Creates segment pairs from a combination of start segments and end segments.
      void create(const std::vector<const TrackingUtilities::CDCSegment2D*>& startSegments,
                  const std::vector<const TrackingUtilities::CDCSegment2D*>& endSegments,
                  std::vector<TrackingUtilities::CDCAxialSegmentPair>& axialSegmentPairs);

    private:
      /// Structure for the segments grouped by super layer id.
      std::array<std::vector<const TrackingUtilities::CDCSegment2D*>, TrackingUtilities::ISuperLayerUtil::c_N> m_segmentsBySuperLayer;

      /// The filter to be used for the segment pair generation.
      ChooseableAxialSegmentPairFilter m_axialSegmentPairFilter;
    };
  }
}
