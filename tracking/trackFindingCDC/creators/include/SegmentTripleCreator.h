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


#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Class providing construction combinatorics for the segment triples.
    class SegmentTripleCreator {

    public:

      /// Empty constructor.
      SegmentTripleCreator() {;}

      /// Empty destructor.
      ~SegmentTripleCreator() {;}

      /// Create the segment triples by combining close by segments in the combination axial-stereo-axial based on the filter selection criteria.
      template<class AxialSegmentPairFilter, class SegmentTripleFilter>
      inline void create(AxialSegmentPairFilter& axialSegmentPairFilter,
                         SegmentTripleFilter& segmentTripleFilter,
                         const std::vector<CDCRecoSegment2D>& segments,
                         std::set<CDCSegmentTriple>& segmentTriples) const
      {

        AxialSegmentsBySuperLayer axialSegmentsBySL;
        StereoSegmentsBySuperLayer stereoSegmentsBySL;

        /// Divide segments by superlayer
        for (const CDCRecoSegment2D& segment : segments) {

          ILayerType iSuperLayer =  segment.getISuperLayer();
          StereoType stereoType = segment.getStereoType();

          if (stereoType == StereoType_c::Axial) {

            const CDCAxialRecoSegment2D* axialSegment = &segment;
            axialSegmentsBySL[iSuperLayer].push_back(axialSegment);


          } else if (stereoType == StereoType_c::StereoU or stereoType == StereoType_c::StereoV) {

            const CDCStereoRecoSegment2D* stereoSegment = &segment;
            stereoSegmentsBySL[iSuperLayer].push_back(stereoSegment);

            // Also store the stereo layers in the neighboring axial layer index
            // Hence we can ask for all stereo segments, which are next to a specific axial layer
            // by taking the stereo layers for the array at the index of the axial layer !
            int iNeighboringAxialLayerInwards = iSuperLayer - 1;
            int iNeighboringAxialLayerOutwards = iSuperLayer + 1;

            stereoSegmentsBySL[iNeighboringAxialLayerInwards].push_back(stereoSegment);
            stereoSegmentsBySL[iNeighboringAxialLayerOutwards].push_back(stereoSegment);

          }
        }

        create(axialSegmentPairFilter,
               segmentTripleFilter,
               axialSegmentsBySL,
               stereoSegmentsBySL,
               segmentTriples);
      }

    private:
      /// Storage structure type for axial segements by their superlayer id
      typedef std::vector<const CDCAxialRecoSegment2D* > AxialSegmentsBySuperLayer[CDCWireTopology::N_SUPERLAYERS];

      /// Storage structure type for stereo segements by their superlayer id
      typedef std::vector<const CDCStereoRecoSegment2D*> StereoSegmentsBySuperLayer[CDCWireTopology::N_SUPERLAYERS];

      /// Creates the segment triples from the segments, which have been grouped by their superlayer id.
      template<class AxialSegmentPairFilter, class SegmentTripleFilter>
      inline void create(AxialSegmentPairFilter& axialSegmentPairFilter,
                         SegmentTripleFilter& segmentTripleFilter,
                         const AxialSegmentsBySuperLayer& axialSegmentsBySL,
                         const StereoSegmentsBySuperLayer& stereoSegmentsBySL,
                         std::set<CDCSegmentTriple>& segmentTriples) const
      {

        //clear the remembered fits
        axialSegmentPairFilter.clear();
        segmentTripleFilter.clear();

        //Make pairs of closeby axial superlayers
        for (ILayerType iAxialSuperLayer = 0; iAxialSuperLayer < CDCWireTopology::N_SUPERLAYERS;
             ++(++iAxialSuperLayer) /*only even slots are filled */) {

          //consider only axial superlayers
          const std::vector<const CDCAxialRecoSegment2D* >& startSegments = axialSegmentsBySL[iAxialSuperLayer];

          //make pairs of this superlayer and the superlayer more to the inside
          {
            ILayerType iStereoSuperLayerIn = iAxialSuperLayer - 1;
            ILayerType iAxialSuperLayerIn = iAxialSuperLayer - 2;

            if (CDCWireTopology::getInstance().isValidISuperLayer(iAxialSuperLayerIn)) {

              const std::vector<const CDCStereoRecoSegment2D*>& middleSegments =
                stereoSegmentsBySL[iStereoSuperLayerIn];

              const std::vector<const CDCAxialRecoSegment2D*>& endSegments =
                axialSegmentsBySL[iAxialSuperLayerIn];


              createForAxialSuperLayerPair(axialSegmentPairFilter,
                                           segmentTripleFilter,
                                           startSegments,
                                           middleSegments,
                                           endSegments,
                                           segmentTriples);
            }
          }

          //make pairs of this superlayer and the superlayer more to the outside
          {
            ILayerType iStereoSuperLayerOut = iAxialSuperLayer + 1;
            ILayerType iAxialSuperLayerOut = iAxialSuperLayer + 2;
            if (CDCWireTopology::getInstance().isValidISuperLayer(iAxialSuperLayerOut)) {

              const std::vector<const CDCStereoRecoSegment2D*>& middleSegments = stereoSegmentsBySL[iStereoSuperLayerOut];
              const std::vector<const CDCAxialRecoSegment2D*>& endSegments  = axialSegmentsBySL[iAxialSuperLayerOut];

              createForAxialSuperLayerPair(axialSegmentPairFilter,
                                           segmentTripleFilter,
                                           startSegments,
                                           middleSegments,
                                           endSegments,
                                           segmentTriples);

            }
          }

          //make pairs from and to the same superlayer
          {
            // Note: By asking the stereo layer for an axial indexwe get all neighboring stereo segments to this axial superlayer
            const std::vector<const CDCStereoRecoSegment2D*>& middleSegments = stereoSegmentsBySL[iAxialSuperLayer];
            const std::vector<const CDCAxialRecoSegment2D*>& endSegments  = axialSegmentsBySL[iAxialSuperLayer];

            createForAxialSuperLayerPair(axialSegmentPairFilter,
                                         segmentTripleFilter,
                                         startSegments,
                                         middleSegments,
                                         endSegments,
                                         segmentTriples);

          }

          continue;

          /*
                //make pairs from and to the same superlayer
                {
                  // via the stereo superlayer outside
                  ILayerType iStereoSuperLayerOut = iAxialSuperLayer + 1;

                  if (CDCWireTopology::getInstance().isValidISuperLayer(iStereoSuperLayerOut)) {

                    const std::vector<const CDCStereoRecoSegment2D*>& middleSegments = stereoSegmentsBySL[iStereoSuperLayerOut];
                    const std::vector<const CDCAxialRecoSegment2D*>& endSegments  = axialSegmentsBySL[iAxialSuperLayer];

                    createForAxialSuperLayerPair(startSegments, middleSegments, endSegments, segmentTriples);

                  }
                }

                {
                  // via the stereo superlayer inside
                  ILayerType iStereoSuperLayerIn = iAxialSuperLayer - 1;

                  if (CDCWireTopology::getInstance().isValidISuperLayer(iStereoSuperLayerIn)) {

                    const std::vector<const CDCStereoRecoSegment2D*>& middleSegments = stereoSegmentsBySL[iStereoSuperLayerIn];
                    const std::vector<const CDCAxialRecoSegment2D*>& endSegments  = axialSegmentsBySL[iAxialSuperLayer];

                    createForAxialSuperLayerPair(startSegments, middleSegments, endSegments, segmentTriples);

                  }
                }

          */
        } // end for iAxialSuperLayer
      }


      /// Creates segment tiples from a combination of start segment, middle segments and end segments.
      template<class AxialSegmentPairFilter, class SegmentTripleFilter>
      inline void createForAxialSuperLayerPair(AxialSegmentPairFilter& axialSegmentPairFilter,
                                               SegmentTripleFilter& segmentTripleFilter,
                                               const std::vector<const CDCAxialRecoSegment2D* >& startSegments,
                                               const std::vector<const CDCStereoRecoSegment2D* >& middleSegments,
                                               const std::vector<const CDCAxialRecoSegment2D* >& endSegments,
                                               std::set< CDCSegmentTriple >& segmentTriples) const
      {

        CDCSegmentTriple segmentTriple;

        for (const CDCAxialRecoSegment2D* startSegment : startSegments) {
          for (const CDCAxialRecoSegment2D* endSegment : endSegments) {

            if (startSegment == endSegment) continue; //Just for safety

            segmentTriple.setStart(startSegment);
            segmentTriple.setMiddle(nullptr);
            segmentTriple.setEnd(endSegment);
            segmentTriple.clearTrajectories();

            if (segmentTriple.getTrajectory2D().isFitted()) {
              B2ERROR("Two dimensional trajectory of CDCAxialSegmentPair still fitted after clearing.");
              continue;
            }
            if (segmentTriple.getTrajectorySZ().isFitted()) {
              B2ERROR("SZ trajectory of CDCAxialSegmentPair still fitted after clearing.");
              continue;
            }

            const CDCAxialSegmentPair& axialSegmentPair = segmentTriple;
            if (not axialSegmentPair.checkSegments()) {
              B2ERROR("CDCAxialSegmentPair containing nullptr encountered in SegmentTripleCreator");
              continue;
            }

            CellWeight pairWeight = axialSegmentPairFilter(axialSegmentPair);
            bool pairIsGood = not isNotACell(pairWeight);

            if (pairIsGood) {

              for (const CDCAxialRecoSegment2D* middleSegment : middleSegments) {

                segmentTriple.setMiddle(middleSegment);
                segmentTriple.getTrajectorySZ().clear();

                if (not segmentTriple.checkSegments()) {
                  B2ERROR("CDCSegmentTriple containing nullptr encountered in SegmentTripleCreator");
                  continue;
                }

                // Ask the filter to assess this triple
                CellWeight cellWeight = segmentTripleFilter(segmentTriple);

                if (not isNotACell(cellWeight)) {
                  segmentTriple.getAutomatonCell().setCellWeight(cellWeight);
                  segmentTriples.insert(segmentTriples.end(), segmentTriple);
                  // insert a copy in the set
                  // const CDCSegmentTriple& newTriple = *(segmentTriples.insert(triple).first);

                }
              }

            } else {
              //B2DEBUG(100,"    Check makesGoodPair false");
            }
          }
        }
      }
    };
  } //end namespace TrackFindingCDC
} //end namespace Belle2

