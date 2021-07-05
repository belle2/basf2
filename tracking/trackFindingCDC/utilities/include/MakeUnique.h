/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
