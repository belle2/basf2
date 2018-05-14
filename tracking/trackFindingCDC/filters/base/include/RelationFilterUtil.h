/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <framework/logging/LogMethod.h>

#include <vector>
#include <limits>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {

    struct RelationFilterUtil {
      /**
       *  @name Strutured creation of neighborhoods
       *
       *  Often one faces the problem of having to build a graph between elements of the \n
       *  same kind. To find suitable neighbors in a general container it would take an amount of time \n
       *  proportional to n*n to compare all available elements to each other, which is often to long. \n
       *  However if we can sort the sequence we can improve look up speed to an element by a great deal. \n
       *  All tracking hits / segments / tracks for which we want to build graphs are therefore made sortable. \n
       *  But the improved look up speed only helps if the neighbors are not scattered around randomly over \n
       *  the sorted range but are close together in a specific section of the range. The time complexity drops \n
       *  than to n*log n + n * m, where n is the number of elements in the collection and m the expected number \n
       *  of neighbors.
       *
       *  Since we already sorted out the arrangement of hits / segments / tracks during their creation, we have to define \n
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
      template <class AObject, class ARelationFilter>
      static void appendUsing(ARelationFilter& relationFilter,
                              const std::vector<AObject*>& froms,
                              const std::vector<AObject*>& tos,
                              std::vector<WeightedRelation<AObject>>& weightedRelations,
                              unsigned int maximumNumberOfRelations = std::numeric_limits<unsigned int>::max())
      {
        for (AObject* from : froms) {
          std::vector<AObject*> possibleTos = relationFilter.getPossibleTos(from, tos);

          for (AObject* to : possibleTos) {
            if (from == to) continue;
            Relation<AObject> relation(from, to);
            Weight weight = relationFilter(relation);
            if (std::isnan(weight)) continue;
            weightedRelations.emplace_back(from, weight, to);

            if (weightedRelations.size() == maximumNumberOfRelations) {
              B2WARNING("Relations Creator reached maximal number of items. Aborting");
              weightedRelations.clear();
              return;
            }
          }
        }
        // sort everything afterwards
        std::sort(std::begin(weightedRelations), std::end(weightedRelations));
      }
      /**@}*/

      /// Shortcut for applying appendUsing with froms=tos
      template <class AObject, class ARelationFilter>
      static void appendUsing(ARelationFilter& relationFilter,
                              const std::vector<AObject*>& objects,
                              std::vector<WeightedRelation<AObject>>& weightedRelations)
      {
        appendUsing(relationFilter, objects, objects, weightedRelations);
      };
    };
  }
}
