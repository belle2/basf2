/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//This module
#include <ecl/modules/eclPacker/eclPackerModule.h>

//STL
#include <ios>
#include <iomanip>

//Framework
#include <framework/logging/Logger.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>

//Raw data
#include <rawdata/dataobjects/RawECL.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLPacker);

ECLPackerModule::ECLPackerModule() :
  m_bufPos(0),
  m_bufLength(0),
  m_bitPos(0),
  m_EclWaveformSamples(),
  m_eclMapper(),
  m_eclRawCOPPERs("", DataStore::c_Event),
  adcBuffer_temp(),
  collectorMaskArray(),
  shaperMaskArray(),
  shaperADCMaskArray(),
  shaperNWaveform(),
  shaperNHits(),
  iEclDigIndices(),
  iEclWfIndices()
{
  setDescription("The module reads ECLDigits from the DataStore and writes ECLRaw data.");
  addParam("InitFileName", m_eclMapperInitFileName, "Initialization file", string("/ecl/data/ecl_channels_map.txt"));
  addParam("RawCOPPERsName", m_eclRawCOPPERsName, "Name of the RawECL container", string("RawECLs"));
  addParam("CompressMode", m_compressMode, "compress mode for ADC samples", true);
  addParam("AmpThreshold", m_ampThreshold, "Amplitude threshold", -50);
  addParam("PackWfRareFactor", m_WaveformRareFactor, "Pack ADC samples for one of N events. No waveform is packed if 0", 100);
  addParam("Pcie40Data",  m_pcie40Data,
           "If true: pack data as if sent from PCIe40 boards. Otherwise: pack data as if sent from COPPERs", false);
}

ECLPackerModule::~ECLPackerModule()
{
}

void ECLPackerModule::initialize()
{
  // require input data
  m_eclDigits.isRequired();
  m_eclDsps.isOptional();

  // register output container in data store
  m_eclRawCOPPERs.registerInDataStore(m_eclRawCOPPERsName);

  B2INFO("ECL Packer: Compress mode = " << m_compressMode);
}

void ECLPackerModule::beginRun()
{
  // Initialize channel mapper at run start to account for possible
  // changes in ECL mapping between runs.
  if (!m_eclMapper.initFromDB()) {
    B2FATAL("ECL Packer: Can't initialize eclChannelMapper!");
  }

  //TODO
}

