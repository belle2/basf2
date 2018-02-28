#include <ecl/modules/eclPacker/eclPackerModule.h>

#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>

#include <framework/utilities/FileSystem.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLPacker)

ECLPackerModule::ECLPackerModule() :
  m_compressMode(false),
  m_eclRawCOPPERs("", DataStore::c_Event)
{
  setDescription("The module reads ECLDigits from the DataStore and writes ECLRaw data.");
  addParam("InitFileName", m_eclMapperInitFileName, "Initialization file", string("/ecl/data/ecl_channels_map.txt"));
  addParam("RawCOPPERsName", m_eclRawCOPPERsName, "Name of the RawECL container", string("RawECLs"));
  addParam("CompressMode", m_compressMode, "compress mode for ADC samples", true);
  addParam("AmpThreshold", m_ampThreshold, "Amplitude threshold", -50);
  addParam("PackWfRareFactor", m_WaveformRareFactor, "Pack ADC samples for one of N events. No waveform is packed if 0", 100);

  m_EvtNum = 0;

  iEclDigIndices = new int[ECL_TOTAL_CHANNELS];
  iEclWfIndices  = new int[ECL_TOTAL_CHANNELS];

  m_eclMapper = new ECLChannelMapper();
}

ECLPackerModule::~ECLPackerModule()
{
  delete m_eclMapper;
  delete[] iEclDigIndices;
  delete[] iEclWfIndices;
}

void ECLPackerModule::initialize()
{
  // require input data
  m_eclDigits.isRequired();
  m_eclDsps.isOptional();

  // register output container in data store
  m_eclRawCOPPERs.registerInDataStore(m_eclRawCOPPERsName);

  // initialize channel mapper from file (temporary)
  std::string ini_file_name = FileSystem::findFile(m_eclMapperInitFileName);
  if (! FileSystem::fileExists(ini_file_name)) {
    B2FATAL("eclChannelMapper initialization file " << ini_file_name << " doesn't exist");
  }

  if (! m_eclMapper->initFromFile(ini_file_name.data())) {
    B2FATAL("Can't initialize eclChannelMapper!");
  }

  // of initialize if from DB TODO
  B2INFO("ECL Packer: eclChannelMapper initialized successfully");
  B2INFO("ECL Packer: Compress mode = " << m_compressMode);
}

void ECLPackerModule::beginRun()
{
  //TODO
}

