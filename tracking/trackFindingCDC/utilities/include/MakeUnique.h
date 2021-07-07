/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Transitional constructed to be replaced when the real thing becomes avaible.
     *
     * FIXME: Replace me with std::make_unique
     */
    template<typename T, typename... Args>
    std::unique_ptr<T> makeUnique(Args&& ... args)
    {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

  }
}
