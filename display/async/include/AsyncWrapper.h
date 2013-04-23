/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ASYNCWRAPPER_H
#define ASYNCWRAPPER_H

#include <framework/core/Module.h>

namespace Belle2 {
  class RxModule;
  class TxModule;
  class RingBuffer;
  class ProcHandler;
  class Module;

  /** Wraps a given Module to execute it asynchronously.
   *
   */
  class AsyncWrapper : public Module {
  public:
    /**
     * Wrap the given module.
     *
     * Takes ownership of wrapMe.
     */
    AsyncWrapper(Module* wrapMe);

    ~AsyncWrapper();

    /** Call this from initialize().
     *
     * This function will fork() to create the wrapping environment, everything done previously will be available in the wrapped module.
     */
    void initialize();
    /** Call this from event().  */
    void event();
    /** Call this from terminate().  */
    void terminate();


  private:
    /** The wrapped module. */
    Module* m_wrappedModule;

    /** process manager. */
    ProcHandler* m_procHandler;

    /** shared memory buffer */
    RingBuffer* m_ringBuffer;

    /** receiving module. */
    RxModule* m_rx;

    /** transmitting module. */
    TxModule* m_tx;

    /** Discard old events when buffer is full. If false, the main process will wait until there is enough space in the buffer. (i.e. synchronous operation) */
    bool m_discardOldEvents;
  };
}
#endif
