/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <G4Box.hh>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

/* Belle2 headers. */
#include <eklm/simulation/Digitizer.h>
#include <eklm/simulation/FiberAndElectronics.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

static const char MemErr[] = "Memory allocation error.";

EKLM::Digitizer::Digitizer(EKLM::GeometryData* geoDat,
                           struct EKLM::DigitizationParams* digPar)
{
  m_geoDat = geoDat;
  m_digPar = digPar;
}

EKLM::Digitizer::~Digitizer()
{
}

void EKLM::Digitizer::readAndSortStepHits()
{
  B2DEBUG(1, "EKLM::Digitizer::readAndSortStepHits()");

  StoreArray<EKLMSimHit> stepHitsArray;
  for (int i = 0; i < stepHitsArray.getEntries(); i++) {

    // search for entries of the same strip
    std::map<int, std::vector<EKLMSimHit*> >::iterator
    it = m_stepHitVolumeMap.find((stepHitsArray[i])->getVolumeID());

    if (it == m_stepHitVolumeMap.end()) { //  new entry
      std::vector<EKLMSimHit*>* vectorHits = NULL;
      try {
        vectorHits = new std::vector<EKLMSimHit*> (1, (stepHitsArray[i]));
      } catch (std::bad_alloc& ba) {
        B2FATAL(MemErr);
      }

      m_stepHitVolumeMap.insert(std::pair<int, std::vector<EKLMSimHit*> >((stepHitsArray[i])->getVolumeID(), *vectorHits));
    } else {
      it->second.push_back(stepHitsArray[i]);
    }
  }
  B2DEBUG(1, "EKLM::Digitizer::readAndSortStepHits()  completed");
}


void EKLM::Digitizer::makeSimHits()
{
  B2DEBUG(1, "EKLM::Digitizer::makeSimHits()");




  //loop over volumes
  for (std::map<int, std::vector<EKLMSimHit*> >::iterator
       volumeIterator = m_stepHitVolumeMap.begin();
       volumeIterator != m_stepHitVolumeMap.end(); volumeIterator++) {

    // we have only tree graphs here, so edge is completely defined by it's track ID
    // map to store (edge  <--> hit) == (vertex <--> hit) correspondence
    std::map < int , EKLMSimHit*> hitMap;

    for (std::vector<EKLMSimHit*>::iterator i = volumeIterator->second.begin();
         i != volumeIterator->second.end(); i++) {
      // ID key
      int key = (*i)->getTrackID();

      // here we merge all StepHits produced by the same track.
      //The leading (with smallest time) hit survies.
      //The others are deleted.
      //Integrated energy deposition is prescribed to the leading hit
      //this procedure removes multiple maps entries  with identical keys

      // search if entry already exist
      std::map < int , EKLMSimHit*>::iterator mapIterator = hitMap.find(key);
      if (mapIterator == hitMap.end()) { // nothing found
        // add an entry
        hitMap.insert(std::pair< int , EKLMSimHit*>(key, *i));
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

    boost::adjacency_list <> G;

    // we here restore the tree of hits using TrackID and ParentTrackID information
    // There is a possibility for two hit trees wich are connected by parentTrackID outside the volume
    // use special ParentTrackID for the tracks coming from abroad  remains these trees splittable
    int non_exsisting_rtracks_counter = hitMap.size();

    for (std::map < int , EKLMSimHit*>::iterator hitIterator = hitMap.begin();
         hitIterator != hitMap.end(); hitIterator++) {

      //search for parent entry in the map
      std::map < int , EKLMSimHit*>::iterator parentIterator =
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
    std::vector<int> component(num_vertices(G));

    // map for component <-> simhit correspondence
    std::map <int, EKLMSim2Hit*> graphComponentToSimHit;

    // loop over the vertices
    for (std::map < int , EKLMSimHit*>::iterator hitIterator = hitMap.begin();
         hitIterator != hitMap.end(); hitIterator++) {
      // get EKLMSimHit corresponding to the current vertex
      EKLMSimHit* stepHit = hitIterator->second;

      // search for the current component in the map
      std::map <int, EKLMSim2Hit*>::iterator current =
        graphComponentToSimHit.find(component[distance(hitMap.begin(), hitIterator)]);
      if (current == graphComponentToSimHit.end()) {    // no  entry for this component

        // create new EKLMSim2Hit and store all information into it
        EKLMSim2Hit* simHit =
          new(m_simHitsArray.nextFreeAddress()) EKLMSim2Hit(stepHit);
        // insert hit to the map
        graphComponentToSimHit.insert(std::pair<int, EKLMSim2Hit*>(component[distance(hitMap.begin(), hitIterator)], simHit));

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
          current->second->setGlobalPosition(stepHit->getGlobalPosition());
          current->second->setLocalPosition(stepHit->getLocalPosition());
          current->second->setTime(stepHit->getTime());
          current->second->setPDG(stepHit->getPDG());
          current->second->setMomentum(stepHit->getMomentum());
        }
      }
    }

  }


  B2DEBUG(1, "EKLM::Digitizer::makeSimHits() completed");
}



void EKLM::Digitizer::readAndSortSimHits()
{

  for (int i = 0; i < m_simHitsArray.getEntries(); i++) {

    // search for entries of the same strip
    std::map<int, std::vector<EKLMSim2Hit*> >::iterator
    it = m_HitStripMap.find((m_simHitsArray[i])->getVolumeID());

    if (it == m_HitStripMap.end()) { //  new entry
      std::vector<EKLMSim2Hit*>* vectorHits = NULL;
      try {
        vectorHits = new std::vector<EKLMSim2Hit*> (1, (m_simHitsArray[i]));
      } catch (std::bad_alloc& ba) {
        B2FATAL(MemErr);
      }
      m_HitStripMap.insert(std::pair<int, std::vector<EKLMSim2Hit*> >
                           ((m_simHitsArray[i])->getVolumeID(), *vectorHits));
    } else {
      it->second.push_back(m_simHitsArray[i]);
    }
  }

}

//!  This function is  to form StripHits from SimHits.
//!  Light propagation into the fiber, SiPM and electronics effects
//!  are simulated in EKLMFiberAndElectronics class
void EKLM::Digitizer::mergeSimHitsToStripHits(double threshold)
{
  for (std::map<int, std::vector<EKLMSim2Hit*> >::iterator it =
         m_HitStripMap.begin(); it != m_HitStripMap.end(); it++) {


    // create fes entry
    EKLM::FiberAndElectronics* fes =
      new EKLM::FiberAndElectronics(*it, m_geoDat, m_digPar);

    // do all work
    fes->processEntry();

    EKLMSim2Hit* simHit = it->second.front();

    // create new stripHit
    EKLMDigit* stripHit =
      new(m_stripHitsArray.nextFreeAddress()) EKLMDigit(simHit);

    stripHit->setMCTime(simHit->getTime());
    stripHit->setGlobalPosition(simHit->getGlobalPosition());
    stripHit->setGeneratedNPE(fes->getGeneratedNPE());
    if (!fes->getFitStatus()) {
      stripHit->setTime(fes->getFitResults()->startTime);
      stripHit->setNPE(fes->getNPE());
    } else {
      stripHit->setTime(0.);
      stripHit->setNPE(0);
    }
    stripHit->setFitStatus(fes->getFitStatus());

    if (stripHit->getNPE() > threshold)
      stripHit->isGood(true);
    else
      stripHit->isGood(false);


    delete fes;
  }
  //    B2INFO( "STOP MERGING HITS");
}

