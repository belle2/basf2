/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Oliver Frost                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
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
