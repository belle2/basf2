/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/Range.h>

#include <algorithm>
#include <iterator>
#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A pair of iterators usable with the range base for loop.
    template<class AIterator>
    class SortedRange : public Range<AIterator> {

    private:
      /// Type of the base class
      using Super = Range<AIterator>;

    public:
      /// Iterator type of the range
      using Iterator = AIterator;

      /// The type the iterator references
      using Reference = typename std::iterator_traits<AIterator>::reference;

    public:
      /// Constructor to adapt a pair as returned by e.g. std::equal_range.
      template<class AOtherIterator>
      explicit SortedRange(const std::pair<AOtherIterator, AOtherIterator>& itPair)
        : Super(AIterator(itPair.first), AIterator(itPair.second))
      {}

      /// Constructor from another range
      template<class Ts, class ItT = GetIterator<Ts>>
      explicit SortedRange(const Ts& ts)
        : Super(AIterator(std::begin(ts)), AIterator(std::end(ts)))
      {}

      /// Inheriting the constructor of the base class
      using Super::Super;

      /// Access to a sub range to mimic the behaviour of a sorted container.
      template<class T>
      SortedRange<AIterator> equal_range(const T& t) const
      { return SortedRange<AIterator>(std::equal_range(this->begin(), this->end(), t)); }

      /// Access to a upper bound to mimic the behaviour of a sorted container.
      template<class T>
      SortedRange<AIterator> upper_bound(const T& t) const
      { return SortedRange<AIterator>(std::upper_bound(this->begin(), this->end(), t)); }

      /// Access to a lower bound to mimic the behaviour of a sorted container.
      template<class T>
      SortedRange<AIterator> lowers_bound(const T& t) const
      { return SortedRange<AIterator>(std::lower_bound(this->begin(), this->end(), t)); }

      /// Counts the number of equivalent items in the range
      bool count(Reference t) const
      {
        Range<AIterator> relevant_range = this->equal_range(t);
        return relevant_range.count(t);
      }

    };
  }
}
