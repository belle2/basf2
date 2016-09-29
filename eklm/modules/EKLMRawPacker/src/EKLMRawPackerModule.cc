/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/modules/EKLMRawPacker/EKLMRawPackerModule.h>
#include <eklm/dataobjects/EKLMDigit.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EKLMRawPacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EKLMRawPackerModule::EKLMRawPackerModule() : Module()
{
  ///Set module properties
  setDescription("an Example to pack data to a RawKLM object");

  ///  maximum # of events to produce( -1 : inifinite)
  addParam("MaxEventNum", max_nevt, "Maximum event number to make", -1);
  addParam("useDefaultModuleId", m_useDefaultElectId, "use default elect id if not found in mapping", true);

  B2INFO("EKLMRawPackerModule: Constructor done.");

  n_basf2evt = 0;
}



EKLMRawPackerModule::~EKLMRawPackerModule()
{
}


void EKLMRawPackerModule::initialize()
{
  B2INFO("EKLMRawPackerModule: initialize() started.");

  m_eventMetaDataPtr.registerInDataStore();

  rawklmarray.registerPersistent();

  B2INFO("EKLMRawPackerModule: initialize() done.");
  loadMap();
}




void EKLMRawPackerModule::event()
{
  B2INFO("pack the event.." << endl);
  StoreArray<EKLMDigit> digits;
  vector<uint32_t> data_words[4][4];//4 copper, 16 finesse
  data_words[4][4].clear();
  //int tot_num_hits=digits.getEntries();

  B2INFO("EKLMRawPackerModule:: entries of eklmdigits " << digits.getEntries());
  ///fill data_words
  int n_Gdidgits = 0;
  for (int d = 0; d < digits.getEntries(); d++) {
    if (!(digits[d]->isGood())) continue;
    n_Gdidgits++;

    int* buf = new int[2];//for one hit, hit length is 2;
    buf[0] = 0;
    buf[1] = 0;
    EKLMDigit* digit = digits[d];
//OBTAIN PARAMETERS OF HIT FROM MC---------------------------
    int   iForward = digit->getEndcap();
    int   iLayer   = digit->getLayer();
    int   iSector  = digit->getSector();
    int   iPlane   = digit->getPlane();
    int   iStrip   = digit->getStrip();

    int   iNPE  = digit->getNPE();
    float fEDep    = digit->getEDep();
    float fTime    = digit->getTime();

//     float fMCTime  = digit->getSiPMMCTime();
//     float fGTime   = digit->getGlobalTime();
//     int   iTDC = 0;                           //---Do we need TDC?
//GET MODULE ID IN ELECTRONIC FROM PARAMETERS----------------------------------
    int electId = 1;
    int moduleId = (iSector - 1)
                   | ((iLayer - 1)   << 2)
                   | ((iForward - 1) << 6);
//                 | ((iPlane-1)   << 7);            //Do we need plane?

    if (m_ModuleIdToelectId.find(moduleId) == m_ModuleIdToelectId.end()) {
      B2INFO("EKLMRawPacker::can not find in mapping for moduleId " << moduleId << " isForward? " << iForward <<
             " , layer " << iLayer << " , sector " << iSector  << endl);
      if (m_useDefaultElectId) electId = 17;     //Defaults: copper=1; finesse=1;
      else                    continue;
    } else   electId = m_ModuleIdToelectId[moduleId];

    int copperId = electId & 0xF;
    int finesse = (electId >> 4) & 0xF;

    B2INFO("EKLMRawPacker::copperId " << copperId << " F: " << iForward << " Lay: " << iLayer << " Sect: " << iSector << " Pl: " <<
           iPlane << " Str: " << iStrip << " Cha: " << iNPE << " T: " << fTime << endl);
//MAKE WORDS WITH INFORMATION
    unsigned short bword1 = 0;
    unsigned short bword2 = 0;
    unsigned short bword3 = 0;
    unsigned short bword4 = 0;
    formatData(iForward, iLayer, iSector, iPlane, iStrip, fEDep, fTime, bword1, bword2, bword3, bword4);
    buf[0] |= bword2;
    buf[0] |= ((bword1 << 16));
    buf[1] |= bword4;
    buf[1] |= ((bword3 << 16));
    data_words[copperId][finesse].push_back(buf[0]);
    data_words[copperId][finesse].push_back(buf[1]);

    delete [] buf;
  }
  rawklmarray.create();

  B2INFO("EKLMRawPackerModule:: N_good_eklmdigits " << n_Gdidgits);

  RawCOPPERPackerInfo rawcprpacker_info;
  for (int i = 0 ; i < 4; i++) {
    // Fill event info (These values will be stored in RawHeader )
    rawcprpacker_info.exp_num = 1;
    rawcprpacker_info.run_subrun_num = 2; // run number : 14bits, subrun # : 8bits
    rawcprpacker_info.eve_num = n_basf2evt;
    rawcprpacker_info.node_id = EKLM_ID + i;                               //?????????????????????????????????
    rawcprpacker_info.tt_ctime = 0x7123456;
    rawcprpacker_info.tt_utime = 0xF1234567;
    rawcprpacker_info.b2l_ctime = 0x7654321;

    //one call per copper
    RawKLM* raw_klm = rawklmarray.appendNew();

    int* buf1, *buf2, *buf3, *buf4;
    int nwords_1st = data_words[i][0].size();
    int nwords_2nd = data_words[i][1].size();
    int nwords_3rd = data_words[i][2].size();
    int nwords_4th = data_words[i][3].size();

    buf1 = NULL;
    buf2 = NULL;
    buf3 = NULL;
    buf4 = NULL;
    buf1 = new int[ nwords_1st + 1];
    buf2 = new int[ nwords_2nd + 1];
    buf3 = new int[ nwords_3rd + 1];
    buf4 = new int[ nwords_4th + 1];

    for (int j = 0; j < nwords_1st; j++) {
      buf1[j] = data_words[i][0][j];
    }
    buf1[nwords_1st] = 0; // DIVOT

    for (int j = 0; j < nwords_2nd; j++) {
      buf2[j] = data_words[i][1][j];
    }
    buf2[nwords_2nd] = 0; // DIVOT

    for (int j = 0; j < nwords_3rd; j++) {
      buf3[j] = data_words[i][2][j];
    }
    buf3[nwords_3rd] = 0; // DIVOT

    for (int j = 0; j < nwords_4th; j++) {
      buf4[j] = data_words[i][3][j];
    }
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

  printf("Event # %.8d\n", n_basf2evt);
  fflush(stdout);

  // Monitor
  if (max_nevt >= 0) {
    if (n_basf2evt >= max_nevt && max_nevt > 0) {
      printf("[DEBUG] RunStop was detected. ( Setting:  Max event # %d ) Processed Event %d \n", max_nevt , n_basf2evt);
      fflush(stdout);
      m_eventMetaDataPtr->setEndOfData();
    }
  }

  n_basf2evt++;
  return;
}



void EKLMRawPackerModule::formatData(int forward, int layer, int sector, int plane, int strip, float charge, float ctime,
                                     unsigned short& bword1,
                                     unsigned short& bword2, unsigned short& bword3, unsigned short& bword4)
{
  bword1 = 0; //Mapping(1st bit - enmpty; 2nd bit - forward; 3,4,5,6 bits - layer; 7,8 - sector; 9 - plane; 10,11,12,13,14,15,16 - strip)
  bword2 = 0; //15 bits of time
  bword3 = 0; //Want to write TDC in 15 bits or less
  bword4 = 0; //15 bits of charge
  bword1 |= (strip & 0x7F);
  bword1 |= ((plane & 1) << 7);
  bword1 |= ((sector & 3) << 8);
  bword1 |= ((layer & 0xF) << 10);
  bword1 |= ((forward & 1) << 14);
  bword2 |= (((int)ctime) & 0x7FFF);                       //TEMPORARY SCALING ?????????????
//  bword3 |= (TDC & 0x7FFF);                              //DO WE NEED TDC ????????????????
  bword4 |= (((int)(charge * 30000.0 / 0.3)) & 0x7FFF);    //TEMPORARY SCALING ?????????????
}

void EKLMRawPackerModule::loadMap()
{
  for (int forward = 1; forward <= 2; forward++) {
    for (int layer = 1; layer <= 12 + (forward - 1) * 2; layer++) {
      for (int sector = 1; sector <= 4; sector++) {
        //Do we need loop over planes as well?
        int copperId = 1;
        int finesseId = 1;

        //Here we somehow get coppers and finesses from the position of sector, layer, etc.
        //copper=?????????????????????????????????????????????
        //finesse=?????????????????????????????????????????????

        int elecid = (copperId & 0xF) | ((finesseId & 0xF) << 4);
        int moduleId = (sector - 1)
                       | ((layer - 1)   << 2)
                       | ((forward - 1) << 6);
//                        | ((plane-1) << 7);            //Do we need plane?
        m_ModuleIdToelectId[moduleId] = elecid;
        B2INFO(" electId: " << elecid << " modId: " << moduleId << endl);
      }
    }
  }
}
