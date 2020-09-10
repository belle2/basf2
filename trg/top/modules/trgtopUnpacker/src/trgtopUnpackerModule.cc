/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2019 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Tong Pang, Vladimir Savinov                              *
* Email:  vladimirsavinov@gmail.com
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <trg/top/modules/trgtopUnpacker/trgtopUnpackerModule.h>

/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
* ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TRGTOPUnpacker);

string TRGTOPUnpackerModule::version() const
{
  return string("1.00");
}

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TRGTOPUnpackerModule::TRGTOPUnpackerModule()
  : Module::Module(), m_eventNumber(0), m_trigType(0), m_nodeId(0), m_nWords(0)
{
  // Set module properties


  string desc = "TRGTOPUnpackerModule(" + version() + ")";
  setDescription(desc);
  setPropertyFlags(c_ParallelProcessingCertified);

  B2DEBUG(20, "TRGTOPUnpacker: Constructor done.");


  // Parameter definitions

}

TRGTOPUnpackerModule::~TRGTOPUnpackerModule()
{
}

void TRGTOPUnpackerModule::initialize()
{
  m_TRGTOPCombinedTimingArray.registerInDataStore();
}


void TRGTOPUnpackerModule::beginRun()
{
}

void TRGTOPUnpackerModule::event()
{

  StoreArray<RawTRG> raw_trgarray;

  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {

      m_nodeId = raw_trgarray[i]->GetNodeID(j);

      if (m_nodeId == 0x12000001) {               ////// TRGTOP NodeID is 0x12000001, verified (tp & vs)

        m_nWords       = raw_trgarray[i]->GetDetectorNwords(j, 0);
        m_eventNumber  = raw_trgarray[i]->GetEveNo(j);
        m_trigType     = raw_trgarray[i]->GetTRGType(j);

        //        if ( m_nWords > 3 ) {                                         ////general header is 3 words long
        if (m_nWords > 0) {                                           ////general header is 3 words long

          //    B2INFO("raw_trgarray.getEntries() = " << raw_trgarray.getEntries());
          //    B2INFO("raw_trgarray[i]->GetNumEntries() = " << raw_trgarray[i]->GetNumEntries());
          //    B2INFO("raw_trgarray[]->GetEveNo(j) = " << raw_trgarray[i]->GetEveNo(j));
          //    B2INFO("raw_trgarray[]->GetNodeID(j) = " << std::hex << raw_trgarray[i]->GetNodeID(j) << std::dec);
          //    B2INFO("raw_trgarray[]->GetDetectorNwords(j,0) = " << m_nWords);

          readCOPPEREvent(raw_trgarray[i] , j);

        }
      }
    }
  }
}

void TRGTOPUnpackerModule::readCOPPEREvent(RawTRG* raw_copper, int i)
{
  //  if (raw_copper->GetDetectorNwords(i, 0) > 3) {                ///general header is 3 words long
  if (raw_copper->GetDetectorNwords(i, 0) > 0) {                ///general header is 3 words long
    fillTreeTRGTOP(raw_copper->GetDetectorBuffer(i, 0));
  }
}

