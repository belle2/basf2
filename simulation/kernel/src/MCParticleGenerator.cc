/*****************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                        *
 * Copyright(C) 2010-2011  Belle II Collaboration                            *
 *                                                                           *
 * Author: The Belle II Collaboration                                        *
 * Contributors: Andreas Moll, Martin Ritter, Doris Kim, Romulus Godang,     *
 *               Marko Staric                                                *
 *                                                                           *
 * This software is provided "as is" without any warranty.                   *
 *                                                                           *
 * 7/28/13 Modified (addToG4) to (addToG4 && mcParticle.getLifetime() != 0.0)*
 * to deal with the resonance particles that enter the detector in GEANT4    *
 * by R. Godang and Doris Kim                                                *
 *****************************************************************************/

#include <simulation/kernel/MCParticleGenerator.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <simulation/kernel/UserInfo.h>

#include <G4ParticleTable.hh>
#include <G4VUserPrimaryParticleInformation.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTypes.hh>

#include <TLorentzVector.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


MCParticleGenerator::MCParticleGenerator(const string& mcCollectionName, MCParticleGraph& mcParticleGraph) :
  G4VPrimaryGenerator(), m_mcCollectionName(mcCollectionName), m_mcParticleGraph(mcParticleGraph)
{

}


MCParticleGenerator::~MCParticleGenerator()
{

}


void MCParticleGenerator::GeneratePrimaryVertex(G4Event* event)
{
  //Get the MCParticle collection from the DataStore
  StoreArray<MCParticle> mcParticles(m_mcCollectionName);

  //Prepare the MCParticle graph
  m_mcParticleGraph.clear();

  //Loop over the primary particles. The MCParticle collection has to be
  //sorted breadth first: primary particles come first and then the daughters
  int nPart = mcParticles.getEntries();
  for (int iPart = 0; iPart < nPart; iPart++) {
    MCParticle* currParticle = mcParticles[iPart];
    if (currParticle->getMother() != NULL) break;

    //Add primary particle (+ daughters) and the vertex
    addParticle(*currParticle, event, NULL, 0, true);
  }
}


void MCParticleGenerator::addParticle(MCParticle& mcParticle,
                                      G4Event* event,
                                      G4PrimaryParticle* lastG4Mother,
                                      int motherIndex,
                                      bool useTime)
{
  G4PrimaryParticle* g4Mother = lastG4Mother;

  //Check if the particle should be added to Geant4
  //Only add particle if its PDG code is known to Geant4 and it is not flagged as virtual.
  bool addToG4 = !mcParticle.isVirtual();

  //MS: distinguish optical photon from the rest of particles
  bool opticalPhoton = mcParticle.getPDG() == 0 && mcParticle.getEnergy() < 10.0 * Unit::eV;

  G4ParticleDefinition* pdef = NULL;
  if (opticalPhoton) {
    pdef = G4OpticalPhoton::OpticalPhotonDefinition();
  } else {
    pdef = G4ParticleTable::GetParticleTable()->FindParticle(mcParticle.getPDG());
  }

  if (pdef == NULL) {
    // RG, DK when the skipped particle has a long lived and it is not flagged as virtual
    if (addToG4 && mcParticle.getLifetime() != 0.0)
      B2WARNING("PDG code " << mcParticle.getPDG() << " unknown to Geant4 - particle skipped.");
    addToG4 = false;
  }

  //Add particle to MCParticle graph
  MCParticleGraph::GraphParticle& graphParticle = m_mcParticleGraph.addParticle();
  graphParticle.setPDG(mcParticle.getPDG());
  graphParticle.setStatus(mcParticle.getStatus());
  graphParticle.setMass(mcParticle.getMass());
  graphParticle.setCharge(mcParticle.getCharge());
  graphParticle.setEnergy(mcParticle.getEnergy());
  graphParticle.setValidVertex(mcParticle.hasValidVertex());
  graphParticle.setProductionTime(mcParticle.getProductionTime());
  graphParticle.setDecayTime(mcParticle.getDecayTime());
  graphParticle.setProductionVertex(mcParticle.getProductionVertex());
  graphParticle.setMomentum(mcParticle.getMomentum());
  graphParticle.setDecayVertex(mcParticle.getDecayVertex());
  graphParticle.setFirstDaughter(mcParticle.getFirstDaughter());
  graphParticle.setLastDaughter(mcParticle.getLastDaughter());
  //Add decay to MCParticle graph
  if (motherIndex > 0) graphParticle.comesFrom(m_mcParticleGraph[motherIndex - 1]);

  //Create a new Geant4 Primary particle and
  //store the link to the GraphMCParticle object as user info.
  G4PrimaryParticle* newPart = NULL;

  if (addToG4) {
    TLorentzVector mcPartMom4 = mcParticle.get4Vector();
    newPart = new G4PrimaryParticle(pdef,
                                    mcPartMom4.X() / Unit::MeV,
                                    mcPartMom4.Y() / Unit::MeV,
                                    mcPartMom4.Z() / Unit::MeV,
                                    mcPartMom4.E() / Unit::MeV);
    newPart->SetMass(mcParticle.getMass() / Unit::MeV);
    newPart->SetCharge(mcParticle.getCharge());
    if (opticalPhoton) {
      TVector3 polarization = mcParticle.getDecayVertex(); // temporary stored here
      newPart->SetPolarization(polarization.X(), polarization.Y(), polarization.Z());
    }
    newPart->SetUserInformation(new ParticleInfo(graphParticle));

    //Set propagation time only if useTime is true, the MCparticle has a valid vertex and has children.
    useTime &= mcParticle.hasValidVertex() && mcParticle.getFirstDaughter() > 0;
    if (useTime) {
      //ProperTime is in particle eigentime, so convert lab lifetime to eigentime
      double propertime = mcParticle.getLifetime() / mcParticle.get4Vector().Gamma();
      newPart->SetProperTime(propertime);
    }

    if (lastG4Mother != NULL) lastG4Mother->SetDaughter(newPart);
    g4Mother = newPart;

    //Do not store the generator info in the MCParticles block unless Geant4 creates a track in the detector.
    graphParticle.setIgnore();

  } else {
    B2DEBUG(100, "The particle " << mcParticle.getIndex() << " (PDG " << mcParticle.getPDG() << ") was not added to Geant4");
  }

  //If there is no Geant4 mother particle and the particle is added to Geant4, create a new primary vertex and assign the particle
  if ((lastG4Mother == NULL) && (addToG4)) {
    //Create the vertex
    TVector3 mcProdVtx = mcParticle.getProductionVertex();
    G4PrimaryVertex* vertex = new G4PrimaryVertex(mcProdVtx.X() / Unit::mm,
                                                  mcProdVtx.Y() / Unit::mm,
                                                  mcProdVtx.Z() / Unit::mm,
                                                  mcParticle.getProductionTime());
    vertex->SetPrimary(newPart);

    //Add the vertex to the event
    event->AddPrimaryVertex(vertex);
    B2DEBUG(10, "Created the vertex (" << mcProdVtx.X() << "," << mcProdVtx.Y() << "," << mcProdVtx.Z() << ") with the primary particle " << mcParticle.getPDG());
  }

  //Add all children
  int currMotherIndex = m_mcParticleGraph.size();
  for (MCParticle * daughter : mcParticle.getDaughters()) {
    addParticle(*daughter, event, g4Mother, currMotherIndex, useTime);
  }
}


//===================================================================
//                       Protected methods
//===================================================================
