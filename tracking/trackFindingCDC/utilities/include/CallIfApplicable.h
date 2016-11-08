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

#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Calls the function with the given object.
     *  If the function cannot be call with the argument the signature, due to the decltype
     *  expression being invalid, the template instantiation is rejected (SFINAE).
     *  In this case the other overload that does not call the function is selected.
     */
    template <class AFunction, class T>
    auto callIfApplicableImpl(AFunction&& function,
                              T&& obj,
                              int favouredDispatch __attribute__((unused)))
    -> decltype(function(std::forward<T>(obj)), void())
    {
      function(std::forward<T>(obj));
    }

    /**
     *  Implemenation that does not call the function with the object.
     *  This implementation is selected with the function is not applicable.
     */
    template <class AFunction, class T>
    void callIfApplicableImpl(AFunction&& function __attribute__((unused)),
                              T&& obj __attribute__((unused)),
                              long disfavouredDispatch __attribute__((unused)))
    {
    }

    /**
     *  Invokes a function with one argument obj, but only if the call is allowed.
     *
     *  Useful in static polymorphic situations where the injected base class
     *  may or many not have a certain method and the derived class wants to override it.
     *  To not completly shadow the method the derived class needs call it if present but
     *  omit it if not.
     */
    template<class AFunction, class T>
    void callIfApplicable(AFunction&& function, T&& obj)
    {
      const int dispatchTag = 0;
      callIfApplicableImpl(std::forward<AFunction>(function),
                           std::forward<T>(obj),
                           dispatchTag);
    }


    /**
     *  Calls a getter function with the given object.
     *  If the function cannot be call with the argument the signature, due to the decltype
     *  expression being invalid, the template instantiation is rejected (SFINAE).
     *  In this case the other overload that does not call the function is selected.
     */
    template <class ADefault, class AFunction, class T>
    auto getIfApplicableImpl(AFunction&& function,
                             T&& obj,
                             ADefault defaultValue __attribute__((unused)),
                             int favouredDispatch __attribute__((unused)))
    -> decltype(function(std::forward<T>(obj)), ADefault())
    {
      return function(std::forward<T>(obj));
    }

    /**
     *  Implemenation that does not call the getter function with the object.
     *  This implementation is selected with the function is not applicable and returns the
     *  provided default value.
     */
    template <class ADefault, class AFunction, class T>
    ADefault getIfApplicableImpl(AFunction&& function __attribute__((unused)),
                                 T&& obj __attribute__((unused)),
                                 ADefault value,
                                 long disfavouredDispatch __attribute__((unused)))
    {
      return value;
    }

    /**
     *  Invokes a getter function with one argument obj, but only if the call is allowed.
     *
     *  If the call is not allowed return the default value.
     *
     *  Useful in static polymorphic situations where the injected base class
     *  may or many not have a certain method and the derived class wants to override it.
     *  To not completly shadow the method the derived class needs call it if present but
     *  omit it if not.
     */
    template<class ADefault, class AFunction, class T>
    ADefault getIfApplicable(AFunction&& function, T&& obj, ADefault value)
    {
      const int dispatchTag = 0;
      return getIfApplicableImpl<ADefault>(std::forward<AFunction>(function),
                                           std::forward<T>(obj),
                                           value,
                                           dispatchTag);
    }

    /// Functor for clearing an object as an example what to serve to callIfApplicable
    struct ClearMethod {
      /// AFunction to clear an object.
      template<class AObj>
      auto operator()(AObj& obj) -> decltype(obj.clear())
      {
        obj.clear();
      }
    };

    /// Clear the object if it supports the clear method
    template<class T>
    void clearIfApplicable(T&& obj)
    {
      // With C++14 you may use a generic lambda and remove the above ClearMethod
      // auto clearMethod = [](auto& obj) -> decltype(obj.clear()) { obj.clear(); };
      auto clearMethod = ClearMethod();
      callIfApplicable(clearMethod, std::forward<T>(obj));
    }
  }
}
