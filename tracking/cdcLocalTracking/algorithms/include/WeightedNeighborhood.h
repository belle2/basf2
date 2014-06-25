/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef WEIGHTEDNEIGHBORHOOD_H
#define WEIGHTEDNEIGHBORHOOD_H

#include <iostream>
#include <map>
#include <limits>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include "NeighborWeight.h"

#include <boost/type_traits/remove_pointer.hpp>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing weighted relations between items of the same type.
    /** The WeightedNeighborhood implements a weighted graph of items of the same types \n
     *  This structure does not store any items but solely the pointers to them. \n
     *  Therefore the memory has to be provided by other means and should not be moved \n
     *  as long as you want to access the neighbors of items via this object. Also the \n
     *  retrival works solely by the memory address of the items. So you have to give \n
     *  the \a same objects for retrival of neighbors and not just one comparing equal \n
     *  to it. Internally the neighbor relations are sorted by their pointer address first and \n
     *  by the weight second, but you should not rely on that to heavily since the neighborhood \n
     *  might get reworked to use an unordered_multimap instead of a multimap in c++11. */
    template<class Item>
    class WeightedNeighborhood {


      /// Item type with a pointer star removed
      typedef typename boost::remove_pointer<Item>::type BareItem;

      /// Reference type of the items the neighborhood is related to
      typedef BareItem&   ItemRef;

      /// Pointer type of the items the neighborhood is related to
      typedef BareItem const*   ItemPtr; //making the pointer constant since we do not want to change the object. the contained data stays as is.

    private:

      /// Structure keeping a pointer to the item and the weight
      /** Structure that is used as key for the multimap that stores the neighbor relations
       *  It keeps a pointer to the item to be stored and uses the pointer address to sort for.
       *  Secondly it also stores the weight for sorting, so that the neighbors to one item are
       *  sorted with increasing weights */
      struct WeightedItemPtr {

        /// Constructor bundeling the original item and the weight of a relation
        WeightedItemPtr(const ItemPtr& itemPtr , const Weight& weight) :
          m_itemPtr(itemPtr)      , m_weight(weight) {;}

        /// Comparison operator establishing a total ordering considering the pointer first and the weight second
        inline bool operator<(const WeightedItemPtr& other) const
        { return m_itemPtr < other.m_itemPtr or (m_itemPtr == other.m_itemPtr  and m_weight < other.m_weight); }

        /// Getter for the pointer to the original item
        ItemPtr getItemPtr() const { return m_itemPtr; }

        /// Getter for the weight
        Weight getWeight() const { return m_weight; }

      private:
        ItemPtr m_itemPtr; ///< Member to store the pointer to the item
        Weight m_weight; ///< Member to store the weight of the neighbor relation

      };


    private:
      /// Multimap that stores the neighborhood relations
      typedef std::multimap<WeightedItemPtr, ItemPtr> Container;

      /// Value type of the neighborhood aka std::pair<const WeightedItemPtr,NeighborPtr>
      typedef typename Container::value_type value_type;

      /// Better type name for the stored relations
      typedef value_type WeightedRelation;

    private:
      /// Iterator type of the multimap to be wrapped by a more expressive iterator
      typedef typename Container::const_iterator container_const_iterator;

    public:

      /// Expressive iterator type of the neighborhood relations
      /** The iterator type of relations in this neighborhood. It inherits for \n
       *  the underlying multimap::iterator and adds some more expressive names \n
       *  for the value parts in it */
      class iterator : public container_const_iterator {
      public:

        /// Constructor taking the raw iterator to be wrapped
        iterator(const container_const_iterator& it) : container_const_iterator(it) {;}

        /// Getter to the pointer to the neighboring item
        ItemPtr getNeighbor()
        { return (*this)->second; }

        /// Getter to the pointer to the original item
        ItemPtr getItem()
        { return (*this)->first.getItemPtr(); }

        /// Getter for the weight assoziated with the neighborhood relation
        Weight getWeight()
        { return (*this)->first.getWeight(); }
      };

      /// Iterator range type for a pair of iterators representing all the neighbors of a specific item.
      typedef std::pair<iterator, iterator> range;


      /// Default constructor
      WeightedNeighborhood() {;}

      /// Empty destructor
      ~WeightedNeighborhood() {;}

    private:

      Container m_neighbors; ///< Memory of the multimap storing the neighborhood relations.

    public:

      /** @name Inserting neighbor relations */
      /**@{*/
      /// Inserts a neighborhood relation in to the neighborhood.
      ///inline void insert( const  WeightedRelation & neighborRelation)
      ///{ m_neighbors.insert(neighborRelation); }

      /// Inserts a new relation
      /** Inserts a new relation. The item and neighbor are given by pointer. \n
       *  Copys of the pointers are stored in the underlying multimap. \n
       *  Additionally a weight can be given. If omited the weight defaults to one. \n
       */
      inline void insert(
        const ItemPtr& itemPtr,
        const ItemPtr& neighborPtr,
        Weight weight = 1
      )
      { m_neighbors.insert(WeightedRelation(WeightedItemPtr(itemPtr, weight) , neighborPtr)); }

      /// Inserts a new relation by pointers
      /** Inserts a new relation. The item and neighbor are given by pointer. \n
       *  Copys of the pointers are stored in the underlying multimap. \n
       *  Additionally a weight must be given in the middle.
       */
      inline void insert(
        ItemPtr itemPtr,
        Weight weight,
        ItemPtr neighborPtr
      )
      { m_neighbors.insert(WeightedRelation(WeightedItemPtr(itemPtr, weight) , neighborPtr)); }


      /// Inserts a new relation by references.
      /** Inserts a new relation. The item and neighbor are given by reference. \n
       *  Since the neighborhood stores pointers to the items, the addresses are taken
       *  out and stored in the multimap.
       *  Copys of the pointers are stored in the underlying multimap. \n
       *  Additionally a weight can be given. If omited the weight defaults to one. \n
       */
      inline void insert(
        ItemRef itemRef,
        ItemRef neighborRef,
        Weight weight
      )
      { insert(&(itemRef), &(neighborRef), weight); }


      /// Inserts a new relation by references.
      /** Inserts a new relation. The item and neighbor are given by reference. \n
       *  Since the neighborhood stores pointers to the items, the addresses are taken
       *  out and stored in the multimap.
       *  Copys of the pointers are stored in the underlying multimap. \n
       *  Additionally a weight must be given in the middle. \n
       */
      inline void insert(
        ItemRef itemRef,
        Weight weight,
        ItemRef neighborRef
      )
      { insert(&(itemRef), weight, &(neighborRef)); }
      /**@}*/

      /** @name Retrival of neigbors
       *  Important since the multimap stores pointers the _same_ object address that have been \n
       *  used to store the items have to be used for the retrival.
       *  This should not be a problem since the memory holding the items should still be available. \n
       *  If the memory was already freed the pointers are rendered invalid anyways and now valid object \n
       *  can be retived anymore.
       */
      /**@{*/

      /// Checks if the item given by pointers has neighbors in the neighborhood
      bool hasNeighbors(ItemPtr itemPtr) const
      { return getLightestNeighbor(itemPtr) != m_neighbors.end(); }

      /// Checks if the item given by reference has neighbors in the neighborhood
      bool hasNeighbors(ItemRef itemRef) const
      { return hasNeighbors(&itemRef); }

      /// Returns an iterator to the neighbor with the least weight by pointer
      /** If the item given by pointer has neighbors in the neighborhood return \n
       *  an iterator pointing to the one with the smallest weight. \n
       *  If their are no neighbors to the item given return the end() of the neighborhood. */
      iterator getLightestNeighbor(ItemPtr itemPtr) const {

        iterator lowerBound = m_neighbors.lower_bound(WeightedItemPtr(itemPtr, LOWEST_WEIGHT));
        return lowerBound.getItem() == itemPtr ? lowerBound : m_neighbors.end();

      }

      /// Returns an iterator to the neighbor with the smallest weight by pointer
      /** If the item given by reference has neighbors in the neighborhood return \n
       *  an iterator pointing to the one with the smallest weight. \n
       *  If their are no neighbors to the item given return the end() of the neighborhood. */
      iterator getLightestNeighbor(ItemRef itemRef) const
      { return getLightestNeighbor(&itemRef); }


      /// Returns an iterator to the neighbor with the highest weight by pointer
      /** If the item given by pointer has neighbors in the neighborhood return \n
       *  an iterator pointing to the one with the highest weight. \n
       *  If their are no neighbors to the item given return the end() of the neighborhood. */
      iterator getHeaviestNeighbor(ItemPtr itemPtr) const {

        iterator upperBound = m_neighbors.upper_bound(WeightedItemPtr(itemPtr, HIGHEST_WEIGHT));

        if (upperBound == m_neighbors.begin()) return m_neighbors.end();
        --upperBound;
        return upperBound.getItem() == itemPtr ? upperBound : m_neighbors.end();

      }

      /// Returns an iterator to the neighbor with the highest weight by pointer
      /** If the item given by reference has neighbors in the neighborhood return \n
       *  an iterator pointing to the one with the highest weight. \n
       *  If their are no neighbors to the item given return the end() of the neighborhood. */
      iterator getHeaviestNeighbor(ItemRef itemRef) const
      { return getHeaviestNeighbor(&itemRef); }


      /// Same as equal_range()
      range getNeighborRange(ItemPtr itemPtr) const
      { return equal_range(itemPtr); }

      /// Same as equal_range()
      range getNeighborRange(ItemRef itemRef) const
      { return equal_range(&itemRef); }


      /// Returns an iterator range representing all neighbors.
      /** Returns the range of all neighbors to the item given by pointer as a pair of iterators. \n
       *  The range is sorted internally by the weight of the neighbor from smallest to biggest. \n
       *  If there are no neighbor an empty range will be returned.
       *  @return a pair of iterators for the range of neighbors */
      range equal_range(ItemPtr itemPtr) const {

        iterator lowerBound(m_neighbors.lower_bound(WeightedItemPtr(itemPtr, LOWEST_WEIGHT)));
        iterator upperBound(m_neighbors.upper_bound(WeightedItemPtr(itemPtr, HIGHEST_WEIGHT)));
        return range(lowerBound, upperBound);

      }
      /**@}*/

      /// Returns an iterator range representing all neighbors.
      /** Returns the range of all neighbors to the item given by reference. \n
       *  The range is sorted internally by the weight of the neighbor from smallest to biggest. \n
       *  If there are no neighbor an empty range will be returned.
       *  @return a pair of iterators for the range of neighbors */
      range equal_range(ItemRef itemRef) const
      { return equal_range(&itemRef); }

      /// Returns the begin of all neighborhood relations
      iterator begin() const { return iterator(m_neighbors.begin()) ;}

      /// Returns the end of all neighborhood relations
      iterator end()  const { return iterator(m_neighbors.end()) ;}

      /// Returns the number of neighborhood relations
      size_t size() { return m_neighbors.size(); }

      /// Drops all stored neighborhood relations
      void clear() {  m_neighbors.clear(); }

      /// Checks for the symmetry of the neighborhood
      /** Explicitly checks for each neighborhood relation, if their is an inverse relation \n
       *  with the same weight. Returns true if all such inverse relations exist. */
      bool isSymmetric() const {
        bool result = true;
        for (iterator itNeighborPairs = begin();
             itNeighborPairs !=  end(); ++itNeighborPairs) {

          const ItemPtr& itemPtr = itNeighborPairs.getItem();
          const Weight& weight = itNeighborPairs.getWeight();
          const ItemPtr& neighborPtr = itNeighborPairs.getNeighbor();

          //search for an inverse pair
          bool found = false;
          range neighborsOfNeighborRange = equal_range(neighborPtr);

          for (iterator itNeighborOfNeighbor = neighborsOfNeighborRange.first;
               itNeighborOfNeighbor != neighborsOfNeighborRange.second and not found;
               ++itNeighborOfNeighbor) {

            if (itNeighborOfNeighbor.getNeighbor() == itemPtr and
                itNeighborOfNeighbor.getWeight() == weight) {
              found = true;
            }
          }
          if (not found) {
            B2WARNING(" Neighborhood is not symmetric in " << *itemPtr <<
                      " to " << *neighborPtr);
            result = false;
          }

        }
        return result;
      }

      /// Output operator to help debugging
      friend std::ostream& operator<<(std::ostream& output, const WeightedNeighborhood& neighborhood) {
        for (iterator itNeighbor = neighborhood.begin();
             itNeighbor != neighborhood.end(); ++itNeighbor) {

          output << itNeighbor.getItem() << " " <<
                 itNeighbor.getItem()->getCellState() << " == " <<
                 itNeighbor.getItem()->getCellWeight() << " + " <<
                 itNeighbor.getWeight() << " + " <<
                 itNeighbor.getNeighbor()->getCellState() << " -> " <<
                 itNeighbor.getNeighbor() << std::endl ;

        }
        return output;
      }

    }; //class
  } // end namespace WeightedNeighborhood
} // namespace Belle2
#endif // WEIGHTEDNEIGHBORHOOD_H_
