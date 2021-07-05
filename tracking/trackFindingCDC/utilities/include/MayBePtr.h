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

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  A type to indicate that a given pointer is explicitly allowed to be nullptr.
     *  Rational: In C++ often pointers are returned and used uncheck, because the general
     *  assumption is that the pointer should not be nullptr.
     *  On the other hand a nullptr is occasionally given or returned to indicate the absence of
     *  a value. This type can be used to state that the latter is the case and that the check
     *  has to be performed before using the pointer.
     *  There is no mechanism though to enforce the check. The construct only clarifies the intend,
     *  but it is up to the user to check first.
     */
    template<class T>
    using MayBePtr = T*;

  }
}
