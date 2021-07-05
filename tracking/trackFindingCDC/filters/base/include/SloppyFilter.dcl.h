/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <string>
namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {

    /// A filter that accepts every <sloppinessFactor>th candidate on average (random selection)
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
