/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <TRandom.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Random filter resulting in a random weight between 0 and 1. Only to be used for testing purposes.
    template<class AFilter>
    class RandomFilter : public AFilter {
    public:
      /// Never reject an item but give back a random value between 0 and 1.
      Weight operator()(const typename AFilter::Object& object __attribute__((unused))) override
      {
        return gRandom->Rndm();
      }
    };
  }
}
