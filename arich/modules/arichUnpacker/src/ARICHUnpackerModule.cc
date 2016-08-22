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

// print bitset
#include <bitset>

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

  ARICHUnpackerModule::ARICHUnpackerModule() : Module(), m_bitMask(0), m_debug(0),
    m_arichgp(ARICHGeometryPar::Instance())

  {
    // set module description
    setDescription("Raw data unpacker for ARICH");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("bitMask", m_bitMask, "hit bit mask (4 bits/channel)", (uint8_t)0xF);
    addParam("debug", m_debug, "print data words bitmap", 0);
    addParam("inputRawDataName", m_inputRawDataName, "name of RawARICH store array", string(""));
    addParam("outputDigitsName", m_outputDigitsName, "name of ARICHDigit store array", string(""));
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

    if (!m_arichgp->isInit()) {
      GearDir content("/Detector/DetectorComponent[@name='ARICH']/Content");
      m_arichgp->Initialize(content);
    }
    if (!m_arichgp->isInit()) B2ERROR("Component ARICH not found in Gearbox");

  }

  void ARICHUnpackerModule::beginRun()
  {
  }

  void ARICHUnpackerModule::event()
  {

    StoreArray<RawARICH> rawData(m_inputRawDataName);
    StoreArray<ARICHDigit> digits(m_outputDigitsName);
    digits.clear();
    StoreObjPtr<EventMetaData> evtMetaData;
    if (m_debug) {
      std::cout << std::endl << "------------------------" << std::endl;
      std::cout << "Run: " << evtMetaData->getRun() << " Event: " << evtMetaData->getEvent()  << std::endl;
      std::cout << "------------------------" << std::endl << std::endl;
    }


    for (auto& raw : rawData) {
      for (int finesse = 0; finesse < 4; finesse++) {
        const int* buffer = raw.GetDetectorBuffer(0, finesse);
        int bufferSize = raw.GetDetectorNwords(0, finesse);

        if (bufferSize < 1)
          continue;

        // read merger header
        unsigned ibyte = 0;
        ARICHRawHeader head;

        readHeader(buffer, ibyte, head);

        if (m_debug) {
          std::cout << "Merger header" << std::endl;
          head.print();
          if (m_debug > 1) printBits(buffer, bufferSize);
        }

        switch (head.type) {
          case 1:
            unpackSuppressedData(buffer, bufferSize, ibyte, head);
            break;
          case 2:
            unpackUnsuppressedData(buffer, bufferSize, ibyte, head);
            break;
          default:
            B2ERROR("ARICHUnpacker: unknown data format type = " << head.type);
        }
      }
    }
  }

  void ARICHUnpackerModule::unpackUnsuppressedData(const int* buffer, int bufferSize, unsigned& ibyte, ARICHRawHeader& head)
  {

    StoreArray<ARICHDigit> digits(m_outputDigitsName);

    while (ibyte < head.length) {

      // new feb
      ARICHRawHeader febHead;
      readHeader(buffer, ibyte, febHead);

      // feb header shift
      ibyte += ARICHFEB_HEADER_SIZE;
      int dataLen = febHead.length - ARICHFEB_HEADER_SIZE;

      if (m_debug) {
        std::cout << std::endl << "FEB header" << std::endl;
        febHead.print();
      }

      // get module ID from mapper
      unsigned moduleID = m_arichgp->getBoardFromMerger((unsigned)head.mergerID, (unsigned)febHead.FEBSlot);

      if (!moduleID) { B2ERROR("ARICHUnpackerModule: no merger 2 module mapping for mergerID " << (unsigned)head.mergerID << " FEB slot: " << (unsigned)febHead.FEBSlot); break;}
      if (m_debug) std::cout << "Hit channels: " << std::endl;
      // read data
      unsigned asicCh = 143;
      for (int i = 0; i < dataLen; i++) {
        int shift = (3 - ibyte % 4) * 8;
        uint8_t hitBitSet = buffer[ibyte / 4] >> shift;
        // store digit if hit
        if (hitBitSet & m_bitMask) {
          digits.appendNew(moduleID, asicCh, hitBitSet);
          if (m_debug && hitBitSet) std::cout << "ch: " << asicCh << " " <<  std::bitset<8>(hitBitSet) << std::endl;
        }
        asicCh--;
        ibyte++;
      }

    }

    if (ceil(ibyte / 4.) != (unsigned)bufferSize)
      B2WARNING("ARICHUnpackerModule: data buffer size mismatch (from copper vs data header: " <<  bufferSize << " vs. " <<  ceil(
                  ibyte / 4.));

  }


  void ARICHUnpackerModule::unpackSuppressedData(const int* buffer, int bufferSize, unsigned& ibyte, ARICHRawHeader& head)
  {

    StoreArray<ARICHDigit> digits(m_outputDigitsName);

    while (ibyte < head.length) {
      // new feb
      ARICHRawHeader febHead;
      readHeader(buffer, ibyte, febHead);
      if (m_debug) febHead.print();
      // feb header shift
      ibyte += ARICHFEB_HEADER_SIZE;
      int dataLen = febHead.length - ARICHFEB_HEADER_SIZE;

      unsigned moduleID =  m_arichgp->getBoardFromMerger((unsigned)head.mergerID, (unsigned)febHead.FEBSlot);

      if (!moduleID) { B2ERROR("ARICHUnpackerModule: no merger 2 module mapping for mergerID " << (unsigned)head.mergerID << " FEB slot: " << (unsigned)febHead.FEBSlot); break;}

      // read data

      for (int i = 0; i < dataLen / 2; i++) {
        int shift = (3 - ibyte % 4) * 8;
        uint8_t asicCh = buffer[ibyte / 4] >> shift;
        ibyte++;
        shift = (3 - ibyte % 4) * 8;
        uint8_t hitBitSet = buffer[ibyte / 4] >> shift;

        // store digit
        digits.appendNew(moduleID, (unsigned)asicCh, hitBitSet);
        ibyte++;
      }

    }

    if (ceil(ibyte / 4.) != (unsigned)bufferSize)
      B2WARNING("ARICHUnpackerModule: data buffer size mismatch (from copper vs data header): " <<
                (unsigned)bufferSize << " vs. " << ceil(ibyte / 4.));

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

