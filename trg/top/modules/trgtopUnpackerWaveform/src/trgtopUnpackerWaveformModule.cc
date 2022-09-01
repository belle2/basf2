/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/top/modules/trgtopUnpackerWaveform/trgtopUnpackerWaveformModule.h>

/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
* ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>

#include <iostream>

//using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TRGTOPUnpackerWaveform);

std::string TRGTOPUnpackerWaveformModule::version() const
{
  return std::string("1.00");
}

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TRGTOPUnpackerWaveformModule::TRGTOPUnpackerWaveformModule()
  : Module::Module(), m_eventNumber(0), m_trigType(0), m_nodeId(0), m_nWords(0), m_reportedAlreadyRun_1(false),
    m_reportedAlreadyRun_2(false)
{
  // Set module properties


  std::string desc = "TRGTOPUnpackerWaveformModule(" + version() + ")" + "Unpacks TOP TRG waveforms";
  setDescription(desc);
  setPropertyFlags(c_ParallelProcessingCertified);

  B2DEBUG(20, "TRGTOPUnpackerWaveform: Constructor done.");

  // Parameter definitions

  addParam("outputTimeStampsSlotsName", m_outputWaveFormTimeStampsSlotsName,
           "name of TRGTOPWaveFormTimeStampsSlot store array", std::string(""));

  addParam("outputTimeStampsName", m_outputWaveFormTimeStampsName,
           "name of TRGTOPWaveFormTimeStamp store array", std::string(""));

  addParam("overrideControlBits", m_overrideControlBits,
           "Override control bits in data",
           true);
  //           false);

}

TRGTOPUnpackerWaveformModule::~TRGTOPUnpackerWaveformModule()
{
}

void TRGTOPUnpackerWaveformModule::initialize()
{

  //  m_TRGTOPWaveFormTimeStampsSlots.isRequired(m_outputTimeStampsSlotsName);
  //  m_TRGTOPWaveFormTimeStamps.isRequired(m_outputTimeStampsName);

  m_TRGTOPWaveFormTimeStampsSlots.registerInDataStore(m_outputWaveFormTimeStampsSlotsName);
  m_TRGTOPWaveFormTimeStamps.registerInDataStore(m_outputWaveFormTimeStampsName);

  m_TRGTOPWaveFormTimeStampsSlots.registerRelationTo(m_TRGTOPWaveFormTimeStamps);
  m_TRGTOPWaveFormTimeStamps.registerRelationTo(m_TRGTOPWaveFormTimeStampsSlots);
}


void TRGTOPUnpackerWaveformModule::beginRun()
{
  m_reportedAlreadyRun_1 = false;
  m_reportedAlreadyRun_2 = false;
}

void TRGTOPUnpackerWaveformModule::event()
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

void TRGTOPUnpackerWaveformModule::readDAQEvent(RawTRG* raw_daq, int j, int channel)
{
  //  if (raw_daq->GetDetectorNwords(j, channel) > 3) {                ///general header is 3 words long
  if (raw_daq->GetDetectorNwords(j, channel) > 0) {
    unpackWaveforms(raw_daq->GetDetectorBuffer(j, channel), channel);
  }
}


