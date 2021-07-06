/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//Framework
#include <framework/pcore/AsyncWrapper.h>


namespace Belle2 {
  /**
   * Displays energy distribution in ECL.
   *
   *
   */
  class EclDisplayAsyncModule : public AsyncWrapper {

  public:

    /**  */
    EclDisplayAsyncModule() : AsyncWrapper("EclDisplay")
    {
      setDescription("Starts the Display module asynchronously (with buffered input). Note that the buffer is limited in size and not all events can be passed to the display once it is full. (Configurable using 'discardOldEvents' param).");
    }

    /**  */
    ~EclDisplayAsyncModule() { }

  };
}
