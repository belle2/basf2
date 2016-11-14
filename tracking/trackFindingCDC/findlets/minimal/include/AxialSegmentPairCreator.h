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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <framework/core/ModuleParamList.h>

#include <vector>
#include <algorithm>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    /// Class providing construction combinatorics for the axial stereo segment pairs.
    template <class AAxialSegmentPairFilter>
    class AxialSegmentPairCreator : public Findlet<const CDCRecoSegment2D, CDCAxialSegmentPair> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCRecoSegment2D, CDCAxialSegmentPair>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      AxialSegmentPairCreator()
      {
        this->addProcessingSignalListener(&m_axialSegmentPairFilter);
      }

      /// Short description of the findlet
      std::string getDescription() override final
      {
        return "Creates axial axial segment pairs from a set of segments filtered by some acceptance criterion";
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final
      {
        m_axialSegmentPairFilter.exposeParameters(moduleParamList, prefix);
      }

      /// Main method constructing pairs in adjacent super layers
      void apply(const std::vector<CDCRecoSegment2D>& inputSegments,
                 std::vector<CDCAxialSegmentPair>& axialSegmentPairs) override final
      {
        // Group the segments by their super layer id
        for (std::vector<const CDCRecoSegment2D*>& segementsInSuperLayer : m_segmentsBySuperLayer) {
          segementsInSuperLayer.clear();
        }

        for (const CDCRecoSegment2D& segment : inputSegments) {
          ISuperLayer iSuperLayer = segment.getISuperLayer();
          const CDCRecoSegment2D* ptrSegment = &segment;
          m_segmentsBySuperLayer[iSuperLayer].push_back(ptrSegment);
        }

        // Make pairs of closeby superlayers
        for (ISuperLayer iSuperLayer = 0; iSuperLayer < ISuperLayerUtil::c_N; ++iSuperLayer) {
          if (not ISuperLayerUtil::isAxial(iSuperLayer)) continue;

          const std::vector<const CDCRecoSegment2D*>& startSegments = m_segmentsBySuperLayer[iSuperLayer];

          // Make pairs of this superlayer and the superlayer more to the inside
          {
            ISuperLayer iStereoSuperLayerIn = ISuperLayerUtil::getNextInwards(iSuperLayer);
            ISuperLayer iAxialSuperLayerIn = ISuperLayerUtil::getNextInwards(iStereoSuperLayerIn);
            if (ISuperLayerUtil::isInCDC(iAxialSuperLayerIn)) {
              const std::vector<const CDCRecoSegment2D*>& endSegments = m_segmentsBySuperLayer[iAxialSuperLayerIn];
              create(startSegments, endSegments, axialSegmentPairs);
            }
          }

          // Make pairs of this superlayer and the superlayer more to the outside
          {
            ISuperLayer iStereoSuperLayerOut = ISuperLayerUtil::getNextOutwards(iSuperLayer);
            ISuperLayer iAxialSuperLayerOut = ISuperLayerUtil::getNextOutwards(iStereoSuperLayerOut);
            if (ISuperLayerUtil::isInCDC(iAxialSuperLayerOut)) {
              const std::vector<const CDCRecoSegment2D*>& endSegments = m_segmentsBySuperLayer[iAxialSuperLayerOut];
              create(startSegments, endSegments, axialSegmentPairs);
            }
          }

        } // end for iSuperLayer

        std::sort(axialSegmentPairs.begin(), axialSegmentPairs.end());
      }

    private:
      /// Creates segment pairs from a combination of start segments and end segments.
      void create(const std::vector<const CDCRecoSegment2D*>& startSegments,
                  const std::vector<const CDCRecoSegment2D*>& endSegments,
                  std::vector<CDCAxialSegmentPair>& axialSegmentPairs)
      {
        CDCAxialSegmentPair axialSegmentPair;
        for (const CDCRecoSegment2D* ptrStartSegment : startSegments) {
          for (const CDCRecoSegment2D* ptrEndSegment : endSegments) {

            if (ptrStartSegment == ptrEndSegment) continue; // Just for safety
            axialSegmentPair.setSegments(ptrStartSegment, ptrEndSegment);
            axialSegmentPair.clearTrajectory2D();

            Weight pairWeight = m_axialSegmentPairFilter(axialSegmentPair);
            if (not isNotACell(pairWeight)) {
              axialSegmentPair.getAutomatonCell().setCellWeight(pairWeight);
              axialSegmentPairs.push_back(axialSegmentPair);
            }

          }
        }
      }

    private:
      /// Structure for the segments grouped by super layer id.
      std::array<std::vector<const CDCRecoSegment2D*>, ISuperLayerUtil::c_N> m_segmentsBySuperLayer;

      /// The filter to be used for the segment pair generation.
      AAxialSegmentPairFilter m_axialSegmentPairFilter;
    };
  }
}
