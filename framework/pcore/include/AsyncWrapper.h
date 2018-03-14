/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  class RxModule;
  class TxModule;
  class RingBuffer;
  class ProcHandler;

  /** Wraps a given Module to execute it asynchronously.
   *
   * The module will be executed in its own process and will receive all
   * event data that would normally be available to it, once available.
   * Anything written to the DataStore by the wrapped module will be lost.
   *
   * To use it, inherit from AsyncWrapper and call it's constructor within
   * your own constructor, passing it the name of the wrapped module.
   *
   * \sa See AsyncDisplayModule for an example that wraps DisplayModule.
   *
   */
  class AsyncWrapper : public Module {
  public:
    /**
     * Wrap am module of given type.
     */
    AsyncWrapper(const std::string& moduleType);

    ~AsyncWrapper();

    /** Access wrapped module (owned by AsyncWrapper). */
    ModulePtr getWrapped() { return m_wrappedModule; }

    /** Call this from initialize().
     *
     * This function will fork() to create the wrapping environment, everything done previously will be available in the wrapped module.
     */
    void initialize();
    /** Call this from event().  */
    void event();
    /** Call this from terminate().  */
    void terminate();

    /** returns true if the current process is on the receiving (async) side of an AsyncWrapper. */
    static bool isAsync() { return s_isAsync; }

    /** Retun number of events available in the RingBuffer. */
    static int numAvailableEvents();


  private:
    /** The wrapped module. */
    ModulePtr m_wrappedModule;

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

    /** true if the current process is on the receiving (async) side of an AsyncWrapper. */
    static bool s_isAsync;

    /** if s_isAsync is true, this contains the corresponding RingBuffer, see numAvailableEvents(). */
    static RingBuffer* s_currentRingBuffer;
  };
}
