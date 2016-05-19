/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jarek Wiechczynski                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdPacker/SVDPackerModule.h>
#include <svd/online/SVDOnlineToOfflineMap.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <arpa/inet.h>
#include <boost/crc.hpp>

#include <sstream>
#include <iomanip>
#include <cstring>
//
#include<vector>



using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDPacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDPackerModule::SVDPackerModule() : Module()
{
  // Set module properties


  // Parameter definitions
  addParam("svdDigitListName", m_svdDigitListName, "Name of the SVD Digits List", string(""));
  addParam("rawSVDListName", m_rawSVDListName, "Name of the raw SVD List", string(""));
  //addParam("xmlMapFileName", m_xmlMapFileName, "path+name of the xml file", string(""));
  addParam("xmlMapFileName", m_xmlMapFileName, "path+name of the xml file", FileSystem::findFile("data/svd/svd_mapping.xml"));
  addParam("NodeID", m_nodeid, "Node ID", 0);
  // initialize event #
  n_basf2evt = 0;

}


SVDPackerModule::~SVDPackerModule()
{
}



void SVDPackerModule::initialize()
{

  StoreArray<RawSVD>::registerPersistent(m_rawSVDListName);
  StoreArray<SVDDigit>::required(m_svdDigitListName);
  loadMap();
  prepFADClist();

}


void SVDPackerModule::beginRun()
{

}



