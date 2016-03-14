/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>

#include <tracking/trackFindingCDC/filters/segment/CDCRecoSegment2DVarSet.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/varsets/PairVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration
    class CDCSegmentPair;

    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const segmentPairVarNames[] = {
      "startFit_startISuperLayer",
      "endFit_startISuperLayer",
      "startFit_nextISuperLayer",
      "endFit_previousISuperLayer",
      "startFit_nextAxialISuperLayer",
      "endFit_previousAxialISuperLayer",

      "startFit_totalPerpS_startSegment",
      "endFit_totalPerpS_startSegment" ,
      "commonFit_totalPerpS_startSegment" ,

      "startFit_totalPerpS_endSegment",
      "endFit_totalPerpS_endSegment",
      "commonFit_totalPerpS_endSegment",

      "startFit_isForwardOrBackwardTo_startSegment",
      "endFit_isForwardOrBackwardTo_startSegment",
      "commonFit_isForwardOrBackwardTo_startSegment",

      "startFit_isForwardOrBackwardTo_endSegment",
      "endFit_isForwardOrBackwardTo_endSegment",
      "commonFit_isForwardOrBackwardTo_endSegment",

      "startFit_perpSGap",
      "endFit_perpSGap",
      "commonFit_perpSGap",

      "startFit_perpSFrontOffset" ,
      "endFit_perpSFrontOffset",
      "commonFit_perpSFrontOffset",
      "startFit_perpSBackOffset",
      "endFit_perpSBackOffset",
      "commonFit_perpSBackOffset",
      "startFit_dist2DToFront_endSegment",
      "endFit_dist2DToBack_startSegment",
      "startFit_absMom2D",
      "endFit_absMom2D",
      "momAngleAtStartBack",
      "momAngleAtEndFront",
      "startFit_chi2" ,
      "endFit_chi2" ,
      "commonFit_chi2" ,
      "commonFit_tanLambda",
      "commonFit_tanLambda_variance" ,
      "szFit_tanLambda",

      "axialFit_curvatureXY" ,
      "axialFit_curvatureXY_variance" ,

      "commonFit_curvatureXY" ,
      "commonFit_curvatureXY_variance"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment
     */
    class CDCSegmentPairVarNames : public VarNames<CDCSegmentPair> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(segmentPairVarNames);

      /// Getter for the name a the given index
      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return segmentPairVarNames[iName];
      }

      /// Marking that the basic facet variables should be included.
      typedef PairVarSet<CDCRecoSegment2DVarSet> NestedVarSet;

      /// Unpack the object for for the nested variable set
      static
      const std::pair<const CDCRecoSegment2D*, const CDCRecoSegment2D*>
      getNested(const CDCSegmentPair* segmentPair)
      {
        return std::make_pair(segmentPair->getStartSegment(), segmentPair->getEndSegment());
      }
    };

    /** Class that computes floating point variables from a facet relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class CDCSegmentPairFitVarSet : public VarSet<CDCSegmentPairVarNames> {

    public:
      /// Construct the varset and take an optional prefix to be prepended to all variable names.
      explicit CDCSegmentPairFitVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the segment pair
      virtual bool extract(const CDCSegmentPair* ptrSegmentPair) IF_NOT_CINT(override final);


    private:
      /// Returns the xy fitter instance that is used by this filter.
      const CDCRiemannFitter& getRiemannFitter() const
      { return m_riemannFitter; }

    private:
      /// Memory of the Riemann fitter for the circle fits.
      CDCRiemannFitter m_riemannFitter;

    };
  }
}