void ECLPackerModule::event()
{

  B2DEBUG(50, "EclPacker:: event called ");
  // output data
  m_eclRawCOPPERs.clear();

  B2DEBUG(50, "EclPacker:: output data arrays created");

  int nActiveChannelsWithADCData, nActiveDSPChannels;
  int triggerPhase = 0, dspMask = 0;

  // get total number of hits
  int nEclDigits   = m_eclDigits.getEntries();
  int nEclWaveform = m_eclDsps.getEntries();

  for (int i = 0; i < ECL_CRATES; i++) {
    collectorMaskArray[i] = 0;
    for (int j = 0; j < ECL_BARREL_SHAPERS_IN_CRATE; j++) {
      shaperMaskArray[i][j]    = 0;
      shaperNHits[i][j]        = 0;
      shaperADCMaskArray[i][j] = 0;
      shaperNWaveform[i][j]    = 0;
    }
  }

  for (int j = 0; j < ECL_TOTAL_CHANNELS; j++) {
    iEclDigIndices[j] = -1;
    iEclWfIndices[j] = -1;
  }

  B2DEBUG(100, "EclPacker:: N_Digits    = " << nEclDigits);
  B2DEBUG(100, "EclPacker:: N_Waveforms = " << nEclWaveform);

  int iCOPPER, iFINESSE, iCrate, iShaper, iChannel, nShapers;

  B2DEBUG(100, "EclPacker:: Hits ======>> ");
  int tot_dsp_hits = 0;
  // fill number of hits, masks and fill correspondance between cellID and index in container
  for (int i_digit = 0; i_digit < nEclDigits; i_digit++) {
    int cid = m_eclDigits[i_digit]->getCellId();
    int amp = m_eclDigits[i_digit]->getAmp();

    if (amp < m_ampThreshold) continue;

    //TODO: Threshold
    iCrate = m_eclMapper->getCrateID(cid);
    iShaper = m_eclMapper->getShaperPosition(cid);
    iChannel = m_eclMapper->getShaperChannel(cid);

    if (iCrate < 1 && iShaper < 1 && iChannel < 1) {
      B2ERROR("Wrong crate/shaper/channel ids: " << iCrate << " " << iShaper << " " << iChannel << " for CID " << cid);
      throw eclPacker_internal_error();
    }
    collectorMaskArray[iCrate - 1] |= (1 << (iShaper - 1));

    shaperMaskArray[iCrate - 1][iShaper - 1] |= (1 << (iChannel - 1));
    shaperNHits[iCrate - 1][iShaper - 1]++;

    iEclDigIndices[cid - 1] = i_digit;
    tot_dsp_hits++;
  }

  B2DEBUG(100, "ECL Packer:: N Hits above threshold  = " << tot_dsp_hits << " nWaveforms = " << nEclWaveform);

  if (m_WaveformRareFactor != 0)
    if (m_EvtNum % m_WaveformRareFactor == 0) {
      B2DEBUG(100, "ECL Packer:: Pack waveform data for this event: " << m_EvtNum);
      for (int i_wf = 0; i_wf < nEclWaveform; i_wf++) {
        int cid = m_eclDsps[i_wf]->getCellId();
        iCrate = m_eclMapper->getCrateID(cid);
        iShaper = m_eclMapper->getShaperPosition(cid);
        iChannel = m_eclMapper->getShaperChannel(cid);

        //check corresponding amplitude in ecl digits
        int amp = 0;
        for (int i_digit = 0; i_digit < nEclDigits; i_digit++) {
          if (m_eclDigits[i_digit]->getCellId() == cid) {
            amp = m_eclDigits[i_digit]->getAmp();
            break;
          }
        }
        if (amp < m_ampThreshold) continue;

        shaperADCMaskArray[iCrate - 1][iShaper - 1] |= (1 << (iChannel - 1));
        shaperNWaveform[iCrate - 1][iShaper - 1]++;

        iEclWfIndices[cid - 1] = i_wf;
      }
    }

  // fill rawCOPPERPacker data
  RawCOPPERPackerInfo rawcprpacker_info;
  rawcprpacker_info.exp_num = 0;
  rawcprpacker_info.run_subrun_num = 1; // run number : 14bits, subrun # : 8bits
  rawcprpacker_info.eve_num = m_EvtNum;
  rawcprpacker_info.tt_ctime = 0x7123456;  //??? (copy-past from CDC)
  rawcprpacker_info.tt_utime = 0xF1234567; //???
  rawcprpacker_info.b2l_ctime = 0x7654321; //???


  B2DEBUG(100, "EclPacker:: proceed COPPERs... ");
  B2DEBUG(100, "EclPacker:: ECL_COPPERS = " << ECL_COPPERS);

  //cycle over all coppers
  for (iCOPPER = 1; iCOPPER <= ECL_COPPERS; iCOPPER++) {

    std::vector <unsigned int> buff[ECL_FINESSES_IN_COPPER];
    for (int i = 0; i < ECL_FINESSES_IN_COPPER; i++) buff[i].clear();

    int iCOPPERNode = (iCOPPER <= ECL_BARREL_COPPERS) ? BECL_ID + iCOPPER : EECL_ID + iCOPPER - ECL_BARREL_COPPERS;

    //check if at least one of FINESSES have hits
    int icr1 = m_eclMapper->getCrateID(iCOPPERNode, 0);
    int icr2 = m_eclMapper->getCrateID(iCOPPERNode, 1);
    B2DEBUG(200, "iCOPPERNode = 0x" << std::hex << iCOPPERNode << std::dec << " nCrate1 = " << icr1 << " nCrate2 = " << icr2);
    if (!(collectorMaskArray[icr1 - 1] || collectorMaskArray[icr2 - 1])) continue;

    rawcprpacker_info.node_id = iCOPPERNode;
    // Create RawECL object

    int nwords[2] = {0, 0};
    const int finesseHeaderNWords = 3;

    //cycle over finesses in copper
    for (iFINESSE = 0; iFINESSE < ECL_FINESSES_IN_COPPER; iFINESSE++) {
      iCrate = m_eclMapper->getCrateID(iCOPPERNode, iFINESSE);

      nShapers = m_eclMapper->getNShapersInCrate(iCrate);
      if (!nShapers) B2ERROR("Ecl packer:: Wrong shapers number " << nShapers);

      if (!shaperMaskArray[iCrate - 1]) continue;
      B2DEBUG(200, "Pack data for iCrate = " << iCrate << " nShapers = " << nShapers);

      // write EclCollector header to the buffer
      unsigned int eclCollectorHeader = (1 << nShapers) - 1;
      if (m_compressMode) eclCollectorHeader += (1 << 12);
      buff[iFINESSE].push_back(eclCollectorHeader);

      for (iShaper = 1; iShaper <= nShapers; iShaper++) {

        nActiveDSPChannels = shaperNHits[iCrate - 1][iShaper - 1];
        B2DEBUG(200, "iCrate = " << iCrate << " iShaper = " << iShaper << " nActiveDSPChannels = " << nActiveDSPChannels);
        nActiveChannelsWithADCData = shaperNWaveform[iCrate - 1][iShaper - 1];
        B2DEBUG(200, "nActiveChannelsWithADCData = " << nActiveChannelsWithADCData);

        // write 4 words of shaper header

        unsigned int shaperDataLength = 4 + nActiveDSPChannels + nActiveChannelsWithADCData * ECL_ADC_SAMPLES_PER_CHANNEL;
        // fill shaperDsp header
        unsigned int shaper_header_w0 = (0x10 << 16) + (shaperDataLength & 0xFFFF);
        buff[iFINESSE].push_back(shaper_header_w0);

        triggerPhase = 0; //?????
        unsigned int shaper_header_w1 = (nActiveChannelsWithADCData & 0x1F) << 24;
        shaper_header_w1 |= (ECL_ADC_SAMPLES_PER_CHANNEL & 0x7F) << 16;
        shaper_header_w1 |= (nActiveDSPChannels & 0x1F) << 8;
        shaper_header_w1 |= triggerPhase;
        buff[iFINESSE].push_back(shaper_header_w1);

        dspMask = shaperMaskArray[iCrate - 1][iShaper - 1];
        B2DEBUG(200, "dspMask = " << std::hex << dspMask);
        unsigned int shaper_header_w2 = (dspMask & 0xFFFF) << 16;
        shaper_header_w2 |= (m_EvtNum & 0xFFFF); // trigger tag
        buff[iFINESSE].push_back(shaper_header_w2);

        unsigned int adcMask = shaperADCMaskArray[iCrate - 1][iShaper - 1];
        B2DEBUG(100, "adcMask = " << std::hex << adcMask);
        unsigned int shaper_header_w3 = (adcMask & 0xFFFF);
        buff[iFINESSE].push_back(shaper_header_w3);

        // cycle over shaper channels and push DSP data to buffer
        for (iChannel = 1; iChannel <= ECL_CHANNELS_IN_SHAPER; iChannel++) {

          int cid = m_eclMapper->getCellId(iCrate, iShaper, iChannel);

          if (cid < 1) continue;

          int i_digit = iEclDigIndices[cid - 1];
          if (i_digit < 0) continue;
          int qua = m_eclDigits[i_digit]->getQuality();
          int amp = m_eclDigits[i_digit]->getAmp();
          int chi = m_eclDigits[i_digit]->getChi();
          int tim = 0;
          int m = 0, p = 0;
          if (qua == 2) {
            // pack chisquare
            int n_bits = ceil(log2(double(chi)));
            if (n_bits > 9) {
              p = ceil(float(n_bits - 9) / 2.0);
              m = chi >> p * 2;
            } else {
              p = 0;
              m = chi;
            }
            tim = (p << 9) | m ;
          } else {
            tim = m_eclDigits[i_digit]->getTimeFit();
          }
          unsigned int hit_data = ((qua & 3) << 30) & 0xC0000000;
          hit_data |= (tim & 0xFFF) << 18;
          hit_data |= ((amp + 128) & 0x3FFFF);
          buff[iFINESSE].push_back(hit_data);

          B2DEBUG(100, "cid = " << cid << " amp = " << amp << " tim = " << tim);
        }

        for (int i = 0; i < ECL_CHANNELS_IN_SHAPER; i++) adcBuffer_temp[i] = 0;
        resetBuffPosition();
        setBuffLength(ECL_ADC_SAMPLES_PER_CHANNEL * ECL_CHANNELS_IN_SHAPER);
        for (iChannel = 1; iChannel <= ECL_CHANNELS_IN_SHAPER; iChannel++) {
          int cid = m_eclMapper->getCellId(iCrate, iShaper, iChannel);
          if (cid < 1) continue;
          int i_wf   = iEclWfIndices[cid - 1];
          if (i_wf < 0) continue;
          B2DEBUG(200, "i_wf = " << i_wf);
          m_eclDsps[i_wf]->getDspA(m_EclWaveformSamples); // Check this method in implementation of ECLDsp.h!!!

          unsigned int adc_data_base = 0;
          unsigned int adc_data_diff_width = 0;
          unsigned int adc_data_offset = 0;

          if (m_compressMode) {
            // calculate adc_data_base and adc_data_diff_width for compressed mode
            unsigned int ampMin = m_EclWaveformSamples[0];
            unsigned int ampMax = m_EclWaveformSamples[0];

            for (unsigned int iSample = 0; iSample < ECL_ADC_SAMPLES_PER_CHANNEL; iSample++) {
              if ((unsigned int) m_EclWaveformSamples[iSample] > ampMax) ampMax = m_EclWaveformSamples[iSample];
              if ((unsigned int) m_EclWaveformSamples[iSample] < ampMin) ampMin = m_EclWaveformSamples[iSample];
            }

            B2DEBUG(250, "ampMin = " << ampMin << " ampMax = " << ampMax);

            adc_data_base = ampMin & 0x3FFFF;
            writeNBits(adcBuffer_temp, adc_data_base, 18);
            adc_data_diff_width = (unsigned int)(log2((float)ampMax - (float)ampMin)) + 1;
            adc_data_diff_width &= 0x1F;
            writeNBits(adcBuffer_temp, adc_data_diff_width, 5);

            B2DEBUG(250, "Width = " << adc_data_diff_width << " Base = " << adc_data_base);

            for (unsigned int iSample = 0; iSample < ECL_ADC_SAMPLES_PER_CHANNEL; iSample++) {
              adc_data_offset = m_EclWaveformSamples[iSample] - adc_data_base;
              B2DEBUG(250, "offset = " << adc_data_offset);
              writeNBits(adcBuffer_temp, adc_data_offset, adc_data_diff_width);
            }
          } else {
            for (unsigned int iSample = 0; iSample < ECL_ADC_SAMPLES_PER_CHANNEL; iSample++) {
              buff[iFINESSE].push_back(m_EclWaveformSamples[iSample]);
            }

          }

        }
        if (m_compressMode) {
          if (m_bitPos > 0) m_bufPos++;
          for (int i = 0; i < m_bufPos; i++) {
            buff[iFINESSE].push_back(adcBuffer_temp[i]);

            B2DEBUG(500, "Buff word " << std::hex << adcBuffer_temp[i]);
          }
        }
      }
    }

    RawECL* newRawECL = m_eclRawCOPPERs.appendNew();

    nwords[0] = buff[0].size();
    nwords[1] = buff[1].size();

    buff[0][0] |= (nwords[0] - finesseHeaderNWords) * 4;
    buff[1][0] |= (nwords[1] - finesseHeaderNWords) * 4;

    B2DEBUG(100, "**** iEvt = " << m_EvtNum << " node= " << iCOPPERNode);
    for (unsigned int i = 0; i < 2; i++)
      for (unsigned int j = 0; j < buff[i].size(); j++) {
        B2DEBUG(210, ">> " << std::hex << setfill('0') << setw(8) << buff[i][j]);
      }

    B2DEBUG(100, "Call PackDetectorBuf");
    newRawECL->PackDetectorBuf((int*)buff[0].data(), nwords[0], (int*)buff[1].data(), nwords[1],
                               NULL, 0, NULL, 0, rawcprpacker_info);
  }
  m_EvtNum++;
}

