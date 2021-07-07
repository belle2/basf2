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

    /// Filter adapter type that passes the weight of a weighted relation as a result
    template<class AFilter>
    class PassThroughFilter : public AFilter {
    public:
      /// Type of the object to be filtered
      using Object = typename AFilter::Object;

    public:
      /// Returns the weight obtained from getWeight()
      Weight operator()(const Object& obj) final;
    };
  }
}
