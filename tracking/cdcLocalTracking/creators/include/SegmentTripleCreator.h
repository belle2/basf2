/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SEGMENTTRIPLECREATOR_H_
#define SEGMENTTRIPLECREATOR_H_

#include <boost/foreach.hpp>

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include <framework/datastore/StoreArray.h>

#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Class providing construction combinatorics for the segment triples.
    template<class AxialAxialSegmentPairFilter, class SegmentTripleFilter>
    class SegmentTripleCreator {

    public:

      /** Constructor. */
      SegmentTripleCreator() {;}

      /** Destructor.*/
      ~SegmentTripleCreator() {;}

      inline void create(const std::vector<CDCRecoSegment2D>& segments,
                         std::set<CDCSegmentTriple>& segmentTriples) const {

        AxialSegmentsBySuperLayer axialSegmentsBySL;
        StereoSegmentsBySuperLayer stereoSegmentsBySL;

        /// Divide segments by superlayer
        BOOST_FOREACH(const CDCRecoSegment2D & segment, segments) {

          ILayerType iSuperLayer =  segment.getISuperLayer();
          AxialType axialType = segment.getAxialType();

          if (axialType == AXIAL) {

            const CDCAxialRecoSegment2D* axialSegment = &segment;
            axialSegmentsBySL[iSuperLayer].push_back(axialSegment);


          } else if (axialType == STEREO_U or axialType == STEREO_V) {

            const CDCStereoRecoSegment2D* stereoSegment = &segment;
            stereoSegmentsBySL[iSuperLayer].push_back(stereoSegment);

          }
        }

        create(axialSegmentsBySL, stereoSegmentsBySL, segmentTriples);
      }

    private:

      typedef std::vector<const CDCAxialRecoSegment2D* > AxialSegmentsBySuperLayer[CDCWireTopology::N_SUPERLAYERS];
      typedef std::vector<const CDCStereoRecoSegment2D*> StereoSegmentsBySuperLayer[CDCWireTopology::N_SUPERLAYERS];

      inline void create(
        const AxialSegmentsBySuperLayer&   axialSegmentsBySL,
        const StereoSegmentsBySuperLayer& stereoSegmentsBySL,
        std::set<CDCSegmentTriple>& segmentTriples
      ) const {

        //clear the remembered fits
        m_axialAxialSegmentPairFilter.clear();
        m_segmentTripleFilter.clear();

        //Make pairs of closeby axial superlayers
        for (ILayerType iAxialSuperLayer = 0; iAxialSuperLayer < CDCWireTopology::N_SUPERLAYERS ;
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

              const std::vector<const CDCAxialRecoSegment2D*>& endSegments    =
                axialSegmentsBySL[iAxialSuperLayerIn];


              createForAxialSuperLayerPair(startSegments,
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

              createForAxialSuperLayerPair(startSegments, middleSegments, endSegments, segmentTriples);

            }
          }
          //continue;

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
            // via the stereo superlayer outside
            ILayerType iStereoSuperLayerIn = iAxialSuperLayer - 1;

            if (CDCWireTopology::getInstance().isValidISuperLayer(iStereoSuperLayerIn)) {

              const std::vector<const CDCStereoRecoSegment2D*>& middleSegments = stereoSegmentsBySL[iStereoSuperLayerIn];
              const std::vector<const CDCAxialRecoSegment2D*>& endSegments  = axialSegmentsBySL[iAxialSuperLayer];

              createForAxialSuperLayerPair(startSegments, middleSegments, endSegments, segmentTriples);

            }
          }
        }
      }



      inline void createForAxialSuperLayerPair(
        const std::vector<const CDCAxialRecoSegment2D* >& startSegments,
        const std::vector<const CDCStereoRecoSegment2D* >& middleSegments,
        const std::vector<const CDCAxialRecoSegment2D* >& endSegments,
        std::set< CDCSegmentTriple >& segmentTriples
      ) const {

        CDCSegmentTriple segmentTriple;

        for (const CDCAxialRecoSegment2D * startSegment : startSegments) {
          for (const CDCAxialRecoSegment2D * endSegment : endSegments) {

            if (startSegment == endSegment) continue; //Just for safety

            segmentTriple.setStart(startSegment);
            segmentTriple.setMiddle(nullptr);
            segmentTriple.setEnd(endSegment);
            segmentTriple.clearTrajectories();

            if (segmentTriple.getTrajectory2D().isFitted()) {
              B2ERROR("CDCAxialAxialSegmentPair still fitted after clearing.")
              continue;
            }
            if (segmentTriple.getTrajectorySZ().isFitted()) {
              B2ERROR("CDCAxialAxialSegmentPair still fitted after clearing.")
              continue;
            }

            const CDCAxialAxialSegmentPair& axialAxialSegmentPair = segmentTriple;
            if (not axialAxialSegmentPair.checkSegments()) {
              B2ERROR("CDCAxialAxialSegmentPair containing nullptr encountered in SegmentTripleCreator");
              continue;
            }

            CellWeight pairWeight = m_axialAxialSegmentPairFilter.isGoodAxialAxialSegmentPair(axialAxialSegmentPair);
            bool pairIsGood = not isNotACell(pairWeight);

            if (pairIsGood) {

              for (const CDCAxialRecoSegment2D * middleSegment : middleSegments) {

                segmentTriple.setMiddle(middleSegment);
                segmentTriple.getTrajectorySZ().clear();

                if (not segmentTriple.checkSegments()) {
                  B2ERROR("CDCSegmentTriple containing nullptr encountered in SegmentTripleCreator");
                  continue;
                }

                // Ask the filter to asses this triple
                CellWeight cellWeight = m_segmentTripleFilter.isGoodSegmentTriple(segmentTriple);

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

    private:
      mutable AxialAxialSegmentPairFilter m_axialAxialSegmentPairFilter;
      mutable SegmentTripleFilter m_segmentTripleFilter;
    };
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SEGMENTTRIPLECREATOR_H_
