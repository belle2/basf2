/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>

#include <analysis/modules/TwoBodyISRPhotonCorrector/TwoBodyISRPhotonCorrectorModule.h>

#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ParticleCopy.h>
#include <TDatabasePDG.h>

#include <algorithm>

using namespace Belle2;

// Register module in the framework
REG_MODULE(TwoBodyISRPhotonCorrector);

TwoBodyISRPhotonCorrectorModule::TwoBodyISRPhotonCorrectorModule() : Module()
{
  //Set module properties
  setDescription("This module corrects the energy and momentum of high energy ISR photons in single ISR events based on the beam energy, photon direction, and mass of the recoil particle. The corrected photons are stored in a new list, the original photon kinematics can be accessed via the originalParticle() metavariable.");
  //Parameter definition
  addParam("inputGammaList", m_inputGammaListName, "Name of photon list containing the ISR gammas to be corrected");
  addParam("outputGammaList", m_outputGammaListName, "Name of photon list containing the corrected ISR gammas");
  addParam("massiveParticlePDGCode", m_massiveParticlePDGCode,
           "Name of the massive particle participating in the two body decay with the ISR gamma.");
}

void TwoBodyISRPhotonCorrectorModule::initialize()
{
  StoreArray<Particle>().isRequired();
  DecayDescriptor inputDD, outputDD;

  // Output list checks
  bool outputValid = outputDD.init(m_outputGammaListName);
  if (!outputValid)
    B2ERROR("Invalid output ParticleList: " << m_outputGammaListName);
  if (outputDD.getMother()->getPDGCode() != Const::photon.getPDGCode())
    B2ERROR("TwoBodyISRPhotonCorrectorModule::event ParticleList " << m_outputGammaListName << " is not a gamma list");

  // Input list checks
  if (m_inputGammaListName == m_outputGammaListName) {
    B2ERROR("TwoBodyISRPhotonCorrectorModule: cannot copy Particles from " << m_inputGammaListName <<
            " to itself!");
  } else if (!inputDD.init(m_inputGammaListName)) {
    B2ERROR("Invalid input ParticleList name: " << m_inputGammaListName);
  } else {
    if (inputDD.getMother()->getPDGCode() != Const::photon.getPDGCode())
      B2ERROR("TwoBodyISRPhotonCorrectorModule::event ParticleList " << m_inputGammaListName << " is not a gamma list");

    StoreObjPtr<ParticleList>().isRequired(m_inputGammaListName);
  }

  m_outputGammaList.registerInDataStore(m_outputGammaListName, DataStore::c_DontWriteOut);

  TParticlePDG* massiveParticlePDG = TDatabasePDG::Instance()->GetParticle(m_massiveParticlePDGCode);
  B2DEBUG(19, "TwoBodyISRPhotonCorrectorModule: PDG code " << m_massiveParticlePDGCode <<
          " selected for massive particle participating in two body decay. Its mass is " <<
          massiveParticlePDG->Mass() << " GeV/c^2.");

}

void TwoBodyISRPhotonCorrectorModule::event()
{
  StoreObjPtr<ParticleList> inputGammaList(m_inputGammaListName);

  bool existingList = m_outputGammaList.isValid();
  if (!existingList) {
    // new particle list: create it
    m_outputGammaList.create();
    m_outputGammaList->initialize(Const::photon.getPDGCode(), m_outputGammaListName);
  } else {
    B2WARNING("TwoBodyISRPhotonCorrectorModule: Output ParticleList " << m_outputGammaListName << " already exists. Overwriting.");
    m_outputGammaList->clear();
  }

  // Initialize the mass to which we are constraining and the beam momentum
  const double massMassiveParticle = TDatabasePDG::Instance()->GetParticle(m_massiveParticlePDGCode)->Mass();
  PCmsLabTransform T;
  ROOT::Math::PxPyPzEVector P4beam = T.getBeamFourMomentum();

  // Loop through the given gamma list
  const unsigned int numParticles = inputGammaList->getListSize();
  for (unsigned int i = 0; i < numParticles; i++) {
    ROOT::Math::PxPyPzEVector P4corrected;

    // Get gamma and it's 4 momentum
    Particle* iParticle = inputGammaList->getParticle(i);
    ROOT::Math::PxPyPzEVector P4gamma = iParticle->get4Vector();

    // Calculate corrected energy for photon in list
    ROOT::Math::XYZVector p3gamma_unit((P4gamma.Vect()).Unit());
    ROOT::Math::XYZVector pbeam(P4beam.Vect());
    double E_corrected = 0.5 * (P4beam.mag2() - massMassiveParticle * massMassiveParticle) /
                         (P4beam.E() - pbeam.Dot(p3gamma_unit));
    P4corrected.SetPxPyPzE(p3gamma_unit.X()*E_corrected, p3gamma_unit.Y()*E_corrected,
                           p3gamma_unit.Z()*E_corrected, E_corrected);

    // Set particle's new 4 momentum in a copy
    Particle* correctedP = ParticleCopy::copyParticle(iParticle);
    correctedP->setMomentumScalingFactor(1.0);
    correctedP->set4Vector(P4corrected);

    m_outputGammaList->addParticle(correctedP);
  }

  const unsigned int numCopy = m_outputGammaList->getListSize();
  if (numCopy != numParticles)
    B2FATAL("Size of the ParticleList " << m_inputGammaListName
            << " has changed while copying the Particles! original size = "
            << numParticles << " vs. new size = " << numCopy);

}


