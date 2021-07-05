/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
