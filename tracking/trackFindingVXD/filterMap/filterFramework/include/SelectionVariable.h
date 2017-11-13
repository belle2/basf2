/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


/** c macros to ensure that the returned name for each Class is the same as the class name */
// is expanded as the string value for X
#define STRING_NAME(X) #X
// is expanded as the function "static const std::string name(void)" which returns the string value of Y
#define PUT_NAME_FUNCTION(Y) static const std::string name(void) {return STRING_NAME(Y); };

#include <string>
#include <typeinfo>
#include <functional>
#include <TBranch.h>
#include <TTree.h>

//#include <stdlib.h>

namespace Belle2 {
  /** Base class of the selection variable objects used for pair filtering
   *
   * The filter tools are ment to select in a very flexible and efficient
   * manner pairs of objects of type templateArgumentType.
   * The selection decision is taken on the base of some selection variables.
   * The arguments of the selection functions are two objects of type
   * templateArgumentType. The selection function returns a templateReturnType
   * which by default is a double.
   * E.g.: To code a selection function returning the sum of two integers
   \code
   #include "tracking/trackFindingVXD/filterTools/SelectionVariable.hh"
   //...

  class Difference: public SelectionVariable< int, int >
  {
    static int
    value( const int & arg1, const int & arg2 ){
      return arg1 + arg2;
    };

    Difference() {};
    ~Difference() {};
  };
    \endcode


   */
  template < class templateArgumentType, size_t Nargs, class templateReturnType = double >
  class SelectionVariable {

    /** this struct is an internal utility.
     * She provides the typename of an std:function taking
     * n parameters type parameterType and Ts... additional parameters.
     * e.g.:
     * FunctionOf<2, double, SpacePoint>::functionType is the type of
     * a function returning a double and having two SpacePoint as arguments
     */
    template <size_t n , class returnType, class parameterType, class ... Ts>
    struct FunctionOf {
      typedef  typename FunctionOf < n - 1, returnType, parameterType,
               parameterType, Ts... >::functionType functionType;
    };

    /** this struct is an internal utility.
     * She is the guardian of the recursion if n = 0
     * the job is done: 0 parameterType just Ts
     */
    template <  class returnType, class parameterType, class ... Ts>
    struct FunctionOf<0, returnType, parameterType, Ts...> {

      typedef  std::function< returnType(Ts...) > functionType;

    };

  public:

    /** Type of the returned object. Needed for SFINAE in Filter.hh */
    typedef templateReturnType variableType;

    /** Type of the argument object. Needed for SFINAE in Filter.hh */
    typedef templateArgumentType argumentType;

    /** Type of the function returning the value. Needed for the automated training */
    typedef typename FunctionOf<Nargs, variableType,
            const argumentType&>::functionType functionType;

    static const size_t c_Nargs = Nargs;
    /** Static method that return the variable associated to:
     *
     * @param arg1 first object of the pair
     * @param arg2 second object of the pair
     *
     * N.B. this method must be static.
     */
    inline static variableType
    value(const templateArgumentType& arg1, const templateArgumentType& arg2);

    /** A bogus virtual denstructor */
    virtual ~SelectionVariable() {};

    /** A bogus constructor */
    SelectionVariable() {};

  };

#define SELECTION_VARIABLE( variableName, nArgs, argumentType, implementation ) \
  class variableName:             \
    public SelectionVariable< argumentType , nArgs, double >     \
  {                 \
  public:               \
    static const std::string name(void) {return #variableName; };   \
    implementation              \
  };                  \


}


