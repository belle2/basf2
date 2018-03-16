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
#include <TBranch.h>
#include <TTree.h>
#include <string>
#include <type_traits>
#include <iostream>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingVXD/filterMap/filterFramework/VoidObserver.h>


namespace Belle2 {
  /**
   * This class is used to select pairs, triplets... of objects.
   * It is meant to be used on a large set of objects and it is
   * designed to be as efficient as hand written code by exploiting
   * templates. The filter structure is defined at compilation time
   * i.e.  ( var1 > xxx && var2 < yyy ) || var1 <zzz
   * while xxx yyy and zzz are defined at run time.
   * The template expansion at compilation time assures short circuit
   * evaluation etc. etc.
   * Several other nice features are coded.
   */
  template<typename ... typePack>
  class Filter {
    /* Empty: just specialized templates are interesting */
  };


  /**
   * The all_same struct is meant to check that all the types in a template
   * pack are of the same type.
   * If at the end the compiler ends here is because the first two
   * types are different: so return false.
   */
  template<typename ... types>
  struct all_same : std::false_type {};

  /**
   * The all_same struct is meant to check that all the types in a template
   * pack are of the same type.
   * It is true if the template pack is formed by a single type.
   */
  template<typename T>
  struct all_same<T> : std::true_type {};

  /**
   * The all_same struct is meant to check that all the types in a template
   * pack are of the same type.
   * It is true if the template pack is empty.
   */
  template< >
  struct all_same< > : std::true_type {};

  /**
   * The all_same struct is meant to check that all the types in a template
   * pack are of the same type.
   * If the first two types are identical then eat the first one and check
   * for the others.
   */
  template<typename T, typename ... types>
  struct all_same<T, T, types ...> : all_same<T, types ...> {};

  /** These are just class names to be leveraged in the template parameters pack  */
  /// Tag class for filter that can be bypassed
  class BypassableFilter;
  /// Tag class for filter that can be activated
  class ActivatableFilter;


  /// Basic Filter ///
  /** Basic building block of the Filter tools
   * Example usage:
      \code

       #include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
       #include <tracking/trackFindingVXD/filterMap/filterFramework/UpperBoundedSet.h>
       #include <tracking/trackFindingVXD/filterMap/filterFramework/Filter.h>

       struct spacePoint{ float x; float y; float z; };

       SquaredDistance2D: public SelectionVariable<TSpacePoint> {
         value(const TSpacePoint & p1, const TSpacePoint &p2){
           return pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2);
         }
       };
       Filter<SquaredDistance2D, UpperBoundedSet, VoidObserver> filter(UpperBoundedSet( 1.1 ));

       spacePoint a( {0,0,0} ), b( {1,0,0} );
       cout << filter.accept(a,b) << endl;

     \endcode

   * will produce the output: true
   * The Variable class will provide the static method value x( arg1, arg2)
   * The RangeType object will provide the method contains to decide if x(arg1, arg2) is good
   * The Observer will be notified of the actions via its static method notify
   */
  template <
    class Variable,
    class RangeType,
    class Observer
    >
  class Filter <Variable, RangeType, Observer> {
  public:
    /** Handy typedef for arguments */
    typedef typename Variable::argumentType argumentType;

    /** Handy typedef for function */
    typedef typename Variable::functionType functionType;

    /** Constructor.
     * To construct the Filter we need a concrete RangeType. The variable and
     * the observer are passed through the template type pack.
     *
     * @param range: a class that provides a method contains such as:
        \code bool contains(Variable::returnType x) \endcode
     */
    explicit Filter(const RangeType& range):
      m_range(range) { };

    /** Empty constructor */
    Filter() = default;


    /** Getter of the range */
    RangeType getRange(void) const { return m_range; }


    /** The accept method of the filter:
     * All the real computations are occuring in this method.
     * @tparam argsType: template arguments depending on the filter
     * @param args: values to be tested by the filters
     * @return boolean indicating if filter is passed
     */
    template<typename ... argsType>
    /** This typename expands to bool only if all the argsType are
     of the same type of the argument of the filter */
    typename std::enable_if<all_same<argumentType, argsType ... >::value, bool>::type
    accept(const argsType& ... args) const
    {
      typename Variable::variableType value = Variable::value(args ...);
      Observer::notify(Variable(), value, m_range, args ...);
      return m_range.contains(value);
    }


