/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/simbklm/BKLMDigitizer.h>

#include <framework/logging/Logger.h>
#include <iostream>
#include <fstream>

//#include <framework/datastore/StoreObjPtr.h>
//#include <framework/datastore/StoreArray.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

//#include <geometry/geodetector/CreatorManager.h>
//#include <geometry/geodetector/CreatorBase.h>

#include <G4DigiManager.hh>

using namespace CLHEP;

namespace Belle2 {

  //! constructor
  BKLMDigitizer::BKLMDigitizer(G4String name) : G4VDigitizerModule(name)
  {
    m_hitsName = name + "_HitsCollection";
    m_digiName = name + "_DigitsCollection";
    collectionName.push_back(m_digiName);
  }

  void BKLMDigitizer::Digitize()
  {

    B2INFO("BKLMDigitizer::Digitize called");

    BKLMSimDigiCollection* dc = new BKLMSimDigiCollection("BKLMDigitizer", m_digiName);

    G4DigiManager* digiMgr = G4DigiManager::GetDMpointer();
    G4int hcID = digiMgr->GetHitsCollectionID(m_hitsName);
    B2INFO("BKLMDigitizer::Digitize -- hcID = " << hcID);
    BKLMSimHitsCollection* hc = (BKLMSimHitsCollection*)(digiMgr->GetHitsCollection(hcID));

    BKLMSimDigi* digi;
    std::vector<int> pstrips, tstrips;
    for (std::vector<BKLMSimHit*>::const_iterator iHit = hc->GetVector()->begin();
         iHit != hc->GetVector()->end(); ++iHit) {
      G4int    be, fb, sector, layer, status;
      G4double time;
      if (hitToDigi(*iHit, be, fb, sector, layer, time, status, pstrips, tstrips)) {

        // phi-strip multiplicity loop
        int pstatus = status;
        for (std::vector<int>::iterator j = pstrips.begin(); j != pstrips.end(); j++) {
          digi = new BKLMSimDigi(be, fb, sector, layer, BKLMSimDigi::PHISTRIP, *j, time, pstatus, *iHit);
          for (std::vector<BKLMSimDigi*>::const_iterator iDigi = dc->GetVector()->begin();
               iDigi != dc->GetVector()->end(); ++iDigi) {
            if ((*iDigi)->GetBE()        != be) continue;
            if ((*iDigi)->GetFB()        != fb) continue;
            if ((*iDigi)->GetSector()    != sector) continue;
            if ((*iDigi)->GetLayer()     != layer) continue;
            if ((*iDigi)->GetDirection() != BKLMSimDigi::PHISTRIP) continue;
            if ((*iDigi)->GetStripNo()   != *j) continue;
            digi->SetFirstDigi(*iDigi);
            break;
          }
          dc->insert(digi);
          pstatus |= BKLMSimDigi::MCMULT;
        }     // end of loop for phi-strip multiplicity

        // theta-strip multiplicity loop
        int tstatus = status;
        for (std::vector<int>::iterator j = tstrips.begin(); j != tstrips.end(); j++) {
          digi = new BKLMSimDigi(be, fb, sector, layer, BKLMSimDigi::THETASTRIP, *j, time, tstatus, *iHit);
          for (std::vector<BKLMSimDigi*>::const_iterator iDigi = dc->GetVector()->begin();
               iDigi != dc->GetVector()->end(); ++iDigi) {
            if ((*iDigi)->GetBE()        != be) continue;
            if ((*iDigi)->GetFB()        != fb) continue;
            if ((*iDigi)->GetSector()    != sector) continue;
            if ((*iDigi)->GetLayer()     != layer) continue;
            if ((*iDigi)->GetDirection() != BKLMSimDigi::THETASTRIP) continue;
            if ((*iDigi)->GetStripNo()   != *j) continue;
            digi->SetFirstDigi(*iDigi);
            break;
          }
          dc->insert(digi);
          tstatus |= BKLMSimDigi::MCMULT;
        }     // end of loop for theta-strip multiplicity

      }       // if ( hitToDigi )

    } // hit loop

    StoreDigiCollection(dc);

  }