void ECLPackerModule::endRun()
{
  //TODO
}

void ECLPackerModule::terminate()
{
}

void ECLPackerModule::setBuffLength(int bufLength)
{
  m_bufLength = bufLength;
}

void ECLPackerModule::resetBuffPosition()
{
  m_bufPos = 0;
  m_bitPos = 0;
}

void ECLPackerModule::writeNBits(unsigned int* buff, unsigned int value, unsigned int bitsToWrite)
{
  unsigned int tmpval = 0;

  if (!bitsToWrite) return;

  if (value > (unsigned int)(1 << bitsToWrite) - 1) {
    B2ERROR("Error compressing ADC samples: tying to write too long word");
    throw Write_adc_samples_error();
  }

  if (m_bitPos + bitsToWrite > 32)
    if (m_bufPos == m_bufLength) {
      B2ERROR("Error compressing ADC samples: unexpectedly reach end of buffer");
      throw Write_adc_samples_error();
    } else {
      tmpval = (1 << m_bitPos) - 1;
      buff[m_bufPos] &= tmpval;
      buff[m_bufPos] += value << m_bitPos;
      m_bufPos++;
      buff[m_bufPos] = value >> (32 - m_bitPos);
      m_bitPos += bitsToWrite;
      m_bitPos -= 32;
    }
  else {
    tmpval = (1 << m_bitPos) - 1;
    buff[m_bufPos] &= tmpval;
    buff[m_bufPos] += value << m_bitPos;
    m_bitPos += bitsToWrite;
    if (m_bitPos == 32) {
      m_bufPos++;
      m_bitPos -= 32;
    }
  }

}
