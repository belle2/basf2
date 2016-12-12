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
       *    It returns NaN in case the neighbor is invalid and shall not be saved.\n
       */
      /**@{*/
      /// Appends relations between elements in the given AItems using the ARelationFilter.
      template <class ARelationFilter, class AItems>
      static void appendUsing(ARelationFilter& relationFilter,
                              AItems& items,
                              std::vector<WeightedRelation<AItem>>& weightedRelations)
      {
        for (AItem& from : items) {
          auto possibleNeighbors =
            relationFilter.getPossibleNeighbors(from, std::begin(items), std::end(items));
          for (AItem& to : possibleNeighbors) {
            // Relations point to the elements. Take the address of the item here
            AItem* ptrFrom = &from;
            AItem* ptrTo = &to;
            if (ptrFrom == ptrTo) continue;
            Relation<AItem> neighborRelation(ptrFrom, ptrTo);
            Weight weight = relationFilter(neighborRelation);
            if (std::isnan(weight)) continue;
            weightedRelations.emplace_back(ptrFrom, weight, ptrTo);
          } // end for from
        } // end for to
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
    };
  }
}
