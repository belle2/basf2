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

#include <tracking/trackFindingCDC/utilities/CompositeProcessingSignalListener.h>

#include <vector>
#include <tuple>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    /// Interface for a minimal algorithm part that wants to expose some parameters to a module
    template<class ... AIOTypes>
    class Findlet : public CompositeProcessingSignalListener {

    public:
      /// Types that should be served to apply on invokation
      using IOTypes = std::tuple<AIOTypes...>;

    protected:
      /// Forward a range of mutable objects
      template<class T>
      struct ToVectorImpl {
        /// A mutable range of Ts.
        using Type = std::vector<typename std::remove_reference<T>::type>;
      };

      /// Specialisation to only forward a range for immutable types.
      template<class T>
      struct ToVectorImpl<const T> {
        /// An immutable range of Ts.
        using Type = const std::vector<typename std::remove_reference<T>::type>;
      };

      /// Short hand for ToRangeImpl
      template<class T>
      using ToVector = typename ToVectorImpl<T>::Type;

    public:
      /// Vector types that should be served to apply on invokation
      using IOVectors = std::tuple< std::vector<AIOTypes>... >;

      /// Make destructor of interface virtual
      virtual ~Findlet() = default;

    public:
      /// Brief description of the purpose of the concret findlet.
      virtual std::string getDescription()
      {
        return "(no description)";
      }

      /**
       *  Forward prefixed parameters of this findlet to the module parameter list.
       *
       */
      virtual void exposeParameters(ModuleParamList* moduleParamList __attribute__((unused)),
                                    const std::string& prefix __attribute__((unused)))
      {
      }

      /// Main function executing the algorithm
      virtual void apply(ToVector<AIOTypes>& ... ioVectors) = 0;
    };
  }
}