void TRGTOPUnpackerModule::fillTreeTRGTOP(int* rdat)
{

  //  B2INFO("data size (32bit words) = " << m_nWords);

  // Information for each data window

  // rvc from the header of the buffer
  int l1_revo = rdat[2] & 0x7ff;  /// L1 timestamp (11 bits)

  // L1 event number
  int trgtag = (rdat[2] >> 12) & 0xfffff; /// 20 LSBs of trgtag (sequential trigger number)

  //  B2INFO("l1_rvc from header = " << l1_revo);
  //  B2INFO("trgtag (evt) from buffer header = " << trgtag);

  bool dataFormatKnown = false;

  int windowSize = -1;
  int numberOfWindows = -1;

  int dataFormatVersionExpected = -1;
  int revoClockDeltaExpected = 4;
  int cntr127DeltaExpected = 4;

  // 3 = 3:  header only
  // 1875 = 3 + 48*39: format used starting ~June 30 2019 and until Oct. 1, 2019 (until Receive FW version 0x02067301)
  // 771 = 3 + 24*32: format used starting ~Oct. 1, 2019 (Receive FW version 0x02067301 and newer)

  //   m_nWords==3 means only a header
  if (m_nWords == 3) {
    windowSize = 0;
    numberOfWindows = 0;
    dataFormatKnown = true;
    dataFormatVersionExpected = 0;
  } else if (m_nWords == 771) {
    windowSize = 32;
    numberOfWindows = 24;
    dataFormatKnown = true;
    dataFormatVersionExpected = 2;
  } else if (m_nWords == 1875) {
    windowSize = 39;
    numberOfWindows = 48;
    dataFormatKnown = true;
    dataFormatVersionExpected = 1;
  }

  if (!dataFormatKnown) {
    B2INFO("Unknown data format / error / exiting");
    return;
  }

  //  if ( dataFormatVersionExpected > 0 ) B2INFO("---------------------------------------------------------------------------------------------------------------");

  //  B2INFO("Data format version (as expected according to data size) = " << dataFormatVersionExpected);

  //  if ( numberOfWindows != 0 ) {
  //    B2INFO("Number of 32bit words in TOP L1 data buffer = " << m_nWords);
  //    B2INFO("Number of windows = " << numberOfWindows);
  //    B2INFO("Window size in 32bit words = " << windowSize);
  //  }

  // various test patterns will be used to check the data
  /* cppcheck-suppress variableScope */
  unsigned int testPattern;

  int revoClockLast = -1;
  int cntr127Last = -1;

  // error counter for possible data corruption
  unsigned int errorCountEvent = 0;

  // need to know when a new decision is made (there could be more than one TOP L1 timing decision stored in the same B2L buffer)
  int t0CombinedDecisionLast = -1;

  // check if this event's buffer is a dummy buffer
  int counterDummyWindows = 0;
  unsigned int testPatternDummyEvent = 0xbbbb;
  for (int iWindow = 0; iWindow < numberOfWindows; iWindow++) {
    int index = iWindow * windowSize + 3;
    testPattern = (rdat[index] >> 16) & 0xffff;
    if (testPattern == testPatternDummyEvent) {
      counterDummyWindows++;
    }
  }

  bool performBufferAnalysis = true;
  // note that events with empty buffer have numberOfWindows=0
  if (counterDummyWindows == numberOfWindows) {
    performBufferAnalysis = false;
  } else {
    if (counterDummyWindows != 0) {
      B2ERROR("Corrupted data? numberOfWindows = " << numberOfWindows << ", counterDummyWondows = " << counterDummyWindows);
      performBufferAnalysis = false;
    }
  }

  // events with no buffer (i.e. no payload), empty (i.e. dummy) windows and presumably corrupted events are NOT analyzed
  if (performBufferAnalysis) {

    // Loop over windows in B2L buffer and locate all unique TOP L1 decisions
    for (int iWindow = 0; iWindow < numberOfWindows; iWindow++) {

      //    B2INFO("window number = " << iWindow);

      // error counters for possible data corruption in the current window
      unsigned int errorCountWindowMinor = 0;
      unsigned int errorCountWindowMajor = 0;

      // a pointer-like variable for accessing the data in the buffer sequentially
      int index = iWindow * windowSize + 3;

      unsigned int testPatternExpected = 0;
      if (dataFormatVersionExpected == 1) testPatternExpected = 0xbbba;
      else if (dataFormatVersionExpected == 2 || dataFormatVersionExpected == 3) testPatternExpected = 0xdddd;

      testPattern = (rdat[index] >> 16) & 0xffff;
      //    B2INFO("testPattern = " << std::hex << testPattern << std::dec);
      if (testPattern != testPatternExpected) {
        //      B2ERROR("Unexpected test pattern 1: " << std::hex << testPattern << std::dec);
        B2ERROR("Unexpected test pattern 1: " << std::hex << testPattern << std::dec << ", window " << iWindow << ", index = " << index);
        errorCountWindowMajor++;
      }

      int dataFormatVersionNow = (rdat[index] >> 11) & 0x1f;
      //    B2INFO("dataFormatVersionNow = " << dataFormatVersionNow);
      if (dataFormatVersionNow == 3 && dataFormatVersionExpected == 2) {
        dataFormatVersionExpected = 3;
      }

      if (dataFormatVersionNow != dataFormatVersionExpected) {
        B2ERROR("Unexpected data format version: " << dataFormatVersionNow << ", window " << iWindow << ", index = " << index);
        errorCountWindowMajor++;
      }

      // revoclk (comes from b2tt) has the range between 0 and 1279 @127MHz => 1279*7.8ns ~10us = 1 revolution (11bits are used)
      int revoClockNow = rdat[index] & 0x7ff;
      //    B2INFO("rvc now = " << revoClockNow);
      // first need to know max revoClock (1279)
      if (revoClockLast != -1 && revoClockNow > revoClockLast)  {
        int revoClockDeltaNow = revoClockNow - revoClockLast;
        if (revoClockDeltaNow != revoClockDeltaExpected) {
          B2ERROR("rvc changed by an unexpected number of units: " << revoClockDeltaNow << ", last rvc = " << revoClockLast <<
                  ", current rvc = " << revoClockNow << ", window " << iWindow << ", index = " << index);
          errorCountWindowMinor++;
        }
      }
      if (revoClockNow > 1279) {
        B2ERROR("Unexpected rvc value = " << revoClockNow << ", window " << iWindow << ", index = " << index);
        errorCountWindowMajor++;
      }
      revoClockLast = revoClockNow;

      int cntr127Now = (rdat[index + 1] >> 16) & 0xffff;
      //    B2INFO("cntr127 now = " << cntr127Now);
      // first need to know max cntr127
      if (cntr127Last != -1 && cntr127Now > cntr127Last)  {
        int cntr127DeltaNow = cntr127Now - cntr127Last;
        if (cntr127DeltaNow != cntr127DeltaExpected) {
          B2ERROR("cntr127 changed by an unexpected number of units: " << cntr127DeltaNow << ", cntr127 last = " << cntr127Last <<
                  ", cntr127 now = " << cntr127Now << ", window " << iWindow << ", index = " << index + 1);
          errorCountWindowMinor++;
        }
      }
      cntr127Last = cntr127Now;

      testPattern = (rdat[index + 1]) & 0x0000ffff;
      //    B2INFO("testPattern = " << std::hex << testPattern << std::dec);
      if (testPattern != 0) {
        B2ERROR("Unexpected test pattern 2: " << testPattern << ", window " << iWindow << ", index = " << index + 1);
        errorCountWindowMajor++;
      }

      // combined t0 from the current window
      int t0CombinedDecisionNow = (rdat[index + 2]) & 0x3ffff;
      int t0CombinedDecisionNowEstimated = 0;
      //    B2INFO("Combined t0 (raw ns) = " << t0CombinedDecisionNow);

      // revo clock when t0 decision was supposed to be posted to GDL
      int revoClockGDL = (rdat[index + 2] >> 18) & 0x7ff;
      //    B2INFO("rvc when t0 was supposed to be posted to GDL/GRL = " << revoClockGDL);

      testPattern = (rdat[index + 2] >> 29) & 0x7;
      //    B2INFO("testPattern = " << std::hex << testPattern << std::dec);
      if (testPattern != 0) {
        B2ERROR("Unexpected test pattern 3: " << testPattern << ", window " << iWindow << ", index = " << index + 2);
        errorCountWindowMajor++;
      }

      int t0CombinedSegments[16];

      t0CombinedSegments[15] = (rdat[index + 11] >> 28) & 0xf;
      t0CombinedSegments[14] = (rdat[index + 11] >> 24) & 0xf;
      t0CombinedSegments[13] = (rdat[index + 11] >> 20) & 0xf;
      t0CombinedSegments[12] = (rdat[index + 11] >> 16) & 0xf;
      t0CombinedSegments[11] = (rdat[index + 11] >> 12) & 0xf;
      t0CombinedSegments[10] = (rdat[index + 11] >>  8) & 0xf;
      t0CombinedSegments[9] = (rdat[index + 11]  >>  4) & 0xf;
      t0CombinedSegments[8] = (rdat[index + 11]) & 0xf;

      t0CombinedSegments[7] = (rdat[index + 12] >> 28) & 0xf;
      t0CombinedSegments[6] = (rdat[index + 12] >> 24) & 0xf;
      t0CombinedSegments[5] = (rdat[index + 12] >> 20) & 0xf;
      t0CombinedSegments[4] = (rdat[index + 12] >> 16) & 0xf;
      t0CombinedSegments[3] = (rdat[index + 12] >> 12) & 0xf;
      t0CombinedSegments[2] = (rdat[index + 12] >>  8) & 0xf;
      t0CombinedSegments[1] = (rdat[index + 12] >>  4) & 0xf;
      t0CombinedSegments[0] = (rdat[index + 12]) & 0xf;

      int nSegmentsCombinedDecision = 0;

      for (int iSlot = 0; iSlot < NUMBER_OF_SLOTS; iSlot++) {
        if (t0CombinedSegments[iSlot] != 0) {
          //  B2INFO("Slot " << iSlot+1 << ", segment = " << t0CombinedSegments[iSlot]);
          nSegmentsCombinedDecision++;
        }
      }

      int t0CombinedSlots[16];

      t0CombinedSlots[0] = (rdat[index + 3] >> 16) & 0xffff;
      t0CombinedSlots[1] = (rdat[index + 3]) & 0xffff;
      t0CombinedSlots[2] = (rdat[index + 4] >> 16) & 0xffff;
      t0CombinedSlots[3] = (rdat[index + 4]) & 0xffff;
      t0CombinedSlots[4] = (rdat[index + 5] >> 16) & 0xffff;
      t0CombinedSlots[5] = (rdat[index + 5]) & 0xffff;
      t0CombinedSlots[6] = (rdat[index + 6] >> 16) & 0xffff;
      t0CombinedSlots[7] = (rdat[index + 6]) & 0xffff;
      t0CombinedSlots[8] = (rdat[index + 7] >> 16) & 0xffff;
      t0CombinedSlots[9] = (rdat[index + 7]) & 0xffff;
      t0CombinedSlots[10] = (rdat[index + 8] >> 16) & 0xffff;
      t0CombinedSlots[11] = (rdat[index + 8]) & 0xffff;
      t0CombinedSlots[12] = (rdat[index + 9] >> 16) & 0xffff;
      t0CombinedSlots[13] = (rdat[index + 9]) & 0xffff;
      t0CombinedSlots[14] = (rdat[index + 10] >> 16) & 0xffff;
      t0CombinedSlots[15] = (rdat[index + 10]) & 0xffff;

      int nSlotsCombinedDecision = 0;

      for (int iSlot = 0; iSlot < NUMBER_OF_SLOTS; iSlot++) {
        if (t0CombinedSegments[iSlot] != 0) {
          //  B2INFO("Slot " << iSlot+1 << ", t0 (raw ns) = " << 3*t0CombinedSlots[iSlot]);
          t0CombinedDecisionNowEstimated = t0CombinedDecisionNowEstimated + 3 * t0CombinedSlots[iSlot];
          nSlotsCombinedDecision++;
        } else if (t0CombinedSlots[iSlot] != 0) {
          B2ERROR("Segment==0 for Slot " << iSlot + 1 << ", t0 (raw ns) = " << 3 * t0CombinedSlots[iSlot] << ", window " << iWindow);
          errorCountWindowMajor++;
        }
      }

      //    if ( nSlotsCombinedDecision == 0 ) error = true;

      if (nSlotsCombinedDecision != 0) {
        t0CombinedDecisionNowEstimated = t0CombinedDecisionNowEstimated / nSlotsCombinedDecision;
      } else {
        t0CombinedDecisionNowEstimated = -1;
        errorCountWindowMajor++;
      }

      int nHitsSlots[16];

      nHitsSlots[0] = (rdat[index + 13] >> 16) & 0x3ff;
      nHitsSlots[1] = (rdat[index + 13]) & 0x3ff;
      nHitsSlots[2] = (rdat[index + 14] >> 16) & 0x3ff;
      nHitsSlots[3] = (rdat[index + 14]) & 0x3ff;
      nHitsSlots[4] = (rdat[index + 15] >> 16) & 0x3ff;
      nHitsSlots[5] = (rdat[index + 15]) & 0x3ff;
      nHitsSlots[6] = (rdat[index + 16] >> 16) & 0x3ff;
      nHitsSlots[7] = (rdat[index + 16]) & 0x3ff;
      nHitsSlots[8] = (rdat[index + 17] >> 16) & 0x3ff;
      nHitsSlots[9] = (rdat[index + 17]) & 0x3ff;
      nHitsSlots[10] = (rdat[index + 18] >> 16) & 0x3ff;
      nHitsSlots[11] = (rdat[index + 18]) & 0x3ff;
      nHitsSlots[12] = (rdat[index + 19] >> 16) & 0x3ff;
      nHitsSlots[13] = (rdat[index + 19]) & 0x3ff;
      nHitsSlots[14] = (rdat[index + 20] >> 16) & 0x3ff;
      nHitsSlots[15] = (rdat[index + 20]) & 0x3ff;

      int nHitsCombinedDecision = 0;

      for (int iSlot = 0; iSlot < NUMBER_OF_SLOTS; iSlot++) {
        if (t0CombinedSegments[iSlot] != 0) {
          //  B2INFO("Slot " << iSlot+1 << ", N hits = " << nHitsSlots[iSlot]);
          nHitsCombinedDecision++;
        } else if (nHitsSlots[iSlot] != 0) {
          B2ERROR("Unexpected nHit = " << nHitsSlots[iSlot] << " for slot " << iSlot << ", window " << iWindow);
          errorCountWindowMajor++;
        }
      }

      // nHits are packed in 10bits, 2 slots per 32bit word
      for (int iWord = 0; iWord < NUMBER_OF_SLOTS / 2; iWord++) {
        testPattern = (rdat[index + 13 + iWord]) & 0xfc00fc00;
        //    B2INFO("testPattern = " << std::hex << testPattern << std::dec);
        if (testPattern != 0) {
          B2ERROR("Unexpected test pattern 4: " << testPattern << ", window " << iWindow << ", index = " << index + 13 + iWord);
          errorCountWindowMajor++;
        }
      }

      int logLikelihoodsSlots[16];

      if (dataFormatVersionExpected == 2 || dataFormatVersionExpected == 3) {
        logLikelihoodsSlots[0] = (rdat[index + 21]) & 0xffff;
        logLikelihoodsSlots[1] = (rdat[index + 21] >> 16) & 0xffff;
        logLikelihoodsSlots[2] = (rdat[index + 22]) & 0xffff;
        logLikelihoodsSlots[3] = (rdat[index + 22] >> 16) & 0xffff;
        logLikelihoodsSlots[4] = (rdat[index + 23]) & 0xffff;
        logLikelihoodsSlots[5] = (rdat[index + 23] >> 16) & 0xffff;
        logLikelihoodsSlots[6] = (rdat[index + 24]) & 0xffff;
        logLikelihoodsSlots[7] = (rdat[index + 24] >> 16) & 0xffff;
        logLikelihoodsSlots[8] = (rdat[index + 25]) & 0xffff;
        logLikelihoodsSlots[9] = (rdat[index + 25] >> 16) & 0xffff;
        logLikelihoodsSlots[10] = (rdat[index + 26]) & 0xffff;
        logLikelihoodsSlots[11] = (rdat[index + 26] >> 16) & 0xffff;
        logLikelihoodsSlots[12] = (rdat[index + 27]) & 0xffff;
        logLikelihoodsSlots[13] = (rdat[index + 27] >> 16) & 0xffff;
        logLikelihoodsSlots[14] = (rdat[index + 28]) & 0xffff;
        logLikelihoodsSlots[15] = (rdat[index + 28] >> 16) & 0xffff;
      } else if (dataFormatVersionExpected == 1) {
        logLikelihoodsSlots[0] = (rdat[index + 21]);
        logLikelihoodsSlots[1] = (rdat[index + 22]);
        logLikelihoodsSlots[2] = (rdat[index + 23]);
        logLikelihoodsSlots[3] = (rdat[index + 24]);
        logLikelihoodsSlots[4] = (rdat[index + 25]);
        logLikelihoodsSlots[5] = (rdat[index + 26]);
        logLikelihoodsSlots[6] = (rdat[index + 27]);
        logLikelihoodsSlots[7] = (rdat[index + 28]);
        logLikelihoodsSlots[8] = (rdat[index + 29]);
        logLikelihoodsSlots[9] = (rdat[index + 30]);
        logLikelihoodsSlots[10] = (rdat[index + 31]);
        logLikelihoodsSlots[11] = (rdat[index + 32]);
        logLikelihoodsSlots[12] = (rdat[index + 33]);
        logLikelihoodsSlots[13] = (rdat[index + 34]);
        logLikelihoodsSlots[14] = (rdat[index + 35]);
        logLikelihoodsSlots[15] = (rdat[index + 36]);
      }

      int nLogLikelihoodsCombinedDecision = 0;

      for (int iSlot = 0; iSlot < NUMBER_OF_SLOTS; iSlot++) {
        if (t0CombinedSegments[iSlot] != 0) {
          //  B2INFO("Slot " << iSlot+1 << ", log likelihood = " << logLikelihoodsSlots[iSlot]);
          nLogLikelihoodsCombinedDecision++;
        } else if (logLikelihoodsSlots[iSlot] != 0) {
          B2ERROR("Unexpected log likelihood = " << logLikelihoodsSlots[iSlot] << " for slot " << iSlot << ", window " << iWindow);
          errorCountWindowMajor++;
        }
      }

      // the rest of the window
      if (dataFormatVersionExpected == 1) index = index + 37;
      else if (dataFormatVersionExpected == 2 || dataFormatVersionExpected == 3) index = index + 29;
      else index = index + 29;

      // trgCntr - currently not working (not incrementing in FW in v02067301, incorrectly implemented in v02007401)
      //    int trgCntrNow = rdat[index];
      //    B2INFO("trgCntrNow = " << trgCntrNow);

      int revoClockT0CombinedDecisionPrev = -1;
      int revoClockT0CombinedDecisionNow = -1;

      // format version 1 does not have trgCntr info
      if (dataFormatVersionExpected == 2 || dataFormatVersionExpected == 3) index++;

      if (dataFormatVersionExpected == 2) {
        // 0xfeedbeef
        testPatternExpected = 0xfeedbeef;
        testPattern = rdat[index];
        //    B2INFO("testPattern = " << std::hex << testPattern << std::dec);
        if (testPattern != testPatternExpected) {
          B2ERROR("Unexpected test pattern 5: " << std::hex << testPattern << std::dec << ", window " << iWindow << ", index = " << index);
          errorCountWindowMajor++;
        }
      } else if (dataFormatVersionExpected == 3) {
        revoClockT0CombinedDecisionPrev = (rdat[index]) & 0x7ff;
        revoClockT0CombinedDecisionNow = (rdat[index] >> 11) & 0x7ff;
      }

      index++;

      // cnttrg: event number at the time of TOP combined t0 decision (should be current L1 event number - 1)
      int cnttrgNow = rdat[index];
      //    B2INFO("trgtag (evt) from buffer header, cnttrg and rvc from window = " << trgtag <<", " << cnttrgNow << ", " << revoClockNow);

      // report and store combined t0 decision - this is where we can write all this info into persistent objects
      if (t0CombinedDecisionNow != t0CombinedDecisionLast) {

        t0CombinedDecisionLast = t0CombinedDecisionNow;

        int t0Residual = t0CombinedDecisionNow - t0CombinedDecisionNowEstimated;

        //-------------------------------------------------------------------------------------------
        /*
          B2INFO("l1_rvc from buffer header, rvc for window# = "
          << l1_revo << ", "
          << revoClockNow << ", "
          << iWindow
          );

          if ( dataFormatVersionExpected == 3 ) {
          B2INFO("rvc for previous and current combined t0 decisions, rvc when the current t0 decision was supposed to be posted to GDL/GRL, current window# = "
          << revoClockT0CombinedDecisionPrev << ", "
          << revoClockT0CombinedDecisionNow << ", "
          << revoClockGDL << ", "
          << iWindow
          );
          }


          B2INFO("trgtag (evt) from buffer header, cnttrg for window# = "
          << trgtag << ", "
          << cnttrgNow << ", "
          << iWindow
          );

          B2INFO("TOP timing (+1735, ns, frame), actual and estimated combined t0 (raw ns, frame9), N slots, discrepancy (raw ns) = " << (int) ((t0CombinedDecisionNow%10240)*clkTo1ns+1735) << ", " << t0CombinedDecisionNow << ", " << t0CombinedDecisionNowEstimated << ", " << nSlotsCombinedDecision << ", " << t0Residual);

          // report slot-level t0 decisions
          for ( int iSlot = 0; iSlot < NUMBER_OF_SLOTS; iSlot++ ) {
          if ( t0CombinedSegments[iSlot] != 0 ) {
          B2INFO("slot= " << iSlot+1
          << ", segment= " << t0CombinedSegments[iSlot]
          << ", TOP timing (+1735, ns, frame) = " << (int) ((3*t0CombinedSlots[iSlot])%10240*clkTo1ns+1735)
          << ", t0 (raw ns, frame9) = " << 3*t0CombinedSlots[iSlot]
          << ", N hits= " << nHitsSlots[iSlot]
          << ", log L= " << logLikelihoodsSlots[iSlot]
          );
          }
          }
        */
        //-------------------------------------------------------------------------------------------

        // store decision in event store
        m_TRGTOPCombinedTimingArray.appendNew();

        int i = m_TRGTOPCombinedTimingArray.getEntries() - 1;

        m_TRGTOPCombinedTimingArray[i]->setEventIdL1(trgtag);
        m_TRGTOPCombinedTimingArray[i]->setEventIdTOP(cnttrgNow);
        m_TRGTOPCombinedTimingArray[i]->setWindowIdTOP(iWindow);
        m_TRGTOPCombinedTimingArray[i]->setRvcB2L(l1_revo);
        m_TRGTOPCombinedTimingArray[i]->setRvcWindow(revoClockNow);
        m_TRGTOPCombinedTimingArray[i]->setRvcTopTimingDecisionPrev(revoClockT0CombinedDecisionPrev);
        m_TRGTOPCombinedTimingArray[i]->setRvcTopTimingDecisionNow(revoClockT0CombinedDecisionNow);
        m_TRGTOPCombinedTimingArray[i]->setRvcTopTimingDecisionNowGdl(revoClockGDL);
        m_TRGTOPCombinedTimingArray[i]->setCombinedTimingTop(t0CombinedDecisionNow);
        m_TRGTOPCombinedTimingArray[i]->setNSlotsCombinedTimingTop(nSlotsCombinedDecision);
        m_TRGTOPCombinedTimingArray[i]->setCombinedTimingTopResidual(t0Residual);
        m_TRGTOPCombinedTimingArray[i]->setNErrorsMinor(errorCountWindowMinor);
        m_TRGTOPCombinedTimingArray[i]->setNErrorsMajor(errorCountWindowMajor);
        m_TRGTOPCombinedTimingArray[i]->setTrigType(m_trigType);

        // now store slot-level decisions

        int nSlots = 0;

        int nHitSum = 0;

        double logLSum_d = 0;
        double logLSum2_d = 0;

        double timingSum_d = 0;
        double timingSum2_d = 0;

        for (int iSlot = 0; iSlot < NUMBER_OF_SLOTS; iSlot++) {
          int slotNErrors = 0;
          // data corruption (no slot-level segment information available but N hits != 0)
          if (t0CombinedSegments[iSlot] == 0 && nHitsSlots[iSlot] != 0) slotNErrors++;
          // store compromised decisions also
          if (t0CombinedSegments[iSlot] != 0 || nHitsSlots[iSlot] != 0) {

            nSlots++;
            nHitSum = nHitSum + nHitsSlots[iSlot];

            logLSum_d = logLSum_d + logLikelihoodsSlots[iSlot];
            logLSum2_d = logLSum2_d + logLikelihoodsSlots[iSlot] * logLikelihoodsSlots[iSlot];

            int timingNow = (3 * t0CombinedSlots[iSlot] % 10240);

            //    B2INFO("   timingNow = " << timingNow );

            timingSum_d = timingSum_d + timingNow;
            timingSum2_d = timingSum2_d + pow(timingNow, 2);

            TRGTOPSlotTiming slotTiming(iSlot + 1);
            slotTiming.setSlotTiming(3 * t0CombinedSlots[iSlot]);
            slotTiming.setSlotSegment(t0CombinedSegments[iSlot]);
            slotTiming.setSlotNHits(nHitsSlots[iSlot]);
            slotTiming.setSlotLogL(logLikelihoodsSlots[iSlot]);
            slotTiming.setSlotNErrors(slotNErrors);
            m_TRGTOPCombinedTimingArray[i]->setSlotTimingDecision(slotTiming);

          }
        }

        if (nSlots == 0) nSlots = 1;

        double logLVar_d = logLSum2_d / nSlots - pow(logLSum_d / nSlots, 2);
        int logLVar = (int)(logLVar_d + 0.5);
        int logLSum = (int)(logLSum_d + 0.5);

        double timingVar_d = timingSum2_d / nSlots - pow(timingSum_d / nSlots, 2);
        int timingVar = (int)(timingVar_d + 0.5);
        //      int timingSum = (int) (timingSum_d + 0.5);
        //      int timingSum2 = (int) (timingSum2_d + 0.5);

        //      B2INFO("nSlots = " << nSlots );
        //      B2INFO("timingSum = " << timingSum );
        //      B2INFO("timingSum2 = " << timingSum2 );
        //      B2INFO("timingVar = " << timingVar );

        if (nSlots == 1) {
          logLVar = -1;
          timingVar = -1;
        }

        m_TRGTOPCombinedTimingArray[i]->setNHitSum(nHitSum);
        m_TRGTOPCombinedTimingArray[i]->setLogLSum(logLSum);
        m_TRGTOPCombinedTimingArray[i]->setLogLVar(logLVar);
        m_TRGTOPCombinedTimingArray[i]->setTimingVar(timingVar);

        //================================================================
        // some inlined validation
        /*
          const vector<Belle2::TRGTOPSlotTiming> tmp = m_TRGTOPCombinedTimingArray[i]->getSlotTimingDecisions();

          cout << "-VS-D-the number of slots in this decision (SlotTimingDecisions.size()) = " << tmp.size() << endl;

          for ( vector<Belle2::TRGTOPSlotTiming>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
          const TRGTOPSlotTiming& slotTiming = *it;

          cout << "-VS-D-slot number   = " << slotTiming.getSlotId() << endl;
          cout << "-VS-D-slot timing   = " << slotTiming.getSlotTiming() << endl;
          cout << "-VS-D-slot segment  = " << slotTiming.getSlotSegment() << endl;
          cout << "-VS-D-slot N hits   = " << slotTiming.getSlotNHits() << endl;
          cout << "-VS-D-slot logL     = " << slotTiming.getSlotLogL() << endl;
          cout << "-VS-D-slot N errors = " << slotTiming.getSlotNErrors() << endl;

          }
        */
        //
        //================================================================

      }

      errorCountEvent = errorCountEvent + errorCountWindowMinor + errorCountWindowMajor;

    }
  }

  // at this time any unexpected features in the data are lumped together
  // this includes possibly corrupted data (checksum would be a better solution to diagnose such problem)
  // AND incorrectly prepared (but not corrupted) data
  if (errorCountEvent != 0) {
    B2INFO("Number of instances of unexpected data diagnozed during unpacking = " << errorCountEvent);
  }

  // need to store the info about error rate / type of errors in unpacking
  // need a new TObject for that (NOT an Array)

}


void TRGTOPUnpackerModule::endRun()
{
}

void TRGTOPUnpackerModule::terminate()
{
}



