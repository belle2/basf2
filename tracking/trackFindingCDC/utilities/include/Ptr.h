/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/Scalar.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class that behaves like a pointer.
    template<typename T>
    using Ptr = ScalarToClass<T*>;

    /// Helper type function to replace a T* with Ptr<T> when needed.
    template<class T>
    using StarToPtr = ScalarToClass<T>;
  }
}
