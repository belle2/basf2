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
  m_mcParticleGraph.generateList(m_mcCollectionName, MCParticleGraph::set_decay_info | MCParticleGraph::check_cyclic);

  //Create the Hit -> MCParticle relations
  if (m_createRelation) {
    RunManager::Instance().buildRelations(m_mcParticleGraph, m_mcCollectionName, m_relCollectionName);
  }
}
