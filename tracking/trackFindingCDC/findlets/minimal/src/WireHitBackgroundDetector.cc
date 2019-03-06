/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Cyrille Praz                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/WireHitBackgroundDetector.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string WireHitBackgroundDetector::getDescription()
{
  return "Marks hits as background based on the result of a filter.";
}

void WireHitBackgroundDetector::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  // TODO: m_wireHitFilter.exposeParameters(moduleParamList, prefix);
}

void WireHitBackgroundDetector::beginRun()
{
  Super::beginRun();
  if (!(m_CDCWireHitRequirementsFromDB.isValid())) {
    B2ERROR("CDC WireHitBackgroundDetector: DBObjPtr<CDCWireHitRequirements> not valid for current run.");
    exit(1);
  }
}

void WireHitBackgroundDetector::apply(std::vector<CDCWireHit>& wireHits)
{
  for (CDCWireHit& wireHit : wireHits) {
    bool markAsBackground = false;

    if (wireHit.getRefChargeDeposit() < m_CDCWireHitRequirementsFromDB->getChargeCut()) {
      B2INFO("CUT:   " << m_CDCWireHitRequirementsFromDB->getChargeCut());
      markAsBackground = true;
    }

    if (markAsBackground) {
      wireHit->setBackgroundFlag();
      wireHit->setTakenFlag();
    }
  }
}