    /** Persist the range on a TTree.
     * @param t is the TTree under which the TBranch will be created
     * @param branchname is the name of the TBranch holding m_range
     */
    void persist(TTree* t, const std::string& branchName)
    {
      m_range.persist(t, branchName, Variable().name());
    }


    /** Set the Branches addresses to this filter.
     * @param t is the TTree containing the TBranch
     * @param branchname is the name of the TBranch holding the m_range
     */
    void setBranchAddress(TTree* t, const std::string& branchName)
    {
      m_range.setBranchAddress(t, branchName, Variable().name());
    }


    /** This method creates a new bypassable Filter with the same range of *this
     * E.g.:
        \code

         #include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
         #include <tracking/trackFindingVXD/filterMap/filterFramework/UpperBoundedSet.h>
         #include <tracking/trackFindingVXD/filterMap/filterFramework/Filter.h>

         struct spacePoint{ float x; float y; float z; };

         SquaredDistance2D: public SelectionVariable<TSpacePoint> {
         value(const TSpacePoint & p1, const TSpacePoint &p2){
           return pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2);
         }
         };
         Filter<SquaredDistance2D, UpperBoundedSet, VoidObserver> filter(UpperBoundedSet( 1.1 ));
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
    Filter<Variable, RangeType, BypassableFilter, Observer>
    bypass(const bool& bypassVariable = false)
    {
      return Filter<Variable, RangeType, BypassableFilter, Observer>(m_range, bypassVariable);
    }


    /** Create a new activatable filter
     * @param enableVariable: If the filter is active
     * @return filter
     */
    Filter<Variable, RangeType, ActivatableFilter, Observer>
    //    __attribute__((deprecated("Please use the bypass( const bool &) method instead")))
    enable(const bool& enableVariable = true)
    {
      return Filter<Variable, RangeType, ActivatableFilter, Observer>(m_range, enableVariable);
    }


    /** Function to observer of a filter
     * @tparam otherObserver: observer to be used
     * @return filter with the respective observer
     */
    template< class otherObserver >
    Filter<Variable, RangeType, otherObserver>
    observeLeaf(const otherObserver&) const
    {
      return Filter<Variable, RangeType, otherObserver>(m_range);
    }


    /** Copy constructor for filter
     * @tparam otherObserver: observer type
     * @param filter: filter to be copied
     */
    template< class otherObserver >
    Filter(const Filter<Variable, RangeType, otherObserver>& filter):
      m_range(filter.getRange()) {};


    /** Getter for name of and reference to the range of the filters.
     * Calls getNameAndReference of RangeType.
     * @param pointers: pointer to vector of pairs of names of and pointers to the filters
     * @return string containing name of the variable and the bounds of the range (see RangeTypes)
     */
    std::string getNameAndReference(std::vector<std::pair<char, void*> >* pointers = nullptr)
    {
      return m_range.getNameAndReference(pointers, Variable::name());
    }

  protected:
    /// Member range of the filter.
    RangeType  m_range;
  };



