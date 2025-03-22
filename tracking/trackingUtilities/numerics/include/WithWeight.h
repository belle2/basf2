/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once


#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Scalar.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A mixin class to attach a weight to an object.
    template<class T>
    class WithWeight : public ScalarToClass<T> {

    private:
      /// Type of the base class
      using Super = ScalarToClass<T>;

    public:
      /// Make the constructor of the base type available
      using Super::Super;

      /// Also forward the copy constructor form the base class object.
      explicit WithWeight(const T& t)
        : Super(t)
      {
      }

      /// Constructor which may also initialise the weight to a desired value
      WithWeight(const T& t, Weight weight)
        : Super(t)
        , m_weight(weight)
      {
      }

      /// Comparison operator establishing an ordering considering the pointer first and the weight second
      bool operator<(const WithWeight<T>& other) const
      {
        // cppcheck-suppress constVariable
        const T& t(*this);
        // cppcheck-suppress constVariable
        const T& otherT(other);
        return t < otherT or (not(otherT < t) and getWeight() < other.getWeight());
      }

      /// Getter for the weight
      Weight getWeight() const
      {
        return m_weight;
      }

      /// Setter for the weight of the object.
      void setWeight(Weight weight)
      {
        m_weight = weight;
      }

      /// Mutable getter for the weight
      Weight& weight()
      {
        return m_weight;
      }

    private:
      /// Memory for the weight.
      Weight m_weight = 0;
    };
  }
}
