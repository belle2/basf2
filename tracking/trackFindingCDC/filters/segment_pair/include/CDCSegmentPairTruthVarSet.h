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

#include "CDCSegmentPairVarSet.h"
#include "MCSegmentPairFilter.h"

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const segmentPairTruthNames[] = {
      "truth"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment pair.
     */
    class CDCSegmentPairTruthVarNames : public VarNames<CDCSegmentPair> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(segmentPairTruthNames);

      /// Getter for the name a the given index
      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return segmentPairTruthNames[iName];
      }

      /// Marking that the basic segmentPair variables should be included.
      typedef CDCSegmentPairVarSet NestedVarSet;
    };

    /** Class that computes floating point variables from segment pair.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class  CDCSegmentPairTruthVarSet : public VarSet<CDCSegmentPairTruthVarNames> {

    public:
      /// Construct the varset and take an optional prefix.
      CDCSegmentPairTruthVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the segmentPair
      virtual bool extract(const CDCSegmentPair* ptrSegmentPair) IF_NOT_CINT(override final);

      /// Initialize the varset before event processing
      virtual void initialize() IF_NOT_CINT(override final);

      /// Initialize the varset before event processing
      virtual void terminate() IF_NOT_CINT(override final);

    public:
      /// SegmentPair filter that gives if the segment pair is a true segmentPair.
      MCSegmentPairFilter m_mcSegmentPairFilter;

    };
  }
}
