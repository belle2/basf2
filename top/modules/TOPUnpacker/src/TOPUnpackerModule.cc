/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPUnpacker/TOPUnpackerModule.h>

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
#include <top/dataobjects/TOPDigit.h>
#include <rawdata/dataobjects/RawBPID.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPUnpacker)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPUnpackerModule::TOPUnpackerModule() : Module(),
    m_topgp(TOPGeometryPar::Instance())

  {
    // set module description
    setDescription("Raw data unpacker for TOP");
    setPropertyFlags(c_ParallelProcessingCertified);

  }

  TOPUnpackerModule::~TOPUnpackerModule()
  {
  }

  void TOPUnpackerModule::initialize()
  {

    StoreArray<RawBPID> rawData;
    rawData.isRequired();

    StoreArray<TOPDigit> digits;
    digits.registerInDataStore();

    const auto& mapper = m_topgp->getFrontEndMapper();
    int mapSize = mapper.getMapSize();
    if (mapSize == 0) B2ERROR("No front-end mapping available for TOP");

  }

  void TOPUnpackerModule::beginRun()
  {
  }

  void TOPUnpackerModule::event()
  {

    StoreArray<RawBPID> rawData;
    StoreArray<TOPDigit> digits;
    digits.clear();

    for (auto & raw : rawData) {
      for (int finesse = 0; finesse < 4; finesse++) {
        const int* buffer = raw.GetDetectorBuffer(0, finesse);
        int bufferSize = raw.GetDetectorNwords(0, finesse);
        if (bufferSize < 1)
          continue;
        unsigned dataFormat = (buffer[0] >> 24) & 0xFF;
        switch (dataFormat) {
          case 1:
            unpackProductionFormat(buffer, bufferSize);
            break;
          default:
            B2ERROR("TOPUnpacker: unknown data format type = " << dataFormat);
        }
      }
    }

  }


  void TOPUnpackerModule::unpackProductionFormat(const int* buffer, int bufferSize)
  {

    StoreArray<TOPDigit> digits;

    unsigned short scrodID = buffer[0] & 0xFFFF;
    const auto* feemap = m_topgp->getFrontEndMapper().getMap(scrodID);
    if (!feemap) {
      B2ERROR("TOPUnpacker: no front-end map available for SCROD ID = " << scrodID);
      return;
    }
    int barID = feemap->barID;
    int column = feemap->column;

    unsigned version = (buffer[0] >> 16) & 0xFF;
    switch (version) {
      case 0:
        for (int i = 1; i < bufferSize; i++) {
          int word = buffer[i];
          int tdc = word & 0xFFFF;
          unsigned chan = ((word >> 16) & 0x7F) + column * 128;
          unsigned flags = (word >> 24) & 0xFF;
          int channelID = m_topgp->getChannelID(chan);
          auto* digit = digits.appendNew(barID, channelID, tdc);
          digit->setHardwareChannelID(chan);
          digit->setHitQuality((TOPDigit::EHitQuality) flags);
        }
        break;
      default:
        B2ERROR("TOPUnpacker: unknown data format version = " << version);
    }

  }



  void TOPUnpackerModule::endRun()
  {
  }

  void TOPUnpackerModule::terminate()
  {
  }


} // end Belle2 namespace

