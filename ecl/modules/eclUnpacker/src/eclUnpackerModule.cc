/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Vasily Shebalin, Mikhail Remnev                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//This module
#include <ecl/modules/eclUnpacker/eclUnpackerModule.h>

//STL
#include <iomanip>

//Framework
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationArray.h>

//Rawdata
#include <rawdata/dataobjects/RawECL.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/utility/ECLChannelMapper.h>
#include <ecl/utility/ECLDspUtilities.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

#define B2DEBUG_eclunpacker(level, msg) \
  if (m_debugLevel >= level) {\
    B2DEBUG(level, msg); \
  }

/*

Data format of data packet from shaperDSP (32 bit words)

---------------------------------------------------------------------------------------
Offset              |          MSW(16 bits)         |               LSW(16 bits)      |
--------------------------------------------------------------------------------------|
                    |                               |                                 |
0                   |              0x10             |  4+DSP_NUM+ADC_NUM*SAMPLES_NUM  |
                    |                               |          (Packet length)        |
--------------------------------------------------------------------------------------|
                    |                                                                 |
1                   |     b[28..24] – number of active ADC data channels (ADC_NUM)    |
                    |     b[22..16] – ADC samples per channel (SAMPLES_NUM)           |
                    |     b[12..8]  – number of active DSP channels (DSP_NUM          |
                    |     b[7..0]   – trigger phase                                   |
                    |                                                                 |
--------------------------------------------------------------------------------------|
2                   |     b[31…16] – dsp_mask        |      b[15…0] – trigger tag     |
                    |                                |                                |
--------------------------------------------------------------------------------------|
3                   |                0               |         b[15…0] – adc_mask     |
                    |                                |                                |
--------------------------------------------------------------------------------------|
4…3+DSP_NUM         |     b[31..30] – quality flag                                    |
                    |                 quality = 0 : good fit                          |
                    |                 quality = 1 : internal error of approximation   |
                    |                 quality = 2 : A < A_th,                         |
                    |                            time is replace with chi2            |
                    |                 quality = 3 : bad fit, A > A_thr                |
                    |     b[29..18] – reconstructed time (chi2 if qality = 2)         |
                    |                 chi2 = m<<(p*2), m = b[29:27], p = b[26:18]     |
                    |     b[17..0]  – reconstructed amplitude                         |
                    |                                                                 |
--------------------------------------------------------------------------------------|
4+DSP_NUM…          |                                                                 |
3+DSP_NUM+          |            ADC_NUM*SAMPLES_NUM  -- ADC samples                  |
ADC_NUM*SAMPLES_NUM |                                                                 |
--------------------------------------------------------------------------------------|

*/


REG_MODULE(ECLUnpacker)

ECLUnpackerModule::ECLUnpackerModule() :
  m_globalEvtNum(0),
  m_localEvtNum(0),
  m_bufPtr(0),
  m_bufPos(0),
  m_bufLength(0),
  m_bitPos(0),
  m_storeTrigTime(0),
  m_storeUnmapped(0),
  m_unpackingParams("ECLUnpackingParameters", false),
  m_eclDigits("", DataStore::c_Event),
  m_debugLevel(0)
{
  setDescription("The module reads RawECL data from the DataStore and writes the ECLDigit data");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("InitFileName",  m_eclMapperInitFileName, "Initialization file",             string("/ecl/data/ecl_channels_map.txt"));
  addParam("ECLDigitsName", m_eclDigitsName,         "Name of the ECLDigits container", string("ECLDigits"));
  addParam("ECLDspsName",   m_eclDspsName,           "Name of the ECLDsp container",    string("ECLDsps"));
  addParam("ECLTrigsName",  m_eclTrigsName,          "Name of the ECLTrig container",   string("ECLTrigs"));
  // flag to store trigger times needed for calibration with pulse generator only, so false by default
  addParam("storeTrigTime", m_storeTrigTime,         "Store trigger time",              false);
  addParam("storeUnmapped", m_storeUnmapped,         "Store ECLDsp for channels that don't "
           "exist in ECL mapping", false);
  addParam("useUnpackingParameters", m_useUnpackingParameters,
           "Use ECLUnpackingParameters payload", true);
}

