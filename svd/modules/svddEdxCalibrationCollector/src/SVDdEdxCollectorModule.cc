/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/modules/svddEdxCalibrationCollector/SVDdEdxCollectorModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <reconstruction/dataobjects/VXDDedxTrack.h>
#include <mdst/dataobjects/Track.h>

#include <TTree.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDdEdxCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDdEdxCollectorModule::SVDdEdxCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties

  setDescription("Collector module used to create the ROOT ntuples used to produce dE/dx calibration payloads");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("LambdaListName", m_LambdaListName, "Name of the Lambda particle list", std::string("Lambda0:cut"));
  addParam("DstarListName", m_DstarListName, "Name of the Dstar particle list", std::string("D*+:cut"));
  addParam("GammaListName", m_GammaListName, "Name of the Gamma particle list", std::string("gamma:cut"));
}

void SVDdEdxCollectorModule::prepare()
{
  B2INFO("Initialisation of the trees");
  std::string objectNameLambda = "Lambda";
  std::string objectNameDstar = "Dstar";
  std::string objectNameGamma = "Gamma";
  // Data object creation --------------------------------------------------
  TTree* LambdaTree = new TTree(objectNameLambda.c_str(), "");
  TTree* DstarTree = new TTree(objectNameDstar.c_str(), "");
  TTree* GammaTree = new TTree(objectNameGamma.c_str(), "");

  // Event info for all trees
  LambdaTree->Branch<int>("event", &m_evt);
  LambdaTree->Branch<int>("exp", &m_exp);
  LambdaTree->Branch<int>("run", &m_run);
  LambdaTree->Branch<double>("time", &m_time);

  DstarTree->Branch<int>("event", &m_evt);
  DstarTree->Branch<int>("exp", &m_exp);
  DstarTree->Branch<int>("run", &m_run);
  DstarTree->Branch<double>("time", &m_time);

  GammaTree->Branch<int>("event", &m_evt);
  GammaTree->Branch<int>("exp", &m_exp);
  GammaTree->Branch<int>("run", &m_run);
  GammaTree->Branch<double>("time", &m_time);

  // Specific decay info for all trees
  LambdaTree->Branch<double>("InvM", &m_InvMLambda);
  LambdaTree->Branch<double>("ProtonMomentum", &m_protonMomentum);
  LambdaTree->Branch<double>("ProtonSVDdEdx", &m_protonSVDdEdx);
  LambdaTree->Branch<double>("PionLambdaMomentum", &m_pionLambdap);
  LambdaTree->Branch<double>("PionLambdaSVDdEdx", &m_pionLambdaSVDdEdx);

  DstarTree->Branch<double>("InvM", &m_InvMDstar);
  DstarTree->Branch<double>("D0InvM", &m_InvMD0);
  DstarTree->Branch<double>("deltaM", &m_DeltaM);
  DstarTree->Branch<double>("KaonMomentum", &m_kaonMomentum);
  DstarTree->Branch<double>("KaonSVDdEdx", &m_kaonSVDdEdx);
  DstarTree->Branch<double>("PionDMomentum", &m_pionDp);
  DstarTree->Branch<double>("PionDSVDdEdx", &m_pionDSVDdEdx);
  DstarTree->Branch<double>("SlowPionMomentum", &m_slowPionMomentum);
  DstarTree->Branch<double>("SlowPionSVDdEdx", &m_slowPionSVDdEdx);

  GammaTree->Branch<double>("InvM", &m_InvMGamma);
  GammaTree->Branch<double>("FirstElectronMomentum", &m_firstElectronMomentum);
  GammaTree->Branch<double>("FirstElectronSVDdEdx", &m_firstElectronSVDdEdx);
  GammaTree->Branch<double>("SecondElectronMomentum", &m_secondElectronMomentum);
  GammaTree->Branch<double>("SecondElectronSVDdEdx", &m_secondElectronSVDdEdx);

  // We register the objects so that our framework knows about them.
  // Don't try and hold onto the pointers or fill these objects directly
  // Use the getObjectPtr functions to access collector objects
  registerObject<TTree>(objectNameLambda, LambdaTree);
  registerObject<TTree>(objectNameDstar, DstarTree);
  registerObject<TTree>(objectNameGamma, GammaTree);
}

VXDDedxTrack const* getSVDDedxFromParticle(Particle const* particle)
{
  const Track* track = particle->getTrack();
  if (!track) {
    return nullptr;
  }

  const VXDDedxTrack* dedxTrack = track->getRelatedTo<VXDDedxTrack>();
  if (!dedxTrack) {
    return nullptr;
  }
  return dedxTrack;
}

