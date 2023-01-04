/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/top/modules/trgtopUnpacker/trgtopUnpackerModule.h>

#include <iostream>

/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
* ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>

//using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TRGTOPUnpacker);

std::string TRGTOPUnpackerModule::version() const
{
  return std::string("1.00");
}

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TRGTOPUnpackerModule::TRGTOPUnpackerModule()
  : Module::Module(), m_eventNumber(0), m_trigType(0), m_nodeId(0), m_nWords(0), m_reportedAlreadyRun_1(false),
    m_reportedAlreadyRun_2(false)
{
  // Set module properties


  std::string desc = "TRGTOPUnpackerModule(" + version() + ")";
  setDescription(desc);
  setPropertyFlags(c_ParallelProcessingCertified);

  B2DEBUG(20, "TRGTOPUnpacker: Constructor done.");


  // Parameter definitions
  addParam("overrideControlBits", m_overrideControlBits,
           "Override control bits in data",
           true);
  //           false);

}

TRGTOPUnpackerModule::~TRGTOPUnpackerModule()
{
}

void TRGTOPUnpackerModule::initialize()
{

  m_TRGTOPCombinedT0DecisionArray.registerInDataStore();
  //  m_TRGTOPCombinedTimingArray.registerInDataStore();
  m_TRGTOPSlotTimingArray.registerInDataStore();

  //  m_TRGTOPCombinedTimingArray.registerRelationTo(m_TRGTOPSlotTimingArray);
  //  m_TRGTOPSlotTimingArray.registerRelationTo(m_TRGTOPCombinedTimingArray);

}


void TRGTOPUnpackerModule::beginRun()
{
  m_reportedAlreadyRun_1 = false;
  m_reportedAlreadyRun_2 = false;
}

void TRGTOPUnpackerModule::event()
{

  StoreArray<RawTRG> raw_trgarray;

  for (int i = 0; i < raw_trgarray.getEntries(); i++) {

    // Check PCIe40 data or Copper data
    if (raw_trgarray[i]->GetMaxNumOfCh(0) == 48) { m_pciedata = true; }
    else if (raw_trgarray[i]->GetMaxNumOfCh(0) == 4) { m_pciedata = false; }
    else { B2FATAL("TRGTOPUnpackerModule: Invalid value of GetMaxNumOfCh from raw data: " << LogVar("Number of ch: ", raw_trgarray[i]->GetMaxNumOfCh(0))); }

    int node_id = 0;
    int ch_id_1 = 0;
    int ch_id_2 = 1;
    if (m_pciedata) {
      node_id = 0x10000001;
      ch_id_1 = 23;
      ch_id_2 = 24;
    } else {
      node_id = 0x12000001;
      ch_id_1 = 0;
      ch_id_2 = 1;
    }

    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {

      m_nodeId = raw_trgarray[i]->GetNodeID(j);

      if (m_nodeId == node_id) {

        int numberOfChannels = raw_trgarray[i]->GetMaxNumOfCh(i);

        //  B2INFO("raw_trgarray.GetMaxNumOfCh() = " << numberOfChannels);

        for (int channel = 0; channel < numberOfChannels; channel++) {

          if (channel != ch_id_1 && channel != ch_id_2) continue;

          m_nWords       = raw_trgarray[i]->GetDetectorNwords(j, channel);

          //    B2INFO("raw_trgarray[" << i << "]->GetDetectorNwords(" << j << ", " << channel << ") = " << m_nWords);

          //        if ( m_nWords > 3 ) {                                         ////general header is 3 words long
          if (m_nWords > 0) {

            m_eventNumber  = raw_trgarray[i]->GetEveNo(j);
            m_trigType     = raw_trgarray[i]->GetTRGType(j);

            //          B2INFO("raw_trgarray.getEntries() = " << raw_trgarray.getEntries());
            //          B2INFO("raw_trgarray[i]->GetNumEntries() = " << raw_trgarray[i]->GetNumEntries());
            //          B2INFO("raw_trgarray[]->GetEveNo(j) = " << raw_trgarray[i]->GetEveNo(j));
            //          B2INFO("raw_trgarray[]->GetNodeID(j) = " << std::hex << raw_trgarray[i]->GetNodeID(j) << std::dec);
            //          B2INFO("raw_trgarray[]->GetDetectorNwords(j,0) = " << m_nWords);

            readDAQEvent(raw_trgarray[i], j, channel);

          }
        }
      }
    }
  }
}

