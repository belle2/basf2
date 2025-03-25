/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/filters/segmentTriple/ChooseableSegmentTripleFilter.h>

#include <tracking/trackingUtilities/topology/ISuperLayer.h>

#include <vector>
#include <array>
#include <string>

namespace Belle2 {


  namespace TrackingUtilities {
    class CDCSegmentTriple;
    class CDCAxialSegmentPair;
    class CDCSegment2D;
  }
  namespace TrackFindingCDC {

    /// Class providing construction combinatorics for the axial stereo segment pairs.
    class SegmentTripleCreator
      : public TrackingUtilities::Findlet<const TrackingUtilities::CDCSegment2D, const TrackingUtilities::CDCAxialSegmentPair, TrackingUtilities::CDCSegmentTriple> {

    private:
      /// Type of the base class
      using Super =
        TrackingUtilities::Findlet<const TrackingUtilities::CDCSegment2D, const TrackingUtilities::CDCAxialSegmentPair, TrackingUtilities::CDCSegmentTriple>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      SegmentTripleCreator();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main method constructing pairs in adjacent super layers
      void apply(const std::vector<TrackingUtilities::CDCSegment2D>& inputSegments,
                 const std::vector<TrackingUtilities::CDCAxialSegmentPair>& inputAxialSegmentPairs,
                 std::vector<TrackingUtilities::CDCSegmentTriple>& segmentTriples) final;

    private:
      /// Creates segment triples by adding the middle segment to the axial segments and filter out the valuable combinations
      void create(const TrackingUtilities::CDCAxialSegmentPair& axialSegmentPair,
                  const std::vector<const TrackingUtilities::CDCSegment2D*>& middleSegments,
                  std::vector<TrackingUtilities::CDCSegmentTriple>& segmentTriples);

    private:
      /// Structure for the segments grouped by super layer id.
      std::array<std::vector<const TrackingUtilities::CDCSegment2D*>, TrackingUtilities::ISuperLayerUtil::c_N> m_segmentsBySuperLayer;

      /// The filter to be used for the segment triple generation.
      ChooseableSegmentTripleFilter m_segmentTripleFilter;
    };
  }
}
