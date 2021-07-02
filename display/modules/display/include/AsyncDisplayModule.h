/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/pcore/AsyncWrapper.h>

namespace Belle2 {
  /** Wraps DisplayModule and starts it in a forked process. */
  class AsyncDisplayModule : public AsyncWrapper {
  public:
    /** constructor. */
    AsyncDisplayModule(): AsyncWrapper("Display")
    {
      setDescription("Starts the Display module asynchronously (with buffered input). Note that the buffer is limited in size and not all events can be passed to the display once it is full. (Configurable using 'discardOldEvents' param).");
    }
    ~AsyncDisplayModule() { } //< dtor
  };
}
