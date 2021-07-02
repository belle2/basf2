/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <arich/modules/arichFillHits/ARICHFillHitsModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
// framework aux
#include <framework/logging/Logger.h>

// Dataobject classes
#include <arich/dataobjects/ARICHDigit.h>
#include <arich/dataobjects/ARICHHit.h>
// magnetic field manager
#include <framework/geometry/BFieldManager.h>

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
    addParam("MagFieldCorrection", m_bcorrect, "Apply hit position correction due to non-perp. mag. field", 0);
    addParam("fillAll", m_fillall, "Make hits for all active channels (useful for likelihood PDF studies)", 0);
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

    arichHits.registerRelationTo(digits);
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

    if (!m_fillall) {
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

        TVector2 hitpos2D = m_geoPar->getChannelPosition(modID, xCh, yCh);
        TVector3 hitpos3D(hitpos2D.X(), hitpos2D.Y(), m_geoPar->getDetectorZPosition() + m_geoPar->getHAPDGeometry().getWinThickness());
        hitpos3D = m_geoPar->getMasterVolume().pointToGlobal(hitpos3D);

        if (m_bcorrect) magFieldCorrection(hitpos3D);
        ARICHHit* hh = arichHits.appendNew(hitpos3D, modID, asicCh);
        hh->addRelationTo(&digit);
      }
    } else {
      for (int imod = 1; imod < 421; imod++) {
        for (int ichn = 0; ichn < 144; ichn++) {

          // remove hot and dead channels
          if (!m_chnMask->isActive(imod, ichn)) continue;
          int chipID    = (imod - 1) * 4   + ichn / 36;

          if (apdHits[chipID]   > m_maxApdHits) continue;
          if (hapdHits[imod - 1] > m_maxHapdHits) continue;

          int xCh, yCh;
          if (not m_chnMap->getXYFromAsic(ichn, xCh, yCh)) {
            B2ERROR("Invalid ARICH hit! This hit will be ignored.");
            continue;
          }

          TVector2 hitpos2D = m_geoPar->getChannelPosition(imod, xCh, yCh);
          TVector3 hitpos3D(hitpos2D.X(), hitpos2D.Y(), m_geoPar->getDetectorZPosition() + m_geoPar->getHAPDGeometry().getWinThickness());
          hitpos3D = m_geoPar->getMasterVolume().pointToGlobal(hitpos3D);
          if (m_bcorrect) magFieldCorrection(hitpos3D);
          arichHits.appendNew(hitpos3D, imod, ichn);
        }
      }
    }

  }

  void ARICHFillHitsModule::magFieldCorrection(TVector3& hitpos)
  {
    TVector3 Bfield = BFieldManager::getField(hitpos);
    TVector3 shift = m_geoPar->getHAPDGeometry().getPhotocathodeApdDistance() / abs(Bfield.Z()) * Bfield;
    hitpos.SetX(hitpos.X() - shift.X());
    hitpos.SetY(hitpos.Y() - shift.Y());
  }

  void ARICHFillHitsModule::endRun()
  {
  }

  void ARICHFillHitsModule::terminate()
  {
  }


} // end Belle2 namespace

