/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <boost/range/iterator_range.hpp>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>



namespace Belle2 {

  namespace TrackFindingCDC {



    /// Class providing construction combinatorics for the segment triples.
    class SegmentPairCreator {

    private:
      /// Iterator range type usable with range based for of input segments.
      typedef std::vector<const CDCRecoSegment2D* > CDCRecoSegmentRange;

      /// Storage structure type for axial segements by their superlayer id
      typedef CDCRecoSegmentRange SegmentRangesBySuperLayer[CDCWireTopology::N_SUPERLAYERS];


    public:
      bool checkSegmentsSortedBySuperLayer(const std::vector<CDCRecoSegment2D>& segments) const
      {
        if (segments.empty()) return true;

        ISuperLayer lastISuperLayer = ISuperLayerUtil::c_InnerVolume;

        for (const CDCRecoSegment2D& segment : segments) {
          if (lastISuperLayer > segment.getISuperLayer()) {
            B2ERROR("not " << lastISuperLayer << " > " <<  segment.getISuperLayer());
            return false;
          }
          lastISuperLayer = segment.getISuperLayer();
        }
        return true;
      }



      /// Create the segment triples by combining close by segments in the combination axial-stereo-axial based on the filter selection criteria.
      template<class ASegmentPairFilter>
      inline void create(ASegmentPairFilter& segmentPairFilter,
                         const std::vector<CDCRecoSegment2D>& segments,
                         std::vector<CDCSegmentPair>& segmentPairs) const
      {


        SegmentRangesBySuperLayer segmentRangesBySuperLayer;

        for (const CDCRecoSegment2D& segment : segments) {
          const CDCRecoSegment2D* ptrSegment = &segment;
          ISuperLayer iSuperLayer = segment.getISuperLayer();
          segmentRangesBySuperLayer[iSuperLayer].push_back(ptrSegment);
        }

        create(segmentPairFilter, segmentRangesBySuperLayer, segmentPairs);

        std::sort(std::begin(segmentPairs), std::end(segmentPairs));

      }


    private:
      /// Creates the axial stereo segment pairs from the segments, which have been grouped by their superlayer id.
      template<class ASegmentPairFilter>
      inline void create(ASegmentPairFilter& segmentPairFilter,
                         const SegmentRangesBySuperLayer& segmentRangesBySuperLayer,
                         std::vector<CDCSegmentPair>& segmentPairs) const
      {

        //clear the remembered fits
        segmentPairFilter.beginEvent();

        //Make pairs of closeby superlayers
        for (ISuperLayer iSuperLayer = 0; iSuperLayer < CDCWireTopology::N_SUPERLAYERS; ++iSuperLayer) {

          const CDCRecoSegmentRange& startSegments = segmentRangesBySuperLayer[iSuperLayer];

          //make pairs of this superlayer and the superlayer more to the inside
          {
            ILayerType iSuperLayerIn = iSuperLayer - 1;
            if (not ISuperLayerUtil::isInCDC(iSuperLayerIn)) {
              const CDCRecoSegmentRange& endSegments = segmentRangesBySuperLayer[iSuperLayerIn];
              create(segmentPairFilter, startSegments, endSegments, segmentPairs);
            }
          }

          //make pairs of this superlayer and the superlayer more to the outside
          {
            ILayerType iSuperLayerOut = iSuperLayer + 1;
            if (not ISuperLayerUtil::isInCDC(iSuperLayerOut)) {
              const CDCRecoSegmentRange& endSegments = segmentRangesBySuperLayer[iSuperLayerOut];
              create(segmentPairFilter, startSegments, endSegments, segmentPairs);
            }
          }

        } // end for iSuperLayer
      }


      /// Creates segment tiples from a combination of start segment, middle segments and end segments.
      template<class ASegmentPairFilter>
      inline void create(ASegmentPairFilter& segmentPairFilter,
                         const CDCRecoSegmentRange& startSegments,
                         const CDCRecoSegmentRange& endSegments,
                         std::vector<CDCSegmentPair>& segmentPairs) const
      {

        CDCSegmentPair segmentPair;

        for (const CDCRecoSegment2D* ptrStartSegment : startSegments) {
          for (const CDCRecoSegment2D* ptrEndSegment : endSegments) {

            if (ptrStartSegment == ptrEndSegment) continue; //Just for safety
            segmentPair.setSegments(ptrStartSegment, ptrEndSegment);
            segmentPair.clearTrajectory3D();

            if (segmentPair.getTrajectory3D().isFitted()) {
              B2ERROR("CDCSegmentPair still fitted after clearing.");
              continue;
            }

            if (not segmentPair.checkSegments()) {
              B2ERROR("CDCAxialSegmentPair containing nullptr encountered in SegmentPairCreator");
              continue;
            }

            CellWeight pairWeight = segmentPairFilter(segmentPair);
            bool pairIsGood = not isNotACell(pairWeight);
            if (pairIsGood) {
              segmentPair.getAutomatonCell().setCellWeight(pairWeight);
              segmentPairs.push_back(segmentPair);
            }

          }
        }
      }
    };

  } //end namespace TrackFindingCDC

} //end namespace Belle2

