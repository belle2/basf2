/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