  /// Bypassable Filter ///
  /** Extended implementation of the Filter tools which allowes to bypass the filter by returning always true if wanted.
   * An external bool variable bypass can force the Filter accept method to always return true.
   * E.g.:
      \code

       #include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
       #include <tracking/trackFindingVXD/filterMap/filterFramework/UpperBoundedSet.h>
       #include <tracking/trackFindingVXD/filterMap/filterFramework/Filter.h>

       struct spacePoint{ float x; float y; float z; };

       SquaredDistance2D: public SelectionVariable<TSpacePoint> {
         value(const TSpacePoint & p1, const TSpacePoint &p2){
           return pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2);
         }
       };
       bool bypassVariable(false);
       Filter<SquaredDistance2D, UpperBoundedSet, BypassableFilter, VoidObserver>
                    filter(UpperBoundedSet( 1.1 ), bypassVariable);

       spacePoint a( {0,0,0} ), b( {10,0,0} );
       cout << filter.accept(a,b) << endl;
       bypassVariable = true;
       cout << filter.accept(a,b) << endl;

     \endcode

   * will produce:
   * false
   * true
   *
   * The Variable class will provide the static method value x(arg1, arg2)
   * The RangeType object will provide the method contains to decide if x(arg1, arg2) is good
   * The Observer will be notified of the actions via its static method notify
   */
  template <
    class Variable,
    class RangeType,
    class Observer
    >
  class Filter <Variable, RangeType, Belle2::BypassableFilter, Observer>:
    public Filter<Variable, RangeType, Observer> {
  public:
    /** Handy typedef for arguments */
    typedef typename Variable::argumentType argumentType;
    /** Handy typedef for function */
    typedef typename Variable::functionType functionType;


    /** Constructor.
     * To construct the Filter we need a concrete Range and a reference to a bool value.
     * The variable and the observer are passed through the template type pack.
     *
     * @param range: a class that provides a method contains:
         \code bool contains( Variable::returnType x) \endcode
     * @param bypass is a bool value controlling the behaviour of accept: if bypass is set to
     * true the method accept will return always true, if bypass is set to false the accept
     * method will return the actual result of the test.
     */
    Filter(const RangeType& range, const bool& bypass):
      Filter<Variable, RangeType, Observer>(range)
    { m_bypass = &bypass; };


    /** Empty constructor */
    Filter() = default;


    /** The accept method of the bypassable filter:
     * All the real computations are occuring in this method.
     * @tparam argsType: template arguments depending on the filter
     * @param args: values to be tested by the filters
     * @return boolean indicating if filter is passed
     */
    template<typename ... argsType>
    /** this typename expands to bool only if all the argsType are
     of the same type of the argument of the filter */
    typename std::enable_if<all_same<argumentType, argsType ...>::value, bool>::type
    accept(const argsType& ... args) const
    {
      typename Variable::variableType value = Variable::value(args ...);
      Observer::notify(Variable(), value, Filter<Variable, RangeType, Observer>::m_range, args ...);
      return (*m_bypass) || Filter<Variable, RangeType, Observer>::m_range.contains(value);
    }


    /** Getter for name of and reference to the range of the filters.
     * Calls getNameAndReference of RangeType.
     * @param pointers: pointer to vector of pairs of names of and pointers to the filters
     * @return string containing name of the variable and the bounds of the range (see RangeTypes)
     */
    std::string getNameAndReference(std::vector<std::pair<char, void*>>* pointers = nullptr)
    {
      return "(" + std::to_string(*m_bypass) + " OR " +
             Filter<Variable, RangeType, Observer>::m_range.getNameAndReference(pointers, Variable::name()) + ")";
    }

  private:
    /// Member pointer to boolean indicating if filters are bypassed (true) or thier actual result is returned (false).
    const bool* m_bypass;
  };



