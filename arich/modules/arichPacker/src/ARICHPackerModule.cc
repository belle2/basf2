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

#include <framework/core/ModuleManager.h>

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

#define ARICH_BUFFER_NWORDS 252 // arich number of words (ints) in buffer;  3 + 33 + 6 * 36; 3 merger header words + 5.5 FEB header words / FEB + 36 data words per / FEB
  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHPacker)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHPackerModule::ARICHPackerModule() : Module(),
    m_nonSuppressed(0), m_bitMask(0), m_debug(0)

  {
    // set module description (e.g. insert text)
    setDescription("Raw data packer for ARICH");
    setPropertyFlags(c_ParallelProcessingCertified);
    addParam("nonSuppressed", m_nonSuppressed, "Pack in non-suppressed format (store all channels)", unsigned(0));
    addParam("version", m_version, "dataformat version", unsigned(6));
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
  }

  void ARICHPackerModule::beginRun()
  {
  }

  void ARICHPackerModule::event()
  {

    StoreObjPtr<EventMetaData> evtMetaData;
    StoreArray<ARICHDigit> digits(m_inputDigitsName);
    StoreArray<RawARICH> rawData(m_outputRawDataName);

    int nModules = N_MERGERS * N_FEB2MERGER;

    vector<const ARICHDigit*>* sortedDigits = new vector<const ARICHDigit*>[nModules];
    int nPacked = 0;
    for (const auto& digit : digits) {
      int moduleID = digit.getModuleID();
      unsigned mergerID = m_mergerMap->getMergerID(moduleID);
      if (!mergerID) { B2WARNING("No module2merger mapping for module ID: " <<  moduleID << "; Digit will not be packed!"); continue;}
      if (!m_copperMap->getCopperID(mergerID)) { B2WARNING("No merger2copper mapping for merger ID: " <<  mergerID << "; Digit will not be packed!"); continue;}
      sortedDigits[moduleID - 1].push_back(&digit);
      nPacked++;
    }

    int buffer[4][ARICH_BUFFER_NWORDS];

    for (const auto& copperID : m_copperMap->getCopperIDs()) {

      int bufferSize[4]  = {0, 0, 0, 0};
      for (int finesse = 0; finesse < 4; finesse++) {

        unsigned ibyte = 0;

        for (int j = 0; j < ARICH_BUFFER_NWORDS; j++) {
          buffer[finesse][j] = 0;
        }

        auto* buf = buffer[finesse];

        // get corresponding merger ID
        unsigned mergerID = m_copperMap->getMergerID(copperID, finesse);
        unsigned mergerSN = m_mergerMap->getMergerSN(mergerID);
        if (!mergerID) continue;

        ARICHRawHeader mergerHead;
        unsigned dataFormat = m_nonSuppressed + 1; // production data -> TODO: use enum
        mergerHead.type = dataFormat;
        mergerHead.version = m_version;
        mergerHead.mergerID = mergerSN;
        mergerHead.FEBSlot = 0;
        mergerHead.trigger = evtMetaData->getEvent();
        // mergerHead.length dont forget

        ibyte += ARICHRAW_HEADER_SIZE;

        int nboards = N_FEB2MERGER;

        for (int k = 0; k < nboards; k++) {

          int moduleID = m_mergerMap->getModuleID(mergerID, k + 1);
          if (moduleID <= 0) continue;

          // FEB header
          ARICHRawHeader FEBHead;
          FEBHead.type = dataFormat;
          FEBHead.version = m_version;
          FEBHead.mergerID = mergerSN;
          FEBHead.FEBSlot = k; // board slots go from 0-5 for now, if firmware is updated to 1-6 add +1 !!
          FEBHead.trigger = evtMetaData->getEvent();

          if (m_nonSuppressed) {
            // data length in bytes
            FEBHead.length = 144 + ARICHFEB_HEADER_SIZE; // 144ch * 1 byte
            writeHeader(buf, ibyte, FEBHead);
            ibyte += ARICHFEB_HEADER_SIZE; // leave slot for FEB header (FEB header to be implemented!)

            // write data
            for (const auto& digit : sortedDigits[moduleID - 1]) {
              unsigned chn = digit->getChannelID();
              //std::cout << "pack: mod: " << boardID << " ch " << chn<< std::endl;
              unsigned shift = 143 - chn;
              unsigned bitmap = (unsigned)digit->getBitmap();
              buf[(ibyte + shift) / 4] += (bitmap << (3 - (ibyte + shift) % 4) * 8);
            }
            ibyte += 144;
          } else {
            FEBHead.length = ARICHFEB_HEADER_SIZE + sortedDigits[moduleID - 1].size() * 2; // each hit is 2 bytes! channel + bitmap
            writeHeader(buf, ibyte, FEBHead);
            ibyte += ARICHFEB_HEADER_SIZE; // leave slot for FEB header (FEB header to be implemented!)

            for (const auto& digit : sortedDigits[moduleID - 1]) {
              unsigned chn = digit->getChannelID();
              //std::cout << "pack: mod: " << boardID << " ch " << chn<< std::endl;
              unsigned shift = (3 - ibyte % 4) * 8;
              buf[ibyte / 4] += (chn << shift);
              ibyte++;
              shift = (3 - ibyte % 4) * 8;
              unsigned bitmap = (unsigned)digit->getBitmap();
              buf[ibyte / 4] += (bitmap << shift);
              ibyte++;
            }
          }
        }

        unsigned merg = 0;
        mergerHead.length = ibyte - ARICHRAW_HEADER_SIZE;
        writeHeader(buf, merg, mergerHead);

        bufferSize[finesse] = ceil(ibyte / 4.);

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

  void ARICHPackerModule::writeHeader(int* buffer, unsigned& ibyte, const ARICHRawHeader& head)
  {

    unsigned char line1[4];
    int shift;

    line1[3] = head.type;
    line1[2] = head.version;
    line1[1] = head.mergerID;
    line1[0] = head.FEBSlot;

    for (int i = 0; i < 4; i++) {
      shift = (3 - ibyte % 4) * 8;
      buffer[ibyte / 4] |= line1[3 - i] << shift;
      ibyte++;
    }

    unsigned char* len = (unsigned char*)&head.length;
    for (int i = 0; i < 4; i++) {
      shift = (3 - ibyte % 4) * 8;
      buffer[ibyte / 4] |= len[3 - i] << shift;
      ibyte++;
    }

    // trigger number
    unsigned char* trg = (unsigned char*)(&head.trigger);
    for (int i = 0; i < 4; i++) {
      shift = (3 - ibyte % 4) * 8;
      buffer[ibyte / 4] |= trg[3 - i] << shift;
      ibyte++;
    }

  }




  void ARICHPackerModule::endRun()
  {
  }

  void ARICHPackerModule::terminate()
  {
  }


} // end Belle2 namespace

