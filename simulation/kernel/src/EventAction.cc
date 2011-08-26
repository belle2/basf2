/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Andreas Moll, Martin Ritter                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/EventAction.h>
#include <simulation/kernel/RunManager.h>
#include <framework/datastore/RelationArray.h>
#include <generators/dataobjects/MCParticleGraph.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

#include <boost/foreach.hpp>
#include <list>
#include <string>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


EventAction::EventAction(const std::string& mcCollectionName, MCParticleGraph &mcParticleGraph):
    G4UserEventAction(), m_mcCollectionName(mcCollectionName),  m_mcParticleGraph(mcParticleGraph)
{

}

EventAction::~EventAction()
{

}

void EventAction::BeginOfEventAction(const G4Event* event)
{
  //Enable recording of Hits
  SensitiveDetectorBase::setActive(true);
}


void EventAction::EndOfEventAction(const G4Event* event)
{
  //Disable recording of Hits
  SensitiveDetectorBase::setActive(false);

  //Create the final MCParticle list and update the indices of the MCParticle graph particles.
  m_mcParticleGraph.generateList(m_mcCollectionName);

  //Build a TrackID to MCParticle index list
  vector<unsigned int> indices;
  indices.resize(m_mcParticleGraph.size() + 1);
  for (unsigned int iParticle = 0; iParticle < m_mcParticleGraph.size(); ++iParticle) {
    MCParticleGraph::GraphParticle& currParticle = m_mcParticleGraph[iParticle];
    //assert(currParticle.getTrackID()<indices.size());
    indices[currParticle.getTrackID()] = currParticle.getIndex() - 1;
  }
  RelationArray::ReplaceVec<> indexReplacement(indices);

  //Update all registered MCParticle Relations and replace the TrackID by the final MCParticle id
  BOOST_FOREACH(const string &relName, SensitiveDetectorBase::getMCParticleRelations()) {
    if (!relName.empty()) {
      RelationArray mcPartRelation(relName);
      if (mcPartRelation) mcPartRelation.consolidate(indexReplacement, RelationArray::Identity());
    }
  }
}
