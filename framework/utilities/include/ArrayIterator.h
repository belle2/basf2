/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <iterator>

class TObject;

namespace Belle2 {
  /** Optimizes class to iterate over TObjArray and classes inheriting from it.
   *  iterators are invalidated if the size of the TObjArray is changed.
   */
  template <class ArrayType, class ValueType> class ObjArrayIterator: public std::iterator<std::forward_iterator_tag, ValueType>  {
  public:
    /** default constructor */
    ObjArrayIterator() = default;
    /** real constructor
     * @param array reference to the TObjArray instance we want to iterate over
     * @param index integer of the index we want to point to right away
     */
    ObjArrayIterator(ArrayType& array, size_t index): m_array{array.GetObjectRef() + index} {}

    /** Convenience constructor in case of a TClonesArray pointer.
     * The "const *" is needed so that a const TClonesArray* can be passed in for const ObjArrayIterators
     * @param array pointer to where the array is located. If any
     *    of the two is NULL a default iterator is created
     * @param end if true point to after the array, otherwise point to the
     *    beginning
     */
    ObjArrayIterator(ArrayType const* array, bool end = false)
    {
      if (array) {
        m_array = array->GetObjectRef() + (end ? array->GetEntriesFast() : 0);
      }
    }

    /** Convenience constructor because we usually have a TClonesArray** member so this takes cares of the checks.
     * The "const * const *" is needed so that a const TClonesArray** can be passed in for const ObjArrayIterators
     * @param array pointer to the pointer where the array is located. If any
     *    of the two is NULL a default iterator is created
     * @param end if true point to after the array, otherwise point to the
     *    beginning
     */
    ObjArrayIterator(ArrayType const* const* array, bool end = false): ObjArrayIterator(array ? * array : nullptr, end) {}
    /** prefix increment */
    ObjArrayIterator<ArrayType, ValueType>& operator++() { ++m_array; return *this; }
    /** postfix increment */
    ObjArrayIterator<ArrayType, ValueType> operator++(int) { auto old = *this; ++(*this); return old; }
    /** check equality. */
    bool operator==(const ObjArrayIterator<ArrayType, ValueType>& rhs) const { return m_array == rhs.m_array; }
    /** check inequality. */
    bool operator!=(const ObjArrayIterator<ArrayType, ValueType>& rhs) const { return !(*this == rhs); }

    /** dereference. */
    ValueType& operator*() const { return *static_cast<ValueType*>(*m_array); }
    /** dereference. */
    ValueType* operator->() const { return &(operator*()); }
  private:
    /** pointer to the fCont member of the TObjArray */
    TObject** m_array{nullptr};
  };

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
    ArrayIterator<ArrayType, ValueType> operator++(int)
    {
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
