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

#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A mixin class to attach a name to an object.
    template<class T>
    class Named : public StarToPtr<T> {

    private:
      /// Type of the base class
      using Super = StarToPtr<T>;

    public:
      /// Make the constructor of the base type available
      using Super::Super;

      /// Constructor taking the name and the desired value.
      Named(const std::string& name, T t)
        : Super(std::move(t))
        , m_name(name)
      {}

      /// Comparison operator establishing an ordering considering the name and the object
      bool operator<(const Named<T>& other) const
      {
        const T& t(*this);
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
