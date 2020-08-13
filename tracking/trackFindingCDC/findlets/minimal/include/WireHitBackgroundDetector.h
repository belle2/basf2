/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Cyrille Praz                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/filters/wireHit/ChooseableWireHitFilter.h>
#include <vector>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCWireHit;

    /// Marks hits as background based on the result of a filter
    class WireHitBackgroundDetector : public Findlet<CDCWireHit&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit&>;

    public:
      /// Default constructor
      WireHitBackgroundDetector();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main algorithm marking hit as background
      void apply(std::vector<CDCWireHit>& wireHits) final;

    private:

      /// Chooseable WireHit filter to be used to filter background
      ChooseableWireHitFilter m_wireHitFilter;

    };
  }
}
