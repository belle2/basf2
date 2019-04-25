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

CutsFromDBWireHitFilter::~CutsFromDBWireHitFilter()
{
  if (m_CDCWireHitRequirementsFromDB) delete m_CDCWireHitRequirementsFromDB;
}

void CutsFromDBWireHitFilter::initialize()
{
  m_CDCWireHitRequirementsFromDB = new DBObjPtr<CDCWireHitRequirements>;
  checkIfDBObjPtrIsValid();
}

void CutsFromDBWireHitFilter::beginRun()
{
  checkIfDBObjPtrIsValid();
}

void CutsFromDBWireHitFilter::checkIfDBObjPtrIsValid()
{
  if (!((*m_CDCWireHitRequirementsFromDB).isValid())) {
    B2WARNING("DBObjPtr<CDCWireHitRequirements> not valid for current run.  { findlet: CutsFromDBWireHitFilter }\n"
              "Cut not applied on CDCWireHit by CutsFromDBWireHitFilter.  { findlet: CutsFromDBWireHitFilter }");
    m_DBPtrIsValidForCurrentRun = false;
  } else {
    m_DBPtrIsValidForCurrentRun = true;
  }
}

template <typename T>
bool CutsFromDBWireHitFilter::isInRange(const T& value, const std::pair<T, T>& range) const
{
  if (range.second == -1) {
    return (value >= range.first);
  } else {
    return (value >= range.first) && (value <= range.second);
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
          isInRange<float>(ADCOverTOT, (*m_CDCWireHitRequirementsFromDB)->getADCOverTOTRangeFirstSuperLayer())) {
        // Check if the conditions "if TOT==x, then ADC<=y" are not respected .
        for (auto& v : (*m_CDCWireHitRequirementsFromDB)->getMaxADCGivenTOTFirstSuperLayer()) {
          if (TOT == v.first and !(isInRange<short>(ADC, std::make_pair(0, v.second)))) {
            // Hit rejected
            return NAN;
          }
        }
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
          isInRange<float>(ADCOverTOT, (*m_CDCWireHitRequirementsFromDB)->getADCOverTOTRangeOuterSuperLayers())) {
        // Check if the conditions "if TOT==x, then ADC<=y" are not respected.
        for (auto& v : (*m_CDCWireHitRequirementsFromDB)->getMaxADCGivenTOTOuterSuperLayers()) {
          if (TOT == v.first and !(isInRange<short>(ADC, std::make_pair(0, v.second)))) {
            // Hit rejected
            return NAN;
          }
        }
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
