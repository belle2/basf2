/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/modules/svddEdxValidationCollector/SVDdEdxValidationCollectorModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/variables/Variables.h>
#include <analysis/variables/PIDVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <reconstruction/dataobjects/VXDDedxTrack.h>
#include <mdst/dataobjects/Track.h>

#include <TTree.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Variable;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDdEdxValidationCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDdEdxValidationCollectorModule::SVDdEdxValidationCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties

  setDescription("Collector module used to create the ROOT ntuples used to produce dE/dx calibration payloads");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("LambdaListName", m_LambdaListName, "Name of the Lambda particle list", std::string("Lambda0:cut"));
  addParam("DstarListName", m_DstarListName, "Name of the Dstar particle list", std::string("D*+:cut"));
  addParam("GammaListName", m_GammaListName, "Name of the Gamma particle list", std::string("gamma:cut"));
}

void SVDdEdxValidationCollectorModule::prepare()
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
  LambdaTree->Branch<double>("ProtonMomentum", &m_protonp);
  LambdaTree->Branch<double>("ProtonSVDdEdx", &m_protonSVDdEdx);

  DstarTree->Branch<double>("InvM", &m_InvMDstar);
  DstarTree->Branch<double>("D0InvM", &m_InvMD0);
  DstarTree->Branch<double>("deltaM", &m_DeltaM);
  DstarTree->Branch<double>("KaonMomentum", &m_kaonp);
  DstarTree->Branch<double>("KaonSVDdEdx", &m_kaonSVDdEdx);
  DstarTree->Branch<double>("PionDMomentum", &m_pionDp);
  DstarTree->Branch<double>("PionDSVDdEdx", &m_pionDSVDdEdx);
  DstarTree->Branch<double>("SlowPionMomentum", &m_slowPionp);
  DstarTree->Branch<double>("SlowPionSVDdEdx", &m_slowPionSVDdEdx);

  GammaTree->Branch<double>("InvM", &m_InvMGamma);
  GammaTree->Branch<double>("FirstElectronMomentum", &m_firstElectronp);
  GammaTree->Branch<double>("FirstElectronSVDdEdx", &m_firstElectronSVDdEdx);
  GammaTree->Branch<double>("SecondElectronMomentum", &m_secondElectronp);
  GammaTree->Branch<double>("SecondElectronSVDdEdx", &m_secondElectronSVDdEdx);

  // Add a plethora of potentially useful PID variables
  LambdaTree->Branch<double>("ProtonElectronIDALL", &m_protonElectronIDALL);
  LambdaTree->Branch<double>("ProtonPionIDALL", &m_protonPionIDALL);
  LambdaTree->Branch<double>("ProtonKaonIDALL", &m_protonKaonIDALL);
  LambdaTree->Branch<double>("ProtonProtonIDALL", &m_protonProtonIDALL);
  LambdaTree->Branch<double>("ProtonElectronIDnoSVD", &m_protonElectronIDnoSVD);
  LambdaTree->Branch<double>("ProtonPionIDnoSVD", &m_protonPionIDnoSVD);
  LambdaTree->Branch<double>("ProtonKaonIDnoSVD", &m_protonKaonIDnoSVD);
  LambdaTree->Branch<double>("ProtonProtonIDnoSVD", &m_protonProtonIDnoSVD);
  LambdaTree->Branch<double>("ProtonElectronIDSVDonly", &m_protonElectronIDSVDonly);
  LambdaTree->Branch<double>("ProtonPionIDSVDonly", &m_protonPionIDSVDonly);
  LambdaTree->Branch<double>("ProtonKaonIDSVDonly", &m_protonKaonIDSVDonly);
  LambdaTree->Branch<double>("ProtonProtonIDSVDonly", &m_protonProtonIDSVDonly);

  LambdaTree->Branch<double>("ProtonBinaryProtonKaonIDALL", &m_protonBinaryProtonKaonIDALL);
  LambdaTree->Branch<double>("ProtonBinaryProtonPionIDALL", &m_protonBinaryProtonPionIDALL);
  LambdaTree->Branch<double>("ProtonBinaryProtonElectronIDALL", &m_protonBinaryProtonElectronIDALL);
  LambdaTree->Branch<double>("ProtonBinaryProtonKaonIDnoSVD", &m_protonBinaryProtonKaonIDnoSVD);
  LambdaTree->Branch<double>("ProtonBinaryProtonPionIDnoSVD", &m_protonBinaryProtonPionIDnoSVD);
  LambdaTree->Branch<double>("ProtonBinaryProtonElectronIDnoSVD", &m_protonBinaryProtonElectronIDnoSVD);
  LambdaTree->Branch<double>("ProtonBinaryProtonKaonIDSVDonly", &m_protonBinaryProtonKaonIDSVDonly);
  LambdaTree->Branch<double>("ProtonBinaryProtonPionIDSVDonly", &m_protonBinaryProtonPionIDSVDonly);
  LambdaTree->Branch<double>("ProtonBinaryProtonElectronIDSVDonly", &m_protonBinaryProtonElectronIDSVDonly);

  LambdaTree->Branch<double>("ProtonBinaryKaonProtonIDALL", &m_protonBinaryKaonProtonIDALL);
  LambdaTree->Branch<double>("ProtonBinaryPionProtonIDALL", &m_protonBinaryPionProtonIDALL);
  LambdaTree->Branch<double>("ProtonBinaryElectronProtonIDALL", &m_protonBinaryElectronProtonIDALL);
  LambdaTree->Branch<double>("ProtonBinaryKaonProtonIDnoSVD", &m_protonBinaryKaonProtonIDnoSVD);
  LambdaTree->Branch<double>("ProtonBinaryPionProtonIDnoSVD", &m_protonBinaryPionProtonIDnoSVD);
  LambdaTree->Branch<double>("ProtonBinaryElectronProtonIDnoSVD", &m_protonBinaryElectronProtonIDnoSVD);
  LambdaTree->Branch<double>("ProtonBinaryKaonProtonIDSVDonly", &m_protonBinaryKaonProtonIDSVDonly);
  LambdaTree->Branch<double>("ProtonBinaryPionProtonIDSVDonly", &m_protonBinaryPionProtonIDSVDonly);
  LambdaTree->Branch<double>("ProtonBinaryElectronProtonIDSVDonly", &m_protonBinaryElectronProtonIDSVDonly);

  DstarTree->Branch<double>("KaonElectronIDALL", &m_kaonElectronIDALL);
  DstarTree->Branch<double>("KaonPionIDALL", &m_kaonPionIDALL);
  DstarTree->Branch<double>("KaonKaonIDALL", &m_kaonKaonIDALL);
  DstarTree->Branch<double>("KaonProtonIDALL", &m_kaonProtonIDALL);
  DstarTree->Branch<double>("KaonElectronIDnoSVD", &m_kaonElectronIDnoSVD);
  DstarTree->Branch<double>("KaonPionIDnoSVD", &m_kaonPionIDnoSVD);
  DstarTree->Branch<double>("KaonKaonIDnoSVD", &m_kaonKaonIDnoSVD);
  DstarTree->Branch<double>("KaonProtonIDnoSVD", &m_kaonProtonIDnoSVD);
  DstarTree->Branch<double>("KaonElectronIDSVDonly", &m_kaonElectronIDSVDonly);
  DstarTree->Branch<double>("KaonPionIDSVDonly", &m_kaonPionIDSVDonly);
  DstarTree->Branch<double>("KaonKaonIDSVDonly", &m_kaonKaonIDSVDonly);
  DstarTree->Branch<double>("KaonProtonIDSVDonly", &m_kaonProtonIDSVDonly);

  DstarTree->Branch<double>("KaonBinaryKaonProtonIDALL", &m_kaonBinaryKaonProtonIDALL);
  DstarTree->Branch<double>("KaonBinaryKaonPionIDALL", &m_kaonBinaryKaonPionIDALL);
  DstarTree->Branch<double>("KaonBinaryKaonElectronIDALL", &m_kaonBinaryKaonElectronIDALL);
  DstarTree->Branch<double>("KaonBinaryKaonProtonIDnoSVD", &m_kaonBinaryKaonProtonIDnoSVD);
  DstarTree->Branch<double>("KaonBinaryKaonPionIDnoSVD", &m_kaonBinaryKaonPionIDnoSVD);
  DstarTree->Branch<double>("KaonBinaryKaonElectronIDnoSVD", &m_kaonBinaryKaonElectronIDnoSVD);
  DstarTree->Branch<double>("KaonBinaryKaonProtonIDSVDonly", &m_kaonBinaryKaonProtonIDSVDonly);
  DstarTree->Branch<double>("KaonBinaryKaonPionIDSVDonly", &m_kaonBinaryKaonPionIDSVDonly);
  DstarTree->Branch<double>("KaonBinaryKaonElectronIDSVDonly", &m_kaonBinaryKaonElectronIDSVDonly);

  DstarTree->Branch<double>("KaonBinaryProtonKaonIDALL", &m_kaonBinaryProtonKaonIDALL);
  DstarTree->Branch<double>("KaonBinaryPionKaonIDALL", &m_kaonBinaryPionKaonIDALL);
  DstarTree->Branch<double>("KaonBinaryElectronKaonIDALL", &m_kaonBinaryElectronKaonIDALL);
  DstarTree->Branch<double>("KaonBinaryProtonKaonIDnoSVD", &m_kaonBinaryProtonKaonIDnoSVD);
  DstarTree->Branch<double>("KaonBinaryPionKaonIDnoSVD", &m_kaonBinaryPionKaonIDnoSVD);
  DstarTree->Branch<double>("KaonBinaryElectronKaonIDnoSVD", &m_kaonBinaryElectronKaonIDnoSVD);
  DstarTree->Branch<double>("KaonBinaryProtonKaonIDSVDonly", &m_kaonBinaryProtonKaonIDSVDonly);
  DstarTree->Branch<double>("KaonBinaryPionKaonIDSVDonly", &m_kaonBinaryPionKaonIDSVDonly);
  DstarTree->Branch<double>("KaonBinaryElectronKaonIDSVDonly", &m_kaonBinaryElectronKaonIDSVDonly);

  DstarTree->Branch<double>("PionDElectronIDALL", &m_pionDElectronIDALL);
  DstarTree->Branch<double>("PionDPionIDALL", &m_pionDPionIDALL);
  DstarTree->Branch<double>("PionDKaonIDALL", &m_pionDKaonIDALL);
  DstarTree->Branch<double>("PionDProtonIDALL", &m_pionDProtonIDALL);
  DstarTree->Branch<double>("PionDElectronIDnoSVD", &m_pionDElectronIDnoSVD);
  DstarTree->Branch<double>("PionDPionIDnoSVD", &m_pionDPionIDnoSVD);
  DstarTree->Branch<double>("PionDKaonIDnoSVD", &m_pionDKaonIDnoSVD);
  DstarTree->Branch<double>("PionDProtonIDnoSVD", &m_pionDProtonIDnoSVD);
  DstarTree->Branch<double>("PionDElectronIDSVDonly", &m_pionDElectronIDSVDonly);
  DstarTree->Branch<double>("PionDPionIDSVDonly", &m_pionDPionIDSVDonly);
  DstarTree->Branch<double>("PionDKaonIDSVDonly", &m_pionDKaonIDSVDonly);
  DstarTree->Branch<double>("PionDProtonIDSVDonly", &m_pionDProtonIDSVDonly);

  DstarTree->Branch<double>("PionDBinaryPionProtonIDALL", &m_pionDBinaryPionProtonIDALL);
  DstarTree->Branch<double>("PionDBinaryPionKaonIDALL", &m_pionDBinaryPionKaonIDALL);
  DstarTree->Branch<double>("PionDBinaryPionElectronIDALL", &m_pionDBinaryPionElectronIDALL);
  DstarTree->Branch<double>("PionDBinaryPionProtonIDnoSVD", &m_pionDBinaryPionProtonIDnoSVD);
  DstarTree->Branch<double>("PionDBinaryPionKaonIDnoSVD", &m_pionDBinaryPionKaonIDnoSVD);
  DstarTree->Branch<double>("PionDBinaryPionElectronIDnoSVD", &m_pionDBinaryPionElectronIDnoSVD);
  DstarTree->Branch<double>("PionDBinaryPionProtonIDSVDonly", &m_pionDBinaryPionProtonIDSVDonly);
  DstarTree->Branch<double>("PionDBinaryPionKaonIDSVDonly", &m_pionDBinaryPionKaonIDSVDonly);
  DstarTree->Branch<double>("PionDBinaryPionElectronIDSVDonly", &m_pionDBinaryPionElectronIDSVDonly);

  DstarTree->Branch<double>("PionDBinaryProtonPionIDALL", &m_pionDBinaryProtonPionIDALL);
  DstarTree->Branch<double>("PionDBinaryKaonPionIDALL", &m_pionDBinaryKaonPionIDALL);
  DstarTree->Branch<double>("PionDBinaryElectronPionIDALL", &m_pionDBinaryElectronPionIDALL);
  DstarTree->Branch<double>("PionDBinaryProtonPionIDnoSVD", &m_pionDBinaryProtonPionIDnoSVD);
  DstarTree->Branch<double>("PionDBinaryKaonPionIDnoSVD", &m_pionDBinaryKaonPionIDnoSVD);
  DstarTree->Branch<double>("PionDBinaryElectronPionIDnoSVD", &m_pionDBinaryElectronPionIDnoSVD);
  DstarTree->Branch<double>("PionDBinaryProtonPionIDSVDonly", &m_pionDBinaryProtonPionIDSVDonly);
  DstarTree->Branch<double>("PionDBinaryKaonPionIDSVDonly", &m_pionDBinaryKaonPionIDSVDonly);
  DstarTree->Branch<double>("PionDBinaryElectronPionIDSVDonly", &m_pionDBinaryElectronPionIDSVDonly);

  DstarTree->Branch<double>("SlowPionElectronIDALL", &m_slowPionElectronIDALL);
  DstarTree->Branch<double>("SlowPionPionIDALL", &m_slowPionPionIDALL);
  DstarTree->Branch<double>("SlowPionKaonIDALL", &m_slowPionKaonIDALL);
  DstarTree->Branch<double>("SlowPionProtonIDALL", &m_slowPionProtonIDALL);
  DstarTree->Branch<double>("SlowPionElectronIDnoSVD", &m_slowPionElectronIDnoSVD);
  DstarTree->Branch<double>("SlowPionPionIDnoSVD", &m_slowPionPionIDnoSVD);
  DstarTree->Branch<double>("SlowPionKaonIDnoSVD", &m_slowPionKaonIDnoSVD);
  DstarTree->Branch<double>("SlowPionProtonIDnoSVD", &m_slowPionProtonIDnoSVD);
  DstarTree->Branch<double>("SlowPionElectronIDSVDonly", &m_slowPionElectronIDSVDonly);
  DstarTree->Branch<double>("SlowPionPionIDSVDonly", &m_slowPionPionIDSVDonly);
  DstarTree->Branch<double>("SlowPionKaonIDSVDonly", &m_slowPionKaonIDSVDonly);
  DstarTree->Branch<double>("SlowPionProtonIDSVDonly", &m_slowPionProtonIDSVDonly);

  DstarTree->Branch<double>("SlowPionBinaryPionProtonIDALL", &m_slowPionBinaryPionProtonIDALL);
  DstarTree->Branch<double>("SlowPionBinaryPionKaonIDALL", &m_slowPionBinaryPionKaonIDALL);
  DstarTree->Branch<double>("SlowPionBinaryPionElectronIDALL", &m_slowPionBinaryPionElectronIDALL);
  DstarTree->Branch<double>("SlowPionBinaryPionProtonIDnoSVD", &m_slowPionBinaryPionProtonIDnoSVD);
  DstarTree->Branch<double>("SlowPionBinaryPionKaonIDnoSVD", &m_slowPionBinaryPionKaonIDnoSVD);
  DstarTree->Branch<double>("SlowPionBinaryPionElectronIDnoSVD", &m_slowPionBinaryPionElectronIDnoSVD);
  DstarTree->Branch<double>("SlowPionBinaryPionProtonIDSVDonly", &m_slowPionBinaryPionProtonIDSVDonly);
  DstarTree->Branch<double>("SlowPionBinaryPionKaonIDSVDonly", &m_slowPionBinaryPionKaonIDSVDonly);
  DstarTree->Branch<double>("SlowPionBinaryPionElectronIDSVDonly", &m_slowPionBinaryPionElectronIDSVDonly);

  DstarTree->Branch<double>("SlowPionBinaryProtonPionIDALL", &m_slowPionBinaryProtonPionIDALL);
  DstarTree->Branch<double>("SlowPionBinaryKaonPionIDALL", &m_slowPionBinaryKaonPionIDALL);
  DstarTree->Branch<double>("SlowPionBinaryElectronPionIDALL", &m_slowPionBinaryElectronPionIDALL);
  DstarTree->Branch<double>("SlowPionBinaryProtonPionIDnoSVD", &m_slowPionBinaryProtonPionIDnoSVD);
  DstarTree->Branch<double>("SlowPionBinaryKaonPionIDnoSVD", &m_slowPionBinaryKaonPionIDnoSVD);
  DstarTree->Branch<double>("SlowPionBinaryElectronPionIDnoSVD", &m_slowPionBinaryElectronPionIDnoSVD);
  DstarTree->Branch<double>("SlowPionBinaryProtonPionIDSVDonly", &m_slowPionBinaryProtonPionIDSVDonly);
  DstarTree->Branch<double>("SlowPionBinaryKaonPionIDSVDonly", &m_slowPionBinaryKaonPionIDSVDonly);
  DstarTree->Branch<double>("SlowPionBinaryElectronPionIDSVDonly", &m_slowPionBinaryElectronPionIDSVDonly);

  GammaTree->Branch<double>("FirstElectronElectronIDALL", &m_firstElectronElectronIDALL);
  GammaTree->Branch<double>("FirstElectronPionIDALL", &m_firstElectronPionIDALL);
  GammaTree->Branch<double>("FirstElectronKaonIDALL", &m_firstElectronKaonIDALL);
  GammaTree->Branch<double>("FirstElectronProtonIDALL", &m_firstElectronProtonIDALL);
  GammaTree->Branch<double>("FirstElectronElectronIDnoSVD", &m_firstElectronElectronIDnoSVD);
  GammaTree->Branch<double>("FirstElectronPionIDnoSVD", &m_firstElectronPionIDnoSVD);
  GammaTree->Branch<double>("FirstElectronKaonIDnoSVD", &m_firstElectronKaonIDnoSVD);
  GammaTree->Branch<double>("FirstElectronProtonIDnoSVD", &m_firstElectronProtonIDnoSVD);
  GammaTree->Branch<double>("FirstElectronElectronIDSVDonly", &m_firstElectronElectronIDSVDonly);
  GammaTree->Branch<double>("FirstElectronPionIDSVDonly", &m_firstElectronPionIDSVDonly);
  GammaTree->Branch<double>("FirstElectronKaonIDSVDonly", &m_firstElectronKaonIDSVDonly);
  GammaTree->Branch<double>("FirstElectronProtonIDSVDonly", &m_firstElectronProtonIDSVDonly);

  GammaTree->Branch<double>("FirstElectronBinaryElectronProtonIDALL", &m_firstElectronBinaryElectronProtonIDALL);
  GammaTree->Branch<double>("FirstElectronBinaryElectronKaonIDALL", &m_firstElectronBinaryElectronKaonIDALL);
  GammaTree->Branch<double>("FirstElectronBinaryElectronPionIDALL", &m_firstElectronBinaryElectronPionIDALL);
  GammaTree->Branch<double>("FirstElectronBinaryElectronProtonIDnoSVD", &m_firstElectronBinaryElectronProtonIDnoSVD);
  GammaTree->Branch<double>("FirstElectronBinaryElectronKaonIDnoSVD", &m_firstElectronBinaryElectronKaonIDnoSVD);
  GammaTree->Branch<double>("FirstElectronBinaryElectronPionIDnoSVD", &m_firstElectronBinaryElectronPionIDnoSVD);
  GammaTree->Branch<double>("FirstElectronBinaryElectronProtonIDSVDonly", &m_firstElectronBinaryElectronProtonIDSVDonly);
  GammaTree->Branch<double>("FirstElectronBinaryElectronKaonIDSVDonly", &m_firstElectronBinaryElectronKaonIDSVDonly);
  GammaTree->Branch<double>("FirstElectronBinaryElectronPionIDSVDonly", &m_firstElectronBinaryElectronPionIDSVDonly);

  GammaTree->Branch<double>("FirstElectronBinaryProtonElectronIDALL", &m_firstElectronBinaryProtonElectronIDALL);
  GammaTree->Branch<double>("FirstElectronBinaryKaonElectronIDALL", &m_firstElectronBinaryKaonElectronIDALL);
  GammaTree->Branch<double>("FirstElectronBinaryPionElectronIDALL", &m_firstElectronBinaryPionElectronIDALL);
  GammaTree->Branch<double>("FirstElectronBinaryProtonElectronIDnoSVD", &m_firstElectronBinaryProtonElectronIDnoSVD);
  GammaTree->Branch<double>("FirstElectronBinaryKaonElectronIDnoSVD", &m_firstElectronBinaryKaonElectronIDnoSVD);
  GammaTree->Branch<double>("FirstElectronBinaryPionElectronIDnoSVD", &m_firstElectronBinaryPionElectronIDnoSVD);
  GammaTree->Branch<double>("FirstElectronBinaryProtonElectronIDSVDonly", &m_firstElectronBinaryProtonElectronIDSVDonly);
  GammaTree->Branch<double>("FirstElectronBinaryKaonElectronIDSVDonly", &m_firstElectronBinaryKaonElectronIDSVDonly);
  GammaTree->Branch<double>("FirstElectronBinaryPionElectronIDSVDonly", &m_firstElectronBinaryPionElectronIDSVDonly);

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

