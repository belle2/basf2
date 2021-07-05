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
TrgEclTiming::TrgEclTiming() : NofTopTC(3), Source(0)
{
  _TCMap = new TrgEclMapping();
  TCEnergy.clear();
  TCTiming.clear();
  TCId.clear();
  m_EventTimingQualityFlag = -1;
  m_EventTimingTCId = 0;
  m_EventTimingTCThetaId = 0;
  m_EventTimingTCEnergy = 0;
  m_EventTimingQualityThresholds = {0.5, 2.0}; // GeV
}
//
//
//
TrgEclTiming::~TrgEclTiming()
{
  delete _TCMap;
}
//
//
//
void
TrgEclTiming::Setup(const std::vector<int>& HitTCId,
                    const std::vector<double>& HitTCEnergy,
                    const std::vector<double>& HitTCTiming)
{
  TCId = HitTCId;
  TCEnergy = HitTCEnergy;
  TCTiming = HitTCTiming;
  return;
}
//========================================================
// timing method selection
//========================================================
double TrgEclTiming::GetEventTiming(int method)
{
  double EventTiming = 0;

  if (method == 0) {
    // Fastest timing (belle)
    EventTiming =  GetEventTiming00();
  } else if (method == 1) {
    // Maximum energy
    EventTiming =  GetEventTiming01();
  } else {
    // Energy weighted timing
    EventTiming =  GetEventTiming02();
  }

  return EventTiming;
}
//========================================================
// Fastest TC timing ( same as belle )
//========================================================
double TrgEclTiming::GetEventTiming00()
{
  Source = 0;
  double FastestEnergy = 0;
  double FastestTiming = 9999;
  int FastestTCId = 0;
  const int hit_size = TCTiming.size();

  for (int ihit = 0; ihit < hit_size; ihit++) {
    if (TCTiming[ihit] < FastestTiming) {
      FastestTiming = TCTiming[ihit];
      FastestTCId = TCId[ihit];
      FastestEnergy = TCEnergy[ihit];
    }
  }

  if (FastestTCId < 81) {
    Source = 1;
  } else if (FastestTCId > 80 && FastestTCId < 513) {
    Source = 2;
  } else {
    Source = 4;
  }

  m_EventTimingTCId = FastestTCId;
  m_EventTimingTCThetaId = _TCMap->getTCThetaIdFromTCId(FastestTCId);
  m_EventTimingTCEnergy = FastestEnergy;

  return FastestTiming;
}
//========================================================
// Timing from most energetic TC timing
//========================================================
double TrgEclTiming::GetEventTiming01()
{
  Source = 0;

  double maxEnergy = 0;
  double maxTiming = 0;
  int maxTCId = 0;
  const int hit_size = TCTiming.size();

  for (int ihit = 0; ihit < hit_size; ihit++) {
    if (TCEnergy[ihit] > maxEnergy) {
      maxEnergy = TCEnergy[ihit] ;
      maxTiming = TCTiming[ihit] ;
      maxTCId   = TCId[ihit];
    }
  }
  if (maxTCId < 81) {
    Source = 1;
  } else if (maxTCId > 80 && maxTCId < 513) {
    Source = 2;
  } else {
    Source = 4;
  }

  if (hit_size == 0) {
    m_EventTimingQualityFlag = 0;
  } else {
    if (maxEnergy > m_EventTimingQualityThresholds[1]) {
      m_EventTimingQualityFlag = 3;
    } else if (maxEnergy > m_EventTimingQualityThresholds[0]) {
      m_EventTimingQualityFlag = 2;
    } else {
      m_EventTimingQualityFlag = 1;
    }
  }

  m_EventTimingTCId = maxTCId;
  m_EventTimingTCThetaId = _TCMap->getTCThetaIdFromTCId(maxTCId);
  m_EventTimingTCEnergy = maxEnergy;

  return maxTiming;
}
//========================================================
// Energy weighted TC timing
//========================================================
double TrgEclTiming::GetEventTiming02()
{
  Source = 0;
  std::vector<double> maxEnergy;
  std::vector<double> maxTiming;

  const int NtopTC = NofTopTC;
  int maxTCId = 0;

  maxEnergy.clear();
  maxTiming.clear();
  maxEnergy.resize(NtopTC, 0);
  maxTiming.resize(NtopTC, 0);

  const int hit_size = TCTiming.size();
  double E_sum = 0;
  double EventTiming = 0;

  for (int iNtopTC = 0; iNtopTC < NtopTC ; iNtopTC++) {
    for (int ihit = 0; ihit < hit_size; ihit++) {
      if (iNtopTC == 0) {
        if (maxEnergy[iNtopTC] < TCEnergy[ihit]) {
          maxEnergy[iNtopTC] =  TCEnergy[ihit];
          maxTiming[iNtopTC] =  TCTiming[ihit];
          maxTCId = TCId[ihit];
        }
      } else if (iNtopTC > 0) {
        if (maxEnergy[iNtopTC - 1] > TCEnergy[ihit] &&
            maxEnergy[iNtopTC]     < TCEnergy[ihit]) {
          maxEnergy[iNtopTC] =  TCEnergy[ihit];
          maxTiming[iNtopTC] =  TCTiming[ihit];
        }
      }
    }
    E_sum += maxEnergy[iNtopTC];
    EventTiming += maxEnergy[iNtopTC] * maxTiming[iNtopTC];
  }

  EventTiming /= E_sum;

  if (maxTCId < 81) {
    Source = 1;
  } else if (maxTCId > 80 && maxTCId < 513) {
    Source = 2;
  } else {
    Source = 4;
  }

  return EventTiming;
}
//========================================================
//
//========================================================
