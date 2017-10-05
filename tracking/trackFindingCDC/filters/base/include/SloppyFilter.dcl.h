/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <string>
namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {

    template<class AFilter>
    class Sloppy : public AFilter {

    private:
      /// The parent class
      using Super = AFilter;

    public:
      /// Inherit constructors
      using Super::Super;

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    public:
      /// Reject an item if the truth variable is 0 or in some fractions of the events, else accept it.
      Weight operator()(const typename Super::Object& object) override;

    private:
      /// Parameter : The prescaling of the false items returning true.
      unsigned int m_param_sloppinessFactor = 10;
    };
  }
}
