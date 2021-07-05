/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/filters/segmentTriple/ChooseableSegmentTripleFilter.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <vector>
#include <array>
#include <string>

namespace Belle2 {


  namespace TrackFindingCDC {
    class CDCSegmentTriple;
    class CDCAxialSegmentPair;
    class CDCSegment2D;

    /// Class providing construction combinatorics for the axial stereo segment pairs.
    class SegmentTripleCreator
      : public Findlet<const CDCSegment2D, const CDCAxialSegmentPair, CDCSegmentTriple> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegment2D, const CDCAxialSegmentPair, CDCSegmentTriple>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      SegmentTripleCreator();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main method constructing pairs in adjacent super layers
      void apply(const std::vector<CDCSegment2D>& inputSegments,
                 const std::vector<CDCAxialSegmentPair>& inputAxialSegmentPairs,
                 std::vector<CDCSegmentTriple>& segmentTriples) final;

    private:
      /// Creates segment triples by adding the middle segment to the axial segments and filter out the valuable combinations
      void create(const CDCAxialSegmentPair& axialSegmentPair,
                  const std::vector<const CDCSegment2D*>& middleSegments,
                  std::vector<CDCSegmentTriple>& segmentTriples);

    private:
      /// Structure for the segments grouped by super layer id.
      std::array<std::vector<const CDCSegment2D*>, ISuperLayerUtil::c_N> m_segmentsBySuperLayer;

      /// The filter to be used for the segment triple generation.
      ChooseableSegmentTripleFilter m_segmentTripleFilter;
    };
  }
}
