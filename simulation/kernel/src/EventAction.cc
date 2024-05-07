/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/EventAction.h>
#include <framework/core/Environment.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <simulation/kernel/SensitiveDetectorBase.h>

#include <string>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


EventAction::EventAction(const std::string& mcCollectionName, MCParticleGraph& mcParticleGraph):
  G4UserEventAction(), m_mcCollectionName(mcCollectionName),  m_mcParticleGraph(mcParticleGraph)
{
  if (false) {
    G4Event* event;
    BeginOfEventAction(event);
    EndOfEventAction(event);
  }
  m_writeSimSteps = Environment::Instance().getWriteSimSteps();
  if (m_writeSimSteps)
    m_VREventStream = new std::ofstream;
}

EventAction::~EventAction()
{
  if (m_writeSimSteps) {
    // Just in case the file is still opened...
    if (m_VREventStream->is_open())
      m_VREventStream->close();
    delete m_VREventStream;
  }
}

void EventAction::BeginOfEventAction(const G4Event*)
{
  //Enable recording of Hits
  SensitiveDetectorBase::setActive(true);
  if (m_writeSimSteps) {
    // Open a new output file for the VR event-history steps
    if (m_VREventStream->is_open())
      m_VREventStream->close();
    if (not m_evtMetaData.isValid())
      B2FATAL("EventMetaData is not valid.");
    int eventNumber = m_evtMetaData->getEvent();
    std::string filename = boost::str(boost::format("event%1%.csv") % eventNumber);
    m_VREventStream->open(filename);
    B2INFO("Opened VR event-history file " << filename);
  }
}


void EventAction::EndOfEventAction(const G4Event*)
{
  //Disable recording of Hits
  SensitiveDetectorBase::setActive(false);

  //Create the final MCParticle list and update the indices of the MCParticle graph particles.
  m_mcParticleGraph.generateList(m_mcCollectionName, MCParticleGraph::c_clearParticles);

  //Build a TrackID to MCParticle index list
  vector<std::pair<unsigned int, bool> > indices;
  indices.resize(m_mcParticleGraph.size() + 1);
  for (unsigned int iParticle = 0; iParticle < m_mcParticleGraph.size(); ++iParticle) {
    MCParticleGraph::GraphParticle& currParticle = m_mcParticleGraph[iParticle];
    //assert(currParticle.getTrackID()<indices.size());
    indices[currParticle.getTrackID()] = std::make_pair(currParticle.getIndex() - 1, currParticle.getIgnore());
  }
  RelationArray::ReplaceVec<> indexReplacement(indices);

  //Update all registered MCParticle Relations and replace the TrackID by the final MCParticle id
  const std::map<std::string, RelationArray::EConsolidationAction>& relations = SensitiveDetectorBase::getMCParticleRelations();
  for (std::map<std::string, RelationArray::EConsolidationAction>::const_iterator it = relations.begin(); it != relations.end();
       ++it) {
    RelationArray mcPartRelation(it->first);
    if (mcPartRelation) mcPartRelation.consolidate(indexReplacement, RelationArray::Identity(), it->second);
  }

  if (m_writeSimSteps) {
    // Close the VR event-history file
    if (m_VREventStream->is_open())
      m_VREventStream->close();
  }
}
