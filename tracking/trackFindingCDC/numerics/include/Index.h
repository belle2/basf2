/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <climits>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// A positive index which may indicate an invalid by returning a negative number
    using Index = int;

    /// An invalid index that can be return if no proper solution exists.
    static const Index c_InvalidIndex = INT_MIN;

    /// Check if an index is valid, hence whether it is bigger than zero.
    inline bool isValidIndex(Index index)
    { return index >= 0; }

  }

}