  /// Activatable Filter ///
  /// TODO: Remove, as it is no longer used...?
  /** Extended implementation of the Filter tools, adding a boolean to activate the filter.
   * An external bool variable enables or disables the Filter.
   * E.g.:
      \code

       #include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>
       #include <tracking/trackFindingVXD/filterMap/filterFramework/UpperBoundedSet.h>
       #include <tracking/trackFindingVXD/filterMap/filterFramework/Filter.h>

       struct spacePoint{ float x; float y; float z; };

       SquaredDistance2D: public SelectionVariable<TSpacePoint> {
         value(const TSpacePoint & p1, const TSpacePoint &p2){
           return pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2);
         }
       };
       bool enable(false);
       Filter<Distance3D, UpperBoundedSet, ActivatableFilter, VoidObserver>
                    filter(UpperBoundedSet( 1.1 ), enable);

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
   * The RangeType object will provide the method contains to decide if x(arg1, arg2) is good
   * The Observer will be notified of the actions via its static method notify
   */
  template <
    class Variable,
    class RangeType,
    class Observer
    >
  class Filter <Variable, RangeType, Belle2::ActivatableFilter, Observer>:
    public Filter<Variable, RangeType, Observer> {
  public:
    /** Handy typedef for arguments */
    typedef typename Variable::argumentType argumentType;
    /** Handy typedef for function */
    typedef typename Variable::functionType functionType;


    /** Constructor.
     * To construct the Filter we need a concrete RangeType and a reference to a bool value.
     * The variable and the observer are passed through the template type pack.
     *
     * @param range: a class that provides a method contain such as
        \code bool contains(Variable::returnType x) \endcode
     * @param enable is a bool value controlling the behaviour of accept: if enable is set to
     * true the filter is active and the actual result of the range check accept will be returned,
     * if set to false the filter is inaactive ant the method accept will return always true.
     */
    Filter(const RangeType& range, const bool& enable):
      Filter<Variable, RangeType, Observer>(range)
    {
      m_enable = & enable ;
    };


    /** Empty constructor */
    Filter() = default;

    /** The accept method of the activatable filter:
     * All the real computations are occuring in this method.
     * @tparam argsType: template arguments depending on the filter
     * @param args: values to be tested by the filters
     * @return boolean indicating if filter is passed
     */
    template<typename ... argsType>
    typename std::enable_if<all_same<argumentType, argsType ...>::value, bool>::type
    accept(const argsType& ... args) const
    {
      typename Variable::variableType value = Variable::value(args ...);
      Observer::notify(Variable(), value, Filter< Variable, RangeType, Observer >::m_range,
                       args ...);
      return (!(*m_enable)) || Filter<Variable, RangeType, Observer>::m_range.contains(value);
    }


    /** Getter for name of and reference to the range of the filters.
     * Calls getNameAndReference of RangeType.
     * @param pointers: pointer to vector of pairs of names of and pointers to the filters
     * @return string containing name of the variable and the bounds of the range (see RangeTypes)
     */
    std::string getNameAndReference(std::vector< std::pair<char, void*>>* pointers = nullptr)
    {
      return "(!(" + std::to_string(*m_enable) + ") OR "
             + Filter<Variable, RangeType, Observer>::m_range.getNameAndReference(pointers, Variable::name()) + ")";
    }

  private:
    /// Member pointer to boolean indicating if filters are active (true) or inactive (false).
    const bool* m_enable;
  };



  /// NOT Operator for a Filter ///
  /// Tag class for NOT operator to identify templates
  class OperatorNot;

  /** Realization of a NOT operator for the Filter classes
   * @tparam someFilter : filter type to which the not operator shall be applied
   * @tparam templateObserverType : observer type of the filter to pass it along
   */
  template <
    class someFilter,
    class templateObserverType
    >
  class Filter <Belle2::OperatorNot, someFilter, templateObserverType> {
    /// Char suffix to be used in the filter name for ROOT to indicate that NOT operator is attached.
    const char* c_notSuffix = "_not";
  public:
    /** Handy typedef for arguments */
    typedef typename someFilter::argumentType argumentType;
    /** Handy typedef for function */
    typedef typename someFilter::functionType functionType;


    /** Constructor creating a NOT filter from a filter
     * @param filter : filter to be copied
     */
    explicit Filter(const someFilter& filter):
      m_filter(filter) { };


    /** Empty constructor */
    Filter() { };


    /** The accept method of the filter with NOT Operator:
     * This will return the inverse of the result of the accept method.
     * @tparam argsType: template arguments depending on the filter
     * @param args: values to be tested by the filters
     * @return boolean indicating if filter is NOT passed
     */
    template<typename ... argsType>
    typename std::enable_if<all_same<argumentType, argsType ...>::value, bool>::type
    accept(const argsType& ... args) const
    {
      return ! m_filter.accept(args ...);
    }


    /** Getter for name of and reference to the range of the filters.
     * Calls getNameAndReference of RangeType.
     * @param pointers: pointer to vector of pairs of names of and pointers to the filters
     * @return string containing name of the variable and the bounds of the range (see RangeTypes)
     */
    std::string getNameAndReference(std::vector< std::pair<char, void*>>* pointers = nullptr)
    {
      return "!" + m_filter.getNameAndReference(pointers);
    }


    /** Will set the observer for the combination of NOT operator + filter.
     * @tparam otherObserver : the new observer
     * @return the filter with the new observer
     */
    template<class otherObserver>
    Filter<Belle2::OperatorNot, decltype(someFilter().observeLeaf(otherObserver())), otherObserver>
    observeLeaf(const otherObserver&) const
    {
      return Filter<Belle2::OperatorNot, decltype(someFilter().observeLeaf(otherObserver())),
             otherObserver>(m_filter.observeLeaf(otherObserver()));
    }


