/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/CDCTrackingEventLevelMdstInfoFiller.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;


// Register the modules to the framework
REG_MODULE(CDCTrackingEventLevelMdstInfoFillerFromHits);
REG_MODULE(CDCTrackingEventLevelMdstInfoFillerFromSegments);


std::string CDCTrackingEventLevelMdstInfoFillerFromHitsFindlet::getDescription()
{
  return "This module adds additional global event level information based on hits about CDC track finding results to the MDST object EventLevelTrackingInfo";
}

void CDCTrackingEventLevelMdstInfoFillerFromHitsFindlet::initialize()
{
  Super::initialize();
  m_eventLevelTrackingInfo.isRequired();
}

void CDCTrackingEventLevelMdstInfoFillerFromHitsFindlet::apply(const std::vector<CDCWireHit>& inputWireHits)
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

  B2DEBUG(20, "Total " << nhitTotal << " taken " << nTaken << " background " << nBg  << " signal " << nTaken - nBg  <<
          " Not assigned " << nRestCleaned);
}


std::string CDCTrackingEventLevelMdstInfoFillerFromSegmentsFindlet::getDescription()
{
  return "This module adds additional global event level information based on 2D segments about CDC track finding results to the MDST object EventLevelTrackingInfo";
}

void CDCTrackingEventLevelMdstInfoFillerFromSegmentsFindlet::initialize()
{
  Super::initialize();
  m_eventLevelTrackingInfo.isRequired();
}

void CDCTrackingEventLevelMdstInfoFillerFromSegmentsFindlet::apply(const std::vector<CDCSegment2D>& inputWireHitSegments)
{
  // Count 2D segments
  int nTaken = 0;
  int nBg = 0;
  for (CDCSegment2D const& seg : inputWireHitSegments) {
    AutomatonCell const& a_cell = seg.getAutomatonCell();
    if (a_cell.hasTakenFlag()) nTaken += 1;
    if (a_cell.hasBackgroundFlag()) nBg += 1;
  }

  m_eventLevelTrackingInfo->setNCDCSegments(inputWireHitSegments.size() - nTaken);
  B2DEBUG(20, "Total number of segments =" << inputWireHitSegments.size() << " Taken " << nTaken << " BG " << nBg);
}
