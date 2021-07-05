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

#include <array>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type to have values *not* based on discrete positions from an array
    template<class T, class ATag = T>
    class ContinuousValue {
    public:
      /// Shortcut for the class itself.
      using This = ContinuousValue<T, ATag>;

      /// Allow default construction
      ContinuousValue() = default;

      /// Constructor from the underlying type.
      explicit ContinuousValue(const T& value) : m_value(value) {};

      /// Allow automatic casts to the underlying type.
      operator const T& () const {return m_value; }

      /// Mock array type to be a drop in replacement for the discrete values
      using Array = std::array<T, 2>;

      /// Extract the range from an array providing the discrete values.
      static
      std::array<This, 2> getRange(const Array& bounds)
      { return {{ This(bounds.front()), This(bounds.back()) }}; }

    private:
      /// Memory for the underlying value
      T m_value;
    };

  }
}
