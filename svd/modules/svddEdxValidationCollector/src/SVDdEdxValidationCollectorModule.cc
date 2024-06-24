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
  LambdaTree->Branch<double>("p_p", &m_p_p);
  LambdaTree->Branch<double>("p_SVDdEdx", &m_p_SVDdEdx);
  // LambdaTree->Branch<double>("pi_p", &m_piD_p);
  // LambdaTree->Branch<double>("pi_SVDdEdx", &m_piD_SVDdEdx);

  DstarTree->Branch<double>("InvM", &m_InvMDstar);
  DstarTree->Branch<double>("D0_InvM", &m_InvMD0);
  DstarTree->Branch<double>("deltaM", &m_DeltaM);
  DstarTree->Branch<double>("K_p", &m_K_p);
  DstarTree->Branch<double>("K_SVDdEdx", &m_K_SVDdEdx);
  DstarTree->Branch<double>("pi_p", &m_piD_p);
  DstarTree->Branch<double>("pi_SVDdEdx", &m_piD_SVDdEdx);
  DstarTree->Branch<double>("piS_p", &m_piS_p);
  DstarTree->Branch<double>("piS_SVDdEdx", &m_piS_SVDdEdx);

  GammaTree->Branch<double>("InvM", &m_InvMGamma);
  GammaTree->Branch<double>("e_1_p", &m_e_1_p);
  GammaTree->Branch<double>("e_1_SVDdEdx", &m_e_1_SVDdEdx);
  GammaTree->Branch<double>("e_2_p", &m_e_2_p);
  GammaTree->Branch<double>("e_2_SVDdEdx", &m_e_2_SVDdEdx);

  // Add a plethora of potentially useful PID variables
  LambdaTree->Branch<double>("p_electronID_ALL", &m_p_electronID_ALL);
  LambdaTree->Branch<double>("p_pionID_ALL", &m_p_pionID_ALL);
  LambdaTree->Branch<double>("p_kaonID_ALL", &m_p_kaonID_ALL);
  LambdaTree->Branch<double>("p_protonID_ALL", &m_p_protonID_ALL);
  LambdaTree->Branch<double>("p_electronID_noSVD", &m_p_electronID_noSVD);
  LambdaTree->Branch<double>("p_pionID_noSVD", &m_p_pionID_noSVD);
  LambdaTree->Branch<double>("p_kaonID_noSVD", &m_p_kaonID_noSVD);
  LambdaTree->Branch<double>("p_protonID_noSVD", &m_p_protonID_noSVD);
  LambdaTree->Branch<double>("p_electronID_SVDonly", &m_p_electronID_SVDonly);
  LambdaTree->Branch<double>("p_pionID_SVDonly", &m_p_pionID_SVDonly);
  LambdaTree->Branch<double>("p_kaonID_SVDonly", &m_p_kaonID_SVDonly);
  LambdaTree->Branch<double>("p_protonID_SVDonly", &m_p_protonID_SVDonly);

  LambdaTree->Branch<double>("p_proton_kaonID_ALL", &m_p_proton_kaonID_ALL);
  LambdaTree->Branch<double>("p_proton_pionID_ALL", &m_p_proton_pionID_ALL);
  LambdaTree->Branch<double>("p_proton_electronID_ALL", &m_p_proton_electronID_ALL);
  LambdaTree->Branch<double>("p_proton_kaonID_noSVD", &m_p_proton_kaonID_noSVD);
  LambdaTree->Branch<double>("p_proton_pionID_noSVD", &m_p_proton_pionID_noSVD);
  LambdaTree->Branch<double>("p_proton_electronID_noSVD", &m_p_proton_electronID_noSVD);
  LambdaTree->Branch<double>("p_proton_kaonID_SVDonly", &m_p_proton_kaonID_SVDonly);
  LambdaTree->Branch<double>("p_proton_pionID_SVDonly", &m_p_proton_pionID_SVDonly);
  LambdaTree->Branch<double>("p_proton_electronID_SVDonly", &m_p_proton_electronID_SVDonly);

  LambdaTree->Branch<double>("p_kaon_protonID_ALL", &m_p_kaon_protonID_ALL);
  LambdaTree->Branch<double>("p_pion_protonID_ALL", &m_p_pion_protonID_ALL);
  LambdaTree->Branch<double>("p_electron_protonID_ALL", &m_p_electron_protonID_ALL);
  LambdaTree->Branch<double>("p_kaon_protonID_noSVD", &m_p_kaon_protonID_noSVD);
  LambdaTree->Branch<double>("p_pion_protonID_noSVD", &m_p_pion_protonID_noSVD);
  LambdaTree->Branch<double>("p_electron_protonID_noSVD", &m_p_electron_protonID_noSVD);
  LambdaTree->Branch<double>("p_kaon_protonID_SVDonly", &m_p_kaon_protonID_SVDonly);
  LambdaTree->Branch<double>("p_pion_protonID_SVDonly", &m_p_pion_protonID_SVDonly);
  LambdaTree->Branch<double>("p_electron_protonID_SVDonly", &m_p_electron_protonID_SVDonly);

  DstarTree->Branch<double>("K_electronID_ALL", &m_K_electronID_ALL);
  DstarTree->Branch<double>("K_pionID_ALL", &m_K_pionID_ALL);
  DstarTree->Branch<double>("K_kaonID_ALL", &m_K_kaonID_ALL);
  DstarTree->Branch<double>("K_protonID_ALL", &m_K_protonID_ALL);
  DstarTree->Branch<double>("K_electronID_noSVD", &m_K_electronID_noSVD);
  DstarTree->Branch<double>("K_pionID_noSVD", &m_K_pionID_noSVD);
  DstarTree->Branch<double>("K_kaonID_noSVD", &m_K_kaonID_noSVD);
  DstarTree->Branch<double>("K_protonID_noSVD", &m_K_protonID_noSVD);
  DstarTree->Branch<double>("K_electronID_SVDonly", &m_K_electronID_SVDonly);
  DstarTree->Branch<double>("K_pionID_SVDonly", &m_K_pionID_SVDonly);
  DstarTree->Branch<double>("K_kaonID_SVDonly", &m_K_kaonID_SVDonly);
  DstarTree->Branch<double>("K_protonID_SVDonly", &m_K_protonID_SVDonly);

  DstarTree->Branch<double>("K_kaon_protonID_ALL", &m_K_kaon_protonID_ALL);
  DstarTree->Branch<double>("K_kaon_pionID_ALL", &m_K_kaon_pionID_ALL);
  DstarTree->Branch<double>("K_kaon_electronID_ALL", &m_K_kaon_electronID_ALL);
  DstarTree->Branch<double>("K_kaon_protonID_noSVD", &m_K_kaon_protonID_noSVD);
  DstarTree->Branch<double>("K_kaon_pionID_noSVD", &m_K_kaon_pionID_noSVD);
  DstarTree->Branch<double>("K_kaon_electronID_noSVD", &m_K_kaon_electronID_noSVD);
  DstarTree->Branch<double>("K_kaon_protonID_SVDonly", &m_K_kaon_protonID_SVDonly);
  DstarTree->Branch<double>("K_kaon_pionID_SVDonly", &m_K_kaon_pionID_SVDonly);
  DstarTree->Branch<double>("K_kaon_electronID_SVDonly", &m_K_kaon_electronID_SVDonly);

  DstarTree->Branch<double>("K_proton_kaonID_ALL", &m_K_proton_kaonID_ALL);
  DstarTree->Branch<double>("K_pion_kaonID_ALL", &m_K_pion_kaonID_ALL);
  DstarTree->Branch<double>("K_electron_kaonID_ALL", &m_K_electron_kaonID_ALL);
  DstarTree->Branch<double>("K_proton_kaonID_noSVD", &m_K_proton_kaonID_noSVD);
  DstarTree->Branch<double>("K_pion_kaonID_noSVD", &m_K_pion_kaonID_noSVD);
  DstarTree->Branch<double>("K_electron_kaonID_noSVD", &m_K_electron_kaonID_noSVD);
  DstarTree->Branch<double>("K_proton_kaonID_SVDonly", &m_K_proton_kaonID_SVDonly);
  DstarTree->Branch<double>("K_pion_kaonID_SVDonly", &m_K_pion_kaonID_SVDonly);
  DstarTree->Branch<double>("K_electron_kaonID_SVDonly", &m_K_electron_kaonID_SVDonly);

  DstarTree->Branch<double>("pi_electronID_ALL", &m_piD_electronID_ALL);
  DstarTree->Branch<double>("pi_pionID_ALL", &m_piD_pionID_ALL);
  DstarTree->Branch<double>("pi_kaonID_ALL", &m_piD_kaonID_ALL);
  DstarTree->Branch<double>("pi_protonID_ALL", &m_piD_protonID_ALL);
  DstarTree->Branch<double>("pi_electronID_noSVD", &m_piD_electronID_noSVD);
  DstarTree->Branch<double>("pi_pionID_noSVD", &m_piD_pionID_noSVD);
  DstarTree->Branch<double>("pi_kaonID_noSVD", &m_piD_kaonID_noSVD);
  DstarTree->Branch<double>("pi_protonID_noSVD", &m_piD_protonID_noSVD);
  DstarTree->Branch<double>("pi_electronID_SVDonly", &m_piD_electronID_SVDonly);
  DstarTree->Branch<double>("pi_pionID_SVDonly", &m_piD_pionID_SVDonly);
  DstarTree->Branch<double>("pi_kaonID_SVDonly", &m_piD_kaonID_SVDonly);
  DstarTree->Branch<double>("pi_protonID_SVDonly", &m_piD_protonID_SVDonly);

  DstarTree->Branch<double>("pi_pion_protonID_ALL", &m_piD_pion_protonID_ALL);
  DstarTree->Branch<double>("pi_pion_kaonID_ALL", &m_piD_pion_kaonID_ALL);
  DstarTree->Branch<double>("pi_pion_electronID_ALL", &m_piD_pion_electronID_ALL);
  DstarTree->Branch<double>("pi_pion_protonID_noSVD", &m_piD_pion_protonID_noSVD);
  DstarTree->Branch<double>("pi_pion_kaonID_noSVD", &m_piD_pion_kaonID_noSVD);
  DstarTree->Branch<double>("pi_pion_electronID_noSVD", &m_piD_pion_electronID_noSVD);
  DstarTree->Branch<double>("pi_pion_protonID_SVDonly", &m_piD_pion_protonID_SVDonly);
  DstarTree->Branch<double>("pi_pion_kaonID_SVDonly", &m_piD_pion_kaonID_SVDonly);
  DstarTree->Branch<double>("pi_pion_electronID_SVDonly", &m_piD_pion_electronID_SVDonly);

  DstarTree->Branch<double>("pi_proton_pionID_ALL", &m_piD_proton_pionID_ALL);
  DstarTree->Branch<double>("pi_kaon_pionID_ALL", &m_piD_kaon_pionID_ALL);
  DstarTree->Branch<double>("pi_electron_pionID_ALL", &m_piD_electron_pionID_ALL);
  DstarTree->Branch<double>("pi_proton_pionID_noSVD", &m_piD_proton_pionID_noSVD);
  DstarTree->Branch<double>("pi_kaon_pionID_noSVD", &m_piD_kaon_pionID_noSVD);
  DstarTree->Branch<double>("pi_electron_pionID_noSVD", &m_piD_electron_pionID_noSVD);
  DstarTree->Branch<double>("pi_proton_pionID_SVDonly", &m_piD_proton_pionID_SVDonly);
  DstarTree->Branch<double>("pi_kaon_pionID_SVDonly", &m_piD_kaon_pionID_SVDonly);
  DstarTree->Branch<double>("pi_electron_pionID_SVDonly", &m_piD_electron_pionID_SVDonly);

  GammaTree->Branch<double>("e_1_electronID_ALL", &m_e_1_electronID_ALL);
  GammaTree->Branch<double>("e_1_pionID_ALL", &m_e_1_pionID_ALL);
  GammaTree->Branch<double>("e_1_kaonID_ALL", &m_e_1_kaonID_ALL);
  GammaTree->Branch<double>("e_1_protonID_ALL", &m_e_1_protonID_ALL);
  GammaTree->Branch<double>("e_1_electronID_noSVD", &m_e_1_electronID_noSVD);
  GammaTree->Branch<double>("e_1_pionID_noSVD", &m_e_1_pionID_noSVD);
  GammaTree->Branch<double>("e_1_kaonID_noSVD", &m_e_1_kaonID_noSVD);
  GammaTree->Branch<double>("e_1_protonID_noSVD", &m_e_1_protonID_noSVD);
  GammaTree->Branch<double>("e_1_electronID_SVDonly", &m_e_1_electronID_SVDonly);
  GammaTree->Branch<double>("e_1_pionID_SVDonly", &m_e_1_pionID_SVDonly);
  GammaTree->Branch<double>("e_1_kaonID_SVDonly", &m_e_1_kaonID_SVDonly);
  GammaTree->Branch<double>("e_1_protonID_SVDonly", &m_e_1_protonID_SVDonly);

  GammaTree->Branch<double>("e_1_electron_protonID_ALL", &m_e_1_electron_protonID_ALL);
  GammaTree->Branch<double>("e_1_electron_kaonID_ALL", &m_e_1_electron_kaonID_ALL);
  GammaTree->Branch<double>("e_1_electron_pionID_ALL", &m_e_1_electron_pionID_ALL);
  GammaTree->Branch<double>("e_1_electron_protonID_noSVD", &m_e_1_electron_protonID_noSVD);
  GammaTree->Branch<double>("e_1_electron_kaonID_noSVD", &m_e_1_electron_kaonID_noSVD);
  GammaTree->Branch<double>("e_1_electron_pionID_noSVD", &m_e_1_electron_pionID_noSVD);
  GammaTree->Branch<double>("e_1_electron_protonID_SVDonly", &m_e_1_electron_protonID_SVDonly);
  GammaTree->Branch<double>("e_1_electron_kaonID_SVDonly", &m_e_1_electron_kaonID_SVDonly);
  GammaTree->Branch<double>("e_1_electron_pionID_SVDonly", &m_e_1_electron_pionID_SVDonly);

  GammaTree->Branch<double>("e_1_proton_electronID_ALL", &m_e_1_proton_electronID_ALL);
  GammaTree->Branch<double>("e_1_kaon_electronID_ALL", &m_e_1_kaon_electronID_ALL);
  GammaTree->Branch<double>("e_1_pion_electronID_ALL", &m_e_1_pion_electronID_ALL);
  GammaTree->Branch<double>("e_1_proton_electronID_noSVD", &m_e_1_proton_electronID_noSVD);
  GammaTree->Branch<double>("e_1_kaon_electronID_noSVD", &m_e_1_kaon_electronID_noSVD);
  GammaTree->Branch<double>("e_1_pion_electronID_noSVD", &m_e_1_pion_electronID_noSVD);
  GammaTree->Branch<double>("e_1_proton_electronID_SVDonly", &m_e_1_proton_electronID_SVDonly);
  GammaTree->Branch<double>("e_1_kaon_electronID_SVDonly", &m_e_1_kaon_electronID_SVDonly);
  GammaTree->Branch<double>("e_1_pion_electronID_SVDonly", &m_e_1_pion_electronID_SVDonly);

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
      m_p_p = partPFromLambda->getMomentumMagnitude();

      if (!dedxTrackPFromLambda) {
        m_p_SVDdEdx = -999.0;
      } else {
        m_p_SVDdEdx = dedxTrackPFromLambda->getDedx(Const::EDetector::SVD);
      }



      m_p_electronID_ALL = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_p_pionID_ALL = std::get<double>
                       (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_p_kaonID_ALL = std::get<double>
                       (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_p_protonID_ALL = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                            partPFromLambda));
      m_p_electronID_noSVD = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_p_pionID_noSVD = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_p_kaonID_noSVD = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_p_protonID_noSVD = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partPFromLambda));
      m_p_electronID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                                  partPFromLambda));
      m_p_pionID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                              partPFromLambda));
      m_p_kaonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                              partPFromLambda));
      m_p_protonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                                partPFromLambda));

      m_p_proton_kaonID_ALL = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                 partPFromLambda));
      m_p_proton_pionID_ALL = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                 partPFromLambda));
      m_p_proton_electronID_ALL = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                     partPFromLambda));
      m_p_proton_kaonID_noSVD = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, CDC, TOP, ARICH, ECL, KLM)")->function(
                                   partPFromLambda));
      m_p_proton_pionID_noSVD = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, CDC, TOP, ARICH, ECL, KLM)")->function(
                                   partPFromLambda));
      m_p_proton_electronID_noSVD = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partPFromLambda));
      m_p_proton_kaonID_SVDonly = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, SVD)")->function(partPFromLambda));
      m_p_proton_pionID_SVDonly = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD)")->function(partPFromLambda));
      m_p_proton_electronID_SVDonly = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, SVD)")->function(partPFromLambda));

      m_p_kaon_protonID_ALL = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                 partPFromLambda));
      m_p_pion_protonID_ALL = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                 partPFromLambda));
      m_p_electron_protonID_ALL = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                     partPFromLambda));
      m_p_kaon_protonID_noSVD = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                   partPFromLambda));
      m_p_pion_protonID_noSVD = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                   partPFromLambda));
      m_p_electron_protonID_noSVD = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partPFromLambda));
      m_p_kaon_protonID_SVDonly = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, SVD)")->function(partPFromLambda));
      m_p_pion_protonID_SVDonly = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD)")->function(partPFromLambda));
      m_p_electron_protonID_SVDonly = std::get<double>
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

      m_K_p = partKFromD->getMomentumMagnitude();
      if (!dedxTrackKFromD) {
        m_K_SVDdEdx = -999.0;
      } else {
        m_K_SVDdEdx = dedxTrackKFromD->getDedx(Const::EDetector::SVD);
      }

      m_piD_p = partPiFromD->getMomentumMagnitude();
      if (!dedxTrackPiFromD) {
        m_piD_SVDdEdx = -999.0;
      } else {
        m_piD_SVDdEdx = dedxTrackPiFromD->getDedx(Const::EDetector::SVD);
      }

      m_piS_p = partPiS->getMomentumMagnitude();
      if (!dedxTrackPiS) {
        m_piS_SVDdEdx = -999.0;
      } else {
        m_piS_SVDdEdx = dedxTrackPiS->getDedx(Const::EDetector::SVD);
      }

      m_K_electronID_ALL = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_pionID_ALL = std::get<double>
                       (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_kaonID_ALL = std::get<double>
                       (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_protonID_ALL = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_electronID_noSVD = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_pionID_noSVD = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_kaonID_noSVD = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_protonID_noSVD = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_electronID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                                  partKFromD));
      m_K_pionID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                              partKFromD));
      m_K_kaonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                              partKFromD));
      m_K_protonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                                partKFromD));

      m_K_kaon_protonID_ALL = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                 partKFromD));
      m_K_kaon_pionID_ALL = std::get<double>
                            (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                               partKFromD));
      m_K_kaon_electronID_ALL = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                   partKFromD));
      m_K_kaon_protonID_noSVD = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_kaon_pionID_noSVD = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_kaon_electronID_noSVD = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_kaon_protonID_SVDonly = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 2212, SVD)")->function(partKFromD));
      m_K_kaon_pionID_SVDonly = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD)")->function(partKFromD));
      m_K_kaon_electronID_SVDonly = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, SVD)")->function(partKFromD));

      m_K_proton_kaonID_ALL = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                 partKFromD));
      m_K_pion_kaonID_ALL = std::get<double>
                            (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                               partKFromD));
      m_K_electron_kaonID_ALL = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                   partKFromD));
      m_K_proton_kaonID_noSVD = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_pion_kaonID_noSVD = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_electron_kaonID_noSVD = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partKFromD));
      m_K_proton_kaonID_SVDonly = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 321, SVD)")->function(partKFromD));
      m_K_pion_kaonID_SVDonly = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD)")->function(partKFromD));
      m_K_electron_kaonID_SVDonly = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, SVD)")->function(partKFromD));


      m_piD_electronID_ALL = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_piD_pionID_ALL = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_piD_kaonID_ALL = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_piD_protonID_ALL = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_piD_electronID_noSVD = std::get<double>
                               (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_piD_pionID_noSVD = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_piD_kaonID_noSVD = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_piD_protonID_noSVD = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_piD_electronID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                                    partPiFromD));
      m_piD_pionID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                                partPiFromD));
      m_piD_kaonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                                partPiFromD));
      m_piD_protonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                                  partPiFromD));

      m_piD_pion_protonID_ALL = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                   partPiFromD));
      m_piD_pion_kaonID_ALL = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                 partPiFromD));
      m_piD_pion_electronID_ALL = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                     partPiFromD));
      m_piD_pion_protonID_noSVD = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                     partPiFromD));
      m_piD_pion_kaonID_noSVD = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_piD_pion_electronID_noSVD = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_piD_pion_protonID_SVDonly = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 2212, SVD)")->function(partPiFromD));
      m_piD_pion_kaonID_SVDonly = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 321, SVD)")->function(partPiFromD));
      m_piD_pion_electronID_SVDonly = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD)")->function(partPiFromD));

      m_piD_proton_pionID_ALL = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                   partPiFromD));
      m_piD_kaon_pionID_ALL = std::get<double>
                              (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                 partPiFromD));
      m_piD_electron_pionID_ALL = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                     partPiFromD));
      m_piD_proton_pionID_noSVD = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, CDC, TOP, ARICH, ECL, KLM)")->function(
                                     partPiFromD));
      m_piD_kaon_pionID_noSVD = std::get<double>
                                (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_piD_electron_pionID_noSVD = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, CDC, TOP, ARICH, ECL, KLM)")->function(partPiFromD));
      m_piD_proton_pionID_SVDonly = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 211, SVD)")->function(partPiFromD));
      m_piD_kaon_pionID_SVDonly = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 211, SVD)")->function(partPiFromD));
      m_piD_electron_pionID_SVDonly = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD)")->function(partPiFromD));

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

      m_e_1_p = partE1FromGamma->getMomentumMagnitude();
      if (!dedxTrackE1FromGamma) {
        m_e_1_SVDdEdx = -999.0;
      } else {
        m_e_1_SVDdEdx = dedxTrackE1FromGamma->getDedx(Const::EDetector::SVD);
      }

      m_e_2_p = partE2FromGamma->getMomentumMagnitude();
      if (!dedxTrackE2FromGamma) {
        m_e_2_SVDdEdx = -999.0;
      } else {
        m_e_2_SVDdEdx = dedxTrackE2FromGamma->getDedx(Const::EDetector::SVD);
      }

      m_e_1_electronID_ALL = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_e_1_pionID_ALL = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_e_1_kaonID_ALL = std::get<double>
                         (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_e_1_protonID_ALL = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                              partE1FromGamma));
      m_e_1_electronID_noSVD = std::get<double>
                               (Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_e_1_pionID_noSVD = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_e_1_kaonID_noSVD = std::get<double>
                           (Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_e_1_protonID_noSVD = std::get<double>
                             (Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(partE1FromGamma));
      m_e_1_electronID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(11, SVD)")->function(
                                                    partE1FromGamma));
      m_e_1_pionID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(211, SVD)")->function(
                                                partE1FromGamma));
      m_e_1_kaonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(321, SVD)")->function(
                                                partE1FromGamma));
      m_e_1_protonID_SVDonly = std::get<double>(Variable::Manager::Instance().getVariable("pidProbabilityExpert(2212, SVD)")->function(
                                                  partE1FromGamma));

      m_e_1_electron_protonID_ALL = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partE1FromGamma));
      m_e_1_electron_kaonID_ALL = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                     partE1FromGamma));
      m_e_1_electron_pionID_ALL = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                     partE1FromGamma));
      m_e_1_electron_protonID_noSVD = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, CDC, TOP, ARICH, ECL, KLM)")->function(
                                         partE1FromGamma));
      m_e_1_electron_kaonID_noSVD = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partE1FromGamma));
      m_e_1_electron_pionID_noSVD = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partE1FromGamma));
      m_e_1_electron_protonID_SVDonly = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 2212, SVD)")->function(partE1FromGamma));
      m_e_1_electron_kaonID_SVDonly = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 321, SVD)")->function(partE1FromGamma));
      m_e_1_electron_pionID_SVDonly = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(11, 211, SVD)")->function(partE1FromGamma));

      m_e_1_proton_electronID_ALL = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partE1FromGamma));
      m_e_1_kaon_electronID_ALL = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                     partE1FromGamma));
      m_e_1_pion_electronID_ALL = std::get<double>
                                  (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD, CDC, TOP, ARICH, ECL, KLM)")->function(
                                     partE1FromGamma));
      m_e_1_proton_electronID_noSVD = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, CDC, TOP, ARICH, ECL, KLM)")->function(
                                         partE1FromGamma));
      m_e_1_kaon_electronID_noSVD = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partE1FromGamma));
      m_e_1_pion_electronID_noSVD = std::get<double>
                                    (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, CDC, TOP, ARICH, ECL, KLM)")->function(
                                       partE1FromGamma));
      m_e_1_proton_electronID_SVDonly = std::get<double>
                                        (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(2212, 11, SVD)")->function(partE1FromGamma));
      m_e_1_kaon_electronID_SVDonly = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(321, 11, SVD)")->function(partE1FromGamma));
      m_e_1_pion_electronID_SVDonly = std::get<double>
                                      (Variable::Manager::Instance().getVariable("pidPairProbabilityExpert(211, 11, SVD)")->function(partE1FromGamma));

      getObjectPtr<TTree>("Gamma")->Fill();
    }
  }
}
