/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/Scalar.h>

#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A mixin class to attach a name to an object.
    template<class T>
    class Named : public ScalarToClass<T> {

    private:
      /// Type of the base class
      using Super = ScalarToClass<T>;

    public:
      /// Make the constructor of the base type available
      using Super::Super;

      /// Constructor taking the name and the desired value.
      Named(const std::string& name, T t)
        : Super(std::move(t))
        , m_name(name)
      {
      }

      /// Comparison operator establishing an ordering considering the name and the object
      bool operator<(const Named<T>& other) const
      {
        // cppcheck-suppress constVariable
        const T& t(*this);
        // cppcheck-suppress constVariable
        const T& otherT(other);
        return getName() < other.getName() or (not(other.getName() < getName()) and t < otherT);
      }

      /// Getter for the name
      std::string getName() const
      {
        return m_name;
      }

      /// Setter for the name of the object.
      void setName(const std::string& name)
      {
        m_name = name;
      }

    private:
      /// Memory for the name.
      std::string m_name = "";
    };
  }
}
