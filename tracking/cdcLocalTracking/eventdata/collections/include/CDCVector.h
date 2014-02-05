/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCVECTOR_H
#define CDCVECTOR_H

#include <vector>
#include <algorithm>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
//#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>

#ifdef __CINT__
//#ifdef CDCLOCALTRACKING_ROOTIFY_CDCVECTOR
// Because ROOTCINT does not like namespaces inside template parameters
// we have to make each object, which we want the instantiate the template
// available outside any namespace. Therefore we also have to include each of
// them as well.

#include <tracking/cdcLocalTracking/eventdata/entities/CDCGenHit.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCWireHit.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit2D.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoTangent.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit3D.h>

typedef Belle2::CDCLocalTracking::CDCGenHit CDCGenHit;
typedef Belle2::CDCLocalTracking::CDCWireHit CDCWireHit;
typedef Belle2::CDCLocalTracking::CDCRecoHit2D CDCRecoHit2D;
typedef Belle2::CDCLocalTracking::CDCRecoTangent CDCRecoTangent;
typedef Belle2::CDCLocalTracking::CDCRecoFacet CDCRecoFacet;
typedef Belle2::CDCLocalTracking::CDCRecoHit3D CDCRecoHit3D;

//#endif // CDCLOCALTRACKING_ROOTIFY_CDCVECTOR
#endif // __CINT__


namespace Belle2 {
  namespace CDCLocalTracking {

    /// A generic vector used for various aggregations
    /** details */

    template<class T>
    class CDCVector : public AutomatonCell {

    private:
      typedef std::vector<T> Container; ///< std::vector to be wrapped

    public:
      typedef T Item; ///< Value type of this container
      typedef const T ConstItem; ///< Value type of this container

    public:
      //typedef T& StoredItemRef;  ///< Reference type to a stored item.

      typedef T value_type; ///< Value type of this container

      typedef typename Container::iterator iterator; ///< Iterator type of this container
      typedef typename Container::const_iterator const_iterator;    ///< Constant iterator type of this container

      typedef typename Container::reverse_iterator reverse_iterator; ///< Reversed iterator type of this container
      typedef typename Container::const_reverse_iterator const_reverse_iterator;  ///< Constant reversed iterator type of this container

      /// Type for a pair of iterators which are iterable with range based for.
      class range : public std::pair<iterator, iterator> {
      public:
        range(const std::pair<iterator, iterator>& iterator_pair) : std::pair<iterator, iterator>(iterator_pair) {;}
        range(const iterator& begin, const iterator& end) : std::pair<iterator, iterator>(begin, end) {;}

        const iterator& begin() const { return this->first;}
        const iterator& end() const { return this->second;}
      };

      /// Type for a pair of constant iterators which are iterable with range based for.
      class const_range : public std::pair<const_iterator, const_iterator> {
      public:
        const_range(const std::pair<const_iterator, const_iterator>& iterator_pair) : std::pair<const_iterator, const_iterator>(iterator_pair) {;}
        const_range(const const_iterator& begin, const const_iterator& end) : std::pair<const_iterator, const_iterator>(begin, end) {;}

        const const_iterator& begin() const { return this->first;}
        const const_iterator& end() const { return this->second;}
      };

      /// Input iterator type usable with stl algorithms
      class input_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void> {
      protected:
        CDCVector<T>& m_collection;
      public:
        explicit input_iterator(CDCVector<T>& collection) : m_collection(collection) {}

        input_iterator& operator= (T const& item) {
          m_collection.push_back(item); return *this;
        }
        input_iterator& operator* ()
        { return *this; }
        input_iterator& operator++ ()
        { return *this; }
        input_iterator operator++ (int)
        { return *this; }
      };


    public:
      /// Default constructor for ROOT compatibility.
      CDCVector() : m_isSorted(true) {;}

      /// Empty deconstructor
      ~CDCVector() {;}

      /// The begin of the container
      iterator begin() { return m_items.begin(); }
      /// The begin of the constant container
      const_iterator begin() const { return m_items.begin(); }
      /// The end of the container
      iterator end() { return m_items.end(); }
      /// The end of the constant container
      const_iterator end() const { return m_items.end(); }

