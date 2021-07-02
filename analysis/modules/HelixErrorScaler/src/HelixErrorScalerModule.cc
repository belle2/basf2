/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: H. Tanigawa                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/HelixErrorScaler/HelixErrorScalerModule.h>

#include <analysis/DecayDescriptor/ParticleListName.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Const.h>

#include <vector>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HelixErrorScaler)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HelixErrorScalerModule::HelixErrorScalerModule() : Module(), m_pdgCode(0)
{
  // Set module properties
  setDescription(R"DOC("scale the error of helix parameters

    Creates a new charged particle list whose helix errors are scaled by constant factors.
    Parameters (a, b) can be set to define impact parameter resolution,
    which limits d0 and z0 errors so that they do not shrink below the resolution.
     )DOC");

  // Parameter definitions
  addParam("inputListName", m_inputListName, "The name of input charged particle list", std::string(""));
  addParam("outputListName", m_outputListName, "The name of output charged particle list", std::string(""));
  addParam("scaleFactors", m_scaleFactors, "vector of five scale factors for helix parameter errors", {1.0, 1.0, 1.0, 1.0, 1.0});
  addParam("d0ResolutionParameters", m_d0ResolPars, "d0 resolution parameters", {0.0, 0.0});
  addParam("z0ResolutionParameters", m_z0ResolPars, "z0 resolution parameters", {0.0, 0.0});

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
  if (Const::chargedStableSet.find(abs(m_pdgCode)) == Const::invalidParticle)
    B2ERROR("Invalid input ParticleList PDG code (must be ChargedStable): " << m_pdgCode);
  m_outputAntiListName = ParticleListName::antiParticleListName(m_outputListName);

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
  m_outputAntiparticleList.registerInDataStore(m_outputAntiListName);

  m_particles.registerRelationTo(m_pidlikelihoods);
  m_particles.registerRelationTo(m_trackfitresults);
}

void HelixErrorScalerModule::event()
{
  RelationArray particlesToMCParticles(m_particles, m_mcparticles);

  // new output particle list
  m_outputparticleList.create();
  m_outputparticleList->initialize(m_pdgCode, m_outputListName);
  m_outputAntiparticleList.create();
  m_outputAntiparticleList->initialize(-1 * m_pdgCode, m_outputAntiListName);
  m_outputAntiparticleList->bindAntiParticleList(*(m_outputparticleList));

  // loop over charged particles
  const unsigned int nPar = m_inputparticleList->getListSize();
  for (unsigned i = 0; i < nPar; i++) {
    const Particle* charged = m_inputparticleList->getParticle(i);
    TrackFitResult* new_trkfit = getTrackFitResultWithScaledError(charged);

    Const::ChargedStable chargedtype(abs(charged->getPDGCode()));

    Particle new_charged(charged->getMdstArrayIndex(), new_trkfit, chargedtype);
    Particle* newCharged = m_particles.appendNew(new_charged);
    const PIDLikelihood* pid = charged->getPIDLikelihood();
    const MCParticle* mcCharged = charged->getRelated<MCParticle>();
    newCharged->addRelationTo(new_trkfit);
    if (pid) newCharged->addRelationTo(pid);
    if (mcCharged != nullptr) newCharged->addRelationTo(mcCharged);

    m_outputparticleList->addParticle(newCharged);

  } // loop over the charged particles
}

TrackFitResult* HelixErrorScalerModule::getTrackFitResultWithScaledError(const Particle* particle)
{
  const TrackFitResult* trkfit = particle->getTrackFitResult();
  const std::vector<float>  helix  = trkfit->getTau();
  const std::vector<float>  cov5   = trkfit->getCov();
  const Const::ParticleType ptype  = trkfit->getParticleType();
  const double              pvalue = trkfit->getPValue();
  const ULong64_t           hitCDC = trkfit->getHitPatternCDC().getInteger();
  const ULong64_t           hitVXD = trkfit->getHitPatternVXD().getInteger();
  const int                 ndf    = trkfit->getNDF();

  // d0, z0 resolution = a (+) b / pseudo-momentum
  TVector3 p = particle->getMomentum();
  double sinTheta = TMath::Sin(p.Theta());
  double pD0 = p.Mag2() / (particle->getEnergy()) * TMath::Power(sinTheta, 1.5); // p*beta*sinTheta**1.5
  double pZ0 = pD0 * sinTheta; // p*beta*sinTheta**2.5
  double d0Resol = TMath::Sqrt(TMath::Power(m_d0ResolPars[0], 2) + TMath::Power(m_d0ResolPars[1] / pD0, 2));
  double z0Resol = TMath::Sqrt(TMath::Power(m_z0ResolPars[0], 2) + TMath::Power(m_z0ResolPars[1] / pZ0, 2));
  double d0Err = TMath::Sqrt(trkfit->getCovariance5()[0][0]);
  double z0Err = TMath::Sqrt(trkfit->getCovariance5()[3][3]);

  double scaleFactors[5] = { TMath::Max(d0Resol / d0Err, m_scaleFactors[0]),
                             m_scaleFactors[1],
                             m_scaleFactors[2],
                             TMath::Max(z0Resol / z0Err, m_scaleFactors[3]),
                             m_scaleFactors[4]
                           };

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
