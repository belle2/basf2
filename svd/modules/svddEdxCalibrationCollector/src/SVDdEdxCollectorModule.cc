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

#include "TTree.h"

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
  LambdaTree->Branch<double>("p_pSVD", &m_p_pSVD);
  LambdaTree->Branch<double>("p_SVDdEdx", &m_p_SVDdEdx);
  LambdaTree->Branch<double>("pi_pSVD", &m_pi_pSVD);
  LambdaTree->Branch<double>("pi_SVDdEdx", &m_pi_SVDdEdx);

  DstarTree->Branch<double>("InvM", &m_InvMDstar);
  DstarTree->Branch<double>("D0_InvM", &m_InvMD0);
  DstarTree->Branch<double>("deltaM", &m_DeltaM);
  DstarTree->Branch<double>("K_pSVD", &m_K_pSVD);
  DstarTree->Branch<double>("K_SVDdEdx", &m_K_SVDdEdx);
  DstarTree->Branch<double>("pi_pSVD", &m_piD_pSVD);
  DstarTree->Branch<double>("pi_SVDdEdx", &m_piD_SVDdEdx);
  DstarTree->Branch<double>("piS_pSVD", &m_piS_pSVD);
  DstarTree->Branch<double>("piS_SVDdEdx", &m_piS_SVDdEdx);

  GammaTree->Branch<double>("InvM", &m_InvMGamma);
  // GammaTree->Branch<double>("cpM", &m_cpMGamma);
  // GammaTree->Branch<double>("cpdr", &m_cpdr);
  // GammaTree->Branch<double>("cpdz", &m_cpdz);
  GammaTree->Branch<double>("e_1_pSVD", &m_e_1_pSVD);
  GammaTree->Branch<double>("e_1_SVDdEdx", &m_e_1_SVDdEdx);
  GammaTree->Branch<double>("e_2_pSVD", &m_e_2_pSVD);
  GammaTree->Branch<double>("e_2_SVDdEdx", &m_e_2_SVDdEdx);

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

  // if (!LambdaParticles.isValid() || abs(LambdaParticles->getPDGCode()) != 3122)
  //     return;
  // if (!DstarParticles.isValid() || abs(DstarParticles->getPDGCode()) != 413)
  //     return;
  // if (!GammaParticles.isValid() || abs(GammaParticles->getPDGCode()) != 22)
  //     return;

  if (!LambdaParticles.isValid() && !DstarParticles.isValid() && !GammaParticles.isValid())
    return;

  // if (LambdaParticles->getListSize() <  1 && DstarParticles->getListSize() <  1 && GammaParticles->getListSize() <  1)
  //     return;
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

      if (!dedxTrackPFromLambda) {
        m_p_pSVD = -999.0;
        m_p_SVDdEdx = -999.0;
      } else {
        m_p_pSVD = dedxTrackPFromLambda->getMomentum();
        m_p_SVDdEdx = dedxTrackPFromLambda->getDedx(Const::EDetector::SVD);
      }

      if (!dedxTrackPiFromLambda) {
        m_pi_pSVD = -999.0;
        m_pi_SVDdEdx = -999.0;
      } else {
        m_pi_pSVD = dedxTrackPiFromLambda->getMomentum();
        m_pi_SVDdEdx = dedxTrackPiFromLambda->getDedx(Const::EDetector::SVD);
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

      if (!dedxTrackKFromD) {
        m_K_pSVD = -999.0;
        m_K_SVDdEdx = -999.0;
      } else {
        m_K_pSVD = dedxTrackKFromD->getMomentum();
        m_K_SVDdEdx = dedxTrackKFromD->getDedx(Const::EDetector::SVD);
      }

      if (!dedxTrackPiFromD) {
        m_piD_pSVD = -999.0;
        m_piD_SVDdEdx = -999.0;
      } else {
        m_piD_pSVD = dedxTrackPiFromD->getMomentum();
        m_piD_SVDdEdx = dedxTrackPiFromD->getDedx(Const::EDetector::SVD);
      }

      if (!dedxTrackPiS) {
        m_piS_pSVD = -999.0;
        m_piS_SVDdEdx = -999.0;
      } else {
        m_piS_pSVD = dedxTrackPiS->getMomentum();
        m_piS_SVDdEdx = dedxTrackPiS->getDedx(Const::EDetector::SVD);
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

      if (!dedxTrackE1FromGamma) {
        m_e_1_pSVD = -999.0;
        m_e_1_SVDdEdx = -999.0;
      } else {
        m_e_1_pSVD = dedxTrackE1FromGamma->getMomentum();
        m_e_1_SVDdEdx = dedxTrackE1FromGamma->getDedx(Const::EDetector::SVD);
      }

      if (!dedxTrackE2FromGamma) {
        m_e_2_pSVD = -999.0;
        m_e_2_SVDdEdx = -999.0;
      } else {
        m_e_2_pSVD = dedxTrackE2FromGamma->getMomentum();
        m_e_2_SVDdEdx = dedxTrackE2FromGamma->getDedx(Const::EDetector::SVD);
      }

      getObjectPtr<TTree>("Gamma")->Fill();
    }
  }
}
