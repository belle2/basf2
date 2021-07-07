/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMPacker/KLMPackerModule.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

REG_MODULE(KLMPacker)

KLMPackerModule::KLMPackerModule() :
  Module(),
  m_ElementNumbers(&(KLMElementNumbers::Instance()))
{
  setDescription("KLM raw data packer (creates RawKLM from KLMDigits).");
  setPropertyFlags(c_ParallelProcessingCertified);
}

KLMPackerModule::~KLMPackerModule()
{
}

void KLMPackerModule::initialize()
{
  m_Digits.isRequired();
  m_EventMetaData.isRequired();
  m_RawKLMs.registerInDataStore();
}

void KLMPackerModule::beginRun()
{
  if (!m_ElectronicsMap.isValid())
    B2FATAL("KLM electronics map is not available.");
}

void KLMPackerModule::event()
{
  /* Indices: copper, data concentrator.
   * Coppers from 0 to 3 are BKLM;
   * coppers from 4 to 7 are EKLM.
   */
  std::vector<uint32_t> dataWords[8][4];

  /* Pack KLM digits. */
  for (const KLMDigit& digit : m_Digits) {
    uint32_t buf[2] = {0};
    uint16_t bword1 = 0;
    uint16_t bword2 = 0;
    uint16_t bword3 = 0;
    uint16_t bword4 = 0;
    int channel =
      m_ElementNumbers->channelNumber(
        digit.getSubdetector(), digit.getSection(), digit.getSector(),
        digit.getLayer(), digit.getPlane(), digit.getStrip());
    const KLMElectronicsChannel* electronicsChannel =
      m_ElectronicsMap->getElectronicsChannel(channel);
    if (electronicsChannel == nullptr)
      B2FATAL("Incomplete KLM electronics map.");
    int flag;
    if (digit.inRPC())
      flag = 2; // RPC
    else
      flag = 4; // Scintillator
    int lane = electronicsChannel->getLane();
    int plane = electronicsChannel->getAxis();
    int strip = electronicsChannel->getChannel();
    formatData(flag, lane, plane, strip,
               digit.getCharge(), digit.getCTime(), digit.getTDC(),
               bword1, bword2, bword3, bword4);
    buf[0] |= bword2;
    buf[0] |= ((bword1 << 16));
    buf[1] |= bword4;
    buf[1] |= ((bword3 << 16));
    if (digit.getSubdetector() == KLMElementNumbers::c_BKLM) {
      int copper = electronicsChannel->getCopper() - BKLM_ID - 1;
      int dataConcentrator = electronicsChannel->getSlot() - 1;
      dataWords[copper][dataConcentrator].push_back(buf[0]);
      dataWords[copper][dataConcentrator].push_back(buf[1]);
    } else {
      int copper = electronicsChannel->getCopper() - EKLM_ID - 1;
      int dataConcentrator = electronicsChannel->getSlot() - 1;
      dataWords[copper + 4][dataConcentrator].push_back(buf[0]);
      dataWords[copper + 4][dataConcentrator].push_back(buf[1]);
    }
  }

  /* Create RawKLM objects. */
  RawCOPPERPackerInfo packerInfo;
  packerInfo.exp_num = m_EventMetaData->getExperiment();
  packerInfo.run_subrun_num = (m_EventMetaData->getRun() << 8) +
                              (m_EventMetaData->getSubrun() & 0xFF);
  packerInfo.eve_num = m_EventMetaData->getEvent();
  packerInfo.tt_ctime = 0;
  packerInfo.tt_utime = 0;
  packerInfo.b2l_ctime = 0;
  int* detectorBuf[4];
  int nWords[4];
  for (int i = 0; i < 8; i++) {
    if (i < 4) // BKLM
      packerInfo.node_id = BKLM_ID + 1 + i;
    else // EKLM
      packerInfo.node_id = EKLM_ID + 1 + i - 4;
    RawKLM* rawKlm = m_RawKLMs.appendNew();
    for (int j = 0; j < 4; j++) {
      nWords[j] = dataWords[i][j].size();
      detectorBuf[j] = new int[nWords[j] + 1];
      for (int k = 0; k < nWords[j]; k++)
        detectorBuf[j][k] = dataWords[i][j][k];
      detectorBuf[j][nWords[j]] = 0;
    }
    rawKlm->PackDetectorBuf(detectorBuf[0], nWords[0] + 1,
                            detectorBuf[1], nWords[1] + 1,
                            detectorBuf[2], nWords[2] + 1,
                            detectorBuf[3], nWords[3] + 1,
                            packerInfo);
    for (int j = 0; j < 4; j++)
      delete[] detectorBuf[j];
  }
}

/*
 * Data format:
 * Word 1: Bit 0-6   -> strip number.
 *         Bit 7     -> plane number.
 *         Bit 8-12  -> lane in the data concentrator.
 *         Bit 13-15 -> data type: RPC=0x010; scintillator=0x100.
 * Word 2: 15 bits of ctime.
 * Word 3: 10 bits of TDC.
 * Word 4: 12 bits of charge.
 */
void KLMPackerModule::formatData(int flag, int lane, int plane, int strip, int charge, uint16_t ctime, uint16_t tdc,
                                 uint16_t& bword1, uint16_t& bword2, uint16_t& bword3, uint16_t& bword4)
{
  bword1 = 0;
  bword2 = 0;
  bword3 = 0;
  bword4 = 0;
  bword1 |= (strip & 0x7F);
  bword1 |= ((plane & 1) << 7);
  bword1 |= ((lane & 0x1F) << 8);
  bword1 |= (flag << 13);
  bword2 |= (ctime & 0xFFFF);
  bword3 |= (tdc & 0x3FF);
  bword4 |= (charge & 0xFFF);
}

void KLMPackerModule::endRun()
{
}

void KLMPackerModule::terminate()
{
}