ECLUnpackerModule::~ECLUnpackerModule()
{

}

void ECLUnpackerModule::initialize()
{
  // Get cached debug level to improve performance
  auto& config = LogSystem::Instance().getCurrentLogConfig(PACKAGENAME());
  m_debugLevel = config.getLogLevel() == LogConfig::c_Debug ? config.getDebugLevel() : 0;

  // require input data
  m_rawEcl.isRequired();

  // register output containers in data store
  m_eclDigits.registerInDataStore(m_eclDigitsName);
  if (m_storeTrigTime) {
    m_eclTrigs.registerInDataStore(m_eclTrigsName);
    m_eclDigits.registerRelationTo(m_eclTrigs);
  }
  m_eclDsps.registerInDataStore(m_eclDspsName);
  m_eclDigits.registerRelationTo(m_eclDsps);
  m_eclDsps.registerRelationTo(m_eclDigits);

}

void ECLUnpackerModule::beginRun()
{
  m_evtNumReportedMask    = 0;
  m_tagsReportedMask      = 0;
  m_phasesReportedMask    = 0;
  m_badHeaderReportedMask = 0;
  // Initialize channel mapper at run start to account for possible
  // changes in ECL mapping between runs.
  if (!m_eclMapper.initFromDB()) {
    B2FATAL("ECL Unpacker: Can't initialize eclChannelMapper!");
  }
  if (!m_unpackingParams.isValid() && m_useUnpackingParameters) {
    B2FATAL("ECL Unpacker: Can't access ECLUnpackingParameters payload");
  }
}

void ECLUnpackerModule::event()
{
  // output data
  m_eclDigits.clear();
  m_eclDsps.clear();
  m_eclTrigs.clear();
  // relations arrays
  RelationArray relDigitToTrig(m_eclDigits, m_eclTrigs);
  if (relDigitToTrig) relDigitToTrig.clear();
  RelationArray relDigitToDsp(m_eclDigits, m_eclDsps);
  if (relDigitToDsp) relDigitToDsp.clear();

  if (m_eventMetaData.isValid()) {
    m_globalEvtNum = m_eventMetaData->getEvent();
  } else {
    m_globalEvtNum = -1;
  }

  for (int i = 0; i < ECL_CRATES; i++) {
    m_eclTrigsBuffer[i].setTrigId(0);
  }

  //=== Read raw event data

  int nRawEclEntries = m_rawEcl.getEntries();

  B2DEBUG_eclunpacker(22, "Ecl unpacker event called N_RAW = " << nRawEclEntries);

  for (int i = 0; i < nRawEclEntries; i++) {
    for (int n = 0; n < m_rawEcl[i]->GetNumEntries(); n++) {
      readRawECLData(m_rawEcl[ i ], n); // read data from RawECL and put into the m_eclDigits container
    }
  }

  //=== Add created ECLTrig objects to the StoreArray

  ECLTrig* new_ecl_trigs[ECL_CRATES] = {};
  for (int i = 0; i < ECL_CRATES; i++) {
    if (m_eclTrigsBuffer[i].getTrigId() > 0) {
      new_ecl_trigs[i] = m_eclTrigs.appendNew(m_eclTrigsBuffer[i]);
    }
  }

  for (int i = 0; i < m_eclDigits.getEntries(); i++) {
    int cid = m_eclDigits[i]->getCellId();
    int crate0 = m_eclMapper.getCrateID(cid) - 1;
    if (new_ecl_trigs[crate0]) {
      relDigitToTrig.add(i, crate0);
    }
  }

  m_localEvtNum++;

}

void ECLUnpackerModule::endRun()
{
}

void ECLUnpackerModule::terminate()
{
}

// meathod to read collector data by 32-bit words
unsigned int ECLUnpackerModule::readNextCollectorWord()
{
  if (m_bufPos == m_bufLength) {
    B2DEBUG_eclunpacker(22, "Reached the end of the FINESSE buffer");
    throw Unexpected_end_of_FINESSE_buffer();
  }
  unsigned int value = m_bufPtr[m_bufPos];
  m_bufPos++;
  return value;
}

