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
  LambdaTree->Branch<double>("p_p", &m_protonp);
  LambdaTree->Branch<double>("p_SVDdEdx", &m_protonSVDdEdx);
  // LambdaTree->Branch<double>("pi_p", &m_pionDp);
  // LambdaTree->Branch<double>("pi_SVDdEdx", &m_pionDSVDdEdx);

  DstarTree->Branch<double>("InvM", &m_InvMDstar);
  DstarTree->Branch<double>("D0_InvM", &m_InvMD0);
  DstarTree->Branch<double>("deltaM", &m_DeltaM);
  DstarTree->Branch<double>("K_p", &m_kaonp);
  DstarTree->Branch<double>("K_SVDdEdx", &m_kaonSVDdEdx);
  DstarTree->Branch<double>("pi_p", &m_pionDp);
  DstarTree->Branch<double>("pi_SVDdEdx", &m_pionDSVDdEdx);
  DstarTree->Branch<double>("piS_p", &m_softPionp);
  DstarTree->Branch<double>("piS_SVDdEdx", &m_softPionSVDdEdx);

  GammaTree->Branch<double>("InvM", &m_InvMGamma);
  GammaTree->Branch<double>("e_1_p", &m_firstElectronp);
  GammaTree->Branch<double>("e_1_SVDdEdx", &m_firstElectronSVDdEdx);
  GammaTree->Branch<double>("e_2_p", &m_secondElectronp);
  GammaTree->Branch<double>("e_2_SVDdEdx", &m_secondElectronSVDdEdx);

  // Add a plethora of potentially useful PID variables
  LambdaTree->Branch<double>("p_electronID_ALL", &m_protonElectronID_ALL);
  LambdaTree->Branch<double>("p_pionID_ALL", &m_protonPionID_ALL);
  LambdaTree->Branch<double>("p_kaonID_ALL", &m_protonKaonID_ALL);
  LambdaTree->Branch<double>("p_protonID_ALL", &m_protonProtonID_ALL);
  LambdaTree->Branch<double>("p_electronID_noSVD", &m_protonElectronID_noSVD);
  LambdaTree->Branch<double>("p_pionID_noSVD", &m_protonPionID_noSVD);
  LambdaTree->Branch<double>("p_kaonID_noSVD", &m_protonKaonID_noSVD);
  LambdaTree->Branch<double>("p_protonID_noSVD", &m_protonProtonID_noSVD);
  LambdaTree->Branch<double>("p_electronID_SVDonly", &m_protonElectronID_SVDonly);
  LambdaTree->Branch<double>("p_pionID_SVDonly", &m_protonPionID_SVDonly);
  LambdaTree->Branch<double>("p_kaonID_SVDonly", &m_protonKaonID_SVDonly);
  LambdaTree->Branch<double>("p_protonID_SVDonly", &m_protonProtonID_SVDonly);

  LambdaTree->Branch<double>("p_proton_kaonID_ALL", &m_protonBinaryProtonKaonID_ALL);
  LambdaTree->Branch<double>("p_proton_pionID_ALL", &m_protonBinaryProtonPionID_ALL);
  LambdaTree->Branch<double>("p_proton_electronID_ALL", &m_protonBinaryProtonElectronID_ALL);
  LambdaTree->Branch<double>("p_proton_kaonID_noSVD", &m_protonBinaryProtonKaonID_noSVD);
  LambdaTree->Branch<double>("p_proton_pionID_noSVD", &m_protonBinaryProtonPionID_noSVD);
  LambdaTree->Branch<double>("p_proton_electronID_noSVD", &m_protonBinaryProtonElectronID_noSVD);
  LambdaTree->Branch<double>("p_proton_kaonID_SVDonly", &m_protonBinaryProtonKaonID_SVDonly);
  LambdaTree->Branch<double>("p_proton_pionID_SVDonly", &m_protonBinaryProtonPionID_SVDonly);
  LambdaTree->Branch<double>("p_proton_electronID_SVDonly", &m_protonBinaryProtonElectronID_SVDonly);

  LambdaTree->Branch<double>("p_kaon_protonID_ALL", &m_protonBinaryKaonProtonID_ALL);
  LambdaTree->Branch<double>("p_pion_protonID_ALL", &m_protonBinaryPionProtonID_ALL);
  LambdaTree->Branch<double>("p_electron_protonID_ALL", &m_protonBinaryElectronProtonID_ALL);
  LambdaTree->Branch<double>("p_kaon_protonID_noSVD", &m_protonBinaryKaonProtonID_noSVD);
  LambdaTree->Branch<double>("p_pion_protonID_noSVD", &m_protonBinaryPionProtonID_noSVD);
  LambdaTree->Branch<double>("p_electron_protonID_noSVD", &m_protonBinaryElectronProtonID_noSVD);
  LambdaTree->Branch<double>("p_kaon_protonID_SVDonly", &m_protonBinaryKaonProtonID_SVDonly);
  LambdaTree->Branch<double>("p_pion_protonID_SVDonly", &m_protonBinaryPionProtonID_SVDonly);
  LambdaTree->Branch<double>("p_electron_protonID_SVDonly", &m_protonBinaryElectronProtonID_SVDonly);

  DstarTree->Branch<double>("K_electronID_ALL", &m_kaonElectronID_ALL);
  DstarTree->Branch<double>("K_pionID_ALL", &m_kaonPionID_ALL);
  DstarTree->Branch<double>("K_kaonID_ALL", &m_kaonKaonID_ALL);
  DstarTree->Branch<double>("K_protonID_ALL", &m_kaonProtonID_ALL);
  DstarTree->Branch<double>("K_electronID_noSVD", &m_kaonElectronID_noSVD);
  DstarTree->Branch<double>("K_pionID_noSVD", &m_kaonPionID_noSVD);
  DstarTree->Branch<double>("K_kaonID_noSVD", &m_kaonKaonID_noSVD);
  DstarTree->Branch<double>("K_protonID_noSVD", &m_kaonProtonID_noSVD);
  DstarTree->Branch<double>("K_electronID_SVDonly", &m_kaonElectronID_SVDonly);
  DstarTree->Branch<double>("K_pionID_SVDonly", &m_kaonPionID_SVDonly);
  DstarTree->Branch<double>("K_kaonID_SVDonly", &m_kaonKaonID_SVDonly);
  DstarTree->Branch<double>("K_protonID_SVDonly", &m_kaonProtonID_SVDonly);

  DstarTree->Branch<double>("K_kaon_protonID_ALL", &m_kaonBinaryKaonProtonID_ALL);
  DstarTree->Branch<double>("K_kaon_pionID_ALL", &m_kaonBinaryKaonPionID_ALL);
  DstarTree->Branch<double>("K_kaon_electronID_ALL", &m_kaonBinaryKaonElectronID_ALL);
  DstarTree->Branch<double>("K_kaon_protonID_noSVD", &m_kaonBinaryKaonProtonID_noSVD);
  DstarTree->Branch<double>("K_kaon_pionID_noSVD", &m_kaonBinaryKaonPionID_noSVD);
  DstarTree->Branch<double>("K_kaon_electronID_noSVD", &m_kaonBinaryKaonElectronID_noSVD);
  DstarTree->Branch<double>("K_kaon_protonID_SVDonly", &m_kaonBinaryKaonProtonID_SVDonly);
  DstarTree->Branch<double>("K_kaon_pionID_SVDonly", &m_kaonBinaryKaonPionID_SVDonly);
  DstarTree->Branch<double>("K_kaon_electronID_SVDonly", &m_kaonBinaryKaonElectronID_SVDonly);

  DstarTree->Branch<double>("K_proton_kaonID_ALL", &m_kaonBinaryProtonKaonID_ALL);
  DstarTree->Branch<double>("K_pion_kaonID_ALL", &m_kaonBinaryPionKaonID_ALL);
  DstarTree->Branch<double>("K_electron_kaonID_ALL", &m_kaonBinaryElectronKaonID_ALL);
  DstarTree->Branch<double>("K_proton_kaonID_noSVD", &m_kaonBinaryProtonKaonID_noSVD);
  DstarTree->Branch<double>("K_pion_kaonID_noSVD", &m_kaonBinaryPionKaonID_noSVD);
  DstarTree->Branch<double>("K_electron_kaonID_noSVD", &m_kaonBinaryElectronKaonID_noSVD);
  DstarTree->Branch<double>("K_proton_kaonID_SVDonly", &m_kaonBinaryProtonKaonID_SVDonly);
  DstarTree->Branch<double>("K_pion_kaonID_SVDonly", &m_kaonBinaryPionKaonID_SVDonly);
  DstarTree->Branch<double>("K_electron_kaonID_SVDonly", &m_kaonBinaryElectronKaonID_SVDonly);

  DstarTree->Branch<double>("pi_electronID_ALL", &m_pionDElectronID_ALL);
  DstarTree->Branch<double>("pi_pionID_ALL", &m_pionDPionID_ALL);
  DstarTree->Branch<double>("pi_kaonID_ALL", &m_pionDKaonID_ALL);
  DstarTree->Branch<double>("pi_protonID_ALL", &m_pionDProtonID_ALL);
  DstarTree->Branch<double>("pi_electronID_noSVD", &m_pionDElectronID_noSVD);
  DstarTree->Branch<double>("pi_pionID_noSVD", &m_pionDPionID_noSVD);
  DstarTree->Branch<double>("pi_kaonID_noSVD", &m_pionDKaonID_noSVD);
  DstarTree->Branch<double>("pi_protonID_noSVD", &m_pionDProtonID_noSVD);
  DstarTree->Branch<double>("pi_electronID_SVDonly", &m_pionDElectronID_SVDonly);
  DstarTree->Branch<double>("pi_pionID_SVDonly", &m_pionDPionID_SVDonly);
  DstarTree->Branch<double>("pi_kaonID_SVDonly", &m_pionDKaonID_SVDonly);
  DstarTree->Branch<double>("pi_protonID_SVDonly", &m_pionDProtonID_SVDonly);

  DstarTree->Branch<double>("pi_pion_protonID_ALL", &m_pionDBinaryPionProtonID_ALL);
  DstarTree->Branch<double>("pi_pion_kaonID_ALL", &m_pionDBinaryPionKaonID_ALL);
  DstarTree->Branch<double>("pi_pion_electronID_ALL", &m_pionDBinaryPionElectronID_ALL);
  DstarTree->Branch<double>("pi_pion_protonID_noSVD", &m_pionDBinaryPionProtonID_noSVD);
  DstarTree->Branch<double>("pi_pion_kaonID_noSVD", &m_pionDBinaryPionKaonID_noSVD);
  DstarTree->Branch<double>("pi_pion_electronID_noSVD", &m_pionDBinaryPionElectronID_noSVD);
  DstarTree->Branch<double>("pi_pion_protonID_SVDonly", &m_pionDBinaryPionProtonID_SVDonly);
  DstarTree->Branch<double>("pi_pion_kaonID_SVDonly", &m_pionDBinaryPionKaonID_SVDonly);
  DstarTree->Branch<double>("pi_pion_electronID_SVDonly", &m_pionDBinaryPionElectronID_SVDonly);

  DstarTree->Branch<double>("pi_proton_pionID_ALL", &m_pionDBinaryProtonPionID_ALL);
  DstarTree->Branch<double>("pi_kaon_pionID_ALL", &m_pionDBinaryKaonPionID_ALL);
  DstarTree->Branch<double>("pi_electron_pionID_ALL", &m_pionDBinaryElectronPionID_ALL);
  DstarTree->Branch<double>("pi_proton_pionID_noSVD", &m_pionDBinaryProtonPionID_noSVD);
  DstarTree->Branch<double>("pi_kaon_pionID_noSVD", &m_pionDBinaryKaonPionID_noSVD);
  DstarTree->Branch<double>("pi_electron_pionID_noSVD", &m_pionDBinaryElectronPionID_noSVD);
  DstarTree->Branch<double>("pi_proton_pionID_SVDonly", &m_pionDBinaryProtonPionID_SVDonly);
  DstarTree->Branch<double>("pi_kaon_pionID_SVDonly", &m_pionDBinaryKaonPionID_SVDonly);
  DstarTree->Branch<double>("pi_electron_pionID_SVDonly", &m_pionDBinaryElectronPionID_SVDonly);

  DstarTree->Branch<double>("piS_electronID_ALL", &m_softPionElectronID_ALL);
  DstarTree->Branch<double>("piS_pionID_ALL", &m_softPionPionID_ALL);
  DstarTree->Branch<double>("piS_kaonID_ALL", &m_softPionKaonID_ALL);
  DstarTree->Branch<double>("piS_protonID_ALL", &m_softPionProtonID_ALL);
  DstarTree->Branch<double>("piS_electronID_noSVD", &m_softPionElectronID_noSVD);
  DstarTree->Branch<double>("piS_pionID_noSVD", &m_softPionPionID_noSVD);
  DstarTree->Branch<double>("piS_kaonID_noSVD", &m_softPionKaonID_noSVD);
  DstarTree->Branch<double>("piS_protonID_noSVD", &m_softPionProtonID_noSVD);
  DstarTree->Branch<double>("piS_electronID_SVDonly", &m_softPionElectronID_SVDonly);
  DstarTree->Branch<double>("piS_pionID_SVDonly", &m_softPionPionID_SVDonly);
  DstarTree->Branch<double>("piS_kaonID_SVDonly", &m_softPionKaonID_SVDonly);
  DstarTree->Branch<double>("piS_protonID_SVDonly", &m_softPionProtonID_SVDonly);

  DstarTree->Branch<double>("piS_pion_protonID_ALL", &m_softPionBinaryPionProtonID_ALL);
  DstarTree->Branch<double>("piS_pion_kaonID_ALL", &m_softPionBinaryPionKaonID_ALL);
  DstarTree->Branch<double>("piS_pion_electronID_ALL", &m_softPionBinaryPionElectronID_ALL);
  DstarTree->Branch<double>("piS_pion_protonID_noSVD", &m_softPionBinaryPionProtonID_noSVD);
  DstarTree->Branch<double>("piS_pion_kaonID_noSVD", &m_softPionBinaryPionKaonID_noSVD);
  DstarTree->Branch<double>("piS_pion_electronID_noSVD", &m_softPionBinaryPionElectronID_noSVD);
  DstarTree->Branch<double>("piS_pion_protonID_SVDonly", &m_softPionBinaryPionProtonID_SVDonly);
  DstarTree->Branch<double>("piS_pion_kaonID_SVDonly", &m_softPionBinaryPionKaonID_SVDonly);
  DstarTree->Branch<double>("piS_pion_electronID_SVDonly", &m_softPionBinaryPionElectronID_SVDonly);

  DstarTree->Branch<double>("piS_proton_pionID_ALL", &m_softPionBinaryProtonPionID_ALL);
  DstarTree->Branch<double>("piS_kaon_pionID_ALL", &m_softPionBinaryKaonPionID_ALL);
  DstarTree->Branch<double>("piS_electron_pionID_ALL", &m_softPionBinaryElectronPionID_ALL);
  DstarTree->Branch<double>("piS_proton_pionID_noSVD", &m_softPionBinaryProtonPionID_noSVD);
  DstarTree->Branch<double>("piS_kaon_pionID_noSVD", &m_softPionBinaryKaonPionID_noSVD);
  DstarTree->Branch<double>("piS_electron_pionID_noSVD", &m_softPionBinaryElectronPionID_noSVD);
  DstarTree->Branch<double>("piS_proton_pionID_SVDonly", &m_softPionBinaryProtonPionID_SVDonly);
  DstarTree->Branch<double>("piS_kaon_pionID_SVDonly", &m_softPionBinaryKaonPionID_SVDonly);
  DstarTree->Branch<double>("piS_electron_pionID_SVDonly", &m_softPionBinaryElectronPionID_SVDonly);

  GammaTree->Branch<double>("e_1_electronID_ALL", &m_firstElectronElectronID_ALL);
  GammaTree->Branch<double>("e_1_pionID_ALL", &m_firstElectronPionID_ALL);
  GammaTree->Branch<double>("e_1_kaonID_ALL", &m_firstElectronKaonID_ALL);
  GammaTree->Branch<double>("e_1_protonID_ALL", &m_firstElectronProtonID_ALL);
  GammaTree->Branch<double>("e_1_electronID_noSVD", &m_firstElectronElectronID_noSVD);
  GammaTree->Branch<double>("e_1_pionID_noSVD", &m_firstElectronPionID_noSVD);
  GammaTree->Branch<double>("e_1_kaonID_noSVD", &m_firstElectronKaonID_noSVD);
  GammaTree->Branch<double>("e_1_protonID_noSVD", &m_firstElectronProtonID_noSVD);
  GammaTree->Branch<double>("e_1_electronID_SVDonly", &m_firstElectronElectronID_SVDonly);
  GammaTree->Branch<double>("e_1_pionID_SVDonly", &m_firstElectronPionID_SVDonly);
  GammaTree->Branch<double>("e_1_kaonID_SVDonly", &m_firstElectronKaonID_SVDonly);
  GammaTree->Branch<double>("e_1_protonID_SVDonly", &m_firstElectronProtonID_SVDonly);

  GammaTree->Branch<double>("e_1_electron_protonID_ALL", &m_firstElectronBinaryElectronProtonID_ALL);
  GammaTree->Branch<double>("e_1_electron_kaonID_ALL", &m_firstElectronBinaryElectronKaonID_ALL);
  GammaTree->Branch<double>("e_1_electron_pionID_ALL", &m_firstElectronBinaryElectronPionID_ALL);
  GammaTree->Branch<double>("e_1_electron_protonID_noSVD", &m_firstElectronBinaryElectronProtonID_noSVD);
  GammaTree->Branch<double>("e_1_electron_kaonID_noSVD", &m_firstElectronBinaryElectronKaonID_noSVD);
  GammaTree->Branch<double>("e_1_electron_pionID_noSVD", &m_firstElectronBinaryElectronPionID_noSVD);
  GammaTree->Branch<double>("e_1_electron_protonID_SVDonly", &m_firstElectronBinaryElectronProtonID_SVDonly);
  GammaTree->Branch<double>("e_1_electron_kaonID_SVDonly", &m_firstElectronBinaryElectronKaonID_SVDonly);
  GammaTree->Branch<double>("e_1_electron_pionID_SVDonly", &m_firstElectronBinaryElectronPionID_SVDonly);

  GammaTree->Branch<double>("e_1_proton_electronID_ALL", &m_firstElectronBinaryProtonElectronID_ALL);
  GammaTree->Branch<double>("e_1_kaon_electronID_ALL", &m_firstElectronBinaryKaonElectronID_ALL);
  GammaTree->Branch<double>("e_1_pion_electronID_ALL", &m_firstElectronBinaryPionElectronID_ALL);
  GammaTree->Branch<double>("e_1_proton_electronID_noSVD", &m_firstElectronBinaryProtonElectronID_noSVD);
  GammaTree->Branch<double>("e_1_kaon_electronID_noSVD", &m_firstElectronBinaryKaonElectronID_noSVD);
  GammaTree->Branch<double>("e_1_pion_electronID_noSVD", &m_firstElectronBinaryPionElectronID_noSVD);
  GammaTree->Branch<double>("e_1_proton_electronID_SVDonly", &m_firstElectronBinaryProtonElectronID_SVDonly);
  GammaTree->Branch<double>("e_1_kaon_electronID_SVDonly", &m_firstElectronBinaryKaonElectronID_SVDonly);
  GammaTree->Branch<double>("e_1_pion_electronID_SVDonly", &m_firstElectronBinaryPionElectronID_SVDonly);

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



      m_protonElectronID_ALL = std::get<double>
                               (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonPionID_ALL = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonKaonID_ALL = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonProtonID_ALL = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                partPFromLambda));
      m_protonElectronID_noSVD = std::get<double>
                                 (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonPionID_noSVD = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonKaonID_noSVD = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonProtonID_noSVD = std::get<double>
                               (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_protonElectronID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                                      partPFromLambda));
      m_protonPionID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                                  partPFromLambda));
      m_protonKaonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                                  partPFromLambda));
      m_protonProtonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                                    partPFromLambda));

      m_protonBinaryProtonKaonID_ALL = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                          partPFromLambda));
      m_protonBinaryProtonPionID_ALL = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                          partPFromLambda));
      m_protonBinaryProtonElectronID_ALL = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                              partPFromLambda));
      m_protonBinaryProtonKaonID_noSVD = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, CDC, TOP, ARICH, ECL, KLM)")->function(
                                            partPFromLambda));
      m_protonBinaryProtonPionID_noSVD = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, CDC, TOP, ARICH, ECL, KLM)")->function(
                                            partPFromLambda));
      m_protonBinaryProtonElectronID_noSVD = std::get<double>
                                             (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                partPFromLambda));
      m_protonBinaryProtonKaonID_SVDonly = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, SVD)")->function(partPFromLambda));
      m_protonBinaryProtonPionID_SVDonly = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD)")->function(partPFromLambda));
      m_protonBinaryProtonElectronID_SVDonly = std::get<double>
                                               (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, SVD)")->function(partPFromLambda));

      m_protonBinaryKaonProtonID_ALL = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                          partPFromLambda));
      m_protonBinaryPionProtonID_ALL = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                          partPFromLambda));
      m_protonBinaryElectronProtonID_ALL = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                              partPFromLambda));
      m_protonBinaryKaonProtonID_noSVD = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                            partPFromLambda));
      m_protonBinaryPionProtonID_noSVD = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                            partPFromLambda));
      m_protonBinaryElectronProtonID_noSVD = std::get<double>
                                             (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                partPFromLambda));
      m_protonBinaryKaonProtonID_SVDonly = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, SVD)")->function(partPFromLambda));
      m_protonBinaryPionProtonID_SVDonly = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD)")->function(partPFromLambda));
      m_protonBinaryElectronProtonID_SVDonly = std::get<double>
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

      m_softPionp = partPiS->getMomentumMagnitude();
      if (!dedxTrackPiS) {
        m_softPionSVDdEdx = -999.0;
      } else {
        m_softPionSVDdEdx = dedxTrackPiS->getDedx(Const::EDetector::SVD);
      }

      m_kaonElectronID_ALL = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonPionID_ALL = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonKaonID_ALL = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonProtonID_ALL = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonElectronID_noSVD = std::get<double>
                               (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonPionID_noSVD = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonKaonID_noSVD = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonProtonID_noSVD = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonElectronID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                                    partKFromD));
      m_kaonPionID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                                partKFromD));
      m_kaonKaonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                                partKFromD));
      m_kaonProtonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                                  partKFromD));

      m_kaonBinaryKaonProtonID_ALL = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                        partKFromD));
      m_kaonBinaryKaonPionID_ALL = std::get<double>
                                   (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                      partKFromD));
      m_kaonBinaryKaonElectronID_ALL = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                          partKFromD));
      m_kaonBinaryKaonProtonID_noSVD = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonBinaryKaonPionID_noSVD = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonBinaryKaonElectronID_noSVD = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonBinaryKaonProtonID_SVDonly = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, SVD)")->function(partKFromD));
      m_kaonBinaryKaonPionID_SVDonly = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD)")->function(partKFromD));
      m_kaonBinaryKaonElectronID_SVDonly = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, SVD)")->function(partKFromD));

      m_kaonBinaryProtonKaonID_ALL = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                        partKFromD));
      m_kaonBinaryPionKaonID_ALL = std::get<double>
                                   (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                      partKFromD));
      m_kaonBinaryElectronKaonID_ALL = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                          partKFromD));
      m_kaonBinaryProtonKaonID_noSVD = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonBinaryPionKaonID_noSVD = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonBinaryElectronKaonID_noSVD = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_kaonBinaryProtonKaonID_SVDonly = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, SVD)")->function(partKFromD));
      m_kaonBinaryPionKaonID_SVDonly = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD)")->function(partKFromD));
      m_kaonBinaryElectronKaonID_SVDonly = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, SVD)")->function(partKFromD));


      m_pionDElectronID_ALL = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDPionID_ALL = std::get<double>
                          (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDKaonID_ALL = std::get<double>
                          (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDProtonID_ALL = std::get<double>
                            (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDElectronID_noSVD = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDPionID_noSVD = std::get<double>
                            (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDKaonID_noSVD = std::get<double>
                            (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDProtonID_noSVD = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDElectronID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                                     partPiFromD));
      m_pionDPionID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                                 partPiFromD));
      m_pionDKaonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                                 partPiFromD));
      m_pionDProtonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                                   partPiFromD));

      m_pionDBinaryPionProtonID_ALL = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                         partPiFromD));
      m_pionDBinaryPionKaonID_ALL = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partPiFromD));
      m_pionDBinaryPionElectronID_ALL = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                           partPiFromD));
      m_pionDBinaryPionProtonID_noSVD = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                           partPiFromD));
      m_pionDBinaryPionKaonID_noSVD = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDBinaryPionElectronID_noSVD = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDBinaryPionProtonID_SVDonly = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD)")->function(partPiFromD));
      m_pionDBinaryPionKaonID_SVDonly = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD)")->function(partPiFromD));
      m_pionDBinaryPionElectronID_SVDonly = std::get<double>
                                            (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD)")->function(partPiFromD));

      m_pionDBinaryProtonPionID_ALL = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                         partPiFromD));
      m_pionDBinaryKaonPionID_ALL = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partPiFromD));
      m_pionDBinaryElectronPionID_ALL = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                           partPiFromD));
      m_pionDBinaryProtonPionID_noSVD = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, CDC, TOP, ARICH, ECL, KLM)")->function(
                                           partPiFromD));
      m_pionDBinaryKaonPionID_noSVD = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDBinaryElectronPionID_noSVD = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_pionDBinaryProtonPionID_SVDonly = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD)")->function(partPiFromD));
      m_pionDBinaryKaonPionID_SVDonly = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD)")->function(partPiFromD));
      m_pionDBinaryElectronPionID_SVDonly = std::get<double>
                                            (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD)")->function(partPiFromD));


      m_softPionElectronID_ALL = std::get<double>
                                 (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_softPionPionID_ALL = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_softPionKaonID_ALL = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_softPionProtonID_ALL = std::get<double>
                               (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_softPionElectronID_noSVD = std::get<double>
                                   (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_softPionPionID_noSVD = std::get<double>
                               (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_softPionKaonID_noSVD = std::get<double>
                               (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_softPionProtonID_noSVD = std::get<double>
                                 (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_softPionElectronID_SVDonly = std::get<double>
                                     (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                        partPiS));
      m_softPionPionID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                                    partPiS));
      m_softPionKaonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                                    partPiS));
      m_softPionProtonID_SVDonly = std::get<double>
                                   (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                      partPiS));

      m_softPionBinaryPionProtonID_ALL = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                            partPiS));
      m_softPionBinaryPionKaonID_ALL = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                          partPiS));
      m_softPionBinaryPionElectronID_ALL = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                              partPiS));
      m_softPionBinaryPionProtonID_noSVD = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                              partPiS));
      m_softPionBinaryPionKaonID_noSVD = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_softPionBinaryPionElectronID_noSVD = std::get<double>
                                             (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_softPionBinaryPionProtonID_SVDonly = std::get<double>
                                             (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD)")->function(partPiS));
      m_softPionBinaryPionKaonID_SVDonly = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD)")->function(partPiS));
      m_softPionBinaryPionElectronID_SVDonly = std::get<double>
                                               (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD)")->function(partPiS));

      m_softPionBinaryProtonPionID_ALL = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                            partPiS));
      m_softPionBinaryKaonPionID_ALL = std::get<double>
                                       (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                          partPiS));
      m_softPionBinaryElectronPionID_ALL = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                              partPiS));
      m_softPionBinaryProtonPionID_noSVD = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, CDC, TOP, ARICH, ECL, KLM)")->function(
                                              partPiS));
      m_softPionBinaryKaonPionID_noSVD = std::get<double>
                                         (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_softPionBinaryElectronPionID_noSVD = std::get<double>
                                             (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiS));
      m_softPionBinaryProtonPionID_SVDonly = std::get<double>
                                             (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD)")->function(partPiS));
      m_softPionBinaryKaonPionID_SVDonly = std::get<double>
                                           (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD)")->function(partPiS));
      m_softPionBinaryElectronPionID_SVDonly = std::get<double>
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

      m_firstElectronElectronID_ALL = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronPionID_ALL = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronKaonID_ALL = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronProtonID_ALL = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partE1FromGamma));
      m_firstElectronElectronID_noSVD = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronPionID_noSVD = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronKaonID_noSVD = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronProtonID_noSVD = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_firstElectronElectronID_SVDonly = std::get<double>
                                          (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                             partE1FromGamma));
      m_firstElectronPionID_SVDonly = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                         partE1FromGamma));
      m_firstElectronKaonID_SVDonly = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                         partE1FromGamma));
      m_firstElectronProtonID_SVDonly = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                           partE1FromGamma));

      m_firstElectronBinaryElectronProtonID_ALL = std::get<double>
                                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                     partE1FromGamma));
      m_firstElectronBinaryElectronKaonID_ALL = std::get<double>
                                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                   partE1FromGamma));
      m_firstElectronBinaryElectronPionID_ALL = std::get<double>
                                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                   partE1FromGamma));
      m_firstElectronBinaryElectronProtonID_noSVD = std::get<double>
                                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                       partE1FromGamma));
      m_firstElectronBinaryElectronKaonID_noSVD = std::get<double>
                                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                     partE1FromGamma));
      m_firstElectronBinaryElectronPionID_noSVD = std::get<double>
                                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                     partE1FromGamma));
      m_firstElectronBinaryElectronProtonID_SVDonly = std::get<double>
                                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, SVD)")->function(partE1FromGamma));
      m_firstElectronBinaryElectronKaonID_SVDonly = std::get<double>
                                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, SVD)")->function(partE1FromGamma));
      m_firstElectronBinaryElectronPionID_SVDonly = std::get<double>
                                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD)")->function(partE1FromGamma));

      m_firstElectronBinaryProtonElectronID_ALL = std::get<double>
                                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                     partE1FromGamma));
      m_firstElectronBinaryKaonElectronID_ALL = std::get<double>
                                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                   partE1FromGamma));
      m_firstElectronBinaryPionElectronID_ALL = std::get<double>
                                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                   partE1FromGamma));
      m_firstElectronBinaryProtonElectronID_noSVD = std::get<double>
                                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                       partE1FromGamma));
      m_firstElectronBinaryKaonElectronID_noSVD = std::get<double>
                                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                     partE1FromGamma));
      m_firstElectronBinaryPionElectronID_noSVD = std::get<double>
                                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, CDC, TOP, ARICH, ECL, KLM)")->function(
                                                     partE1FromGamma));
      m_firstElectronBinaryProtonElectronID_SVDonly = std::get<double>
                                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, SVD)")->function(partE1FromGamma));
      m_firstElectronBinaryKaonElectronID_SVDonly = std::get<double>
                                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, SVD)")->function(partE1FromGamma));
      m_firstElectronBinaryPionElectronID_SVDonly = std::get<double>
                                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD)")->function(partE1FromGamma));

      getObjectPtr<TTree>("Gamma")->Fill();
    }
  }
}
