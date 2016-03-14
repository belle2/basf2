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

#include <tracking/trackFindingCDC/utilities/Ptr.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Mixin class to attach a mark that is shared among many instances.
    template<class T>
    class WithSharedMark : public StarToPtr<T> {
    private:
      /// Base class of the mixin
      using Super = StarToPtr<T>;

    public:
      /// Mixin class Wraps an object including r the mark is provided from a shared position.
      WithSharedMark(const T& obj, bool* isMarked) : Super(obj), m_isMarked(isMarked)
      {;}

      /// Check if the object has been marked
      bool isMarked() const { return *m_isMarked; }

      /// Mark this object
      void mark(bool mark = true) { *m_isMarked = mark; }

      /// Unmark this object
      void unmark() { *m_isMarked = false; }

    private:
      /// Reference to the shared marks.
      bool* m_isMarked;
    };
  }
}
