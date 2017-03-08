//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclTiming.cc
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGECLCLUSTER_SHORT_NAMES
#include <framework/gearbox/Unit.h>


#include <framework/logging/Logger.h>
#include "trg/ecl/TrgEclTiming.h"

using namespace std;
using namespace Belle2;
//
//
//
TrgEclTiming::TrgEclTiming() : NofTopTC(3)
{

  _TCMap = new TrgEclMapping();
  TCEnergy.clear();
  TCTiming.clear();
  TCId.clear();

}

TrgEclTiming::~TrgEclTiming()
{

  delete _TCMap;
}
void TrgEclTiming::Setup(std::vector<int> HitTCId, std::vector<double>HitTCEnergy, std::vector<double> HitTCTiming)
{
  TCId = HitTCId;
  TCEnergy = HitTCEnergy;
  TCTiming = HitTCTiming;
  return;
}

double TrgEclTiming::GetEventTiming(int method)
{
  double EventTiming = 0;

  if (method == 0) { // Fastest timing (belle)
    EventTiming =  GetEventTiming00();
  } else if (method == 1) { // Maximum energy
    EventTiming =  GetEventTiming01();
  } else { // Energy weighted timing

    EventTiming =  GetEventTiming02();
  }

  return EventTiming;
}
double TrgEclTiming::GetEventTiming00()
{
  double Fastest = 9999;
  const int hit_size = TCTiming.size();

  for (int ihit = 0; ihit < hit_size; ihit++) {
    if (TCTiming[ihit] < Fastest) {
      Fastest = TCTiming[ihit];
    }
  }

  return Fastest ;

}

double TrgEclTiming::GetEventTiming01()
{
  double maxEnergy = 0;
  double maxTiming = 0;

  const int hit_size = TCTiming.size();

  for (int ihit = 0; ihit < hit_size; ihit++) {
    if (TCEnergy[ihit] > maxEnergy) {
      maxEnergy = TCEnergy[ihit] ;
      maxTiming = TCTiming[ihit] ;
    }
  }

  return maxTiming;

}


double TrgEclTiming::GetEventTiming02()
{
  std::vector<double> maxEnergy;
  std::vector<double> maxTiming;

  const int NtopTC = NofTopTC;

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
        }
      } else if (iNtopTC > 0) {
        if (maxEnergy[iNtopTC - 1] > TCEnergy[ihit] && maxEnergy[iNtopTC] < TCEnergy[ihit]) {
          maxEnergy[iNtopTC] =  TCEnergy[ihit];
          maxTiming[iNtopTC] =  TCTiming[ihit];
        }
      }
    }
    E_sum += maxEnergy[iNtopTC];
    EventTiming += maxEnergy[iNtopTC] * maxTiming[iNtopTC];
  }

  EventTiming /= E_sum;


  return EventTiming;

}


//
//===<END>
//