void SVDdEdxCollectorModule::collect()
{

  m_evt = m_emd->getEvent();
  m_run = m_emd->getRun();
  m_exp = m_emd->getExperiment();
  m_time = m_emd->getTime() / 1e9 / 3600.; // from ns to hours

  StoreObjPtr<ParticleList> LambdaParticles(m_LambdaListName);
  StoreObjPtr<ParticleList> DstarParticles(m_DstarListName);
  StoreObjPtr<ParticleList> GammaParticles(m_GammaListName);


  if (!LambdaParticles.isValid() && !DstarParticles.isValid() && !GammaParticles.isValid())
    return;

  if (LambdaParticles->getListSize() > 0) {
    for (unsigned int iParticle = 0; iParticle < LambdaParticles->getListSize(); ++iParticle) {

      std::vector<int> indicesLambda = LambdaParticles->getParticle(0)->getDaughterIndices();
      if (indicesLambda.size() != 2)
        return;
      const Particle* partLambda = LambdaParticles->getParticle(0);
      const Particle* partPFromLambda = LambdaParticles->getParticle(0)->getDaughter(0);
      const Particle* partPiFromLambda = LambdaParticles->getParticle(0)->getDaughter(1);

      const VXDDedxTrack* dedxTrackPFromLambda = getSVDDedxFromParticle(partPFromLambda);
      const VXDDedxTrack* dedxTrackPiFromLambda = getSVDDedxFromParticle(partPiFromLambda);

      m_InvMLambda = partLambda->getMass();

      m_protonMomentum = partPFromLambda->getMomentumMagnitude();
      if (!dedxTrackPFromLambda) {
        m_protonSVDdEdx = -999.0;
      } else {
        m_protonSVDdEdx = dedxTrackPFromLambda->getDedx(Const::EDetector::SVD);
      }

      m_pionLambdap = partPiFromLambda->getMomentumMagnitude();
      if (!dedxTrackPiFromLambda) {
        m_pionLambdaSVDdEdx = -999.0;
      } else {
        m_pionLambdaSVDdEdx = dedxTrackPiFromLambda->getDedx(Const::EDetector::SVD);
      }
      getObjectPtr<TTree>("Lambda")->Fill();
    }
  }

  if (DstarParticles->getListSize() > 0) {
    for (unsigned int iParticle = 0; iParticle < DstarParticles->getListSize(); ++iParticle) {

      std::vector<int> indicesDstar = DstarParticles->getParticle(0)->getDaughterIndices();
      if (indicesDstar.size() != 2)
        return;

      const Particle* partDstar = DstarParticles->getParticle(0);
      const Particle* partD0 = DstarParticles->getParticle(0)->getDaughter(0);
      const Particle* partPiS = DstarParticles->getParticle(0)->getDaughter(1);
      const Particle* partKFromD = DstarParticles->getParticle(0)->getDaughter(0)->getDaughter(0);
      const Particle* partPiFromD = DstarParticles->getParticle(0)->getDaughter(0)->getDaughter(1);

      const VXDDedxTrack* dedxTrackPiS = getSVDDedxFromParticle(partPiS);
      const VXDDedxTrack* dedxTrackKFromD = getSVDDedxFromParticle(partKFromD);
      const VXDDedxTrack* dedxTrackPiFromD = getSVDDedxFromParticle(partPiFromD);


      m_InvMDstar = partDstar->getMass();
      m_InvMD0 = partD0->getMass();
      m_DeltaM = m_InvMDstar - m_InvMD0;

      m_kaonMomentum = partKFromD->getMomentumMagnitude();
      if (!dedxTrackKFromD) {
        m_kaonSVDdEdx = -999.0;
      } else {
        m_kaonSVDdEdx = dedxTrackKFromD->getDedx(Const::EDetector::SVD);
      }

      m_pionDp = partPiFromD->getMomentumMagnitude();
      if (!dedxTrackPiFromD) {
        m_pionDSVDdEdx = -999.0;
      } else {
        m_pionDSVDdEdx = dedxTrackPiFromD->getDedx(Const::EDetector::SVD);
      }

      m_slowPionMomentum = partPiS->getMomentumMagnitude();
      if (!dedxTrackPiS) {
        m_slowPionSVDdEdx = -999.0;
      } else {
        m_slowPionSVDdEdx = dedxTrackPiS->getDedx(Const::EDetector::SVD);
      }
      getObjectPtr<TTree>("Dstar")->Fill();
    }
  }

  if (GammaParticles->getListSize() > 0) {
    for (unsigned int iParticle = 0; iParticle < GammaParticles->getListSize(); ++iParticle) {
      std::vector<int> indicesGamma = GammaParticles->getParticle(0)->getDaughterIndices();
      if (indicesGamma.size() != 2)
        return;

      const Particle* partGamma = GammaParticles->getParticle(0);
      const Particle* partE1FromGamma = GammaParticles->getParticle(0)->getDaughter(0);
      const Particle* partE2FromGamma = GammaParticles->getParticle(0)->getDaughter(1);

      const VXDDedxTrack* dedxTrackE1FromGamma = getSVDDedxFromParticle(partE1FromGamma);
      const VXDDedxTrack* dedxTrackE2FromGamma = getSVDDedxFromParticle(partE2FromGamma);

      m_InvMGamma = partGamma->getMass();

      m_firstElectronMomentum = partE1FromGamma->getMomentumMagnitude();
      if (!dedxTrackE1FromGamma) {
        m_firstElectronSVDdEdx = -999.0;
      } else {
        m_firstElectronSVDdEdx = dedxTrackE1FromGamma->getDedx(Const::EDetector::SVD);
      }

      m_secondElectronMomentum = partE2FromGamma->getMomentumMagnitude();
      if (!dedxTrackE2FromGamma) {
        m_secondElectronSVDdEdx = -999.0;
      } else {
        m_secondElectronSVDdEdx = dedxTrackE2FromGamma->getDedx(Const::EDetector::SVD);
      }

      getObjectPtr<TTree>("Gamma")->Fill();
    }
  }
}
