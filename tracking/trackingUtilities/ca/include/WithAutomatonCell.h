/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>
#include <tracking/trackFindingCDC/utilities/Scalar.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Mixin class to attach an automaton cell to an object or pointer.
    template<class T>
    class WithAutomatonCell : public ScalarToClass<T> {

    private:
      /// Base class of the mixin
      using Super = ScalarToClass<T>;

    public:
      /// Mixin class wraps an object a
      explicit WithAutomatonCell(const T& obj)
        : Super(obj)
        , m_automatonCell()
      {
      }

      /// Mutable getter for the automaton cell
      AutomatonCell& getAutomatonCell() const
      {
        return m_automatonCell;
      }

      /// Mockup as we do not have any sub-items
      void unsetAndForwardMaskedFlag() const
      {
        getAutomatonCell().unsetMaskedFlag();
      }

      /// Mockup as we do not have any sub-items
      void setAndForwardMaskedFlag() const
      {
        getAutomatonCell().setMaskedFlag();
      }

      /// Mockup as we do not have any sub-items
      void receiveMaskedFlag() const {}

    private:
      /// Memory for the automaton cell
      mutable AutomatonCell m_automatonCell;
    };
  }
}
