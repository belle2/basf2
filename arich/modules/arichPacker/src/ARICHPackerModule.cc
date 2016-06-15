/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <arich/modules/arichPacker/ARICHPackerModule.h>



// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// Dataobject classes
#include <arich/dataobjects/ARICHDigit.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <framework/dataobjects/EventMetaData.h>
#include <bitset>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHPacker)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHPackerModule::ARICHPackerModule() : Module(),
    m_arichgp(ARICHGeometryPar::Instance()), m_nonSuppressed(0), m_bitMask(0), m_debug(0)

  {
    // set module description (e.g. insert text)
    setDescription("Raw data packer for ARICH");
    setPropertyFlags(c_ParallelProcessingCertified);
    addParam("nonSuppressed", m_nonSuppressed, "Pack in non-suppressed format (store all channels)", 0);
    addParam("bitMask", m_bitMask, "hit bit mask (4 bits/channel)", (unsigned)0xF);
    addParam("debug", m_debug, "print packed bitmap", 0);
    addParam("inputDigitsName", m_inputDigitsName, "name of ARICHDigit store array", string(""));
    addParam("outputRawDataName", m_outputRawDataName, "name of RawARICH store array", string(""));

  }

  ARICHPackerModule::~ARICHPackerModule()
  {
  }

  void ARICHPackerModule::initialize()
  {

    StoreArray<ARICHDigit> digits(m_inputDigitsName);
    digits.isRequired();

    StoreArray<RawARICH> rawData(m_outputRawDataName);
    rawData.registerInDataStore();

    if (!m_arichgp->isInit()) {
      GearDir content("/Detector/DetectorComponent[@name='ARICH']/Content");
      m_arichgp->Initialize(content);
    }
    if (!m_arichgp->isInit()) B2ERROR("Component ARICH not found in Gearbox");

  }

  void ARICHPackerModule::beginRun()
  {
  }

  void ARICHPackerModule::event()
  {

    StoreObjPtr<EventMetaData> evtMetaData;
    StoreArray<ARICHDigit> digits(m_inputDigitsName);
    StoreArray<RawARICH> rawData(m_outputRawDataName);

    int nModules = m_arichgp->getNMCopies();

    vector<const ARICHDigit*>* sortedDigits = new vector<const ARICHDigit*>[nModules];

    for (const auto& digit : digits) {
      int boardID = digit.getModuleID();
      sortedDigits[boardID - 1].push_back(&digit);
    }

    int bufferDim = 0;
    bufferDim = 1 + 12 + 6 * 18; // 1 merger header word + 2 FEB header words / FEB + 18 data words per / FEB

    int buffer[4][bufferDim];

    for (const auto& copperID : m_arichgp->getCopperIDs()) {
      int bufferSize[4] = {0, 0, 0, 0};
      for (int finesse = 0; finesse < 4; finesse++) {

        for (int j = 0; j < bufferDim; j++) {
          buffer[finesse][j] = 0;
        }

        auto* buf = buffer[finesse];
        auto& i = bufferSize[finesse];

        // get corresponding merger ID
        unsigned mergerID = m_arichgp->getMergerFromCooper(copperID, finesse);
        if (!mergerID) continue;

        unsigned dataFormat = m_nonSuppressed + 1; // production data -> TODO: use enum
        unsigned version = 0;
        buf[i] = (mergerID << 8) + (version << 16) + (dataFormat << 24);
        i++; // one merger (finesse) header word

        int nboards = m_arichgp->getNBoardsOnMerger(mergerID);

        for (int k = 0; k < nboards; k++) {

          int boardID = m_arichgp->getBoardFromMerger(mergerID, k);
          if (boardID < 0) continue;

          // FEB first header word
          buf[i] = k + (mergerID << 8) + (version << 16) + (dataFormat << 24);
          i++;

          if (m_nonSuppressed) {
            // data length in bytes
            buf[i] = unsigned(18 * 4); // 144ch * 4bit
            i++;
            // write data
            for (const auto& digit : sortedDigits[boardID - 1]) {
              unsigned chn = digit->getChannelID();
              unsigned chan_n = chn / 8;
              unsigned chan_i = chn % 8;
              unsigned bitmap = (unsigned)digit->getBitmap();
              buf[i + chan_n] += (bitmap << chan_i * 4);
            }
            i += 18;
          }

          else {
            // data length in bytes (2 bytes/digit)
            buf[i] = (unsigned)sortedDigits[boardID - 1].size() * 2;
            i++;
            int j = 1;
            // write data
            for (const auto& digit : sortedDigits[boardID - 1]) {
              unsigned chn = digit->getChannelID();
              unsigned bitmap = (unsigned)digit->getBitmap();
              unsigned chn_data = bitmap + (chn << 8);
              if (j % 2 == 1) buf[i] = (chn_data << 16);
              else {buf[i] += chn_data; i++;}
              j++;
            }
            if (j % 2 == 0) i++;
          }
        }

        if (m_debug) {
          std::cout << "Pack finesse: " << finesse << std::endl;
          for (int i = 0; i < bufferSize[finesse]; i++) {
            std::cout << i << "-th word bitset " << bitset<32>(buf[i]) << std::endl;
          }
        }
      }

      RawCOPPERPackerInfo info;
      info.exp_num = evtMetaData->getExperiment();
      info.run_subrun_num = (evtMetaData->getRun() << 8) +
                            (evtMetaData->getSubrun() & 0xFF); // run number : 14bits, subrun # : 8bits
      info.eve_num = evtMetaData->getEvent();
      info.node_id = ARICH_ID + copperID;
      info.tt_ctime = 0;
      info.tt_utime = 0;
      info.b2l_ctime = 0;
      info.hslb_crc16_error_bit = 0;
      info.truncation_mask = 0;
      info.type_of_data = 0;

      auto* raw = rawData.appendNew();
      raw->PackDetectorBuf(buffer[0], bufferSize[0],
                           buffer[1], bufferSize[1],
                           buffer[2], bufferSize[2],
                           buffer[3], bufferSize[3],
                           info);

    }

    delete [] sortedDigits;

  }


  void ARICHPackerModule::endRun()
  {
  }

  void ARICHPackerModule::terminate()
  {
  }


} // end Belle2 namespace

