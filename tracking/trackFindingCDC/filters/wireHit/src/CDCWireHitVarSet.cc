/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/wireHit/CDCWireHitVarSet.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>
#include <cdc/dataobjects/CDCHit.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

CDCWireHitVarSet::CDCWireHitVarSet() : Super()
{
}

void CDCWireHitVarSet::initialize()
{
  Super::initialize();
}

bool CDCWireHitVarSet::extract(const CDCWireHit* wireHit)
{
  const auto* cdcHit = wireHit->getHit();
  var<named("adc")>() = cdcHit->getADCCount();
  var<named("tot")>() = cdcHit->getTOT();
  var<named("tdc")>() = cdcHit->getTDCCount();
  var<named("slayer")>() = cdcHit->getISuperLayer() == 0 ? 0 : 1;
  return true;
}
