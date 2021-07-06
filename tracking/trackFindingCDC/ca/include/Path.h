/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type for a path through a neighborhood
    template<class T>
    using Path = std::vector<T*>;

  }
}
