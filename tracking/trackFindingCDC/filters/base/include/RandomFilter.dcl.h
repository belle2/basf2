/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Random filter resulting in a random weight between 0 and 1.
     *  Only to be used for testing purposes.
     */
    template<class AFilter>
    class RandomFilter : public AFilter {
    public:
      /// Type of the object to be filtered
      using Object = typename AFilter::Object;

    public:
      /// Never reject an item but give back a random value between 0 and 1.
      Weight operator()(const Object& obj) final;
    };
  }
}
