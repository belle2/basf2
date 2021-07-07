/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Class wrapping a scalar type (for which std::is_scalar is true).
     *
     *  Needed because you cannot inherit from scalar type directly in
     *  a mixin based inheritance structure.
     *  It does not support all the basic operations, but transperently unpacks
     *  if conversion to the underlying type is requested by the using context.
     */
    template<class T>
    class Scalar {
    public:
      /// Wrap scalar type as an object.
      explicit Scalar(T obj)
        : m_obj(obj)
      {
      }

      /// Allow unpacking.
      operator T& ()& {
        return m_obj;
      }

      /// Allow const unpacking.
      operator T const& () const&
      {
        return m_obj;
      }

      /// Mimic the original item pointer access for the cast that T is a pointer
      T operator->() const
      {
        return m_obj;
      }

      /// Dereferencing access for the case that T is a pointer
      typename std::remove_pointer<T>::type& operator*() const
      {
        return *m_obj;
      }

      /// Transport ordering
      bool operator<(const Scalar<T>& rhs) const
      {
        return m_obj < rhs.m_obj;
      }

      /// Transport equality
      bool operator==(const Scalar<T>& rhs) const
      {
        return m_obj == rhs.m_obj;
      }

    private:
      /// Memory for the underlying scalar type.
      T m_obj;
    };

    /// Helper type function to replace a T with Scalar<T> when std::is_scalar<T>.
    template<class T, bool a_isScalar = false>
    struct ScalarToClassImpl {
      /// Base implementation just forwards the original type.
      using type = T;
    };

    /// Specialisation for scalar types
    template<class T>
    struct ScalarToClassImpl<T, true> {
      /// Wrap scalar type as a class.
      using type = Scalar<T>;
    };

    /// Helper type function to replace a scalar type T with Scalar<T> when needed.
    template<class T>
    using ScalarToClass = typename ScalarToClassImpl<T, std::is_scalar<T>::value>::type;
  }
}
