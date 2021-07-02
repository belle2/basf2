/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <online_book/awesome/simulation/AWESOMESensitiveDetector.h>

/* Belle2 headers. */
#include <framework/gearbox/Unit.h>
#include <framework/geometry/B2Vector3.h>

/* Geant4 headers. */
#include <G4StepPoint.hh>
#include <G4Track.hh>

/* CLHEP headers. */
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Vector/ThreeVector.h>

using namespace Belle2;
using namespace Belle2::AWESOME;

AWESOMESensitiveDetector::AWESOMESensitiveDetector() :
  Simulation::SensitiveDetectorBase{"AwesomeSensitiveDetector", Const::EDetector::TEST}
{
  /* MCParticles must be optional, not required. */
  m_MCParticles.isOptional();
  /* Register the simulted hits and all the necessary relations in the datastore. */
  m_SimHits.registerInDataStore();
  m_MCParticles.registerRelationTo(m_SimHits);
  /*
   * Register the Relation so that the Geant4 TrackIDs get replaced by the actual
   * MCParticle indices after simulating the events. This is needed as
   * secondary particles might not be stored so everything relating to those
   * particles will be attributed to the last saved mother particle.
  */
  registerMCParticleRelation(m_MCParticlesToSimHits);
}

bool AWESOMESensitiveDetector::step(G4Step* step, G4TouchableHistory*)
{
  /* Get some basic information from Geant4. */
  const G4Track& track   = *step->GetTrack();
  const int trackID      = track.GetTrackID();
  const double energyDep = step->GetTotalEnergyDeposit() * Unit::MeV;
  G4StepPoint* preStep   = step->GetPreStepPoint();
  G4StepPoint* postStep  = step->GetPostStepPoint();
  /* Ignore everything below 1 eV. */
  if (energyDep < Unit::eV)
    return false;
  /*
   * Compute other useful quantities to be stored.
   * We must be sure that the quantities are stored using the proper Belle2 units:
   * positions in cm, time in ns, etc.
   */
  const CLHEP::Hep3Vector position = 0.5 * (preStep->GetPosition() + postStep->GetPosition()) / CLHEP::cm; // Now in cm
  const double time = 0.5 * (preStep->GetGlobalTime() + postStep->GetGlobalTime()); // Already in ns
  /* Store the simulated hit. */
  AWESOMESimHit* simHit = m_SimHits.appendNew();
  simHit->setEnergyDep(energyDep);
  simHit->setPosition(B2Vector3{position.x(), position.y(), position.z()});
  simHit->setTime(time);
  /*
   * Add a relation between the current MCParticle and the simulated hit.
   * Since the MCParticle index is not yet defined we use the trackID from Geant4.
   */
  m_MCParticlesToSimHits.add(trackID, simHit->getArrayIndex());
  return true;
}
