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
  addParam("MaxEventNum", m_MaxNEvents, "Maximum event number to make", -1);
  addParam("useDefaultModuleId", m_useDefaultElectId,
           "Use default elect id if not found in mapping", true);
  m_GeoDat = NULL;
  m_NEvents = 0;
}

EKLMRawPackerModule::~EKLMRawPackerModule()
{
}

void EKLMRawPackerModule::initialize()
{
  m_eventMetaDataPtr.registerInDataStore();
  m_RawKLMArray.registerPersistent();
  m_GeoDat = &(EKLM::GeometryData::Instance());
}

void EKLMRawPackerModule::beginRun()
{
}

void EKLMRawPackerModule::event()
{
  B2INFO("pack the event.." << endl);
  StoreArray<EKLMDigit> digits;
  B2INFO("EKLMRawPackerModule:: entries of eklmdigits " << digits.getEntries());
  int n_Gdidgits = 0;
  const EKLMDataConcentratorLane* lane;
  int i, endcap, layer, sector, sectorGlobal, copper, dataConcentrator;
  vector<uint32_t> data_words[4][4]; // Indices: copper - 1, data concentrator.
  uint32_t buf[2];
  uint16_t bword1, bword2, bword3, bword4;
  EKLMDigit* eklmDigit;
  if (!m_ElectronicsMap.isValid())
    B2FATAL("No EKLM electronics map.");
  for (i = 0; i < digits.getEntries(); i++) {
    eklmDigit = digits[i];
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
    data_words[copper][dataConcentrator].push_back(buf[0]);
    data_words[copper][dataConcentrator].push_back(buf[1]);
  }
//  B2INFO("EKLMRawPackerModule:: N_good_eklmdigits " << n_Gdidgits);
  RawCOPPERPackerInfo rawcprpacker_info;
  for (i = 0; i < 4; i++) {
    // Fill event info (These values will be stored in RawHeader)
    rawcprpacker_info.exp_num = 1;
    /* Run number : 14bits, subrun number : 8bits. */
    rawcprpacker_info.run_subrun_num = 2;
    rawcprpacker_info.eve_num = m_NEvents;
    rawcprpacker_info.node_id = EKLM_ID + 1 + i;
    rawcprpacker_info.tt_ctime = 0x7123456;
    rawcprpacker_info.tt_utime = 0xF1234567;  //Triger info may be required
    rawcprpacker_info.b2l_ctime = 0x7654321;
    //one call per copper
    RawKLM* raw_klm = m_RawKLMArray.appendNew();
    int* buf1, *buf2, *buf3, *buf4;
    int nwords_1st = data_words[i][0].size();
    int nwords_2nd = data_words[i][1].size();
    int nwords_3rd = data_words[i][2].size();
    int nwords_4th = data_words[i][3].size();
    buf1 = NULL;
    buf2 = NULL;
    buf3 = NULL;
    buf4 = NULL;
    buf1 = new int[nwords_1st + 1];
    buf2 = new int[nwords_2nd + 1];
    buf3 = new int[nwords_3rd + 1];
    buf4 = new int[nwords_4th + 1];
    for (int j = 0; j < nwords_1st; j++) buf1[j] = data_words[i][0][j];
    buf1[nwords_1st] = 0; // DIVOT
    for (int j = 0; j < nwords_2nd; j++) buf2[j] = data_words[i][1][j];
    buf2[nwords_2nd] = 0; // DIVOT
    for (int j = 0; j < nwords_3rd; j++) buf3[j] = data_words[i][2][j];
    buf3[nwords_3rd] = 0; // DIVOT
    for (int j = 0; j < nwords_4th; j++) buf4[j] = data_words[i][3][j];
    buf4[nwords_4th] = 0; // DIVOT
    raw_klm->PackDetectorBuf(buf1, nwords_1st + 1,
                             buf2, nwords_2nd + 1,
                             buf3, nwords_3rd + 1,
                             buf4, nwords_4th + 1,
                             rawcprpacker_info);
    delete [] buf1;
    delete [] buf2;
    delete [] buf3;
    delete [] buf4;
  }
  B2INFO("Event # " << m_NEvents);
  // Monitor
  if (m_MaxNEvents >= 0) {
    if (m_NEvents >= m_MaxNEvents && m_MaxNEvents > 0) {
      B2INFO("RunStop was detected. ( Setting:  Max event # " << m_MaxNEvents <<
             " ) Processed Event " << m_NEvents);
      m_eventMetaDataPtr->setEndOfData();
    }
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

