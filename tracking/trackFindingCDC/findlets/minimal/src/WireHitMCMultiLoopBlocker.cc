/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/WireHitMCMultiLoopBlocker.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string WireHitMCMultiLoopBlocker::getDescription()
{
  return "Marks all hits that were not reached after the specified number of loops as background based on MC information.";
}

void WireHitMCMultiLoopBlocker::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "UseNLoops"),
                                m_param_useNLoops,
                                "Maximal number of loops accepted as non background",
                                m_param_useNLoops);
}

void WireHitMCMultiLoopBlocker::initialize()
{
  Super::initialize();
  if (std::isfinite(m_param_useNLoops)) {
    CDCMCManager::getInstance().requireTruthInformation();
  }
}

void WireHitMCMultiLoopBlocker::beginEvent()
{
  Super::beginEvent();
  if (std::isfinite(m_param_useNLoops)) {
    CDCMCManager::getInstance().fill();
  }
}

void WireHitMCMultiLoopBlocker::apply(std::vector<CDCWireHit>& wireHits)
{
  if (not std::isfinite(m_param_useNLoops)) return;
  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  auto isWithinMCLoops = [&mcHitLookUp, this](const CDCWireHit & wireHit) {
    const CDCSimHit* simHit = mcHitLookUp.getClosestPrimarySimHit(wireHit.getHit());
    if (not simHit) return false;
    // Reject hits with no assoziated CDCSimHit.

    const double tof = simHit->getFlightTime();

    const MCParticle* mcParticle = simHit->getRelated<MCParticle>();
    if (not mcParticle) return true;
    // Accept hits with no assoziated MCParticle (e.g. beam background.)

    const double speed = mcParticle->get4Vector().Beta() * Const::speedOfLight;

    const TVector3 mom3D = mcParticle->getMomentum();
    const float absMom2D = mom3D.Perp();
    const float absMom3D = mom3D.Mag();

    const Vector3D pos3D(0.0, 0.0, 0.0);
    const double bendRadius = CDCBFieldUtil::absMom2DToBendRadius(absMom2D, pos3D);
    const double bendCircumfence =  2 * M_PI * bendRadius;
    const double loopLength = bendCircumfence * absMom3D / absMom2D;
    const double loopTOF =  loopLength / speed;
    if (tof > loopTOF * m_param_useNLoops) {
      return false;
    } else {
      return true;
    }
  };

  for (CDCWireHit& wireHit : wireHits) {
    if (not isWithinMCLoops(wireHit)) {
      wireHit.getAutomatonCell().setBackgroundFlag();
      wireHit.getAutomatonCell().setTakenFlag();
    }
  }
}
