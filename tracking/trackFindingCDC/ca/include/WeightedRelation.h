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

#include <utility>

#include <tracking/trackFindingCDC/numerics/WithWeight.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type for two related objects of the same type.
    template<class T>
    class WeightedRelation : public std::pair<WithWeight<T*>, T*> {

    private:
      /// Type of the base class
      using Super = std::pair<WithWeight<T*>, T*>;

    public:
      /// Make the constructor of the base type available
      using Super::Super;

      /// Creating a relation with one object on the from side, one on the to side and a weight.
      WeightedRelation(T* from, Weight weight, T* to) :
        Super(WithWeight<T * >(from, weight), to)
      {}

      /// Operator for ordering of relations.
      bool operator<(const WeightedRelation<T>& rhs)
      {
        return (getWeightedFrom() < rhs.getWeightedFrom()) or
               (not(rhs.getWeightedFrom() < getWeightedFrom()) and getTo() < rhs.getTo());
      }

      /// Operator to compare key type weighted item to the relations for assoziative lookups.
      friend bool operator<(const WithWeight<T*>& weightedPtr,
                            const WeightedRelation<T>& weightedRelation)
      { return weightedPtr < weightedRelation.getWeightedFrom(); }

      /// Operator to compare key type weighted item to the relations for assoziative lookups.
      friend bool operator<(const WeightedRelation<T>& weightedRelation,
                            const WithWeight<T*>& weightedPtr)
      { return weightedRelation.getWeightedFrom() < weightedPtr; }

      /// Operator to compare key type item to the relations for assoziative lookups.
      friend bool operator<(T* ptr,
                            const WeightedRelation<T>& weightedRelation)
      { return ptr < weightedRelation.getFrom(); }

      /// Operator to compare key type item to the relations for assoziative lookups.
      friend bool operator<(const WeightedRelation<T>& weightedRelation,
                            T* ptr)
      { return weightedRelation.getFrom() <  ptr; }

      /// Operator for easy unpacking of the relation destination
      operator T* () const
      { return this->second; }

      /// Getter for the pointer to the from side object
      T* getFrom() const
      { return this->first; }

      /// Getter for the pointer to the weighted from side object
      const WithWeight<T*>& getWeightedFrom() const
      { return this->first; }

      /// Getter for the weight
      Weight getWeight() const
      { return this->first.getWeight(); }

      /// Getter for the pointer to the to side object
      T* getTo() const
      { return this->second; }

    };

  } //end namespace TrackFindingCDC
} //end namespace Belle2
