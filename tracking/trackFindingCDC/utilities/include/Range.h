/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Oliver Frost                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/GetIterator.h>

#include <algorithm>
#include <iterator>
#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A pair of iterators usable with the range base for loop.
    template<class AIterator>
    class Range : public std::pair<AIterator, AIterator> {

    private:
      /// Type of the base class
      using Super = std::pair<AIterator, AIterator>;

    public:
      /// Iterator type of the range
      using Iterator = AIterator;

      /// Iterator definition for stl
      using iterator = Iterator;

      /// The type the iterator references
      using Reference = typename std::iterator_traits<AIterator>::reference;

      /// The type behind the iterator (make it possible to use the range as a "list")
      using value_type = typename std::iterator_traits<AIterator>::value_type;

    public:
      /// Default constructor for ROOT
      Range() = default;

      /// Constructor to adapt a pair as returned by e.g. std::equal_range.
      template<class AOtherIterator>
      explicit Range(const std::pair<AOtherIterator, AOtherIterator>& itPair)
        : Super(AIterator(itPair.first), AIterator(itPair.second))
      {}

      /// Constructor from another range
      template<class Ts, class ItT = GetIterator<Ts>>
      explicit Range(const Ts& ts)
        : Super(AIterator(std::begin(ts)), AIterator(std::end(ts)))
      {}

      /// Inheriting the constructor of the base class
      using Super::Super;

      /// Begin of the range for range based for.
      Iterator begin() const
      { return this->first; }

      /// End of the range for range based for.
      Iterator end() const
      { return this->second; }

      /// Checks if the begin equals the end iterator, hence if the range is empty.
      bool empty() const
      { return begin() == end(); }

      /// Returns the total number of objects in this range
      std::size_t size() const
      { return std::distance(begin(), end()); }

      /// Returns the derefenced iterator at begin()
      Reference front() const
      { return *(begin()); }

      /// Returns the derefenced iterator before end()
      Reference back() const
      { return *(end() - 1); }

      /// Returns the object at index i
      Reference operator[](std::size_t i) const
      { return *(begin() + i); }

      /// Returns the object at index i
      Reference at(std::size_t i) const
      {
        if (not(i < size())) {
          throw std::out_of_range("Range : Requested index " + std::to_string(i) + " is out of bounds.");
        }
        return operator[](i);
      }

      /// Counts the number of equivalent items in the range
      bool count(Reference t)
      { return std::count(this->begin(), this->end(), t); }

    };

    /// Adapter function to make pair of iterators (e.g. from equal_range) into a range usable in range base for loops.
    template<class AIterator>
    Range<AIterator> asRange(std::pair<AIterator, AIterator> const& x)
    {
      return Range<AIterator>(x);
    }

    /// Adapter function to make pair of iterators (e.g. from equal_range) into a range usable in range base for loops.
    template<class AIterator>
    Range<AIterator> asRange(AIterator const& itBegin, AIterator const& itEnd)
    {
      return Range<AIterator>(std::make_pair(itBegin, itEnd));
    }
  }
}
