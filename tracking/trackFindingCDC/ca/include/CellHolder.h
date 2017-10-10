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

#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/IsApplicable.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class AutomatonCell;

    /// Getter functor for the automaton cell of an object
    struct GetAutomatonCell {
      /// Returns the automaton cell of an object.
      template<class T, class SFINAE = decltype(&T::getAutomatonCell)>
      AutomatonCell & operator()(T& t)
      {
        return t.getAutomatonCell();
      }
    };

    /**
     *  Predicate to check for the CellHolder concept
     *
     *  Sole requirement is that it must support .getAutomatonCell()
     *  which should return a reference to an AutomatonCell.
     */
    template <class T>
    constexpr bool isCellHolder()
    {
      return isApplicable<GetAutomatonCell, T&>();
    }

    /// Helper function to statically assert for the CellHolder concept.
    template <class T>
    void static_assert_isCellHolder()
    {
      static_assert(isCellHolder<T>(), "Template argument must support the CellHolder concept, "
                    "i.e. it must support cellHolder.getAutomatonCell()");
    }
  }
}
