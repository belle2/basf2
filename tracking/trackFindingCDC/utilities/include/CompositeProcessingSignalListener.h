/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/ProcessingSignalListener.h>

#include <vector>

namespace Belle2 {
  class ModuleParamList;
  namespace TrackFindingCDC {
    /// Partial implementation for an algorithm part that wants to dispatch processing signals to subobjects.
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

      /// @brief Expose the set of parameters of the filter to the module parameter list.
      /// @param moduleParamList Module parameter list to add the new parameters to
      /// @param prefix Prefix (or sometimes postfix) to be added to all nely added parameters.
      virtual void exposeParameters(ModuleParamList* moduleParamList __attribute__((unused)),
                                    const std::string& prefix __attribute__((unused)))
      {};

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