    /** Persist the filter on a TTree.
     * @param t is the TTree under which the TBranch will be created
     * @param branchname is the name of the TBranch holding m_range
     */
    void persist(TTree* t, const std::string& branchName)
    {
      std::string nameOfFilter(branchName);
      nameOfFilter += c_notSuffix;
      m_filter.persist(t, nameOfFilter);
    }


    /** Set the Branches addresses to this filter.
     * @param t is the TTree containing the TBranch
     * @param branchname is the name of the TBranch holding the m_range
     */
    void setBranchAddress(TTree* t, const std::string& branchName)
    {
      std::string nameOfFilter(branchName);
      nameOfFilter += c_notSuffix;
      m_filter.setBranchAddress(t, nameOfFilter);
    }

  private:
    /// Member variable containing the filter
    someFilter m_filter;
  };


  /** Definition of the NOT operator ! for the Filter class
   * @tparam types : template pack defining the filter
   * @param filter : filter to which the NOT operator shall be added
   * @return !filter, so a filter with NOT operator attached
   */
  template<typename ... types>
  Filter<OperatorNot, Filter<types...>, VoidObserver>
  operator !(const Filter<types...>& filter)
  {
    return Filter<OperatorNot, Filter<types...>, VoidObserver>(filter);
  }



  /// AND Operator for two Filters ///
  /// Tag class for AND operator to identify templates
  class OperatorAnd;

  /** Realization of the AND operator between two objects of the Filter class.
   * @tparam FilterA : a filter
   * @tparam FilterB : another filter
   * @tparam templateObserverType : the observer type to be used
   */
  template <
    class FilterA,
    class FilterB,
    class templateObserverType
    >
  class Filter <Belle2::OperatorAnd, FilterA, FilterB, templateObserverType> {
    /// Char suffix to be used in the filter name for ROOT to indicate the filter A of the combination A AND B.
    const char* c_andSuffixA = "_and_A";
    /// Char suffix to be used in the filter name for ROOT to indicate the filter B of the combination A AND B.
    const char* c_andSuffixB = "_and_B";
  public:
    /** Handy typedef for arguments type A*/
    typedef typename FilterA::argumentType argumentType;
    /** Handy typedef for arguments type B */
    typedef typename FilterB::argumentType argumentTypeB;
    /** Handy typedef for function type A */
    typedef typename FilterA::functionType functionTypeA;
    /** Handy typedef for function type B */
    typedef typename FilterB::functionType functionTypeB;
    /** Handy typedef for combined function type */
    typedef typename std::enable_if<all_same<functionTypeA, functionTypeB>::value, functionTypeA>::type functionType;


    /** Constructor creating a filter representing the boolean AND combination of two filters.
     * @param filterA : a filter
     * @param filterB : another filter
     */
    Filter(const FilterA& filterA, const FilterB& filterB):
      m_filterA(filterA), m_filterB(filterB) { };


    /** Empty constructor */
    Filter() { };


    /** The accept method for the combination of two filters with the AND Operator:
     * This will return the combined result of the accept method for the two filters
     * using the AND logic.
     * @tparam argsType: template arguments depending on the filters
     * @param args: values to be tested by the filters
     * @return boolean indicating if both filters are passed
     */
    template<typename ... argsType>
    typename std::enable_if<all_same<argumentType, argumentTypeB, argsType ...>::value, bool>::type
    accept(const argsType& ... args) const
    {
      templateObserverType::prepare(args ...);
      bool returnValue =  m_filterA.accept(args ...) && m_filterB.accept(args ...);
      templateObserverType::collect(args ...);
      return returnValue;
    }


    /** Will set the observer for this and both of the via the AND operation combined filters it contains.
     * Only the top level AND filter will have the new observer,
     * all subsequent Boolean filters will NOT be observed (VoidObserver).
     * @tparam : otherObserver
     * @return : filter with the new observer
     */
    template<class otherObserver>
    Filter<Belle2::OperatorAnd, decltype(FilterA().observeLeaf(otherObserver())),
           decltype(FilterB().observeLeaf(otherObserver())), otherObserver>
           observe(const otherObserver&) const
    {
      // this will recursively loop over all AND Filters and set the SAME observer
      return Filter<Belle2::OperatorAnd,
             decltype(FilterA().observeLeaf(otherObserver())),
             decltype(FilterB().observeLeaf(otherObserver())),
             otherObserver>(m_filterA.observeLeaf(otherObserver()), m_filterB.observeLeaf(otherObserver()));
    }


