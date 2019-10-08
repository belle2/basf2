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

#include <tracking/trackFindingCDC/findlets/minimal/WireHitCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/WireHitBackgroundBlocker.h>
#include <tracking/trackFindingCDC/findlets/minimal/WireHitBackgroundDetector.h>
#include <tracking/trackFindingCDC/findlets/minimal/WireHitMCMultiLoopBlocker.h>
#include <tracking/trackFindingCDC/findlets/minimal/AsicBackgroundDetector.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <string>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;

    /// Findlet preparing the wire hits for the track finding
    class WireHitPreparer : public Findlet<CDCWireHit> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      WireHitPreparer();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main function preparing the wire hits
      void apply(std::vector<CDCWireHit>& outputWireHits) final;

    private:
      // Findlets
      /// Creates the wire hits from CDCHits attaching geometry information.
      WireHitCreator m_wireHitCreator;

      /// Marks hits as background based on simple heuristics
      WireHitBackgroundBlocker m_wireHitBackgroundBlocker;

      /// Marks hits as background based on the result of a filter
      WireHitBackgroundDetector m_wireHitBackgroundDetector;

      /// Marks hits as background based on ASIC cross-talk signature
      AsicBackgroundDetector m_asicBackgroundDetector;

      /// Marks higher order loops as background for tuning analysis
      WireHitMCMultiLoopBlocker m_wireHitMCMultiLoopBlocker;
    };
  }
}
