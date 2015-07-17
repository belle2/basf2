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

    /** Class that behaves like T*.
     *  Needed because you cannot inherit from T* directly in
     *  a mixin based inheritance structure.
     *  I cannot believe I have to write this in C++ though...
     */
    template<class T>
    class Ptr {
    public:
      /// Wrap the object. Memory for the mark is provided from a shared position.
      Ptr(T* obj) : m_obj(obj)
      {;}

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

    private:
      /// Reference to the marked items.
      T* m_obj;
    };
  }
}
