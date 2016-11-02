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

#include <tracking/trackFindingCDC/findlets/base/ProcessingSignalListener.h>

#include <framework/core/ModuleParamList.h>
#include <boost/range/adaptor/reversed.hpp>
#include <vector>
#include <tuple>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Interface for a minimal algorithm part that wants to expose some parameters to a module
    template<class ... AIOTypes>
    class Findlet : public ProcessingSignalListener {

    public:
      /// Types that should be served to apply on invokation
      using IOTypes = std::tuple<AIOTypes...>;

    protected:
      /// Forward a range of mutable objects
      template<class T>
      struct ToVectorImpl {
        /// A mutable range of Ts.
        using Type = std::vector<typename std::remove_reference<T>::type >;
      };

      /// Specialisation to only forward a range for immutable types.
      template<class T>
      struct ToVectorImpl<const T> {
        /// An immutable range of Ts.
        using Type = const std::vector<T>;
      };

      /// Short hand for ToRangeImpl
      template<class T>
      using ToVector = typename ToVectorImpl<T>::Type;

    public:
      /// Vector types that should be served to apply on invokation
      using IOVectors = std::tuple< std::vector<AIOTypes>... >;

      /// Allow default constructin
      Findlet() = default;

      /// Disallow copies
      Findlet(const Findlet&) = delete;

      /// Disallow assignment
      Findlet& operator= (const Findlet&) = delete;

    public:
      /// Brief description of the purpose of the concret findlet.
      virtual std::string getDescription()
      {
        return "(no description)";
      }

      /** Forward prefixed parameters of this findlet to the module parameter list */
      virtual void exposeParameters(ModuleParamList*, const std::string& = "")
      {
      }

      /// Main function executing the algorithm
      virtual void apply(ToVector<AIOTypes>& ... ranges) = 0;

      /// Register a processing signal listener that is contained in this findlet.
      void addProcessingSignalListener(ProcessingSignalListener* psl)
      { m_subordinaryProcessingSignalListeners.push_back(psl); }

      /// Receive signal before the start of the event processing
      virtual void initialize() override
      {
        for (ProcessingSignalListener* psl :  m_subordinaryProcessingSignalListeners) {
          psl->initialize();
        }
      }

      /// Receive signal for the beginning of a new run.
      virtual void beginRun() override
      {
        for (ProcessingSignalListener* psl :  m_subordinaryProcessingSignalListeners) {
          psl->beginRun();
        }
      }

      /// Receive signal for the start of a new event.
      virtual void beginEvent() override
      {
        for (ProcessingSignalListener* psl :  m_subordinaryProcessingSignalListeners) {
          psl->beginEvent();
        }
      }

      /// Receive signal for the end of the run.
      virtual void endRun() override
      {
        using boost::adaptors::reverse;
        for (ProcessingSignalListener* psl : reverse(m_subordinaryProcessingSignalListeners)) {
          psl->endRun();
        }
      }

      /// Receive Signal for termination of the event processing.
      virtual void terminate() override
      {
        using boost::adaptors::reverse;
        for (ProcessingSignalListener* psl : reverse(m_subordinaryProcessingSignalListeners)) {
          psl->terminate();
        }
      }

    private:
      /// References to subordinary signal processing listener contained in this findlet.
      std::vector<ProcessingSignalListener*> m_subordinaryProcessingSignalListeners;

    };

  }
}
