/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/PrimaryGeneratorAction.h>
#include <simulation/kernel/MCParticleGenerator.h>

#include <G4Event.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


PrimaryGeneratorAction::PrimaryGeneratorAction(const string& mcCollectionName,
                                               MCParticleGraph& mcParticleGraph) : G4VUserPrimaryGeneratorAction()
{
  m_mcParticleGenerator = new MCParticleGenerator(mcCollectionName, mcParticleGraph);
  if (false) {
    G4Event* event = nullptr;
    GeneratePrimaries(event);
  }
}


PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete m_mcParticleGenerator;
}


void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  m_mcParticleGenerator->GeneratePrimaryVertex(event);
}