void SVDPackerModule::event()
{

  StoreArray<RawSVD> rawSVDList(m_rawSVDListName);
  StoreArray<SVDDigit> svdDigits(m_svdDigitListName);

  rawSVDList.clear();


  if (! m_map) {
    B2ERROR("xml map not loaded, going to the next module");
    return;
  }


  RawCOPPERPackerInfo rawcprpacker_info;
  rawcprpacker_info.exp_num = 0;
  rawcprpacker_info.run_subrun_num = 1; // run number : 14bits, subrun # : 8bits
  rawcprpacker_info.eve_num = n_basf2evt;
  rawcprpacker_info.node_id = m_nodeid;
  //B2INFO(" event number =  " << n_basf2evt << endl)
  // ???????
  rawcprpacker_info.tt_ctime = 0x7123456;
  rawcprpacker_info.tt_utime = 0xF1234567;
  rawcprpacker_info.b2l_ctime = 0x7654321;


  unsigned int nEntries_SVDDigits = svdDigits.getEntries();
  //B2INFO("nEntries_SVDDigits = "  << nEntries_SVDDigits);


  data_words.clear();


  // DataInfo contains info on 6 samples and strip number
  vector<DataInfo> fadc_apv_matrix[48][48]; // for all layers, 48 FADCs and at most 48 APV25 for each


  for (unsigned int i = 0; i < nEntries_SVDDigits; i++) {

    const SVDDigit* hit = svdDigits[i];

    short int cellID = hit->getCellID();
    //float cellPOS = hit->getCellPosition();
    VxdID sensID = hit->getSensorID();
    bool isU = hit->isUStrip();
    short idx = hit->getIndex();

    int sensor = sensID.getSensorNumber();
    int ladder = sensID.getLadderNumber();
    int layer = sensID.getLayerNumber();


    if (idx == 0) { // fill fadc_apv_matrix

      // find FADC/APV for given DSSD sensor
      const SVDOnlineToOfflineMap::ChipInfo& CHIP_info = m_map->getChipInfo(layer, ladder, sensor, isU, cellID);
      unsigned short fadc = CHIP_info.fadc;
      unsigned short apv = CHIP_info.apv;
      unsigned short apvChannel = CHIP_info.apvChannel;

      // return 0-47 for given FADC number
      auto fadcIter = fadcNumbers.find(fadc);


      // fill DataInfo
      for (unsigned short j = 0; j < 6; j++)
        dataInfo.data[j] = svdDigits[i + j]->getCharge();
      dataInfo.channel =  apvChannel;

      //fill fadc_apv_matrix[fadc][apv] with DataInfo object
      fadc_apv_matrix[fadcIter->second][apv].push_back(dataInfo);


    } // endif i%6==0
  } // end of the loop that fills fadc_apv_matrix !!!!




//new RawSVD entry
  RawSVD* raw_svd = rawSVDList.appendNew();

  // for each buffer in RawSVD object we store 12 FTBs/FADCs (48 in total)
  unsigned int j_buf[3]; // table of values indicating where new buffer begins

  for (unsigned int iFADC = 0; iFADC < 48; iFADC++) {

    iCRC = 0;

    // here goes FTB header
    data32 = 0xffaa0000;

    if (iFADC > 0
        and iFADC % 12 == 0)   j_buf[iFADC / 12 - 1] =
            data_words.size(); // iFADC=12 -> j_buf[0]=size;   iFADC=24 -> j_buf[1]=size;  iFADC=36 -> j_buf[2]=size;


    //adds data32 to data vector and to crc16Input for further crc16 calculation
    addData32(data32);

    m_FTBHeader.errorsField = 0;
    m_FTBHeader.eventNumber = iFADC; // for now FTB header contains FTB number 0-47 !!!!!!!!!!

    addData32(data32);


    // here goes FADC header
    m_MainHeader.trgNumber = 0;
    m_MainHeader.trgTiming = 0;
    m_MainHeader.FADCnum = iFADCnumber[iFADC]; // write original FADC number
    m_MainHeader.evtType = 0;
    m_MainHeader.runType = 0x2; //zero-suppressed mode
    m_MainHeader.check = 6; // 110

    addData32(data32);

    for (unsigned int iAPV = 0; iAPV < 48; iAPV++) {

      // here goes APV header
      m_APVHeader.CMC1 = 0;
      m_APVHeader.CMC2 = 0;
      m_APVHeader.reserved = 0;
      m_APVHeader.errorBit = 0;
      m_APVHeader.pipelineAddr = 0;
      m_APVHeader.APVnum = iAPV;
      m_APVHeader.check = 2;

      addData32(data32);

      // alias for table element (DataInfo object)
      vector<DataInfo>& apv_data_vec = fadc_apv_matrix[iFADC][iAPV];

      if (apv_data_vec.size() > 0) { // if any data for given FADC/APV
        for (std::vector<DataInfo>::iterator apv_data = apv_data_vec.begin(); apv_data != apv_data_vec.end(); ++apv_data) {

          m_data_A.sample1 = apv_data->data[0];
          m_data_A.sample2 = apv_data->data[1];
          m_data_A.sample3 = apv_data->data[2];
          m_data_A.stripNum = apv_data->channel;
          m_data_A.check = 0;

          addData32(data32);

          m_data_B.sample4 = apv_data->data[3];
          m_data_B.sample5 = apv_data->data[4];
          m_data_B.sample6 = apv_data->data[5];
          m_data_B.stripNum = apv_data->channel;
          m_data_B.check = 0;

          addData32(data32);
        }
      }

    } // end APV loop

    // here goes FADC trailer
    m_FADCTrailer.FTBFlags = 0x001f;
    m_FADCTrailer.emPipeAddr = 0;
    m_FADCTrailer.wiredOrErr = 0;
    m_FADCTrailer.error0 = 0;
    m_FADCTrailer.error1 = 0;
    m_FADCTrailer.error2 = 0;
    m_FADCTrailer.check = 14;

    addData32(data32);


    // crc16 calculation
    //first swap all 32-bits word -> big endian
    uint32_t tmpBuffer[iCRC];

    for (unsigned short i = 0; i < iCRC; i++)
      tmpBuffer[i] = htonl(crc16Input[i]);

    //compute crc16
    boost::crc_basic<16> bcrc(0x8005, 0xffff, 0, false, false);
    bcrc.process_block(tmpBuffer, tmpBuffer + iCRC);
    unsigned int crc = bcrc.checksum();


    // here goes FTB trailer
    m_FTBTrailer.crc16 = crc;
    m_FTBTrailer.controlWord = 0xff55;

    data_words.push_back(data32);

  } // end FADC loop


  //binPrintout(data_words.size());


// # of 32-bit frames in each buffer
  unsigned int nwords_1st = j_buf[0];
  unsigned int nwords_2nd = j_buf[1] - j_buf[0];
  unsigned int nwords_3rd = j_buf[2] - j_buf[1];
  unsigned int nwords_4th = data_words.size() - j_buf[2];

  int* buf1 = new int[nwords_1st];
  int* buf2 = new int[nwords_2nd];
  int* buf3 = new int[nwords_3rd];
  int* buf4 = new int[nwords_4th];


// filling buffers
  for (unsigned int j = 0; j < nwords_1st; j++) {
    buf1[j] = data_words[j];
  }

  for (unsigned int j = 0; j < nwords_2nd; j++) {
    buf2[j] = data_words[nwords_1st + j];
  }

  for (unsigned int j = 0; j < nwords_3rd; j++) {
    buf3[j] = data_words[nwords_1st + nwords_2nd + j];
  }

  for (unsigned int j = 0; j < nwords_4th; j++) {
    buf4[j] = data_words[nwords_1st + nwords_2nd + nwords_3rd + j];
  }


  raw_svd->PackDetectorBuf(buf1, nwords_1st,
                           buf2, nwords_2nd,
                           buf3, nwords_3rd,
                           buf4, nwords_4th,
                           rawcprpacker_info);

  delete [] buf1;
  delete [] buf2;
  delete [] buf3;
  delete [] buf4;

  n_basf2evt++;

} // end event function



void SVDPackerModule::terminate()
{
  delete m_map;

}


void SVDPackerModule::endRun()
{

}

//load the sensor MAP from xml file
void SVDPackerModule::loadMap()
{
  m_map = new SVDOnlineToOfflineMap(m_xmlMapFileName);

}

// mapping FADC numbers as 0-47
void SVDPackerModule::prepFADClist()
{
  unsigned short it = 0;
  while (it < 48) {
    fadcNumbers[iFADCnumber[it]] = it;
    it++;
  }

}


// function for printing 32-bit frames in binary mode
void SVDPackerModule::binPrintout(unsigned int nwords)
{

  B2INFO("\nbinary printout:");
  for (unsigned int j = 0; j < nwords; j++) {

    uint32_t ulFlag = 1 << (sizeof(data_words[j]) * 8 - 1);
    for (; ulFlag > 0; ulFlag >>= 1)
      printf("%d", data_words[j] & ulFlag ? 1 : 0);

    cout << endl;
  }

}


