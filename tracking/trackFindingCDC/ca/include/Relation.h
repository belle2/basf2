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

    /// Type for two related objects of the same type.
    template<class T>
    class Relation
      : public std::pair<T*, T*> {

    private:
      /// Type of the base class
      using Super = std::pair<T*, T*>;

    public:
      /// Make the constructor of the base type available
      using Super::Super;

      /// Operator to compare key type item to the relations for assoziative lookups.
      friend bool operator<(T* ptr,
                            const Relation<T>& relation)
      { return ptr < relation.getFrom(); }

      /// Operator to compare key type item to the relations for assoziative lookups.
      friend bool operator<(const Relation<T>& relation,
                            T* ptr)
      { return relation.getFrom() < ptr; }

      /// Getter for the pointer to the from side object
      T* getFrom() const
      { return this->first; }

      /// Getter for the pointer to the to side object
      T* getTo() const
      { return this->second; }

    };

  }
}
