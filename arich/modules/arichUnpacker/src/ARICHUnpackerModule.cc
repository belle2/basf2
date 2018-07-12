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
#include <arich/modules/arichUnpacker/ARICHUnpackerModule.h>

#include <arich/modules/arichUnpacker/ARICHRawDataHeader.h>

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
#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <arich/dataobjects/ARICHDigit.h>
#include <arich/dataobjects/ARICHInfo.h>
#include <arich/dataobjects/ARICHRawDigit.h>

// print bitset
#include <bitset>

#include <sstream>
#include <TH1F.h>
#include <TH2F.h>
#include <TF1.h>

using namespace std;

namespace Belle2 {

  //using namespace ARICH;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHUnpacker)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHUnpackerModule::ARICHUnpackerModule() : Module(), m_bitMask(0), m_debug(0)
  {
    // set module description
    setDescription("Raw data unpacker for ARICH");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("bitMask", m_bitMask, "hit bit mask (8 bits/channel, only used for unsuppresed format!)", (uint8_t)0xFF);
    addParam("debug", m_debug, "prints debug information", 0);

    addParam("inputRawDataName", m_inputRawDataName, "name of RawARICH store array", string(""));
    addParam("outputDigitsName", m_outputDigitsName, "name of ARICHDigit store array", string(""));
    addParam("outputRawDigitsName", m_outputRawDigitsName, "name of ARICHRawDigit store array", string(""));
    addParam("outputarichinfoName", m_outputarichinfoName, "name of ARICHInfo store array", string(""));
    addParam("RawUnpackerMode", m_rawmode, "Activate RawUnpacker mode", 0);
    addParam("DisableUnpackerMode", m_disable_unpacker, "Disable Regular Unpacker mode", 0);

  }

  ARICHUnpackerModule::~ARICHUnpackerModule()
  {
  }

  void ARICHUnpackerModule::initialize()
  {

    StoreArray<RawARICH> rawData(m_inputRawDataName);
    rawData.isRequired();

    StoreArray<ARICHDigit> digits(m_outputDigitsName);
    digits.registerInDataStore();

    StoreArray<ARICHRawDigit> rawdigits(m_outputRawDigitsName);
    rawdigits.registerInDataStore();

    StoreObjPtr<ARICHInfo> arichinfo(m_outputarichinfoName);
    arichinfo.registerInDataStore();

  }

  void ARICHUnpackerModule::beginRun()
  {
  }

