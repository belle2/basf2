
//This module
#include <ecl/modules/eclUnpacker/eclUnpackerModule.h>

//STL
#include <iomanip>

//Framework
#include <framework/utilities/FileSystem.h>

//Rawdata
#include <rawdata/dataobjects/RawECL.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <ecl/dataobjects/ECLDsp.h>
#include "ecl/utility/ECLChannelMapper.h"

using namespace std;
using namespace Belle2;
using namespace ECL;

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
  m_eclDigits("", DataStore::c_Event)
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

  m_EvtNum = 0;
}

ECLUnpackerModule::~ECLUnpackerModule()
{

}

void ECLUnpackerModule::initialize()
{

  // require input data
  m_rawEcl.isRequired();

  // register output containers in data store
  m_eclDigits.registerInDataStore(m_eclDigitsName);
  if (m_storeTrigTime) {
    m_eclTrigs.registerInDataStore(m_eclTrigsName);
    m_eclDigits.registerRelationTo(m_eclTrigs);
  }
  m_eclDsps.registerInDataStore(m_eclDspsName);
  m_eclDsps.registerRelationTo(m_eclDigits);

  // make full name of the initialization file
  std::string ini_file_name = FileSystem::findFile(m_eclMapperInitFileName);
  if (! FileSystem::fileExists(ini_file_name)) {
    B2FATAL("ECL Unpacker : eclChannelMapper initialization file " << ini_file_name << " doesn't exist");
  }

  // initialize channel mapper from file (temporary)
  if (! m_eclMapper.initFromFile(ini_file_name.data())) {
    B2FATAL("ECL Unpacker:: Can't initialize eclChannelMapper");
  }

  B2INFO("ECL Unpacker: eclChannelMapper initialized successfully");

  // or initialize it from DB TODO
}

void ECLUnpackerModule::beginRun()
{
  //TODO
  m_tagsReported   = false;
  m_phasesReported = false;
}

void ECLUnpackerModule::event()
{
  // output data
  m_eclDigits.clear();
  m_eclDsps.clear();
  m_eclTrigs.clear();


  int nRawEclEntries = m_rawEcl.getEntries();

  B2DEBUG(50, "Ecl unpacker event called N_RAW = " << nRawEclEntries);

  for (int i = 0; i < nRawEclEntries; i++) {
    for (int n = 0; n < m_rawEcl[i]->GetNumEntries(); n++) {
      readRawECLData(m_rawEcl[ i ], n); // read data from RawECL and put into the m_eclDigits container
    }
  }

  m_EvtNum++;

}

void ECLUnpackerModule::endRun()
{
  //TODO
}

void ECLUnpackerModule::terminate()
{
}

