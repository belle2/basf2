/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <memory>

namespace Belle2 {
  /* Todo: Should be replaced as soon it is available in std library
  * note: this implementation does not disable this overload for array types
  */
  template<typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&& ... args)
  {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }
}
