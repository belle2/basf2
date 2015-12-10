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

#include <framework/core/ModuleParamList.h>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Only forward a range for immutable types
    template<class T>
    struct ToRangesImpl {
      /// A mutable range of Ts.
      using Type = std::vector<T>;
    };

    /// Specialisation to only forward a range for immutable types.
    template<class T>
    struct ToRangesImpl<const T> {
      /// An immutable range of Ts.
      using Type = const std::vector<T>;
    };

    /// Short hand for ToRangeImpl
    template<class T>
    using ToRanges = typename ToRangesImpl<T>::Type;

    /// Interface for a minimal algorithm part that wants to expose some parameters to a module
    template<class ... ScalarIOTypes>
    class Findlet {
    public:
      /** Forward the parameters of this findlet to the module parameter list */
      virtual void exposeParameters(ModuleParamList*)
      {
      }

      /// Initialize the Module before event processing
      virtual void initialize()
      {
      }

      /// Signal the beginning of a new run.
      virtual void beginRun()
      {
      }

      /// Start processing the current event
      virtual void beginEvent()
      {
      }

      /// Main function executing the algorithm
      virtual void apply(ToRanges<ScalarIOTypes>& ... ranges) = 0;

      /// End processing the current event
      virtual void endEvent()
      {
      }

      /// Signal the end of the run.
      virtual void endRun()
      {
      }

      /// Singal to terminate the event processing
      virtual void terminate()
      {
      }
    };

  } //end namespace TrackFindingCDC
} //end namespace Belle2
