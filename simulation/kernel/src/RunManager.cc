/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/RunManager.h>
#include <framework/logging/Logger.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

RunManager* RunManager::m_instance = NULL;

RunManager& RunManager::Instance()
{
  static SingletonDestroyer siDestroyer;
  if (!m_instance) m_instance = new RunManager();
  return *m_instance;
}


void RunManager::beginRun(int runNumber)
{
  //Check if all the necessary initializations have already be done.
  if (ConfirmBeamOnCondition()) {
    //Set run number
    SetRunIDCounter(runNumber);

    //Finish initialization
    ConstructScoringWorlds();
    RunInitialization();
  } else {
    B2FATAL("The Geant4 kernel was not initialized properly ! The method 'ConfirmBeamOnCondition()' failed. ")
  }
}


void RunManager::processEvent(int evtNumber)
{
  //Delete the hit track relation map
  m_trackHitMap.clear();

  //Geant4 simulation
  currentEvent = GenerateEvent(evtNumber);
  eventManager->ProcessOneEvent(currentEvent);
  AnalyzeEvent(currentEvent);
  UpdateScoring();
  StackPreviousEvent(currentEvent);
  currentEvent = NULL;
}


void RunManager::endRun()
{
  RunTermination();
}


void RunManager::destroy()
{
  if (m_instance == NULL) return;

  delete m_instance;
  m_instance = NULL;
}


bool RunManager::addRelation(TObject* hit, G4Step* step)
{
  bool result = true;

  //Get the track ID
  int trkID = step->GetTrack()->GetTrackID();

  //Check if the track ID was already added to the map. If not add the track ID and the hit to the map.
  map<int, set<TObject*> >::iterator mapIter = m_trackHitMap.find(trkID);
  if (mapIter == m_trackHitMap.end()) {
    m_trackHitMap[trkID].insert(hit);
  } else {
    //Check if the hit was already added. If not add the hit.
    set<TObject*>::iterator setIter = mapIter->second.find(hit);
    if (setIter == mapIter->second.end()) {
      mapIter->second.insert(hit);
    } else {
      B2ERROR("Error adding a hit to the MCParticle<->hit relation. The hit has already been added to the track ID " << trkID << " !")
      result = false;
    }
  }
  return result;
}


void RunManager::buildRelations(MCParticleGraph& particleGraph, const string& mcCollectionName, const std::string& relCollectionName)
{
  StoreArray<MCParticle> mcParticles(mcCollectionName);
  StoreArray<Relation> mcPartRelation(relCollectionName);
  int iRel = 0;

  //Loop over the MCParticle Graph
  for (unsigned int iParticle = 0; iParticle < particleGraph.size(); ++iParticle) {
    MCParticleGraph::GraphParticle& currParticle = particleGraph[iParticle];

    //Get the MCParticle
    MCParticle* mcPart = mcParticles[currParticle.getIndex()-1];

    //Loop over the list of associated hits and create the relation objects
    map<int, set<TObject*> >::iterator mapIter = m_trackHitMap.find(currParticle.getTrackID());
    if (mapIter != m_trackHitMap.end()) {
      BOOST_FOREACH(TObject* hit, mapIter->second) {
        new(mcPartRelation->AddrAt(iRel)) Relation(hit, mcPart); //build relation hit -> MCParticle
        iRel++;
      }
    }
  }
}


//============================================================================
//                              Private methods
//============================================================================

RunManager::RunManager() : G4RunManager()
{

}


RunManager::~RunManager()
{

}
