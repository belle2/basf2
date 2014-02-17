/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCSEGMENTTRIPLEFILTER_H_
#define MCSEGMENTTRIPLEFILTER_H_


#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>
#include <tracking/cdcLocalTracking/fitting/CDCSZFitter.h>

#include <tracking/cdcLocalTracking/creator_filters/axial_axial/MCAxialAxialSegmentPairFilter.h>
#include <tracking/cdcLocalTracking/mclookup/CDCMCLookUp.h>


namespace Belle2 {
  namespace CDCLocalTracking {
    /// Filter for the constuction of segment triples based on monte carlo information
    class MCSegmentTripleFilter {

    public:


      /** Constructor. */
      MCSegmentTripleFilter();
      MCSegmentTripleFilter(const CDCMCLookUp& mcLookUp);

      /** Destructor.*/
      ~MCSegmentTripleFilter();

    public:

      void clear() {;}

      CellWeight isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& axialAxialSegmentPair);

      CellWeight isGoodSegmentTriple(const CDCSegmentTriple& triple);

    private:
      CellWeight isGoodTriple(const CDCAxialRecoSegment2D& startSegment,
                              const CDCStereoRecoSegment2D& middleSegment,
                              const CDCAxialRecoSegment2D& endSegment);

      void setTrajectoryOf(const CDCSegmentTriple& segmentTriple);


    private:
      const CDCMCLookUp& m_mcLookUp;
      MCAxialAxialSegmentPairFilter m_mcAxialAxialSegmentPairFilter;

    }; // end class MCSegmentTripleFilter


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //MCSEGMENTTRIPLEFILTER_H_
