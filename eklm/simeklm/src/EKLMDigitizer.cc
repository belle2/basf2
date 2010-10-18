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

#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>


#include<geometry/geodetector/CreatorManager.h>
#include<geometry/geodetector/CreatorBase.h>

#include<eklm/geoeklm/GeoEKLMBelleII.h>

#include "boost/lexical_cast.hpp"


using namespace CLHEP;

namespace Belle2 {

  G4Allocator<EKLMDigitizer> EKLMDigitizerAllocator;


  void EKLMDigitizer::getSimHits()
  {
    for (std::vector<EKLMSimHit*>::iterator iHit = m_HitCollection->GetVector()->begin();
         iHit != m_HitCollection->GetVector()->end(); ++iHit) {

      gGeoManager->SetCurrentPoint((*iHit)->getPos().x(), (*iHit)->getPos().y(), (*iHit)->getPos().z());
      gGeoManager->FindNode();
      std::string StripName = gGeoManager->GetCurrentVolume()->GetName();


      // search for entries of the same strip
      std::map<std::string, std::vector<EKLMSimHit*> >::iterator it = m_HitStripMap.find(StripName);

      if (it == m_HitStripMap.end()) { //  new entry
        std::vector<EKLMSimHit*> *vectorHits = new std::vector<EKLMSimHit*> (1, *iHit);
        m_HitStripMap.insert(std::pair<std::string, std::vector<EKLMSimHit*> >(StripName, *vectorHits));
      } else {
        it->second.push_back(*iHit);
      }

    }
  }
  //!  This function is intended to form StripHits from SimHits.
  void EKLMDigitizer::mergeSimHitsToStripHits()
  {
    DEBUG(1, "STRAT MERGING HITS");
    for (std::map<std::string, std::vector<EKLMSimHit*> >::iterator it = m_HitStripMap.begin();
         it != m_HitStripMap.end(); it++) {
      std::vector<EKLMSimHit*> vectorHits = it->second;

      Hep3Vector hitPos(0, 0, 0);
      double hitE = 0;
      double hitTfirst = 100000;  // time of the first hit
      int leadingParticlePDG = 0;
      for (std::vector<EKLMSimHit*> ::iterator i = vectorHits.begin(); i != vectorHits.end(); i++) {
        hitE += (*i)->getEDep();
        hitPos += (*i)->getPos() * (*i)->getEDep();
        if (hitTfirst > (*i)->getTime()) {
          hitTfirst = (*i)->getTime();
          leadingParticlePDG = (*i)->getPDGCode();
        }
      }
      hitPos = hitPos / hitE;



      double firstHitDist = 0;
      double secondHitDist = 0;
      lightPropagationDistance(firstHitDist, secondHitDist, hitPos);
      gGeoManager->GetCurrentVolume()->SetVisibility(true);  // potentially dangerous!   use only after EKLMDigitizer::lightPropagationDistance


      std::cout << it->first << std::endl;


      // Create hit
      EKLMStripHit *stripHit = new EKLMStripHit();
      stripHit->setName(it->first);
      stripHit->setNumberPhotoElectrons(energyToPhotoElectrons(hitE, firstHitDist));
      stripHit->setTime(lightPropagationTime(firstHitDist));
      stripHit->setLeadingParticlePDGCode(leadingParticlePDG);
      m_HitVector.push_back(stripHit);


      // Create mirrored hit
      /*
      EKLMStripHit *mirroredStripHit = new EKLMStripHit();
      mirroredStripHit->setName(it->first);
      mirroredStripHit->setNumberPhotoElectrons(energyToPhotoElectrons(hitE, secondHitDist,true));
      mirroredStripHit->setTime(lightPropagationTime(secondHitDist));
      mirroredStripHit->setLeadingParticlePDGCode(leadingParticlePDG);
      m_HitVector.push_back(mirroredStripHit);
      */
    }
  }

  void EKLMDigitizer::saveStripHits()
  {
    for (std::vector<EKLMStripHit*>::const_iterator iter = m_HitVector.begin();
         iter != m_HitVector.end(); ++iter)
      storeEKLMObject("StripHitsEKLMArray", *iter);
  }

  int EKLMDigitizer::energyToPhotoElectrons(double energy , double dist, bool isMirrored)
  {

    // nearly arbitrary function. to be updated
    return energy*20;
  }
  double EKLMDigitizer::lightPropagationTime(double L)
  {
    // nearly arbitrary function. to be updated
    double speed = 17;// (cm/ns)   // should be accessible via xml!
    return L*speed;
  }

  void EKLMDigitizer::lightPropagationDistance(double &firstHitDist, double &secondHitDist, Hep3Vector pos)
  {
    gGeoManager->SetCurrentPoint(pos.x(), pos.y(), pos.z());
    gGeoManager->FindNode();
    double globalPos[] = {pos.x(), pos.y(), pos.z()};
    double localPos[3];
    gGeoManager->MasterToLocal(globalPos, localPos);  // coordinates in the strip frame
    double xmin = 0; // half of the strip length
    double xmax = 0; // half of the strip length
    gGeoManager->GetCurrentVolume()->GetShape()->GetAxisRange(1, xmin, xmax);  // set strip length
    firstHitDist = xmax - localPos[0];     //  direct light hit
    secondHitDist = 4 * xmax - firstHitDist;     //  reflected light hit
  }


}