void SVDdEdxValidationCollectorModule::collect()
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
      // const Particle* partPiFromLambda = LambdaParticles->getParticle(0)->getDaughter(1);

      const VXDDedxTrack* dedxTrackPFromLambda = getSVDDedxFromParticle(partPFromLambda);
      // const VXDDedxTrack* dedxTrackPiFromLambda = getSVDDedxFromParticle(partPiFromLambda);

      m_InvMLambda = partLambda->getMass();
      m_protonp = partPFromLambda->getMomentumMagnitude();

      if (!dedxTrackPFromLambda) {
        m_protonSVDdEdx = -999.0;
      } else {
        m_protonSVDdEdx = dedxTrackPFromLambda->getDedx(Const::EDetector::SVD);
      }



      m_protonElectronIDALL = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonPionIDALL = std::get<double>
                          (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonKaonIDALL = std::get<double>
                          (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonProtonIDALL = std::get<double>
                            (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                               partPFromLambda));
      m_protonElectronIDnoSVD = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonPionIDnoSVD = std::get<double>
                            (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonKaonIDnoSVD = std::get<double>
                            (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonProtonIDnoSVD = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonElectronIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                                     partPFromLambda));
      m_protonPionIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                                 partPFromLambda));
      m_protonKaonIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                                 partPFromLambda));
      m_protonProtonIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                                   partPFromLambda));

      m_protonBinaryProtonKaonIDALL = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                         partPFromLambda));
      m_protonBinaryProtonPionIDALL = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                         partPFromLambda));
      m_protonBinaryProtonElectronIDALL = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                             partPFromLambda));
      m_protonBinaryProtonKaonIDnoSVD = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, CDC, TOP, ARICH, ECL, KLM)")->function(
                                           partPFromLambda));
      m_protonBinaryProtonPionIDnoSVD = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, CDC, TOP, ARICH, ECL, KLM)")->function(
                                           partPFromLambda));
      m_protonBinaryProtonElectronIDnoSVD = std::get<double>
                                            (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, CDC, TOP, ARICH, ECL, KLM)")->function(
                                               partPFromLambda));
      m_protonBinaryProtonKaonIDSVDonly = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, SVD)")->function(partPFromLambda));
      m_protonBinaryProtonPionIDSVDonly = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD)")->function(partPFromLambda));
      m_protonBinaryProtonElectronIDSVDonly = std::get<double>
                                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, SVD)")->function(partPFromLambda));

      m_protonBinaryKaonProtonIDALL = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                         partPFromLambda));
      m_protonBinaryPionProtonIDALL = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                         partPFromLambda));
      m_protonBinaryElectronProtonIDALL = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                             partPFromLambda));
      m_protonBinaryKaonProtonIDnoSVD = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                           partPFromLambda));
      m_protonBinaryPionProtonIDnoSVD = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                           partPFromLambda));
      m_protonBinaryElectronProtonIDnoSVD = std::get<double>
                                            (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                               partPFromLambda));
      m_protonBinaryKaonProtonIDSVDonly = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, SVD)")->function(partPFromLambda));
      m_protonBinaryPionProtonIDSVDonly = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD)")->function(partPFromLambda));
      m_protonBinaryElectronProtonIDSVDonly = std::get<double>
                                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, SVD)")->function(partPFromLambda));

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

      m_kaonp = partKFromD->getMomentumMagnitude();
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

      m_slowPionp = partPiS->getMomentumMagnitude();
      if (!dedxTrackPiS) {
        m_slowPionSVDdEdx = -999.0;
      } else {
        m_slowPionSVDdEdx = dedxTrackPiS->getDedx(Const::EDetector::SVD);
      }

      m_kaonElectronIDALL = std::get<double>
                            (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonPionIDALL = std::get<double>
                        (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonKaonIDALL = std::get<double>
                        (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonProtonIDALL = std::get<double>
                          (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonElectronIDnoSVD = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonPionIDnoSVD = std::get<double>
                          (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonKaonIDnoSVD = std::get<double>
                          (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonProtonIDnoSVD = std::get<double>
                            (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonElectronIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                                   partKFromD));
      m_kaonPionIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                               partKFromD));
      m_kaonKaonIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                               partKFromD));
      m_kaonProtonIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                                 partKFromD));

      m_kaonBinaryKaonProtonIDALL = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partKFromD));
      m_kaonBinaryKaonPionIDALL = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                     partKFromD));
      m_kaonBinaryKaonElectronIDALL = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                         partKFromD));
      m_kaonBinaryKaonProtonIDnoSVD = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonBinaryKaonPionIDnoSVD = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonBinaryKaonElectronIDnoSVD = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonBinaryKaonProtonIDSVDonly = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, SVD)")->function(partKFromD));
      m_kaonBinaryKaonPionIDSVDonly = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD)")->function(partKFromD));
      m_kaonBinaryKaonElectronIDSVDonly = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, SVD)")->function(partKFromD));

      m_kaonBinaryProtonKaonIDALL = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partKFromD));
      m_kaonBinaryPionKaonIDALL = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                     partKFromD));
      m_kaonBinaryElectronKaonIDALL = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                         partKFromD));
      m_kaonBinaryProtonKaonIDnoSVD = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonBinaryPionKaonIDnoSVD = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonBinaryElectronKaonIDnoSVD = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonBinaryProtonKaonIDSVDonly = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, SVD)")->function(partKFromD));
      m_kaonBinaryPionKaonIDSVDonly = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD)")->function(partKFromD));
      m_kaonBinaryElectronKaonIDSVDonly = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, SVD)")->function(partKFromD));


      m_pionDElectronIDALL = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDPionIDALL = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDKaonIDALL = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDProtonIDALL = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDElectronIDnoSVD = std::get<double>
                               (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDPionIDnoSVD = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDKaonIDnoSVD = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDProtonIDnoSVD = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDElectronIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                                    partPiFromD));
      m_pionDPionIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                                partPiFromD));
      m_pionDKaonIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                                partPiFromD));
      m_pionDProtonIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                                  partPiFromD));

      m_pionDBinaryPionProtonIDALL = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                        partPiFromD));
      m_pionDBinaryPionKaonIDALL = std::get<double>
                                   (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                      partPiFromD));
      m_pionDBinaryPionElectronIDALL = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                          partPiFromD));
      m_pionDBinaryPionProtonIDnoSVD = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                          partPiFromD));
      m_pionDBinaryPionKaonIDnoSVD = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDBinaryPionElectronIDnoSVD = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDBinaryPionProtonIDSVDonly = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD)")->function(partPiFromD));
      m_pionDBinaryPionKaonIDSVDonly = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD)")->function(partPiFromD));
      m_pionDBinaryPionElectronIDSVDonly = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD)")->function(partPiFromD));

      m_pionDBinaryProtonPionIDALL = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                        partPiFromD));
      m_pionDBinaryKaonPionIDALL = std::get<double>
                                   (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                      partPiFromD));
      m_pionDBinaryElectronPionIDALL = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                          partPiFromD));
      m_pionDBinaryProtonPionIDnoSVD = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, CDC, TOP, ARICH, ECL, KLM)")->function(
                                          partPiFromD));
      m_pionDBinaryKaonPionIDnoSVD = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDBinaryElectronPionIDnoSVD = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDBinaryProtonPionIDSVDonly = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD)")->function(partPiFromD));
      m_pionDBinaryKaonPionIDSVDonly = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD)")->function(partPiFromD));
      m_pionDBinaryElectronPionIDSVDonly = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD)")->function(partPiFromD));


      m_slowPionElectronIDALL = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_slowPionPionIDALL = std::get<double>
                            (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_slowPionKaonIDALL = std::get<double>
                            (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_slowPionProtonIDALL = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_slowPionElectronIDnoSVD = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_slowPionPionIDnoSVD = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_slowPionKaonIDnoSVD = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_slowPionProtonIDnoSVD = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_slowPionElectronIDSVDonly = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                       partPiS));
      m_slowPionPionIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                                   partPiS));
      m_slowPionKaonIDSVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                                   partPiS));
      m_slowPionProtonIDSVDonly = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                     partPiS));

      m_slowPionBinaryPionProtonIDALL = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                           partPiS));
      m_slowPionBinaryPionKaonIDALL = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                         partPiS));
      m_slowPionBinaryPionElectronIDALL = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                             partPiS));
      m_slowPionBinaryPionProtonIDnoSVD = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                             partPiS));
      m_slowPionBinaryPionKaonIDnoSVD = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_slowPionBinaryPionElectronIDnoSVD = std::get<double>
                                            (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_slowPionBinaryPionProtonIDSVDonly = std::get<double>
                                            (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD)")->function(partPiS));
      m_slowPionBinaryPionKaonIDSVDonly = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD)")->function(partPiS));
      m_slowPionBinaryPionElectronIDSVDonly = std::get<double>
                                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD)")->function(partPiS));

      m_slowPionBinaryProtonPionIDALL = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                           partPiS));
      m_slowPionBinaryKaonPionIDALL = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                         partPiS));
      m_slowPionBinaryElectronPionIDALL = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                             partPiS));
      m_slowPionBinaryProtonPionIDnoSVD = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, CDC, TOP, ARICH, ECL, KLM)")->function(
                                             partPiS));
      m_slowPionBinaryKaonPionIDnoSVD = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_slowPionBinaryElectronPionIDnoSVD = std::get<double>
                                            (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_slowPionBinaryProtonPionIDSVDonly = std::get<double>
                                            (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD)")->function(partPiS));
      m_slowPionBinaryKaonPionIDSVDonly = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD)")->function(partPiS));
      m_slowPionBinaryElectronPionIDSVDonly = std::get<double>
                                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD)")->function(partPiS));

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

      m_firstElectronp = partE1FromGamma->getMomentumMagnitude();
      if (!dedxTrackE1FromGamma) {
        m_firstElectronSVDdEdx = -999.0;
      } else {
        m_firstElectronSVDdEdx = dedxTrackE1FromGamma->getDedx(Const::EDetector::SVD);
      }

      m_secondElectronp = partE2FromGamma->getMomentumMagnitude();
      if (!dedxTrackE2FromGamma) {
        m_secondElectronSVDdEdx = -999.0;
      } else {
        m_secondElectronSVDdEdx = dedxTrackE2FromGamma->getDedx(Const::EDetector::SVD);
      }

      m_firstElectronElectronIDALL = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronPionIDALL = std::get<double>
                                 (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronKaonIDALL = std::get<double>
                                 (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronProtonIDALL = std::get<double>
                                   (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                      partE1FromGamma));
      m_firstElectronElectronIDnoSVD = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronPionIDnoSVD = std::get<double>
                                   (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronKaonIDnoSVD = std::get<double>
                                   (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronProtonIDnoSVD = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronElectronIDSVDonly = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                            partE1FromGamma));
      m_firstElectronPionIDSVDonly = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                        partE1FromGamma));
      m_firstElectronKaonIDSVDonly = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                        partE1FromGamma));
      m_firstElectronProtonIDSVDonly = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                          partE1FromGamma));

      m_firstElectronBinaryElectronProtonIDALL = std::get<double>
                                                 (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                    partE1FromGamma));
      m_firstElectronBinaryElectronKaonIDALL = std::get<double>
                                               (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                  partE1FromGamma));
      m_firstElectronBinaryElectronPionIDALL = std::get<double>
                                               (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                  partE1FromGamma));
      m_firstElectronBinaryElectronProtonIDnoSVD = std::get<double>
                                                   (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                      partE1FromGamma));
      m_firstElectronBinaryElectronKaonIDnoSVD = std::get<double>
                                                 (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                    partE1FromGamma));
      m_firstElectronBinaryElectronPionIDnoSVD = std::get<double>
                                                 (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                    partE1FromGamma));
      m_firstElectronBinaryElectronProtonIDSVDonly = std::get<double>
                                                     (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, SVD)")->function(partE1FromGamma));
      m_firstElectronBinaryElectronKaonIDSVDonly = std::get<double>
                                                   (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, SVD)")->function(partE1FromGamma));
      m_firstElectronBinaryElectronPionIDSVDonly = std::get<double>
                                                   (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD)")->function(partE1FromGamma));

      m_firstElectronBinaryProtonElectronIDALL = std::get<double>
                                                 (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                    partE1FromGamma));
      m_firstElectronBinaryKaonElectronIDALL = std::get<double>
                                               (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                  partE1FromGamma));
      m_firstElectronBinaryPionElectronIDALL = std::get<double>
                                               (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                  partE1FromGamma));
      m_firstElectronBinaryProtonElectronIDnoSVD = std::get<double>
                                                   (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                      partE1FromGamma));
      m_firstElectronBinaryKaonElectronIDnoSVD = std::get<double>
                                                 (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                    partE1FromGamma));
      m_firstElectronBinaryPionElectronIDnoSVD = std::get<double>
                                                 (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                    partE1FromGamma));
      m_firstElectronBinaryProtonElectronIDSVDonly = std::get<double>
                                                     (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, SVD)")->function(partE1FromGamma));
      m_firstElectronBinaryKaonElectronIDSVDonly = std::get<double>
                                                   (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, SVD)")->function(partE1FromGamma));
      m_firstElectronBinaryPionElectronIDSVDonly = std::get<double>
                                                   (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD)")->function(partE1FromGamma));

      getObjectPtr<TTree>("Gamma")->Fill();
    }
  }
}
