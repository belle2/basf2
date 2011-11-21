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

#include <framework/datastore/StoreArray.h>

#include "G4VPhysicalVolume.hh"


#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
// #include <boost/graph/graph_traits.hpp>
// #include <boost/graph/properties.hpp>
// #include <boost/range/irange.hpp>

#include <eklm/dataobjects/EKLMSimHit.h>



//-----------------------

#include <eklm/simeklm/EKLMFiberAndElectronics.h>

#include <eklm/eklmutils/EKLMutils.h>
#include <framework/logging/Logger.h>
#include<iostream>
#include<fstream>

#include <framework/datastore/StoreObjPtr.h>


#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>

#include <TRandom.h>

#include <geometry/CreatorManager.h>
#include <geometry/CreatorBase.h>

#include <eklm/geoeklm/GeoEKLMBelleII.h>


using namespace CLHEP;
using namespace std;

namespace Belle2 {



  void EKLMDigitizer::readAndSortStepHits()
  {
    B2DEBUG(1, "EKLMDigitizer::readAndSortStepHits()");

    StoreArray<EKLMStepHit> stepHitsArray;
    for (int i = 0; i < stepHitsArray.getEntries(); i++) {

      // search for entries of the same strip
      map<const G4VPhysicalVolume *, vector<EKLMStepHit*> >::iterator
      it = m_stepHitVolumeMap.find((stepHitsArray[i])->getVolume());

      if (it == m_stepHitVolumeMap.end()) { //  new entry
        vector<EKLMStepHit*> *vectorHits = new vector<EKLMStepHit*> (1, (stepHitsArray[i]));

        m_stepHitVolumeMap.insert(pair<const G4VPhysicalVolume *, vector<EKLMStepHit*> >((stepHitsArray[i])->getVolume(), *vectorHits));
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

    StoreArray<EKLMSimHit> simHitsArray;


    //loop over volumes
    for (map<const G4VPhysicalVolume *, vector<EKLMStepHit*> >::iterator volumeIterator = m_stepHitVolumeMap.begin();
         volumeIterator != m_stepHitVolumeMap.end(); volumeIterator++) {


      // we have only tree graphs here, so edge is completely defined by it's track ID
      // map to store (edge  <--> hit) == (vertex <--> hit) correspondence
      map < int , EKLMStepHit*> hitMap;

      for (vector<EKLMStepHit*>::iterator i = volumeIterator->second.begin(); i != volumeIterator->second.end(); i++) {
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

      for (map < int , EKLMStepHit*>::iterator hitIterator = hitMap.begin(); hitIterator != hitMap.end(); hitIterator++) {

        //search for parent entry in the map
        map < int , EKLMStepHit*>::iterator parentIterator =
          hitMap.find(hitIterator->second->getParentTrackID());

        if (parentIterator != hitMap.end()) { // nothing found
          // see comments above on the vertex numbering
          add_edge(distance(hitMap.begin(), hitIterator), distance(hitMap.begin(), parentIterator)  , G);
        } else {
          add_edge(distance(hitMap.begin(), hitIterator), non_exsisting_rtracks_counter++  , G);
        }
      }

//       graph_traits < adjacency_list <> >::vertex_iterator it, end;
//       property_map < adjacency_list <>, vertex_index_t >::type  index_map = get(vertex_index, G);

      // search for connected subgraphs
      vector<int> component(num_vertices(G));

      // map for component <-> simhit correspondence
      map <int, EKLMSimHit*> graphComponentToSimHit;

      // loop over the vertices
      for (map < int , EKLMStepHit*>::iterator hitIterator = hitMap.begin(); hitIterator != hitMap.end(); hitIterator++) {
        // get EKLMStepHit corresponding to the current vertex
        EKLMStepHit * stepHit = hitIterator->second;

        // search for the current component in the map
        map <int, EKLMSimHit*>::iterator current = graphComponentToSimHit.find(component[distance(hitMap.begin(), hitIterator)]);
        if (current == graphComponentToSimHit.end()) {    // no  entry for this component

          // create new EKLMSimHit and store all information into it
          EKLMSimHit *simHit = new(simHitsArray->AddrAt(simHitsArray.getEntries()))  EKLMSimHit();
          // insert hit to the map
          graphComponentToSimHit.insert(pair<int, EKLMSimHit*>(component[distance(hitMap.begin(), hitIterator)], simHit));
          simHit->setGlobalPos(stepHit->getPosition());
          simHit->setTime(stepHit->getTime());
          simHit->setEDep(stepHit->getEDep());
          simHit->setPDGCode(stepHit->getPDG());
          //simHit->setVolType(volType);
          simHit->setMomentum(stepHit->getMomentum());
          simHit->setEnergy(stepHit->getEnergy());

//    if (volType == 0) { // strip
//      simHit->set_nEndcap(stepHit->get_nEndcap());
//      simHit->set_nSector(stepHit->get_nSector());
//      simHit->set_nLayer(stepHit->get_nLayer());
//      simHit->set_nPlane(stepHit->get_nPlane());
//      simHit->set_nStrip(stepHit->get_nStrip());
//    }
        } else { // entry already exist
          // compare hittime. The leading one has smallest time
          if (current->second->getTime() < stepHit->getTime()) {
            // new hit is successor, add edep of the successor to the ancestor
            current->second->setEDep(current->second->getEDep() + stepHit->getEDep());
          } else {
            // new hit is ancestor,  modify everything
            current->second->setEDep(current->second->getEDep() + stepHit->getEDep());
            current->second->setGlobalPos(stepHit->getPosition());
            current->second->setTime(stepHit->getTime());
            current->second->setPDGCode(stepHit->getPDG());
            current->second->setMomentum(stepHit->getMomentum());
            current->second->setEnergy(stepHit->getEnergy());
          }
        }
      }

    }


    B2INFO("EKLMDigitizer::makeSimHits() completed");
  }











  void EKLMDigitizer::readAndSortSimHits()
  {
    StoreArray<EKLMSimHit> simHitsArray;
    for (int i = 0; i < simHitsArray.getEntries(); i++) {

      // search for entries of the same strip
      map<G4VPhysicalVolume *, vector<EKLMSimHit*> >::iterator
      it = m_HitStripMap.find((simHitsArray[i])->getPV());

      if (it == m_HitStripMap.end()) { //  new entry
        vector<EKLMSimHit*> *vectorHits =
          new vector<EKLMSimHit*> (1, (simHitsArray[i]));
        m_HitStripMap.insert(pair<G4VPhysicalVolume *, vector<EKLMSimHit*> >
                             ((simHitsArray[i])->getPV(), *vectorHits));
      } else {
        it->second.push_back(simHitsArray[i]);
      }
    }

  }

  //!  This function is  to form StripHits from SimHits.
  //!  Light propagation into the fiber, SiPM and electronics effects
  //!  are simulated in EKLMFiberAndElectronics class
  void EKLMDigitizer::mergeSimHitsToStripHits()
  {
    //    B2INFO( "STRAT MERGING HITS");
    for (map<G4VPhysicalVolume *, vector<EKLMSimHit*> >::iterator it =
           m_HitStripMap.begin(); it != m_HitStripMap.end(); it++) {

      //      B2DEBUG(1, "MAP ENTRY");
      EKLMFiberAndElectronics * fiberAndElectronicsSimulator =
        new EKLMFiberAndElectronics(*it);
      fiberAndElectronicsSimulator->processEntry();

      EKLMSimHit *simHit = it->second.front();

      StoreArray<EKLMStripHit> stripHitsArray;
      EKLMStripHit *stripHit = new(stripHitsArray->AddrAt(stripHitsArray.getEntries()))EKLMStripHit();

      //      EKLMStripHit *stripHit = new EKLMStripHit();

      stripHit->set_nEndcap(simHit->get_nEndcap());
      stripHit->set_nLayer(simHit->get_nLayer());
      stripHit->set_nSector(simHit->get_nSector());
      stripHit->set_nPlane(simHit->get_nPlane());
      stripHit->set_nStrip(simHit->get_nStrip());
      stripHit->setPV(simHit->getPV());

      //stripHit->setHistogramm(cloneHist);

      if (!fiberAndElectronicsSimulator->getFitStatus()) {
        stripHit->setTime(fiberAndElectronicsSimulator->getFitResults(0));
        stripHit->setNumberPhotoElectrons(fiberAndElectronicsSimulator->
                                          getFitResults(3));
      } else {
        stripHit->setNumberPhotoElectrons(-1);
        stripHit->setTime(-1);
      }

      stripHit->setLeadingParticlePDGCode(0);

      //      m_HitVector.push_back(stripHit);
      delete fiberAndElectronicsSimulator;
    }
    //    B2INFO( "STOP MERGING HITS");
  }
}

