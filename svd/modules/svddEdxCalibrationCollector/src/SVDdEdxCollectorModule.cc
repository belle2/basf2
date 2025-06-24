/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/modules/svddEdxCalibrationCollector/SVDdEdxCollectorModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <reconstruction/dataobjects/VXDDedxTrack.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <TTree.h>
#include <string>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;

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
  addParam("GenericListName", m_GenericListName, "Name of the generic track particle list", std::string("pi+:cut"));
}

void SVDdEdxCollectorModule::prepare()
{
  B2INFO("Initialisation of the trees");
  std::string objectNameLambda = "Lambda";
  std::string objectNameDstar = "Dstar";
  std::string objectNameGamma = "Gamma";
  std::string objectNameGeneric = "Generic";
  // Data object creation --------------------------------------------------
  TTree* LambdaTree = new TTree(objectNameLambda.c_str(), "");
  TTree* DstarTree = new TTree(objectNameDstar.c_str(), "");
  TTree* GammaTree = new TTree(objectNameGamma.c_str(), "");
  TTree* GenericTree = new TTree(objectNameGeneric.c_str(), "");

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
  LambdaTree->Branch<double>("DIRA", &m_diraLambda);
  LambdaTree->Branch<double>("ProtonMomentum", &m_protonMomentum);
  LambdaTree->Branch<double>("ProtonSVDdEdx", &m_protonSVDdEdx);
  LambdaTree->Branch<double>("ProtonSVDdEdxErr", &m_protonSVDdEdxErr);
  // LambdaTree->Branch<double>("ProtonSVDdEdxMean", &m_protonSVDdEdxMean);
  LambdaTree->Branch<double>("ProtonSVDdEdxTrackMomentum", &m_protondEdxTrackMomentum);
  LambdaTree->Branch<int>("ProtonSVDdEdxTrackNHits", &m_protondEdxTrackNHits);
  // LambdaTree->Branch<int>("ProtonSVDdEdxTrackNHitsUsed", &m_protondEdxTrackNHitsUsed);
  LambdaTree->Branch<double>("ProtonSVDdEdxTrackCosTheta", &m_protondEdxTrackCosTheta);
  // LambdaTree->Branch<std::vector<double>>("ProtonSVDdEdxList", &m_protonSVDdEdxList);
  LambdaTree->Branch<double>("ProtonnSVDHits", &m_protonnSVDHits);

  LambdaTree->Branch<double>("PionLambdaMomentum", &m_pionLambdap);
  LambdaTree->Branch<double>("PionLambdaSVDdEdx", &m_pionLambdaSVDdEdx);
  LambdaTree->Branch<double>("PionLambdaSVDdEdxErr", &m_pionLambdaSVDdEdxErr);
  // LambdaTree->Branch<double>("PionLambdaSVDdEdxMean", &m_pionLambdaSVDdEdxMean);
  LambdaTree->Branch<double>("PionLambdaSVDdEdxTrackMomentum", &m_pionLambdadEdxTrackMomentum);
  LambdaTree->Branch<int>("PionLambdaSVDdEdxTrackNHits", &m_pionLambdadEdxTrackNHits);
  // LambdaTree->Branch<int>("PionLambdaSVDdEdxTrackNHitsUsed", &m_pionLambdadEdxTrackNHitsUsed);
  LambdaTree->Branch<double>("PionLambdaSVDdEdxTrackCosTheta", &m_pionLambdadEdxTrackCosTheta);
  // LambdaTree->Branch<std::vector<double>>("PionLambdaSVDdEdxList", &m_pionLambdaSVDdEdxList);
  LambdaTree->Branch<double>("PionLambdanSVDHits", &m_pionLambdanSVDHits);

  DstarTree->Branch<double>("InvM", &m_InvMDstar);
  DstarTree->Branch<double>("D0InvM", &m_InvMD0);
  DstarTree->Branch<double>("deltaM", &m_DeltaM);
  DstarTree->Branch<double>("D0DIRA", &m_diraD0);
  // DstarTree->Branch<double>("D0Momentum", &m_D0Momentum);
  DstarTree->Branch<double>("KaonMomentum", &m_kaonMomentum);
  // DstarTree->Branch<double>("KaonMomentumX", &m_kaonMomentumX);
  // DstarTree->Branch<double>("KaonMomentumY", &m_kaonMomentumY);
  // DstarTree->Branch<double>("KaonMomentumZ", &m_kaonMomentumZ);
  DstarTree->Branch<double>("KaonSVDdEdx", &m_kaonSVDdEdx);
  DstarTree->Branch<double>("KaonSVDdEdxErr", &m_kaonSVDdEdxErr);
  // DstarTree->Branch<double>("KaonSVDdEdxMean", &m_kaonSVDdEdxMean);
  DstarTree->Branch<double>("KaonSVDdEdxTrackMomentum", &m_kaondEdxTrackMomentum);
  DstarTree->Branch<int>("KaonSVDdEdxTrackNHits", &m_kaondEdxTrackNHits);
  // DstarTree->Branch<int>("KaonSVDdEdxTrackNHitsUsed", &m_kaondEdxTrackNHitsUsed);
  DstarTree->Branch<double>("KaonSVDdEdxTrackCosTheta", &m_kaondEdxTrackCosTheta);
  // DstarTree->Branch<std::vector<double>>("KaonSVDdEdxList", &m_kaonSVDdEdxList);
  DstarTree->Branch<double>("KaonnSVDHits", &m_kaonnSVDHits);

  DstarTree->Branch<double>("PionDMomentum", &m_pionDp);
  // DstarTree->Branch<double>("PionDMomentumX", &m_pionDMomentumX);
  // DstarTree->Branch<double>("PionDMomentumY", &m_pionDMomentumY);
  // DstarTree->Branch<double>("PionDMomentumZ", &m_pionDMomentumZ);
  DstarTree->Branch<double>("PionDSVDdEdx", &m_pionDSVDdEdx);
  DstarTree->Branch<double>("PionDSVDdEdxErr", &m_pionDSVDdEdxErr);
  // DstarTree->Branch<double>("PionDSVDdEdxMean", &m_pionDSVDdEdxMean);
  DstarTree->Branch<double>("PionDSVDdEdxTrackMomentum", &m_pionDdEdxTrackMomentum);
  DstarTree->Branch<int>("PionDSVDdEdxTrackNHits", &m_pionDdEdxTrackNHits);
  // DstarTree->Branch<int>("PionDSVDdEdxTrackNHitsUsed", &m_pionDdEdxTrackNHitsUsed);
  DstarTree->Branch<double>("PionDSVDdEdxTrackCosTheta", &m_pionDdEdxTrackCosTheta);
  // DstarTree->Branch<std::vector<double>>("PionDSVDdEdxList", &m_pionDSVDdEdxList);
  DstarTree->Branch<double>("PionDnSVDHits", &m_pionDnSVDHits);

  DstarTree->Branch<double>("SlowPionMomentum", &m_slowPionMomentum);
  // DstarTree->Branch<double>("SlowPionMomentumX", &m_slowPionMomentumX);
  // DstarTree->Branch<double>("SlowPionMomentumY", &m_slowPionMomentumY);
  // DstarTree->Branch<double>("SlowPionMomentumZ", &m_slowPionMomentumZ);
  DstarTree->Branch<double>("SlowPionSVDdEdx", &m_slowPionSVDdEdx);
  DstarTree->Branch<double>("SlowPionSVDdEdxErr", &m_slowPionSVDdEdxErr);
  // DstarTree->Branch<double>("SlowPionSVDdEdxMean", &m_slowPionSVDdEdxMean);
  DstarTree->Branch<double>("SlowPionSVDdEdxTrackMomentum", &m_slowPiondEdxTrackMomentum);
  DstarTree->Branch<int>("SlowPionSVDdEdxTrackNHits", &m_slowPiondEdxTrackNHits);
  // DstarTree->Branch<int>("SlowPionSVDdEdxTrackNHitsUsed", &m_slowPiondEdxTrackNHitsUsed);
  DstarTree->Branch<double>("SlowPionSVDdEdxTrackCosTheta", &m_slowPiondEdxTrackCosTheta);
  // DstarTree->Branch<std::vector<double>>("SlowPionSVDdEdxList", &m_slowPionSVDdEdxList);
  DstarTree->Branch<double>("SlowPionnSVDHits", &m_slowPionnSVDHits);

  GammaTree->Branch<double>("InvM", &m_InvMGamma);
  GammaTree->Branch<double>("dr", &m_drGamma);
  GammaTree->Branch<double>("DIRA", &m_diraGamma);
  GammaTree->Branch<double>("FirstElectronMomentum", &m_firstElectronMomentum);
  GammaTree->Branch<double>("FirstElectronSVDdEdx", &m_firstElectronSVDdEdx);
  GammaTree->Branch<double>("FirstElectronSVDdEdxErr", &m_firstElectronSVDdEdxErr);
  // GammaTree->Branch<double>("FirstElectronSVDdEdxMean", &m_firstElectronSVDdEdxMean);
  GammaTree->Branch<double>("FirstElectronSVDdEdxTrackMomentum", &m_firstElectrondEdxTrackMomentum);
  GammaTree->Branch<int>("FirstElectronSVDdEdxTrackNHits", &m_firstElectrondEdxTrackNHits);
  // GammaTree->Branch<int>("FirstElectronSVDdEdxTrackNHitsUsed", &m_firstElectrondEdxTrackNHitsUsed);
  GammaTree->Branch<double>("FirstElectronSVDdEdxTrackCosTheta", &m_firstElectrondEdxTrackCosTheta);
  // GammaTree->Branch<std::vector<double>>("FirstElectronSVDdEdxList", &m_firstElectronSVDdEdxList);
  GammaTree->Branch<double>("FirstElectronnSVDHits", &m_firstElectronnSVDHits);

  GammaTree->Branch<double>("SecondElectronMomentum", &m_secondElectronMomentum);
  GammaTree->Branch<double>("SecondElectronSVDdEdx", &m_secondElectronSVDdEdx);
  GammaTree->Branch<double>("SecondElectronSVDdEdxErr", &m_secondElectronSVDdEdxErr);
  // GammaTree->Branch<double>("SecondElectronSVDdEdxMean", &m_secondElectronSVDdEdxMean);
  GammaTree->Branch<double>("SecondElectronSVDdEdxTrackMomentum", &m_secondElectrondEdxTrackMomentum);
  GammaTree->Branch<int>("SecondElectronSVDdEdxTrackNHits", &m_secondElectrondEdxTrackNHits);
  // GammaTree->Branch<int>("SecondElectronSVDdEdxTrackNHitsUsed", &m_secondElectrondEdxTrackNHitsUsed);
  GammaTree->Branch<double>("SecondElectronSVDdEdxTrackCosTheta", &m_secondElectrondEdxTrackCosTheta);
  // GammaTree->Branch<std::vector<double>>("SecondElectronSVDdEdxList", &m_secondElectronSVDdEdxList);
  GammaTree->Branch<double>("SecondElectronnSVDHits", &m_secondElectronnSVDHits);

  GenericTree->Branch<double>("TrackMomentum", &m_genericp);
  GenericTree->Branch<double>("TrackSVDdEdx", &m_genericSVDdEdx);
  GenericTree->Branch<double>("TrackSVDdEdxErr", &m_genericSVDdEdxErr);
  // GenericTree->Branch<double>("TrackSVDdEdxMean", &m_genericSVDdEdxMean);
  GenericTree->Branch<double>("TrackSVDdEdxTrackMomentum", &m_genericdEdxTrackMomentum);
  GenericTree->Branch<int>("TrackSVDdEdxTrackNHits", &m_genericdEdxTrackNHits);
  // GenericTree->Branch<int>("TrackSVDdEdxTrackNHitsUsed", &m_genericdEdxTrackNHitsUsed);
  // GenericTree->Branch<double>("TrackSVDdEdxTrackCosTheta", &m_genericdEdxTrackCosTheta);
  // GenericTree->Branch<std::vector<double>>("TrackSVDdEdxList", &m_genericSVDdEdxList);
  GenericTree->Branch<double>("TracknSVDHits", &m_genericnSVDHits);

  // We register the objects so that our framework knows about them.
  // Don't try and hold onto the pointers or fill these objects directly
  // Use the getObjectPtr functions to access collector objects
  registerObject<TTree>(objectNameLambda, LambdaTree);
  registerObject<TTree>(objectNameDstar, DstarTree);
  registerObject<TTree>(objectNameGamma, GammaTree);
  registerObject<TTree>(objectNameGeneric, GenericTree);
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
  StoreObjPtr<ParticleList> GenericParticles(m_GenericListName);

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
      m_diraLambda = std::get<double>(Variable::Manager::Instance().getVariable(
                                        std::string("cosAngleBetweenMomentumAndVertexVector"))->function(partLambda));

      m_protonMomentum = partPFromLambda->getMomentumMagnitude();
      if (!dedxTrackPFromLambda) {
        m_protonSVDdEdx = -999.0;
        m_protonSVDdEdxErr = -999.0;
        // m_protonSVDdEdxMean = -999.0;
        m_protondEdxTrackMomentum = -999.0;
        m_protondEdxTrackNHits = -1;
        // m_protondEdxTrackNHitsUsed = -1;
        m_protondEdxTrackCosTheta = -999.0;
      } else {
        m_protonSVDdEdx = dedxTrackPFromLambda->getDedx(Const::EDetector::SVD);
        m_protonSVDdEdxErr = dedxTrackPFromLambda->getDedxError(Const::EDetector::SVD);
        // m_protonSVDdEdxMean = dedxTrackPFromLambda->getDedxMean(Const::EDetector::SVD);
        m_protondEdxTrackMomentum = dedxTrackPFromLambda->getMomentum();
        m_protondEdxTrackNHits = (int) dedxTrackPFromLambda->size();
        // m_protondEdxTrackNHitsUsed = (int) dedxTrackPFromLambda->getNHitsUsed();
        m_protondEdxTrackCosTheta = dedxTrackPFromLambda->getCosTheta();
        // m_protonSVDdEdxList = dedxTrackPFromLambda->getDedxList();
      }
      auto trackFitPFromLambda = partPFromLambda->getTrackFitResult();
      if (!trackFitPFromLambda) {m_protonnSVDHits = Const::doubleNaN;}
      else {m_protonnSVDHits = trackFitPFromLambda->getHitPatternVXD().getNSVDHits();}

      m_pionLambdap = partPiFromLambda->getMomentumMagnitude();
      if (!dedxTrackPiFromLambda) {
        m_pionLambdaSVDdEdx = -999.0;
        m_pionLambdaSVDdEdxErr = -999.0;
        // m_pionLambdaSVDdEdxMean = -999.0;
        m_pionLambdadEdxTrackMomentum = -999.0;
        m_pionLambdadEdxTrackNHits = -1;
        // m_pionLambdadEdxTrackNHitsUsed = -1;
        m_pionLambdadEdxTrackCosTheta = -999.0;
      } else {
        m_pionLambdaSVDdEdx = dedxTrackPiFromLambda->getDedx(Const::EDetector::SVD);
        m_pionLambdaSVDdEdxErr = dedxTrackPiFromLambda->getDedxError(Const::EDetector::SVD);
        // m_pionLambdaSVDdEdxMean = dedxTrackPiFromLambda->getDedxMean(Const::EDetector::SVD);
        m_pionLambdadEdxTrackMomentum = dedxTrackPiFromLambda->getMomentum();
        m_pionLambdadEdxTrackNHits = (int) dedxTrackPiFromLambda->size();
        // m_pionLambdadEdxTrackNHitsUsed = (int) dedxTrackPiFromLambda->getNHitsUsed();
        m_pionLambdadEdxTrackCosTheta = dedxTrackPiFromLambda->getCosTheta();
        // m_pionLambdaSVDdEdxList = dedxTrackPiFromLambda->getDedxList();
      }
      auto trackFitPiFromLambda = partPiFromLambda->getTrackFitResult();
      if (!trackFitPiFromLambda) {m_pionLambdanSVDHits = Const::doubleNaN;}
      else {m_pionLambdanSVDHits = trackFitPiFromLambda->getHitPatternVXD().getNSVDHits();}

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
      // m_D0Momentum = partD0->getMomentumMagnitude();
      m_diraD0 = std::get<double>(Variable::Manager::Instance().getVariable(
                                    std::string("cosAngleBetweenMomentumAndVertexVector"))->function(partD0));;

      m_kaonMomentum = partKFromD->getMomentumMagnitude();
      // m_kaonMomentumX = partKFromD->getPx();
      // m_kaonMomentumY = partKFromD->getPy();
      // m_kaonMomentumZ = partKFromD->getPz();
      if (!dedxTrackKFromD) {
        m_kaonSVDdEdx = -999.0;
        m_kaonSVDdEdxErr = -999.0;
        // m_kaonSVDdEdxMean = -999.0;
        m_kaondEdxTrackMomentum = -999.0;
        m_kaondEdxTrackNHits = -1;
        // m_kaondEdxTrackNHitsUsed = -1;
        m_kaondEdxTrackCosTheta = -999.0;
      } else {
        m_kaonSVDdEdx = dedxTrackKFromD->getDedx(Const::EDetector::SVD);
        m_kaonSVDdEdxErr = dedxTrackKFromD->getDedxError(Const::EDetector::SVD);
        // m_kaonSVDdEdxMean = dedxTrackKFromD->getDedxMean(Const::EDetector::SVD);
        m_kaondEdxTrackMomentum = dedxTrackKFromD->getMomentum();
        m_kaondEdxTrackNHits = (int) dedxTrackKFromD->size();
        // m_kaondEdxTrackNHitsUsed = (int) dedxTrackKFromD->getNHitsUsed();
        m_kaondEdxTrackCosTheta = dedxTrackKFromD->getCosTheta();
        // m_kaonSVDdEdxList = dedxTrackKFromD->getDedxList();
      }

      auto trackFitKFromD = partKFromD->getTrackFitResult();
      if (!trackFitKFromD) {m_kaonnSVDHits = Const::doubleNaN;}
      else {m_kaonnSVDHits = trackFitKFromD->getHitPatternVXD().getNSVDHits();}

      m_pionDp = partPiFromD->getMomentumMagnitude();
      // m_pionDMomentumX = partPiFromD->getPx();
      // m_pionDMomentumY = partPiFromD->getPy();
      // m_pionDMomentumZ = partPiFromD->getPz();
      if (!dedxTrackPiFromD) {
        m_pionDSVDdEdx = -999.0;
        m_pionDSVDdEdxErr = -999.0;
        // m_pionDSVDdEdxMean = -999.0;
        m_pionDdEdxTrackMomentum = -999.0;
        m_pionDdEdxTrackNHits = -1;
        // m_pionDdEdxTrackNHitsUsed = -1;
        m_pionDdEdxTrackCosTheta = -999.0;
      } else {
        m_pionDSVDdEdx = dedxTrackPiFromD->getDedx(Const::EDetector::SVD);
        m_pionDSVDdEdxErr = dedxTrackPiFromD->getDedxError(Const::EDetector::SVD);
        // m_pionDSVDdEdxMean = dedxTrackPiFromD->getDedxMean(Const::EDetector::SVD);
        m_pionDdEdxTrackMomentum = dedxTrackPiFromD->getMomentum();
        m_pionDdEdxTrackNHits = (int) dedxTrackPiFromD->size();
        // m_pionDdEdxTrackNHitsUsed = (int) dedxTrackPiFromD->getNHitsUsed();
        m_pionDdEdxTrackCosTheta = dedxTrackPiFromD->getCosTheta();
        // m_pionDSVDdEdxList = dedxTrackPiFromD->getDedxList();
      }

      auto trackFitPiFromD = partPiFromD->getTrackFitResult();
      if (!trackFitPiFromD) {m_pionDnSVDHits = Const::doubleNaN;}
      else {m_pionDnSVDHits = trackFitPiFromD->getHitPatternVXD().getNSVDHits();}

      m_slowPionMomentum = partPiS->getMomentumMagnitude();
      m_slowPionMomentumX = partPiS->getPx();
      m_slowPionMomentumY = partPiS->getPy();
      m_slowPionMomentumZ = partPiS->getPz();
      if (!dedxTrackPiS) {
        m_slowPionSVDdEdx = -999.0;
        m_slowPionSVDdEdxErr = -999.0;
        // m_slowPionSVDdEdxMean = -999.0;
        m_slowPiondEdxTrackMomentum = -999.0;
        m_slowPiondEdxTrackNHits = -1;
        // m_slowPiondEdxTrackNHitsUsed = -1;
        m_slowPiondEdxTrackCosTheta = -999.0;
      } else {
        m_slowPionSVDdEdx = dedxTrackPiS->getDedx(Const::EDetector::SVD);
        m_slowPionSVDdEdxErr = dedxTrackPiS->getDedxError(Const::EDetector::SVD);
        // m_slowPionSVDdEdxMean = dedxTrackPiS->getDedxMean(Const::EDetector::SVD);
        m_slowPiondEdxTrackMomentum = dedxTrackPiS->getMomentum();
        m_slowPiondEdxTrackNHits = (int) dedxTrackPiS->size();
        // m_slowPiondEdxTrackNHitsUsed = (int) dedxTrackPiS->getNHitsUsed();
        m_slowPiondEdxTrackCosTheta = dedxTrackPiS->getCosTheta();
        // m_slowPionSVDdEdxList = dedxTrackPiS->getDedxList();
      }
      auto trackFitPiS = partPiS->getTrackFitResult();
      if (!trackFitPiS) {m_slowPionnSVDHits = Const::doubleNaN;}
      else {m_slowPionnSVDHits = trackFitPiS->getHitPatternVXD().getNSVDHits();}

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
      m_drGamma = std::get<double>(Variable::Manager::Instance().getVariable(std::string("dr"))->function(partGamma));
      m_diraGamma = std::get<double>(Variable::Manager::Instance().getVariable(
                                       std::string("cosAngleBetweenMomentumAndVertexVector"))->function(partGamma));

      auto trackFitE1FromGamma = partE1FromGamma->getTrackFitResult();
      if (!trackFitE1FromGamma) {m_firstElectronnSVDHits = Const::doubleNaN;}
      else {m_firstElectronnSVDHits = trackFitE1FromGamma->getHitPatternVXD().getNSVDHits();}

      auto trackFitE2FromGamma = partE2FromGamma->getTrackFitResult();
      if (!trackFitE2FromGamma) {m_secondElectronnSVDHits = Const::doubleNaN;}
      else {m_secondElectronnSVDHits = trackFitE2FromGamma->getHitPatternVXD().getNSVDHits();}


      m_firstElectronMomentum = partE1FromGamma->getMomentumMagnitude();
      if (!dedxTrackE1FromGamma) {
        m_firstElectronSVDdEdx = -999.0;
        m_firstElectronSVDdEdxErr = -999.0;
        // m_firstElectronSVDdEdxMean = -999.0;
        m_firstElectrondEdxTrackMomentum = -999.0;
        m_firstElectrondEdxTrackNHits = -1;
        // m_firstElectrondEdxTrackNHitsUsed = -1;
        m_firstElectrondEdxTrackCosTheta = -999.0;
      } else {
        m_firstElectronSVDdEdx = dedxTrackE1FromGamma->getDedx(Const::EDetector::SVD);
        m_firstElectronSVDdEdxErr = dedxTrackE1FromGamma->getDedxError(Const::EDetector::SVD);
        // m_firstElectronSVDdEdxMean = dedxTrackE1FromGamma->getDedxMean(Const::EDetector::SVD);
        m_firstElectrondEdxTrackMomentum = dedxTrackE1FromGamma->getMomentum();
        m_firstElectrondEdxTrackNHits = (int) dedxTrackE1FromGamma->size();
        // m_firstElectrondEdxTrackNHitsUsed = (int) dedxTrackE1FromGamma->getNHitsUsed();
        m_firstElectrondEdxTrackCosTheta = dedxTrackE1FromGamma->getCosTheta();
        // m_firstElectronSVDdEdxList = dedxTrackE1FromGamma->getDedxList();
      }

      m_secondElectronMomentum = partE2FromGamma->getMomentumMagnitude();
      if (!dedxTrackE2FromGamma) {
        m_secondElectronSVDdEdx = -999.0;
        m_secondElectronSVDdEdxErr = -999.0;
        // m_secondElectronSVDdEdxMean = -999.0;
        m_secondElectrondEdxTrackMomentum = -999.0;
        m_secondElectrondEdxTrackNHits = -1;
        // m_secondElectrondEdxTrackNHitsUsed = -1;
        m_secondElectrondEdxTrackCosTheta = -999.0;
      } else {
        m_secondElectronSVDdEdx = dedxTrackE2FromGamma->getDedx(Const::EDetector::SVD);
        m_secondElectronSVDdEdxErr = dedxTrackE2FromGamma->getDedxError(Const::EDetector::SVD);
        // m_secondElectronSVDdEdxMean = dedxTrackE2FromGamma->getDedxMean(Const::EDetector::SVD);
        m_secondElectrondEdxTrackMomentum = dedxTrackE2FromGamma->getMomentum();
        m_secondElectrondEdxTrackNHits = (int) dedxTrackE2FromGamma->size();
        // m_secondElectrondEdxTrackNHitsUsed = (int) dedxTrackE2FromGamma->getNHitsUsed();
        m_secondElectrondEdxTrackCosTheta = dedxTrackE2FromGamma->getCosTheta();
        // m_secondElectronSVDdEdxList = dedxTrackE2FromGamma->getDedxList();
      }

      getObjectPtr<TTree>("Gamma")->Fill();
    }
  }

  if (GenericParticles->getListSize() > 0) {
    for (unsigned int iParticle = 0; iParticle < GenericParticles->getListSize(); ++iParticle) {

      const Particle* partGeneric = GenericParticles->getParticle(0);

      const VXDDedxTrack* dedxTrackGeneric = getSVDDedxFromParticle(partGeneric);

      m_genericp = partGeneric->getMomentumMagnitude();

      if (!dedxTrackGeneric) {
        m_genericSVDdEdx = -999.0;
        m_genericSVDdEdxErr = -999.0;
        // m_genericSVDdEdxMean = -999.0;
        m_genericdEdxTrackMomentum = -999.0;
        m_genericdEdxTrackNHits = -1;
        // m_genericdEdxTrackNHitsUsed = -1;
        m_genericdEdxTrackCosTheta = -999.0;
      } else {
        m_genericSVDdEdx = dedxTrackGeneric->getDedx(Const::EDetector::SVD);
        m_genericSVDdEdxErr = dedxTrackGeneric->getDedxError(Const::EDetector::SVD);
        // m_genericSVDdEdxMean = dedxTrackGeneric->getDedxMean(Const::EDetector::SVD);
        m_genericdEdxTrackMomentum = dedxTrackGeneric->getMomentum();
        m_genericdEdxTrackNHits = (int) dedxTrackGeneric->size();
        // m_genericdEdxTrackNHitsUsed = (int) dedxTrackGeneric->getNHitsUsed();
        m_genericdEdxTrackCosTheta = dedxTrackGeneric->getCosTheta();
        // m_genericSVDdEdxList = dedxTrackGeneric->getDedxList();
      }

      auto trackFitGeneric = partGeneric->getTrackFitResult();
      if (!trackFitGeneric) {m_genericnSVDHits = Const::doubleNaN;}
      else {m_genericnSVDHits = trackFitGeneric->getHitPatternVXD().getNSVDHits();}

      getObjectPtr<TTree>("Generic")->Fill();
    }
  }


}
