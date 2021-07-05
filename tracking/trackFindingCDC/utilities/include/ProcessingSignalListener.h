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

#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Interface for an algorithm part that needs to receive the module processing signals.
    class ProcessingSignalListener {

    public:
      /// Allow default construction
      ProcessingSignalListener();

      /// Disallow copies
      ProcessingSignalListener(const ProcessingSignalListener&) = delete;

      /// Disallow assignment
      ProcessingSignalListener& operator= (const ProcessingSignalListener&) = delete;

      /// Make destructor of interface virtual
      virtual ~ProcessingSignalListener();

      /// Receive signal before the start of the event processing
      virtual void initialize();

      /// Receive signal for the beginning of a new run.
      virtual void beginRun();

      /// Receive signal for the start of a new event.
      virtual void beginEvent();

      /// Receive signal for the end of the run.
      virtual void endRun();

      /// Receive Signal for termination of the event processing.
      virtual void terminate();

    private:
      /// Flag to keep track whether initialization happend before
      bool m_initialized = false;

      /// Flag to keep track whether termination happend before
      bool m_terminated = false;

      /// Name of the type during initialisation
      std::string m_initializedAs;
    };
  }
}
