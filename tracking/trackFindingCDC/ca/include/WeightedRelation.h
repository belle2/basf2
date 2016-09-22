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
#include <utility>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type for two related objects of the same type with a weight.
    template<class T>
    class WeightedRelation : public std::pair<std::pair<T*, Weight>, T*> {

      /// Type of the base class
      using Super = std::pair<std::pair<T*, Weight>, T*>;

    public:
      /// Default constructor
      WeightedRelation()
        : Super(std::pair<T *, Weight>(nullptr, NAN), nullptr)
      {}

      /// Creating a relation with one object on the from side, one on the to side and a weight.
      WeightedRelation(T* from, Weight weight, T* to)
        : Super(std::pair<T *, Weight>(from, weight), to)
      {}

      /// Operator for ordering of relations.
      bool operator<(const WeightedRelation<T>& rhs)
      {
        return (getFrom() < rhs.getFrom() or
                (not(rhs.getFrom() < getFrom()) and
                 // highest weight first
                 (getWeight() > rhs.getWeight() or
                  (not(rhs.getWeight() > getWeight()) and
                   (getTo() < rhs.getTo())))));
      }

      /// Operator to compare key type weighted item to the relations for assoziative lookups.
      friend bool operator<(const std::pair<T*, Weight>& weightedPtr,
                            const WeightedRelation<T>& weightedRelation)
      {
        return (weightedPtr.first < weightedRelation.getFrom() or
                (not(weightedRelation.getFrom() < weightedPtr.first) and
                 // highest weight first
                 (weightedPtr.second > weightedRelation.getWeight())));
      }

      /// Operator to compare key type weighted item to the relations for assoziative lookups.
      friend bool operator<(const WeightedRelation<T>& weightedRelation,
                            const std::pair<T*, Weight>& weightedPtr)
      {
        return (weightedRelation.getFrom() < weightedPtr.first or
                (not(weightedPtr.first < weightedRelation.getFrom()) and
                 // highest weight first
                 (weightedRelation.getWeight() > weightedPtr.second)));
      }

      /// Operator to compare key type item to the relations for assoziative lookups.
      friend bool operator<(T* ptr,
                            const WeightedRelation<T>& weightedRelation)
      { return ptr < weightedRelation.getFrom(); }

      /// Operator to compare key type item to the relations for assoziative lookups.
      friend bool operator<(const WeightedRelation<T>& weightedRelation,
                            T* ptr)
      { return weightedRelation.getFrom() < ptr; }

      /// Getter for the pointer to the from side object
      T* getFrom() const
      { return getWeightedFrom().first; }

      /// Getter for the weight
      Weight getWeight() const
      { return getWeightedFrom().second; }

      /// Getter for the pointer to the weighted from side object
      const std::pair<T*, Weight>& getWeightedFrom() const
      { return this->first; }

      /// Getter for the pointer to the to side object
      T* getTo() const
      { return this->second; }

      /// Make a relation in the opposite direciton with the same weight
      WeightedRelation<T> reversed() const
      { return WeightedRelation<T>(getTo(), getWeight(), getFrom()); }

    };

  } // namespace TrackFindingCDC
} // namespace Belle2
