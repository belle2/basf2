/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>


using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDPacker);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

std::string Belle2::SVD::SVDPackerModule::m_xmlFileName = std::string("SVDChannelMapping.xml");

SVDPackerModule::SVDPackerModule() : Module(),
  m_mapping(m_xmlFileName),
  m_FADCTriggerNumberOffset(0)
{
  // Set module properties


  // Parameter definitions
  addParam("SVDEventInfo", m_svdEventInfoName, "Name of the SVDEventInfo object", string("SVDEventInfoSim"));
  addParam("svdShaperDigitListName", m_svdShaperDigitListName, "Name of the SVD Shaper Digits List", string(""));
  addParam("rawSVDListName", m_rawSVDListName, "Name of the raw SVD List", string(""));
  //addParam("xmlMapFileName", m_xmlMapFileName, "path+name of the xml file", FileSystem::findFile("data/svd/svd_mapping.xml"));
  addParam("NodeID", m_nodeid, "Node ID", 0);
  addParam("FADCTriggerNumberOffset", m_FADCTriggerNumberOffset,
           "number to be added to the FADC trigger number to match the main trigger number", 0);
  addParam("binPrintout", m_binPrintout, "Print binary data created by the Packer", bool(false));
  // initialize event #
  n_basf2evt = 0;

}


SVDPackerModule::~SVDPackerModule()
{
}



void SVDPackerModule::initialize()
{

  m_rawSVD.registerInDataStore(m_rawSVDListName);
  m_svdShaperDigit.isRequired(m_svdShaperDigitListName);
  m_eventMetaDataPtr.isRequired();
  m_svdEventInfoPtr.isRequired(m_svdEventInfoName);
}


void SVDPackerModule::beginRun()
{
  if (m_mapping.hasChanged()) { m_map = std::make_unique<SVDOnlineToOfflineMap>(m_mapping->getFileName()); }

  //number of FADC boards
  nFADCboards = m_map->getFADCboardsNumber();

  // filling FADCnumberMap and FADCnumberMapRev
  m_map->prepFADCmaps(FADCnumberMap, FADCnumberMapRev);

  // passing APV<->FADC mapping from SVDOnlineToOfflineMap object
  APVmap = &(m_map->APVforFADCmap);

}


