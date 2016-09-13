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

#include <tracking/trackFindingCDC/filters/segment/BasicSegmentVarSet.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <tracking/trackFindingCDC/varsets/PairVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration
    class CDCSegmentPair;

    /// Names of the variables to be generated.
    constexpr
    static char const* const segmentPairBasicVarNames[1] = {
      "axial_first"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment
     */
    class BasicSegmentPairVarNames : public VarNames<CDCSegmentPair> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(segmentPairBasicVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return segmentPairBasicVarNames[iName];
      }

      /// Marking that the basic facet variables should be included.
      typedef PairVarSet<BasicSegmentVarSet> NestedVarSet;

      /// Unpack the object for for the nested variable set
      static const std::pair<const CDCRecoSegment2D*, const CDCRecoSegment2D*>
      getNested(const CDCSegmentPair* segmentPair)
      {
        return std::make_pair(segmentPair->getFromSegment(), segmentPair->getToSegment());
      }
    };

    /** Class that computes floating point variables from a facet relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class BasicSegmentPairVarSet : public VarSet<BasicSegmentPairVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<BasicSegmentPairVarNames>;

    public:
      /// Construct the varset.
      explicit BasicSegmentPairVarSet();

      /// Generate and assign the variables from the segment pair
      virtual bool extract(const CDCSegmentPair* ptrSegmentPair) override;
    };
  }
}
