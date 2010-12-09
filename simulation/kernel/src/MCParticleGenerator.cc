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

#include <generators/dataobjects/MCParticle.h>
#include <framework/datastore/Units.h>
#include <framework/datastore/StoreArray.h>

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
  //Get the MCParticle collection from the DataStore
  StoreArray<MCParticle> mcParticles(m_mcCollectionName);

  for (int iPart = 0; iPart < mcParticles.GetEntries(); iPart++) {
    MCParticle *currParticle = mcParticles[iPart];

    //Make one primary vertex for each primary particle, the rest gets added recursively by addParticle
    //The MCParticle collection has to be sorted breadth first: primary particles come first and then the daughters


  }
}

