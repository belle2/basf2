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
#include <arich/modules/arichFillHits/ARICHFillHitsModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// Dataobject classes
#include <arich/dataobjects/ARICHDigit.h>
#include <arich/dataobjects/ARICHHit.h>
#include <bitset>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHFillHits)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHFillHitsModule::ARICHFillHitsModule() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Fills ARICHHits collection from ARICHDigits");
    setPropertyFlags(c_ParallelProcessingCertified);
    addParam("bitMask", m_bitMask, "hit bit mask (8 bits/channel)", (uint8_t)0xFF);
    addParam("maxApdHits" , m_maxApdHits , "Remove hits with more than MaxApdHits per APD chip", (uint8_t)18);
    addParam("maxHapdHits", m_maxHapdHits, "Remove hits with more than MaxHapdHits per HAPD", (uint8_t)100);
  }

  ARICHFillHitsModule::~ARICHFillHitsModule()
  {
  }

  void ARICHFillHitsModule::initialize()
  {

    StoreArray<ARICHDigit> digits;
    digits.isRequired();

    StoreArray<ARICHHit> arichHits;
    arichHits.registerInDataStore();

  }

  void ARICHFillHitsModule::beginRun()
  {
  }

  void ARICHFillHitsModule::event()
  {
    StoreObjPtr<EventMetaData> evtMetaData;

    StoreArray<ARICHDigit> digits;
    StoreArray<ARICHHit> arichHits;

    // calculate number of hits on each apd and on each hapd
    std::vector<uint8_t> apdHits(420 * 4, 0);
    std::vector<uint8_t> hapdHits(420, 0);
    for (const auto& digit : digits) {
      uint8_t bits = digit.getBitmap();
      if (!(bits & m_bitMask)) continue;

      int moduleID  = digit.getModuleID();
      if (moduleID > 420 || moduleID < 1) continue;
      moduleID--;
      int channelID = digit.getChannelID();
      if (channelID > 143 || channelID < 0) continue;
      int chipID    = moduleID * 4   + channelID / 36;
      apdHits[chipID]++;
      hapdHits[moduleID]++;
    }

    for (const auto& digit : digits) {
      int asicCh = digit.getChannelID();
      int modID = digit.getModuleID();
      if (modID > 420 || modID < 1) continue;
      if (asicCh > 143 || asicCh < 0) continue;
      uint8_t hitBitmap = digit.getBitmap();
      if (!(hitBitmap & m_bitMask)) continue;

      // remove hot and dead channels
      if (!m_chnMask->isActive(modID, asicCh)) continue;

      int chipID    = (modID - 1) * 4   + asicCh / 36;

      if (apdHits[chipID]   > m_maxApdHits) continue;
      if (hapdHits[modID - 1] > m_maxHapdHits) continue;


      int xCh, yCh;
      if (not m_chnMap->getXYFromAsic(asicCh, xCh, yCh)) {
        B2ERROR("Invalid ARICH hit! This hit will be ignored.");
        continue;
      }

      TVector2 hitpos = m_geoPar->getChannelPosition(modID, xCh, yCh);

      arichHits.appendNew(m_geoPar->getMasterVolume().pointToGlobal(TVector3(hitpos.X(), hitpos.Y(),
                          m_geoPar->getDetectorZPosition() + m_geoPar->getHAPDGeometry().getWinThickness())), modID, asicCh);
    }

  }


  void ARICHFillHitsModule::endRun()
  {
  }

  void ARICHFillHitsModule::terminate()
  {
  }


} // end Belle2 namespace

