/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>

#include <analysis/modules/TwoBodyISRPhotonCorrector/TwoBodyISRPhotonCorrectorModule.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ParticleCopy.h>
#include <TDatabasePDG.h>

#include <algorithm>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(TwoBodyISRPhotonCorrector);

TwoBodyISRPhotonCorrectorModule::TwoBodyISRPhotonCorrectorModule() : Module()
{
  //Set module properties
  setDescription("This module calculates the energy and momentum of photons in single ISR events based on the beam energy, photon direction, and mass of the recoil particle. A copy of the photon list must be set before using this module. The copy should be filled with the new kinematics.");
//  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameter definition
  addParam("gammaList", m_gammaList, "Name of photon list containing the ISR gammas to be corrected");
  addParam("massiveParticle", m_massiveParticle,
           "Name of the massive particle participating in the two body decay with the ISR gamma.");
}

void TwoBodyISRPhotonCorrectorModule::initialize()
{
  StoreObjPtr<ParticleList>().isRequired(m_gammaList);
  StoreArray<Belle2::Particle> particles;
  particles.isRequired();

  TParticlePDG* massiveParticlePDG = TDatabasePDG::Instance()->GetParticle(m_massiveParticle);
  B2INFO("PDG: " << m_massiveParticle <<
         " selected for massive particle participating in two body decay. It's mass is " <<
         massiveParticlePDG->Mass() << " GeV/c^2.");

}

void TwoBodyISRPhotonCorrectorModule::event()
{

  StoreArray<Particle> particles;
  StoreObjPtr<ParticleList> plist(m_gammaList);

  if (!plist) {
    B2ERROR("ParticleList " << m_gammaList << " not found");
    return;
  }

  if (plist->getPDGCode() != 22) {
    B2ERROR("TwoBodyISRPhotonCorrectorModule::event ParticleList " << m_gammaList << " is not a gamma list");
    return;
  }

  double mMassiveParticle = TDatabasePDG::Instance()->GetParticle(m_massiveParticle)->Mass();

  PCmsLabTransform T;
  ROOT::Math::PxPyPzEVector Pbeam, Pgamma, Pcorrected;

  Pbeam = T.getBeamFourMomentum();

  //ROOT::Math::PxPyPzEVector daughters4Vector;

  const unsigned int numParticles = plist->getListSize();
  for (unsigned int i = 0; i < numParticles; i++) {
    // Get gamma and it's 4 momentum
    Particle* iParticle = plist->getParticle(i);

    Pgamma = iParticle->get4Vector();

    // Calculate corrected energy for photon in list
    ROOT::Math::XYZVector pgamma_versor((Pgamma.Vect()).Unit());
    ROOT::Math::XYZVector pbeam(Pbeam.Vect());
    double E_corrected = 0.5 * (Pbeam.mag2() - mMassiveParticle * mMassiveParticle) / (Pbeam.E() - pbeam.Dot(pgamma_versor));
    Pcorrected.SetPxPyPzE(pgamma_versor.X()*E_corrected, pgamma_versor.Y()*E_corrected,
                          pgamma_versor.Z()*E_corrected, E_corrected);

    // Set particle's new 4 momentum
    iParticle->set4Vector(Pcorrected);
  }
}

void TwoBodyISRPhotonCorrectorModule::terminate()
{
}

