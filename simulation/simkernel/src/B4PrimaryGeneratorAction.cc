/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/simkernel/B4PrimaryGeneratorAction.h>
#include <simulation/simkernel/B4PrimaryGeneratorMessenger.h>

#include "G4ParticleDefinition.hh"
#include "G4ParticleMomentum.hh"
#include "Randomize.hh"
#include "G4HEPEvtInterface.hh"

using namespace Belle2;

B4PrimaryGeneratorAction::B4PrimaryGeneratorAction()
{
  //-------------------------------------------------------------
  // Initialize and set default value for each member variable.
  //-------------------------------------------------------------
  m_nParticle = 1;
  m_particleName = "pi-";
  m_minCos = -0.8;
  m_maxCos = 0.8;
  m_phiStart = 0.;
  m_phiEnd = 360.;
  m_pMomentum = 1.;
  m_deltaP = 0.;
  m_posX = 0;
  m_posY = 0;
  m_posZ = 0;

  m_generatorName = "tester";

  m_HEPEvt = 0;

  m_particleGun = new G4ParticleGun(1);

  //--------------------------------------------
  // TESTER parameters passed by this messenger
  //--------------------------------------------
  m_messenger = new B4PrimaryGeneratorMessenger(this);
}

B4PrimaryGeneratorAction::~B4PrimaryGeneratorAction()
{
  if (m_particleGun) delete m_particleGun;
  if (m_messenger)   delete m_messenger;
  if (m_HEPEvt)      delete m_HEPEvt;
}

void B4PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  if (m_generatorName == "tester") {
    std::cout << " Generator \"tester\" is called. " << std::endl;

    // Set particle type
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition* particle
    = particleTable->FindParticle(m_particleName);
    m_particleGun->SetParticleDefinition(particle);

    // Set particle position
    m_particleGun->SetParticlePosition(G4ThreeVector(m_posX, m_posY, m_posZ));

    //--------------------------------------------
    // Loop over all particles and set parameters
    //--------------------------------------------
    for (G4int i = 0; i < m_nParticle; i++) {
      // Set particle momentum
      G4double pMag = m_pMomentum;

      // Randomize p
      if (m_deltaP > 0.) pMag = m_pMomentum - m_deltaP * (1.0 - 2.0 * G4UniformRand());
      pMag = pMag * GeV;

      // Randomize cos(theta)
      G4double costheta = m_minCos + (m_maxCos - m_minCos) * G4UniformRand();

      // Randomize phi
      G4double phi = m_phiStart + (m_phiEnd - m_phiStart) * G4UniformRand();
      phi = phi * degree;

      //computer 3-vector momentum
      G4double sintheta = sqrt(1. - costheta * costheta);
      G4ParticleMomentum aMomentum;
      aMomentum[0] = pMag * sintheta * cos(phi);
      aMomentum[1] = pMag * sintheta * sin(phi);
      aMomentum[2] = pMag * costheta;

      //use ParticleGun to generate event
      m_particleGun->SetParticleMomentum(aMomentum);
      m_particleGun->GeneratePrimaryVertex(anEvent);
    }
  } else if (m_generatorName == "HEPEvt") {

    // If no HEPEvt pointer, create one.
    if (!m_HEPEvt) {
      std::cout << " Generator \"HEPEvt\" is called. " << std::endl;
      m_HEPEvt = new G4HEPEvtInterface(m_HEPEvtFileName);
    }

    m_HEPEvt->GeneratePrimaryVertex(anEvent);
  }
}

