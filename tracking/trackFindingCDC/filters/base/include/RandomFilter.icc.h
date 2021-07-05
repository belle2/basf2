/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/RandomFilter.dcl.h>

#include <TRandom.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class AFilter>
    Weight RandomFilter<AFilter>::operator()(const Object& obj __attribute__((unused)))
    {
      return gRandom->Rndm();
    }
  }
}
