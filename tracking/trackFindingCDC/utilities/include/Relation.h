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

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type for two related objects
    template <class AFrom, class ATo = AFrom>
    class Relation : public std::pair<AFrom*, ATo*> {

    private:
      /// Type of the base class
      using Super = std::pair<AFrom*, ATo*>;

    public:
      /// Type of from which the relation originates.
      using From = AFrom;

      /// Type of to which the relation points
      using To = ATo;

    public:
      /// Make the constructor of the base type available
      using Super::Super;

      /// Operator for ordering of relations.
      bool operator<(const Relation<From, To>& rhs) const
      {
        return (getFrom() < rhs.getFrom() or
                (not(rhs.getFrom() < getFrom()) and
                 (getTo() < rhs.getTo())));
      }

      /// Operator to compare key type item to the relations for assoziative lookups.
      friend bool operator<(const From* ptrFrom, const Relation<From, To>& relation)
      {
        return ptrFrom < relation.getFrom();
      }

      /// Operator to compare key type item to the relations for assoziative lookups.
      friend bool operator<(const Relation<From, To>& relation, const From* ptrFrom)
      {
        return relation.getFrom() < ptrFrom;
      }

      /// Getter for the pointer to the from side object
      From* getFrom() const
      {
        return this->first;
      }

      /// Getter for the pointer to the to side object
      To* getTo() const
      {
        return this->second;
      }

      /// Make a relation in the opposite direciton with the same weight
      Relation<To, From> reversed() const
      {
        return Relation<To, From>(getTo(), getFrom());
      }
    };
  }
}
