/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef AXIALSTEREOSEGMENTPAIRCREATOR_H
#define AXIALSTEREOSEGMENTPAIRCREATOR_H

#include <boost/range/iterator_range.hpp>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialStereoSegmentPair.h>



namespace Belle2 {

  namespace TrackFindingCDC {



    /// Class providing construction combinatorics for the segment triples.
    class AxialStereoSegmentPairCreator {

    private:
      /// Iterator range type usable with range based for of input segments.
      typedef std::vector<const CDCRecoSegment2D* > CDCRecoSegmentRange;

      /// Storage structure type for axial segements by their superlayer id
      typedef CDCRecoSegmentRange SegmentRangesBySuperLayer[CDCWireTopology::N_SUPERLAYERS];


    public:
      /// Empty constructor.
      AxialStereoSegmentPairCreator() {;}

      /// Empty destructor.
      ~AxialStereoSegmentPairCreator() {;}

      bool checkSegmentsSortedBySuperLayer(const std::vector<CDCRecoSegment2D>& segments) const
      {
        if (segments.empty()) return true;

        ISuperLayerType lastISuperLayer = INNER_ISUPERLAYER;

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
      template<class AxialStereoSegmentPairFilter>
      inline void create(AxialStereoSegmentPairFilter& axialStereoSegmentPairFilter,
                         const std::vector<CDCRecoSegment2D>& segments,
                         std::vector<CDCAxialStereoSegmentPair>& axialStereoSegmentPairs) const
      {


        SegmentRangesBySuperLayer segmentRangesBySuperLayer;

        for (const CDCRecoSegment2D& segment : segments) {
          const CDCRecoSegment2D* ptrSegment = &segment;
          ISuperLayerType iSuperLayer = segment.getISuperLayer();
          segmentRangesBySuperLayer[iSuperLayer].push_back(ptrSegment);
        }

        create(axialStereoSegmentPairFilter, segmentRangesBySuperLayer, axialStereoSegmentPairs);

        std::sort(std::begin(axialStereoSegmentPairs), std::end(axialStereoSegmentPairs));

      }


    private:
      /// Creates the axial stereo segment pairs from the segments, which have been grouped by their superlayer id.
      template<class AxialStereoSegmentPairFilter>
      inline void create(AxialStereoSegmentPairFilter& axialStereoSegmentPairFilter,
                         const SegmentRangesBySuperLayer& segmentRangesBySuperLayer,
                         std::vector<CDCAxialStereoSegmentPair>& axialStereoSegmentPairs) const
      {

        //clear the remembered fits
        axialStereoSegmentPairFilter.clear();

        //Make pairs of closeby superlayers
        for (ISuperLayerType iSuperLayer = 0; iSuperLayer < CDCWireTopology::N_SUPERLAYERS; ++iSuperLayer) {

          const CDCRecoSegmentRange& startSegments = segmentRangesBySuperLayer[iSuperLayer];

          //make pairs of this superlayer and the superlayer more to the inside
          {
            ILayerType iSuperLayerIn = iSuperLayer - 1;
            if (isValidISuperLayer(iSuperLayerIn)) {
              const CDCRecoSegmentRange& endSegments = segmentRangesBySuperLayer[iSuperLayerIn];
              create(axialStereoSegmentPairFilter, startSegments, endSegments, axialStereoSegmentPairs);
            }
          }

          //make pairs of this superlayer and the superlayer more to the outside
          {
            ILayerType iSuperLayerOut = iSuperLayer + 1;
            if (isValidISuperLayer(iSuperLayerOut)) {
              const CDCRecoSegmentRange& endSegments = segmentRangesBySuperLayer[iSuperLayerOut];
              create(axialStereoSegmentPairFilter, startSegments, endSegments, axialStereoSegmentPairs);
            }
          }

        } // end for iSuperLayer
      }


      /// Creates segment tiples from a combination of start segment, middle segments and end segments.
      template<class AxialStereoSegmentPairFilter>
      inline void create(AxialStereoSegmentPairFilter& axialStereoSegmentPairFilter,
                         const CDCRecoSegmentRange& startSegments,
                         const CDCRecoSegmentRange& endSegments,
                         std::vector<CDCAxialStereoSegmentPair>& axialStereoSegmentPairs) const
      {

        CDCAxialStereoSegmentPair axialStereoSegmentPair;

        for (const CDCRecoSegment2D* ptrStartSegment : startSegments) {
          for (const CDCRecoSegment2D* ptrEndSegment : endSegments) {

            if (ptrStartSegment == ptrEndSegment) continue; //Just for safety
            axialStereoSegmentPair.setSegments(ptrStartSegment, ptrEndSegment);
            axialStereoSegmentPair.clearTrajectory3D();

            if (axialStereoSegmentPair.getTrajectory3D().isFitted()) {
              B2ERROR("CDCAxialStereoSegmentPair still fitted after clearing.")
              continue;
            }

            if (not axialStereoSegmentPair.checkSegments()) {
              B2ERROR("CDCAxialAxialSegmentPair containing nullptr encountered in AxialStereoSegmentPairCreator");
              continue;
            }

            CellWeight pairWeight = axialStereoSegmentPairFilter(axialStereoSegmentPair);
            bool pairIsGood = not isNotACell(pairWeight);
            if (pairIsGood) {
              axialStereoSegmentPair.getAutomatonCell().setCellWeight(pairWeight);
              axialStereoSegmentPairs.push_back(axialStereoSegmentPair);
            }

          }
        }
      }
    };

  } //end namespace TrackFindingCDC

} //end namespace Belle2

#endif // AXIALSTEREOSEGMENTPAIRCREATOR_H
