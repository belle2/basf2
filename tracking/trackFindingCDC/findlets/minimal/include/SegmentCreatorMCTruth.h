/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <vector>
#include <string>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Findlet that generates segments from wire hits using the mc truth information.
    class SegmentCreatorMCTruth :
      public Findlet<const CDCWireHit, CDCRecoSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit, CDCRecoSegment2D>;

    public:
      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final;

      /// Short description of the findlet
      virtual std::string getDescription() override final;

      /// Initialize the Module before event processing
      virtual void initialize() override final;

      /// Start processing the current event
      virtual void beginEvent() override final;

      /// Main function of the segment finding by the cellular automaton.
      virtual void apply(const std::vector<CDCWireHit>& inputWireHits,
                         std::vector<CDCRecoSegment2D>& outputSegments) override final;

    private:
      /// Parameter : Setup the drift length as it can be estimated from two dimensional information
      bool m_param_reconstructedDriftLength = false;

      /// Parameter : Switch to reconstruct the positions in the segments immitating the facet ca picking up all correct hits.
      bool m_param_reconstructedPositions = false;

    }; // end class SegmentCreator
  } //end namespace TrackFindingCDC
} //end namespace Belle2
