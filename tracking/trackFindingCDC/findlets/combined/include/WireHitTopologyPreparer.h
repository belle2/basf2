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
#include <tracking/trackFindingCDC/findlets/minimal/WireHitMCMultiLoopBlocker.h>
#include <tracking/trackFindingCDC/findlets/minimal/WireHitTopologyFiller.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <string>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Legacy : Findlet implementing the creation and filling of the CDCWireHitTopology
    class WireHitTopologyPreparer :
      public Findlet<CDCWireHit> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      WireHitTopologyPreparer()
      {
        addProcessingSignalListener(&m_wireHitCreator);
        addProcessingSignalListener(&m_wireHitBackgroundBlocker);
        addProcessingSignalListener(&m_wireHitMCMultiLoopBlocker);
        addProcessingSignalListener(&m_wireHitTopologyFiller);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Combine the CDCHits from the DataStore with the geometry information to have them both at hand in the CDC tracking modules. "
               "Also set all CDCWireHits as unused.";
      }

      /// Expose the parameters of the wire hit preparation
      virtual void exposeParameters(ModuleParamList* moduleParamList,
                                    const std::string& prefix = "") override
      {
        m_wireHitCreator.exposeParameters(moduleParamList, prefix);
        m_wireHitBackgroundBlocker.exposeParameters(moduleParamList, prefix);
        m_wireHitMCMultiLoopBlocker.exposeParameters(moduleParamList, prefix);
        m_wireHitTopologyFiller.exposeParameters(moduleParamList, prefix);
      }

      /// Generates the segment.
      virtual void apply(std::vector<CDCWireHit>& outputWireHits) override final
      {
        m_wireHitCreator.apply(outputWireHits);
        m_wireHitBackgroundBlocker.apply(outputWireHits);
        m_wireHitMCMultiLoopBlocker.apply(outputWireHits);
        m_wireHitTopologyFiller.apply(outputWireHits);
      }

    private:
      // Findlets
      /// Creates the wire hits from CDCHits attaching geometry information.
      WireHitCreator m_wireHitCreator;

      /// Marks hits as background based on simple heuristics
      WireHitBackgroundBlocker m_wireHitBackgroundBlocker;

      /// Marks higher order loops as background for tuning analysis
      WireHitMCMultiLoopBlocker m_wireHitMCMultiLoopBlocker;

      /// Publishes the created wire hits to the wire hit topology.
      WireHitTopologyFiller m_wireHitTopologyFiller;

    };

  }
}
