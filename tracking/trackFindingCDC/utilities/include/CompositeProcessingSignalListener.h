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

#include <boost/range/adaptor/reversed.hpp>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Partial implemenation for an algorithm part that wants to dispatch processing signals to subobjects.
    class CompositeProcessingSignalListener : public ProcessingSignalListener {

    private:
      /// Type of the base class
      using Super = ProcessingSignalListener;

    public:
      /// Register a processing signal listener to be notified.
      void addProcessingSignalListener(ProcessingSignalListener* psl)
      {
        m_subordinaryProcessingSignalListeners.push_back(psl);
      }

      /// Receive and dispatch signal before the start of the event processing
      void initialize() override
      {
        Super::initialize();
        for (ProcessingSignalListener* psl :  m_subordinaryProcessingSignalListeners) {
          psl->initialize();
        }
      }

      /// Receive and dispatch signal for the beginning of a new run.
      void beginRun() override
      {
        Super::beginRun();
        for (ProcessingSignalListener* psl : m_subordinaryProcessingSignalListeners) {
          psl->beginRun();
        }
      }

      /// Receive and dispatch signal for the start of a new event.
      void beginEvent() override
      {
        Super::beginEvent();
        for (ProcessingSignalListener* psl : m_subordinaryProcessingSignalListeners) {
          psl->beginEvent();
        }
      }

      /// Receive and dispatch signal for the end of the run.
      void endRun() override
      {
        using boost::adaptors::reverse;
        for (ProcessingSignalListener* psl : reverse(m_subordinaryProcessingSignalListeners)) {
          psl->endRun();
        }
        Super::endRun();
      }

      /// Receive and dispatch Signal for termination of the event processing.
      void terminate() override
      {
        using boost::adaptors::reverse;
        for (ProcessingSignalListener* psl : reverse(m_subordinaryProcessingSignalListeners)) {
          psl->terminate();
        }
        Super::terminate();
      }

    private:
      /// References to subordinary signal processing listener contained in this findlet.
      std::vector<ProcessingSignalListener*> m_subordinaryProcessingSignalListeners;
    };
  }
}
