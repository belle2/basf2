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
      /// Constructor from another range
      template<class Ts>
      Range(Ts& ts) : Super(std::begin(ts), std::end(ts)) {}

      /// Inheriting the constructor of the base class
      using Super::Super;

      /// The type the iterator references
      typedef typename std::iterator_traits<AIterator>::reference ReferenceType;

      /// Begin of the range for range based for.
      const AIterator& begin() const
      { return this->first; }

      /// End of the range for range based for.
      const AIterator& end() const
      { return this->second; }

      /// Checks if the begin equals the end iterator, hence if the range is empty.
      bool empty() const
      { return begin() == end(); }

      /// Returns the total number of objects in this range
      size_t size() const
      { return std::distance(begin(), end()); }

      /// Returns the derefenced begin iterator.
      ReferenceType front() const
      { return *(begin()); }

      /// Returns the derefenced end iterator
      ReferenceType back() const
      { return *(end()); }

      /// Returns the object at index i
      ReferenceType operator[](size_t pos) const
      { return *std::advance(AIterator(begin()), pos); }

    };
  }
}
