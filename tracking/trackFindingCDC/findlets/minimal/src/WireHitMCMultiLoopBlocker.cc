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

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCSimHitLookUp.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBFieldUtil.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <root/TVector3.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string WireHitMCMultiLoopBlocker::getDescription()
{
  return "Marks all hits that were not reached after the specified number of loops as background "
         "based on MC information.";
}

void WireHitMCMultiLoopBlocker::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "UseNLoops"),
                                m_param_useNLoops,
                                "Maximal number of loops accepted",
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
  double nLoops = m_param_useNLoops;
  auto isWithinMCLoops = [&mcHitLookUp, nLoops](const CDCWireHit & wireHit) {

    // Reject hits with no assoziated CDCSimHit.
    const CDCSimHit* simHit = mcHitLookUp.getClosestPrimarySimHit(wireHit.getHit());
    if (not simHit) return false;

    const double tof = simHit->getFlightTime();

    // Accept hits with no assoziated MCParticle (e.g. beam background.)
    const MCParticle* mcParticle = simHit->getRelated<MCParticle>();
    if (not mcParticle) return true;

    const double speed = mcParticle->get4Vector().Beta() * Const::speedOfLight;

    const TVector3 mom3D = mcParticle->getMomentum();
    const float absMom2D = mom3D.Perp();
    const float absMom3D = mom3D.Mag();

    const Vector3D pos3D(0.0, 0.0, 0.0);
    const double bendRadius = CDCBFieldUtil::absMom2DToBendRadius(absMom2D, pos3D);
    const double bendCircumfence =  2 * M_PI * bendRadius;
    const double loopLength = bendCircumfence * absMom3D / absMom2D;
    const double loopTOF =  loopLength / speed;

    if (tof > loopTOF * nLoops) {
      return false;
    } else {
      return true;
    }
  };

  for (CDCWireHit& wireHit : wireHits) {
    if (not isWithinMCLoops(wireHit)) {
      wireHit->setBackgroundFlag();
      wireHit->setTakenFlag();
    }
  }
}
