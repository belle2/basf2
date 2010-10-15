/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/simeklm/EKLMDigitizer.h>
#include <eklm/eklmutils/EKLMutils.h>
#include <framework/logging/Logger.h>
#include<iostream>
#include<fstream>

#include <G4Navigator.hh>
#include<G4VisAttributes.hh>

#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>


#include "G4TransportationManager.hh"

#include<geometry/geodetector/CreatorManager.h>
#include<geometry/geodetector/CreatorBase.h>

#include<eklm/geoeklm/GeoEKLMBelleII.h>

#include "boost/lexical_cast.hpp"

namespace Belle2 {

  G4Allocator<EKLMDigitizer> EKLMDigitizerAllocator;


  void EKLMDigitizer::getSimHits()
  {
    for (std::vector<EKLMSimHit*>::iterator iHit = m_HitCollection->GetVector()->begin();
         iHit != m_HitCollection->GetVector()->end(); ++iHit) {


      // initialize G4 Navigator to find the name of the Strip
      G4Navigator* theNavigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
      //get strip name
      G4String StripName = theNavigator->LocateGlobalPointAndSetup((*iHit)->getPos())->GetName();

      // search for entries of the same strip
      std::map<G4String, std::vector<EKLMSimHit*> >::iterator it = m_HitStripMap.find(StripName);

      if (it == m_HitStripMap.end()) { //  new entry
        std::vector<EKLMSimHit*> *vectorHits = new std::vector<EKLMSimHit*> (1, *iHit);
        m_HitStripMap.insert(std::pair<G4String, std::vector<EKLMSimHit*> >(StripName, *vectorHits));
      } else {
        it->second.push_back(*iHit);
      }

    }
  }
  //!  This function is intended to form StripHits from SimHits.
  void EKLMDigitizer::mergeSimHitsToStripHits()
  {
    for (std::map<G4String, std::vector<EKLMSimHit*> >::iterator it = m_HitStripMap.begin();
         it != m_HitStripMap.end(); it++) {
      std::vector<EKLMSimHit*> vectorHits = it->second;

      G4ThreeVector hitPos(0, 0, 0);
      G4double hitE = 0;
      G4double hitTfirst = 100000;  // time of the first hit
      G4int leadingParticlePDG = 0;
      for (std::vector<EKLMSimHit*> ::iterator i = vectorHits.begin(); i != vectorHits.end(); i++) {
        hitE += (*i)->getEDep();
        hitPos += (*i)->getPos() * (*i)->getEDep();
        if (hitTfirst > (*i)->getTime()) {
          hitTfirst = (*i)->getTime();
          leadingParticlePDG = (*i)->getPDGCode();
        }
      }
      hitPos = hitPos / hitE;

      EKLMStripHit *stripHit = new EKLMStripHit();
      /*
      CreatorManager & CreatorMgr=CreatorManager::Instance();
      // potentially dangerous! will crash if EKLM creator name will be changed!
      TGeoVolume* topEKLM=dynamic_cast<GeoEKLMBelleII*>(&CreatorMgr.getCreator("EKLMBelleII"))->getTopmostVolume();
      topEKLM->PrintNodes();
      exit(0);
      //      TGeoVolume *strip= topEKLM->FindNode(it->first)->GetVolume();
      //      std::cout<<strip->GetName()<<std::endl;
      */

      //std::cout<<gGeoManager->GetCurrentNode()->GetVolume()->GetName()<<std::endl;
      //      std::cout<<gGeoManager->FindNode(hitPos.x()/mm,hitPos.y()/mm,hitPos.z()/mm)<<std::endl;;
      std::cout << gGeoManager->GetPath() << " " << gGeoManager->GetCurrentNode()->GetVolume()->GetName() << std::endl;

      double globalPos[] = {hitPos.x() / mm, hitPos.y() / mm, hitPos.z() / mm};
      double localPos[3];
      gGeoManager->MasterToLocal(globalPos, localPos);
      std::cout << hitPos << " --->  (" << localPos[0] << "," << localPos[1] << "," << localPos[2] << ")" << std::endl;



      stripHit->setName(it->first);
      stripHit->setNumberPhotoElectrons(energyToPhotoElectrons(hitE, hitPos));
      stripHit->setTime(lightPropagationTime(hitTfirst, hitPos));
      stripHit->setLeadingParticlePDGCode(leadingParticlePDG);
      m_HitVector.push_back(stripHit);
    }
  }

  void EKLMDigitizer::saveStripHits()
  {
    for (std::vector<EKLMStripHit*>::const_iterator iter = m_HitVector.begin();
         iter != m_HitVector.end(); ++iter)
      storeEKLMObject("StipHitsEKLMArray", *iter);
  }

  G4double EKLMDigitizer::energyToPhotoElectrons(G4double energy , G4ThreeVector pos)
  {

    // not implemented
    return energy;
  }
  G4double EKLMDigitizer::lightPropagationTime(G4double time, G4ThreeVector pos)
  {

    //not implemented
    return time;
  }


}

