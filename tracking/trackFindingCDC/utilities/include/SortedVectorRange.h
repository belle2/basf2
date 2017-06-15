/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2017 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Nils Braun                                               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/SortedRange.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A pair of iterators into a vector usable with the range base for loop.
    template<class T>
    using SortedVectorRange = SortedRange<typename std::vector<T>::iterator>;

    /// A pair of iterators into a constant vector usable with the range base for loop.
    template<class T>
    using SortedConstVectorRange = SortedRange<typename std::vector<T>::const_iterator>;


  }
}