      /// The reverse begin of the container
      reverse_iterator rbegin() { return m_items.rbegin(); }
      /// The reverse begin of the constant container
      const_reverse_iterator rbegin() const { return m_items.rbegin(); }
      /// The reverse end of the container
      reverse_iterator rend() { return m_items.rend(); }
      /// The reverse end of the constant container
      const_reverse_iterator rend() const { return m_items.rend(); }

      /// Getter for the element by its position for easy read access from python.
      /** This gets the an element by its position in the vector. Returns a default \n
       *  constructed element for indices outside the valid range. */
      const Item at(size_t idx) const {
        if (idx < size()) {
          const_iterator itItem = begin();
          std::advance(itItem, idx);
          return *itItem;
        } else {
          return Item();
        }
      }

      /// Returns the number of items in this collection.
      size_t size() const { return m_items.size(); }

      /// Indicates whether the collections is empty.
      bool empty() const { return m_items.empty(); }

      /// Removes all items from the collection.
      void clear() { m_items.clear(); m_isSorted = true;}

      /// Reserves space in std::vector as Container
      void reserve(size_t n) { m_items.reserve(n); }

      /// Gives the first item of the vector
      Item& back() { return m_items.back(); }
      /// Gives the first item of the constant vector
      ConstItem& back() const { return m_items.back(); }

      /// Gives the last item of the vector
      Item& front() { return m_items.front(); }
      /// Gives the last item of the constant vector
      ConstItem& front() const { return m_items.front(); }

      /// Returns an input iterator to the vector
      input_iterator input() { return input_iterator(*this); }

      /// Appends a new elements to the vector. Keeps track if the elements are still sorted
      void push_back(Item const& item) {
        //if the vector was empty and there is only one element added it is sorted
        //else it remains sorted if the added element is not lower than the last
        //this also asures that the m_isSorted field acquires the right value as soon as an element gets added
        //before the empty vector has an undefined sort state put this does not really matter
        //since all access operation are to yield end iterators anyway
        m_isSorted = empty() or (m_isSorted and not(item < m_items.back()));
        m_items.push_back(item);
      }

      /// Appends a range of elements to the vector.
      void push_back(const_iterator first, const_iterator last) { for (; first != last; ++first) push_back(*first); }

      /// Appends a copy of a whole other vector to this vector.
      void push_back(const CDCVector<T>& collection)
      { reserve(size() + collection.size()); push_back(collection.begin(), collection.end()); }

      /// Stores an element into the set. Same method as in the set for easy storage.
      /** The STL unfortunatly does not define a common method to store elements in all kinds \n
       *  of container types. With this method we provide a common method name to just store instances \n
       *  no matter we are storing to a set or to vector */
      //inline StoredItemRef store(Item const& item)
      //{ push_back(item); return back(); }

      /// Inserts an element before the given iterator position
      /** This can be used to just store items the same way as in sets by giving .end() to the pos iterator
        * @return iterator to the new element */
      iterator insert(iterator pos, const Item& item)
      { m_isSorted = false; return m_items.insert(pos, item); }

      /// Sorts the vector
      void sort() {
        std::sort(m_items.begin(), m_items.end());
        m_isSorted = true;
      }

      /// Checks the sort state of the vector by the internal flag
      bool isSorted() const { return empty() or m_isSorted; }

      /// Explicitly checks a the sort state of the vector by a single iteration over it.
      bool checkSorted() const {
        if (empty()) return true;
        bool result = true;

        const_iterator itFirst = begin();
        const_iterator itSecond = begin();
        ++itSecond;

        while (itSecond != end() and result) {
          result = *itFirst++ < *itSecond++ ? true : false;
        }
        return result;
      }

      /// Establishes the vector to be sorted.
      /** This sorts the elements, only if they are not sorted already */
      void ensureSorted() { if (not isSorted()) sort(); }

