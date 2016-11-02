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

#include <tracking/trackFindingCDC/utilities/SortedRange.h>

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
    class WeightedNeighborhood
      : public SortedRange<typename std::vector<WeightedRelation<AItem> >::const_iterator> {

    private:
      /// Container type that stores the neighborhood relations
      using Super = SortedRange<typename std::vector<WeightedRelation<AItem> >::const_iterator>;

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
      static void appendUsing(ARelationFilter& relationFilter,
                              AItemRange& itemRange,
                              std::vector<WeightedRelation<AItem> >& weightedRelations)
      {
        for (auto && item : itemRange) {
          AItem& itemRef(item);
          AItem* itemPtr = &itemRef; // Relations point to the elements. Take the address of the item here
          auto possibleNeighbors = relationFilter.getPossibleNeighbors(item,
                                   std::begin(itemRange),
                                   std::end(itemRange));
          for (auto && neighbor : possibleNeighbors) {
            AItem& neighborRef(neighbor);
            AItem* neighborPtr = &neighborRef; // Relations point to the elements. Take the address of the item here

            if (neighborPtr == itemPtr) continue;

            Relation<AItem> neighborRelation(itemPtr, neighborPtr);
            Weight weight = relationFilter(neighborRelation);

            if (not std::isnan(weight)) {
              // The neighborhood takes references and keeps them
              // Make a emplace_back here and sort the whole vector afterwards
              weightedRelations.push_back(WeightedRelation<AItem>(neighborRelation.getFrom(),
                                                                  weight,
                                                                  neighborRelation.getTo()));
            }

          } //end for possibleNeighbor

        } // end for item

        // sort everything afterwards
        std::sort(std::begin(weightedRelations), std::end(weightedRelations));
      }
      /**@}*/


    public:
      /// Using the constructors of the base class
      using Super::Super;

      /// Constructor to view a collection of relations as a neighborhood.
      explicit WeightedNeighborhood(const std::vector<WeightedRelation<AItem> >& weightedRelations)
        : Super(weightedRelations.begin(), weightedRelations.end())
      {
      }

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
      { return getLightestNeighbor(itemPtr) != this->end(); }

      /// Checks if the two given elements are registered as neighbors with the given weight.
      bool areNeighborsWithWeight(AItem* itemPtr,
                                  Weight weight,
                                  AItem* neighborPtr) const
      {
        for (const WeightedRelation<AItem>& relation
             : this->equal_range(std::make_pair(itemPtr, weight))) {
          if (relation.getTo() == neighborPtr and relation.getWeight() == weight) {
            return true;
          }
        }
        return false;
      }

      /// Checks if the two given elements are registered as neighbors with any weight.
      bool areNeighbors(AItem* itemPtr, AItem* neighborPtr) const
      {
        for (const WeightedRelation<AItem>& relation : this->equal_range(itemPtr)) {
          if (relation.getTo() == neighborPtr) {
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

    };
  }
}
