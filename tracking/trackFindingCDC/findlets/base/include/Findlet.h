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
#include <tuple>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Interface for a minimal algorithm part that wants to expose some parameters to a module
    template<class ... AIOTypes>
    class Findlet {

    public:
      /// Types that should be served to apply on invokation
      using IOTypes = std::tuple<AIOTypes...>;

    private:
      /// Only forward a range for immutable types
      template<class T>
      struct ToVectorImpl {
        /// A mutable range of Ts.
        using Type = std::vector<T>;
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


    public:
      /// Brief description of the purpose of the concret findlet.
      virtual std::string getDescription()
      {
        return "(no description)";
      }

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
      virtual void apply(ToVector<AIOTypes>& ... ranges) = 0;

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
