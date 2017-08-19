/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <numeric>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * Special container type to chain instances of the same type of containers together.
     *
     * E.g. one could use it, to chain two vectors together, with
     *
     *      std::vector<int> a = {1, 2, 3};
     *      std::vector<int> b = {4, 5, 6};
     *      ChainedArray<std::vector<int>> chain({a, b});
     *
     * When looping over the chain, all the elements of a and b are processed one ofter the other, so
     *
     *      for(const auto& i : chain) {
     *          std::cout << i << " ";
     *      }
     *
     * Would produce 1 2 3 4 5 6.
     */
    template <class ArrayType>
    class ChainedArray {
      /// Make it possible to use the stl with this
      using value_type = typename ArrayType::value_type;
      /// The type of the size to return
      using size_type = std::size_t;
    private:
      /// A helper class to be used as an iterator for this chained array, which loops over all inner arrays.
      class ConstChainedIterator : public std::iterator<std::forward_iterator_tag, value_type> {
      public:
        /// Constructor
        ConstChainedIterator(const typename std::vector<ArrayType>::const_iterator& outerIterator,
                             const typename std::vector<ArrayType>::const_iterator& outerEnd) :
          m_outerIterator(outerIterator), m_outerEnd(outerEnd)
        {
          setInnerIteratorToNextValidStart();
        }

        /// Equality
        bool operator==(const ConstChainedIterator& rhs) const
        {
          return this->m_outerIterator == rhs.m_outerIterator and (this->m_innerIterator == rhs.m_innerIterator
                                                                   or m_outerIterator == m_outerEnd);
        }

        /// Inequality
        bool operator!=(const ConstChainedIterator& rhs) const
        {
          return !(*this == rhs);
        }

        /// Increment the counter
        ConstChainedIterator& operator++()
        {
          if (m_outerIterator == m_outerEnd) {
            return *this;
          }

          m_innerIterator++;
          if (m_innerIterator == std::end(*m_outerIterator)) {
            m_outerIterator++;
            setInnerIteratorToNextValidStart();
          }
          return *this;
        }

        /// Dereference
        const value_type& operator*() const
        {
          return *m_innerIterator;
        }

        /// Dereference
        const value_type* operator->() const
        {
          return &(operator*());
        }

      private:
        /// A pointer to the outer position (which array to use)
        typename std::vector<ArrayType>::const_iterator m_outerIterator;
        /// A pointer to the inner position (where in the array are we)
        typename ArrayType::const_iterator m_innerIterator;
        /// A pointer to the last position (where should we stop)
        typename std::vector<ArrayType>::const_iterator m_outerEnd;

        /// Helper function to look for the next valid inner array position, skipping empty arrays.
        void setInnerIteratorToNextValidStart()
        {
          for (; m_outerIterator != m_outerEnd; ++m_outerIterator) {
            if (not m_outerIterator->empty()) {
              m_innerIterator = m_outerIterator->begin();
              break;
            }
          }
        }
      };

    public:
      /// Empty constructor without content.
      ChainedArray() : m_array( {}), m_size(0) {}

      /// Delete the copy constructor
      ChainedArray(const ChainedArray&) = delete;
      /// Delete the assignment constructor
      ChainedArray& operator=(ChainedArray&) = delete;
      /// Default the move constructor
      ChainedArray(ChainedArray&&) = default;

      /// Constructor taking a vector of arrays.
      ChainedArray(const std::vector<ArrayType>& array) : m_array(array),
        m_size(0)
      {
        const auto& addSizes = [](const int& currentValue, const ArrayType & item) {
          return currentValue + item.size();
        };

        m_size = std::accumulate(std::begin(m_array), std::end(m_array), 0, addSizes);
      }

      /// Return the (precalculated) size.
      size_type size() const
      {
        return m_size;
      }

      /// For for loops
      ConstChainedIterator begin() const
      {
        return ConstChainedIterator(std::begin(m_array), std::end(m_array));
      }

      /// For for loops
      ConstChainedIterator end() const
      {
        return ConstChainedIterator(std::end(m_array), std::end(m_array));
      }

    private:
      /// Storage of the vector of arrays
      const std::vector<ArrayType> m_array;
      /// Storage of the cached size
      size_type m_size;
    };
  }
}