/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/CDCTrackingEventLevelMdstInfoFillerModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

// Register the CDCTrackingEventLevelMdstInfoFillerModule to the framework
REG_MODULE(CDCTrackingEventLevelMdstInfoFiller);

CDCTrackingEventLevelMdstInfoFillerFindlet::CDCTrackingEventLevelMdstInfoFillerFindlet()
{
}

std::string CDCTrackingEventLevelMdstInfoFillerFindlet::getDescription()
{
  return "This module adds additional global event level information about CDC track finding results to the MDST object CDCTrackingEventLevelTrackingInfo";
}

void CDCTrackingEventLevelMdstInfoFillerFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
}

void CDCTrackingEventLevelMdstInfoFillerFindlet::initialize()
{
  Super::initialize();
  m_eventLevelTrackingInfo.registerInDataStore();
}

// Actual work
void CDCTrackingEventLevelMdstInfoFillerFindlet::apply(const std::vector<CDCWireHit>& inputWireHits)
{
  if (!m_eventLevelTrackingInfo.isValid()) {
    m_eventLevelTrackingInfo.create();
  }

  int nhitTotal = inputWireHits.size(); //total number of hits
  int nTaken = 0;  // bg+assigned to tracks
  int nBg   = 0;  //
  for (CDCWireHit const& hit : inputWireHits) {
    AutomatonCell const& a_cell = hit.getAutomatonCell();
    if (a_cell.hasTakenFlag()) nTaken += 1;
    if (a_cell.hasBackgroundFlag()) nBg += 1;
  }
  int nSignal = nTaken - nBg;

  B2ASSERT("More taken than background hits", nSignal >= 0);

  int nRest = nhitTotal - nSignal;

  int nRestCleaned = nhitTotal - nTaken;

  m_eventLevelTrackingInfo->setNCDCHitsNotAssigned(nRest);
  m_eventLevelTrackingInfo->setNCDCHitsNotAssignedPostCleaning(nRestCleaned);

  B2DEBUG(10, "Total " << nhitTotal << " taken " << nTaken << " background " << nBg  << " signal " << nTaken - nBg  <<
          " Not assigned " << nRestCleaned);
}