// meathod to read collector data by 32-bit words
unsigned int ECLUnpackerModule::readNextCollectorWord()
{
  if (m_bufPos == m_bufLength) {
    B2DEBUG(50, "Reached the end of the FINESSE buffer");
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


  std::vector <int> eclWaveformSamples;

  int nodeID = rawCOPPERData->GetNodeID(n);


  // loop over FINESSEs in the COPPER
  for (int iFINESSE = 0; iFINESSE < ECL_FINESSES_IN_COPPER; iFINESSE++) {

    ECLTrig* eclTrig = 0;

    m_bitPos = 0;
    m_bufPos = 0;

    // trigger phase of the Collector connected to this FINESSE
    // -1 if there are no triggered shapers
    int triggerPhase0 = -1;
    int triggerTag0   = -1;

    m_bufLength = rawCOPPERData->GetDetectorNwords(n, iFINESSE);

    if (m_bufLength <= 0) continue;

    // get Number of Collector/Crate connected to the FINESSE
    iCrate = m_eclMapper.getCrateID(nodeID, iFINESSE);

    // pointer to data from COPPER/FINESSE
    m_bufPtr = (unsigned int*)rawCOPPERData->GetDetectorBuffer(n, iFINESSE);

    B2DEBUG(15, "***** iEvt " << m_EvtNum << " node " << std::hex << nodeID);

    // dump buffer data
    for (int i = 0; i < m_bufLength; i++) {
      B2DEBUG(500, "" << std::hex << setfill('0') << setw(8) << m_bufPtr[i]);

    }
    B2DEBUG(15, "***** ");


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

      // read the collector header
      value = readNextCollectorWord();
      shapersMask = value & 0xFFF;           // mask of active shapers
      compressMode = (value & 0xF000) >> 12; // compression mode for ADC data, 0 -- disabled, 1 -- enabled

      B2DEBUG(50, "ShapersMask = " << std::hex << shapersMask << " compressMode =  "  <<  compressMode);

      // make new eclTrig oject to store trigger time for crate if there are triggered shapers in the crate
      if (m_storeTrigTime && shapersMask != 0) eclTrig = m_eclTrigs.appendNew();

      // loop over all shapers in crate
      for (iShaper = 1; iShaper <= nShapers; iShaper++) {

        // check if shaper is active
        int thisShaperMask = (1 << (iShaper - 1)) & shapersMask;
        if (thisShaperMask != (1 << (iShaper - 1))) continue;

        // read the shaper header
        value = readNextCollectorWord();
        shaperDataLength = value & 0xFFFF; // amount of words in DATA section (without COLLECTOR HEADER)
        B2DEBUG(50, "iCrate = " << iCrate << " iShaper = " << iShaper);
        B2DEBUG(50, "Shaper HEADER = 0x" << std::hex << value << " dataLength = " << std::dec << shaperDataLength);
        // check shaperDSP header
        if ((value & 0x00FF0000) != 0x00100000) {
          B2ERROR("Ecl Unpacker:: bad shaper header");
          throw Bad_ShaperDSP_header();
        }

        value = readNextCollectorWord();
        nActiveChannelsWithADCData = (value >> 24) & 0x1F;//number of channels with ADC data
        nADCSamplesPerChannel = (value >> 16) & 0x7F;    //ADC samples per channel
        nActiveDSPChannels = (value >> 8) & 0x1F;       //number of active channels in DSP
        triggerPhase = value & 0xFF;                   //trigger phase

        // check that trigger phases for all shapers in the crate are equal
        if (triggerPhase0 == -1) triggerPhase0 = triggerPhase;
        else if (triggerPhase != triggerPhase0 && !m_phasesReported) {
          B2ERROR("Different trigger phases for crate " << iCrate << " :: " << triggerPhase << " != " << triggerPhase0
                  << " ECL data is corrupted for whole run probably") ;
          m_phasesReported = true;
        }

        B2DEBUG(50, "nActiveADCChannels = " << nActiveChannelsWithADCData << " samples " << nADCSamplesPerChannel << " nActiveDSPChannels "
                << nActiveDSPChannels);

        value = readNextCollectorWord();

        dspMask    = (value >> 16) & 0xFFFF;  // Active DSP channels mask
        triggerTag = value & 0xFFFF;          // trigger tag
        B2DEBUG(50, "DSPMASK = 0x" << std::hex << dspMask << " triggerTag " << std::dec << triggerTag);

        if (triggerTag0 == -1) triggerTag0 = triggerTag;
        else if (triggerTag != triggerTag0) {
          if (!m_tagsReported) {
            B2ERROR("Different trigger tags for crate " << iCrate << " :: " << triggerTag << " != " << triggerTag0
                    << " ECL data is corrupted for whole run probably") ;
            m_tagsReported = true;
          }
          triggerTag0 |= (1 << 16);
        }

        value = readNextCollectorWord();
        adcMask = value & 0xFFFF; // mask for channels with ADC data
        adcHighMask = (value >> 16) & 0xFFFF;
        B2DEBUG(50, "ADCMASK = 0x" << std::hex << adcMask << " adcHighMask = 0x" << adcHighMask);

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

          if (cellID < 1) continue; // channel is not connected to crystal

          // fill eclDigits data object
          B2DEBUG(100, "New eclDigit: cid = " << cellID << " amp = " << dspAmplitude << " time = " << dspTime << " qflag = " <<
                  dspQualityFlag);

          // construct eclDigit object and save it in DataStore
          ECLDigit* newEclDigit = m_eclDigits.appendNew();
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
          if (m_storeTrigTime) newEclDigit->addRelationTo(eclTrig);

        }



        if (nRead != nActiveDSPChannels) {
          B2ERROR("Number of active DSP channels and number of read channels don't match (Corrupted data?)" << " nRead = " << nRead <<
                  " nActiveDSP = " << nActiveDSPChannels);
          // do something (throw an exception etc.) TODO
        }


        //read ADC data
        nRead = 0;
        for (ind = 0; ind < ECL_CHANNELS_IN_SHAPER; ind++) {
          //check if there is ADC data for this channel
          if (((1 << ind) & adcMask) != (1 << ind)) continue;
          iChannel = ind + 1;
          adcDataBase = 0;
          adcDataDiffWidth = 0;
          eclWaveformSamples.clear();
          for (indSample = 0; indSample < nADCSamplesPerChannel; indSample++) {
            if (compressMode == 0) value = readNextCollectorWord();
            else {
              if (indSample == 0) {
                value = readNBits(18);
                adcDataBase = value;
                B2DEBUG(200, "adcDataBase = " << adcDataBase);
                value = readNBits(5);
                adcDataDiffWidth = value;
                B2DEBUG(200, "adcDataDiffWidth = " << adcDataDiffWidth);
              }
              value = readNBits(adcDataDiffWidth);
              B2DEBUG(200, "adcDataOffset = " << value);
              value += adcDataBase;
            }
            // fill waveform data for single channel
            eclWaveformSamples.push_back(value);
          }

          // save ADC data to the eclDsp DataStore object if any
          if (eclWaveformSamples.size() > 0) {

            if (eclWaveformSamples.size() != nADCSamplesPerChannel)
              B2ERROR("Wrong number of ADC samples. Actual number of read samples "
                      << eclWaveformSamples.size() << " != number of samples in header "
                      << nADCSamplesPerChannel);

            cellID = m_eclMapper.getCellId(iCrate, iShaper, iChannel);

            if (cellID > 0 || m_storeUnmapped) {
              m_eclDsps.appendNew(cellID, eclWaveformSamples, true);
            }

          }

          nRead++;
        } // read ADC data loop

        if (m_bitPos > 0) {
          m_bufPos++;
          m_bitPos = 0;
        }

        if (nRead != nActiveChannelsWithADCData) {
          B2ERROR("Number of channels with ADC data " << nActiveChannelsWithADCData << " and number of read channels " << nRead <<
                  " don't match (Corrupted data?) ");
          // do something (throw an exception etc.) TODO
        }



      } // loop over shapers

      // fill trigid, trgtime for eclTrig object
      if (eclTrig) {
        eclTrig->setTrigId(iCrate);
        eclTrig->setTimeTrig(triggerPhase0);
        eclTrig->setTrigTag(triggerTag0);
      }





    } // try
    catch (...) {
      // errors while reading data block
      // do something (count errors etc) TODO
      B2ERROR("Corrupted data from ECL collector");
    }

  }// loop ove FINESSes

}
