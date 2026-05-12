/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/WireHitMCMultiLoopBlocker.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>

#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>

#include <tracking/trackingUtilities/eventdata/trajectories/CDCBFieldUtil.h>

#include <tracking/trackingUtilities/geometry/Vector3D.h>

#include <tracking/trackingUtilities/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

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

    // Reject hits with no associated CDCSimHit.
    const CDCSimHit* simHit = mcHitLookUp.getClosestPrimarySimHit(wireHit.getHit());
    if (not simHit) return false;

    const double tof = simHit->getFlightTime();

    // Accept hits with no associated MCParticle (e.g. beam background.)
    const MCParticle* mcParticle = simHit->getRelated<MCParticle>();
    if (not mcParticle) return true;

    const double speed = mcParticle->get4Vector().Beta() * Const::speedOfLight;

    const ROOT::Math::XYZVector mom3D = mcParticle->getMomentum();
    const float absMom2D = mom3D.Rho();
    const float absMom3D = mom3D.R();

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
