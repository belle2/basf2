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

#include <tracking/trackFindingCDC/ca/WeightedRelation.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <tracking/trackFindingCDC/utilities/Range.h>

#include <vector>
#include <utility>
#include <iostream>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Class representing weighted relations between items of the same type.
     *  The WeightedNeighborhood implements a weighted graph of items of the same types \n
     *  This structure does not store any items but solely the pointers to them. \n
     *  Therefore the memory has to be provided by other means and should not be moved \n
     *  as long as you want to access the neighbors of items via this object. Also the \n
     *  retrival works solely by the memory address of the items. So you have to give \n
     *  the \a same objects for retrival of neighbors and not just one comparing equal \n
     *  to it. Internally the neighbor relations are sorted by their pointer address first and \n
     *  by the weight second.
     */
    template<class AItem>
    class WeightedNeighborhood {

    private:
      /// Container type that stores the neighborhood relations
      typedef std::vector<WeightedRelation<AItem> > Container;

    public:
      /// Iterator type into the relations
      using iterator = typename Container::const_iterator;

      /// Iterator range type for a pair of iterators representing all the neighbors of a specific item.
      using range = Range<iterator>;

    public:
      /**
       *  @name Strutured creation of neighborhoods
       *
       *  Often on faces the problem of having to build a neighborhood relation between elements of the \n
       *  same kind. To find suitable neighbors in a general container it would take an amount of time \n
       *  proportional to n*n to compare all available elements to each other, which is often to long. \n
       *  However if we can sort the sequence we can improve look up speed to an element by a great deal. \n
       *  All tracking hits we want to build neighborhoods for are therefore already made sortable. \n
       *  But the improved look up speed only helps if the neighbors are not scattered around randomly over \n
       *  the sorted range but are close together in a specific section of the range. The time complexity drops \n
       *  than to n*log n + n * m, where n is the number of elements in the collection and m the expected number \n
       *  of neighbors.
       *
       *  Since we already sorted out the arrangement of hits during their creation, we have to define \n
       *  the region where to look for neighbors. We keep the general logic for look up the same \n
       *  but vary the definition of what a neighborhood is supposed to be we factor the later out into \n
       *  a strategy object called the RelationFilter with the following interface methods : \n
       *  * getPossibleNeighbors(item, itBegin, itEnd) returns a range iterable object of items
       *    which are possible neighbors \n
       *
       *  * operator(Relation<AItem>) checks every neighboring object and returns a weight to
       *    indicate the quality of the neighbor. \n
       *    It returns NOT_A_NEIGHBOR (NaN) in case the neighbor is invalid and shall not be saved.\n
       */
      /**@{*/

      /// Appends relations between elements in the given AItemRange using the ARelationFilter.
      template<class ARelationFilter, class AItemRange>
      void appendUsing(ARelationFilter& relationFilter, AItemRange& itemRange)
      {
        //Relation<AItem> neighborRelation;
        for (auto && item : itemRange) {
          AItem& itemRef(item);
          AItem* itemPtr = &itemRef; // Relations point to the elements. Take the address of the item here
          //neighborRelation.setFrom(itemPtr);
          auto possibleNeighbors = relationFilter.getPossibleNeighbors(item, std::begin(itemRange), std::end(itemRange));
          for (auto && neighbor : possibleNeighbors) {
            AItem& neighborRef(neighbor);
            AItem* neighborPtr = &neighborRef; // Relations point to the elements. Take the address of the item here
            //neighborRelation.setTo(neighborPtr);

            Relation<AItem> neighborRelation(itemPtr, neighborPtr);
            Weight weight = relationFilter(neighborRelation);

            if (not std::isnan(weight)) {
              // The neighborhood takes references and keeps them
              // Make a emplace_back here and sort the whole vector afterwards
              m_weightedNeighbors.push_back(WeightedRelation<AItem>(neighborRelation.getFrom(),
                                                                    weight,
                                                                    neighborRelation.getTo()));
            }

          } //end for possibleNeighbor

        } // end for item

        // sort everything afterwards
        std::sort(std::begin(m_weightedNeighbors), std::end(m_weightedNeighbors));
      }
      /**@}*/

      /**
       * @name Inserting neighbor relations
       */
      /**@{*/

      /**
       *  Inserts a new relation. The item and neighbor are given by pointer. \n
       *  Copys of the pointers are stored in the underlying container. \n
       *  Additionally a weight can be given. If omited the weight defaults to one. \n
       */
      inline void insert(AItem* itemPtr,
                         AItem* neighborPtr,
                         Weight weight = 1)
      {
        insert(itemPtr, weight, neighborPtr);
      }

      /**
       *  Inserts a new relation.
       *  The item and neighbor are given by pointer. \n
       *  Copys of the pointers are stored in the underlying container. \n
       *  Additionally a weight must be given in the middle.
       */
      inline void insert(AItem* itemPtr,
                         Weight weight,
                         AItem* neighborPtr)
      {
        if (not std::isnan(weight)) {
          WeightedRelation<AItem> newRelation(itemPtr, weight, neighborPtr);
          auto itInsertAt = std::lower_bound(std::begin(m_weightedNeighbors), std::end(m_weightedNeighbors), newRelation);
          m_weightedNeighbors.insert(itInsertAt, newRelation);
        }
      }
      /**@}*/

      /**
       *  @name Retrival of neigbors
       *  Important: Since the underlying container stores pointers the _same_ object address that have been \n
       *  used to store the items have to be used for the retrival.
       *  This should not be a problem since the memory holding the items should still be available. \n
       *  If the memory was already freed the pointers are rendered invalid anyways and now valid object \n
       *  can be retived anymore.
       */
      /**@{*/

      /// Checks if the item given by pointers has neighbors in the neighborhood
      bool hasNeighbors(AItem* itemPtr) const
      { return getLightestNeighbor(itemPtr) != m_weightedNeighbors.end(); }

      /**
       *  Returns an iterator to the neighbor relation with the lowest weight.
       *  If the given item pointer has neighbors in the neighborhood return \n
       *  an iterator pointing to the one with the smallest weight. \n
       *  If their are no neighbors to the item given return the end() of the neighborhood.
       */
      iterator getLightestNeighbor(AItem* itemPtr) const
      {
        iterator lowerBound = std::lower_bound(std::begin(m_weightedNeighbors),
                                               std::end(m_weightedNeighbors),
                                               itemPtr);
        return lowerBound.getItem() == itemPtr ? lowerBound : m_weightedNeighbors.end();
      }

      /**
       *  Returns an iterator to the neighbor relation with the highest weight.
       *  If the given item pointer has neighbors in the neighborhood return \n
       *  an iterator pointing to the one with the highest weight. \n
       *  If their are no neighbors to the item given return the end() of the neighborhood.
       */
      iterator getHeaviestNeighbor(AItem* itemPtr) const
      {
        iterator upperBound = std::upper_bound(std::begin(m_weightedNeighbors),
                                               std::end(m_weightedNeighbors),
                                               itemPtr);
        if (upperBound == m_weightedNeighbors.begin()) return m_weightedNeighbors.end();
        --upperBound;
        return upperBound.getItem() == itemPtr ? upperBound : m_weightedNeighbors.end();
      }

      /// Same as equal_range()
      range getNeighborRange(AItem* itemPtr) const
      { return this->equal_range(itemPtr); }

      /**
       *  Returns an iterator range representing all neighbors.
       *  Returns the range of all neighbors to the item given by pointer as a pair of iterators. \n
       *  The range is sorted internally by the weight of the neighbor from smallest to biggest. \n
       *  If there are no neighbor an empty range will be returned.
       *  @return a pair of iterators for the range of neighbors
       */
      range equal_range(AItem* itemPtr) const
      {
        return range(std::equal_range(std::begin(m_weightedNeighbors),
                                      std::end(m_weightedNeighbors),
                                      itemPtr));
      }

      range equal_range(AItem* itemPtr, Weight weight) const
      {
        return range(std::equal_range(std::begin(m_weightedNeighbors),
                                      std::end(m_weightedNeighbors),
                                      WithWeight<AItem*>(itemPtr, weight)));
      }
      /**@}*/

      /// Returns the begin of all neighborhood relations
      iterator begin() const
      { return iterator(m_weightedNeighbors.begin()); }

      /// Returns the end of all neighborhood relations
      iterator end() const
      { return iterator(m_weightedNeighbors.end()); }

      /// Returns the number of neighborhood relations
      size_t size()
      { return m_weightedNeighbors.size(); }

      /// Drops all stored neighborhood relations
      void clear()
      { m_weightedNeighbors.clear(); }

      /// Checks if the two given elements are registered as neighbors with the given weight.
      bool areNeighborsWithWeight(AItem* itemPtr,
                                  Weight weight,
                                  AItem* neighborPtr) const
      {
        for (const WeightedRelation<AItem>& relation : equal_range(itemPtr, weight)) {
          if (relation.getTo() == neighborPtr) {
            return true;
          }
        }
        return false;
      }

      /// Checks if the two given elements are registered as neighbors with any weight.
      bool areNeighbors(AItem* itemPtr, AItem* neighborPtr) const
      {
        for (const WeightedRelation<AItem>& relation : equal_range(itemPtr)) {
          if (relation.getTo == neighborPtr) {
            return true;
          }
        }
        return false;
      }

      /**
       *  Checks for the symmetry of the neighborhood
       *  Explicitly checks for each neighborhood relation, if their is an inverse relation \n
       *  with the same weight. Returns true if all such inverse relations exist.
       */
      bool isSymmetric() const
      {
        bool result = true;
        for (const WeightedRelation<AItem>& relation : *this) {
          AItem* itemPtr = relation.getFrom();
          Weight weight = relation.getWeight();
          AItem* neighborPtr = relation.getTo();

          if (not areNeighborsWithWeight(neighborPtr, weight, itemPtr)) {
            B2WARNING(" Neighborhood is not symmetric in " << *itemPtr <<
                      " to " << *neighborPtr);
            result = false;
          }
        }
        return result;
      }

    private:
      /// Memory of the neighborhood relations.
      Container m_weightedNeighbors;

    }; // class WeightedNeighborhood
  } // namespace TrackFindingCDC
} // namespace Belle2
