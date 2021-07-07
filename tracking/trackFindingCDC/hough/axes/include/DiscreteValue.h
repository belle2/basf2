/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <array>
#include <vector>
#include <iostream>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Representation for a discrete position in an array of discrete positions.
    template<class T, class ATag = T>
    class DiscreteValue : public std::vector<T>::const_iterator {

    private:
      /// Type of the base class
      using Super = typename std::vector<T>::const_iterator;

      /// Type of this class
      using This = DiscreteValue<T, ATag>;

    public:
      /// Inheriting the constructors from the base class
      using Super::Super;

      /// Default constructor for to obtain a valid object in all contexts.
      DiscreteValue() = default;

      /// Constructor from underlying iterator type
      explicit DiscreteValue(const Super& iter) : Super(iter) {}

      /** Constructor from a reference to the underlying type
       *  Care must be taken to feed it a reference that comes from a vector*/
      explicit DiscreteValue(T& value) : Super(&value) {}

      /// Allow explicit conversion to the container type.
      explicit operator const T& () const
      { return **this; }

      /// Output operator for debugging
      friend std::ostream& operator<<(std::ostream& output, const This& discreteValue)
      { return output << *discreteValue; }

      /// The type of the array which contains the underlying values.
      using Array = std::vector<T>;

      /// Extract the range from an array providing the discrete values.
      static
      std::array<This, 2> getRange(Array& values)
      { return {{ This(values.front()), This(values.back()) }}; }
    };

  }
}
