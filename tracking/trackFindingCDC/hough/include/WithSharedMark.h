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

namespace Belle2 {
  namespace TrackFindingCDC {
    template<class T>
    class WithSharedMark : public T {
    public:
      /// Mixin class Wraps an object including r the mark is provided from a shared position.
      WithSharedMark(const T& obj, bool* isMarked) : T(obj), m_isMarked(isMarked)
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