    /** will set the observer for the Leaves of this filter, this AND filter will not be observed.
     * NOTE: if this function is used on the top level Boolean filter,
     * the whole filter will NOT be observed! See accept function!
     * @tparam otherObserver : the new observer
     * @return filter with new observer
     */
    template<class otherObserver>
    Filter<Belle2::OperatorAnd, decltype(FilterA().observeLeaf(otherObserver())),
           decltype(FilterB().observeLeaf(otherObserver())), VoidObserver>
           observeLeaf(const otherObserver&) const
    {
      // This will recursively loop over all lower level Filters of the AND combination and set the SAME observer.
      // For the top level filter the void observer is used.
      return Filter<Belle2::OperatorAnd,
             decltype(FilterA().observeLeaf(otherObserver())),
             decltype(FilterB().observeLeaf(otherObserver())),
             VoidObserver>(m_filterA.observeLeaf(otherObserver()), m_filterB.observeLeaf(otherObserver()));
    }


    /** Persist the filter on a TTree.
     * @param t is the TTree under which the TBranch will be created
     * @param branchname is the name of the TBranch holding m_range
     */
    void persist(TTree* t, const std::string& branchName)
    {
      std::string nameOfFilterA(branchName);
      nameOfFilterA += c_andSuffixA;
      m_filterA.persist(t, nameOfFilterA);

      std::string nameOfFilterB(branchName);
      nameOfFilterB += c_andSuffixB;
      m_filterB.persist(t, nameOfFilterB);
    }


    /** Set the Branches addresses to this filter.
     * @param t is the TTree containing the TBranch
     * @param branchname is the name of the TBranch holding the m_range
     */
    void setBranchAddress(TTree* t, const std::string& branchName)
    {
      std::string nameOfFilterA(branchName);
      nameOfFilterA += c_andSuffixA;
      m_filterA.setBranchAddress(t, nameOfFilterA);

      std::string nameOfFilterB(branchName);
      nameOfFilterB += c_andSuffixB;
      m_filterB.setBranchAddress(t, nameOfFilterB);
    }


    /** Getter for name of and reference to the range of the filters.
     * Calls getNameAndReference of RangeType.
     * @param pointers: pointer to vector of pairs of names of and pointers to the filters
     * @return string containing name of the AND-combined variable and the bounds of the range (see RangeTypes)
     */
    std::string getNameAndReference(std::vector< std::pair<char, void*>>* pointers = nullptr)
    {
      return "(" + m_filterA.getNameAndReference(pointers) + " AND " + m_filterB.getNameAndReference(pointers) + ")";
    }

  private:
    /// Member containing the filter A of the combination A AND B
    FilterA m_filterA;
    /// Member containing the filter B of the combination A AND B
    FilterB m_filterB;
  };


  /** Definition of the boolean AND operator && of the Filter class
   * @tparam types1 : template pack of filter A
   * @tparam types2 : template pack of filter B
   * @param filter1 : filter A
   * @param filter2 : filter B
   * @return Boolean AND combination of the two filters
   */
  template <
    typename ... types1,
    typename ... types2
    >
  Filter<Belle2::OperatorAnd, Belle2::Filter<types1...>, Belle2::Filter<types2...>, Belle2::VoidObserver>
  operator &&(const Filter<types1...>& filter1, const Filter<types2...>& filter2)
  {
    return Filter<OperatorAnd, Filter<types1...>, Filter<types2...>, VoidObserver> (filter1, filter2);
  }



  /// OR Operator for two Filters ///
  /// Tag class for OR operator to identify templates
  class OperatorOr;

  /** Realization of the OR operator between two objects of the Filter class.
   * @tparam FilterA : a filter
   * @tparam FilterB : another filter
   * @tparam templateObserverType : the observer type to be used
   */
  template <
    class FilterA,
    class FilterB,
    class templateObserverType
    >
  // cppcheck-suppress copyCtorAndEqOperator
  class Filter <Belle2::OperatorOr, FilterA, FilterB, templateObserverType > {
    /// Char suffix to be used in the filter name for ROOT to indicate the filter A of the combination A OR B.
    const char* c_orSuffixA = "_or_A";
    /// Char suffix to be used in the filter name for ROOT to indicate the filter B of the combination A OR B.
    const char* c_orSuffixB = "_or_B";

