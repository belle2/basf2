/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
