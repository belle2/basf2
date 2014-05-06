/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/vxdCaTracking/ExporterEventInfo.h"


using namespace std;
using namespace Belle2;
// using namespace Belle2::Tracking;



void ExporterEventInfo::addHit(ExporterHitInfo aHit) { m_hits.push_back(aHit); }



void ExporterEventInfo::addTc(ExporterTcInfo aTc) { m_tcs.push_back(aTc); }



std::vector<ExporterHitInfo>* ExporterEventInfo::getHits() { return &m_hits; }



std::vector<ExporterTcInfo>* ExporterEventInfo::getTcs() { return &m_tcs; }



int ExporterEventInfo::getNPXDTrueHits() { return countHitsOfType(0); }



int ExporterEventInfo::getNSVDTrueHits() { return countHitsOfType(1); }


int ExporterEventInfo::getNMCHits()
{
  int nMCHits = 0;
  for (ExporterTcInfo tc : m_tcs) {
    nMCHits += tc.getNHits();
  }
  return nMCHits;
}

int ExporterEventInfo::countHitsOfType(int type)
{
  int counter = 0;
  for (ExporterHitInfo hit : m_hits) {
    if (hit.getType() == type) { ++counter; }
  }
  return counter;
}