// read given number of bits from the buffer (in order to read compressed ADC data)
unsigned int ECLUnpackerModule::readNBits(int bitsToRead)
{
  unsigned int val = 0;

  val = m_bufPtr[m_bufPos] >> m_bitPos;
  if (m_bitPos + bitsToRead > 31)
    if (m_bufPos == m_bufLength) {
      B2ERROR("Reached the end of the FINESSE buffer while read compressed ADC data");

      throw Unexpected_end_of_FINESSE_buffer();
    } else {
      m_bufPos++;
      val += m_bufPtr[m_bufPos] << (32 - m_bitPos);
      m_bitPos += bitsToRead;
      m_bitPos -= 32;
    }
  else {
    m_bitPos += bitsToRead;
    if (m_bitPos == 32) {
      m_bufPos++;
      m_bitPos -= 32;
    }
  }

  val &= (1 << bitsToRead) - 1;

  return val;
}

void ECLUnpackerModule::readRawECLData(RawECL* rawCOPPERData, int n)
{
  RelationArray relDigitToDsp(m_eclDigits, m_eclDsps);
  int iCrate, iShaper, iChannel, cellID;

  int shapersMask;
  int adcDataBase, adcDataDiffWidth;
  int compressMode, shaperDataLength;
  unsigned int value = 0;
  unsigned int nRead = 0, ind = 0, indSample = 0;
  unsigned int nActiveChannelsWithADCData, nADCSamplesPerChannel, nActiveDSPChannels;
  int triggerPhase;
  int dspMask = 0, triggerTag = 0;
  int nShapers;
  int adcMask, adcHighMask, dspTime, dspAmplitude, dspQualityFlag;
  // Mask of shapers that discarded waveform data due to beam burst suppression
  int burstSuppressionMask;


  std::vector <int> eclWaveformSamples;

  int nodeID = rawCOPPERData->GetNodeID(n);


  // loop over FINESSEs in the COPPER
  for (int iFINESSE = 0; iFINESSE < ECL_FINESSES_IN_COPPER; iFINESSE++) {

    m_bitPos = 0;
    m_bufPos = 0;

    m_bufLength = rawCOPPERData->GetDetectorNwords(n, iFINESSE);

    if (m_bufLength <= 0) continue;

    // get Number of Collector/Crate connected to the FINESSE
    iCrate = m_eclMapper.getCrateID(nodeID, iFINESSE);

    // pointer to data from COPPER/FINESSE
    m_bufPtr = (unsigned int*)rawCOPPERData->GetDetectorBuffer(n, iFINESSE);

    B2DEBUG_eclunpacker(21, "***** iEvt " << m_localEvtNum << " node " << std::hex << nodeID);

    // dump buffer data
    for (int i = 0; i < m_bufLength; i++) {
      B2DEBUG_eclunpacker(29, "" << std::hex << setfill('0') << setw(8) << m_bufPtr[i]);
    }
    B2DEBUG_eclunpacker(21, "***** ");


    m_bufPos = 0; // set read position to the 1-st word

    // get number of shapers depending on the subsystem this crate belongs to(barrel/forward/backward)
    int eclSubSystem = m_eclMapper.getSubSystem(iCrate);
    switch (eclSubSystem) {
      case 0  : nShapers = ECL_BARREL_SHAPERS_IN_CRATE; break;
      case 1  : nShapers = ECL_FWD_SHAPERS_IN_CRATE; break;
      case 2  : nShapers = ECL_BKW_SHAPERS_IN_CRATE; break;
      default : nShapers = ECL_BARREL_SHAPERS_IN_CRATE;
    }

    try {
      burstSuppressionMask = 0;

      // trigger phase of the Collector connected to this FINESSE
      // -1 if there are no triggered shapers
      int triggerPhase0 = -1;
      int triggerTag0   = -1;

      // read the collector header
      value = readNextCollectorWord();
      shapersMask = value & 0xFFF;           // mask of active shapers
      compressMode = (value & 0xF000) >> 12; // compression mode for ADC data, 0 -- disabled, 1 -- enabled

      B2DEBUG_eclunpacker(22, "ShapersMask = " << std::hex << shapersMask << " compressMode =  "  <<  compressMode);

      // loop over all shapers in crate
      for (iShaper = 1; iShaper <= nShapers; iShaper++) {

        // check if shaper is active
        int thisShaperMask = (1 << (iShaper - 1)) & shapersMask;
        if (thisShaperMask != (1 << (iShaper - 1))) continue;

        // read the shaper header
        value = readNextCollectorWord();
        shaperDataLength = value & 0xFFFF; // amount of words in DATA section (without COLLECTOR HEADER)
        B2DEBUG_eclunpacker(22, "iCrate = " << iCrate << " iShaper = " << iShaper);
        B2DEBUG_eclunpacker(22, "Shaper HEADER = 0x" << std::hex << value << " dataLength = " << std::dec << shaperDataLength);
        // check shaperDSP header
        if ((value & 0x00FF0000) != 0x00100000) {
          doBadHeaderReport(iCrate);
          throw Bad_ShaperDSP_header();
        }

        value = readNextCollectorWord();
        burstSuppressionMask |= ((value >> 29) & 1) << (iShaper - 1); // burst suppression bit
        nActiveChannelsWithADCData = (value >> 24) & 0x1F;//number of channels with ADC data
        nADCSamplesPerChannel = (value >> 16) & 0x7F;    //ADC samples per channel
        nActiveDSPChannels = (value >> 8) & 0x1F;       //number of active channels in DSP
        triggerPhase = value & 0xFF;                   //trigger phase

        // check that trigger phases for all shapers in the crate are equal
        if (triggerPhase0 == -1) triggerPhase0 = triggerPhase;
        else if (triggerPhase != triggerPhase0) {
          doPhasesReport(iCrate, triggerPhase0, triggerPhase);
        }

        B2DEBUG_eclunpacker(22, "nActiveADCChannels = " << nActiveChannelsWithADCData << " samples " << nADCSamplesPerChannel <<
                            " nActiveDSPChannels "
                            << nActiveDSPChannels);

        value = readNextCollectorWord();

        dspMask    = (value >> 16) & 0xFFFF;  // Active DSP channels mask
        triggerTag = value & 0xFFFF;          // trigger tag
        B2DEBUG_eclunpacker(22, "DSPMASK = 0x" << std::hex << dspMask << " triggerTag " << std::dec << triggerTag);

        if (triggerTag0 == -1) triggerTag0 = triggerTag;
        else if (triggerTag != triggerTag0) {
          doTagsReport(iCrate, triggerTag0, triggerTag);
          triggerTag0 |= (1 << 16);
        }

        if (m_globalEvtNum >= 0) {
          if (triggerTag != (m_globalEvtNum & 0xFFFF)) {
            doEvtNumReport(iCrate, triggerTag, m_globalEvtNum);
          }
        }

        value = readNextCollectorWord();
        adcMask = value & 0xFFFF; // mask for channels with ADC data
        adcHighMask = (value >> 16) & 0xFFFF;
        B2DEBUG_eclunpacker(22, "ADCMASK = 0x" << std::hex << adcMask << " adcHighMask = 0x" << adcHighMask);

        ECLDigit* newEclDigits[ECL_CHANNELS_IN_SHAPER] = {};
        int newEclDigitsIdx[ECL_CHANNELS_IN_SHAPER] = {};

        nRead = 0;
        // read DSP data (quality, fitted time, amplitude)
        for (ind = 0; ind < ECL_CHANNELS_IN_SHAPER; ind++) {
          // check if DSP channel is active
          if (((1 << ind) & dspMask) != (1 << ind)) continue;
          iChannel = ind + 1;
          value = readNextCollectorWord();
          dspTime = (int)(value << 2) >> 20;
          dspQualityFlag = (value >> 30) & 0x3;
          dspAmplitude  = (value & 0x3FFFF) - 128;
          nRead++;

          cellID = m_eclMapper.getCellId(iCrate, iShaper, iChannel);

          if (!isDSPValid(cellID, iCrate, iShaper, iChannel, dspAmplitude, dspTime, dspQualityFlag)) continue;

          // fill eclDigits data object
          B2DEBUG_eclunpacker(23, "New eclDigit: cid = " << cellID << " amp = " << dspAmplitude << " time = " << dspTime << " qflag = " <<
                              dspQualityFlag);

          // construct eclDigit object and save it in DataStore
          ECLDigit* newEclDigit = m_eclDigits.appendNew();
          newEclDigitsIdx[ind]  = m_eclDigits.getEntries() - 1;
          newEclDigits[ind]     = newEclDigit;
          newEclDigit->setCellId(cellID);
          newEclDigit->setAmp(dspAmplitude);
          newEclDigit->setQuality(dspQualityFlag);
          if (dspQualityFlag == 2) {
            // amplitude is lower than threshold value time = trg_time => fit_time = 0
            newEclDigit->setTimeFit(0);
            // the time data is replaced with chi2 data
            const int chi_mantissa = dspTime & 0x1FF;
            const int chi_exponent = (dspTime >> 9) & 7;
            const int chi2  = chi_mantissa << (chi_exponent * 2);
            newEclDigit->setChi(chi2);

          } else {
            // otherwise we do not have chi2 information
            newEclDigit->setTimeFit(dspTime);
            newEclDigit->setChi(0);
          }

        }



        if (nRead != nActiveDSPChannels) {
          B2ERROR("Number of active DSP channels and number of read channels don't match (Corrupted data?)"
                  << LogVar("nRead", nRead) << LogVar("nActiveDSP", nActiveDSPChannels));
          // do something (throw an exception etc.) TODO
        }

        //read ADC data
        eclWaveformSamples.resize(nADCSamplesPerChannel);
        nRead = 0;
        for (ind = 0; ind < ECL_CHANNELS_IN_SHAPER; ind++) {
          //check if there is ADC data for this channel
          if (((1 << ind) & adcMask) != (1 << ind)) continue;
          iChannel = ind + 1;
          adcDataBase = 0;
          adcDataDiffWidth = 0;
          for (indSample = 0; indSample < nADCSamplesPerChannel; indSample++) {
            if (compressMode == 0) value = readNextCollectorWord();
            else {
              if (indSample == 0) {
                value = readNBits(18);
                adcDataBase = value;
                B2DEBUG_eclunpacker(24, "adcDataBase = " << adcDataBase);
                value = readNBits(5);
                adcDataDiffWidth = value;
                B2DEBUG_eclunpacker(24, "adcDataDiffWidth = " << adcDataDiffWidth);
              }
              value = readNBits(adcDataDiffWidth);
              B2DEBUG_eclunpacker(24, "adcDataOffset = " << value);
              value += adcDataBase;
            }
            // fill waveform data for single channel
            eclWaveformSamples[indSample] = value;
          }

          // save ADC data to the eclDsp DataStore object if any
          if (nADCSamplesPerChannel > 0) {

            cellID = m_eclMapper.getCellId(iCrate, iShaper, iChannel);

            if (cellID > 0 || m_storeUnmapped) {
              ECLDsp* newEclDsp = m_eclDsps.appendNew(cellID, eclWaveformSamples);

              if (m_useUnpackingParameters) {
                // Check run-dependent unpacking parameters
                auto params = m_unpackingParams->get(iCrate, iShaper, iChannel);
                if (params & ECL_OFFLINE_ADC_FIT) {
                  auto result = ECLDspUtilities::shapeFitter(cellID, eclWaveformSamples, triggerPhase0);
                  if (result.quality == 2) result.time = 0;

                  if (!newEclDigits[ind]) {
                    ECLDigit* newEclDigit = m_eclDigits.appendNew();
                    newEclDigits[ind] = newEclDigit;
                    newEclDigit->setCellId(cellID);
                    newEclDigit->setAmp(result.amp);
                    newEclDigit->setTimeFit(result.time);
                    newEclDigit->setQuality(result.quality);
                    newEclDigit->setChi(result.chi2);
                  }
                }
              }
              // Add relation from ECLDigit to ECLDsp
              if (newEclDigits[ind]) {
                int eclDspIdx = m_eclDsps.getEntries() - 1;
                relDigitToDsp.add(newEclDigitsIdx[ind], eclDspIdx);
                // newEclDigits[ind]->addRelationTo(newEclDsp);
              }
            }

          }

          nRead++;
        } // read ADC data loop

        if (m_bitPos > 0) {
          m_bufPos++;
          m_bitPos = 0;
        }

        if (nRead != nActiveChannelsWithADCData) {
          B2ERROR("Number of channels with ADC data and "
                  "number of read channels don't match (Corrupted data?)"
                  << LogVar("active channels", nActiveChannelsWithADCData)
                  << LogVar("read channels", nRead));
          // do something (throw an exception etc.) TODO
        }



      } // loop over shapers

      // make new eclTrig oject to store trigger time for crate if there are triggered shapers in the crate
      if (m_storeTrigTime && shapersMask != 0) {
        int trigId = iCrate & 0x3F;
        ECLTrig* eclTrig = &m_eclTrigsBuffer[trigId - 1];
        // fill trigid, trgtime for eclTrig object
        trigId |= (burstSuppressionMask & 0xFFF) << 6;
        eclTrig->setTrigId(trigId);
        eclTrig->setTimeTrig(triggerPhase0);
        eclTrig->setTrigTag(triggerTag0);
      }

    } // try
    catch (...) {
      // errors while reading data block
      // do something (count errors etc) TODO
      B2ERROR("Corrupted data from ECL collector");
    }

  }// loop over FINESSEs

}