  public:
    /** Handy typedef for arguments type A */
    typedef typename FilterA::argumentType argumentType;
    /** Handy typedef for arguments type B */
    typedef typename FilterB::argumentType argumentTypeB;
    /** Handy typedef for function type A */
    typedef typename FilterA::functionType functionTypeA;
    /** Handy typedef for function type B */
    typedef typename FilterB::functionType functionTypeB;
    /** Handy typedef for the combined function type */
    typedef typename std::enable_if<all_same<functionTypeA, functionTypeB>::value, functionTypeA>::type functionType;


    /** Constructor creating a filter representing the boolean OR combination of two filters.
     * @param filterA : a filter
     * @param filterB : another filter
     */
    Filter(const FilterA& filterA, const FilterB& filterB):
      m_filterA(filterA), m_filterB(filterB) { };


    /** Empty constructor */
    Filter() { };


    /** The accept method for the combination of two filters with the OR Operator:
     * This will return the combined result of the accept method for the two filters
     * using the OR logic.
     * @tparam argsType: template arguments depending on the filters
     * @param args: values to be tested by the filters
     * @return boolean indicating if one of the filters is passed
     */
    template<typename ... argsType>
    typename std::enable_if <all_same<argumentType, argumentTypeB, argsType ...>::value, bool>::type
    accept(const argsType& ... args) const
    {
      templateObserverType::prepare(args ...);
      bool returnValue =  m_filterA.accept(args ...) || m_filterB.accept(args ...);
      templateObserverType::collect(args ...);
      return returnValue;
    }


    /** Will set the observer for this and both via the OR operator combined filters it contains,
     * all subsequent Boolean Filters will not be observed (VoidObserver)
     * @tparam otherObserver : the new type observer to be used
     * @return filter with the new observer
     */
    template<class otherObserver>
    Filter<Belle2::OperatorOr, decltype(FilterA().observeLeaf(otherObserver())),
           decltype(FilterB().observeLeaf(otherObserver())), otherObserver>
           observe(const otherObserver&) const
    {
      // this will recursively loop over all "and" Filters and set the SAME observer
      return Filter<Belle2::OperatorOr,
             decltype(FilterA().observeLeaf(otherObserver())),
             decltype(FilterB().observeLeaf(otherObserver())),
             otherObserver>(m_filterA.observeLeaf(otherObserver()), m_filterB.observeLeaf(otherObserver()));
    }


    /** Will set the observer for both via the OR operation combined filters it contains,
     * this filter itself will not be observed (VoidObserver).
     * NOTE: If this function is used on the top level Boolean filter,
     * the whole filter will NOT be observed! See accept function!
     * @tparam otherObserver : the new observer type to be used
     * @return filter with the new observer
     */
    template<class otherObserver>
    Filter<Belle2::OperatorOr, decltype(FilterA().observeLeaf(otherObserver())),
           decltype(FilterB().observeLeaf(otherObserver())), VoidObserver>
           observeLeaf(const otherObserver&) const
    {
      // This will recursively loop over all lower level Filters of the AND combination and set the SAME observer.
      // For the top level filter the void observer is used.
      return Filter<Belle2::OperatorOr,
             decltype(FilterA().observeLeaf(otherObserver())),
             decltype(FilterB().observeLeaf(otherObserver())),
             VoidObserver>(m_filterA.observeLeaf(otherObserver()), m_filterB.observeLeaf(otherObserver()));
    }


    /** Persist the filter on a TTree.
     * @param t is the TTree under which the TBranch will be created
     * @param branchname is the name of the TBranch holding m_range
     */
    void persist(TTree* t, const std::string& branchName)
    {
      std::string nameOfFilterA(branchName);
      nameOfFilterA += c_orSuffixA;
      m_filterA.persist(t, nameOfFilterA);

      std::string nameOfFilterB(branchName);
      nameOfFilterB += c_orSuffixB;
      m_filterB.persist(t, nameOfFilterB);
    }