void TRGTOPUnpackerWaveformModule::unpackWaveforms(int* rdat, int channel)
{

  //  B2INFO("channel, data size (32bit words) = " << channel << ", " << m_nWords);

  // Information for each data window

  // rvc from the header of the buffer
  //  int l1_revo = rdat[2] & 0x7ff;  /// L1 timestamp (11 bits)

  // L1 event number
  //  int trgtag = (rdat[2] >> 12) & 0xfffff; /// 20 LSBs of trgtag (sequential trigger number)

  //  B2INFO("l1_rvc from header = " << l1_revo);
  //  B2INFO("trgtag (evt) from buffer header = " << trgtag);

  bool dataFormatKnown = false;

  int windowSize = -1;
  int numberOfWindows = -1;

  //  int dataFormatVersionExpected = -1;
  //  int revoClockDeltaExpected = 4;
  //  int cntr127DeltaExpected = 4;

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
    //    dataFormatVersionExpected = 0;
  } else if (m_nWords == 771) {
    windowSize = 32;
    numberOfWindows = 24;
    dataFormatKnown = true;
    //    dataFormatVersionExpected = 2;
  } else if (m_nWords == 1875) {
    windowSize = 39;
    numberOfWindows = 48;
    dataFormatKnown = true;
    //    dataFormatVersionExpected = 1;
  } else if (m_nWords == 1539) {
    windowSize = 32;
    numberOfWindows = 48;
    dataFormatKnown = true;
    //    dataFormatVersionExpected = 4;
  } else if (m_nWords == 3075) {
    windowSize = 32;
    numberOfWindows = 96;
    dataFormatKnown = true;
    //    dataFormatVersionExpected = 4;
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

  //  int revoClockLast = -1;
  //  int cntr127Last = -1;

  // error counter for possible data corruption
  //  unsigned int errorCountEvent = 0;

  // need to know when a new decision is made (there could be more than one TOP L1 timing decision stored in the same B2L buffer)
  //  int t0CombinedDecisionLast = -1;
  //  int logLSumLast = -1;
  //  int logLSumNow = 0;

  bool performBufferAnalysis = true;
  bool reportAllErrors = true;
  //  bool reportSummaryErrors = true;

  // check if this event's buffer is a dummy buffer
  int counterDummyWindows = 0;
  unsigned int testPatternDummyEvent = 0xbbbb;
  for (int iWindow = 0; iWindow < numberOfWindows; iWindow++) {
    int index = iWindow * windowSize + 3;
    // various test patterns will be used to check the data
    unsigned int testPattern = (rdat[index] >> 16) & 0xffff;
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
      //      if (testPattern & 0x4) reportSummaryErrors = false;
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
  //  int clocksRvcJumpFirst = -1;
  //  int clocksCntr127JumpFirst = -1;

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
          // -1276 is simply going to the next cycle of rvc counting
          if (revoClockDeltaNow != -1276) {
            if (reportAllErrors) B2INFO("rvc changed by an unexpected number of units: " << revoClockDeltaNow << ", last rvc = " <<
                                           revoClockLast <<
                                           ", current rvc = " << revoClockNow << ", window " << iWindow << ", index = " << index);
            numberRvcJumps++;
            if (windowRvcJumpFirst < 0) {
              windowRvcJumpFirst = iWindow;
        //              clocksRvcJumpFirst = revoClockDeltaNow;
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
            if (reportAllErrors) B2INFO("cntr127 changed by an unexpected number of units: " << cntr127DeltaNow << ", cntr127 last = " <<
                                           cntr127Last <<
                                           ", cntr127 now = " << cntr127Now << ", window " << iWindow << ", index = " << index + 1);
            numberCntr127Jumps++;
            if (windowCntr127JumpFirst < 0) {
              windowCntr127JumpFirst = iWindow;
        //              clocksCntr127JumpFirst = cntr127DeltaNow;
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
    B2INFO("l1_rvc from header = " << l1_revo);
    B2INFO("trgtag (evt) from buffer header = " << trgtag);
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

  // events with no buffer (i.e. no payload), empty (i.e. dummy) windows and presumably corrupted events are NOT analyzed
  if (performBufferAnalysis) {

    for (int iWindow = 0; iWindow < numberOfWindows; iWindow++) {
      int index = iWindow * windowSize + 3;
      int dataFormatVersionNow = (rdat[index] >> 11) & 0x1f;
      if (dataFormatVersionNow < 4) return;
    }

    // clear TimeStamps - this should be done elsewhere automatically
    //  m_TRGTOPWaveFormTimeStampsSlots.clear();
    //  m_TRGTOPWaveFormTimeStamps.clear();

    // store waveforms in event store

    for (int i = 0; i < 8; i++) {

      int slot = i + 1;
      if (channel == 0) slot = slot + 8;

      auto* timeStampsSlotStore = m_TRGTOPWaveFormTimeStampsSlots.appendNew(slot, 4 * numberOfWindows);

      int nActualTimeStamps = 0;

      int firstActualTimeStampValue = -1;
      int firstActualTimeStampClockCycle = -1;

      // Loop over windows in B2L buffer and retrieve the waveforms
      for (int iWindow = 0; iWindow < numberOfWindows; iWindow++) {

        int clockCycle = iWindow * 4;

        // a pointer-like variable for accessing the data in the buffer sequentially
        int index = iWindow * windowSize + 3;

        // data start at rdat[index+16]

        int shift_data = i * 2;

        int value1 = (rdat[index + 16 + shift_data] >> 16) & 0xffff;
        int value2 = (rdat[index + 16 + shift_data]) & 0xffff;
        int value3 = (rdat[index + 17 + shift_data] >> 16) & 0xffff;
        int value4 = (rdat[index + 17 + shift_data]) & 0xffff;

        TRGTOPWaveFormTimeStamp timeStamp1(value1, slot);
        TRGTOPWaveFormTimeStamp timeStamp2(value2, slot);
        TRGTOPWaveFormTimeStamp timeStamp3(value3, slot);
        TRGTOPWaveFormTimeStamp timeStamp4(value4, slot);

        auto* timeStampStore = m_TRGTOPWaveFormTimeStamps.appendNew(timeStamp1);
        timeStampsSlotStore->addRelationTo(timeStampStore);
        timeStampStore->addRelationTo(timeStampsSlotStore);

        timeStampStore = m_TRGTOPWaveFormTimeStamps.appendNew(timeStamp2);
        timeStampsSlotStore->addRelationTo(timeStampStore);
        timeStampStore->addRelationTo(timeStampsSlotStore);

        timeStampStore = m_TRGTOPWaveFormTimeStamps.appendNew(timeStamp3);
        timeStampsSlotStore->addRelationTo(timeStampStore);
        timeStampStore->addRelationTo(timeStampsSlotStore);

        timeStampStore = m_TRGTOPWaveFormTimeStamps.appendNew(timeStamp4);
        timeStampsSlotStore->addRelationTo(timeStampStore);
        timeStampStore->addRelationTo(timeStampsSlotStore);

        if (!timeStamp1.isEmptyClockCycle()) {
          nActualTimeStamps++;
          if (nActualTimeStamps == 1) {
            firstActualTimeStampValue = value1;
            firstActualTimeStampClockCycle = clockCycle;
          }
        }
        clockCycle++;

        if (!timeStamp2.isEmptyClockCycle()) {
          nActualTimeStamps++;
          if (nActualTimeStamps == 1) {
            firstActualTimeStampValue = value2;
            firstActualTimeStampClockCycle = clockCycle;
          }
        }
        clockCycle++;

        if (!timeStamp3.isEmptyClockCycle()) {
          nActualTimeStamps++;
          if (nActualTimeStamps == 1) {
            firstActualTimeStampValue = value3;
            firstActualTimeStampClockCycle = clockCycle;
          }
        }
        clockCycle++;

        if (!timeStamp4.isEmptyClockCycle()) {
          nActualTimeStamps++;
          if (nActualTimeStamps == 1) {
            firstActualTimeStampValue = value4;
            firstActualTimeStampClockCycle = clockCycle;
          }
        }
        clockCycle++;

      }

      timeStampsSlotStore->setNumberOfActualTimeStamps(nActualTimeStamps);
      timeStampsSlotStore->setFirstActualTimeStampValue(firstActualTimeStampValue);
      timeStampsSlotStore->setFirstActualTimeStampClockCycle(firstActualTimeStampClockCycle);

    }
  }

}


void TRGTOPUnpackerWaveformModule::endRun()
{
}

void TRGTOPUnpackerWaveformModule::terminate()
{
}