  G4bool BKLMDigitizer::Store()
  {

    B2INFO("BKLMDigitizer::Store called");

    G4DigiManager* digiMgr = G4DigiManager::GetDMpointer();

    // store BKLMSimHits
    G4int hcID = digiMgr->GetHitsCollectionID(m_hitsName);
    B2INFO("BKLMDigitizer::Store -- hcID = " << hcID);
    BKLMSimHitsCollection* hc = (BKLMSimHitsCollection*)(digiMgr->GetHitsCollection(hcID));
    if (hc) {
      for (std::vector<BKLMSimHit*>::const_iterator it = hc->GetVector()->begin();
           it != hc->GetVector()->end(); ++it) {
        //(*it)->Store();
      }
    }

    // store BKLMSimDigis
    G4int dcID = digiMgr->GetDigiCollectionID(m_digiName);
    B2INFO("BKLMDigitizer::Store -- dcID = " << dcID);
    BKLMSimDigiCollection* dc = (BKLMSimDigiCollection*)(digiMgr->GetDigiCollection(dcID));
    if (dc) {
      for (std::vector<BKLMSimDigi*>::const_iterator it = dc->GetVector()->begin();
           it != dc->GetVector()->end(); ++it) {
        //(*it)->Store();
      }
    }

    return true;

  }

  G4bool BKLMDigitizer::hitToDigi(const BKLMSimHit* hit, G4int& be, G4int& fb,
                                  G4int& sector, G4int& layer, G4double& time, G4int& status,
                                  std::vector<int>& pstrips, std::vector<int>& tstrips)
  {

    G4bool found = false;

    if (hit->getInRPCGas()) {

      //Belle2::BKLM_McEffFilter& efficiency   = Belle2::BKLM_McEffFilter::instance();
      //Belle2::BKLM_McMultiHit&  multiplicity = Belle2::BKLM_McMultiHit::instance();

      pstrips.clear();
      tstrips.clear();

      time   = hit->getHitTime() / Unit::ns;
      status = (time <= 32000.0 ? 0 : BKLMSimDigi::OOTIME);

      const G4ThreeVector pos(hit->getHitPos().x() / Unit::cm,
                              hit->getHitPos().y() / Unit::cm,
                              hit->getHitPos().z() / Unit::cm);

      /*
      const Belle::KlmGeo& klmgeo = Belle::KlmGeo::instance();

      G4int  pstrip, tstrip;
      for( std::vector<KlmSector*>::const_iterator s = klmgeo.sectors().begin();
           s != klmgeo.sectors().end() && !found; s++ ){
        for( std::vector<KlmModule*>::const_iterator m = (*s)->modules().begin();
             m != (*s)->modules().end() && !found; m++ ){
          if ( (*m)->strips( (*s)->globalToLocal( pos ), pstrip, tstrip ) ){
            found = true;
            be = ((*m)->barrelEnd() == 'B' ? BKLMSimDigi::BARREL : BKLMSimDigi::ENDCAP);
            fb = ((*m)->frontBack() == 'F' ? BKLMSimDigi::FORWARD : BKLMSimDigi::BACKWARD);
            sector = (*m)->sector();
            layer = (*m)->layer();
            pstrips.push_back( pstrip );
            tstrips.push_back( tstrip );
            status |= efficiency.doit( (*m)->barrelEnd(), (*m)->frontBack(),
                                       (*m)->sector(),    (*m)->layer(),
                                       pstrip,            tstrip );
            double pdiv, tdiv;
            (*m)->stripDivisions( (*s)->globalToLocal( pos ), pdiv, tdiv );
            multiplicity.doit( be, fb, sector, layer, pdiv, tdiv, pstrips, tstrips );
          }
        }
      }
      */

    } else {

      // hit is in a BKLM scintillator

    }

    return found;

  }

  /*
  void BKLMDigitizer::saveStripHits()
  {
    for (std::vector<BKLMStripHit*>::const_iterator iter = m_HitVector.begin();
         iter != m_HitVector.end(); ++iter)
      storeBKLMObject("StripHitsBKLMArray", *iter);
  }
  */

}