    /** Set the Branches addresses to this filter.
     * @param t is the TTree containing the TBranch
     * @param branchname is the name of the TBranch holding the m_range
     */
    void setBranchAddress(TTree* t, const std::string& branchName)
    {
      std::string nameOfFilterA(branchName);
      nameOfFilterA += c_orSuffixA;
      m_filterA.setBranchAddress(t, nameOfFilterA);

      std::string nameOfFilterB(branchName);
      nameOfFilterB += c_orSuffixB;
      m_filterB.setBranchAddress(t, nameOfFilterB);
    }


    /** Getter for name of and reference to the range of the filters.
     * Calls getNameAndReference of RangeType.
     * @param pointers: pointer to vector of pairs of names of and pointers to the filters
     * @return string containing name of the OR-combined variable and the bounds of the range (see RangeTypes)
     */
    std::string getNameAndReference(std::vector< std::pair<char, void*>>* pointers = nullptr)
    {
      return "(" + m_filterA.getNameAndReference(pointers) + " OR " + m_filterB.getNameAndReference(pointers) + ")";
    }

  private:
    /// Member containing the filter A of the combination A OR B
    FilterA  m_filterA;
    /// Member containing the filter B of the combination A OR B
    FilterB  m_filterB;
  };


  /** Definition of the boolean OR operator || of the Filter class
   * @tparam types1 : template pack of filter A
   * @tparam types2 : template pack of filter B
   * @param filter1 : filter A
   * @param filter2 : filter B
   * @return Boolean OR combination of the two filters
   */
  template <
    typename ... types1,
    typename ... types2
    >
  Filter<Belle2::OperatorOr, Belle2::Filter<types1...>, Belle2::Filter<types2...> , Belle2::VoidObserver>
  operator ||(const Filter<types1...>& filter1 , const Filter<types2...>& filter2)
  {
    return Filter<OperatorOr, Filter<types1...>, Filter<types2...>, VoidObserver> (filter1, filter2);
  }



  /// Observer Stuff ///
  /** Recursive function to initialize all the observers in a binary boolean Filter.
   * @tparam booleanBinaryOperator : Tag Class to identify the Filter operator
   * @tparam types1 : template pack of a filter
   * @tparam types2 : template pack of another filter
   * @tparam observer : observer type to be used
   * @tparam argsTypes : argument types of the filter
   * @param args : arguments of the filter
   * @return boolean to indicate if the observer has been initialized
   */
  template<class booleanBinaryOperator,
           typename ... types1,
           typename ... types2,
           class observer,
           typename ... argsTypes>
  bool initializeObservers(const Filter<booleanBinaryOperator, Belle2::Filter<types1...>,
                           Belle2::Filter<types2...> , observer>&,
                           argsTypes ... args)
  {
    return observer::initialize(args ...)
           && initializeObservers(Belle2::Filter<types1...>(), args...)
           && initializeObservers(Belle2::Filter<types2...>(), args ...);
  }


  /** Recursive function to initialize all the observers in a unary boolean Filter.
   * @tparam booleanUnaryOperator : Tag Class to identify the Filter operator
   * @tparam types1 : template pack of the filter
   * @tparam observer : observer type to be used
   * @tparam argsTypes : argument types of the filter
   * @param args : arguments of the filter
   * @return boolean to indicate if the observer has been initialized
   */
  template<class booleanUnaryOperator,
           typename ... types1,
           class observer,
           typename ... argsTypes>
  bool initializeObservers(const Filter<booleanUnaryOperator, Belle2::Filter<types1...>, observer>&, argsTypes ... args)
  {
    return observer::initialize(args ...) && initializeObservers(Belle2::Filter<types1...>(), args...);
  }


  /** Initialize the observer of a RangeType Filter.
   * @tparam Variable : Filter variable type
   * @tparam RangeType : range type
   * @tparam observer : observer type to be used
   * @tparam argsTypes : argument types of the filter
   * @param filter : Filter for which the observer shall be initialized
   * @param args : arguments of the filter
   * @return boolean to indicate if the observer has been initialized
   */
  template<class Variable, class RangeType, class observer, typename ... argsTypes>
  bool initializeObservers(const Belle2::Filter<Variable, RangeType, observer>& filter, argsTypes ... args)
  {
    return observer::initialize(Variable(), filter.getRange(), args ...);
  }
}
