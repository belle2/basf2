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
#include <tracking/trackFindingVXD/sectorMap/filterFramework/VoidObserver.h>
#include <TBranch.h>
#include <TTree.h>
#include <string>
#include <type_traits>
#include <iostream>


#include <framework/logging/Logger.h>

namespace Belle2 {


  template< typename ... typePack >
  class Filter { /* Empty: just specialized templates are interesting */

  };

  template< typename ... types >
  struct all_same : std::false_type {};

  template< >
  struct all_same< > : std::true_type {};

  template<typename T >
  struct all_same< T > : std::true_type {};

  template<typename T, typename ... types >
  struct all_same< T, T, types ...> : all_same< T, types ...> {};


  /** these are just classnames to be leveraged in the template parameters pack  */
  class BypassableFilter;
  class ActivableFilter;


  /** Basic building block of the Filter tools
   *
   * Example usage:
  \code


  #include <tracking/trackFindingVXD/sectorMap/filterFramework/SelectionVariable.h>
  #include <tracking/trackFindingVXD/sectorMap/filterFramework/UpperBoundedSet.h>
  #include <tracking/trackFindingVXD/sectorMap/filterFramework/Filter.h>

  struct spacePoint{ float x; float y;float z; };

  SquaredDistance2D: public SelectionVariable< TSpacePoint > {
    value( const TSpacePoint & p1, const TSpacePoint &p2 ){
      return pow(p1.x - p2.x, 2 ) + pow(p1.y - p2.y, 2 );
    }
  };
  Filter< Distance3D, UpperBoundedSet, VoidObserver > filter( UpperBoundedSet( 1.1 ));

  spacePoint a( {0,0,0} ), b( {1,0,0} );
  cout << filter.accept(a,b) << endl;


  \endcode
   * will produce the output: true
   * The Variable class will provide the static method value x( arg1, arg2)
   * The Range object will provide the method contains to decide if x(arg1, arg2) is good
   * The Observer will be notified of the actions via its static method notify
   */
  template <
    class Variable,
    class Range,
    class Observer
    >
  class Filter < Variable, Range, Observer  > {
  public:

    /** Constructor.
     *
     * To construct the Filter we need a concrete Range. The variable and
     * the observer are passed through the template type pack.
     *
     * @param range: a class that provides a method \code
     bool contains( Variable::returnType x) \endcode
     *
     */
    Filter(const Range& range):
      m_range(range) { };

    Filter() { };

    /** Getter of the range */
    Range getRange(void) const { return m_range; }

    /** Handy typedef */
    typedef  typename Variable::argumentType argumentType;

    /** All the real computations are occuring in this method */
    template< typename ... argsType >
    /* this typename expands to bool only if all the argsType are
     of the same type of the argument of the filter*/
    typename std::enable_if< all_same< argumentType  , argsType ... >::value, bool >::type
    accept(const argsType& ... args) const
    {
      typename Variable::variableType value = Variable::value(args ...);
      Observer::notify(Variable(), value, m_range, args ...);
      return m_range.contains(value);

    }

    /** argTypes method persists the range.
     * @param t is the TTree under which the TBranch will be created
     * @param branchname is the name of the TBranch holding m_range
     */
    void persist(TTree* t, const std::string& branchName)
    {
      m_range.persist(t, branchName , Variable().name());
    }

    /** This method creates a new bypassable Filter with the same range of *this
     * E.g.:
    \code


    #include <tracking/trackFindingVXD/sectorMap/filterFramework/SelectionVariable.h>
    #include <tracking/trackFindingVXD/sectorMap/filterFramework/UpperBoundedSet.h>
    #include <tracking/trackFindingVXD/sectorMap/filterFramework/Filter.h>

    struct spacePoint{ float x; float y;float z; };

    SquaredDistance2D: public SelectionVariable< TSpacePoint > {
    value( const TSpacePoint & p1, const TSpacePoint &p2 ){
      return pow(p1.x - p2.x, 2 ) + pow(p1.y - p2.y, 2 );
    }
    };
    Filter< Distance3D, UpperBoundedSet, VoidObserver > filter( UpperBoundedSet( 1.1 ));
    bool bypassVariable(false);
    auto bypassableFilter( filter.bypass( bypassVariable ) );

    spacePoint a( {0,0,0} ), b( {10,0,0} );
    cout << bypassableFilter.accept(a,b) << endl;
    bypassVariable = true;
    cout << bypassableFilter.accept(a,b) << endl;

    \endcode

     * will produce:
     * false
     * true
     */
    Filter< Variable, Range, BypassableFilter, Observer>
    bypass(const bool& bypassVariable = false)
    {
      return Filter< Variable, Range, BypassableFilter, Observer>(m_range, bypassVariable);
    }

