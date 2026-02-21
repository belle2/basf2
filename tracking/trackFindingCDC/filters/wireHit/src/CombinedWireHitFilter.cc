/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/wireHit/CombinedWireHitFilter.h>
#include <tracking/trackFindingCDC/filters/wireHit/CutsFromDBWireHitFilter.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <tracking/dbobjects/WireHitFilterSettings.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;


CombinedWireHitFilter::CombinedWireHitFilter()
{
  this->addProcessingSignalListener(&m_mvaFilter);
  this->addProcessingSignalListener(&m_cutsFromDBFilter);
}

void CombinedWireHitFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_mvaFilter.exposeParameters(moduleParamList, prefix);
  m_cutsFromDBFilter.exposeParameters(moduleParamList, prefix);
}


Weight CombinedWireHitFilter::operator()(const CDCWireHit& wireHit)
{
  if (not m_WireHitFilterSettings.isValid()) {
    B2FATAL("WireHitFilterSetting DB object is invalid");
  }
  const int switchSLayer = m_WireHitFilterSettings->getMVASwitchSuperLayer();
  const auto* cdcHit = wireHit.getHit();
  const auto sl = cdcHit->getISuperLayer();

  if (sl < switchSLayer) {
    B2INFO(" here cut " << sl);
    return m_cutsFromDBFilter(wireHit);
  } else {
    B2INFO(" here mva " << sl);
    return m_mvaFilter(wireHit);
  }
}
