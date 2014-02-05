/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCGENHITVECTOR_H
#define CDCGENHITVECTOR_H

#include <vector>
#include <algorithm>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
//#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>

#include <tracking/cdcLocalTracking/topology/CDCWire.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCGenHit.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCWireHit.h>


#ifdef __CINT__
//#ifdef CDCLOCALTRACKING_ROOTIFY_CDCGENHITVECTOR
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

//#endif // CDCLOCALTRACKING_ROOTIFY_CDCGENHITVECTOR
#endif // __CINT__


namespace Belle2 {
  namespace CDCLocalTracking {

    /// A generic vector used for different hit aggregates.
    /** details */

    template<class T>
    class CDCGenHitVector : public AutomatonCell {

    private:
      typedef std::vector<T> Container; ///< std::vector to be wrapped

    public:
      typedef T Item; ///< Value type of this container

    public:
      typedef T& StoredItemRef;  ///< Reference type to a stored item.

      typedef T value_type; ///< Value type of this container

      typedef typename Container::iterator iterator; ///< Iterator type of this container
      typedef typename Container::const_iterator const_iterator;    ///< Constant iterator type of this container

      typedef typename Container::reverse_iterator reverse_iterator; ///< Reversed iterator type of this container
      typedef typename Container::const_reverse_iterator const_reverse_iterator;  ///< Constant reversed iterator type of this container

      typedef std::pair<iterator, iterator> range; ///< Pair type of iterators indicating a range
      typedef std::pair<const_iterator, const_iterator> const_range; ///< Pair type of constant iterators indication a range

      /// Input iterator type usable with stl algorithms
      class input_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void> {
      protected:
        CDCGenHitVector<T>& m_collection;
      public:
        explicit input_iterator(CDCGenHitVector<T>& collection) : m_collection(collection) {}

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
      CDCGenHitVector() {;}

      /// Empty deconstructor
      ~CDCGenHitVector() {;}

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
      const Item& back() const { return m_items.back(); }

      /// Gives the last item of the vector
      Item& front() { return m_items.front(); }
      /// Gives the last item of the constant vector
      const Item& front() const { return m_items.front(); }


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
      void push_back(const CDCGenHitVector<T>& collection)
      { reserve(size() + collection.size()); push_back(collection.begin(), collection.end()); }

      /// Stores an element into the set. Same method as in the set for easy storage.
      /** The STL unfortunatly does not define a common method to store elements in all kinds \n
       *  of container types. With this method we provide a common method name to just store instances \n
       *  no matter we are storing to a set or to vector */
      inline StoredItemRef store(Item const& item)
      { push_back(item); return back(); }

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

      void ensureSorted() { if (not isSorted() or not checkSorted()) sort(); }

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

      /// Returns returns the equal range iterator pair for the item in the vector.
      /** The equal range is only available in sorted vectors. If the isSorted state is not set \n
       *  this method always returns two end iterators. In case of a sorted vector this is as fast as the set look up. */
      range equal_range(const Item& item)
      { return isSorted() ? std::equal_range(begin(), end(), item) : range(end() , end()); }

      /// Returns returns the equal range constant iterator pair for the item in the constant vector.
      /** The equal range is only available in sorted vectors. If the isSorted state is not set \n
       *  this method always returns two end iterators. In case of a sorted vector this is as fast as the set look up. */
      const_range equal_range(const Item& item) const
      { return isSorted() ? std::equal_range(begin(), end(), item) : const_range(end() , end()); }


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
      bool contains(const CDCGenHitVector<T>& items) const {
        if (isSorted() and items.isSorted()) {
          return std::includes(begin(), end(), items.begin(), items.end());
        } else {
          for (const_iterator itItem = items.begin(); itItem != end() ; ++itItem) {
            if (!contains(*itItem)) return false;
          }
          return true;
        }
      }

    private:
      /// Helper class for STL algorithms searching for specific wire
      class HasWirePredicate {
      public:
        HasWirePredicate(const Belle2::CDCLocalTracking::CDCWire& wire) : m_wire(wire) {;}
        bool operator()(const Item& item) { return item->hasWire(m_wire); }
      private:
        const CDCWire& m_wire;
      };
      /// Helper class for STL algorithms searching for specific wire hit
      class HasWireHitPredicate {
      public:
        HasWireHitPredicate(const Belle2::CDCLocalTracking::CDCWireHit& wirehit) : m_wirehit(wirehit) {;}
        bool operator()(const Item& item) { return item->hasWireHit(m_wirehit); }
      private:
        const Belle2::CDCLocalTracking::CDCWireHit& m_wirehit;
      };

