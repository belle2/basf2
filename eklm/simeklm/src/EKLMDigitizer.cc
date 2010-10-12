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
#include <framework/logging/Logger.h>
#include<iostream>
#include<fstream>

#include <G4Navigator.hh>
#include<G4VisAttributes.hh>

#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>



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


      // search for etries of the same strip
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
      stripHit->setName(it->first);
      stripHit->setNumberPhotoElectrons(energyToPhotoElectrons(hitE, hitPos));
      stripHit->setTime(lightPropagationTime(hitTfirst, hitPos));
      stripHit->setLeadingParticlePDGCode(leadingParticlePDG);
      m_HitVector.push_back(stripHit);
    }
  }

  void EKLMDigitizer::saveStripHits()
  {
    int iHit = 0;

    StoreArray<EKLMStripHit> eklmStripHitArray("StipHitsEKLMArray");

    for (std::vector<EKLMStripHit*>::const_iterator iter = m_HitVector.begin(); iter != m_HitVector.end(); ++iter)
      new(eklmStripHitArray->AddrAt(iHit++)) EKLMStripHit(**iter);
    for (std::vector<EKLMStripHit*>::const_iterator iter = m_HitVector.begin(); iter != m_HitVector.end(); ++iter)
      std::cout << (*iter)->getName() << " " << (*iter)->getTime() << " " << (*iter)->getNumberPhotoElectrons() << " " <<
                (*iter)->getLeadingParticlePDGCode() << std::endl;
  }

  G4double EKLMDigitizer::energyToPhotoElectrons(G4double energy , G4ThreeVector pos)
  {
    return energy;
  }
  G4double EKLMDigitizer::lightPropagationTime(G4double time, G4ThreeVector pos)
  {
    return time;
  }


}

