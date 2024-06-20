/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>

#include <tracking/ckf/pxd/entities/CKFToPXDState.h>
#include <tracking/ckf/svd/entities/CKFToSVDState.h>

#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>

#include <framework/logging/Logger.h>

#include <vector>
#include <limits>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  name Structured creation of neighborhoods
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
    struct RelationFilterUtil {
      /* *@{*/
      /** Appends relations between elements in the given AItems using the ARelationFilter. */
      template <class AObject, class ARelationFilter>
      static void appendUsing(ARelationFilter& relationFilter,
                              const std::vector<AObject*>& froms,
                              const std::vector<AObject*>& tos,
                              std::vector<WeightedRelation<AObject>>& weightedRelations,
                              unsigned int maximumNumberOfRelations = std::numeric_limits<unsigned int>::max())
      {
        for (AObject* from : froms) {
          StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo;

          std::vector<AObject*> possibleTos = relationFilter.getPossibleTos(from, tos);

          for (AObject* to : possibleTos) {
            if (from == to) continue;
            Relation<AObject> relation(from, to);
            Weight weight = relationFilter(relation);
            if (std::isnan(weight)) continue;
            weightedRelations.emplace_back(from, weight, to);

            if (weightedRelations.size() == maximumNumberOfRelations) {
              B2WARNING("Relations Creator reached maximal number of items: skipping the event.");
              if (m_eventLevelTrackingInfo.isValid()) {
                if (std::is_base_of<AObject, CKFToPXDState>::value) {
                  m_eventLevelTrackingInfo->setPXDCKFAbortionFlag();
                } else if (std::is_base_of<AObject, CKFToSVDState>::value) {
                  m_eventLevelTrackingInfo->setSVDCKFAbortionFlag();
                } else if (std::is_base_of<AObject, vxdHoughTracking::VXDHoughState>::value) {
                  B2INFO("Skipping processing VXDHoughTracking track candidate, not setting AbortionFlag.");
                } else {
                  B2WARNING("Undefined class used for CKFStates. Could not set AbortionFlag.");
                }
              }

              weightedRelations.clear();
              return;
            }
          }
        }
        // sort everything afterwards
        std::sort(std::begin(weightedRelations), std::end(weightedRelations));
      }
      /* *@}*/

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
