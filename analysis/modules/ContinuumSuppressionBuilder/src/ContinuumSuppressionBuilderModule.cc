/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/ContinuumSuppressionBuilder/ContinuumSuppressionBuilderModule.h>

#include <analysis/ContinuumSuppression/Thrust.h>
#include <analysis/ContinuumSuppression/KsfwMoments.h>
#include <analysis/ContinuumSuppression/FoxWolfram.h>
#include <analysis/ContinuumSuppression/CleoCones.h>

#include <analysis/dataobjects/RestOfEvent.h>

#include <analysis/utility/CLHEPToROOT.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ROOTToCLHEP.h>

#include <analysis/VertexFitting/KFit/MakeMotherKFit.h>
#include <analysis/VertexFitting/KFit/VertexFitKFit.h>

#include <framework/dataobjects/Helix.h>

#include <framework/geometry/BFieldManager.h>

#include <mdst/dataobjects/TrackFitResult.h>

#include <vector>
#include <Math/Vector3D.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ContinuumSuppressionBuilder);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ContinuumSuppressionBuilderModule::ContinuumSuppressionBuilderModule() : Module()
{
  // Set module properties
  setDescription("Creates for each Particle in the given ParticleLists a ContinuumSuppression dataobject and makes basf2 relation between them.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("particleList", m_particleListName, "Name of the ParticleList", std::string(""));
  addParam("ROEMask", m_ROEMask, "ROE mask", std::string(RestOfEvent::c_defaultMaskName));
  addParam("performIPProfileFit", m_ipProfileFit, "switch to turn on vertex fit of tracks with IP profile constraint", false);
}

void ContinuumSuppressionBuilderModule::initialize()
{
  // Input
  m_plist.isRequired(m_particleListName);
  StoreArray<Particle>().isRequired();

  if (m_ROEMask.empty()) {
    m_ROEMask = RestOfEvent::c_defaultMaskName;
  }

  if (m_ROEMask == "FS1" or m_ROEMask == "ROE") {
    B2ERROR("The ROE mask for the continuum suppression must not be called " << m_ROEMask);
  }

  // Output
  m_csarray.registerInDataStore(m_ROEMask);
  StoreArray<Particle>().registerRelationTo(m_csarray);

  // set magnetic field
  m_Bfield = BFieldManager::getFieldInTesla(ROOT::Math::XYZVector(0, 0, 0)).Z();
}

void ContinuumSuppressionBuilderModule::event()
{
  m_BeamSpotCenter = m_beamSpotDB->getIPPosition();
  m_beamSpotCov.ResizeTo(3, 3);
  m_beamSpotCov = m_beamSpotDB->getCovVertex();

  for (unsigned i = 0; i < m_plist->getListSize(); i++) {
    addContinuumSuppression(m_plist->getParticle(i));
  }
}

void ContinuumSuppressionBuilderModule::addContinuumSuppression(const Particle* particle)
{
  // Create ContinuumSuppression object
  ContinuumSuppression* qqVars = m_csarray.appendNew();

  // Create relation: Particle <-> ContinuumSuppression
  particle->addRelationTo(qqVars);

  std::vector<ROOT::Math::PxPyPzEVector> p_cms_sigB, p_cms_roe, p_cms_all;

  std::vector<std::pair<ROOT::Math::PxPyPzEVector, int>> p_cms_q_sigA;
  std::vector<std::pair<ROOT::Math::PxPyPzEVector, int>> p_cms_q_sigB;
  std::vector<std::pair<ROOT::Math::PxPyPzEVector, int>> p_cms_q_roe;

  std::vector<float> ksfwFS0;
  std::vector<float> ksfwFS1;

  std::vector<float> cleoConesAll;
  std::vector<float> cleoConesROE;

  double et[2];

  ROOT::Math::XYZVector thrustB;
  ROOT::Math::XYZVector thrustO;

  float thrustBm = -1;
  float thrustOm = -1;
  float cosTBTO  = -1;
  float cosTBz   = -1;
  float R2       = -1;


  // -- B Cand --------------------------------------------------------------------------
  PCmsLabTransform T;
  double BeamEnergy = T.getCMSEnergy() / 2;

  ROOT::Math::PxPyPzEVector p_cms_missA(0, 0, 0, 2 * BeamEnergy);
  ROOT::Math::PxPyPzEVector p_cms_missB(0, 0, 0, 2 * BeamEnergy);
  et[0] = et[1] = 0;

  // -- SIG A --- Use B primary daughters - (Belle: use_finalstate_for_sig == 0) --------
  std::vector<Belle2::Particle*> signalDaughters = particle->getDaughters();

  for (const Belle2::Particle* sigFS0 : signalDaughters) {
    ROOT::Math::PxPyPzEVector p_cms = T.rotateLabToCms() * sigFS0->get4Vector();

    p_cms_q_sigA.emplace_back(p_cms, sigFS0->getCharge());

    p_cms_missA -= p_cms;
    et[0] += p_cms.Pt();
  }

  // -- SIG B --- Use B final-state daughters - (Belle: use_finalstate_for_sig == 1) ----
  std::vector<const Belle2::Particle*> signalFSParticles = particle->getFinalStateDaughters();

  for (const Belle2::Particle* sigFS1 : signalFSParticles) {
    ROOT::Math::PxPyPzEVector p_cms = T.rotateLabToCms() * sigFS1->get4Vector();

    p_cms_all.push_back(p_cms);
    p_cms_sigB.push_back(p_cms);

    p_cms_q_sigB.emplace_back(p_cms, sigFS1->getCharge());

    p_cms_missB -= p_cms;
    et[1] += p_cms.Pt();
  }

  // -- ROE -----------------------------------------------------------------------------
  const RestOfEvent* roe = particle->getRelated<RestOfEvent>();

  if (roe) {

    // Charged tracks
    //
    std::vector<const Particle*> chargedROEParticles = roe->getChargedParticles(m_ROEMask);

    for (const Particle* chargedROEParticle : chargedROEParticles) {

      ROOT::Math::PxPyPzEVector p = ipProfileFit(chargedROEParticle);

      ROOT::Math::PxPyPzEVector p_cms = T.rotateLabToCms() * p;

      p_cms_all.push_back(p_cms);
      p_cms_roe.push_back(p_cms);

      p_cms_q_roe.emplace_back(p_cms, chargedROEParticle->getCharge());

      p_cms_missA -= p_cms;
      p_cms_missB -= p_cms;
      et[0] += p_cms.Pt();
      et[1] += p_cms.Pt();
    }

    // ECLCluster
    //
    std::vector<const Particle*> roePhotons = roe->getPhotons(m_ROEMask);

    for (const Particle* photon : roePhotons) {

      if (photon->getECLClusterEHypothesisBit() == ECLCluster::EHypothesisBit::c_nPhotons) {

        ROOT::Math::PxPyPzEVector p_cms = T.rotateLabToCms() * photon->get4Vector();
        p_cms_all.push_back(p_cms);
        p_cms_roe.push_back(p_cms);

        p_cms_q_roe.emplace_back(p_cms, photon->getCharge());

        p_cms_missA -= p_cms;
        p_cms_missB -= p_cms;
        et[0] += p_cms.Pt();
        et[1] += p_cms.Pt();
      }
    }

    // Thrust variables
    thrustB = Thrust::calculateThrust(p_cms_sigB);
    thrustO = Thrust::calculateThrust(p_cms_roe);
    thrustBm = thrustB.R();
    thrustOm = thrustO.R();
    cosTBTO  = fabs(thrustB.Unit().Dot(thrustO.Unit()));
    cosTBz   = fabs(cos(thrustB.Theta()));

    // Cleo Cones
    CleoCones cc(p_cms_all, p_cms_roe, thrustB, true, true);
    cleoConesAll = cc.cleo_cone_with_all();
    cleoConesROE = cc.cleo_cone_with_roe();

    // Fox-Wolfram Moments: Uses all final-state tracks (= sigB + ROE)
    FoxWolfram FW(p_cms_all);
    FW.calculateBasicMoments();
    R2 = FW.getR(2);

    // KSFW moments
    ROOT::Math::PxPyPzEVector p_cms_B = T.rotateLabToCms() * particle->get4Vector();
    double Hso0_max(2 * (2 * BeamEnergy - p_cms_B.E()));
    KsfwMoments KsfwM(Hso0_max,
                      p_cms_q_sigA,
                      p_cms_q_sigB,
                      p_cms_q_roe,
                      p_cms_missA,
                      p_cms_missB,
                      et);
    // use_finalstate_for_sig == 0
    KsfwM.usefinal(0);
    ksfwFS0.push_back(KsfwM.mm2());
    ksfwFS0.push_back(KsfwM.et());
    ksfwFS0.push_back(KsfwM.Hso(0, 0));
    ksfwFS0.push_back(KsfwM.Hso(0, 1));
    ksfwFS0.push_back(KsfwM.Hso(0, 2));
    ksfwFS0.push_back(KsfwM.Hso(0, 3));
    ksfwFS0.push_back(KsfwM.Hso(0, 4));
    ksfwFS0.push_back(KsfwM.Hso(1, 0));
    ksfwFS0.push_back(KsfwM.Hso(1, 2));
    ksfwFS0.push_back(KsfwM.Hso(1, 4));
    ksfwFS0.push_back(KsfwM.Hso(2, 0));
    ksfwFS0.push_back(KsfwM.Hso(2, 2));
    ksfwFS0.push_back(KsfwM.Hso(2, 4));
    ksfwFS0.push_back(KsfwM.Hoo(0));
    ksfwFS0.push_back(KsfwM.Hoo(1));
    ksfwFS0.push_back(KsfwM.Hoo(2));
    ksfwFS0.push_back(KsfwM.Hoo(3));
    ksfwFS0.push_back(KsfwM.Hoo(4));
    // use_finalstate_for_sig == 1
    KsfwM.usefinal(1);
    ksfwFS1.push_back(KsfwM.mm2());
    ksfwFS1.push_back(KsfwM.et());
    ksfwFS1.push_back(KsfwM.Hso(0, 0));
    ksfwFS1.push_back(KsfwM.Hso(0, 1));
    ksfwFS1.push_back(KsfwM.Hso(0, 2));
    ksfwFS1.push_back(KsfwM.Hso(0, 3));
    ksfwFS1.push_back(KsfwM.Hso(0, 4));
    ksfwFS1.push_back(KsfwM.Hso(1, 0));
    ksfwFS1.push_back(KsfwM.Hso(1, 2));
    ksfwFS1.push_back(KsfwM.Hso(1, 4));
    ksfwFS1.push_back(KsfwM.Hso(2, 0));
    ksfwFS1.push_back(KsfwM.Hso(2, 2));
    ksfwFS1.push_back(KsfwM.Hso(2, 4));
    ksfwFS1.push_back(KsfwM.Hoo(0));
    ksfwFS1.push_back(KsfwM.Hoo(1));
    ksfwFS1.push_back(KsfwM.Hoo(2));
    ksfwFS1.push_back(KsfwM.Hoo(3));
    ksfwFS1.push_back(KsfwM.Hoo(4));

    // TODO: The following is from the original belle ksfwmoments.cc module.
    //       Not sure if necessary here (i.e., will we be using rooksfw in belle II in the same way?).
    //       printf("rooksfw::rooksfw: mm2=%f et=%f hoo2=%f hso02=%f\n",
    //       m_mm2[0], et[0], m_Hoo[0][2], m_Hso[0][0][2]);
  }

  // Fill ContinuumSuppression with content
  qqVars->addThrustB(thrustB);
  qqVars->addThrustO(thrustO);
  qqVars->addThrustBm(thrustBm);
  qqVars->addThrustOm(thrustOm);
  qqVars->addCosTBTO(cosTBTO);
  qqVars->addCosTBz(cosTBz);
  qqVars->addR2(R2);
  qqVars->addKsfwFS0(ksfwFS0);
  qqVars->addKsfwFS1(ksfwFS1);
  qqVars->addCleoConesALL(cleoConesAll);
  qqVars->addCleoConesROE(cleoConesROE);
}

ROOT::Math::PxPyPzEVector ContinuumSuppressionBuilderModule::ipProfileFit(const Particle* particle)
{
  // Return original particle if vertex fit is not requested
  if (!m_ipProfileFit) return particle->get4Vector();

  // Access track fit result of particle
  const TrackFitResult* trackFitResult = particle->getTrackFitResult();
  if (!trackFitResult) return particle->get4Vector();

  // Move helix of TrackFitResult by IP position
  Helix helix = trackFitResult->getHelix();
  helix.passiveMoveBy(m_BeamSpotCenter);

  // Reject particle movements that push helix too far away
  if (std::abs(helix.getD0()) > 5.0 || std::abs(helix.getZ0()) > 10.0) return particle->get4Vector();

  // Create particle with shifted vertex position
  ROOT::Math::XYZVector p_helix = helix.getMomentum(m_Bfield);
  double shiftedParticleMass = particle->getPDGMass();
  double shiftedParticleEnergy = std::sqrt(p_helix.Mag2() + shiftedParticleMass * shiftedParticleMass);
  ROOT::Math::PxPyPzEVector momentumOfShiftedParticle(p_helix.X(), p_helix.y(), p_helix.Z(), shiftedParticleEnergy);
  const Particle shiftedParticle(momentumOfShiftedParticle, particle->getPDGCode());

  // Perform vertex fit with IP profile constraint
  analysis::VertexFitKFit kv;
  kv.setMagneticField(m_Bfield);
  kv.addParticle(&shiftedParticle);
  kv.setIpProfile(ROOTToCLHEP::getPoint3D(m_BeamSpotCenter), ROOTToCLHEP::getHepSymMatrix(m_beamSpotCov));
  enum analysis::KFitError::ECode fitError = kv.doFit();
  if (fitError != analysis::KFitError::kNoError) return particle->get4Vector();

  // Calculate updated particle momentum
  analysis::MakeMotherKFit kmm;
  kmm.setMagneticField(m_Bfield);
  kmm.addTrack(kv.getTrackMomentum(0), kv.getTrackPosition(0), kv.getTrackError(0), kv.getTrack(0).getCharge());
  kmm.setTrackVertexError(kv.getTrackVertexError(0));
  kmm.setVertex(kv.getVertex());
  kmm.setVertexError(kv.getVertexError());
  fitError = kmm.doMake();
  if (fitError != analysis::KFitError::kNoError) return particle->get4Vector();
  return CLHEPToROOT::getLorentzVector(kmm.getMotherMomentum());
}
