/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/simeklm/EKLMDigitizer.h>
#include <eklm/simeklm/EKLMFiberAndElectronics.h>

#include <framework/logging/Logger.h>

#include "G4VPhysicalVolume.hh"
#include <eklm/geoeklm/G4PVPlacementGT.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>


#include "G4Box.hh"
#include <framework/gearbox/Unit.h>


using namespace CLHEP;
using namespace std;

namespace Belle2 {



  void EKLMDigitizer::readAndSortStepHits()
  {
    B2DEBUG(1, "EKLMDigitizer::readAndSortStepHits()");

    StoreArray<EKLMStepHit> stepHitsArray;
    for (int i = 0; i < stepHitsArray.getEntries(); i++) {

      // search for entries of the same strip
      map<const G4VPhysicalVolume*, vector<EKLMStepHit*> >::iterator
      it = m_stepHitVolumeMap.find((stepHitsArray[i])->getVolume());

      if (it == m_stepHitVolumeMap.end()) { //  new entry
        vector<EKLMStepHit*> *vectorHits =
          new vector<EKLMStepHit*> (1, (stepHitsArray[i]));

        m_stepHitVolumeMap.insert(pair<const G4VPhysicalVolume*, vector<EKLMStepHit*> >((stepHitsArray[i])->getVolume(), *vectorHits));
      } else {
        it->second.push_back(stepHitsArray[i]);
      }
    }
    B2DEBUG(1, "EKLMDigitizer::readAndSortStepHits()  completed");
  }


  void EKLMDigitizer::makeSimHits()
  {
    using namespace boost;
    B2DEBUG(1, "EKLMDigitizer::makeSimHits()");




    //loop over volumes
    for (map<const G4VPhysicalVolume*, vector<EKLMStepHit*> >::iterator
         volumeIterator = m_stepHitVolumeMap.begin();
         volumeIterator != m_stepHitVolumeMap.end(); volumeIterator++) {

      // we have only tree graphs here, so edge is completely defined by it's track ID
      // map to store (edge  <--> hit) == (vertex <--> hit) correspondence
      map < int , EKLMStepHit*> hitMap;

      for (vector<EKLMStepHit*>::iterator i = volumeIterator->second.begin();
           i != volumeIterator->second.end(); i++) {
        // ID key
        int key = (*i)->getTrackID();

        // here we merge all StepHits produced by the same track.
        //The leading (with smallest time) hit survies.
        //The others are deleted.
        //Integrated energy deposition is prescribed to the leading hit
        //this procedure removes multiple maps entries  with identical keys

        // search if entry already exist
        map < int , EKLMStepHit*>::iterator mapIterator = hitMap.find(key);
        if (mapIterator == hitMap.end()) { // nothing found
          // add an entry
          hitMap.insert(pair< int , EKLMStepHit*>(key, *i));
        } else { // entry already exists
          // get  time of the entries
          double oldTime = mapIterator->second->getTime();
          double newTime = (*i)->getTime();
          if (newTime > oldTime) { // new hit is newer
            //add edep of the new hit to the old one
            mapIterator->second->increaseEDep((*i)->getEDep());
          } else { // new hit is older
            // add edep of the old hit to the new one
            (*i)->increaseEDep(mapIterator->second->getEDep());
            //change second element of the pair to point to the new hit
            mapIterator->second = *i;
          }
        }
      }

      //  Now we have map with key of the track ID
      // we are ready to create the graph

      // since vertex numbering always starts from 0,
      // we will use NUMBER OF TRACK IN THE MAP (started from 0)
      // istead of Track ID itself (which could be quite large).
      // this procedure drastically decreases number of graph vertices

      adjacency_list <> G;

      // we here restore the tree of hits using TrackID and ParentTrackID information
      // There is a possibility for two hit trees wich are connected by parentTrackID outside the volume
      // use special ParentTrackID for the tracks coming from abroad  remains these trees splittable
      int non_exsisting_rtracks_counter = hitMap.size();

      for (map < int , EKLMStepHit*>::iterator hitIterator = hitMap.begin();
           hitIterator != hitMap.end(); hitIterator++) {

        //search for parent entry in the map
        map < int , EKLMStepHit*>::iterator parentIterator =
          hitMap.find(hitIterator->second->getParentTrackID());

        if (parentIterator != hitMap.end()) { // nothing found
          // see comments above on the vertex numbering
          add_edge(distance(hitMap.begin(), hitIterator),
                   distance(hitMap.begin(), parentIterator)  , G);
        } else {
          add_edge(distance(hitMap.begin(), hitIterator),
                   non_exsisting_rtracks_counter++  , G);
        }
      }

      // search for connected subgraphs
      vector<int> component(num_vertices(G));

      // map for component <-> simhit correspondence
      map <int, EKLMSimHit*> graphComponentToSimHit;

      // loop over the vertices
      for (map < int , EKLMStepHit*>::iterator hitIterator = hitMap.begin();
           hitIterator != hitMap.end(); hitIterator++) {
        // get EKLMStepHit corresponding to the current vertex
        EKLMStepHit* stepHit = hitIterator->second;

        // search for the current component in the map
        map <int, EKLMSimHit*>::iterator current =
          graphComponentToSimHit.find(component[distance(hitMap.begin(), hitIterator)]);
        if (current == graphComponentToSimHit.end()) {    // no  entry for this component

          // create new EKLMSimHit and store all information into it
          EKLMSimHit* simHit =
            new(m_simHitsArray->AddrAt(m_simHitsArray.getEntries()))
          EKLMSimHit(stepHit);
          // insert hit to the map
          graphComponentToSimHit.insert(pair<int, EKLMSimHit*>(component[distance(hitMap.begin(), hitIterator)], simHit));

          simHit->setPlane(stepHit->getPlane());
          simHit->setStrip(stepHit->getStrip());
          simHit->setMomentum(stepHit->getMomentum());
          simHit->setEnergy(stepHit->getEnergy());

        } else { // entry already exist
          // compare hittime. The leading one has smallest time
          if (current->second->getTime() < stepHit->getTime()) {
            // new hit is successor, add edep of the successor to the ancestor
            current->second->setEDep(current->second->getEDep() +
                                     stepHit->getEDep());
          } else {
            // new hit is ancestor,  modify everything
            current->second->setEDep(current->second->getEDep() +
                                     stepHit->getEDep());
            current->second->setPosition(stepHit->getPosition());
            current->second->setTime(stepHit->getTime());
            current->second->setPDG(stepHit->getPDG());
            current->second->setMomentum(stepHit->getMomentum());
            current->second->setEnergy(stepHit->getEnergy());
          }
        }
      }

    }


    B2DEBUG(1, "EKLMDigitizer::makeSimHits() completed");
  }



