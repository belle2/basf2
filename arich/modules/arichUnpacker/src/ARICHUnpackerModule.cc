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

    addParam("bitMask", m_bitMask, "hit bit mask (4 bits/channel)", (unsigned)0xF);
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


    for (auto& raw : rawData) {
      for (int finesse = 0; finesse < 4; finesse++) {
        const int* buffer = raw.GetDetectorBuffer(0, finesse);
        int bufferSize = raw.GetDetectorNwords(0, finesse);

        if (bufferSize < 1)
          continue;

        unsigned dataFormat = (buffer[0] >> 24) & 0xFF;

        switch (dataFormat) {
          case 1:
            unpackSuppressedData(buffer, bufferSize);
            break;
          case 2:
            unpackUnsuppressedData(buffer, bufferSize);
            break;
          default:
            B2ERROR("ARICHUnpacker: unknown data format type = " << dataFormat);
        }
      }
    }
  }

  void ARICHUnpackerModule::unpackUnsuppressedData(const int* buffer, int bufferSize)
  {

    StoreArray<ARICHDigit> digits(m_outputDigitsName);

    unsigned mergerID = (buffer[0] >> 8) & 0xFF;

    unsigned version = (buffer[0] >> 16) & 0xFF;

    switch (version) {
      case 0:
        for (int i = 1; i < bufferSize;) {
          unsigned boardN = buffer[i]  & 0xFF;
          int boardID = m_arichgp->getBoardFromMerger(mergerID, boardN);
          i += 2;
          for (int j = 0; j < 144; j++) {
            unsigned chan = buffer[i] & (m_bitMask << (j % 8) * 4);
            uint8_t bitmap = (buffer[i] >> (j % 8) * 4) & 0xF;
            if (chan) digits.appendNew(boardID, j, bitmap);
            if ((j + 1) % 8 == 0) i++;
          }
        }
        break;

      default:
        B2ERROR("ARICHUnpacker::unpackData: unknown data format version = " << version);
    }
  }


  void ARICHUnpackerModule::unpackSuppressedData(const int* buffer, int bufferSize)
  {

    StoreArray<ARICHDigit> digits(m_outputDigitsName);

    unsigned short mergerID = (buffer[0] >> 8) & 0xFF;
    unsigned version = (buffer[0] >> 16) & 0xFF;

    switch (version) {
      case 0:
        for (int i = 1; i < bufferSize;) {
          unsigned boardN = buffer[i]  & 0xFF;
          int boardID = m_arichgp->getBoardFromMerger(mergerID, boardN);
          i++;
          int nhits = buffer[i] / 2;
          i++;

          // if the FEB is not assigned, skip its data
          if (boardID < 0) {i += ceil(double(nhits) / 2.); continue;}

          for (int j = 1; j < nhits + 1; j++) {
            unsigned word = 0;
            if (j % 2) word = (buffer[i] >> 16);
            else {word = buffer[i]; i++;}
            unsigned is_hit = word & m_bitMask;
            unsigned chn = (word >> 8) & (0xFF);
            uint8_t bitmap = word & (0xF);
            if (is_hit) digits.appendNew(boardID, chn, bitmap);
          }
          if (nhits % 2) i++;
        }
        break;

      default:
        B2ERROR("ARICHUnpacker::unpackData: unknown data format version = " << version);
    }

  }


  void ARICHUnpackerModule::endRun()
  {
  }

  void ARICHUnpackerModule::terminate()
  {
  }


} // end Belle2 namespace

