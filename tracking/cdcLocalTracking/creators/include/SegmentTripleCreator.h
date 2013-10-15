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
    template<class Filter>
    class SegmentTripleCreator {

    public:

      /** Constructor. */
      SegmentTripleCreator() {;}
      SegmentTripleCreator(const Filter& filter)  : m_filter(filter) {;}

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
        m_filter.clear();

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

        BOOST_FOREACH(const CDCAxialRecoSegment2D * startSegment, startSegments) {
          BOOST_FOREACH(const CDCAxialRecoSegment2D * endSegment, endSegments) {

            if (startSegment != endSegment and m_filter.isGoodPair(*startSegment, *endSegment)) {

              createForSegmentPair(startSegment, middleSegments, endSegment, segmentTriples);

            } else {
              //B2DEBUG(100,"    Check makesGoodPair false");
            }
          }
        }
      }

      inline void createForSegmentPair(const CDCAxialRecoSegment2D* startSegment,
                                       const std::vector<const CDCStereoRecoSegment2D* >& middleSegments,
                                       const CDCAxialRecoSegment2D* endSegment,
                                       std::set<CDCSegmentTriple>& segmentTriples) const {

        BOOST_FOREACH(const CDCAxialRecoSegment2D * middleSegment, middleSegments) {

          // Create a segment to work on. The middle segment is not yet assigned and is initialized to nullptr
          CDCSegmentTriple triple(startSegment, middleSegment, endSegment);

          // Ask the filter to asses this triple
          CellWeight cellWeight = m_filter.isGoodTriple(triple);

          if (not isNotACell(cellWeight)) {

            // insert a copy in the set
            const CDCSegmentTriple& newTriple = *(segmentTriples.insert(triple).first);

            newTriple.setCellWeight(cellWeight);

          }
        }
      }

    private:
      mutable Filter m_filter;

    };
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //SEGMENTTRIPLECREATOR_H_
