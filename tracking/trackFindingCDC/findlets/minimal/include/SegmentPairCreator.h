/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/findlets/minimal/AxialSegmentPairCreator.h>

#include <tracking/trackFindingCDC/filters/segmentPair/ChooseableSegmentPairFilter.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <vector>
#include <array>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCSegmentPair;

    /// Class providing construction combinatorics for the axial stereo segment pairs.
    class SegmentPairCreator : public Findlet<const CDCSegment2D, CDCSegmentPair> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegment2D, CDCSegmentPair>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      SegmentPairCreator();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main method constructing pairs in adjacent super layers
      void apply(const std::vector<CDCSegment2D>& inputSegments,
                 std::vector<CDCSegmentPair>& segmentPairs) final;

    private:
      /// Creates segment pairs from a combination of from segments and to segments.
      void create(const std::vector<const CDCSegment2D*>& fromSegments,
                  const std::vector<const CDCSegment2D*>& toSegments,
                  std::vector<CDCSegmentPair>& segmentPairs);

    private:
      /// Parameter : Switch to enable the search for axial to axial pairs to enable more stable reconstruction of the middle stereo.
      bool m_param_axialBridging = false;

    private:
      /// Findlet responsible for the creation of axial axial segment pairs
      AxialSegmentPairCreator m_axialSegmentPairCreator;

      /// The filter to be used for the segment pair generation.
      ChooseableSegmentPairFilter m_segmentPairFilter;

      // Object pools
      /// Structure for the segments grouped by super layer id.
      std::array<std::vector<const CDCSegment2D*>, ISuperLayerUtil::c_N> m_segmentsBySuperLayer;
    };
  }
}
