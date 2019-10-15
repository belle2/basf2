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
  m_CDCWireHitRequirementsFromDB(nullptr), m_DBPtrIsValidForCurrentRun(false)
{
}

void CutsFromDBWireHitFilter::initialize()
{
  m_CDCWireHitRequirementsFromDB = std::make_unique<DBObjPtr<CDCWireHitRequirements> >();
  checkIfDBObjPtrIsValid();
}

void CutsFromDBWireHitFilter::beginRun()
{
  checkIfDBObjPtrIsValid();
}

void CutsFromDBWireHitFilter::checkIfDBObjPtrIsValid()
{
  if (!m_CDCWireHitRequirementsFromDB) {
    B2ERROR("std::unique_ptr<DBObjPtr<CDCWireHitRequirements> > m_CDCWireHitRequirementsFromDB not properly set.\n"
            "Cut not applied on CDCWireHit by CutsFromDBWireHitFilter.  { findlet: CutsFromDBWireHitFilter }");
    m_DBPtrIsValidForCurrentRun = false;
  } else {
    if (!((*m_CDCWireHitRequirementsFromDB).isValid())) {
      B2WARNING("DBObjPtr<CDCWireHitRequirements> not valid for current run.  { findlet: CutsFromDBWireHitFilter }\n"
                "Cut not applied on CDCWireHit by CutsFromDBWireHitFilter.  { findlet: CutsFromDBWireHitFilter }");
      m_DBPtrIsValidForCurrentRun = false;
    } else {
      m_DBPtrIsValidForCurrentRun = true;
    }
  }
}

/** Check if value >= range.first and value <= range.second
If range.second == -1, then check only if value >= range.first */
template <typename T>
bool CutsFromDBWireHitFilter::isInRange(const T& value, const std::pair<T, T>& range) const
{
  if (range.second == -1) {
    return (value >= range.first);
  } else {
    return (value >= range.first) && (value <= range.second);
  }
}

/** Check if value <= upper_value
If upper_value == -1, then return true */
template <typename T>
bool CutsFromDBWireHitFilter::isLessThanOrEqualTo(const T& value, const T& upper_value) const
{
  if (upper_value == -1) {
    return true;
  } else {
    return value <= upper_value;
  }
}

Weight CutsFromDBWireHitFilter::operator()(const CDCWireHit& wireHit)
{
  const short ADC = (*wireHit.getHit()).getADCCount();
  const short TOT = (*wireHit.getHit()).getTOT();
  const float ADCOverTOT = (TOT != 0) ? static_cast<float>(ADC) / TOT : 0;

  if (m_DBPtrIsValidForCurrentRun) {
    if ((*wireHit.getHit()).getISuperLayer() == 0) {
      // First super layer (0)
      // Check if ADC, TOT and ADC/TOT are in the corresponding allowed ranges.
      if (isInRange<short>(ADC, (*m_CDCWireHitRequirementsFromDB)->getADCRangeFirstSuperLayer()) &&
          isInRange<short>(TOT, (*m_CDCWireHitRequirementsFromDB)->getTOTRangeFirstSuperLayer()) &&
          isInRange<float>(ADCOverTOT, (*m_CDCWireHitRequirementsFromDB)->getADCOverTOTRangeFirstSuperLayer()) &&
          isLessThanOrEqualTo<short>(ADC, (*m_CDCWireHitRequirementsFromDB)->getMaxADCGivenTOTFirstSuperLayer(TOT))) {
        // Hit accepted
        return ADC;
      } else {
        // Hit rejected
        return NAN;
      }

    } else {
      // Outer super layers (1-8)
      // Check if ADC, TOT and ADC/TOT are in the corresponding allowed ranges.
      if (isInRange<short>(ADC, (*m_CDCWireHitRequirementsFromDB)->getADCRangeOuterSuperLayers()) &&
          isInRange<short>(TOT, (*m_CDCWireHitRequirementsFromDB)->getTOTRangeOuterSuperLayers()) &&
          isInRange<float>(ADCOverTOT, (*m_CDCWireHitRequirementsFromDB)->getADCOverTOTRangeOuterSuperLayers()) &&
          isLessThanOrEqualTo<short>(ADC, (*m_CDCWireHitRequirementsFromDB)->getMaxADCGivenTOTOuterSuperLayers(TOT))) {
        // Hit accepted
        return ADC;
      } else {
        // Hit rejected
        return NAN;
      }
    }
  }
  // If the DB pointer is not valid, the hit is accepted (cf. B2WARNING above)
  return ADC;
}
