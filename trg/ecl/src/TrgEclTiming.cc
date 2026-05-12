/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#define TRG_SHORT_NAMES
#define TRGECLCLUSTER_SHORT_NAMES
#include <framework/gearbox/Unit.h>

#include "trg/ecl/TrgEclTiming.h"

using namespace std;
using namespace Belle2;
//
//
//
TrgEclTiming::TrgEclTiming() : m_NofTopTC(3), m_Source(0)
{
  m_TCMap = new TrgEclMapping();
  m_TCEnergy.clear();
  m_TCTiming.clear();
  m_TCId.clear();
  m_EventTimingQualityFlag = -1;
  m_EventTimingTCId = 0;
  m_EventTimingTCThetaId = 0;
  m_EventTimingTCEnergy = 0;
  m_EventTimingQualityThreshold = {1.0, 20.0}; // GeV
}
//
//
//
TrgEclTiming::~TrgEclTiming()
{
  delete m_TCMap;
}
//
//
//
void
TrgEclTiming::Setup(const std::vector<int>& HitTCId,
                    const std::vector<double>& HitTCEnergy,
                    const std::vector<double>& HitTCTiming)
{
  m_TCId = HitTCId;
  m_TCEnergy = HitTCEnergy;
  m_TCTiming = HitTCTiming;

  return;
}
//========================================================
// timing method selection
//========================================================
double TrgEclTiming::getEventTiming(int method)
{
  double EventTiming = 0;

  if (method == 0) {
    // Fastest timing (belle)
    EventTiming =  getEventTiming00();
  } else if (method == 1) {
    // Maximum energy
    EventTiming =  getEventTiming01();
  } else {
    // Energy weighted timing
    EventTiming =  getEventTiming02();
  }

  return EventTiming;
}
//========================================================
// Fastest TC timing ( same as belle )
//========================================================
double TrgEclTiming::getEventTiming00()
{
  m_Source = 0;
  double FastestEnergy = 0;
  double FastestTiming = 9999;
  int FastestTCId = 0;
  const int hit_size = m_TCTiming.size();

  for (int ihit = 0; ihit < hit_size; ihit++) {
    if (m_TCTiming[ihit] < FastestTiming) {
      FastestTiming = m_TCTiming[ihit];
      FastestTCId = m_TCId[ihit];
      FastestEnergy = m_TCEnergy[ihit];
    }
  }

  if (FastestTCId < 81) {
    m_Source = 1;
  } else if (FastestTCId < 513) {
    m_Source = 2;
  } else {
    m_Source = 4;
  }

  m_EventTimingTCId = FastestTCId;
  m_EventTimingTCThetaId = m_TCMap->getTCThetaIdFromTCId(FastestTCId);
  m_EventTimingTCEnergy = FastestEnergy;

  return FastestTiming;
}
//========================================================
// Timing from most energetic TC timing
//========================================================
double TrgEclTiming::getEventTiming01()
{
  m_Source = 0;

  double maxEnergy = 0;
  double maxTiming = 0;
  int maxTCId = 0;
  const int hit_size = m_TCTiming.size();

  for (int ihit = 0; ihit < hit_size; ihit++) {
    if (m_TCEnergy[ihit] > maxEnergy) {
      maxEnergy = m_TCEnergy[ihit] ;
      maxTiming = m_TCTiming[ihit] ;
      maxTCId   = m_TCId[ihit];
    }
  }
  if (maxTCId < 81) {
    m_Source = 1;
  } else if (maxTCId < 513) {
    m_Source = 2;
  } else {
    m_Source = 4;
  }

  if (hit_size == 0) {
    m_EventTimingQualityFlag = 0;
  } else {
    if (maxEnergy > m_EventTimingQualityThreshold[1]) {
      m_EventTimingQualityFlag = 3;
    } else if (maxEnergy > m_EventTimingQualityThreshold[0]) {
      m_EventTimingQualityFlag = 2;
    } else {
      m_EventTimingQualityFlag = 1;
    }
  }

  m_EventTimingTCId = maxTCId;
  m_EventTimingTCThetaId = m_TCMap->getTCThetaIdFromTCId(maxTCId);
  m_EventTimingTCEnergy = maxEnergy;

  return maxTiming;
}
//========================================================
// Energy weighted TC timing
//========================================================
double TrgEclTiming::getEventTiming02()
{
  m_Source = 0;
  std::vector<double> maxEnergy;
  std::vector<double> maxTiming;

  const int NtopTC = m_NofTopTC;
  int maxTCId = 0;

  maxEnergy.clear();
  maxTiming.clear();
  maxEnergy.resize(NtopTC, 0);
  maxTiming.resize(NtopTC, 0);

  const int hit_size = m_TCTiming.size();
  double E_sum = 0;
  double EventTiming = 0;

  for (int iNtopTC = 0; iNtopTC < NtopTC ; iNtopTC++) {
    for (int ihit = 0; ihit < hit_size; ihit++) {
      if (iNtopTC == 0) {
        if (maxEnergy[iNtopTC] < m_TCEnergy[ihit]) {
          maxEnergy[iNtopTC] =  m_TCEnergy[ihit];
          maxTiming[iNtopTC] =  m_TCTiming[ihit];
          maxTCId = m_TCId[ihit];
        }
      } else if (iNtopTC > 0) {
        if (maxEnergy[iNtopTC - 1] > m_TCEnergy[ihit] &&
            maxEnergy[iNtopTC]     < m_TCEnergy[ihit]) {
          maxEnergy[iNtopTC] =  m_TCEnergy[ihit];
          maxTiming[iNtopTC] =  m_TCTiming[ihit];
        }
      }
    }
    E_sum += maxEnergy[iNtopTC];
    EventTiming += maxEnergy[iNtopTC] * maxTiming[iNtopTC];
  }

  EventTiming /= E_sum;

  if (maxTCId < 81) {
    m_Source = 1;
  } else if (maxTCId < 513) {
    m_Source = 2;
  } else {
    m_Source = 4;
  }

  return EventTiming;
}
//========================================================
//
//========================================================