    Filter< Variable, Range, ActivableFilter, Observer>
    //    __attribute__((deprecated("Please use the bypass( const bool &) method instead")))
    enable(const bool& enableVariable = true)
    {
      return Filter< Variable, Range, ActivableFilter, Observer>(m_range, enableVariable);
    }

    template< class otherObserver >
    Filter< Variable, Range, otherObserver>
    observe(const otherObserver&)
    {
      return Filter< Variable , Range, otherObserver>(m_range);
    }

    template< class otherObserver >
    Filter(const Filter< Variable, Range, otherObserver>& filter):
      m_range(filter.getRange()) {};



  protected:
    Range  m_range;

  };


  /** Basic building block of the Filter tools
   *
   * An external bool variable bypass can force the Filter accept method to true. E.g.:
  \code


  #include <tracking/trackFindingVXD/sectorMap/filterFramework/SelectionVariable.h>
  #include <tracking/trackFindingVXD/sectorMap/filterFramework/UpperBoundedSet.h>
  #include <tracking/trackFindingVXD/sectorMap/filterFramework/Filter.h>

  struct spacePoint{ float x; float y;float z; };

  SquaredDistance2D: public SelectionVariable< TSpacePoint > {
    value( const TSpacePoint & p1, const TSpacePoint &p2 ){
      return pow(p1.x - p2.x, 2 ) + pow(p1.y - p2.y, 2 );
    }
  };
  bool bypassVariable(false);
  Filter< Distance3D, UpperBoundedSet, BypassableFilter, VoidObserver >
               filter( UpperBoundedSet( 1.1 ), bypassVariable);


  spacePoint a( {0,0,0} ), b( {10,0,0} );
  cout << filter.accept(a,b) << endl;
  bypassVariable = true;
  cout << filter.accept(a,b) << endl;

  \endcode

     * will produce:
     * false
     * true
     *
     * The Variable class will provide the static method value x( arg1, arg2)
     * The Range object will provide the method contains to decide if x(arg1, arg2) is good
     * The Observer will be notified of the actions via its static method notify
     */

  template <
    class Variable,
    class Range,
    class Observer
    >
  class Filter < Variable, Range, Belle2::BypassableFilter, Observer  >:
    public Filter< Variable, Range, Observer> {
  public:

    /** Constructor.
     *
     * To construct the Filter we need a concrete Range and a reference to a bool value.
     * The variable and the observer are passed through the template type pack.
     *
     * @param range: a class that provides a method \code
     bool contains( Variable::returnType x) \endcode
     * @param bypass is a bool value controlling the behaviour of accept: if bypass is set to
     * true the method accept will return always true, if bypass is set to false the accept
     * method will return the actual result of the test.
     */
    Filter(const Range& range , const bool& bypass):
      Filter< Variable, Range, Observer >(range)
    { m_bypass = &bypass; };
    Filter()  {};

    typedef  typename Variable::argumentType argumentType;

    /** All the real computations are occuring in this method */
    template< typename ... argsType >
    /* this typename expands to bool only if all the argsType are
     of the same type of the argument of the filter*/
    typename std::enable_if< all_same< argumentType, argsType ... >::value,
             bool >::type
             accept(const argsType& ... args) const
    {
      typename Variable::variableType value = Variable::value(args ...);
      Observer::notify(Variable(), value, Filter< Variable, Range, Observer >::m_range, args ...);
      return (*m_bypass) || Filter< Variable, Range, Observer >::m_range.contains(value);
    }

  private:

    const  bool* m_bypass;
  };

  /*  template <
    class Variable,
    class Range,
    class Observer
    >
    const bool * Filter < Variable, Range, BypassableFilter, Observer  >::m_bypass = NULL;
  */


