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

#include <iterator>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Generic identity functor.
    struct GetMe {
      /// Operator that just returns the object itself - gets me.
      template <class T>
      const T& operator()(const T& t) const
      {
        return t;
      }
    };

    /// Generic dereferencing functor
    template <class AGet = GetMe>
    struct Deref : public AGet {
      /// Operator that just returns the deferenced object.
      template <class T>
      auto operator()(const T& t) -> decltype(AGet()(*t)) const
      {
        AGet get;
        return get(*t);
      }
    };

    /// Generic functor to perform arrow lookup
    template <class AGet = GetMe>
    struct Arrow : public AGet {
      /// Operator that performs the arrow lookup
      template <class T>
      auto operator()(const T& t) -> decltype(AGet()(*t.operator->())) const
      {
        AGet get;
        return get(*t.operator->());
      }

      /// Simple deref implementation for pointers as they do not defined the arrow
      template <class T>
      auto operator()(const T* t) -> decltype(AGet()(*t)) const
      {
        AGet get;
        return get(*t);
      }
    };

    /// Generic functor to perform lookup applying an arrow if necessary
    template <class AGet>
    struct MayBeArrow : public AGet {

    private:
      /// Type of this class
      using This = MayBeArrow<AGet>;

      /// Returns the got value of an object. Disfavoured option.
      template <class T>
      static auto impl(const T& t,
                       int favouredTag __attribute__((unused)))
      -> decltype(AGet()(t))
      {
        AGet get;
        return get(t);
      }

      /// Returns the got value of an object with indirection. Disfavoured option.
      template <class T>
      static auto impl(const T& t,
                       long disfavouredTag __attribute__((unused)))
      -> decltype(Arrow<AGet>()(t))
      {
        Arrow<AGet> get;
        assert(&*t != nullptr);
        return get(t);
      }

    public:
      /// Operator that just returns the object itself - gets me.
      template <class T>
      auto operator()(const T& t)
      -> decltype(This::impl(t, 0))
      {
        int dispatchTag = 0;
        return This::impl(t, dispatchTag);
      }
    };

    /// Generic equal binary functor which can be narrowed down with a getter
    template <class AGet = GetMe>
    struct Equal {
      /// Generic operator comparing two objects
      template <class T1, class T2>
      bool operator()(const T1& t1, const T2& t2) const
      {
        AGet get;
        return get(t1) == get(t2);
      }
    };

    /// Generic unequal binary functor which can be narrowed down with a getter
    template <class AGet = GetMe>
    struct Unequal {
      /// Generic operator comparing two objects
      template <class T1, class T2>
      bool operator()(const T1& t1, const T2& t2) const
      {
        AGet get;
        return get(t1) != get(t2);
      }
    };

    /// Generic less compare binary functor which can be narrowed down with a getter
    template <class AGet = GetMe>
    struct Less {
      /// Generic operator comparing two objects
      template <class T1, class T2>
      bool operator()(const T1& t1, const T2& t2) const
      {
        AGet get;
        return get(t1) < get(t2);
      }
    };

    /// Generic greater compare binary functor which can be narrowed down with a getter
    template <class AGet = GetMe>
    struct Greater {
      /// Generic operator comparing two objects
      template <class T1, class T2>
      bool operator()(const T1& t1, const T2& t2) const
      {
        AGet get;
        return get(t1) > get(t2);
      }
    };

    /// Adapter of a category function to find the common category of several objects
    template <class ACategoryFunction>
    struct Common {

      /// Return value for invalid operations
      static const auto c_Invalid = ACategoryFunction::c_Invalid;

      /// Returned type
      using Category = decltype(c_Invalid);

      /**
       *  Returns the common category value of a range.
       *
       *  In case the range is empty or the category value differ between the elements
       *  it returns the c_Invalid category value.
       */
      template <class Ts>
      Category operator()(const Ts& ts) const
      {
        auto itBegin = std::begin(ts);
        auto itEnd = std::end(ts);
        if (itBegin == itEnd) return c_Invalid; // empty case
        ACategoryFunction catFunc;
        const Category cat = catFunc(*itBegin);
        for (auto it = itBegin; it != itEnd; ++it) {
          if (cat != catFunc(*it)) return c_Invalid;
        }
        return cat;
      }

      /**
       *  Returns the common category value of two values.
       *
       *  In case the category value differ between the elements
       *  it returns the c_Invalid category value.
       */
      template<class T1, class T2>
      Category operator()(const T1& t1, const T2& t2) const
      {
        ACategoryFunction catFunc;
        Category cat1 = catFunc(t1);
        Category cat2 = catFunc(t2);
        if (cat1 == cat2) return cat1;
        return c_Invalid;
      }
    };
  }
}
