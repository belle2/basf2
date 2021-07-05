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

#include <tracking/trackFindingCDC/utilities/Range.h>
#include <tracking/trackFindingCDC/utilities/GetIterator.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class Ts, class It = GetIterator<Ts> >
    Range<std::reverse_iterator<It> > reversedRange(Ts& ts)
    {
      return { std::rbegin(ts), std::rend(ts) };
    }
  }
}
