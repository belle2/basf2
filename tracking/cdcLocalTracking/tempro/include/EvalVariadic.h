/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef EVALVARIADIC_H
#define EVALVARIADIC_H

namespace Belle2 {
  namespace CDCLocalTracking {

    /** Function serves as a placeholder for a variadic list of statements to be evaluated.
     *  The variadic syntax of C++ allows for variadic calls to a function which first map each element in an expression
     *  However, to evaluate statements for their side effects only one still has to wrap the statements by a dummy
     *  function call. This function provides this dummy structure, since it only takes a variadic argument list,
     *  but has no instructions on its own.
     *  Loosely inspired from http://en.wikipedia.org/wiki/Variadic_templates
     */
    template<typename... Args>
    static inline void evalVariadic(Args&& ...) {}

    /** Structure to serve as a placeholder for a variadic initializer list of statements to be evaluated.
     *  Similar to the evalVariadic functions this structure evaluates as variadic list of statements, but
     *  uses a initializer list instead of a function call.
     *  Loosely inspired from http://en.wikipedia.org/wiki/Variadic_templates
     */
    struct EvalVariadic {
      /// Constructor taking the variadic initalizer list.
      template<typename ...T> EvalVariadic(T...) {}
    };


  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // EVALVARIADIC_H
