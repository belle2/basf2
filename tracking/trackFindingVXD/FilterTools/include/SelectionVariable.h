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

#include <string>
#include <typeinfo>
#include <cxxabi.h>
#include <stdlib.h>

using namespace std;

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
   #include "tracking/trackFindingVXD/FilterTools/SelectionVariable.hh"
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

    /** Returns the name of the selection variable.
     *
     * The name of the selection variable is the name of the C++ object
     * derived from SelectionVariable.
     * E.g.:
     \code
    #include "tracking/trackFindingVXD/FilterTools/SelectionVariable.hh"
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
    //...
    Difference someFilter();
    cout << someFilter.name() << endl;
    //...
    \endcode

     * will produce the output:
     " "Difference"
     * In principle the method can be overriden by the derived class,
     * but the persistency mechanism rely on name unicity, so why would
     * you like to write an external name manager?
     */

    const std::string name(void) {
      char* realname(NULL);
      int status(0);
      realname = abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
      std::string name(realname);
      free(realname);
      return name ;
    }

    /** A bogus virtual denstructor */
    virtual ~SelectionVariable() {};

    /** A bogus constructor */
    SelectionVariable() {};

  };

}


