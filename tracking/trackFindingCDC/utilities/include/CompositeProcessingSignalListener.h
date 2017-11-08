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

#include <tracking/trackFindingCDC/utilities/ProcessingSignalListener.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Partial implemenation for an algorithm part that wants to dispatch processing signals to subobjects.
    class CompositeProcessingSignalListener : public ProcessingSignalListener {

    private:
      /// Type of the base class
      using Super = ProcessingSignalListener;

    public:
      /// Receive and dispatch signal before the start of the event processing
      void initialize() override;

      /// Receive and dispatch signal for the beginning of a new run.
      void beginRun() override;

      /// Receive and dispatch signal for the start of a new event.
      void beginEvent() override;

      /// Receive and dispatch signal for the end of the run.
      void endRun() override;

      /// Receive and dispatch Signal for termination of the event processing.
      void terminate() override;

    protected:
      /// Register a processing signal listener to be notified.
      void addProcessingSignalListener(ProcessingSignalListener* psl);

      /// Get the number of currently registered listeners.
      int getNProcessingSignalListener();

    private:
      /// References to subordinary signal processing listener contained in this findlet.
      std::vector<ProcessingSignalListener*> m_subordinaryProcessingSignalListeners;
    };
  }
}
