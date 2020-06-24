/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sasha Glazov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/CDCTrackingEventLevelMdstInfoFillerModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

// Register the CDCTrackingEventLevelMdstInfoFillerModule to the framework
REG_MODULE(CDCTrackingEventLevelMdstInfoFiller);


std::string CDCTrackingEventLevelMdstInfoFillerFindlet::getDescription()
{
  return "This module adds additional global event level information about CDC track finding results to the MDST object CDCTrackingEventLevelTrackingInfo";
}

void CDCTrackingEventLevelMdstInfoFillerFindlet::initialize()
{
  Super::initialize();
  m_eventLevelTrackingInfo.isRequired();
}

// Actual work
void CDCTrackingEventLevelMdstInfoFillerFindlet::apply(const std::vector<CDCWireHit>& inputWireHits,
                                                       const std::vector<CDCSegment2D>& inputWireHitSegments)
{
  int nhitTotal = inputWireHits.size(); //total number of hits
  int nTaken = 0;  // bg+assigned to tracks
  int nBg   = 0;  //
  for (CDCWireHit const& hit : inputWireHits) {
    AutomatonCell const& a_cell = hit.getAutomatonCell();
    if (a_cell.hasTakenFlag()) nTaken += 1;
    if (a_cell.hasBackgroundFlag()) nBg += 1;

    if (! a_cell.hasTakenFlag()) {
      // not signal and not background,
      m_eventLevelTrackingInfo->setCDCLayer(hit.getWireID().getICLayer());
    }
  }
  int nSignal = nTaken - nBg;

  B2ASSERT("More background than taken CDC Wire hits", nSignal >= 0);

  int nRest = nhitTotal - nSignal;

  int nRestCleaned = nhitTotal - nTaken;

  m_eventLevelTrackingInfo->setNCDCHitsNotAssigned(nRest);
  m_eventLevelTrackingInfo->setNCDCHitsNotAssignedPostCleaning(nRestCleaned);

  B2DEBUG(10, "Total " << nhitTotal << " taken " << nTaken << " background " << nBg  << " signal " << nTaken - nBg  <<
          " Not assigned " << nRestCleaned);

  // Count 2D segments too:
  nTaken = 0;
  nBg = 0;
  for (CDCSegment2D const& seg : inputWireHitSegments) {
    AutomatonCell const& a_cell = seg.getAutomatonCell();
    if (a_cell.hasTakenFlag()) nTaken += 1;
    if (a_cell.hasBackgroundFlag()) nBg += 1;
  }

  m_eventLevelTrackingInfo->setNCDCSegments(inputWireHitSegments.size() - nTaken);
  B2DEBUG(10, "Total number of segments =" << inputWireHitSegments.size() << " Taken " << nTaken << " BG " << nBg);
}
