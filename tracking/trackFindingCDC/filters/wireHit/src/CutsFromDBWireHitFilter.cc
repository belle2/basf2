/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Cyrille Praz                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/wireHit/CutsFromDBWireHitFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <cdc/dataobjects/CDCHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;


CutsFromDBWireHitFilter::CutsFromDBWireHitFilter() :
  m_CDCWireHitRequirementsFromDB("CDCWireHitRequirements"), m_DBPtrIsValidForCurrentRun(true)
{
}

void CutsFromDBWireHitFilter::beginRun()
{
  if (!(m_CDCWireHitRequirementsFromDB.isValid())) {
    B2WARNING("DBObjPtr<CDCWireHitRequirements> not valid for current run.  { findlet: CutsFromDBWireHitFilter }\n"
              "Cut not applied on CDCWireHit by CutsFromDBWireHitFilter.  { findlet: CutsFromDBWireHitFilter }");
    m_DBPtrIsValidForCurrentRun = false;
  }

}

Weight CutsFromDBWireHitFilter::operator()(const CDCWireHit& wireHit)
{
  int ADC = (*wireHit.getHit()).getADCCount();
  if (m_DBPtrIsValidForCurrentRun) {
    if (ADC > m_CDCWireHitRequirementsFromDB->getMinADC()) {
      // Hit accepted
      return ADC;
    } else {
      // Hit rejected
      return NAN;
    }
  }
  // Hit accepted (cf. B2WARNING above)
  return ADC;
}
