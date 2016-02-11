/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackStore.h>
#include <tracking/trackFindingCDC/mclookup/CDCSimHitLookUp.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>


#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRLWireHitSegment.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
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
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final
      {
        moduleParamList->addParameter(prefixed(prefix, "reconstructedPositions"),
                                      m_param_recostructedPositions,
                                      "Switch to reconstruct the positions in the segments "
                                      "immitating the facet ca picking up all correct hits.",
                                      m_param_recostructedPositions);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Constructs segments from wire hits using the mc truth information.";
      }

      /// Initialize the Module before event processing
      virtual void initialize() override final
      {
        CDCMCManager::getInstance().requireTruthInformation();
        Super::initialize();
      }

      /// Start processing the current event
      virtual void beginEvent() override final
      {
        CDCMCManager::getInstance().fill();
        Super::beginEvent();
      }

      /// Main function of the segment finding by the cellular automaton.
      virtual void apply(const std::vector<CDCWireHit>& inputWireHits,
                         std::vector<CDCRecoSegment2D>& outputSegments) override final;


    private:
      /// Switch to reconstruct the positions in the segments immitating the facet ca picking up all correct hits.
      bool m_param_recostructedPositions = false;
    }; // end class SegmentCreator

    void SegmentCreatorMCTruth::apply(const std::vector<CDCWireHit>& inputWireHits,
                                      std::vector<CDCRecoSegment2D>& outputSegments)
    {
      const CDCMCTrackStore& mcTrackStore = CDCMCTrackStore::getInstance();
      const CDCSimHitLookUp& simHitLookUp = CDCSimHitLookUp::getInstance();

      using CDCHitVector = CDCMCTrackStore::CDCHitVector;

      const std::map<ITrackType, std::vector<CDCHitVector>>& mcSegmentsByMCParticleIdx =
                                                           mcTrackStore.getMCSegmentsByMCParticleIdx();

      std::size_t nSegments = 0;
      for (const std::pair<ITrackType, std::vector<CDCHitVector>>& mcSegmentsAndMCParticleIdx :  mcSegmentsByMCParticleIdx) {
        const std::vector<CDCHitVector>& mcSegments = mcSegmentsAndMCParticleIdx.second;
        nSegments += mcSegments.size();
      }

      outputSegments.reserve(outputSegments.size() + nSegments);
      for (const std::pair<ITrackType, std::vector<CDCHitVector>>& mcSegmentsAndMCParticleIdx : mcSegmentsByMCParticleIdx) {

        const std::vector<CDCHitVector>& mcSegments = mcSegmentsAndMCParticleIdx.second;
        for (const CDCHitVector& mcSegment : mcSegments) {
          outputSegments.push_back(CDCRecoSegment2D());
          CDCRecoSegment2D& recoSegment2D = outputSegments.back();
          for (const CDCHit* ptrHit : mcSegment) {
            CDCRecoHit2D recoHit2D = simHitLookUp.getClosestPrimaryRecoHit2D(ptrHit, inputWireHits);
            recoSegment2D.push_back(recoHit2D);
          }
        }
      }

      if (m_param_recostructedPositions) {
        for (CDCRecoSegment2D& segment : outputSegments) {
          CDCRLWireHitSegment rlWireHitSegment = segment.getRLWireHitSegment();
          segment = CDCRecoSegment2D::reconstructUsingFacets(rlWireHitSegment);
        }
      }
    }
  } //end namespace TrackFindingCDC
} //end namespace Belle2
