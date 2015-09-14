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

    /** Calls the function with the given object.
     *  If the function can not be call with the argument the signature due to decltype
     *  is invalid and the template instantiation is rejected (SFINAE)
     *  In this case the other overload that does not call the function is selected. */
    template<class AFunction, class T>
    auto callIfApplicableImpl(AFunction&& function, T&& obj, int)
    -> decltype(function(std::forward<T>(obj)), void())
    { function(std::forward<T>(obj)); }

    /** Implemenation that does not call the function with the object.
     *  This implementation is selected with the function is not applicable.
     */
    template<class AFunction, class T>
    void callIfApplicableImpl(AFunction&&, T&&, long)
    {}

    /** Invokes a function with one argument obj, but only if the call is allowed.
     *
     *  Useful in static polymorphic situations where the injected base class
     *  may or many not have a certain method and the derived class wants to override it.
     *  To not completly shadow the method the derived class needs call it if present but
     *  omit it if not.
     */
    template<class AFunction, class T>
    void callIfApplicable(AFunction&& function, T&& obj)
    {
      callIfApplicableImpl(std::forward<AFunction>(function),
                           std::forward<T>(obj), static_cast<int>(0));
    }


    /** Calls a getter function with the given object.
     *  If the function can not be call with the argument the signature due to decltype
     *  is invalid and the template instantiation is rejected (SFINAE)
     *  In this case the other overload that does not call the function is selected. */
    template<class ADefault, class AFunction, class T>
    auto getIfApplicableImpl(AFunction&& function, T&& obj, ADefault /*value*/, int)
    -> decltype(function(std::forward<T>(obj)), ADefault())
    { return function(std::forward<T>(obj)); }

    /** Implemenation that does not call the getter function with the object.
     *  This implementation is selected with the function is not applicable and returns the
     * provided default value
     */
    template<class ADefault, class AFunction, class T>
    ADefault getIfApplicableImpl(AFunction&&, T&&, ADefault value, long)
    { return value;}

    /** Invokes a getter function with one argument obj, but only if the call is allowed.
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
      return getIfApplicableImpl<ADefault>(std::forward<AFunction>(function),
                                           std::forward<T>(obj), value, static_cast<int>(0));
    }


    /// Functor for clearing an object.
    struct ClearMethod {
      /// AFunction to clear an object.
      template<class AObj> auto operator()(AObj& obj) -> decltype(obj.clear()) { obj.clear(); }
    };

    /// Clear the object if it supports the clear method
    template<class T>
    void clearIfApplicable(T&& obj)
    { callIfApplicable(ClearMethod(), std::forward<T>(obj)); }

  }
}
