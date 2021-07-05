/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "trg/top/dataobjects/TRGTOPUnpackerStore.h"

using namespace std;
using namespace Belle2;

void TRGTOPUnpackerStore::setEventIdL1(int eventIdL1) { m_eventIdL1 = eventIdL1; }
void TRGTOPUnpackerStore::setEventIdTOP(int eventIdTOP) { m_eventIdTOP = eventIdTOP; }
void TRGTOPUnpackerStore::setWindowIdTOP(int windowIdTOP) { m_windowIdTOP = windowIdTOP; }
void TRGTOPUnpackerStore::setRvcB2L(int rvcB2L) { m_rvcB2L = rvcB2L; }
void TRGTOPUnpackerStore::setRvcWindow(int rvcWindow) { m_rvcWindow = rvcWindow; }
void TRGTOPUnpackerStore::setRvcTopTimingDecisionPrev(int rvcTopTimingDecisionPrev) { m_rvcTopTimingDecisionPrev = rvcTopTimingDecisionPrev; }
void TRGTOPUnpackerStore::setRvcTopTimingDecisionNow(int rvcTopTimingDecisionNow) { m_rvcTopTimingDecisionNow = rvcTopTimingDecisionNow; }
void TRGTOPUnpackerStore::setRvcTopTimingDecisionNowGdl(int rvcTopTimingDecisionNowGdl) { m_rvcTopTimingDecisionNowGdl = rvcTopTimingDecisionNowGdl; }
void TRGTOPUnpackerStore::setCombinedTimingTop(int combinedTimingTop) { m_combinedTimingTop = combinedTimingTop; }
void TRGTOPUnpackerStore::setNSlotsCombinedTimingTop(int nSlotsCombinedTimingTop) { m_nSlotsCombinedTimingTop = nSlotsCombinedTimingTop; }
void TRGTOPUnpackerStore::setCombinedTimingTopResidual(int combinedTimingTopResidual) { m_combinedTimingTopResidual = combinedTimingTopResidual; }
void TRGTOPUnpackerStore::setNErrorsMinor(int nErrorsMinor) { m_nErrorsMinor = nErrorsMinor; }
void TRGTOPUnpackerStore::setNErrorsMajor(int nErrorsMajor) { m_nErrorsMajor = nErrorsMajor; }
void TRGTOPUnpackerStore::setTrigType(int trigType) { m_trigType = trigType; }
void TRGTOPUnpackerStore::setNHitSum(int nHitSum) { m_nHitSum = nHitSum; }
void TRGTOPUnpackerStore::setLogLSum(int logLSum) { m_logLSum = logLSum; }
void TRGTOPUnpackerStore::setLogLVar(int logLVar) { m_logLVar = logLVar; }
void TRGTOPUnpackerStore::setTimingVar(int timingVar) { m_timingVar = timingVar; }
//void TRGTOPUnpackerStore::set(int ) { m_ = ; }

void TRGTOPUnpackerStore::setSlotTimingDecision(TRGTOPSlotTiming slotTiming) { m_slotTimingDecisions.push_back(slotTiming); }

// no longer needed (in ROOT v6)
//ClassImp(TRGTOPUnpackerStore)



