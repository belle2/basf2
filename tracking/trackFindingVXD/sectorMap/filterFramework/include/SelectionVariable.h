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
  template < class templateArgumentType, class templateReturnType = double >
  class SelectionVariable {

  public:

    /** Type of the returned object. Needed for SFINAE in Filter.hh */
    typedef templateReturnType variableType;

    /** Type of the argument object. Needed for SFINAE in Filter.hh */
    typedef templateArgumentType argumentType;

    /** Static method that return the
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

#define SELECTION_VARIABLE( variableName, argumentType, implementation ) \
  class variableName:             \
    public SelectionVariable< argumentType , double >     \
  {                 \
  public:               \
    static const std::string name(void) {return #variableName; };   \
    implementation              \
  };                  \


}