void ECLPackerModule::event()
{
  if (m_eventMetaData.isValid()) {
    m_EvtNum = m_eventMetaData->getEvent();
  } else {
    m_EvtNum = -1;
  }

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
    iCrate = m_eclMapper.getCrateID(cid);
    iShaper = m_eclMapper.getShaperPosition(cid);
    iChannel = m_eclMapper.getShaperChannel(cid);

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
        iCrate = m_eclMapper.getCrateID(cid);
        iShaper = m_eclMapper.getShaperPosition(cid);
        iChannel = m_eclMapper.getShaperChannel(cid);

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

  //Set the number of nodes
  int max_nodes;
  if (m_pcie40Data) {
    max_nodes = 3;
  } else {
    max_nodes = ECL_COPPERS;
  }
  const static int max_channels = MAX_PCIE40_CH;

  //cycle over all coppers
  for (iCOPPER = 1; iCOPPER <= max_nodes; iCOPPER++) {
    std::vector <unsigned int> buff[max_channels];
    int channels_count;
    if (m_pcie40Data) {
      channels_count = iCOPPER < 3 ? 18 : 16;
    } else {
      channels_count = 2;
    }

    for (int i = 0; i < max_channels; i++) buff[i].clear();

    int iCOPPERNode;
    if (m_pcie40Data) {
      iCOPPERNode = BECL_ID + iCOPPER;
    } else {
      iCOPPERNode = (iCOPPER <= ECL_BARREL_COPPERS) ? BECL_ID + iCOPPER : EECL_ID + iCOPPER - ECL_BARREL_COPPERS;
    }

    bool skipNode = true;

    //check if at least one of FINESSES have hits
    for (int i = 0; i < channels_count; i++) {
      int icr = m_eclMapper.getCrateID(iCOPPERNode, i, m_pcie40Data);
      B2DEBUG(200, "iCOPPERNode = 0x" << std::hex << iCOPPERNode << std::dec << " nCrate = " << icr);
      if (!collectorMaskArray[icr - 1]) continue;
      skipNode = false;
      break;
    }

    if (skipNode) continue;

    rawcprpacker_info.node_id = iCOPPERNode;

    // Create RawECL object

    int nwords[max_channels] = {0, 0};
    const int finesseHeaderNWords = 3;

    //cycle over finesses in copper
    for (iFINESSE = 0; iFINESSE < channels_count; iFINESSE++) {
      iCrate = m_eclMapper.getCrateID(iCOPPERNode, iFINESSE, m_pcie40Data);

      nShapers = m_eclMapper.getNShapersInCrate(iCrate);
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

          const int cid = m_eclMapper.getCellId(iCrate, iShaper, iChannel);

          if (cid < 1) continue;

          const int i_digit = iEclDigIndices[cid - 1];
          if (i_digit < 0) continue;
          const int qua = m_eclDigits[i_digit]->getQuality();
          const int amp = m_eclDigits[i_digit]->getAmp();
          const int chi = m_eclDigits[i_digit]->getChi();
          int tim = 0;
          if (qua == 2) {
            // pack chisquare

            int chi_mantissa = 0, chi_exponent = 0;
            int n_bits = ceil(log2(double(chi)));
            if (n_bits > 9) {
              chi_exponent = ceil(float(n_bits - 9) / 2.0);
              chi_mantissa = chi >> chi_exponent * 2;
            } else {
              chi_exponent = 0;
              chi_mantissa = chi;
            }
            tim = (chi_exponent << 9) | chi_mantissa;
          } else {
            // pack time
            tim = m_eclDigits[i_digit]->getTimeFit();
          }
          unsigned int hit_data = ((unsigned int)(qua & 3) << 30) & 0xC0000000;
          hit_data |= (tim & 0xFFF) << 18;
          hit_data |= ((amp + 128) & 0x3FFFF);
          buff[iFINESSE].push_back(hit_data);

          B2DEBUG(100, "cid = " << cid << " amp = " << amp << " tim = " << tim);
        }

        for (int i = 0; i < ECL_CHANNELS_IN_SHAPER; i++) adcBuffer_temp[i] = 0;
        resetBuffPosition();
        setBuffLength(static_cast<int>(ECL_ADC_SAMPLES_PER_CHANNEL) * static_cast<int>(ECL_CHANNELS_IN_SHAPER));
        for (iChannel = 1; iChannel <= ECL_CHANNELS_IN_SHAPER; iChannel++) {
          int cid = m_eclMapper.getCellId(iCrate, iShaper, iChannel);
          if (cid < 1) continue;
          int i_wf   = iEclWfIndices[cid - 1];
          if (i_wf < 0) continue;
          B2DEBUG(200, "i_wf = " << i_wf);
          m_eclDsps[i_wf]->getDspA(m_EclWaveformSamples); // Check this method in implementation of ECLDsp.h!!!

          if (m_compressMode) {
            unsigned int adc_data_base = 0;
            unsigned int adc_data_diff_width = 0;

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
              unsigned int adc_data_offset = m_EclWaveformSamples[iSample] - adc_data_base;
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

    for (int i = 0; i < channels_count; i++) {
      nwords[i] = buff[i].size();

      buff[i][0] |= (nwords[i] - finesseHeaderNWords) * 4;
    }

    B2DEBUG(100, "**** iEvt = " << m_EvtNum << " node= " << iCOPPERNode);
    for (int i = 0; i < channels_count; i++)
      for (unsigned int j = 0; j < buff[i].size(); j++) {
        B2DEBUG(210, ">> " << std::hex << setfill('0') << setw(8) << buff[i][j]);
      }

    B2DEBUG(100, "Call PackDetectorBuf");
    if (m_pcie40Data) {
      int* pcie40_words[MAX_PCIE40_CH];
      int pcie40_nwords[MAX_PCIE40_CH] = {};

      for (int i = 0; i < channels_count; i++) {
        pcie40_nwords[i] = nwords[i];
        pcie40_words[i] = new int[ nwords[i] ];
        for (int j = 0; j < nwords[i]; j++) {
          pcie40_words[i][j] = buff[i][j];
        }
      }

      newRawECL->PackDetectorBuf(pcie40_words, pcie40_nwords, rawcprpacker_info);
    } else { // COPPER data
      newRawECL->PackDetectorBuf((int*)buff[0].data(), nwords[0], (int*)buff[1].data(), nwords[1],
                                 nullptr, 0, nullptr, 0, rawcprpacker_info);
    }
  }
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
  if (!bitsToWrite) return;

  if (bitsToWrite > sizeof(value) * 8) {
    B2ERROR("Error compressing ADC samples: tying to write too long word");
    throw Write_adc_samples_error();
  }

  if (m_bitPos + bitsToWrite > 32) {
    if (m_bufPos == m_bufLength) {
      B2ERROR("Error compressing ADC samples: unexpectedly reach end of buffer");
      throw Write_adc_samples_error();
    } else {
      unsigned tmpval = (1 << m_bitPos) - 1;
      buff[m_bufPos] &= tmpval;
      buff[m_bufPos] += value << m_bitPos;
      m_bufPos++;
      buff[m_bufPos] = value >> (32 - m_bitPos);
      m_bitPos += bitsToWrite;
      m_bitPos -= 32;
    }
  } else {
    unsigned tmpval = (1 << m_bitPos) - 1;
    buff[m_bufPos] &= tmpval;
    buff[m_bufPos] += value << m_bitPos;
    m_bitPos += bitsToWrite;
    if (m_bitPos == 32) {
      m_bufPos++;
      m_bitPos -= 32;
    }
  }

}
