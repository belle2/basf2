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

#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A mixin class to attach a weight to an object.
    template<class T>
    class WithWeight : public T {

    public:
      /// Make the constructor of the base type available
      using T::T;

      /// Also forward the copy constructor form the base class object.
      explicit WithWeight(const T& t): T(t) {;}

      /// Getter for the weight
      Weight getWeight() const
      {
        return m_weight;
      }

      /// Setter for the weight of the object.
      void setWeight(const Weight& weight)
      {
        m_weight = weight;
      }

    private:
      /// Memory for the weight.
      Weight m_weight = 0;

    };
  }
}
