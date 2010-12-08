/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/MCParticleGenerator.h>

#include <G4Event.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


MCParticleGenerator::MCParticleGenerator(const string& mcCollectionName) : G4VPrimaryGenerator()
{
  m_mcCollectionName = mcCollectionName;
}


MCParticleGenerator::~MCParticleGenerator()
{

}


void MCParticleGenerator::GeneratePrimaryVertex(G4Event* event)
{

}