  void EKLMDigitizer::readAndSortSimHits()
  {

    for (int i = 0; i < m_simHitsArray.getEntries(); i++) {

      // search for entries of the same strip
      map<const G4VPhysicalVolume*, vector<EKLMSimHit*> >::iterator
      it = m_HitStripMap.find((m_simHitsArray[i])->getVolume());

      if (it == m_HitStripMap.end()) { //  new entry
        vector<EKLMSimHit*> *vectorHits =
          new vector<EKLMSimHit*> (1, (m_simHitsArray[i]));
        m_HitStripMap.insert(pair<const G4VPhysicalVolume*, vector<EKLMSimHit*> >
                             ((m_simHitsArray[i])->getVolume(), *vectorHits));
      } else {
        it->second.push_back(m_simHitsArray[i]);
      }
    }

  }

  //!  This function is  to form StripHits from SimHits.
  //!  Light propagation into the fiber, SiPM and electronics effects
  //!  are simulated in EKLMFiberAndElectronics class
  void EKLMDigitizer::mergeSimHitsToStripHits(double threshold)
  {
    for (map<const G4VPhysicalVolume*, vector<EKLMSimHit*> >::iterator it =
           m_HitStripMap.begin(); it != m_HitStripMap.end(); it++) {


      // create fiberAndElectronicsSimulator entry
      EKLMFiberAndElectronics* fiberAndElectronicsSimulator =
        new EKLMFiberAndElectronics(*it);

      // do all work
      fiberAndElectronicsSimulator->processEntry();

      EKLMSimHit* simHit = it->second.front();

      // create new stripHit
      EKLMDigit* stripHit =
        new(m_stripHitsArray->AddrAt(m_stripHitsArray.getEntries()))
      EKLMDigit(simHit);

      //***
      //      G4Box* box1 = (G4Box*)(simHit->getVolume()->GetLogicalVolume()->GetSolid()->GetConstituentSolid(0));
      stripHit->setMCTS(simHit->getTime());


      //***

      if (!fiberAndElectronicsSimulator->getFitStatus()) {
        stripHit->setTime(fiberAndElectronicsSimulator->getFitResults(0));
        stripHit->setNumberPhotoElectrons(fiberAndElectronicsSimulator->
                                          getFitResults(3));
      } else {
        stripHit->setTime(0.);
        stripHit->setNumberPhotoElectrons(0);
      }
      stripHit->setFitStatus(fiberAndElectronicsSimulator->getFitStatus());

      if (stripHit->getNumberPhotoElectrons() > threshold)
        stripHit->isGood(true);
      else
        stripHit->isGood(false);


      delete fiberAndElectronicsSimulator;
    }
    //    B2INFO( "STOP MERGING HITS");
  }
}

