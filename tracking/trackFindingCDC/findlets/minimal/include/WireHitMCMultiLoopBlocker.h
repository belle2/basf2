/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Marks all hits that are not on the first loop of the track by considering the mc truth information as background
    class WireHitMCMultiLoopBlocker : public Findlet<CDCWireHit> {

    private:
      /// Type of the base class
      typedef Findlet<CDCWireHit> Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Marks all hits that are not on the first loop of the track by considering the mc truth information as background";
      }

      /// Signals the start of the event processing
      virtual void initialize() override final
      {
        CDCMCManager::getInstance().requireTruthInformation();
        Super::initialize();
      }

      /// Prepare the Monte Carlo information at the start of the event
      virtual void beginEvent() override final
      {
        CDCMCManager::getInstance().fill();
        Super::beginEvent();
      }

      /// Main algorithm marking the hit of higher loops as background
      virtual void apply(std::vector<CDCWireHit>& wireHits) override final
      {
        const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

        auto isNotOnFirstLoop = [&mcHitLookUp](const CDCWireHit & wireHit) {
          const CDCSimHit* simHit = mcHitLookUp.getClosestPrimarySimHit(wireHit.getHit());
          if (not simHit) return true;
          // Reject hits with no assoziated CDCSimHit.

          const double tof = simHit->getFlightTime();

          const MCParticle* mcParticle = simHit->getRelated<MCParticle>();
          if (not mcParticle) return false;
          // Accept hits with no assoziated MCParticle (e.g. beam background.)

          const double speed = mcParticle->get4Vector().Beta() * Const::speedOfLight;

          const TVector3 mom3D = mcParticle->getMomentum();
          const float absMom2D = mom3D.Perp();
          const float absMom3D = mom3D.Mag();

          const Vector3D pos3D(0.0, 0.0, 0.0);
          const double bendRadius = absMom2DToBendRadius(absMom2D, pos3D);
          const double bendCircumfence =  2 * M_PI * bendRadius;
          const double loopLength = bendCircumfence * absMom3D / absMom2D;
          const double loopTOF =  loopLength / speed;
          if (tof > loopTOF) {
            return true;
          } else {
            return false;
          }
        };

        for (CDCWireHit& wireHit : wireHits) {
          if (isNotOnFirstLoop(wireHit)) {
            wireHit.getAutomatonCell().setBackgroundFlag();
            wireHit.getAutomatonCell().setTakenFlag();
          }
        }
      }
    }; // end class WireHitMCMultiLoopBlocker

  } // end namespace TrackFindingCDC
} // end namespace Belle2
