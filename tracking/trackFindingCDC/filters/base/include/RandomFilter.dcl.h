/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
