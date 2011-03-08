/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Andreas Moll                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/EventAction.h>
#include <simulation/kernel/RunManager.h>

#include <geometry/geodetector/GeoDetector.h>
#include <geometry/geodetector/CreatorManager.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>

#include <boost/foreach.hpp>

#include <G4UImanager.hh>
#include <G4SDManager.hh>
#include <G4DigiManager.hh>

#include <list>
#include <string>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


EventAction::EventAction(const std::string& mcCollectionName, const std::string& relCollectionName, MCParticleGraph& mcParticleGraph, bool createRelation):
    G4UserEventAction(), m_mcCollectionName(mcCollectionName), m_relCollectionName(relCollectionName),
    m_mcParticleGraph(mcParticleGraph), m_createRelation(createRelation)
{

}


EventAction::~EventAction()
{

}


void EventAction::BeginOfEventAction(const G4Event* event)
{

}


void EventAction::EndOfEventAction(const G4Event* event)
{
  //Create the final MCParticle list and update the indices of the MCParticle graph particles.
  m_mcParticleGraph.generateList(m_mcCollectionName, MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

  //Create the MCParticles->Hits relations
  //Loop over all created instances of sensitive detector classes.
  //Get the relation collection from the DataStore for each sensitive detector
  //and replace the TrackID set in the FromIndex by the proper MCParticle index.
  if (m_createRelation) {
    //Build a TrackID to MCParticle index map for faster replacement in the second step
    TrackIDMCPIndexMap trackIDMCPIndexMap;
    for (unsigned int iParticle = 0; iParticle < m_mcParticleGraph.size(); ++iParticle) {
      MCParticleGraph::GraphParticle& currParticle = m_mcParticleGraph[iParticle];
      trackIDMCPIndexMap.insert(make_pair(currParticle.getTrackID(), currParticle.getIndex() - 1));
    }

    //Replace the TrackID by the index
    CreatorManager& creatorManager = CreatorManager::Instance();
    const list<string>& calledCreators = GeoDetector::Instance().getCalledCreators();

    BOOST_FOREACH(string creatorName, calledCreators) {
      list<SensitiveDetectorBase*> senDetList = creatorManager.getCreator(creatorName).getSensitiveDetectorList();

      BOOST_FOREACH(SensitiveDetectorBase* senDet, senDetList) {
        string relColName = senDet->getRelationCollectionName();
        if (!relColName.empty()) {
          StoreArray<Relation> mcPartRelation(relColName);
          for (int iRel = 0; iRel < mcPartRelation.GetEntries(); iRel++) {
            Relation *currRel = mcPartRelation[iRel];
            TrackIDMCPIndexMap::iterator findIter = trackIDMCPIndexMap.find(currRel->getFromIndex());
            if (findIter != trackIDMCPIndexMap.end()) {
              currRel->setFromIndex(findIter->second);
            } else {
              B2ERROR("Building relation " << relColName << ": Could not find TrackID " << currRel->getFromIndex())
            }
          }
        }
      }
    }
  }
}