  /** Basic building block of the Filter tools
   *
   * An external bool variable Enable/Disable the Filter E.g.:
  \code


  #include <tracking/trackFindingVXD/sectorMap/filterFramework/SelectionVariable.h>
  #include <tracking/trackFindingVXD/sectorMap/filterFramework/UpperBoundedSet.h>
  #include <tracking/trackFindingVXD/sectorMap/filterFramework/Filter.h>

  struct spacePoint{ float x; float y;float z; };

  SquaredDistance2D: public SelectionVariable< TSpacePoint > {
    value( const TSpacePoint & p1, const TSpacePoint &p2 ){
      return pow(p1.x - p2.x, 2 ) + pow(p1.y - p2.y, 2 );
    }
  };
  bool enable(false);
  Filter< Distance3D, UpperBoundedSet, ActivableFilter, VoidObserver >
               filter( UpperBoundedSet( 1.1 ), enable);

  enable = false;
  spacePoint a( {0,0,0} ), b( {10,0,0} );
  cout << filter.accept(a,b) << endl;
  enable = true;
  cout << filter.accept(a,b) << endl;

  \endcode

     * will produce:
     * true
     * false
     *
     * The Variable class will provide the static method value x( arg1, arg2)
     * The Range object will provide the method contains to decide if x(arg1, arg2) is good
     * The Observer will be notified of the actions via its static method notify
     */

  template <
    class Variable,
    class Range,
    class Observer
    >
  class Filter < Variable, Range, Belle2::ActivableFilter, Observer  >:
    public Filter< Variable, Range, Observer> {
  public:

    /** Constructor.
     *
     * To construct the Filter we need a concrete Range and a reference to a bool value.
     * The variable and the observer are passed through the template type pack.
     *
     * @param range: a class that provides a method \code
     bool contains( Variable::returnType x) \endcode
     * @param bypass is a bool value controlling the behaviour of accept: if bypass is set to
     * true the method accept will return always true, if bypass is set to false the accept
     * method will return the actual result of the test.
     */
    Filter(const Range& range , const bool& enable):
      Filter< Variable, Range, Observer >(range)
    {
      m_enable = & enable ;
    };


    Filter() { };

    /** Handy typedef */
    typedef  typename Variable::argumentType argumentType;

    /** All the real computations are occuring here */
    template< typename ... argsType >
    typename std::enable_if <
    all_same< argumentType, argsType ... >::value, bool >::type
    accept(const argsType& ... args) const
    {
      typename Variable::variableType value = Variable::value(args ...);
      Observer::notify(Variable(), value, Filter< Variable, Range, Observer >::m_range,
                       args ...);

//    bool enableCopy  = *m_enable;
//    B2INFO("mrange.contains: " << ", enable (should be true): " << std::to_string(enableCopy) );
      return (!(*m_enable)) || Filter< Variable, Range, Observer >::m_range.contains(value);
    }

  private:

    const bool* m_enable;
  };

  /*template <
    class Variable,
    class Range,
    class Observer
    >
  const bool * Filter < Variable, Range, ActivableFilter, Observer  >::m_enable = NULL;
  */

  /***
   * Boolean NOT of a given filters
   */

  class OperatorNot;

  template <
    class someFilter,
    class templateObserverType
    >
  class Filter < Belle2::OperatorNot, someFilter, templateObserverType > {
  public:

    typedef  typename someFilter::argumentType argumentType;


    template< typename ... argsType >
    typename std::enable_if< all_same< argumentType,
             argsType ... >::value, bool >::type
             accept(const argsType& ... args) const
    {
      return ! m_filter.accept(args ...);
    }

    void persist(TTree* t, const std::string& branchName)
    {

      std::string nameOfFilter(branchName);
      nameOfFilter += "_not";
      m_filter.persist(t, nameOfFilter);

    }

    Filter(const someFilter& filter):
      m_filter(filter) { };

    Filter() {};
  private:
    someFilter  m_filter;

  };

  template<typename ... types >
  Filter< OperatorNot, Filter< types...>, VoidObserver >
  operator !(const Filter< types...>& filter)
  {
    return Filter< OperatorNot, Filter< types...>, VoidObserver >(filter);
  }


  /***
   * Boolean AND among two filters
   */


  class OperatorAnd;

  template <
    class FilterA,
    class FilterB,
    class templateObserverType
    >
  class Filter < Belle2::OperatorAnd, FilterA, FilterB, templateObserverType > {
  public:

    typedef  typename FilterA::argumentType argumentType;
    typedef  typename FilterB::argumentType argumentTypeB;

    template< typename ... argsType >
    typename std::enable_if <
    all_same< argumentType, argumentTypeB,
              argsType ... >::value, bool >::type
              accept(const argsType& ... args) const
    {
      templateObserverType::prepare(args ...);
      bool returnValue =  m_filterA.accept(args ...) && m_filterB.accept(args ...);
      templateObserverType::collect(args ...);
      return returnValue;
    }

