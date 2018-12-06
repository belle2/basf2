/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Mikhail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