  void ARICHUnpackerModule::event()
  {

    StoreArray<RawARICH> rawData(m_inputRawDataName);
    StoreArray<ARICHDigit> digits(m_outputDigitsName);
    StoreArray<ARICHRawDigit> rawdigits(m_outputRawDigitsName);
    StoreObjPtr<ARICHInfo> arichinfo(m_outputarichinfoName);
    arichinfo.create();
    StoreObjPtr<EventMetaData> evtMetaData;

    digits.clear();
    int trgtype = 16;

    if (m_debug) {
      std::cout << std::endl << "------------------------" << std::endl;
      std::cout << "Run: " << evtMetaData->getRun() << " Event: " << evtMetaData->getEvent()  << std::endl;
      std::cout << "------------------------" << std::endl << std::endl;
    }

    // regular Unpacker mode, fill ARICHDigit
    if (m_disable_unpacker == 0) {

      for (auto& raw : rawData) {
        for (int finesse = 0; finesse < 4; finesse++) {
          const int* buffer = raw.GetDetectorBuffer(0, finesse);
          int bufferSize = raw.GetDetectorNwords(0, finesse);

          if (bufferSize < 1)
            continue;

          // record the trigger type from the B2L data
          trgtype = raw.GetTRGType(0);

          // read merger header
          unsigned ibyte = 0;
          ARICHRawHeader head;

          readHeader(buffer, ibyte, head);

          if (m_debug > 1) printBits(buffer, bufferSize);

          if (m_debug) {
            std::cout << "Merger header" << std::endl;
            head.print();
          }

          while (ibyte < head.length) {

            // new feb
            ARICHRawHeader febHead;
            readHeader(buffer, ibyte, febHead);
            if (m_debug) febHead.print();

            if (/*febHead.type != head.type ||*/ febHead.version != head.version || febHead.mergerID != head.mergerID
                                                 || febHead.trigger != head.trigger) B2ERROR("ARICHUnpackerModule: data in FEB header " << (unsigned)febHead.FEBSlot <<
                                                       " not consistent with data in merger header " << (unsigned)head.mergerID);

            // feb header shift
            ibyte += ARICHFEB_HEADER_SIZE;
            int dataLen = febHead.length - ARICHFEB_HEADER_SIZE;

            febHead.FEBSlot += 1; /// temporary! FEB Slots on merger should be 1-6 (now 0-5). Remove when firmware is updated!

            unsigned mergID = m_mergerMap->getMergerIDfromSN((unsigned)head.mergerID);

            if (mergID == 99) { B2ERROR("ARICHUnpackerModule: unknown merger number: " << mergID << " (SN: " << (unsigned)head.mergerID << "). Merger date will be skipped"); break;}

            unsigned moduleID = m_mergerMap->getModuleID(mergID, (unsigned)febHead.FEBSlot);

            if (!moduleID) { B2ERROR("ARICHUnpackerModule: no merger to FEB mapping:: " << mergID << " (SN: " << (unsigned)head.mergerID << ") FEB slot: " << (unsigned)febHead.FEBSlot << ". Merger data will be skipped"); break;}

            // read data
            if (m_debug) std::cout << "Hit channels: " << std::endl;
            if (febHead.type == 1) {
              for (int i = 0; i < dataLen / 2; i++) {
                int shift = (3 - ibyte % 4) * 8;
                uint8_t asicCh = buffer[ibyte / 4] >> shift;
                ibyte++;
                shift = (3 - ibyte % 4) * 8;
                uint8_t hitBitSet = buffer[ibyte / 4] >> shift;
                if (m_debug && hitBitSet) std::cout << "ch: " << (unsigned)asicCh << " " <<  std::bitset<8>(hitBitSet) << std::endl;
                // store digit
                digits.appendNew(moduleID, (unsigned)asicCh, hitBitSet);
                ibyte++;
              }
            } else if (febHead.type == 2) {
              unsigned asicCh = 143;
              for (int i = 0; i < dataLen; i++) {
                int shift = (3 - ibyte % 4) * 8;
                uint8_t hitBitSet = buffer[ibyte / 4] >> shift;
                // store digit if hit
                if (hitBitSet & m_bitMask) {
                  digits.appendNew(moduleID, asicCh, hitBitSet);
                }
                asicCh--;
                ibyte++;
              }
            } else B2ERROR("ARICHUnpackerModule: Unknown data type " << febHead.type);

          }

          if (ceil(ibyte / 4.) != (unsigned)bufferSize)
            B2WARNING("ARICHUnpackerModule: data buffer size mismatch (from copper vs data header: " <<  bufferSize << " vs. " <<  ceil(
                        ibyte / 4.));
        }
      } // end of rawData loop

    } // end of regular unpacker

    // RawUnpacker mode, fill ARICHRawDigit
    if (m_rawmode == 1) {
      for (auto& raw : rawData) {
        for (int finesse = 0; finesse < 4; finesse++) {
          const int* buf = (const int*)raw.GetDetectorBuffer(0, finesse);
          int bufSize = raw.GetDetectorNwords(0, finesse);
          if (bufSize < 1) continue;
          m_ibyte = 0;
          // read merger header
          int type = calbyte(buf);
          int ver = calbyte(buf);
          int boardid = calbyte(buf);
          int febno = calbyte(buf);
          unsigned int length_all = calword(buf);
          unsigned int mrg_evtno = calword(buf);
          ARICHRawDigit* rawdigit = rawdigits.appendNew(type, ver, boardid, febno, length_all, mrg_evtno);
          rawdigit->setCopperId(raw.GetNodeID(0));
          rawdigit->setHslbId(finesse);
          int nfebs = 0;
          while (m_ibyte < length_all) {
            int type_feb = calbyte(buf);
            ver = calbyte(buf);
            boardid = calbyte(buf);
            febno = calbyte(buf);
            unsigned int length = calword(buf);
            int evtno = calword(buf);
            unsigned int ibyte = 0;
            std::stringstream ss;
            ss << "type=" << type_feb << ", ver=" << ver << " "
               << ", boardid=" << boardid << ", febno=" << febno
               << ", length=" << length << ", evtno=" << evtno << " ";
            bool hasHit = false;
            long long feb_trigno = 0;
            for (int i = 0; i < 10; i++) {
              int val = calbyte(buf);
              ibyte++;
              if (i < 6) {
                feb_trigno |= (0xff & val) << (5 - i) * 8;
              }
            }
            ARICHRawDigit::FEBDigit feb;
            nfebs++;
            if (type_feb == 0x02) {//Raw mode
              int ch = 143;
              //B2INFO("raw mode");
              while (ibyte < length) {
                int val = calbyte(buf);
                if (val != 0) {
                  ibyte++;
                  ss << "ch# " << ch << "(" << val << ") ";
                  hasHit = true;
                  if (febno < 0 || febno > 6) {
                    B2ERROR("FEB is bad : " << febno << " hslb-" << finesse << ":"
                            << " type=" << type_feb << ", ver=" << ver << " "
                            << ", boardid=" << boardid << ", febno=" << febno
                            << ", length=" << length << ", evtno=" << evtno);
                  }
                  feb.push_back(ch, val);
                }
                ch--;
                if (ch < 0) break;
              }
            } else if (type_feb == 0x01) { // Suppressed mode
              if (length > 144 * 2 + 10) B2FATAL("error " << length);
              //B2INFO("suppreed mode");
              while (ibyte < length) {
                int ch = calbyte(buf);
                ibyte++;
                int val = calbyte(buf);
                ibyte++;
                if (val != 0) {
                  ss << "ch# " << ch << "(" << val << ") ";
                  hasHit = true;
                  if (febno < 0 || febno > 6) {
                    B2ERROR("FEB is bad : " << febno << " hslb-" << finesse << ":"
                            << " type=" << type_feb << ", ver=" << ver << " "
                            << ", boardid=" << boardid << ", febno=" << febno
                            << ", length=" << length << ", evtno=" << evtno);
                    return;
                  }
                  feb.push_back(ch, val);
                }
              }
            }
            rawdigit->addFEB(feb, type, ver, boardid, febno, length, evtno, feb_trigno);
            if (m_debug && hasHit) {
              B2INFO(ss.str());
            }
          }
        }
      }

    } // end of raw unpacker

    arichinfo->settrgtype(trgtype);
    arichinfo->setntrack(0);
    arichinfo->setnexthit(0);
    arichinfo->setnhit(0);

  }


