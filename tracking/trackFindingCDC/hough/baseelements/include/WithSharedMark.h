/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/Scalar.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Mixin class to attach a mark that is shared among many instances.
    template<class T>
    class WithSharedMark : public ScalarToClass<T> {
    private:
      /// Base class of the mixin
      using Super = ScalarToClass<T>;

    public:
      /// Mixin class Wraps an object including r the mark is provided from a shared position.
      WithSharedMark(const T& obj, bool* isMarked)
        : Super(obj)
        , m_isMarked(isMarked)
      {
      }

      /// Check if the object has been marked
      bool isMarked() const
      {
        return *m_isMarked;
      }

      /// Mark this object
      void mark(bool mark = true)
      {
        *m_isMarked = mark;
      }

      /// Unmark this object
      void unmark()
      {
        *m_isMarked = false;
      }

    private:
      /// Reference to the shared marks.
      bool* m_isMarked;
    };
  }
}