    public:
      /// Erases all tracking entities assoziated with specific wire from the vector.
      void eraseAll(const CDCWire& wire) {
        m_items.erase(remove_if(m_items.begin(), m_items.end(), HasWirePredicate(wire)), m_items.end());
        //remove erase idiom
      }
      /// Erases all tracking entities assoziated with specific wire hit from the vector.
      void eraseAll(const Belle2::CDCLocalTracking::CDCWireHit& wirehit) {
        m_items.erase(remove_if(m_items.begin(), m_items.end(), HasWireHitPredicate(wirehit)), m_items.end());
        //remove erase idiom
      }

      /// Checks if any stored tracking entity is assoziated with a specific wire.
      bool hasWire(const CDCWire& wire) const {
        const_iterator found = std::find_if(begin(), end(), HasWirePredicate(wire));
        return found != end();
      }

      /// Checks if any stored tracking entity is assoziated with a specific wire hit.
      bool hasWireHit(const Belle2::CDCLocalTracking::CDCWireHit& wirehit) const {
        const_iterator found = std::find_if(begin(), end(), HasWireHitPredicate(wirehit));
        return found != end();
      }

      /// Copy all entities in this collection assoziated with a specific wire to the given collection.
      void collectForWire(const CDCWire& wire, CDCGenHitVector<T>& collect) const {
        input_iterator inputTo = collect.input();
        for (const_iterator itItem = begin(); itItem != end(); ++itItem) {
          if ((*itItem)->hasWire(wire)) inputTo = *itItem;
        }
      }
      /// Copy all entities in this collection assoziated with a specific wire hit to the given collection.
      void collectForWireHit(const Belle2::CDCLocalTracking::CDCWireHit& wirehit, CDCGenHitVector<T>& collect) const {
        input_iterator inputTo = collect.input();
        for (const_iterator itItem = begin(); itItem != end(); ++itItem) {
          if ((*itItem)->hasWireHit(wirehit)) inputTo = *itItem;
        }
      }

      /// Calculates the average center of mass of all stored tracking entities.
      Vector2D getCenterOfMass2D() const {
        Vector2D accumulate(0.0, 0.0);
        //B2DEBUG(100,"getCenterOfMass");
        for (const_iterator itItem = begin(); itItem != end(); ++itItem) {
          //B2DEBUG(100,(*itItem)->getCenterOfMass2D());
          accumulate.add((*itItem)->getCenterOfMass2D());
        }
        accumulate.divide(size());
        return accumulate;
      }

      /// Returns the common axial type of all tracking entities.
      /** This checks if all tracking entities are located in the same superlayer and \n
       *  returns the axial type of the later. Returns INVALID_AXIALTYPE if the superlayer \n
       *  is not shared among the tracking entities. */
      AxialType getAxialType() const
      { return getISuperLayer() == INVALIDSUPERLAYER ? INVALID_AXIALTYPE : (*begin())->getAxialType(); }


      /// Returns the common super layer id of all stored tracking entities
      /** This checks if all tracking entities are located in the same superlayer and \n
       *  returns the superlayer id of the later. Returns INVALIDSUPERLAYER if the superlayer \n
       *  is not shared among the tracking entities. */
      ILayerType getISuperLayer() const {
        if (empty()) return INVALIDSUPERLAYER;
        const_iterator itItem = begin();
        const Item& firstItem =  *itItem;
        ILayerType iSuperlayer = firstItem->getISuperLayer();
        for (++itItem; itItem != end(); ++itItem) {
          if (iSuperlayer != (*itItem)->getISuperLayer()) return INVALIDSUPERLAYER ;
        }
        return iSuperlayer;
      }

      /// Calculates the sum of all squared distances of the stored tracking entities to the circle as see from the transvers plane.
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const {

        FloatType accumulate = 0;
        for (const_iterator itItem = begin(); itItem != end(); ++itItem) {
          const Item& item = *itItem;
          accumulate += item->getSquaredDist2D(trajectory2D);
        }
        return accumulate;

      }

      // Methodes that explicitly use the order of the items as a feature of the storing vector.
      // meaning things that can not be done with the set

      /// Calculates the travel distance of the first track entity in the vector
      FloatType getStartPerpS(const CDCTrajectory2D& trajectory2D) const
      { return front()->getStartPerpS(trajectory2D); }

      /// Calculates the travel distance of the last track entity in the vector
      FloatType getEndPerpS(const CDCTrajectory2D& trajectory2D) const
      { return back()->getEndPerpS(trajectory2D); }

      /// Checks if the last entity in the vector lies at greater travel distance than the first entity
      bool isForwardTrajectory(const CDCTrajectory2D& trajectory2D) const
      { return getStartPerpS(trajectory2D) < getEndPerpS(trajectory2D); }

    private:
      bool m_isSorted; ///< Memory for the sort state of the vector
      std::vector<T> m_items;

    private:
      /// ROOT Macro to make CDCGenHitVector a ROOT class.
      ClassDefInCDCLocalTracking(CDCGenHitVector, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // CDCGENHITVECTOR_H