bool ECLUnpackerModule::isDSPValid(int cellID, int crate, int shaper, int channel, int, int, int quality)
{
  // Channel is not connected to crystal
  if (cellID < 1) return false;

  if (m_useUnpackingParameters) {
    // Check if data for this channel should be discarded in current run.
    auto params = m_unpackingParams->get(crate, shaper, channel);
    if (params & ECL_DISCARD_DSP_DATA) {
      if (params & ECL_KEEP_GOOD_DSP_DATA) {
        if (quality == 0) return true;
      }
      return false;
    }
  }

  return true;
}

void ECLUnpackerModule::doEvtNumReport(unsigned int iCrate, int tag, int evt_number)
{
  if (!evtNumReported(iCrate)) {
    B2ERROR("ECL trigger tag is inconsistent with event number."
            << LogVar("crate", iCrate)
            << LogVar("trigger tag", tag)
            << LogVar("event number", evt_number));
    m_evtNumReportedMask |= 1L << (iCrate - 1);
  }
}
void ECLUnpackerModule::doTagsReport(unsigned int iCrate, int tag0, int tag1)
{
  if (!tagsReported(iCrate)) {
    B2ERROR("Different trigger tags. ECL data is corrupted for whole run probably."
            << LogVar("crate", iCrate)
            << LogVar("trigger tag0", tag0) << LogVar("trigger tag1", tag1));
    m_tagsReportedMask |= 1L << (iCrate - 1);
  }
}
void ECLUnpackerModule::doPhasesReport(unsigned int iCrate, int phase0, int phase1)
{
  if (!phasesReported(iCrate)) {
    B2ERROR("Different trigger phases. ECL data is corrupted for whole run probably."
            << LogVar("crate", iCrate)
            << LogVar("trigger phase0", phase0) << LogVar("trigger phase1", phase1));
    m_phasesReportedMask |= 1L << (iCrate - 1);
  }
}
void ECLUnpackerModule::doBadHeaderReport(unsigned int iCrate)
{
  if (!badHeaderReported(iCrate)) {
    B2ERROR("Bad shaper header."
            << LogVar("crate", iCrate));
    m_badHeaderReportedMask |= 1L << (iCrate - 1);
  }
}

