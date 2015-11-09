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

    // Add parameters
    addParam("inputDigitsName", m_inputDigitsName,
             "name of TOPDigit store array", string(""));
    addParam("outputRawDataName", m_outputRawDataName,
             "name of RawTOP store array", string(""));

  }

  TOPPackerModule::~TOPPackerModule()
  {
  }

  void TOPPackerModule::initialize()
  {

    StoreArray<TOPDigit> digits(m_inputDigitsName);
    digits.isRequired();

    StoreArray<RawTOP> rawData(m_outputRawDataName);
    rawData.registerInDataStore();

    if (!m_topgp->isInitialized()) {
      GearDir content("/Detector/DetectorComponent[@name='TOP']/Content");
      m_topgp->Initialize(content);
    }
    if (!m_topgp->isInitialized()) B2ERROR("Component TOP not found in Gearbox");

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
    StoreArray<TOPDigit> digits(m_inputDigitsName);
    StoreArray<RawTOP> rawData(m_outputRawDataName);

    const auto& mapper = m_topgp->getFrontEndMapper();
    int mapSize = mapper.getMapSize();
    if (mapSize == 0) return;

    vector<const TOPDigit*>* sortedDigits = new vector<const TOPDigit*>[mapSize];

    for (const auto& digit : digits) {
      int barID = digit.getBarID();
      int boardstack = digit.getHardwareChannelID() / 128;
      const auto* feemap = mapper.getMap(barID, boardstack);
      if (!feemap) {
        B2ERROR("TOPPacker: no front-end map available for barID " << barID <<
                " boardstack# " << boardstack);
        continue;
      }
      sortedDigits[feemap->getIndex()].push_back(&digit);
    }

    for (const auto& copperID : mapper.getCopperIDs()) {
      vector<int> Buffer[4];
      for (int finesse = 0; finesse < 4; finesse++) {
        const auto* feemap = mapper.getMapFromCopper(copperID, finesse);
        if (!feemap) continue;
        unsigned scrodID = feemap->getScrodID();
        unsigned dataFormat = 1; // production data -> TODO: use enum
        unsigned version = 0;
        Buffer[finesse].push_back(scrodID + (version << 16) + (dataFormat << 24));
        for (const auto& digit : sortedDigits[feemap->getIndex()]) {
          unsigned tdc = digit->getTDC() & 0xFFFF;
          unsigned chan = digit->getHardwareChannelID() % 128;
          unsigned flags = (unsigned) digit->getHitQuality();
          Buffer[finesse].push_back(tdc + (chan << 16) + (flags << 24));
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
      raw->PackDetectorBuf(Buffer[0].data(), Buffer[0].size(),
                           Buffer[1].data(), Buffer[1].size(),
                           Buffer[2].data(), Buffer[2].size(),
                           Buffer[3].data(), Buffer[3].size(),
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

