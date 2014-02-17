/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVALUATINGAXIALAXIALSEGMENTPAIRFILTER_H_
#define EVALUATINGAXIALAXIALSEGMENTPAIRFILTER_H_

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include <tracking/cdcLocalTracking/fitting/CDCRiemannFitter.h>
#include <tracking/cdcLocalTracking/fitting/CDCSZFitter.h>

#include "MCAxialAxialSegmentPairFilter.h"

#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    template<class RealAxialAxialSegmentPairFilter>
    class EvaluatingAxialAxialSegmentPairFilter {

    public:

      /// Constructor
      EvaluatingAxialAxialSegmentPairFilter();

      /// Empty destructor
      ~EvaluatingAxialAxialSegmentPairFilter();

    public:

      /// Clears all remember information from the last event
      void clear();

      /// Checks if a pair of axial segments is a good combination
      CellWeight isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& axialAxialSegmentPair);

    private:
      RealAxialAxialSegmentPairFilter& getRealAxialAxialSegmentPairFilter()
      { return m_realAxialAxialSegmentPairFilter; }

      MCAxialAxialSegmentPairFilter& getMCAxialAxialSegmentPairFilter()
      { return m_mcAxialAxialSegmentPairFilter; }

    private:
      RealAxialAxialSegmentPairFilter m_realAxialAxialSegmentPairFilter; //< filter to be compared with Monte Carlo data.
      MCAxialAxialSegmentPairFilter m_mcAxialAxialSegmentPairFilter; //< monte carlo filter

    }; // end class EvaluatingAxialAxialSegmentPairFilter






    template<class RealAxialAxialSegmentPairFilter>
    EvaluatingAxialAxialSegmentPairFilter<RealAxialAxialSegmentPairFilter>::EvaluatingAxialAxialSegmentPairFilter()
    {

    }


    template<class RealAxialAxialSegmentPairFilter>
    EvaluatingAxialAxialSegmentPairFilter<RealAxialAxialSegmentPairFilter>::~EvaluatingAxialAxialSegmentPairFilter()
    {

    }




    template<class RealAxialAxialSegmentPairFilter>
    void EvaluatingAxialAxialSegmentPairFilter<RealAxialAxialSegmentPairFilter>::clear()
    {
      //Nothing to do...
    }



    template<class RealAxialAxialSegmentPairFilter>
    CellWeight EvaluatingAxialAxialSegmentPairFilter<RealAxialAxialSegmentPairFilter>::isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& axialAxialSegmentPair)
    {

      const CDCAxialRecoSegment2D* ptrStartSegment = axialAxialSegmentPair.getStart();
      const CDCAxialRecoSegment2D* ptrEndSegment = axialAxialSegmentPair.getEnd();

      if (ptrStartSegment == nullptr) {
        B2ERROR("OptimizingAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair invoked with nullptr as start segment");
        return NOT_A_CELL;
      }

      if (ptrEndSegment == nullptr) {
        B2ERROR("OptimizingAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair invoked with nullptr as end segment");
        return NOT_A_CELL;
      }

      const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
      const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;


      CellWeight mcWeight = getMCAxialAxialSegmentPairFilter(axialAxialSegmentPair);
      bool mcDecision = not isNotACell(mcWeight);
      // Forget about the Monte Carlo fit
      axialAxialSegmentPair.clearTrajectory2D();

      CellWeight realWeight = getRealAxialAxialSegmentPairFilter(axialAxialSegmentPair);
      bool realDecision = not isNotACell(realWeight);

      return realWeight;

    }


  } //end namespace CDCLocalTracking
} //end namespace Belle2



#endif // EVALUATINGAXIALAXIALSEGMENTPAIRFILTER_H_
