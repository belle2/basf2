/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/PrimaryGeneratorAction.h>
#include <simulation/kernel/MCParticleGenerator.h>

#include <G4Event.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


PrimaryGeneratorAction::PrimaryGeneratorAction(const string& mcCollectionName, MCParticleGraph& mcParticleGraph) : G4VUserPrimaryGeneratorAction()
{
  m_mcParticleGenerator = new MCParticleGenerator(mcCollectionName, mcParticleGraph);
  if (false) {
    G4Event* event;
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
