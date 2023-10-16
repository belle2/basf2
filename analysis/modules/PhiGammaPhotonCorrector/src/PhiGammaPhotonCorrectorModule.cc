/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>

#include <analysis/modules/PhiGammaPhotonCorrector/PhiGammaPhotonCorrectorModule.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ParticleCopy.h>

#include <algorithm>

using namespace std;
using namespace Belle2;

// Register module in the framework
REG_MODULE(PhiGammaPhotonCorrector);

PhiGammaPhotonCorrectorModule::PhiGammaPhotonCorrectorModule() : Module()
{
  //Set module properties
  setDescription("This module calculates and resets the energy and momentum of photons in phi gamma events based on the beam energy and the mass of the phi meson. Note that the photon's original 4 momentum is replaced, so selection cuts should be made before calling this module.");
//  setPropertyFlags(c_ParallelProcessingCertified);
  //Parameter definition
  addParam("particleList", m_particleList, "Name of photon list containing the ISR gammas to be corrected");
}

void PhiGammaPhotonCorrectorModule::initialize()
{
  StoreObjPtr<ParticleList>().isRequired(m_particleList);
  StoreArray<Belle2::Particle> particles;
  particles.isRequired();


}

void PhiGammaPhotonCorrectorModule::event()
{

  StoreArray<Particle> particles;
  StoreObjPtr<ParticleList> plist(m_particleList);

  if (!plist) {
    B2ERROR("ParticleList " << m_particleList << " not found");
    return;
  }

  if (plist->getPDGCode() != 22) {
    B2ERROR("PhiGammaPhotonCorrectorModule::event ParticleList " << m_particleList << " is not a gamma list");
    return;
  }

  double mPhiPDG = 1.019461; // GeV/c^2 **Should find a way to standardize this
  PCmsLabTransform T;
  TLorentzVector Pbeam, Pgamma, Pcorrected;
  Pbeam[0] = T.getBeamFourMomentum().Px();
  Pbeam[1] = T.getBeamFourMomentum().Py();
  Pbeam[2] = T.getBeamFourMomentum().Pz();
  Pbeam[3] = T.getBeamFourMomentum().E();
  //ROOT::Math::PxPyPzEVector daughters4Vector;

  const unsigned int numParticles = plist->getListSize();
  for (unsigned int i = 0; i < numParticles; i++) {
    // Get gamma and it's 4 momentum
    Particle* iParticle = plist->getParticle(i);

    Pgamma[0] = iParticle->getMomentum().x();
    Pgamma[1] = iParticle->getMomentum().y();
    Pgamma[2] = iParticle->getMomentum().z();
    Pgamma[3] = iParticle->getEnergy();

    // Calculate corrected energy for photon in list
    TVector3 pgamma_versor((Pgamma.Vect()).Unit());
    TVector3 pbeam(Pbeam.Vect());
    double E_corrected = 0.5 * (Pbeam.Mag2() - mPhiPDG * mPhiPDG) / (Pbeam.E() - pbeam.Dot(pgamma_versor));
    Pcorrected.SetPxPyPzE(pgamma_versor.X()*E_corrected, pgamma_versor.Y()*E_corrected,
                          pgamma_versor.Z()*E_corrected, E_corrected);

    // Set particle's new 4 momentum
    ROOT::Math::PxPyPzEVector P_gamma_corrected;
    P_gamma_corrected = {Pcorrected[0],
                         Pcorrected[1],
                         Pcorrected[2],
                         Pcorrected[3]
                        };
    iParticle->set4Vector(P_gamma_corrected);
  }
}

void PhiGammaPhotonCorrectorModule::terminate()
{
}

