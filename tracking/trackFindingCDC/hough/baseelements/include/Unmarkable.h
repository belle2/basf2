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

#include <tracking/trackFindingCDC/utilities/Scalar.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Mixin class to attach a mark that is shared among many instances.
    template<class T>
    class Unmarkable : public ScalarToClass<T> {
    private:
      /// Base class of the mixin
      using Super = ScalarToClass<T>;

    public:
      /// Mixin class to make an object unmarkable (the mark information is saved - but never used in the tree
      /// by intention)
      Unmarkable(const T& obj, bool* isMarked)
        : Super(obj)
        , m_isMarked(isMarked)
      {
      }

      /// Check if the object has been marked. As we do not want this object to be marked, just return false.
      bool isMarked() const
      {
        return false;
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
