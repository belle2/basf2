/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ASYNCDISPLAYMODULE_H
#define ASYNCDISPLAYMODULE_H

#include <display/modules/display/DisplayModule.h>
#include <display/async/AsyncWrapper.h>

namespace Belle2 {
  /** Wraps DisplayModule and starts it in a forked process. */
  class AsyncDisplayModule : public AsyncWrapper {
  public:
    /** constructor. */
    AsyncDisplayModule(): AsyncWrapper(new DisplayModule()) {
      setDescription("Starts the Display module asynchronously (with buffered input). Note that the buffer is limited in size and not all events can be passed to the display once it is full. (Configurable using 'DiscardOldEvents' param).");
    }
    ~AsyncDisplayModule() { } //< dtor
  };
}

#endif