  void ARICHUnpackerModule::readHeader(const int* buffer, unsigned& ibyte, ARICHRawHeader& head)
  {

    // read the first line of header
    char line1[4];
    int shift;
    for (int i = 0; i < 4; i++) {
      shift = (3 - ibyte % 4) * 8;
      line1[3 - i] = buffer[ibyte / 4] >> shift;
      ibyte++;
    }

    head.type = line1[3];
    head.version = line1[2];
    head.mergerID = line1[1];
    head.FEBSlot = line1[0];

    // data length
    char len[4];
    for (int i = 0; i < 4; i++) {
      shift = (3 - ibyte % 4) * 8;
      len[3 - i] = buffer[ibyte / 4] >> shift;
      ibyte++;
    }

    uint32_t* tmp = (uint32_t*)len;
    head.length = *tmp;

    // trigger number
    char trg[4];
    for (int i = 0; i < 4; i++) {
      shift = (3 - ibyte % 4) * 8;
      trg[3 - i] = buffer[ibyte / 4] >> shift;
      ibyte++;
    }
    tmp = (uint32_t*)trg;
    head.trigger = *tmp;

  }

  void ARICHUnpackerModule::printBits(const int* buffer, int bufferSize)
  {
    for (int i = 0; i < bufferSize; i++) {
      std::cout << i << "-th word bitset: " << std::bitset<32>(*(buffer + i)) << std::endl;
    }
  }


  void ARICHUnpackerModule::endRun()
  {
  }

  void ARICHUnpackerModule::terminate()
  {
  }


} // end Belle2 namespace

