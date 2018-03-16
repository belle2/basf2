/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/GetValueType.h>

#include <algorithm>
#include <utility>
#include <type_traits>
#include <cassert>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type for two related objects with a weight.
    template <class AFrom, class ATo = AFrom>
    class WeightedRelation : public std::pair<std::pair<AFrom*, Weight>, ATo*> {

      /// Type of the base class
      using Super = std::pair<std::pair<AFrom*, Weight>, ATo*>;

    public:
      /// Type of from which the relation originates.
      using From = AFrom;

      /// Type of to which the relation points
      using To = ATo;

    public:
      /// Default constructor
      WeightedRelation() = default;

      /// Creating a relation with one object on the from side, one on the to side and a weight.
      WeightedRelation(From* from, Weight weight, To* to)
        : Super( {from, weight}, to)
      {}

      /// Operator for ordering of relations.
      bool operator<(const WeightedRelation<From, To>& rhs) const
      {
        return (getFrom() < rhs.getFrom() or
                (not(rhs.getFrom() < getFrom()) and
                 // highest weight first
                 (getWeight() > rhs.getWeight() or
                  (not(rhs.getWeight() > getWeight()) and
                   (getTo() < rhs.getTo())))));
      }

      /// Operator to compare key type weighted item to the relations for assoziative lookups.
      friend bool operator<(const std::pair<From*, Weight>& weightedPtr,
                            const WeightedRelation<From, To>& weightedRelation)
      {
        return (weightedPtr.first < weightedRelation.getFrom() or
                (not(weightedRelation.getFrom() < weightedPtr.first) and
                 // highest weight first
                 (weightedPtr.second > weightedRelation.getWeight())));
      }

      /// Operator to compare key type weighted item to the relations for assoziative lookups.
      friend bool operator<(const WeightedRelation<From, To>& weightedRelation,
                            const std::pair<From*, Weight>& weightedPtr)
      {
        return (weightedRelation.getFrom() < weightedPtr.first or
                (not(weightedPtr.first < weightedRelation.getFrom()) and
                 // highest weight first
                 (weightedRelation.getWeight() > weightedPtr.second)));
      }

      /// Operator to compare key type item to the relations for assoziative lookups.
      friend bool operator<(const From* ptrFrom, const WeightedRelation<From, To>& weightedRelation)
      {
        return ptrFrom < weightedRelation.getFrom();
      }

      /// Operator to compare key type item to the relations for assoziative lookups.
      friend bool operator<(const WeightedRelation<From, To>& weightedRelation, const From* ptrFrom)
      {
        return weightedRelation.getFrom() < ptrFrom;
      }

      /// Getter for the pointer to the from side object
      From* getFrom() const
      {
        return getWeightedFrom().first;
      }

      /// Getter for the weight
      Weight getWeight() const
      {
        return getWeightedFrom().second;
      }

      /// Setter for the weight
      void setWeight(Weight weight)
      {
        this->first.second = weight;
      }

      /// Getter for the pointer to the weighted from side object
      const std::pair<From*, Weight>& getWeightedFrom() const
      {
        return this->first;
      }

      /// Getter for the pointer to the to side object
      To* getTo() const
      {
        return this->second;
      }

      /// Make a relation in the opposite direciton with the same weight
      WeightedRelation<To, From> reversed() const
      {
        //return WeightedRelation<To, From>(getTo(), getWeight(), getFrom());
        return {getTo(), getWeight(), getFrom()};
      }
    };

    /// Utility structure with functions related to weighted relations
    template <class AFrom, class ATo = AFrom>
    struct WeightedRelationUtil {
      /**
       *  Checks for the symmetry of a range of weighted relations
       *  Explicitly checks for each weighted relation, if their is an reverse relation
       *  with the same weight. Returns true if all such inverse relations exist.
       */
      template <class AWeightedRelations>
      static bool areSymmetric(const AWeightedRelations& weightedRelations)
      {
        static_assert(std::is_same<AFrom, ATo>::value, "Symmetric check requires some types in From and To");
        assert(std::is_sorted(std::begin(weightedRelations), std::end(weightedRelations)));
        auto reversedRelationExists =
        [&weightedRelations](const WeightedRelation<AFrom, ATo>& weightedRelation) -> bool {
          auto reversedRelations = std::equal_range(std::begin(weightedRelations),
          std::end(weightedRelations),
          weightedRelation.reversed());
          return reversedRelations.first != reversedRelations.second;
        };
        return std::all_of(std::begin(weightedRelations),
                           std::end(weightedRelations),
                           reversedRelationExists);
      }
    };
  }
}
