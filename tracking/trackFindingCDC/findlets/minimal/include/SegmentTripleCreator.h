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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Class providing construction combinatorics for the axial stereo segment pairs.
    template<class ASegmentTripleFilter>
    class SegmentTripleCreator
      : public Findlet<const CDCRecoSegment2D, const CDCAxialSegmentPair, CDCSegmentTriple> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCRecoSegment2D, const CDCAxialSegmentPair, CDCSegmentTriple>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      SegmentTripleCreator()
      {
        addProcessingSignalListener(&m_segmentTripleFilter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Creates segment triple from a set of segments  and already combined segment pairs filtered by some acceptance criterion";
      }

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final
      {
        m_segmentTripleFilter.exposeParameters(moduleParamList, prefix);
      }

      /// Main method constructing pairs in adjacent super layers
      virtual void apply(const std::vector<CDCRecoSegment2D>& inputSegments,
                         const std::vector<CDCAxialSegmentPair>& inputAxialSegmentPairs,
                         std::vector<CDCSegmentTriple>& segmentTriples) override
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

        for (const CDCAxialSegmentPair& axialSegmentPair : inputAxialSegmentPairs) {
          const CDCRecoSegment2D* startSegmentPtr = axialSegmentPair.getStartSegment();
          const CDCRecoSegment2D* endSegmentPtr = axialSegmentPair.getEndSegment();

          ISuperLayer startISuperLayer = startSegmentPtr->getISuperLayer();
          ISuperLayer endISuperLayer = endSegmentPtr->getISuperLayer();

          B2ASSERT("Invalid start ISuperLayer", ISuperLayerUtil::isAxial(startISuperLayer));
          B2ASSERT("Invalid end ISuperLayer", ISuperLayerUtil::isAxial(endISuperLayer));

          if (startISuperLayer != endISuperLayer) {
            ISuperLayer middleISuperLayer = (startISuperLayer + endISuperLayer) / 2;

            B2ASSERT("Middle ISuperLayer is not stereo",
                     not ISuperLayerUtil::isAxial(middleISuperLayer));

            const std::vector<const CDCRecoSegment2D*>& middleSegmentPtrs
              = m_segmentsBySuperLayer[middleISuperLayer];
            create(axialSegmentPair, middleSegmentPtrs, segmentTriples);
          } else {
            // Case where start and end super layer are the same
            // Look for stereo segments in the adjacent super layer
            ISuperLayer middleISuperLayerIn = ISuperLayerUtil::getNextInwards(startISuperLayer);
            ISuperLayer middleISuperLayerOut = ISuperLayerUtil::getNextOutwards(startISuperLayer);
            for (ISuperLayer middleISuperLayer : {middleISuperLayerIn, middleISuperLayerOut}) {
              if (ISuperLayerUtil::isInCDC(middleISuperLayer)) {
                const std::vector<const CDCRecoSegment2D*>& middleSegmentPtrs
                  = m_segmentsBySuperLayer[middleISuperLayer];
                create(axialSegmentPair, middleSegmentPtrs, segmentTriples);
              }
            }
          }
        }
        std::sort(std::begin(segmentTriples), std::end(segmentTriples));
      }

    private:
      /// Creates segment pairs from a combination of start segments and end segments.
      void create(const CDCAxialSegmentPair& axialSegmentPair,
                  const std::vector<const CDCRecoSegment2D*>& middleSegmentPtrs,
                  std::vector<CDCSegmentTriple>& segmentTriples)
      {
        CDCSegmentTriple segmentTriple(axialSegmentPair);
        for (const CDCRecoSegment2D* middleSegmentPtr : middleSegmentPtrs) {
          segmentTriple.setMiddleSegment(middleSegmentPtr);
          segmentTriple.clearTrajectory3D();

          if (not segmentTriple.checkSegments()) {
            B2ERROR("CDCSegmentTriple containing nullptr encountered in SegmentTripleCreator");
            continue;
          }

          // Ask the filter to assess this triple
          CellWeight cellWeight = m_segmentTripleFilter(segmentTriple);

          if (not isNotACell(cellWeight)) {
            segmentTriple.getAutomatonCell().setCellWeight(cellWeight);
            segmentTriples.insert(segmentTriples.end(), segmentTriple);
          }
        }
      }

    private:
      /// Structure for the segments grouped by super layer id.
      std::array<std::vector<const CDCRecoSegment2D*>, ISuperLayerUtil::c_N> m_segmentsBySuperLayer;

      /// The filter to be used for the segment pair generation.
      ASegmentTripleFilter m_segmentTripleFilter;

    };
  }
}
