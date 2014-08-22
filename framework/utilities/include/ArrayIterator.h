/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARRAYITERATOR_H
#define ARRAYITERATOR_H

#include <iterator>

namespace Belle2 {
  /** Generic iterator class for arrays, allowing use of STL algorithms, range-based for etc.
   *
   * Implements all operations required for a forward iterator. Note that wether objects can actually
   * be added (i.e. extending the array) depends on the implementation of operator[].
   *
   * It only requires that ArrayType provides operator[int] returning pointers or references to ValueType, you can then
   * return ArrayIterator objects * with indices 0 and size() from begin() and end(). You should also define
   * iterator and const_iterator typedefs as part of ArrayType. See StoreArray for examples.
   *
   * Note that dereferencing an iterator via * returns a reference to ValueType, regardless of wether ArrayType::operator[]
   * returns ValueType* or ValueType&.
   */
  template <class ArrayType, class ValueType>
  class ArrayIterator : public std::iterator<std::forward_iterator_tag, ValueType> {
    /** dereference if argument is a pointer to ValueType. */
    static ValueType& deref_if_needed(ValueType& t) { return t; }
    /** dereference if argument is a pointer to ValueType. */
    static ValueType& deref_if_needed(ValueType* t) { return *t; }
  public:
    /** Default constructor (not that you can dereference these). */
    ArrayIterator(): m_array(NULL), m_index(-1) {}

    /** Constructor. */
    explicit ArrayIterator(const ArrayType* array, int index):
      m_array(array),
      m_index(index)
    {}

    /** prefix increment. */
    ArrayIterator<ArrayType, ValueType>& operator++() { ++m_index; return *this; }

    /** postfix increment. */
    ArrayIterator<ArrayType, ValueType> operator++(int) {
      ArrayIterator<ArrayType, ValueType> old(*this);
      ++(*this);
      return old;
    }

    /** check equality. */
    bool operator==(const ArrayIterator<ArrayType, ValueType>& rhs) const { return m_index == rhs.m_index && m_array == rhs.m_array; }
    /** check inequality. */
    bool operator!=(const ArrayIterator<ArrayType, ValueType>& rhs) const { return !(*this == rhs); }

    /** dereference. */
    ValueType& operator*() const { return deref_if_needed((*m_array)[m_index]); }
    /** dereference. */
    ValueType* operator->() const { return &(operator*()); }

  private:
    const ArrayType* m_array; /**< Array to iterate over. */
    int m_index; /**< Current index. */
  };
}

#endif
