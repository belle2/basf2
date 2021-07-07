/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
