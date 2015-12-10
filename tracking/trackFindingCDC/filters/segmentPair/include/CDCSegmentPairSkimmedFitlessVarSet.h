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

#include <tracking/trackFindingCDC/varsets/SkimmedVarSet.h>
#include <tracking/trackFindingCDC/filters/segmentPair/CDCSegmentPairFitlessVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  Class that computes floating point variables from segment pairs.
     *  Is the same as the CDCSegmentPairFitlessVarSet but skimmed for non sensical combinations
     *  without the expensive common fit.
     */
    class  CDCSegmentPairSkimmedFitlessVarSet : public SkimmedVarSet<CDCSegmentPairFitlessVarSet> {

    private:
      /// Name of the super class.
      typedef SkimmedVarSet<CDCSegmentPairFitlessVarSet> Super;

    public:
      /// Construct the varset and take an optional prefix.
      explicit CDCSegmentPairSkimmedFitlessVarSet(const std::string& prefix = "");

      /// Implement the skim cut.
      virtual bool accept(const CDCSegmentPair* ptrSegmentPair) override final;
    };
  }
}