      /// Establishes a state where each element is only present once inside the vector. This may change the order of elements!
      /** Removes all duplicated entries, but may has to sort the vector first, to detect them. */
      void ensureUnique() {
        ensureSorted();
        iterator last = std::unique(m_items.begin(), m_items.end());
        //erase remove idiom (here with unique)
        erase(last, end());
      }

    public:
      /// Returns an iterator to the found element. end() if not found.
      /** For in sorted vectors this is rather slow. But if the isSorted state is set the look up is \n
       *  as fast as for sets. */
      const_iterator find(const Item& item) const {
        if (isSorted()) {
          const_iterator found = lower_bound(item);
          return found != m_items.end() and * found == item ? found : end();
        } else {
          return std::find(begin(), end(), item);
        }
      }
      /// Returns returns the lower bound iterator of the item in the vector.
      /** The lower bound is only available in sorted vectors. If the isSorted state is not set \n
       *  this method always returns end. In case of a sorted vector this is as fast as the set look up. */
      const_iterator lower_bound(const Item& item) const
      { return isSorted() ? std::lower_bound(begin(), end(), item) : end(); }

      /// Returns returns the equal range iterator pair for the entity that is coaligned with the items in this vector.
      /** The equal range is only available in sorted vectors. If the isSorted state is not set \n
       *  this method always returns two end iterators.
       *  The passed entity must support operator< as both arguments, meaning it is coaligned with items of this vector.*/
      template<class Coaligned>
      range equal_range(const Coaligned& coaligned)
      { return isSorted() ? std::equal_range(begin(), end(), coaligned) : range(end() , end()); }

      /// Returns returns the equal range constant iterator pair for the item in the constant vector.
      /** The equal range is only available in sorted vectors. If the isSorted state is not set \n
       *  this method always returns two end iterators.
       *  The passed entity must support operator< as both arguments, meaning it is coaligned with items of this vector.*/
      template<class Coaligned>
      const_range equal_range(const Coaligned& coaligned) const
      { return isSorted() ? std::equal_range(begin(), end(), coaligned) : const_range(end() , end()); }


      // Note if you have to remove lots of items from the collection at random positions, better use sets.

      /// Erases an item denoted by it iterator into the vector.
      /** This is inefficient for vectors. */
      void erase(iterator itItem) { m_items.erase(itItem); }

      /// Erases a range from the vector.
      /** Erases a range of items from the vector. The given iterators should point into the vector. This is inefficient.*/
      void erase(iterator itBegin, iterator itEnd) { m_items.erase(itBegin, itEnd); }

      /// Erases an item from the vector
      /** Before erasing the item the item has to be found. For sorted vectors this is a bit faster, \n
          but still the whole method is rather inefficient for vectors*/
      void erase(const Item& item) {
        if (isSorted()) {
          range rangeToRemove = equal_range(item);
          erase(rangeToRemove.first, rangeToRemove.second);
        } else {
          //erase remove idiom
          erase(std::remove(begin(), end(), item), end());
        }
      }

      /// Evaluates how many times the item is in the collection
      /** This normally takes a whole iteration over the vector to find all elements.
       *  Speeded up for sorted vectors. */
      size_t count(const Item& item) const {
        if (isSorted()) {
          const_range rangeToCount = equal_range(item);
          return std::distance(rangeToCount.first, rangeToCount.second);
        } else {
          return std::count(begin(), end(), item);
        }
      }

      /// Evaluates if the given item is in the collection
      bool contains(const Item& item) const { return find(item) != end(); }

      /// Evaluate if the given collection is completelly contained in *this*
      /** Inefficient only if both vectors are sorted. */
      bool contains(const CDCVector<T>& items) const {
        if (isSorted() and items.isSorted()) {
          return std::includes(begin(), end(), items.begin(), items.end());
        } else {
          for (const_iterator itItem = items.begin(); itItem != end() ; ++itItem) {
            if (!contains(*itItem)) return false;
          }
          return true;
        }
      }


    protected:
      bool m_isSorted; ///< Memory for the sort state of the vector
      std::vector<T> m_items;

    private:
      /// ROOT Macro to make CDCVector a ROOT class.
      ClassDefInCDCLocalTracking(CDCVector, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // CDCVECTOR_H
