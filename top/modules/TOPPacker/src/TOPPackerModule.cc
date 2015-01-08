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
#include <top/modules/TOPPacker/TOPPackerModule.h>

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
#include <rawdata/dataobjects/RawTOP.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPPacker)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPPackerModule::TOPPackerModule() : Module(),
    m_topgp(TOPGeometryPar::Instance())

  {
    // set module description (e.g. insert text)
    setDescription("Raw data packer for TOP");
    setPropertyFlags(c_ParallelProcessingCertified);

  }

  TOPPackerModule::~TOPPackerModule()
  {
  }

  void TOPPackerModule::initialize()
  {

    StoreArray<TOPDigit> digits;
    digits.isRequired();

    StoreArray<RawTOP> rawData;
    rawData.registerInDataStore();

    const auto& mapper = m_topgp->getFrontEndMapper();
    int mapSize = mapper.getMapSize();
    if (mapSize == 0) B2ERROR("No front-end mapping available for TOP");

  }

  void TOPPackerModule::beginRun()
  {
  }

  void TOPPackerModule::event()
  {

    StoreObjPtr<EventMetaData> evtMetaData;
    StoreArray<TOPDigit> digits;
    StoreArray<RawTOP> rawData;

    const auto& mapper = m_topgp->getFrontEndMapper();
    int mapSize = mapper.getMapSize();
    if (mapSize == 0) return;

    vector<const TOPDigit*>* sortedDigits = new vector<const TOPDigit*>[mapSize];

    for (const auto & digit : digits) {
      int barID = digit.getBarID();
      int col = digit.getHardwareChannelID() / 128;
      const auto* feemap = mapper.getMap(barID, col);
      if (!feemap) {
        B2ERROR("TOPPacker: no front-end map available for barID " << barID <<
                " column " << col);
        continue;
      }
      sortedDigits[feemap->index].push_back(&digit);
    }

    unsigned sortedDigitsMaxsize = 0;
    for (int i = 0; i < mapSize; i++) {
      unsigned size = sortedDigits[i].size();
      if (size > sortedDigitsMaxsize) sortedDigitsMaxsize = size;
    }

    int bufferDim = 1 + sortedDigitsMaxsize; // 1 header word + 1 data word per digit
    int buffer[4][bufferDim];

    for (const auto & copperID : mapper.getCopperIDs()) {
      int bufferSize[4] = {0, 0, 0, 0};
      for (int finesse = 0; finesse < 4; finesse++) {
        auto* buf = buffer[finesse];
        auto& i = bufferSize[finesse];
        const auto* feemap = mapper.getMapFromCopper(copperID, finesse);
        if (!feemap) continue;
        unsigned scrodID = feemap->scrodID;
        unsigned dataFormat = 1; // production data -> TODO: use enum
        unsigned version = 0;
        buf[i] = scrodID + (version << 16) + (dataFormat << 24);
        i++; // one header word
        for (const auto & digit : sortedDigits[feemap->index]) {
          unsigned tdc = digit->getTDC() & 0xFFFF;
          unsigned chan = digit->getHardwareChannelID() % 128;
          unsigned flags = (unsigned) digit->getHitQuality();
          buf[i] = tdc + (chan << 16) + (flags << 24);
          i++; // one data word per digit
        }
      }
      RawCOPPERPackerInfo info;
      info.exp_num = evtMetaData->getExperiment();
      info.run_subrun_num = (evtMetaData->getRun() << 8) +
                            (evtMetaData->getSubrun() & 0xFF); // run number : 14bits, subrun # : 8bits
      info.eve_num = evtMetaData->getEvent();
      info.node_id = TOP_ID + copperID;
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


  void TOPPackerModule::endRun()
  {
  }

  void TOPPackerModule::terminate()
  {
  }


} // end Belle2 namespace

