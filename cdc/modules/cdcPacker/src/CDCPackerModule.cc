/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * CDC packer module                                                    *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Satoru Yamada and Makoto Uchida                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcPacker/CDCPackerModule.h>
#include <cdc/modules/cdcPacker/CDCChannelData.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRawHit.h>
#include <cdc/dbobjects/CDCChannelMap.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <framework/database/Database.h>
#include <framework/database/DBArray.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportArray.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>

using namespace std;
using namespace Belle2;
using namespace CDC;

#define NUM_CDC_COPPER 75
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCPacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCPackerModule::CDCPackerModule() : Module()
{
  //Set module properties
  setDescription("Generate RawCDC object from CDCHit");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("rawCDCName", m_rawCDCName, "Name of the RawCDC List name..", string(""));
  addParam("cdcRawHitName", m_cdcRawHitName, "Name of the CDCRawHit (Suppressed mode).", string(""));
  addParam("cdcHitName", m_cdcHitName, "Name of the CDCHit List name..", string(""));
  addParam("fadcThreshold", m_fadcThreshold, "Threshold voltage (mV).", 10);
  addParam("xmlMapFileName", m_xmlMapFileName, "path+name of the xml file",
           string("/cdc/data/ch_map.dat"));
  addParam("enableStoreRawCDC", m_enableStoreCDCRawHit, "Enable to store to the RawCDC object", true);
  addParam("enablePrintOut", m_enablePrintOut, "Enable to print out the data to the terminal", true);
  addParam("enableDatabase", m_enableDatabase, "Enable database to read the channel map.", true);

  m_channelMapFromDB.addCallback(this, &CDCPackerModule::loadMap);
}

CDCPackerModule::~CDCPackerModule()
{
}

void CDCPackerModule::initialize()
{

  B2INFO("CDCPacker: initialize() Called.");

  m_rawCDCs.registerInDataStore(m_rawCDCName);
  StoreArray<CDCRawHit> storeCDCRawHits(m_cdcRawHitName);

  storeCDCRawHits.registerInDataStore();

  StoreArray<CDCHit> storeDigit(m_cdcHitName);

  storeDigit.registerInDataStore();

  loadMap();

  B2INFO("CDCPacker: FADC threshold: " << m_fadcThreshold);

  m_event = 0;

}

void CDCPackerModule::beginRun()
{
  B2INFO("CDCPacker: beginRun() called.");
}

int CDCPackerModule::getFEEID(int copper_id, int slot_id)
{
  //
  // The releation between COPPER ID and FEE ID depends on
  // cable connection, which has not been finalized yet. ( 2015.6.16 )
  //

  //#define DEFAULT
#ifdef DEFAULT
  return (copper_id * 4 + slot_id);
#endif

  //#define PATTERN1
#ifdef PATTERN1
  return (copper_id  + slot_id * NUM_CDC_COPPER);
#endif

#define PATTERN2
#ifdef PATTERN2
  return ((copper_id / 15) * 60 + (copper_id % 15) + slot_id * 15);
#endif

}

