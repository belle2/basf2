/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/MCParticleGenerator.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <simulation/kernel/UserInfo.h>
#include <geometry/GeometryManager.h>

#include <G4ParticleTable.hh>
#include <G4VUserPrimaryParticleInformation.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTypes.hh>
#include <G4VPhysicalVolume.hh>
#include <G4LogicalVolume.hh>
#include <G4VSolid.hh>
#include <CLHEP/Vector/LorentzVector.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


MCParticleGenerator::MCParticleGenerator(const string& mcCollectionName, MCParticleGraph& mcParticleGraph) :
  G4VPrimaryGenerator(), m_mcCollectionName(mcCollectionName), m_mcParticleGraph(mcParticleGraph)
{
  G4VPhysicalVolume* topVolume = geometry::GeometryManager::getInstance().getTopVolume();
  if (topVolume) {
    m_topSolid = topVolume->GetLogicalVolume()->GetSolid();
  }
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
  // ... at least that would be the case if there is only one generator, but if
  // more than one generator is used that is not necessarily true. So ignore
  // that last statement.
  //
  // Let's add all top level particles, i.e. without mother, and addParticle
  // will be called recursively for all daughters
  int nPart = mcParticles.getEntries();
  for (int iPart = 0; iPart < nPart; iPart++) {
    MCParticle* currParticle = mcParticles[iPart];
    if (currParticle->getMother() != NULL) continue;

    //Add primary particle (+ daughters) and the vertex
    addParticle(*currParticle, event, NULL, 0, true);
  }
}

G4PrimaryVertex*  MCParticleGenerator::determineVertex(const MCParticleGraph::GraphParticle& p, double& productionTimeShift)
{
  // We want to determine the simulation vertex for the given particle.
  // So lets see where it is
  B2Vector3D mcProdVtx = p.getProductionVertex();
  G4ThreeVector pos(mcProdVtx.X() / Unit::mm * CLHEP::mm,
                    mcProdVtx.Y() / Unit::mm * CLHEP::mm,
                    mcProdVtx.Z() / Unit::mm * CLHEP::mm);
  productionTimeShift = 0;
  // and make sure its inside the simulation volume
  if (m_topSolid && m_topSolid->Inside(pos) == kOutside) {
    // Ok, we're outside the simulation volume. Check if we can actually
    // get to the simulation volume in time, no fields outside the volume
    ROOT::Math::PxPyPzEVector mcPartMom4 = p.get4Vector();
    CLHEP::HepLorentzVector mom(mcPartMom4.X() / Unit::MeV * CLHEP::MeV,
                                mcPartMom4.Y() / Unit::MeV * CLHEP::MeV,
                                mcPartMom4.Z() / Unit::MeV * CLHEP::MeV,
                                mcPartMom4.E() / Unit::MeV * CLHEP::MeV);
    const auto dir = mom.vect().unit();
    double distance = m_topSolid->DistanceToIn(pos, dir);

    if (distance == kInfinity) {
      B2DEBUG(10, "Particle starts outside simulation volume and isn't intersecting, skipping");
      return nullptr;
    }

    const double speed = (p.getMass() == 0 ? mom.beta() : 1) * CLHEP::c_light;
    const double flightLength = speed * p.getLifetime();
    if (distance > flightLength) {
      B2DEBUG(10, "Particle starts outside simulation volume and doesn't reach it before decaying, skipping");
      return nullptr;
    }
    pos += dir * distance;
    productionTimeShift = (distance / speed);
  }
  return new G4PrimaryVertex(pos.x(), pos.y(), pos.z(), p.getProductionTime() + productionTimeShift);
}

