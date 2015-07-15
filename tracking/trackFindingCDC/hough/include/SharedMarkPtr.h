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
    class SharedMarkPtr {
    public:
      /// Wrap the object. Memory for the mark is provided from a shared position.
      SharedMarkPtr(T* obj, bool* isMarked) : m_obj(obj), m_isMarked(isMarked)
      {
      }

      /// Allow automatic unpacking.
      operator T* ()
      { return m_obj;}

      /// Allow automatic unpacking.
      operator T const* () const
      { return m_obj; }

      /// Mimic the original item pointer access.
      T* operator->()
      { return m_obj;}

      /// Mimic the original item pointer access.
      const T* operator->() const
      { return m_obj;}

      /// Check if the object has been marked
      bool isMarked() const { return *m_isMarked; }

      /// Mark this object
      void mark(bool mark = true) { *m_isMarked = mark; }

      /// Unmark this object
      void unmark() { *m_isMarked = false; }

    private:
      /// Reference to the marked items.
      T* m_obj;

      /// Reference to the shared marks.
      bool* m_isMarked;
    };

  }
}