void CDCPackerModule::event()
{

  // Create Data objects.
  StoreArray<CDCRawHit> cdcRawHits(m_cdcRawHitName);
  StoreArray<CDCHit> cdcHits(m_cdcHitName);
  //  std::vector<int> eWire_nhit(36882, 0);

  int tot_chdata_bytes[302];
  memset(tot_chdata_bytes, 0, sizeof(int) * 302);

  const int ch_data_bytes = 8;  // 8bytes ( 1hit/ch case)

  std::vector<CDCChannelData> chData;
  chData.clear();

  for (const auto& hit : cdcHits) {
    int eWire = (int)(hit.getID());
    int sly = eWire / 4096;
    int ily = (eWire % 4096) / 512;
    int iwire = (eWire % 512);
    short tdc = hit.getTDCCount();
    short adc = hit.getADCCount();

    //
    // If not prepared the map element for this cell, exit.
    //
    if (m_fee_board[ sly ][ ily ][ iwire] < 0 || m_fee_ch[ sly ][ ily ][ iwire] < 0) {
      printf("Hit WireID %8d SL %3d IL %3d WI %4d BOARD %3d CH %3d\n",
             (int)(hit.getID()), sly, ily , iwire,
             m_fee_board[ sly ][ ily ][ iwire], m_fee_ch[ sly ][ ily ][ iwire]);
      exit(1);
    }

    if (hit.is2ndHit() == false) { // first hit timing for one cell.
      // increase 8 bytes (4 bhytes).
      tot_chdata_bytes[ m_fee_board[ sly ][ ily ][ iwire] ] += ch_data_bytes;
      CDCChannelData chd(m_fee_board[sly][ily][iwire], m_fee_ch[sly][ily][iwire], 8, 0xbbaa, adc, tdc);
      chData.push_back(chd);
    } else { // second hit timing
      // Search ChData object for this cell.
      const int boardId = m_fee_board[sly][ily][iwire];
      const int channel = m_fee_ch[sly][ily][iwire];
      auto fi = std::find_if(chData.begin(), chData.end(),
      [&](CDCChannelData & ch) {
        return (ch.getBoard() == boardId && ch.getChannel() == channel);
      });
      if (fi != chData.end()) {
        tot_chdata_bytes[ m_fee_board[ sly ][ ily ][ iwire] ] += 2;
        const size_t index = std::distance(chData.begin(), fi);
        chData[index].setTDC2ndHit(tdc);
      }
    }

  }

  for (const auto& c : chData) {
    int board = c.getBoard();
    int ch = c.getChannel();
    bool flag = c.is2ndHit();
    int len = c.getDataLength();
    if (!((len == 8 && flag == false) ||
          (len == 10 && flag == true))) {
      B2ERROR("inconsistent data object board : " << board << " ch " << ch);
    }
  }


  RawCOPPERPackerInfo rawcprpacker_info;
  rawcprpacker_info.exp_num = 0;
  rawcprpacker_info.run_subrun_num = 1; // run number : 14bits, subrun # : 8bits
  rawcprpacker_info.eve_num = m_event;
  rawcprpacker_info.tt_ctime = 0x7123456;
  rawcprpacker_info.tt_utime = 0xF1234567;
  rawcprpacker_info.b2l_ctime = 0x7654321;

  const int packet_header_words = 3; // 12bytes

  for (int i = 0 ; i < NUM_CDC_COPPER; i++) {

    rawcprpacker_info.node_id = 0x02000000 + i + 1; // CDC detector ID + (node ID).
    int nwords[4] = {0, 0, 0, 0};
    int* buf[4] = {0, 0, 0, 0};

    for (int j = 0; j < 4; j++) {
      int fee_id = getFEEID(i, j);
      nwords[ j ] = ((tot_chdata_bytes[ fee_id ] + 3) / 4) + packet_header_words;

      buf[ j ] = new int[ nwords[ j ] ];

      // Store CDC header in buffer.
      const char type = 0x20; // suppressed mode.
      const char ver = 0x0; // version (always 0).
      const short trigTime = 0x0;
      const short dataLength = nwords[j] * 4 - packet_header_words * 4;
      const int trigNum = m_event;

      *(buf[j] + 0) = (type << 24) | (ver << 16) | fee_id;
      *(buf[j] + 1) = ((trigTime << 16) | dataLength);
      *(buf[j] + 2) = trigNum;

      short* sbuf = (short*)(buf[j] + 3);

      bool halfOffset = false;
      short reservedValue = 0xcccc;
      for (const auto& c : chData) {
        const int board = c.getBoard();
        const int ch = c.getChannel();
        const int len = c.getDataLength();
        if (board == fee_id) {
          //    printf("board %.8x ch %.8d adc %.8x tdc %.8x\n" ,board, ch, c.getADCCount(), c.getTDCCount());
          if (halfOffset == false) {
            *sbuf++ = c.getTOT();
            *sbuf++ = ((ch << 8) | len);
            *sbuf++ = c.getTDCCount();
            *sbuf++ = c.getADCCount();
            if (c.is2ndHit()) {
              reservedValue = c.getTDCCount2ndHit();
              halfOffset = true;
            }
          } else {
            *sbuf++ = ((ch << 8) | len);
            *sbuf++ = reservedValue;
            *sbuf++ = c.getADCCount();
            *sbuf++ = c.getTOT();
            if (c.is2ndHit()) {
              *sbuf++ = c.getTDCCount2ndHit();
              *sbuf++ = c.getTDCCount();
              halfOffset = false;
            } else {
              reservedValue = c.getTDCCount();
            }
          }
        }
      }
      if (halfOffset == true) {
        *sbuf++ = 0xff02;
        *sbuf = reservedValue;
      }
    }

    RawCDC* raw_cdc = m_rawCDCs.appendNew();
    raw_cdc->PackDetectorBuf(buf[0], nwords[0],
                             buf[1], nwords[1],
                             buf[2], nwords[2],
                             buf[3], nwords[3],
                             rawcprpacker_info);

    for (int j = 0; j < 4; j++) {
      if (buf[j] != NULL) delete [] buf[j];
    }
  }
  m_event++;

}

void CDCPackerModule::endRun()
{
  B2INFO("CDCPacker : End run.");
}

void CDCPackerModule::terminate()
{
  B2INFO("CDCPacker : Terminated.");
}


const WireID CDCPackerModule::getWireID(int iBoard, int iCh)
{
  return m_map[iBoard][iCh];
}

void CDCPackerModule::loadMap()
{

  // Frontend : 48 channels/board
  // Number of board : 302

  for (int i = 0 ; i < 9; ++i) {
    for (int j = 0 ; j < 8; ++j) {
      for (int k = 0 ; k < 384; ++k) {
        m_fee_board[i][j][k] = -1;
        m_fee_ch[i][j][k] = -1;
      }
    }
  }

  if (m_enableDatabase == false) {

    // Read the channel map from the local text.
    std::string fileName = FileSystem::findFile(m_xmlMapFileName);
    std::cout << fileName << std::endl;
    if (fileName == "") {
      B2ERROR("CDCPacker can't fine a filename: " << m_xmlMapFileName);
      exit(1);
    }

    ifstream ifs;
    ifs.open(fileName.c_str());
    int isl;
    int il;
    int iw;
    int iBoard;
    int iCh;

    while (!ifs.eof()) {

      ifs >>  isl >> il >> iw >> iBoard >> iCh;
      if (isl >= 9) continue; // Super layers should be from 0 t0 8.
      const WireID  wireId(isl, il, iw);
      m_map[iBoard][iCh] = wireId;
      m_fee_board[isl][il][iw] = iBoard;
      m_fee_ch[isl][il][iw] = iCh;
    }
  } else {
    // Read the channel map from the database.
    for (const auto& cm : m_channelMapFromDB) {
      const int isl = cm.getISuperLayer();
      if (isl >= 9) continue; // Super layers should be from 0 t0 8.
      const int il = cm.getILayer();
      const int iw = cm.getIWire();
      const int iBoard = cm.getBoardID();
      const int iCh = cm.getBoardChannel();
      const WireID  wireId(isl, il, iw);
      m_map[iBoard][iCh] = wireId;
      m_fee_board[isl][il][iw] = iBoard;
      m_fee_ch[isl][il][iw] = iCh;
    }
  }
}