void TRGTOPUnpackerModule::readDAQEvent(RawTRG* raw_daq, int j, int channel)
{
  //  if (raw_daq->GetDetectorNwords(j, channel) > 3) {                ///general header is 3 words long
  if (raw_daq->GetDetectorNwords(j, channel) > 0) {
    unpackT0Decisions(raw_daq->GetDetectorBuffer(j, channel), channel);
  }
}

void TRGTOPUnpackerModule::unpackT0Decisions(int* rdat, int channel)
{

  //  B2INFO("channel, data size (32bit words) = " << channel << ", " << m_nWords);

  // Information for each data window

  // rvc from the header of the buffer
  int rvcL1 = rdat[2] & 0x7ff;  /// L1 timestamp (11 bits)

  // L1 event number
  int eventNumberL1 = (rdat[2] >> 12) & 0xfffff; /// 20 LSBs of eventNumberL1 (sequential trigger number)

  //  B2INFO("l1_rvc from header = " << rvcL1);
  //  B2INFO("eventNumberL1 (evt) from buffer header = " << eventNumberL1);

  bool dataFormatKnown = false;

  int windowSize = -1;
  int numberOfWindows = -1;

  int dataFormatVersionExpected = -1;
  // cppcheck-suppress variableScope
  int revoClockDeltaExpected = 4;
  // cppcheck-suppress variableScope
  int cntr127DeltaExpected = 4;

  // 3 = 3:  header only
  // 1875 = 3 + 48*39: format used starting ~June 30 2019 and until Oct. 1, 2019 (until Receive FW version 0x02067301)
  // 771 = 3 + 24*32: format used starting ~Oct. 1, 2019 (Receive FW version 0x02067301 and newer)
  // 1539 = 3 + 48*32: format used starting ~Mar. 25, 2021 (Receive FW version 0x03020003 and newer)
  // 3075 = 3 + 96*32: format used starting ~Mar. 25, 2021 (Receive FW version 0x03020003 and newer)

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
  } else if (m_nWords == 1539) {
    windowSize = 32;
    numberOfWindows = 48;
    dataFormatKnown = true;
    dataFormatVersionExpected = 4;
  } else if (m_nWords == 3075) {
    windowSize = 32;
    numberOfWindows = 96;
    dataFormatKnown = true;
    dataFormatVersionExpected = 5;
  }

  if (!dataFormatKnown) {
    if (!m_reportedAlreadyRun_1) {
      B2INFO("Unknown data format / error / exiting. This condition is reported only once per run.");
      m_reportedAlreadyRun_1 = true;
    }
    return;
  }

  // FTSW / rvc / the difference is 1280 (expected)
  //  int revoClockDeltaJump1 = -92;
  //  int revoClockDeltaJump2 = 1188;

  // VME / 16bit counter
  //  int cntr127DeltaJump1 = -92;
  //  int cntr127DeltaJump2 = -65532;

  //  if ( dataFormatVersionExpected > 0 ) B2INFO("---------------------------------------------------------------------------------------------------------------");

  //  B2INFO("Data format version (as expected according to data size) = " << dataFormatVersionExpected);

  //  if ( numberOfWindows != 0 ) {
  //    B2INFO("Number of 32bit words in TOP L1 data buffer = " << m_nWords);
  //    B2INFO("Number of windows = " << numberOfWindows);
  //    B2INFO("Window size in 32bit words = " << windowSize);
  //  }

  // various test patterns will be used to check the data
  unsigned int testPattern;

  int revoClockLast = -1;
  int cntr127Last = -1;

  bool performBufferAnalysis = true;
  bool reportAllErrors = true;
  // bool reportSummaryErrors = true;

  // check if this event's buffer is a dummy buffer
  int counterDummyWindows = 0;
  unsigned int testPatternDummyEvent = 0xbbbb;
  for (int iWindow = 0; iWindow < numberOfWindows; iWindow++) {
    int index = iWindow * windowSize + 3;
    testPattern = (rdat[index] >> 16) & 0xffff;
    if (testPattern == testPatternDummyEvent) {
      counterDummyWindows++;
    }
    // March 30, 2022: need to be able to override the settings in data
    if (!m_overrideControlBits) {
      // Oct. 31, 2020: three most significant bits are now used to control unpacker from FW
      // Note that setting either flag for any of the windows changes it for all windows here
      testPattern = (rdat[index + 2] >> 29) & 0x7;
      if (testPattern & 0x1) performBufferAnalysis = false;
      if (testPattern & 0x2) reportAllErrors = false;
      // if (testPattern & 0x4) reportSummaryErrors = false;
    }
  }

  // note that events with empty buffer have numberOfWindows=0
  if (counterDummyWindows == numberOfWindows) {
    performBufferAnalysis = false;
  } else {
    if (counterDummyWindows != 0) {
      if (reportAllErrors) B2ERROR("Corrupted data? numberOfWindows = " << numberOfWindows << ", counterDummyWindows = " <<
                                     counterDummyWindows);
      performBufferAnalysis = false;
    }
  }

  /*
  int numberRvcJumps = 0;
  int numberCntr127Jumps = 0;
  int windowRvcJumpFirst = -1;
  int windowCntr127JumpFirst = -1;
  int clocksRvcJumpFirst = -1;
  int clocksCntr127JumpFirst = -1;

  if (performBufferAnalysis) {
    for (int iWindow = 0; iWindow < numberOfWindows; iWindow++) {
      int index = iWindow * windowSize + 3;

      // revoclk (comes from b2tt) has the range between 0 and 1279 @127MHz => 1279*7.8ns ~10us = 1 revolution (11bits are used)
      int revoClockNow = rdat[index] & 0x7ff;
      //    B2INFO("rvc now = " << revoClockNow);
      // first need to know max revoClock (1279)
      if (revoClockLast != -1)  {
        //      if (revoClockLast != -1 && revoClockNow > revoClockLast)  {
        int revoClockDeltaNow = revoClockNow - revoClockLast;
        if (revoClockDeltaNow != revoClockDeltaExpected) {
          // -1276 is simply going to the next cycle of rvc counting, but 1188 or -92 means that there was one frame missing in data / corrupted data
          if (revoClockDeltaNow != -1276) {
            if (reportAllErrors) B2INFO("rvc changed by an unexpected number of units (1): " << revoClockDeltaNow << ", last rvc = " <<
                                           revoClockLast <<
                                           ", current rvc = " << revoClockNow << ", window " << iWindow << ", index = " << index);
            numberRvcJumps++;
            if (windowRvcJumpFirst < 0) {
              windowRvcJumpFirst = iWindow;
              clocksRvcJumpFirst = revoClockDeltaNow;
            }
          }
        }
      }
      revoClockLast = revoClockNow;

      int cntr127Now = (rdat[index + 1] >> 16) & 0xffff;
      //    B2INFO("cntr127 now = " << cntr127Now);
      // first need to know max cntr127
      if (cntr127Last != -1)  {
        //      if (cntr127Last != -1 && cntr127Now > cntr127Last)  {
        int cntr127DeltaNow = cntr127Now - cntr127Last;
        if (cntr127DeltaNow != cntr127DeltaExpected) {
          // 65444 is the value of the difference in cntr127 (VME counter) because we use 16 bits of 64 bit-long counter
          if (cntr127DeltaNow != 65444) {
            if (reportAllErrors) B2INFO("cntr127 changed by an unexpected number of units (1): " << cntr127DeltaNow << ", cntr127 last = " <<
                                           cntr127Last <<
                                           ", cntr127 now = " << cntr127Now << ", window " << iWindow << ", index = " << index + 1);
            numberCntr127Jumps++;
            if (windowCntr127JumpFirst < 0) {
              windowCntr127JumpFirst = iWindow;
              clocksCntr127JumpFirst = cntr127DeltaNow;
            }
          }
        }
      }
      cntr127Last = cntr127Now;
    }
  }
  */

  /*
  if (numberRvcJumps > 0) {
    B2INFO("The number of rvc jumps = " << numberRvcJumps);
    B2INFO("The window of the first rvc jump = " << windowRvcJumpFirst);
    B2INFO("The number of clock cycles associated with the first rvc jump = " << clocksRvcJumpFirst);
  }

  if (numberCntr127Jumps > 0) {
    B2INFO("The number of cntr127 jumps = " << numberCntr127Jumps);
    B2INFO("The window of the first cntr127 jump = " << windowCntr127JumpFirst);
    B2INFO("The number of clock cycles associated with the first cntr127 jump = " << clocksCntr127JumpFirst);
  }
  */

  // debugging: report everything from every single window when we are seeing unexpected jumps in either of the two counters
  /*
  if (numberRvcJumps > 0 || numberCntr127Jumps > 0) {
    B2INFO("===========================================================================================================");
    B2INFO("l1_rvc from header = " << rvcL1);
    B2INFO("eventNumberL1 (evt) from buffer header = " << eventNumberL1);
    B2INFO("Reporting the entire data buffer");
    B2INFO("Header 0 = : " << std::hex << rdat[0] << std::dec);
    B2INFO("Header 1 = : " << std::hex << rdat[1] << std::dec);
    B2INFO("Header 2 = : " << std::hex << rdat[2] << std::dec);
    for (int iWindow = 0; iWindow < numberOfWindows; iWindow++) {
      int index = iWindow * windowSize + 3;

      B2INFO("---------------------------------------------------------------------------------");
      int revoClockNow = rdat[index] & 0x7ff;
      B2INFO("w rvc ----------------------------     = " << iWindow << " " << revoClockNow);

      int cntr127Now = (rdat[index + 1] >> 16) & 0xffff;
      B2INFO("w cntr127 ---------------------------- = " << iWindow << " " << cntr127Now);

      for (int i = 0; i < 24; i++) {
  B2INFO("w i = : " << iWindow << " " << i << " " << std::hex << rdat[index+i] << std::dec);
      }
    }
  }
  */

  // reset "most recent" rvc and cntr127
  revoClockLast = -1;
  cntr127Last = -1;

  // events with no buffer (i.e. no payload), empty (i.e. dummy) windows and presumably corrupted events are NOT analyzed
  if (performBufferAnalysis) {

    int slotT0Last[8];
    int slotNHitLast[8];
    int slotSegmentLast[8];

    int otherInformationLast[8];

    int combinedT0Last;
    int combinedT0RVC2GDLLast;

    // Loop over windows in B2L buffer and locate all unique slot-level TOP L1 decisions
    for (int iWindow = 0; iWindow < numberOfWindows; iWindow++) {

      //      B2INFO("REAL DEBUG:  window number = " << iWindow);

      int clockCycle = iWindow * 4;

      // error counters for possible data corruption in the current window
      unsigned int errorCountWindow = 0;

      // a pointer-like variable for accessing the data in the buffer sequentially
      int index = iWindow * windowSize + 3;

      unsigned int testPatternExpected = 0;
      if (dataFormatVersionExpected == 1) testPatternExpected = 0xbbba;
      else if (dataFormatVersionExpected >= 2) testPatternExpected = 0xdddd;

      testPattern = (rdat[index] >> 16) & 0xffff;
      //    B2INFO("testPattern = " << std::hex << testPattern << std::dec);
      if (testPattern != testPatternExpected) {
        if (reportAllErrors) B2ERROR("An unexpected test pattern: " << std::hex << testPattern << std::dec << ", window " << iWindow <<
                                       ", index = " << index);
        errorCountWindow++;
      }

      int dataFormatVersionNow = (rdat[index] >> 11) & 0x1f;
      //    B2INFO("dataFormatVersionNow = " << dataFormatVersionNow);
      if (dataFormatVersionNow == 3 && dataFormatVersionExpected == 2) {
        dataFormatVersionExpected = 3;
      }
      if (dataFormatVersionNow == 4 && dataFormatVersionExpected == 2) {
        dataFormatVersionExpected = 4;
      }
      if (dataFormatVersionNow == 5) {
        dataFormatVersionExpected = 5;
      }

      if (dataFormatVersionNow != dataFormatVersionExpected) {
        if (reportAllErrors) {
          if (!m_reportedAlreadyRun_2) {
            B2ERROR("An unexpected data format version: " << dataFormatVersionNow << ", window " << iWindow << ", index = " <<
                    index);
            m_reportedAlreadyRun_2 = true;
          }
        }
        errorCountWindow++;
      }

      // revoclk (comes from b2tt) has the range between 0 and 1279 @127MHz => 1279*7.8ns ~10us = 1 revolution (11bits are used)
      int revoClockNow = rdat[index] & 0x7ff;
      //      B2INFO("window = " << iWindow << ", rvc now = " << revoClockNow);
      // first need to know max revoClock (1279)
      if (revoClockLast != -1)  {
        //      if (revoClockLast != -1 && revoClockNow > revoClockLast)  {
        int revoClockDeltaNow = revoClockNow - revoClockLast;
        if (revoClockDeltaNow != revoClockDeltaExpected) {
          if (revoClockDeltaNow != -1276) {
            if (reportAllErrors) B2INFO("rvc changed by an unexpected number of units (2): " << revoClockDeltaNow << ", last rvc = " <<
                                          revoClockLast <<
                                          ", current rvc = " << revoClockNow << ", window " << iWindow << ", index = " << index);
            //            errorCountWindow++;
          }
        }
      }
      if (revoClockNow > 1279) {
        if (reportAllErrors) B2ERROR("An unexpected rvc value = " << revoClockNow << ", window " << iWindow << ", index = " << index);
        errorCountWindow++;
      }
      revoClockLast = revoClockNow;

      int cntr127Now = (rdat[index + 1] >> 16) & 0xffff;
      //    B2INFO("cntr127 now = " << cntr127Now);
      // first need to know max cntr127
      if (cntr127Last != -1) {
        //      if (cntr127Last != -1 && cntr127Now > cntr127Last)  {
        int cntr127DeltaNow = cntr127Now - cntr127Last;
        if (cntr127DeltaNow != cntr127DeltaExpected) {
          // 65444 is the value of the difference in cntr127 (VME counter) because we use 16 bits of 64 bit-long counter
          if (cntr127DeltaNow != 65444 && cntr127DeltaNow != -65532) {
            if (reportAllErrors) B2INFO("cntr127 changed by an unexpected number of units (2): " << cntr127DeltaNow << ", cntr127 last = " <<
                                          cntr127Last <<
                                          ", cntr127 now = " << cntr127Now << ", window " << iWindow << ", index = " << index + 1);
            errorCountWindow++;
          }
        }
      }
      cntr127Last = cntr127Now;

      //

      int slotT0[8];
      int slotNHit[8];
      int slotSegment[8];

      int otherInformation[8];

      //      int versionFW = rdat[index + 3];

      int combinedT0 = rdat[index + 2] & 0x3ffff;
      int combinedT0RVC2GDL = (rdat[index + 2] >> 18) & 0x7ff;
      int combinedT0ClockCycle = clockCycle;

      slotT0[0] = (rdat[index + 4] >> 16) & 0xffff;
      slotT0[1] = (rdat[index + 4]) & 0xffff;
      slotT0[2] = (rdat[index + 5] >> 16) & 0xffff;
      slotT0[3] = (rdat[index + 5]) & 0xffff;
      slotT0[4] = (rdat[index + 6] >> 16) & 0xffff;
      slotT0[5] = (rdat[index + 6]) & 0xffff;
      slotT0[6] = (rdat[index + 7] >> 16) & 0xffff;
      slotT0[7] = (rdat[index + 7]) & 0xffff;

      slotNHit[0] = (rdat[index + 8] >> 20) & 0x3ff;
      slotNHit[1] = (rdat[index + 8] >> 10) & 0x3ff;
      slotNHit[2] = (rdat[index + 8]) & 0x3ff;

      slotNHit[3] = (rdat[index + 9] >> 20) & 0x3ff;
      slotNHit[4] = (rdat[index + 9] >> 10) & 0x3ff;
      slotNHit[5] = (rdat[index + 9]) & 0x3ff;

      slotNHit[6] = (rdat[index + 10] >> 10) & 0x3ff;
      slotNHit[7] = (rdat[index + 10]) & 0x3ff;

      slotSegment[0] = (rdat[index + 11] >> 28) & 0xf;
      slotSegment[1] = (rdat[index + 11] >> 24) & 0xf;
      slotSegment[2] = (rdat[index + 11] >> 20) & 0xf;
      slotSegment[3] = (rdat[index + 11] >> 16) & 0xf;
      slotSegment[4] = (rdat[index + 11] >> 12) & 0xf;
      slotSegment[5] = (rdat[index + 11] >> 8) & 0xf;
      slotSegment[6] = (rdat[index + 11] >> 4) & 0xf;
      slotSegment[7] = (rdat[index + 11] >> 0) & 0xf;

      // the following information is board-dependent
      otherInformation[0] = (rdat[index + 12] >> 16) & 0xffff;
      otherInformation[1] = (rdat[index + 12]) & 0xffff;
      otherInformation[2] = (rdat[index + 13] >> 16) & 0xffff;
      otherInformation[3] = (rdat[index + 13]) & 0xffff;
      otherInformation[4] = (rdat[index + 14] >> 16) & 0xffff;
      otherInformation[5] = (rdat[index + 14]) & 0xffff;
      otherInformation[6] = (rdat[index + 15] >> 16) & 0xffff;
      otherInformation[7] = (rdat[index + 15]) & 0xffff;

      if (iWindow != 0) {

        // try to retrieve combined t0 decisions (limited info)
        if (channel == 0) {
          if (combinedT0 != combinedT0Last ||
              combinedT0RVC2GDL != combinedT0RVC2GDLLast
             ) {

            //      int toptrgT0_revoToNS = combinedT0%revoToNS;
            //      int rvcL1_NS = rvcL1 * 8;
            //      int deltaT0 = rvcL1_NS >= toptrgT0_revoToNS ? rvcL1_NS - toptrgT0_revoToNS : rvcL1_NS - toptrgT0_revoToNS + revoToNS;
            //      int latencyL12TOPTRG = revoClockNow >= rvcL1 ? revoClockNow - rvcL1 : revoClockNow - rvcL1 + 1280;

            TRGTOPCombinedT0Decision combinedT0Decision(combinedT0,
                                                        revoClockNow,
                                                        combinedT0ClockCycle,
                                                        combinedT0RVC2GDL,
                                                        eventNumberL1,
                                                        rvcL1
                                                       );

            m_TRGTOPCombinedT0DecisionArray.appendNew(combinedT0Decision);

            /*
            cout << "-DEBUG- new combined decision: t0 (2ns), eventNumberL1, rvc2GDL, clock cycle, rvcNOW, rvcL1, latency, deltaT0 (ns) = "
            << combinedT0/2 << ", "
            << eventNumberL1 << ", "
            << combinedT0RVC2GDL << ", "
            << combinedT0ClockCycle << ", "
            << revoClockNow << ", "
            << rvcL1 << ", "
            << latencyL12TOPTRG << ", "
            << deltaT0
            << endl;
            */

            combinedT0Last = combinedT0;
            combinedT0RVC2GDLLast = combinedT0RVC2GDL;
          }

          // retrieve slot-level decisions (limited info) for slots 1 through 8 as observed on the board used for slots 9 through 16
          for (int i = 0; i < 8; i++) {

            if (otherInformation[i] != otherInformationLast[i]) {

              int slotOther = i + 1;

              TRGTOPSlotTiming slotTiming(slotOther,
                                          2 * otherInformation[i],
                                          0,
                                          0,
                                          0,
                                          clockCycle,
                                          errorCountWindow,
                                          0,
                                          -1
                                         );

              //        auto* thisSlotTiming = m_TRGTOPSlotTimingArray.appendNew(slotTiming);
              m_TRGTOPSlotTimingArray.appendNew(slotTiming);

              otherInformationLast[i] = otherInformation[i];

            }
          }
        }

        // retrieve slot-level decisions (complete info) for the currently-processed readout board

        int slotNow = 1;
        if (channel == 0) slotNow = 9;

        for (int i = 0; i < 8; i++) {

          if (slotT0[i] != slotT0Last[i] ||
              slotNHit[i] != slotNHitLast[i] ||
              slotSegment[i] != slotSegmentLast[i]) {

            /*
            cout << "-DEBUG- slot, index, clockCycle = " << slotNow << ", " << index << ", " << clockCycle
            << ", combT0 = " << combT0
            <<",  rvcL1 from header = " << rvcL1
            << ", eventNumberL1 (evt) = " << eventNumberL1
            << ", FW version = " << std::hex << versionFW << std::dec
            << ", slot T0 = " << slotT0[i]
            << ", NHit = " << slotNHit[i]
            << ", Segment = " << slotSegment[i]
            << ", other information = " << otherInformation[i]
            << endl;
            */

            //      TRGTOPSlotTiming* thisSlotTiming(NULL);

            // logL is currently not available (use nHit in lieu of that), so set to 0
            // firstTS info is only available for slots 1 through 8 (channel == 1)
            // -1 for firstTS means no info
            // 1 as the next to the last parameter means "information from the board being read-out"

            if (channel == 1) {

              TRGTOPSlotTiming slotTiming(slotNow,
                                          2 * slotT0[i],
                                          slotSegment[i],
                                          slotNHit[i],
                                          0,
                                          clockCycle,
                                          errorCountWindow,
                                          1,
                                          otherInformation[i]
                                         );
              //        thisSlotTiming = m_TRGTOPSlotTimingArray.appendNew(slotTiming);
              m_TRGTOPSlotTimingArray.appendNew(slotTiming);
            } else {

              TRGTOPSlotTiming slotTiming(slotNow,
                                          2 * slotT0[i],
                                          slotSegment[i],
                                          slotNHit[i],
                                          0,
                                          clockCycle,
                                          errorCountWindow,
                                          1,
                                          -1
                                         );
              //        thisSlotTiming = m_TRGTOPSlotTimingArray.appendNew(slotTiming);
              m_TRGTOPSlotTimingArray.appendNew(slotTiming);
            }

            // slot-level decisions are not currently related to combined decisions
            //      thisSlotTiming->addRelationTo(thisCombinedTiming);
            //      thisCombinedTiming->addRelationTo(thisSlotTiming);

            slotT0Last[i] = slotT0[i];
            slotNHitLast[i] = slotNHit[i];
            slotSegmentLast[i] = slotSegment[i];

          }

          slotNow++;

        }
      } else {

        combinedT0Last = combinedT0;
        for (int i = 0; i < 8; i++) {
          slotT0Last[i] = slotT0[i];
          slotNHitLast[i] = slotNHit[i];
          slotSegmentLast[i] = slotSegment[i];
          otherInformationLast[i] = otherInformation[i];

          combinedT0Last = combinedT0;
          combinedT0RVC2GDLLast = combinedT0RVC2GDL;
        }
      }
    }
  }

}


void TRGTOPUnpackerModule::endRun()
{
}

void TRGTOPUnpackerModule::terminate()
{
}