    template< class otherObserver >
    Filter<  Belle2::OperatorAnd, FilterA, FilterB, otherObserver>
    observe(const otherObserver&)
    {
      return Filter< Belle2::OperatorAnd, FilterA, FilterB, otherObserver >(m_filterA, m_filterB);
    }

    void persist(TTree* t, const std::string& branchName)
    {
      std::string nameOfFilterA(branchName);
      nameOfFilterA += "_and_A";
      m_filterA.persist(t, nameOfFilterA);

      std::string nameOfFilterB(branchName);
      nameOfFilterB += "_and_B";
      m_filterB.persist(t, nameOfFilterB);

    }



    Filter(const FilterA& filterA, const FilterB& filterB):
      m_filterA(filterA), m_filterB(filterB) { };

    Filter() { };



  private:
    FilterA  m_filterA;
    FilterB  m_filterB;

  };

  template <
    typename ... types1,
    typename ... types2
    >
  Filter< Belle2::OperatorAnd, Belle2::Filter< types1...>, Belle2::Filter< types2...> , Belle2::VoidObserver >
  operator &&(const Filter< types1...>& filter1 , const Filter< types2...>& filter2)
  {
    return Filter < OperatorAnd,
           Filter< types1...>,
           Filter< types2...>, VoidObserver > (filter1, filter2);
  }


  /***
   * Boolean OR among two filters
   */

  class OperatorOr;

  template <
    class FilterA,
    class FilterB,
    class templateObserverType
    >
  class Filter < Belle2::OperatorOr, FilterA, FilterB, templateObserverType > {
  public:

    typedef  typename FilterA::argumentType argumentType;
    typedef  typename FilterB::argumentType argumentTypeB;



    template< typename ... argsType >
    typename std::enable_if <
    all_same<  argumentType, argumentTypeB,
               argsType ... >::value, bool >::type
               accept(const argsType& ... args) const
    {

      return m_filterA.accept(args ...) || m_filterB.accept(args ...);

    }



    Filter(const FilterA& filterA, const FilterB& filterB):
      m_filterA(filterA), m_filterB(filterB) { };
    Filter() {};

    void persist(TTree* t, const std::string& branchName)
    {
      std::string nameOfFilterA(branchName);
      nameOfFilterA += "_or_A";
      m_filterA.persist(t, nameOfFilterA);

      std::string nameOfFilterB(branchName);
      nameOfFilterB += "_or_B";
      m_filterB.persist(t, nameOfFilterB);

    }

  private:
    FilterA  m_filterA;
    FilterB  m_filterB;

  };

  template <
    typename ... types1,
    typename ... types2
    >
  Filter< Belle2::OperatorOr, Belle2::Filter< types1...>, Belle2::Filter< types2...> , Belle2::VoidObserver >
  operator ||(const Filter< types1...>& filter1 , const Filter< types2...>& filter2)
  {
    return Filter < OperatorOr,
           Filter< types1...>,
           Filter< types2...>, VoidObserver > (filter1, filter2);
  }

  /** Initilize all the observers in a binary boolean Filter. */
  template< class booleanBinaryOperator,
            typename ... types1,
            typename ... types2,
            class observer,
            typename ... argsTypes>
  bool initializeObservers(Filter< booleanBinaryOperator,
                           Belle2::Filter< types1...>, Belle2::Filter< types2...> , observer>,
                           argsTypes ... args)
  {
    return observer::initialize(args ...) &&
           initializeObservers(Belle2::Filter< types1...>(), args...) &&
           initializeObservers(Belle2::Filter< types2...>(), args ...);
  }

  /** Initilize all the observers in a unary boolean Filter. */
  template< class booleanUnaryOperator,
            typename ... types1,
            class observer,
            typename ... argsTypes>
  bool initializeObservers(Filter< booleanUnaryOperator,
                           Belle2::Filter< types1...>, observer>, argsTypes ... args)
  {
    return observer::initialize(args ...) && initializeObservers(Belle2::Filter< types1...>(), args...);
  }

  /** Initilize the observer of a Range Filter. */

  template< class Variable, class Range, class observer, typename ... argsTypes>
  bool initializeObservers(Belle2::Filter<Variable, Range, observer> filter, argsTypes ... args)
  {
    return observer::initialize(Variable(), filter.getRange(), args ...);
  }
}
