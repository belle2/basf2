/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/HelixErrorScaler/HelixErrorScalerModule.h>

#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/utility/ParticleCopy.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Const.h>
#include <framework/geometry/B2Vector3.h>

#include <Math/Vector3D.h>
#include <vector>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HelixErrorScaler);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HelixErrorScalerModule::HelixErrorScalerModule() : Module(), m_pdgCode(0), m_scaleKshort(false)
{
  // Set module properties
  setDescription(R"DOC(scale the error of helix parameters

Creates a new charged particle list whose helix errors are scaled by constant factors.
Different sets of scale factors are defined for tracks with/without a PXD hit.
For tracks with a PXD hit, in order to avoid severe underestimation of d0 and z0 errors,
lower limits (best resolution) can be set in a momentum-dependent form.
The module also accepts a V0 Kshort particle list as input and applies the error correction to its daughters.
Note the difference in impact parameter resolution between V0 daughters and tracks from IP,
as V0 daughters are free from multiple scattering through the beam pipe.
     )DOC");

  // Parameter definitions
  addParam("inputListName", m_inputListName, "The name of input particle list (charged stable or V0 Kshort)", std::string(""));
  addParam("outputListName", m_outputListName, "The name of output charged particle list", std::string(""));
  addParam("scaleFactors_PXD", m_scaleFactors_PXD,
           "vector of five scale factors for helix parameter errors (for tracks with a PXD hit)", {1.0, 1.0, 1.0, 1.0, 1.0});
  addParam("scaleFactors_noPXD", m_scaleFactors_noPXD,
           "vector of five scale factors for helix parameter errors (for tracks without a PXD hit)", {1.0, 1.0, 1.0, 1.0, 1.0});
  addParam("d0ResolutionParameters", m_d0ResolPars, "d0 best resolution parameters", {0.0, 0.0});
  addParam("z0ResolutionParameters", m_z0ResolPars, "z0 best resolution parameters", {0.0, 0.0});
  addParam("d0MomentumThreshold", m_d0MomThr, "d0 best resolution is kept constant below this momentum.", 0.0);
  addParam("z0MomentumThreshold", m_z0MomThr, "z0 best resolution is kept constant below this momentum.", 0.0);

}

void HelixErrorScalerModule::initialize()
{
  // check the validity of output ParticleList name
  bool valid = m_decaydescriptor.init(m_outputListName);
  if (!valid)
    B2ERROR("Invalid output ParticleList name: " << m_outputListName);

  // output particle
  const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();
  m_pdgCode  = mother->getPDGCode();
  if (m_pdgCode == Const::Kshort.getPDGCode()) {
    m_scaleKshort = true;
  } else  if (Const::chargedStableSet.find(abs(m_pdgCode)) == Const::invalidParticle) {
    B2ERROR("Invalid input ParticleList PDG code (must be ChargedStable): " << m_pdgCode);
  }

  // get existing particle lists
  if (m_inputListName == m_outputListName) {
    B2ERROR("Input and output particle list names are the same: " << m_inputListName);
  } else if (!m_decaydescriptor.init(m_inputListName)) {
    B2ERROR("Invalid input particle list name: " << m_inputListName);
  } else {
    m_inputparticleList.isRequired(m_inputListName);
  }

  // make output list
  m_outputparticleList.registerInDataStore(m_outputListName);
  if (! m_scaleKshort) {
    m_outputAntiListName = ParticleListName::antiParticleListName(m_outputListName);
    m_outputAntiparticleList.registerInDataStore(m_outputAntiListName);
  }

  m_particles.registerRelationTo(m_pidlikelihoods);
  m_particles.registerRelationTo(m_trackfitresults);
}

void HelixErrorScalerModule::event()
{
  RelationArray particlesToMCParticles(m_particles, m_mcparticles);

  // new output particle list
  if (! m_outputparticleList.isValid()) {
    m_outputparticleList.create();
    m_outputparticleList->initialize(m_pdgCode, m_outputListName);
  }

  if (! m_scaleKshort) {
    m_outputAntiparticleList.create();
    m_outputAntiparticleList->initialize(-1 * m_pdgCode, m_outputAntiListName);
    m_outputAntiparticleList->bindAntiParticleList(*(m_outputparticleList));
  }

  if (m_scaleKshort) { // scale V0 Kshort

    // loop over Kshort
    const unsigned int nPar = m_inputparticleList->getListSize();
    for (unsigned i = 0; i < nPar; i++) {
      const Particle* particle = m_inputparticleList->getParticle(i);
      if (particle->getParticleSource() != Particle::EParticleSourceObject::c_V0) {
        B2WARNING(" Input ParticleList " << m_inputListName <<
                  " contains a particle which is not from V0. It will not be copied to output list.");
        continue;
      }

      if (particle->getNDaughters() != 2)
        B2ERROR("V0 particle should have exactly two daughters");

      Particle* newV0 = ParticleCopy::copyParticle(particle);

      const Particle* dauP = newV0->getDaughter(0);
      const Particle* dauM = newV0->getDaughter(1);

      Particle* newDauP = getChargedWithScaledError(dauP);
      Particle* newDauM = getChargedWithScaledError(dauM);

      ROOT::Math::PxPyPzEVector v0Momentum = newDauP->get4Vector() + newDauM->get4Vector();
      newV0->set4VectorDividingByMomentumScaling(v0Momentum);

      newV0->replaceDaughter(dauP, newDauP);
      newV0->replaceDaughter(dauM, newDauM);

      m_outputparticleList->addParticle(newV0);

    } // loop over Kshort

  } else { // scale charged particles

    // loop over charged particles
    const unsigned int nPar = m_inputparticleList->getListSize();
    for (unsigned i = 0; i < nPar; i++) {
      const Particle* charged = m_inputparticleList->getParticle(i);
      Particle* newCharged = getChargedWithScaledError(charged);
      m_outputparticleList->addParticle(newCharged);
    }
  }

}

