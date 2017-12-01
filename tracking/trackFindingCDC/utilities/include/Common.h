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

#include <boost/optional.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Adapter of a category function to find the common category of several objects
    template <class AFunctor>
    struct Common {

    private:
      /// Memory for the nested functor
      AFunctor m_functor;

    public:
      /**
       *  Returns the common category value of a range.
       *
       *  In case the category value differ between the values return empty optional.
       */
      template <class Ts, class Category = decltype(m_functor(*std::declval<Ts>().begin()))>
      boost::optional<Category> operator()(const Ts& ts) const
      {
        auto it = std::begin(ts);
        auto itEnd = std::end(ts);
        if (it == itEnd) return {}; // empty case
        Category category = m_functor(*it);
        for (; it != itEnd; ++it) {
          if (category != m_functor(*it)) return {};
        }
        return {category};
      }

      /**
       *  Returns the common category of two values.
       *
       *  In case the category value differ between the values return empty optional
       */
      template<class T1, class T2, class Category = decltype(m_functor(std::declval<T1>()))>
      boost::optional<Category> operator()(const T1& t1, const T2& t2) const
      {
        Category cat1 = m_functor(t1);
        Category cat2 = m_functor(t2);
        if (cat1 == cat2) return {cat1};
        return {};
      }
    };
  }
}