#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage="
#endif
void SVDPackerModule::event()
{

  if (!m_eventMetaDataPtr.isValid()) {  // give up...
    B2ERROR("Missing valid EventMetaData.");
    return;
  }

  if (!m_svdEventInfoPtr.isValid()) {
    B2ERROR("Missing valid SVDEventInfo.");
    return;
  }

  // retrieve the information from SVDEventInfo to store it in the FADCHeaders for this event
  SVDModeByte modeByte = m_svdEventInfoPtr->getModeByte();

  uint8_t triggerBin = modeByte.getTriggerBin();
  uint8_t runType = modeByte.getRunType();
  uint8_t daqMode = modeByte.getDAQMode();
  uint8_t daqType = 0;
  if (daqMode == 3) daqType = 1;

  uint8_t xTalk = m_svdEventInfoPtr->isCrossTalkEvent();
  uint8_t triggerType = (m_svdEventInfoPtr->getTriggerType()).getType();


  m_rawSVD.clear();

  if (! m_map) {
    B2ERROR("xml map not loaded, going to the next module");
    return;
  }


  RawCOPPERPackerInfo rawcprpacker_info;
  rawcprpacker_info.exp_num = 0;
  rawcprpacker_info.run_subrun_num = 1; // run number : 14bits, subrun # : 8bits
  rawcprpacker_info.eve_num = n_basf2evt;
  rawcprpacker_info.node_id = m_nodeid;
  rawcprpacker_info.tt_ctime = 0x7123456;
  rawcprpacker_info.tt_utime = 0xF1234567;
  rawcprpacker_info.b2l_ctime = 0x7654321;


  unsigned int nEntries_SVDShaperDigits = m_svdShaperDigit.getEntries();

  // DataInfo contains info on 6 samples and strip number
  vector<DataInfo> (*fadc_apv_matrix)[48] = new
  vector<DataInfo>[nFADCboards][48]; // for all layers, nFADCboards FADCs and at most 48 APV25 for each

  for (unsigned int i = 0; i < nEntries_SVDShaperDigits; i++) {

    const SVDShaperDigit* hit = m_svdShaperDigit[i];

    short int cellID = hit->getCellID();
    VxdID sensID = hit->getSensorID();
    bool isU = hit->isUStrip();

    int sensor = sensID.getSensorNumber();
    int ladder = sensID.getLadderNumber();
    int layer = sensID.getLayerNumber();

    // find FADC/APV for given DSSD sensor
    const SVDOnlineToOfflineMap::ChipInfo& CHIP_info = m_map->getChipInfo(layer, ladder, sensor, isU, cellID);
    unsigned short fadc = CHIP_info.fadc;
    unsigned short apv = CHIP_info.apv;
    unsigned short apvChannel = CHIP_info.apvChannel;

    //do not create data words for APV missing in the hardware mapping
    if (fadc == 0) continue;

    // return 0-47 for given FADC number
    auto fadcIter = FADCnumberMap.find(fadc);

    //getting charge samples from SVDShaperDigit object
    SVDShaperDigit::APVFloatSamples samples = hit->getSamples();

    // fill DataInfo
    for (unsigned short j = 0; j < 6; j++) {
      dataInfo.data[j] = samples[j];
    }
    dataInfo.channel =  apvChannel;

    //fill fadc_apv_matrix[fadc][apv] with DataInfo object
    fadc_apv_matrix[fadcIter->second][apv].push_back(dataInfo);

  }// end of the loop that fills fadc_apv_matrix !!!!


  for (unsigned int iFADC = 0; iFADC < nFADCboards; iFADC++) {

    //get original FADC number
    unsigned short FADCorg = FADCnumberMapRev[iFADC];

    //new RawSVD entry --> moved inside FADC loop
    RawSVD* raw_svd = m_rawSVD.appendNew();
    data_words.clear();


    //--------------- start creating Raw Data words

    // here goes FTB header
    data32 = 0xffaa0000;

    //adds data32 to data vector
    addData32(data32);

    m_FTBHeader.errorsField = 0xf0;
    m_FTBHeader.eventNumber = (m_eventMetaDataPtr->getEvent() & 0xFFFFFF);

    addData32(data32);

    // here goes FADC header
    m_MainHeader.trgNumber = ((m_eventMetaDataPtr->getEvent() - m_FADCTriggerNumberOffset) & 0xFF);
    m_MainHeader.trgType = triggerType;
    m_MainHeader.trgTiming = triggerBin;
    m_MainHeader.xTalk = xTalk;
    m_MainHeader.FADCnum = FADCorg; // write original FADC number
    m_MainHeader.DAQType = daqType;
    m_MainHeader.DAQMode = daqMode;

    m_MainHeader.runType = runType; //zero-suppressed mode
    m_MainHeader.check = 6; // 110

    addData32(data32);

    // getting a set of APV numbers for current FADC
    auto apv_range = APVmap->equal_range(FADCorg);

    for (auto& it = apv_range.first; it != apv_range.second; ++it) {
      unsigned short iAPV = it->second;

      // here goes APV header
      m_APVHeader.CMC1 = 0;
      m_APVHeader.CMC2 = 0;

      m_APVHeader.fifoErr = 0;
      m_APVHeader.frameErr = 0;
      m_APVHeader.detectErr = 0;
      m_APVHeader.apvErr = 0;

      m_APVHeader.pipelineAddr = 0;
      m_APVHeader.APVnum = iAPV;
      m_APVHeader.check = 2;

      addData32(data32);

      // alias for table element (DataInfo object)
      vector<DataInfo>& apv_data_vec = fadc_apv_matrix[iFADC][iAPV];

      if (apv_data_vec.size() > 0) { // if any data for given FADC/APV
        for (std::vector<DataInfo>::iterator apv_data = apv_data_vec.begin(); apv_data != apv_data_vec.end(); ++apv_data) {

          // here go DATA words

          //skip 1st data frame if simulate 3-sample data
          if (m_svdEventInfoPtr->getNSamples() == 6) {
            m_data_A.sample1 = apv_data->data[0];
            m_data_A.sample2 = apv_data->data[1];
            m_data_A.sample3 = apv_data->data[2];
            m_data_A.stripNum = apv_data->channel;
            m_data_A.check = 0;

            addData32(data32);
          }

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
    m_FADCTrailer.FTBFlags = 0;
    m_FADCTrailer.dataSizeCut = 0;
    m_FADCTrailer.nullDigits = 0;
    m_FADCTrailer.fifoErrOR = 0;
    m_FADCTrailer.frameErrOR = 0;
    m_FADCTrailer.detectErrOR = 0;
    m_FADCTrailer.apvErrOR = 0;
    m_FADCTrailer.check = 14;

    addData32(data32);


    // crc16 calculation
    //first swap all 32-bits word -> big endian
    unsigned short nCRC = data_words.size();
    uint32_t tmpBuffer[nCRC];

    for (unsigned short i = 0; i < nCRC; i++)
      tmpBuffer[i] = htonl(data_words[i]);

    //compute crc16
    boost::crc_basic<16> bcrc(0x8005, 0xffff, 0, false, false);
    bcrc.process_block(tmpBuffer, tmpBuffer + nCRC);
    unsigned int crc = bcrc.checksum();


    // here goes FTB trailer
    m_FTBTrailer.crc16 = crc;
    m_FTBTrailer.controlWord = 0xff55;

    addData32(data32);


    // ******* modified and moved inside FADC loop **********

    unsigned int nwords_1st = data_words.size();
    unsigned int nwords_2nd = 0;
    unsigned int nwords_3rd = 0;
    unsigned int nwords_4th = 0;

    int* buf1 = new int[nwords_1st];
    int* buf2 = nullptr;
    int* buf3 = nullptr;
    int* buf4 = nullptr;

    // filling buffers
    for (unsigned int j = 0; j < nwords_1st; j++) {
      buf1[j] = data_words[j];
    }

    raw_svd->PackDetectorBuf(buf1, nwords_1st,
                             buf2, nwords_2nd,
                             buf3, nwords_3rd,
                             buf4, nwords_4th,
                             rawcprpacker_info);

    delete [] buf1;

    // ********************************************************

  } // end FADC loop


  if (m_binPrintout) binPrintout(data_words.size());

  delete [] fadc_apv_matrix;

  n_basf2evt++;

} // end event function
#ifndef __clang__
#pragma GCC diagnostic pop
#endif


void SVDPackerModule::terminate()
{
}


void SVDPackerModule::endRun()
{
}



// function for printing 32-bit frames in binary mode
void SVDPackerModule::binPrintout(unsigned int nwords)
{

  B2INFO("\nbinary printout:");
  for (unsigned int j = 0; j < nwords; j++) {

    uint32_t ulFlag = 1;
    ulFlag <<= (sizeof(data_words[j]) * 8 - 1);
    for (; ulFlag > 0; ulFlag >>= 1)
      printf("%d", (data_words[j] & ulFlag) ? 1 : 0);

    cout << endl;
  }

}

