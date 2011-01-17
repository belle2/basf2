/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/MCParticleGenerator.h>

#include <generators/dataobjects/MCParticle.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>

#include <G4PrimaryParticle.hh>
#include <G4ParticleTable.hh>
#include <G4VUserPrimaryParticleInformation.hh>
#include <G4Event.hh>

#include <TLorentzVector.h>

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
  //Create a MCParticle graph from the MCParticle collection
  fillGraphFromCollection();

  //For testing
  m_mcParticleGraph.generateList("TestMCCol", MCParticleGraph::set_decay_info | MCParticleGraph::check_cyclic);

  /*

    for (int iPart = 0; iPart < mcParticles.GetEntries(); iPart++) {
      MCParticle *currParticle = mcParticles[iPart];

      //Make one primary vertex for each primary particle, the rest gets added recursively by addParticle
      //The MCParticle collection has to be sorted breadth first: primary particles come first and then the daughters
      if (currParticle->getMother() != NULL) break;

      G4PrimaryParticle* primaryParticle = addParticle(*currParticle, TVector3(0,0,0), true);



    }*/
}

/*
G4PrimaryParticle* MCParticleGenerator::addParticle(MCParticle &mc, const TVector3& boost, bool use_time)
{
  TLorentzVector partMom4 = mc.get4Vector();

  //Get the particle definition, check if the return value is not NULL
  G4ParticleDefinition* pdef = G4ParticleTable::GetParticleTable()->FindParticle(mc.getPDG());
  if (pdef == NULL) return NULL;

  //Create a new Geant4 Primary particle and store the link to the MCParticle object as user info.
  particle = new G4PrimaryParticle(pdef, partMom4.X() / Unit::MeV, partMom4.Y() / Unit::MeV, partMom4.Z() / Unit::MeV, partMom4.E() / Unit::MeV);
  particle->SetMass(mc.getMass() / Unit::MeV);
  particle->SetUserInformation(new ParticleInfo(mc));


  G4PrimaryParticle *particle = m_seen[mc.getIndex()];
  //Check if we already saw that particle and return it
  //FIXME: Check if Geant4 can handle compound decays, i think not
  if (particle) return particle;

  G4ParticleDefinition* pdef = G4ParticleTable::GetParticleTable()->FindParticle(mc.getPDG());
  //If Geant4 does not know our particle, set an unknown particle: Will decay immidiatly but
  //leave a track for us to see
  bool is_unknown(false);
  if (!pdef) {
    pdef = G4UnknownParticle::UnknownParticle();
    is_unknown = true;
  }

  TLorentzVector p = mc.get4Vector();
  //if (is_unknown) p.Boost(boost);

  particle = new G4PrimaryParticle(mc.getPDG(), p.X() / MeV, p.Y() / MeV, p.Z() / MeV, p.E() / MeV);
  particle->SetMass(mc.getMass() / MeV);
  particle->SetUserInformation(new ParticleInfo(mc));

  const TVector3 &dv = mc.getDecayVertex();
  //Set propergation time only if use_time and, valid vertex, inside beampipe cut and if there are children
  use_time &= mc.hasValidVertex() && dv.Perp() < m_rcut && fabs(dv.Z()) < m_zcut && mc.getFirstDaughter() > 0;
  if (use_time) {
    //ProperTime is in particle eigentime, so convert lab lifetime to eigentime
    double propertime = mc.getLifetime() / mc.get4Vector().Gamma();
    particle->SetProperTime(propertime);
  }
  //Set Propergation time for unknown particles, causes geant4 to boost decay particles
  //if (is_unknown) particle->SetProperTime(1);
  //Add all children
  BOOST_FOREACH(MCParticle* daughter, mc.getDaughters()) {
    G4PrimaryParticle* d = addParticle(*daughter, -mc.get4Vector().BoostVector(), use_time);
    particle->SetDaughter(d);
  }
  m_seen[mc.getIndex()] = particle;
  return particle;
}*/


//===================================================================
//                       Protected methods
//===================================================================

void MCParticleGenerator::fillGraphFromCollection()
{
  //Get the MCParticle collection from the DataStore
  StoreArray<MCParticle> mcParticles(m_mcCollectionName);

  //Prepare the MCParticle graph
  m_mcParticleGraph.clear();

  //Make list of particles
  for (int i = 0; i < mcParticles.GetEntries(); ++i) {
    m_mcParticleGraph.addParticle();
  }

  //Read particles from the MCParticle collection and fill the graph
  for (int iPart = 0; iPart < mcParticles.GetEntries(); iPart++) {
    MCParticle& currParticle = *mcParticles[iPart];
    MCParticleGraph::GraphParticle &graphParticle = m_mcParticleGraph[iPart];

    //Copy the properties of the MCParticle
    graphParticle.setPDG(currParticle.getPDG());
    graphParticle.setStatus(currParticle.getStatus());
    graphParticle.setMass(currParticle.getMass());
    graphParticle.setEnergy(currParticle.getEnergy());
    graphParticle.setValidVertex(currParticle.hasValidVertex());
    graphParticle.setProductionTime(currParticle.getProductionTime());
    graphParticle.setDecayTime(currParticle.getDecayTime());
    graphParticle.setProductionVertex(currParticle.getProductionVertex());
    graphParticle.setMomentum(currParticle.getMomentum());
    graphParticle.setDecayVertex(currParticle.getDecayVertex());
    graphParticle.setFirstDaughter(currParticle.getFirstDaughter());
    graphParticle.setLastDaughter(currParticle.getLastDaughter());

    //Add decays
    for (int index = currParticle.getFirstDaughter(); index <= currParticle.getLastDaughter(); ++index) {
      if (index > 0) graphParticle.decaysInto(m_mcParticleGraph[index-1]);
    }
  }
}