void MCParticleGenerator::addParticle(const MCParticle& mcParticle,
                                      G4Event* event,
                                      G4PrimaryParticle* lastG4Mother,
                                      int motherIndex,
                                      bool useTime)
{
  G4PrimaryParticle* g4Mother = lastG4Mother;

  //Check if the particle should be added to Geant4
  //Only add particle if its PDG code is known to Geant4 and it is not flagged as virtual or initial.
  bool addToG4 = (!mcParticle.isVirtual()) && (!mcParticle.isInitial());

  //MS: distinguish optical photon from the rest of particles
  bool opticalPhoton = mcParticle.getPDG() == 0 && mcParticle.getEnergy() < 10.0 * Unit::eV;
  //SD: add neutral long-lived particle
  bool neutral_llp = mcParticle.getCharge() == 0.0 && mcParticle.getLifetime() > 0.0;

  G4ParticleDefinition* pdef = NULL;
  if (opticalPhoton) {
    pdef = G4OpticalPhoton::OpticalPhotonDefinition();
  } else {
    pdef = G4ParticleTable::GetParticleTable()->FindParticle(mcParticle.getPDG());
    if (pdef == NULL && neutral_llp) {
      pdef = G4ParticleTable::GetParticleTable()->FindParticle("LongLivedNeutralParticle");
    }
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
  graphParticle.setEnergy(mcParticle.getEnergy());
  graphParticle.setValidVertex(mcParticle.hasValidVertex());
  graphParticle.setProductionTime(mcParticle.getProductionTime());
  graphParticle.setDecayTime(mcParticle.getDecayTime());
  graphParticle.setProductionVertex(B2Vector3D(mcParticle.getProductionVertex()));
  graphParticle.setMomentum(mcParticle.getMomentum());
  graphParticle.setDecayVertex(mcParticle.getDecayVertex());
  graphParticle.setFirstDaughter(mcParticle.getFirstDaughter());
  graphParticle.setLastDaughter(mcParticle.getLastDaughter());
  //Add decay to MCParticle graph
  if (motherIndex > 0) graphParticle.comesFrom(m_mcParticleGraph[motherIndex - 1]);

  //Create a new Geant4 Primary particle and
  //store the link to the GraphMCParticle object as user info.
  G4PrimaryParticle* newPart = nullptr;
  // And maybe we need to generate a new primary Geant4 vertex for this particle.
  G4PrimaryVertex* vertex = nullptr;
  // And in the rare case that the particle started outside the simulation volume
  // we need an adjusted production time
  double productionTimeShift{0};

  // Ok, we want to add the particle and it doesn't have a mother so lets see
  // if we can find a proper vertex
  if (addToG4 and lastG4Mother == nullptr) {
    vertex = determineVertex(graphParticle, productionTimeShift);
    // we don't have a mother and no vertex so skip this particle
    if (not vertex) addToG4 = false;
  }
  // So now we have a particle to add and either a mother or a vertex.
  // Let's create the particle in Geant4
  if (addToG4) {
    const ROOT::Math::PxPyPzEVector mcPartMom4 = graphParticle.get4Vector();
    newPart = new G4PrimaryParticle(pdef,
                                    mcPartMom4.X() / Unit::MeV * CLHEP::MeV,
                                    mcPartMom4.Y() / Unit::MeV * CLHEP::MeV,
                                    mcPartMom4.Z() / Unit::MeV * CLHEP::MeV,
                                    mcPartMom4.E() / Unit::MeV * CLHEP::MeV);
    newPart->SetMass(graphParticle.getMass() / Unit::MeV * CLHEP::MeV);
    if (opticalPhoton) {
      TVector3 polarization = graphParticle.getDecayVertex(); // temporary stored here
      newPart->SetPolarization(polarization.X(), polarization.Y(), polarization.Z());
    }
    newPart->SetUserInformation(new ParticleInfo(graphParticle));

    //Set propagation time only if useTime is true, the MCparticle has a valid vertex and has children.
    useTime &= graphParticle.hasValidVertex() && mcParticle.getFirstDaughter() > 0;
    if (useTime) {
      //ProperTime is in particle eigentime, so convert lab lifetime to eigentime
      //correct for any possible shift between simulation vertex and generation vertex
      double propertime = (graphParticle.getLifetime() - productionTimeShift) / mcPartMom4.Gamma();
      newPart->SetProperTime(propertime);
    }

    if (lastG4Mother) {
      lastG4Mother->SetDaughter(newPart);
      // This particle will be produced at the decay vertex of its mother so we **want** Geant4 to
      // overwrite production time/vertex, so as convention having NaN here means we'll
      // take over the values from Geant4 instead of the ones from the generator
      graphParticle.setProductionTime(std::numeric_limits<float>::quiet_NaN());
    } else {
      vertex->SetPrimary(newPart);
      event->AddPrimaryVertex(vertex);
    }

    g4Mother = newPart;

    //Do not store the generator info in the MCParticles block unless Geant4 creates a track in the detector.
    graphParticle.setIgnore();

  } else {
    B2DEBUG(100, "The particle " << mcParticle.getIndex() << " (PDG " << mcParticle.getPDG() << ") was not added to Geant4");
  }

  //Add all children
  int currMotherIndex = m_mcParticleGraph.size();
  for (MCParticle* daughter : mcParticle.getDaughters()) {
    addParticle(*daughter, event, g4Mother, currMotherIndex, useTime);
  }
}


//===================================================================
//                       Protected methods
//===================================================================
