/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/Range.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A pair of iterators into a vector usable with the range base for loop.
    template<class T>
    using VectorRange = Range<typename std::vector<T>::iterator>;

    /// A pair of iterators into a constant vector usable with the range base for loop.
    template<class T>
    using ConstVectorRange = Range<typename std::vector<T>::const_iterator>;


  }
}
