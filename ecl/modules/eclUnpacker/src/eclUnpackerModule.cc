#include <ecl/modules/eclUnpacker/eclUnpackerModule.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLUnpacker)

ECLUnpackerModule::ECLUnpackerModule() :
  m_eclDigits("", DataStore::c_Event)
{
  setPropertyFlags(c_ParallelProcessingCertified);

  setDescription("The module reads RawECL data from the DataStore and writes the ECLDigit data");
  addParam("InitFileName", m_eclMapperInitFileName, "Initialization file", string(""));
  addParam("ECLDigitsName", m_eclDigitsName, "Name of the ECLDigits container", string("ECLDigits"));

  m_EvtNum = 0;
}

ECLUnpackerModule::~ECLUnpackerModule()
{

}

void ECLUnpackerModule::initialize()
{

  // require input data
  StoreArray<RawECL>::required();

  // register output conteinder in data store
  m_eclDigits.registerInDataStore(m_eclDigitsName);

  // initialize channel mapper from file (temporary)
  m_eclMapper.initFromFile(m_eclMapperInitFileName.c_str());

  // of initialize if from DB TODO

}

void ECLUnpackerModule::beginRun()
{
  //TODO
}

void ECLUnpackerModule::event()
{

  // input data
  StoreArray<RawECL> rawECLData;

  // output data
  m_eclDigits.clear();


  int nRawEclEntries = rawECLData.getEntries();

  B2DEBUG(50, "Ecl unpacker event called N_RAW = " << nRawEclEntries);

  for (int i = 0; i < nRawEclEntries; i++) {
    for (int n = 0; n < rawECLData[i]->GetNumEntries(); n++) {
      readRawECLData(rawECLData[ i ], n); // read data from RawECL and put into the m_eclDigits container
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
int ECLUnpackerModule::readNextCollectorWord()
{
  if (m_bufPos == m_bufLength) {
    B2DEBUG(50, "Reached the end of the FINESSE buffer");
    throw Unexpected_end_of_FINESSE_buffer();
  }
  int value = m_bufPtr[m_bufPos];
  m_bufPos++;
  return value;
}

// read given number of bits from the buffer (in order to read compressed ADC data)
int ECLUnpackerModule::readNBits(int bitsToRead)
{
  int val = 0;

  val = m_bufPtr[m_bufPos] >> bitsToRead;
  if (m_bitPos + bitsToRead > 32)
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

  val &= ((unsigned long long)1 << bitsToRead) - 1;

  return val;
}



void ECLUnpackerModule::readRawECLData(RawCOPPER* rawCOPPERData, int n)
{
  int iCrate, iShaper, iChannel, cellID;

  int shapersMask;
  int adcDataBase, adcDataDiffWidth;
  int compressMode, shaperDataLength;
  int value = 0, nRead = 0, ind = 0, indSample = 0;
  int nActiveChannelsWithADCData, nADCSamplesPerChannel, nActiveDSPChannels;
  // triggerPhase;   /TODO ?
  int dspMask, triggerTag;
  int nShapers;
  int adcMask, adcHighMask, dspTime, dspAmplitude, dspQualityFlag;

  std::vector <int> eclWaveformSamples;

//  unsigned int evnum = rawCOPPERData->GetEveNo(n);
  int nodeID = rawCOPPERData->GetNodeID(n);


  for (int iFINESSE = 0; iFINESSE < ECL_FINESSES_IN_COPPER; iFINESSE++) {

    m_bufLength = rawCOPPERData->GetDetectorNwords(n, iFINESSE);

    if (m_bufLength <= 0) continue;

    iCrate = m_eclMapper.getCrateID(nodeID, iFINESSE);

    // pointer to data from COPPER/FINESSE
    m_bufPtr = rawCOPPERData->GetDetectorBuffer(n, iFINESSE);

    B2DEBUG(15, "***** iEvt " << m_EvtNum << " node " << std::hex << nodeID);
    for (int i = 0; i < m_bufLength; i++) {
      //    B2DEBUG(15,"" << std::hex << setfill('0') << setw(8) << m_bufPtr[i]);

    }
    //  B2DEBUG(15,"***** " );

    m_bufPos = 0; // set read position to 1-st word


    int eclSubSystem = m_eclMapper.getSubSystem(iCrate);
    switch (eclSubSystem) {
      case 0  : nShapers = ECL_BARREL_SHAPERS_IN_CRATE; break;
      case 1  : nShapers = ECL_FWD_SHAPERS_IN_CRATE; break;
      case 2  : nShapers = ECL_BKW_SHAPERS_IN_CRATE; break;
      default:  nShapers = ECL_BARREL_SHAPERS_IN_CRATE;
    }

    try {

      // read the collector header
      value = readNextCollectorWord();
      shapersMask = value & 0xFFF;        // mask of active shapers
      compressMode = (value & 0xF000) >> 12; // compression mode for ADC data, 0 -- disabled, 1 -- enabled

      B2DEBUG(50, "ShapersMask = " << std::hex << shapersMask << " compressMode =  "  <<  compressMode);
      for (iShaper = 1; iShaper <= nShapers; iShaper++) {

        // check if shaper is active
        int thisShaperMask = (1 << (iShaper - 1)) & shapersMask;
        if (thisShaperMask != (1 << (iShaper - 1))) continue;

        value = readNextCollectorWord();
        shaperDataLength = value & 0xFFFF; // amount of words in DATA section (without COLLECTOR HEADER)
        // check shaperDSP header
        B2DEBUG(50, "Shaper HEADER = 0x" << std::hex << value << " dataLength = " << std::dec << shaperDataLength);
        if ((value & 0x00FF0000) != 0x00100000) throw Bad_ShaperDSP_header();


        value = readNextCollectorWord();
        nActiveChannelsWithADCData = (value >> 24) & 0x1F;//number of channels with ADC data
        nADCSamplesPerChannel = (value >> 16) & 0x7F;   //ADC samples per channel
        nActiveDSPChannels = (value >> 8) & 0x1F;      //number of active channels in DSP
//        triggerPhase = value & 0xFF;                  //trigger phase

        B2DEBUG(50, "nActiveADCChannels = " << nActiveChannelsWithADCData << " samples " << nADCSamplesPerChannel << " nActiveDSPChannels "
                << nActiveDSPChannels);

        value = readNextCollectorWord();

        dspMask    = (value >> 16) & 0xFFFF;  // Active DSP channels mask
        triggerTag = value & 0xFFFF;          // trigger tag
        B2DEBUG(50, "DSPMASK = 0x" << std::hex << dspMask << " triggerTag " << std::dec << triggerTag);

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
          dspTime = (value >> 18) & 0xFFF;
          dspQualityFlag = (value >> 30) & 0x3;
          dspAmplitude  = (value & 0x3FFFF) - 128;
          nRead++;

          cellID = m_eclMapper.getCellId(iCrate, iShaper, iChannel);

          if (cellID < 1) continue; // channel is not connected to crystal

          // fill eclDigits data object
          if (dspAmplitude > 50) {
            //B2DEBUG(10, "New eclDigit: cid = " << cellID << " amp = " << dspAmplitude << " time = " << dspTime << " qflag = " <<    dspQualityFlag);
            //B2DEBUG(10,"iCrate = " << iCrate << " iShaper = " << iShaper << " iChannel = " << iChannel);
          }

          ECLDigit* newEclDigit = m_eclDigits.appendNew();
          newEclDigit->setCellId(cellID);
          newEclDigit->setAmp(dspAmplitude);
          newEclDigit->setTimeFit(dspTime);
          newEclDigit->setQuality(dspQualityFlag);
          newEclDigit->setChi(0); // TODO

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
                value = readNBits(5);
                adcDataDiffWidth = value;
              }
              value = readNBits(adcDataDiffWidth);
              value += adcDataBase;
            }
            // fill waveform data for single channel
            eclWaveformSamples.push_back(value);
          }
          nRead++;
          // fill ecl Waveform data object here (TODO)

        }
        if (nRead != nActiveChannelsWithADCData) {
          B2ERROR("Number of channels with ADC data and number of read channels don't match (Corrupted data?)");
          // do something (throw an exception etc.) TODO
        }



      } // loop over shapers



    } // try
    catch (...) {
      // errors while reading data block
      // do something (count errors etc) TODO
      B2ERROR("Corrupted data from ECL collector");
    }
  }
}