Particle* HelixErrorScalerModule::getChargedWithScaledError(const Particle* particle)
{
  const TrackFitResult* new_trkfit = getTrackFitResultWithScaledError(particle);

  Const::ChargedStable chargedtype(abs(particle->getPDGCode()));
  Particle new_charged(particle->getMdstArrayIndex(), new_trkfit, chargedtype);
  Particle* newCharged = m_particles.appendNew(new_charged);

  // add relation to PID, MCParticle (if any) and TrackFitResult
  const PIDLikelihood* pid = particle->getPIDLikelihood();
  const MCParticle* mcCharged = particle->getRelated<MCParticle>();
  newCharged->addRelationTo(new_trkfit);
  if (pid) newCharged->addRelationTo(pid);
  if (mcCharged != nullptr) newCharged->addRelationTo(mcCharged);

  return newCharged;
}

const TrackFitResult* HelixErrorScalerModule::getTrackFitResultWithScaledError(const Particle* particle)
{
  const TrackFitResult* trkfit = particle->getTrackFitResult();

  const std::vector<float>  helix  = trkfit->getTau();
  const std::vector<float>  cov5   = trkfit->getCov();
  const Const::ParticleType ptype  = trkfit->getParticleType();
  const double              pvalue = trkfit->getPValue();
  const ULong64_t           hitCDC = trkfit->getHitPatternCDC().getInteger();
  const ULong64_t           hitVXD = trkfit->getHitPatternVXD().getInteger();
  const int                 ndf    = trkfit->getNDF();

  std::vector<double> scaleFactors = getScaleFactors(particle, trkfit);
  std::vector<float> cov5_scaled;
  unsigned int counter = 0;
  for (unsigned int j = 0; j < 5; j++) {
    for (unsigned int k = j; k < 5; k++) {
      cov5_scaled.push_back(cov5[counter++] * scaleFactors[j] * scaleFactors[k]);
    }
  }
  TrackFitResult* new_trkfit = m_trackfitresults.appendNew(helix, cov5_scaled, ptype, pvalue, hitCDC, hitVXD, ndf);
  return new_trkfit;
}

std::vector<double> HelixErrorScalerModule::getScaleFactors(const Particle* particle, const TrackFitResult* trkfit)
{
  if (trkfit->getHitPatternVXD().getNPXDHits() > 0) {

    // d0, z0 resolution = a (+) b / pseudo-momentum
    B2Vector3D p = particle->getMomentum();
    double sinTheta = TMath::Sin(p.Theta());
    double pD0 = p.Mag2() / (particle->getEnergy()) * TMath::Power(sinTheta, 1.5); // p*beta*sinTheta**1.5
    double pZ0 = pD0 * sinTheta; // p*beta*sinTheta**2.5

    pD0 = TMath::Max(pD0, m_d0MomThr); // if pD0 is smaller than the threshold, *overwrite* it with the threshold.
    pZ0 = TMath::Max(pZ0, m_z0MomThr); // if pZ0 is smaller than the threshold, *overwrite* it with the threshold.
    double d0Resol = TMath::Sqrt(TMath::Power(m_d0ResolPars[0], 2) + TMath::Power(m_d0ResolPars[1] / pD0, 2));
    double z0Resol = TMath::Sqrt(TMath::Power(m_z0ResolPars[0], 2) + TMath::Power(m_z0ResolPars[1] / pZ0, 2));
    double d0Err = TMath::Sqrt(trkfit->getCovariance5()[0][0]);
    double z0Err = TMath::Sqrt(trkfit->getCovariance5()[3][3]);

    std::vector<double> scaleFactors = { TMath::Max(d0Resol / d0Err, m_scaleFactors_PXD[0]),
                                         m_scaleFactors_PXD[1],
                                         m_scaleFactors_PXD[2],
                                         TMath::Max(z0Resol / z0Err, m_scaleFactors_PXD[3]),
                                         m_scaleFactors_PXD[4]
                                       };
    return scaleFactors;
  } else {
    return m_scaleFactors_noPXD;
  }
}



