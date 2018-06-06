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
  }

  ARICHFillHitsModule::~ARICHFillHitsModule()
  {
  }

  void ARICHFillHitsModule::initialize()
  {

    StoreArray<ARICHDigit> digits;
    digits.isRequired();

    StoreObjPtr<ARICHChannelMask> eventmask;
    eventmask.isOptional();

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
    StoreObjPtr<ARICHChannelMask> eventmask;

    for (const auto& digit : digits) {
      int asicCh = digit.getChannelID();
      int modID = digit.getModuleID();
      uint8_t hitBitmap = digit.getBitmap();
      if (!(hitBitmap & m_bitMask)) continue;

      // remove hot and dead channels
      if (!m_chnMask->isActive(modID, asicCh)) continue;
      if (eventmask) {
        if (!eventmask->isActive(modID, asicCh)) continue;
      }


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

