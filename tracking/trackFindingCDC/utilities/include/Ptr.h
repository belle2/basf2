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

    /**
     *  Class that behaves like T*.
     *  Needed because you cannot inherit from T* directly in
     *  a mixin based inheritance structure.
     *  I cannot believe I have to write this in C++ though...
     */
    template<class T>
    class Ptr {
    public:
      /// Wrap the pointer to an object.
      explicit Ptr(T* obj) : m_obj(obj)
      {}

      /// Wrap the reference to an object. Constructing a pointer from a reference takes the address.
      explicit Ptr(T& obj) : m_obj(&obj)
      {}

      /// Allow decay of the referenced object type from non-const to const.
      operator Ptr<const T>() const
      { return Ptr<const T>(m_obj); }

      /// Allow explicit unpacking.
      explicit operator T*& ()&
      { return m_obj; }

      /// Allow explicit unpacking.
      explicit operator T* const& () const&
      { return m_obj; }

      /// Allow automatic from temporary return values unpacking.
      operator T* ()&&
      { return m_obj; }

      /// Mimic the original item pointer access.
      T* operator->() const
      { return m_obj;}

      /// Dereferencing the pointer
      T& operator*() const
      { return *m_obj; }

      /// Transport ordering of pointer
      bool operator<(const Ptr<T>& rhs) const
      { return m_obj < rhs.m_obj; }

      /// Transport equality of pointer
      bool operator==(const Ptr<T>& rhs) const
      { return m_obj == rhs.m_obj; }

    private:
      /// Reference to the marked items.
      T* m_obj;
    };

    /// Helper type function to replace a T* with Ptr<T> when needed.
    template<class T>
    struct ReplaceStarWithPtrImpl {
      /// Base implementation just forwards the original type.
      using Type = T;
    };

    /// Helper type function to replace a T* with Ptr<T> when needed.
    template<class T>
    struct ReplaceStarWithPtrImpl<T*> {
      /// Specilisation replaces T* with Ptr<T>.
      using Type = Ptr<T>;
    };

    /// Helper type function to replace a T* with Ptr<T> when needed.
    template<class T>
    using StarToPtr = typename ReplaceStarWithPtrImpl<T>::Type;

  }
}
