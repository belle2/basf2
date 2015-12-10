/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/ca/Relation.h>

#include <tracking/trackFindingCDC/ca/NeighborWeight.h>

#include <tracking/trackFindingCDC/utilities/Range.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>

#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class representing weighted relations between items of the same type.
    /** The WeightedNeighborhood implements a weighted graph of items of the same types \n
     *  This structure does not store any items but solely the pointers to them. \n
     *  Therefore the memory has to be provided by other means and should not be moved \n
     *  as long as you want to access the neighbors of items via this object. Also the \n
     *  retrival works solely by the memory address of the items. So you have to give \n
     *  the \a same objects for retrival of neighbors and not just one comparing equal \n
     *  to it. Internally the neighbor relations are sorted by their pointer address first and \n
     *  by the weight second.*/
    template<class AItem>
    class WeightedNeighborhood {

    private:
      /// Structure keeping a pointer to the item and the weight
      /** Structure that is used as key of the neighbor relations
       *  It keeps a pointer to the item to be stored and uses the pointer address to sort for.
       *  Secondly it also stores the weight for sorting, so that the neighbors to one item are
       *  sorted with increasing weights */
      struct WeightedItemPtr {

        /// Constructor bundeling the original item and the weight of a relation
        WeightedItemPtr(AItem* itemPtr , const NeighborWeight& weight) :
          m_itemPtr(itemPtr), m_weight(weight) {}

        /// Comparison operator establishing a total ordering considering the pointer first and the weight second
        inline bool operator<(const WeightedItemPtr& other) const
        { return m_itemPtr < other.m_itemPtr or (m_itemPtr == other.m_itemPtr and m_weight < other.m_weight); }

        /// Getter for the pointer to the original item
        AItem* getItemPtr() const
        { return m_itemPtr; }

        /// Getter for the weight
        NeighborWeight getWeight() const
        { return m_weight; }

      private:
        AItem* m_itemPtr; ///< Member to store the pointer to the item
        NeighborWeight m_weight; ///< Member to store the weight of the neighbor relation

      };


    private:
      /// Container type that stores the neighborhood relations
      typedef std::vector<std::pair<WeightedItemPtr, AItem*>> Container;


    public:
      /// Value type of the neighborhood aka std::pair<const WeightedItemPtr,NeighborPtr>
      typedef typename Container::value_type value_type;

      /// Better type name for the stored relations.
      typedef value_type WeightedRelation;

    public:
      /// Extracts the item from which the relation points.
      /// Handy in range based for loops in the neighborhood which iterates the relations.
      friend AItem* getItem(const WeightedRelation& relation)
      { return relation.first.getItemPtr(); }

      /// Extracts the weight of the relation.
      /// Handy in range based for loops in the neighborhood which iterates the relations.
      friend NeighborWeight getWeight(const WeightedRelation& relation)
      { return relation.first.getWeight(); }

      /// Extracts the item to which the relation points.
      /// Handy in range based for loops in the neighborhood which iterates the relations.
      friend AItem* getNeighbor(const WeightedRelation& relation)
      { return relation.second; }


    private:
      /// Iterator type of the container to be wrapped by a more expressive iterator.
      typedef typename Container::const_iterator container_const_iterator;

    public:

      /// Expressive iterator type of the neighborhood relations
      /** The iterator type of relations in this neighborhood. It inherits for \n
       *  the underlying container iterator and adds some more expressive names \n
       *  for the value parts in it */
      class iterator : public container_const_iterator {
      public:

        /// Constructor taking the raw iterator to be wrapped
        iterator(const container_const_iterator& it) : container_const_iterator(it) {}

        /// Getter to the pointer to the original item
        AItem* getItem()
        { return (*this)->first.getItemPtr(); }

        /// Getter for the weight assoziated with the neighborhood relation
        NeighborWeight getWeight()
        { return (*this)->first.getWeight(); }

        /// Getter to the pointer to the neighboring item
        AItem* getNeighbor()
        { return (*this)->second; }
      };

      /// Iterator range type for a pair of iterators representing all the neighbors of a specific item.
      using range = Range<iterator>;

    private:
      /// Operator to compare key types WeightedItemPtr to the relations in the vector for lookup.
      friend bool operator<(const WeightedItemPtr& weightedItemPtr,
                            const WeightedRelation& weightedRelation)
      {  return weightedItemPtr < weightedRelation.first; }

      /// Operator to compare key types WeightedItemPtr to the relations in the vector for lookup.
      friend bool operator<(const WeightedRelation& weightedRelation,
                            const WeightedItemPtr& weightedItemPtr)
      {  return weightedRelation.first < weightedItemPtr; }

    private:
      Container m_neighbors; ///< Memory of the neighborhood relations.

    public:

      /** @name Inserting neighbor relations */
      /**@{*/
      /// Inserts a neighborhood relation in to the neighborhood.
      ///inline void insert( const  WeightedRelation & neighborRelation)
      ///{ m_neighbors.insert(neighborRelation); }

      /// Inserts a new relation
      /** Inserts a new relation. The item and neighbor are given by pointer. \n
       *  Copys of the pointers are stored in the underlying container. \n
       *  Additionally a weight can be given. If omited the weight defaults to one. \n
       */
      inline void insert(AItem* itemPtr,
                         AItem* neighborPtr,
                         const NeighborWeight& weight = 1)
      {
        insert(itemPtr, weight, neighborPtr);
      }

      /// Inserts a new relation by pointers
      /** Inserts a new relation. The item and neighbor are given by pointer. \n
       *  Copys of the pointers are stored in the underlying container. \n
       *  Additionally a weight must be given in the middle.
       */
      inline void insert(AItem* itemPtr,
                         const NeighborWeight& weight,
                         AItem* neighborPtr)
      {
        if (not isNotANeighbor(weight)) {
          WeightedRelation newRelation(WeightedItemPtr(itemPtr, weight), neighborPtr);
          auto itInsertAt = std::lower_bound(std::begin(m_neighbors), std::end(m_neighbors), newRelation);
          m_neighbors.insert(itInsertAt, newRelation);
        }
        //assert(std::is_sorted(std::begin(m_neighbors), std::end(m_neighbors)));
      }


      /** @name Retrival of neigbors
       *  Important: Since the underlying container stores pointers the _same_ object address that have been \n
       *  used to store the items have to be used for the retrival.
       *  This should not be a problem since the memory holding the items should still be available. \n
       *  If the memory was already freed the pointers are rendered invalid anyways and now valid object \n
       *  can be retived anymore.
       */
      /**@{*/

      /// Checks if the item given by pointers has neighbors in the neighborhood
      bool hasNeighbors(AItem* itemPtr) const
      { return getLightestNeighbor(itemPtr) != m_neighbors.end(); }

      /// Returns an iterator to the neighbor with the least weight by pointer
      /** If the item given by pointer has neighbors in the neighborhood return \n
       *  an iterator pointing to the one with the smallest weight. \n
       *  If their are no neighbors to the item given return the end() of the neighborhood. */
      iterator getLightestNeighbor(AItem* itemPtr) const
      {
        iterator lowerBound = std::lower_bound(std::begin(m_neighbors), std::end(m_neighbors), WeightedItemPtr(itemPtr, LOWEST_WEIGHT));
        return lowerBound.getItem() == itemPtr ? lowerBound : m_neighbors.end();

      }

      /// Returns an iterator to the neighbor with the highest weight by pointer
      /** If the item given by pointer has neighbors in the neighborhood return \n
       *  an iterator pointing to the one with the highest weight. \n
       *  If their are no neighbors to the item given return the end() of the neighborhood. */
      iterator getHeaviestNeighbor(AItem* itemPtr) const
      {
        iterator upperBound = std::upper_bound(std::begin(m_neighbors), std::end(m_neighbors), WeightedItemPtr(itemPtr, HIGHEST_WEIGHT));
        if (upperBound == m_neighbors.begin()) return m_neighbors.end();
        --upperBound;
        return upperBound.getItem() == itemPtr ? upperBound : m_neighbors.end();

      }

      /// Same as equal_range()
      range getNeighborRange(AItem* itemPtr) const
      { return equal_range(itemPtr); }

      /// Returns an iterator range representing all neighbors.
      /** Returns the range of all neighbors to the item given by pointer as a pair of iterators. \n
       *  The range is sorted internally by the weight of the neighbor from smallest to biggest. \n
       *  If there are no neighbor an empty range will be returned.
       *  @return a pair of iterators for the range of neighbors */
      range equal_range(AItem* itemPtr) const
      {
        iterator lowerBound = std::lower_bound(std::begin(m_neighbors), std::end(m_neighbors), WeightedItemPtr(itemPtr, LOWEST_WEIGHT));
        iterator upperBound = std::upper_bound(std::begin(m_neighbors), std::end(m_neighbors), WeightedItemPtr(itemPtr, HIGHEST_WEIGHT));
        return range(lowerBound, upperBound);
      }

      range equal_range(AItem* itemPtr, const NeighborWeight& weight) const
      { return range(std::equal_range(std::begin(m_neighbors), std::end(m_neighbors), WeightedItemPtr(itemPtr, weight))); }
      /**@}*/

      /// Returns the begin of all neighborhood relations
      iterator begin() const
      { return iterator(m_neighbors.begin()) ;}

      /// Returns the end of all neighborhood relations
      iterator end() const
      { return iterator(m_neighbors.end()) ;}

      /// Returns the number of neighborhood relations
      size_t size()
      { return m_neighbors.size(); }

      /// Drops all stored neighborhood relations
      void clear()
      { m_neighbors.clear(); }

      /// Checks if the two given elements are registered as neighbors with the given weight.
      bool areNeighborsWithWeight(AItem* itemPtr,
                                  const NeighborWeight& weight,
                                  AItem* neighborPtr) const
      {
        for (const WeightedRelation& relation : equal_range(itemPtr, weight)) {
          if (getNeighbor(relation) == neighborPtr) {
            return true;
          }
        }
        return false;
      }

      /// Checks if the two given elements are registered as neighbors with any weight.
      bool areNeighbors(AItem* itemPtr, AItem* neighborPtr) const
      {
        for (const WeightedRelation& relation : equal_range(itemPtr)) {
          if (getNeighbor(relation) == neighborPtr) {
            return true;
          }
        }
        return false;
      }

      /// Checks for the symmetry of the neighborhood
      /** Explicitly checks for each neighborhood relation, if their is an inverse relation \n
       *  with the same weight. Returns true if all such inverse relations exist. */
      bool isSymmetric() const
      {
        bool result = true;
        for (const WeightedRelation& relation : *this) {
          AItem* itemPtr = getItem(relation);
          const NeighborWeight& weight = getWeight(relation);
          AItem* neighborPtr = getNeighbor(relation);

          if (not areNeighborsWithWeight(neighborPtr, weight, itemPtr)) {
            B2WARNING(" Neighborhood is not symmetric in " << *itemPtr <<
                      " to " << *neighborPtr);
            result = false;
          }
        }
        return result;
      }


      /** @name Strutured creation of neighborhoods
        */

      /** Often on faces the problem of having to build a neighborhood relation between elements of the \n
       *  same kind. To find suitable neighbors in a general container it would take an amount of time \n
       *  proportional to n*n to compare all available elements to each other, which is often to long. \n
       *  However if we can sort the sequence we can improve look up speed to an element by a great deal. \n
       *  All tracking hits we want to build neighborhoods for are therefore already made sortable. \n
       *  But the improved look up speed only helps if the neighbors are not scattered around randomly over \n
       *  the sorted range but are close together in a specific section of the range. The time complexity drops \n
       *  than to n*log n + n * m where n is the number of elements in the collection and m the expected number \n
       *  of neighbors.
       *
       *  Since we already sorted out the arrangement of hits during their creation, we have to define \n
       *  the region where to look for neighbors. We keep the general logic for look up the same \n
       *  but vary the definition of what a neighborhood is supposed to be we factor the later out into \n
       *  a strategy object called the RelationFilter with the following interface methods : \n
       *  * getPossibleNeighbors(item, itBegin, itEnd) returns a range iterable object of items which are possible neighbors \n
       *
       *  * isGoodNeighbor(item, neighborItem) checks every neighboring object and returns a weight to indicate the quality of the neighbor.\n
       *    It returns NOT_A_NEIGHBOR (NaN) in case the neighbor is invalid and shall not be saved.\n
       */
      /**@{*/
      /// Clears the neighborhood and creates relations between elements in the given range using an instance of the RelationFilter given as first template argument.
      template<class ARelationFilter, class AItemRange>
      void createUsing(const AItemRange& itemRange)
      {
        clear();
        appendUsing<ARelationFilter>(itemRange);
      }

      /// Clears the neighborhood and creates relations between elements in the given range using the ARelationFilter given as first function argument.
      template<class ARelationFilter, class AItemRange>
      void createUsing(ARelationFilter& relationFilter, AItemRange& itemRange)
      {
        clear();
        appendUsing(relationFilter, itemRange);
      }

      /// Appends relations between elements in the given range using an instance of the ARelationFilter given as first template argument to the existing neighborhood.
      template<class ARelationFilter, class AItemRange>
      void appendUsing(const AItemRange& itemRange)
      {
        ARelationFilter relationFilter;
        appendUsing(relationFilter, itemRange);
      }

      /// Appends relations between elements in the given range using the ARelationFilter given as first function argument to the existing neighborhood.
      template<class ARelationFilter, class AItemRange>
      void appendUsing(ARelationFilter& relationFilter, AItemRange& itemRange)
      {
        Relation<AItem> neighborRelation;
        for (auto& item : itemRange) {
          neighborRelation.first = item;
          for (auto& possibleNeighbor : relationFilter.getPossibleNeighbors(item, std::begin(itemRange), std::end(itemRange))) {
            neighborRelation.second = possibleNeighbor;

            NeighborWeight weight = relationFilter(neighborRelation);

            if (not isNotANeighbor(weight)) {
              // The neighborhood takes references and keeps them
              // Make a push_back here and sort the whole vector afterwards
              m_neighbors.push_back(WeightedRelation(WeightedItemPtr(item, weight), possibleNeighbor));

            }

          } //end for possibleNeighbor

        } // end for item

        // sort everything afterwards
        std::sort(std::begin(m_neighbors), std::end(m_neighbors));
        //assert(std::is_sorted(std::begin(m_neighbors), std::end(m_neighbors)));

      }
      /**@}*/


      /// Output operator to help debugging
      friend std::ostream& operator<<(std::ostream& output, const WeightedNeighborhood& neighborhood)
      {
        for (const WeightedRelation& relation : neighborhood) {
          AItem* const ptrItem = getItem(relation);
          NeighborWeight const weight = getWeight(relation);
          AItem* const ptrNeighbor = getNeighbor(relation);

          output <<
                 ptrItem << " " <<
                 ptrItem->getCellState() << " == " <<
                 ptrItem->getCellWeight() << " + " <<
                 weight << " + " <<
                 ptrNeighbor->getCellState() << " -> " <<
                 ptrNeighbor << std::endl ;
        }
        return output;
      }





    }; //class
  } // end namespace WeightedNeighborhood
} // namespace Belle2
