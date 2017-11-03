/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/FunctorTag.h>

#include <type_traits>
#include <utility>
#include <cassert>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Generic identity functor.
    struct Id {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Operator that just returns the object itself - gets me.
      template <class T>
      T&& operator()(T&& t) const
      {
        return std::forward<T>(t);
      }
    };

    /// Generic functor to wrap a constant as a function.
    template<class T>
    struct Constant {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      Constant() = default;

      /// Constructor from the constant value
      Constant(const T& t)
        : m_t(t)
      {
      }

      /// Operator returning the constant
      template <class S>
      constexpr const T& operator()(const S&) const
      {
        return m_t;
      }

    private:
      /// Memory for the constant to be returned
      T m_t{};
    };



    /// Function coercing a scalar value to a constant functor
    template<class T, class SFINAE = std::enable_if_t<not isFunctor<T>()> >
    constexpr Constant<T> toFunctor(const T& t)
    {
      return Constant<T> {t};
    }

    /// Alternative coercion from a functor yields the functor itself
    template<class AFunctor, class SFINAE = std::enable_if_t<isFunctor<AFunctor>()> >
    constexpr const AFunctor & toFunctor(const AFunctor& get)
    {
      return get;
    }

    /// Type function computing the result type of the toFunctor coercion
    template <class T>
    using ToFunctor = typename std::decay<decltype(toFunctor(std::declval<T>()))>::type;



    /// Functor factory from the functional composition of two functors
    template <class AFunctor1, class AFunctor2>
    struct Composition {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Allow default construction
      Composition() = default;

      /// Constructor from the nested functor
      explicit Composition(const AFunctor2& functor2)
        : m_functor1()
        , m_functor2(functor2)
      {
      }

      /// Constructor from the first and the nested functor
      Composition(const AFunctor1& functor1, const AFunctor2& functor2)
        : m_functor1(functor1)
        , m_functor2(functor2)
      {
      }

    private: // Members here for lookup reasons
      /// Memory for the nested functor
      AFunctor1 m_functor1;

      /// Memory for the nested functor
      AFunctor2 m_functor2;

    public:
      /// Operator getting the result of the function composition
      template<class T>
      auto operator()(const T& t) const -> decltype(m_functor1(m_functor2(t)))
      {
        return m_functor1(m_functor2(t));
      }
    };

    /// Functor factory turning a binary functor and two functors into a new functor
    /// which executes the binary operation with input from the two given functors
    template <class ABinaryOp, class AFunctor1, class AFunctor2 = AFunctor1>
    struct BinaryJoin : public ABinaryOp {
    public:
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Allow default construction
      BinaryJoin() = default;

      /// Constructor from the nested functors
      BinaryJoin(const ABinaryOp& binaryOp, const AFunctor1& functor1, const AFunctor2& functor2)
        : m_binaryOp(binaryOp)
        , m_functor1(functor1)
        , m_functor2(functor2)
      {
      }


      /// Constructor from the nested functors
      BinaryJoin(const AFunctor1& functor1, const AFunctor2& functor2)
        : m_binaryOp()
        , m_functor1(functor1)
        , m_functor2(functor2)
      {
      }

    private: // Members here for lookup reasons
      /// Memory for the binary operation
      ABinaryOp m_binaryOp;

      /// Memory for the first nested functor
      AFunctor1 m_functor1;

      /// Memory for the second nested functor
      AFunctor2 m_functor2;

    public:
      /// Operator getting the result of the binary operation from two objects transformed by the two functors
      template <class T1, class T2>
      auto operator()(const T1& t1, const T2& t2) const -> decltype(m_binaryOp(m_functor1(t1), m_functor2(t2)))
      {
        return m_binaryOp(m_functor1(t1), m_functor2(t2));
      }

      /// Operator getting the result of the binary operation from one object transformed by the two functors
      template <class T>
      auto operator()(const T& t) const -> decltype(m_binaryOp(m_functor1(t), m_functor2(t)))
      {
        return m_binaryOp(m_functor1(t), m_functor2(t));
      }
    };

    /// Generic functor to try to functors and choose the first to be applicable
    template <class AFunctor1, class AFunctor2>
    struct Alternation {

    public:
      /// Marker function for the isFunctor test
      operator FunctorTag();

    private: // Members first for lookup reasons
      /// Memory for the first nested functor
      AFunctor1 m_functor1;

      /// Memory for the second nested functor
      AFunctor2 m_functor2;

    public:
      /// Allow default construction
      Alternation() = default;

      /// Constructor from the nested functors
      Alternation(const AFunctor1& functor1, const AFunctor2& functor2 = AFunctor2())
        : m_functor1(functor1)
        , m_functor2(functor2)
      {
      }

    public:
      /// Implementation applying the first functor. Favoured option.
      template <class... T>
      auto impl(int favouredTag __attribute__((unused)), T&& ... t) const
      -> decltype(m_functor1(std::forward<T>(t)...))
      {
        return m_functor1(std::forward<T>(t)...);
      }

      /// Implementation applying the second functor. Disfavoured option.
      template <class... T>
      auto impl(long disfavouredTag __attribute__((unused)), T&& ... t) const
      -> decltype(m_functor2(std::forward<T>(t)...))
      {
        return m_functor2(std::forward<T>(t)...);
      }

    public:
      /// Operator to dispatch to the two functors and returns the first applicable option.
      template <class... T>
      auto operator()(T&& ... t) const -> decltype(this->impl(0, std::forward<T>(t)...))
      {
        int dispatchTag = 0;
        return impl(dispatchTag, std::forward<T>(t)...);
      }
    };


    // ******************** (void)(?) ********************

    /// Functor returning void from an abitrary objects.
    struct Void {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Operator always returning void
      template <class T>
      void operator()(T&& t __attribute__((unused))) const
      {
      }
    };

    /// Meta-functor that discards any return value that the given functor emits
    template <class AFunctor = Id>
    using VoidOf = Composition<Void, AFunctor>;

    /// Meta-functor that calles the given functor in case the call works - otherwise do nothing
    template <class AFunctor>
    using IfApplicable = Alternation<AFunctor, Void>;

    /// Invokes a function with the given arguments if the call is allowed - otherwise do nothing
    template<class AFunction, class... T>
    void invokeIfApplicable(AFunction&& function, T&& ... t)
    {
      IfApplicable<AFunction> invokeIfApplicableImpl(std::forward<AFunction>(function));
      invokeIfApplicableImpl(std::forward<T>(t)...);
    }

    /// Invokes a getter function with the given argument if the call is allowed - otherwise return the default value
    template<class ADefault, class AFunctor, class T>
    ADefault getIfApplicable(AFunctor&& function, T&& obj, ADefault value)
    {
      Alternation<AFunctor, Constant<ADefault> > getIfApplicableImpl{std::forward<AFunctor>(function), value};
      return getIfApplicableImpl(std::forward<T>(obj));
    }


    // ******************** get<I>(?) ********************

    /// Functor to get the I part (as of std::get<I>) from an abitrary objects.
    template<int I>
    struct Get {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Operator getting the I part (as of std::get<I>) of an abitrary object
      template <class T>
      auto operator()(const T& t) const -> decltype(std::get<I>(t))
      {
        using std::get;
        return get<I>(t);
      }
    };

    /// Meta-functor to get the I part (as of std::get<I>) of an object returned from another functor.
    template <int I, class AFunctor = Id>
    using GetOf = Composition<Get<I>, AFunctor>;


    // ******************** ?.first ********************

    /// Functor to get the .first from an abitrary objects.
    /// Implementated with get<0>, could be improved, but this is shorter for now
    using First = Get<0>;

    /// Meta-functor to get the .first of an object returned from another functor.
    template<class AFunctor = Id>
    using FirstOf = GetOf<0, AFunctor>;


    // ******************** ?.second ********************

    /// Functor to get the .second from an abitrary objects.
    /// Implementated with get<1>, could be improved, but this is shorter for now
    using Second = Get<1>;

    /// Meta-functor to get the .second of an object returned from another functor.
    template<class AFunctor = Id>
    using SecondOf = GetOf<1, AFunctor>;


    // ******************** ?.size() ********************

    /// Functor to get the .size() from an abitrary objects.
    struct Size {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Operator getting the .size() of an abitrary object
      template <class T>
      auto operator()(const T& t) const -> decltype(t.size())
      {
        return t.size();
      }
    };

    /// Meta-functor to get the .size() of an abitrary object returned from another functor
    template <class AFunctor = Id>
    using SizeOf = Composition<Size, AFunctor>;


    // ******************** ?.clear() ********************

    /// Functor to get the .clear() from an abitrary objects.
    struct Clear {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Operator getting the .clear() of an abitrary object
      template <class T>
      auto operator()(T& t) const -> decltype(t.clear())
      {
        return t.clear();
      }
    };

    /// Meta-functor to get the .clear() of an abitrary object returned from another functor
    template <class AFunctor = Id>
    using ClearOf = Composition<Clear, AFunctor>;

    /// Functor invoking the clear method of an object if present - otherwise do nothing
    using ClearIfApplicable = IfApplicable<Clear>;

    /// Function invoking the clear method of an object if present - otherwise do nothing
    static const ClearIfApplicable clearIfApplicable{};

    // ******************** not(?) ********************

    /// Functor to get the logical negation from an abitrary objects.
    struct Not {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Operator getting the logical negation of an abitrary object
      template <class T>
      auto operator()(const T& t) const -> decltype(not t)
      {
        return not t;
      }
    };

    /// Meta-functor to get the logical negation of an object returned from another functor.
    template <class AFunctor = Id>
    using NotOf = Composition<Not, AFunctor>;

    /// Operator to construct a logical negated functor.
    template <class AFunctor, class SFINAE = std::enable_if_t<isFunctor<AFunctor>()> >
    NotOf<AFunctor> operator!(const AFunctor& functor)
    {
      return NotOf<AFunctor> {functor};
    }


    // ******************** *? ********************

    /// Functor to get the referenced object from an abitrary objects.
    struct Deref {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Operator getting the referenced object of an abitrary object
      template <class T>
      auto operator()(const T& t) const -> decltype(*t)
      {
        return *t;
      }

      /// Specialisation for pointers to make an assertion that no nullptr is derefernced
      template <class T>
      auto operator()(const T* t) const -> decltype(*t)
      {
        assert(t != nullptr);
        return *t;
      }
    };

    /// Meta-functor from a functor applying a dereference afterwards
    template <class AFunctor = Id>
    using DerefOf = Composition<Deref, AFunctor>;

    /// Meta-functor from a functor applying a dereference before
    template <class AFunctor = Id>
    using DerefTo = Composition<AFunctor, Deref>;

    // ******************** *(?->) ********************

    /// Functor to get the indirection from an abitrary objects.
    struct Indirect {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Operator getting the indirection of an abitrary object
      template <class T>
      auto operator()(const T& t) const -> decltype(*(t.operator->()))
      {
        assert(t.operator->() != nullptr);
        return *(t.operator->());
      }

      /// Specialisation for pointers that indirect to themselves
      template <class T>
      const T& operator()(const T* t) const
      {
        assert(t != nullptr);
        return *t;
      }
    };

    /// Meta-functor from a functor applying an indirection afterwards
    template <class AFunctor = Id>
    using IndirectOf = Composition<Indirect, AFunctor>;

    /// Meta-functor from a functor applying an indirection before
    template <class AFunctor = Id>
    using IndirectTo = Composition<AFunctor, Indirect>;

    /**
     *  Frequently used meta-functor from a functor applying an indirection before if necessary
     *  Considers both t->get() and t.get() to find a value.
     */
    template <class AFunctor>
    using MayIndirectTo = Alternation<IndirectTo<AFunctor>, AFunctor>;
    // using MayIndirectTo = AFunctor;

    // ******************** ? < ? aka less ********************

    /// Binary functor for less comparision of abitrary objects - equivalent to std::less<> (c++14)
    struct Less {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Operator for less comparision of an abitrary object
      template<class T1, class T2>
      auto operator()(const T1& t1, const T2& t2) const -> decltype(t1 < t2)
      {
        return t1 < t2;
      }
    };

    /// Meta-functor for comparision of two objects or two values returned from another functors.
    template <class AFunctor1 = Id, class AFunctor2 = AFunctor1>
    using LessOf = BinaryJoin<Less, AFunctor1, AFunctor2>;

    /// Operator to construct a less comparision functor from two functors or on functor and a constant value.
    template <class ALHS, class ARHS, class SFINAE = std::enable_if_t<isFunctor<ALHS>() or isFunctor<ARHS>()>>
    LessOf<ToFunctor<ALHS>, ToFunctor<ARHS> > operator<(const ALHS& lhs, const ARHS& rhs)
    {
      return {toFunctor(lhs), toFunctor(rhs)};
    }



    // ******************** ? > ? aka greater ********************

    /// Binary functor for greater comparision of abitrary objects - equivalent to std::greater<> (c++14)
    struct Greater {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Operator for greater comparision of an abitrary object
      template<class T1, class T2>
      auto operator()(const T1& t1, const T2& t2) const -> decltype(t1 > t2)
      {
        return t1 > t2;
      }
    };

    /// Meta-functor for comparision of two objects or two values returned from another functors.
    template <class AFunctor1 = Id, class AFunctor2 = AFunctor1>
    using GreaterOf = BinaryJoin<Greater, AFunctor1, AFunctor2>;

    /// Operator to construct a greater comparision functor from two functors or on functor and a constant value.
    template <class ALHS, class ARHS, class SFINAE = std::enable_if_t<isFunctor<ALHS>() or isFunctor<ARHS>()>>
    GreaterOf<ToFunctor<ALHS>, ToFunctor<ARHS> > operator>(const ALHS& lhs, const ARHS& rhs)
    {
      return {toFunctor(lhs), toFunctor(rhs)};
    }

    // ******************** ? == ? aka equal_to ********************

    /// Binary functor for equality comparision of abitrary objects - equivalent to std::equal_to<> (c++14)
    struct Equal {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Operator for equality comparision of an abitrary object
      template<class T1, class T2>
      auto operator()(const T1& t1, const T2& t2) const -> decltype(t1 == t2)
      {
        return t1 == t2;
      }
    };

    /// Meta-functor for comparision of two objects or two values returned from another functors.
    template <class AFunctor1 = Id, class AFunctor2 = AFunctor1>
    using EqualOf = BinaryJoin<Equal, AFunctor1, AFunctor2>;

    /// Operator to construct a equality comparision functor from two functors or on functor and a constant value.
    template <class ALHS, class ARHS, class SFINAE = std::enable_if_t<isFunctor<ALHS>() or isFunctor<ARHS>()>>
    EqualOf<ToFunctor<ALHS>, ToFunctor<ARHS> > operator==(const ALHS& lhs, const ARHS& rhs)
    {
      return {toFunctor(lhs), toFunctor(rhs)};
    }

    /// Unary functor for equality comparison to NAN
    struct IsNaN {
      /// Marker function for the isFunctor test
      operator FunctorTag();

      /// Operator for equality comparision to NaN
      template<class T>
      bool operator()(const T& t) const
      {
        return std::isnan(t);
      }
    };

    // ******************** Other operators are left as exercise ********************

  }
}
