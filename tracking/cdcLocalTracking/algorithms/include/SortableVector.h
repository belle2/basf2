/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef SORTABLEVECTOR_H
#define SORTABLEVECTOR_H

#include <vector>
#include <algorithm>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>

//Unpacked version for ROOT dictionary generation
#include <tracking/cdcLocalTracking/topology/CDCWireUnpackedFromNamespaces.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCEntitiesUnpackedFromNamespaces.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// A generic vector which keeps track if its elements are sorted and speeds up lookups accordingly.
    template<class T>
    class SortableVector : public UsedTObject {

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
        /// Constructor translating a pair of iterators (as returned by equal_range) to a range usable with range based for.
        range(const std::pair<iterator, iterator>& iterator_pair) : std::pair<iterator, iterator>(iterator_pair) {;}

        /// Constructor translating two iterators (e.g. lower_bound, upper_bound) to a range usable with range based for.
        range(const iterator& begin, const iterator& end) : std::pair<iterator, iterator>(begin, end) {;}

        /// Begin of the range for range based for.
        iterator begin() const { return this->first;}

        /// End of the range for range based for.
        iterator end() const { return this->second;}
      };

      /// Type for a pair of constant iterators which are iterable with range based for.
      class const_range : public std::pair<const_iterator, const_iterator> {
      public:
        /// Constructor translating a pair of iterators (as returned by equal_range) to a range usable with range based for.
        const_range(const std::pair<const_iterator, const_iterator>& iterator_pair) : std::pair<const_iterator, const_iterator>(iterator_pair) {;}

        /// Constructor translating two iterators (e.g. lower_bound, upper_bound) to a range usable with range based for.
        const_range(const const_iterator& begin, const const_iterator& end) : std::pair<const_iterator, const_iterator>(begin, end) {;}

        /// Begin of the range for range based for.
        const_iterator begin() const { return this->first;}

        /// End of the range for range based for.
        const_iterator end() const { return this->second;}

        /// Checks if the begin equals the end iterator, hence if the range is empty.
        bool empty() const { return this->first == this->second; }

        /// Returns the derefenced begin iterator.
        const T& front() const { return *begin(); }

        /// Returns the derefenced end iterator
        const T& back() const { return *end(); }

        /// Returns the object at index i
        const T& at(size_t pos) const { return *(begin() + pos); }

        /// Returns the total number of objects in this range
        size_t size() const { return std::distance(begin(), end()); }

      };


      /// Type for a pair of constant reverse iterators which are iterable with range based for.
      class const_reverse_range : public std::pair<const_reverse_iterator, const_reverse_iterator> {
      public:
        /// Constructor translating a pair of iterators to a range usable with range based for.
        const_reverse_range(const std::pair<const_reverse_iterator, const_reverse_iterator>& iterator_reverse_pair) :
          std::pair<const_reverse_iterator, const_reverse_iterator>(iterator_reverse_pair) {;}

        /// Constructor translating two iterators (e.g. rbegin, rend) to a range usable with range based for.
        const_reverse_range(const const_reverse_iterator& begin, const const_reverse_iterator& end) :
          std::pair<const_reverse_iterator, const_reverse_iterator>(begin, end) {;}

        /// Begin of the range for range based for.
        const_reverse_iterator begin() const { return this->first;}

        /// End of the range for range based for.
        const_reverse_iterator end() const { return this->second;}
      };


      /// Input iterator type for the sortable vector usable with stl algorithms
      class input_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void> {
      protected:
        /// Reference to the sorted vector.
        SortableVector<T>& m_collection;

      public:
        /// Constructor taking the sortable vector to be filled as a reference
        explicit input_iterator(SortableVector<T>& collection) : m_collection(collection) {}

        /// Assignment operator putting a new element into the sortable vector
        input_iterator& operator= (T const& item)
        { m_collection.push_back(item); return *this; }

        /// Derefering returns the input iterator itself
        input_iterator& operator* ()
        { return *this; }

        /// Increment operator, returns input iterator unchanged
        input_iterator& operator++ ()
        { return *this; }

        /// Increment operator, returns input iterator unchanged
        input_iterator operator++ (int)
        { return *this; }
      };


    public:
      /// Default constructor for ROOT compatibility.
      SortableVector() : m_isSorted(true) {;}

      /// Empty deconstructor
      ~SortableVector() {;}

      operator const_range() { return const_range(begin(), end()); }

      ///Defines an ordering of sortable vector instance by lexicographical comparison
      bool operator<(SortableVector<T> const& rhs) const
      { return  m_items < rhs.m_items; }

      ///Swaps the items out of the other vector
      void swap(SortableVector<T>& rhs)
      { m_items.swap(rhs.m_items); std::swap(m_isSorted, rhs.m_isSorted); }

      ///Implements the standard swap idiom
      friend void swap(SortableVector<T>& lhs, SortableVector<T>& rhs)
      { std::swap(lhs.m_items, rhs.m_items); std::swap(lhs.m_isSorted, rhs.m_isSorted); B2DEBUG(200, "SortableVector::swap");}


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

      /// Getter for a reverse range usable with range based for
      const_reverse_range reverseRange() const { return const_reverse_range(rbegin(), rend()); }


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

      /// Unchecked access to the elements of the vector
      Item& operator[](size_t const& index) { return m_items[index]; }

      /// Unchecked access to the elements of the constant vector
      ConstItem& operator[](size_t const& index) const { return m_items[index]; }

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
      void push_back(const SortableVector<T>& collection)
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
      /** For unsorted vectors this is rather slow. But if the isSorted state is set the look up is \n
       *  as fast as for sets. */
      const_iterator find(const Item& item) const {
        if (isSorted()) {
          const_iterator found = lower_bound(item);
          return (found != m_items.end() and (*found) == item) ? found : end();
        } else {
          return std::find(begin(), end(), item);
        }
      }


      /// Returns an iterator to the found element. end() if not found.
      /** This method gains some speed up by considering the memory address of the given object first.
       *  If the address lies within the memory range of the vector the iterator can be forwarded to this position.
       *  without any comparision operation. Otherwise the normal find() is invoked.
       *  This essentially casts a reference into the vector to an iterator.
       *  Note: Although considering the memory address this operation is as fail safe as the normal find operation.
       *  If the address does not point to a proper object inside the vector this method will fail as would the normal find.
       */
      const_iterator findFast(const Item& item) const {
        if (containsPointer(&item)) {
          const_iterator itBegin = begin();
          const Item* ptrItem = &item;
          const Item* ptrBegin = &*itBegin;
          size_t idxItem = ptrItem - ptrBegin;
          std::advance(itBegin, idxItem);
          return itBegin;
        } else {
          return find(item);
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
      bool contains(const SortableVector<T>& items) const {
        if (isSorted() and items.isSorted()) {
          return std::includes(begin(), end(), items.begin(), items.end());
        } else {
          for (const_iterator itItem = items.begin(); itItem != end() ; ++itItem) {
            if (!contains(*itItem)) return false;
          }
          return true;
        }
      }


      /// Evaluates if the pointer address of the given object points to a place inside the containers range
      bool containsPointer(const T* item) const {
        // Note size_t is required to be large enough to contain any of the platform's memory addresses
        size_t instanceAddress = (size_t)item;

        const_iterator itBegin = begin();
        const_iterator itEnd = end();

        size_t beginAddress = (size_t)(&*itBegin);
        size_t endAddress = (size_t)(&*itEnd);

        bool isInside = beginAddress <= instanceAddress and instanceAddress < endAddress;
        bool paddingIsCorrect = ((instanceAddress - beginAddress) % sizeof(T)) == 0;

        return isInside and paddingIsCorrect;
      }



    protected:
      bool m_isSorted; ///< Memory for the sort state of the vector.
      std::vector<T> m_items; ///< Memory of the wrapped vector containing the items.

    private:
      /// ROOT Macro to make SortableVector a ROOT class.
      ClassDefInCDCLocalTracking(SortableVector, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // SORTABLEVECTOR_H
