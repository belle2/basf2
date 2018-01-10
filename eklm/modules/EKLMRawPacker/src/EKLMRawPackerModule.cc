/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/modules/EKLMRawPacker/EKLMRawPackerModule.h>

using namespace std;
using namespace Belle2;

REG_MODULE(EKLMRawPacker)

EKLMRawPackerModule::EKLMRawPackerModule() : Module()
{
  setDescription("EKLM raw data packer (creates RawKLM from EKLMDigit).");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_GeoDat = NULL;
  m_NEvents = 0;
}

EKLMRawPackerModule::~EKLMRawPackerModule()
{
}

void EKLMRawPackerModule::initialize()
{
  m_Digits.isRequired();
  m_RawKLMs.registerInDataStore();
  m_GeoDat = &(EKLM::GeometryData::Instance());
}

void EKLMRawPackerModule::beginRun()
{
}

void EKLMRawPackerModule::event()
{
  int n_Gdidgits = 0;
  const EKLMDataConcentratorLane* lane;
  int i, j, k, endcap, layer, sector, sectorGlobal, copper, dataConcentrator;
  vector<uint32_t> dataWords[4][4]; // Indices: copper - 1, data concentrator.
  int* detectorBuf[4], nWords[4];
  uint32_t buf[2];
  uint16_t bword1, bword2, bword3, bword4;
  RawCOPPERPackerInfo packerInfo;
  RawKLM* rawKlm;
  EKLMDigit* eklmDigit;
  if (!m_ElectronicsMap.isValid())
    B2FATAL("No EKLM electronics map.");
  for (i = 0; i < m_Digits.getEntries(); i++) {
    eklmDigit = m_Digits[i];
    if (!(eklmDigit->isGood()))
      continue;
    n_Gdidgits++;
    buf[0] = 0;
    buf[1] = 0;
    bword1 = 0;
    bword2 = 0;
    bword3 = 0;
    bword4 = 0;
    endcap = eklmDigit->getEndcap();
    layer = eklmDigit->getLayer();
    sector = eklmDigit->getSector();
    sectorGlobal = m_GeoDat->sectorNumber(endcap, layer, sector);
    lane = m_ElectronicsMap->getLaneBySector(sectorGlobal);
    if (lane == NULL)
      B2FATAL("Incomplete EKLM electronics map.");
    formatData(lane, eklmDigit->getPlane(),
               eklmDigit->getStrip(), eklmDigit->getCharge(),
               eklmDigit->getTime(), bword1, bword2, bword3, bword4);
    buf[0] |= bword2;
    buf[0] |= ((bword1 << 16));
    buf[1] |= bword4;
    buf[1] |= ((bword3 << 16));
    copper = lane->getCopper() - 1;
    dataConcentrator = lane->getDataConcentrator();
    dataWords[copper][dataConcentrator].push_back(buf[0]);
    dataWords[copper][dataConcentrator].push_back(buf[1]);
  }
  for (i = 0; i < 4; i++) {
    // Fill event info (These values will be stored in RawHeader)
    packerInfo.exp_num = 1;
    /* Run number : 14bits, subrun number : 8bits. */
    packerInfo.run_subrun_num = 2;
    packerInfo.eve_num = m_NEvents;
    packerInfo.node_id = EKLM_ID + 1 + i;
    packerInfo.tt_ctime = 0x7123456;
    packerInfo.tt_utime = 0xF1234567;  //Triger info may be required
    packerInfo.b2l_ctime = 0x7654321;
    rawKlm = m_RawKLMs.appendNew();
    for (j = 0; j < 4; j++) {
      nWords[j] = dataWords[i][j].size();
      detectorBuf[j] = new int[nWords[j] + 1];
      for (k = 0; k < nWords[j]; k++)
        detectorBuf[j][k] = dataWords[i][j][k];
      detectorBuf[j][nWords[j]] = 0;
    }
    rawKlm->PackDetectorBuf(detectorBuf[0], nWords[0] + 1,
                            detectorBuf[1], nWords[1] + 1,
                            detectorBuf[2], nWords[2] + 1,
                            detectorBuf[3], nWords[3] + 1,
                            packerInfo);
    for (j = 0; j < 4; j++)
      delete[] detectorBuf[j];
  }
  m_NEvents++;
  return;
}

/*
 * Data format:
 * Word 1: Bit  0-6 - strip number.
 *         Bit  7 - plane number.
 *         Bit  8-12 - lane in the data concentrator.
 *         Bit  13-15 - data type: RPC=0x010 scintillator=0x100
 * Word 2: 15 bits of ctime, !!!!!!!!!!!!!!! NOT FILLED YET !!!!!!!!!!!!!
 * Word 3: TDC in 10 bits.
 * Word 4: 12 bits of charge. We use 15 yet since EDep from mc does not fit
 *         12 bits, e.g. after packing/unpacking variable changes 4829 -> 733
 */
void EKLMRawPackerModule::formatData(
  const EKLMDataConcentratorLane* lane, int plane, int strip,
  int charge, float time,
  uint16_t& bword1, uint16_t& bword2, uint16_t& bword3, uint16_t& bword4)
{
  bword1 = 0;
  bword2 = 0;
  bword3 = 0;
  bword4 = 0;
  bword1 |= (strip & 0x7F);
  bword1 |= (((plane - 1) & 1) << 7);
  bword1 |= ((lane->getLane() & 0x1F) << 8);
  bword1 |= (4 << 13);
  //bword2 |= (ctime & 0x7FFF); //Do we need CTIME ?
  bword3 |= (((int)time) & 0x3FF);
  bword4 |= (charge & 0xFFFF);  //SHOULD BE 12 BITS!!!
}

void EKLMRawPackerModule::endRun()
{
}

void EKLMRawPackerModule::terminate()
{
}

