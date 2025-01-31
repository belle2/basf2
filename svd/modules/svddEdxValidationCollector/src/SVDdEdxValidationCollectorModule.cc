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

  LambdaTree->Branch<double>("ProtonElectronLLSVDonly", &m_protonElectronLLSVDonly);
  LambdaTree->Branch<double>("ProtonPionLLSVDonly", &m_protonPionLLSVDonly);
  LambdaTree->Branch<double>("ProtonKaonLLSVDonly", &m_protonKaonLLSVDonly);
  LambdaTree->Branch<double>("ProtonProtonLLSVDonly", &m_protonProtonLLSVDonly);

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

  DstarTree->Branch<double>("KaonElectronLLSVDonly", &m_kaonElectronLLSVDonly);
  DstarTree->Branch<double>("KaonPionLLSVDonly", &m_kaonPionLLSVDonly);
  DstarTree->Branch<double>("KaonKaonLLSVDonly", &m_kaonKaonLLSVDonly);
  DstarTree->Branch<double>("KaonProtonLLSVDonly", &m_kaonProtonLLSVDonly);

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

  DstarTree->Branch<double>("PionDElectronLLSVDonly", &m_pionDElectronLLSVDonly);
  DstarTree->Branch<double>("PionDPionLLSVDonly", &m_pionDPionLLSVDonly);
  DstarTree->Branch<double>("PionDKaonLLSVDonly", &m_pionDKaonLLSVDonly);
  DstarTree->Branch<double>("PionDProtonLLSVDonly", &m_pionDProtonLLSVDonly);

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

  DstarTree->Branch<double>("SlowPionElectronLLSVDonly", &m_slowPionElectronLLSVDonly);
  DstarTree->Branch<double>("SlowPionPionLLSVDonly", &m_slowPionPionLLSVDonly);
  DstarTree->Branch<double>("SlowPionKaonLLSVDonly", &m_slowPionKaonLLSVDonly);
  DstarTree->Branch<double>("SlowPionProtonLLSVDonly", &m_slowPionProtonLLSVDonly);

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

  GammaTree->Branch<double>("FirstElectronElectronLLSVDonly", &m_firstElectronElectronLLSVDonly);
  GammaTree->Branch<double>("FirstElectronPionLLSVDonly", &m_firstElectronPionLLSVDonly);
  GammaTree->Branch<double>("FirstElectronKaonLLSVDonly", &m_firstElectronKaonLLSVDonly);
  GammaTree->Branch<double>("FirstElectronProtonLLSVDonly", &m_firstElectronProtonLLSVDonly);

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

  static Manager::FunctionPtr electronPIDSVDOnlyFunction = pidProbabilityExpert({"11", "SVD"});
  static Manager::FunctionPtr pionPIDSVDOnlyFunction = pidProbabilityExpert({"211", "SVD"});
  static Manager::FunctionPtr kaonPIDSVDOnlyFunction = pidProbabilityExpert({"321", "SVD"});
  static Manager::FunctionPtr protonPIDSVDOnlyFunction = pidProbabilityExpert({"2212", "SVD"});
  static Manager::FunctionPtr binaryKaonProtonPIDSVDOnlyFunction = pidPairProbabilityExpert({"321", "2212", "SVD"});
  static Manager::FunctionPtr binaryPionProtonPIDSVDOnlyFunction = pidPairProbabilityExpert({"211", "2212", "SVD"});
  static Manager::FunctionPtr binaryElectronProtonPIDSVDOnlyFunction = pidPairProbabilityExpert({"11", "2212", "SVD"});
  static Manager::FunctionPtr binaryProtonKaonPIDSVDOnlyFunction = pidPairProbabilityExpert({"2212", "321", "SVD"});
  static Manager::FunctionPtr binaryProtonPionPIDSVDOnlyFunction = pidPairProbabilityExpert({"2212", "211", "SVD"});
  static Manager::FunctionPtr binaryProtonElectronPIDSVDOnlyFunction = pidPairProbabilityExpert({"2212", "11", "SVD"});
  static Manager::FunctionPtr binaryKaonElectronPIDSVDOnlyFunction = pidPairProbabilityExpert({"321", "11", "SVD"});
  static Manager::FunctionPtr binaryElectronKaonPIDSVDOnlyFunction = pidPairProbabilityExpert({"11", "321", "SVD"});
  static Manager::FunctionPtr binaryPionElectronPIDSVDOnlyFunction = pidPairProbabilityExpert({"211", "11", "SVD"});
  static Manager::FunctionPtr binaryElectronPionPIDSVDOnlyFunction = pidPairProbabilityExpert({"11", "211", "SVD"});

  static Manager::FunctionPtr electronLLSVDOnlyFunction = pidLogLikelihoodValueExpert({"11", "SVD"});
  static Manager::FunctionPtr pionLLSVDOnlyFunction = pidLogLikelihoodValueExpert({"211", "SVD"});
  static Manager::FunctionPtr kaonLLSVDOnlyFunction = pidLogLikelihoodValueExpert({"321", "SVD"});
  static Manager::FunctionPtr protonLLSVDOnlyFunction = pidLogLikelihoodValueExpert({"2212", "SVD"});

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



      m_protonElectronIDALL = Variable::electronID(partPFromLambda);
      m_protonPionIDALL = Variable::pionID(partPFromLambda);
      m_protonKaonIDALL = Variable::kaonID(partPFromLambda);
      m_protonProtonIDALL = Variable::protonID(partPFromLambda);
      m_protonElectronIDnoSVD = Variable::electronID_noSVD(partPFromLambda);
      m_protonPionIDnoSVD = Variable::pionID_noSVD(partPFromLambda);
      m_protonKaonIDnoSVD = Variable::kaonID_noSVD(partPFromLambda);
      m_protonProtonIDnoSVD = Variable::protonID_noSVD(partPFromLambda);
      m_protonElectronIDSVDonly = std::get<double>(electronPIDSVDOnlyFunction(partPFromLambda));
      m_protonPionIDSVDonly = std::get<double>(pionPIDSVDOnlyFunction(partPFromLambda));
      m_protonKaonIDSVDonly = std::get<double>(kaonPIDSVDOnlyFunction(partPFromLambda));
      m_protonProtonIDSVDonly = std::get<double>(protonPIDSVDOnlyFunction(partPFromLambda));
      m_protonElectronLLSVDonly = std::get<double>(electronLLSVDOnlyFunction(partPFromLambda));
      m_protonPionLLSVDonly = std::get<double>(pionLLSVDOnlyFunction(partPFromLambda));
      m_protonKaonLLSVDonly = std::get<double>(kaonLLSVDOnlyFunction(partPFromLambda));
      m_protonProtonLLSVDonly = std::get<double>(protonLLSVDOnlyFunction(partPFromLambda));

      m_protonBinaryProtonKaonIDALL = Variable::binaryPID(partPFromLambda, {2212., 321.});
      m_protonBinaryProtonPionIDALL = Variable::binaryPID(partPFromLambda, {2212., 211.});
      m_protonBinaryProtonElectronIDALL = Variable::binaryPID(partPFromLambda, {2212., 11.});
      m_protonBinaryProtonKaonIDnoSVD = Variable::binaryPID_noSVD(partPFromLambda, {2212., 321.});
      m_protonBinaryProtonPionIDnoSVD = Variable::binaryPID_noSVD(partPFromLambda, {2212., 211.});
      m_protonBinaryProtonElectronIDnoSVD = Variable::binaryPID_noSVD(partPFromLambda, {2212., 11.});
      m_protonBinaryProtonKaonIDSVDonly = std::get<double>(binaryProtonKaonPIDSVDOnlyFunction(partPFromLambda));
      m_protonBinaryProtonPionIDSVDonly = std::get<double>(binaryProtonPionPIDSVDOnlyFunction(partPFromLambda));
      m_protonBinaryProtonElectronIDSVDonly = std::get<double>(binaryProtonElectronPIDSVDOnlyFunction(partPFromLambda));

      m_protonBinaryKaonProtonIDALL = Variable::binaryPID(partPFromLambda, {321., 2212.});
      m_protonBinaryPionProtonIDALL = Variable::binaryPID(partPFromLambda, {211., 2212.});
      m_protonBinaryElectronProtonIDALL = Variable::binaryPID(partPFromLambda, {11., 2212.});
      m_protonBinaryKaonProtonIDnoSVD = Variable::binaryPID_noSVD(partPFromLambda, {321., 2212.});
      m_protonBinaryPionProtonIDnoSVD = Variable::binaryPID_noSVD(partPFromLambda, {211., 2212.});
      m_protonBinaryElectronProtonIDnoSVD = Variable::binaryPID_noSVD(partPFromLambda, {11., 2212.});
      m_protonBinaryKaonProtonIDSVDonly = std::get<double>(binaryKaonProtonPIDSVDOnlyFunction(partPFromLambda));
      m_protonBinaryPionProtonIDSVDonly = std::get<double>(binaryPionProtonPIDSVDOnlyFunction(partPFromLambda));
      m_protonBinaryElectronProtonIDSVDonly = std::get<double>(binaryElectronProtonPIDSVDOnlyFunction(partPFromLambda));

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

      static Manager::FunctionPtr binaryKaonPionPIDSVDOnlyFunction = pidPairProbabilityExpert({"321", "211", "SVD"});
      static Manager::FunctionPtr binaryPionKaonPIDSVDOnlyFunction = pidPairProbabilityExpert({"211", "321", "SVD"});

      m_kaonElectronIDALL = Variable::electronID(partKFromD);
      m_kaonPionIDALL = Variable::pionID(partKFromD);
      m_kaonKaonIDALL = Variable::kaonID(partKFromD);
      m_kaonProtonIDALL = Variable::protonID(partKFromD);
      m_kaonElectronIDnoSVD = Variable::electronID(partKFromD);
      m_kaonPionIDnoSVD = Variable::pionID_noSVD(partKFromD);
      m_kaonKaonIDnoSVD = Variable::kaonID_noSVD(partKFromD);
      m_kaonProtonIDnoSVD = Variable::protonID_noSVD(partKFromD);
      m_kaonElectronIDSVDonly = std::get<double>(electronPIDSVDOnlyFunction(partKFromD));
      m_kaonPionIDSVDonly = std::get<double>(pionPIDSVDOnlyFunction(partKFromD));
      m_kaonKaonIDSVDonly = std::get<double>(kaonPIDSVDOnlyFunction(partKFromD));
      m_kaonProtonIDSVDonly = std::get<double>(protonPIDSVDOnlyFunction(partKFromD));
      m_kaonElectronLLSVDonly = std::get<double>(electronLLSVDOnlyFunction(partKFromD));
      m_kaonPionLLSVDonly = std::get<double>(pionLLSVDOnlyFunction(partKFromD));
      m_kaonKaonLLSVDonly = std::get<double>(kaonLLSVDOnlyFunction(partKFromD));
      m_kaonProtonLLSVDonly = std::get<double>(protonLLSVDOnlyFunction(partKFromD));


      m_kaonBinaryKaonProtonIDALL = Variable::binaryPID(partKFromD, {321., 2212.});
      m_kaonBinaryKaonPionIDALL = Variable::binaryPID(partKFromD, {321., 211.});
      m_kaonBinaryKaonElectronIDALL = Variable::binaryPID(partKFromD, {321., 11.});
      m_kaonBinaryKaonProtonIDnoSVD = Variable::binaryPID_noSVD(partKFromD, {321., 2212.});
      m_kaonBinaryKaonPionIDnoSVD = Variable::binaryPID_noSVD(partKFromD, {321., 211.});
      m_kaonBinaryKaonElectronIDnoSVD = Variable::binaryPID_noSVD(partKFromD, {321., 11.});
      m_kaonBinaryKaonProtonIDSVDonly = std::get<double>(binaryKaonProtonPIDSVDOnlyFunction(partKFromD));

      m_kaonBinaryKaonPionIDSVDonly = std::get<double>(binaryKaonPionPIDSVDOnlyFunction(partKFromD));
      m_kaonBinaryKaonElectronIDSVDonly = std::get<double>(binaryKaonElectronPIDSVDOnlyFunction(partKFromD));

      m_kaonBinaryProtonKaonIDALL = Variable::binaryPID(partKFromD, {2212., 321.});
      m_kaonBinaryPionKaonIDALL = Variable::binaryPID(partKFromD, {211., 321.});
      m_kaonBinaryElectronKaonIDALL = Variable::binaryPID(partKFromD, {11., 321.});
      m_kaonBinaryProtonKaonIDnoSVD = Variable::binaryPID_noSVD(partKFromD, {2212., 321.});
      m_kaonBinaryPionKaonIDnoSVD = Variable::binaryPID_noSVD(partKFromD, {211., 321.});
      m_kaonBinaryElectronKaonIDnoSVD = Variable::binaryPID_noSVD(partKFromD, {11., 321.});
      m_kaonBinaryProtonKaonIDSVDonly = std::get<double>(binaryProtonKaonPIDSVDOnlyFunction(partKFromD));
      m_kaonBinaryPionKaonIDSVDonly = std::get<double>(binaryPionKaonPIDSVDOnlyFunction(partKFromD));
      m_kaonBinaryElectronKaonIDSVDonly = std::get<double>(binaryElectronKaonPIDSVDOnlyFunction(partKFromD));


      m_pionDElectronIDALL = Variable::electronID(partPiFromD);
      m_pionDPionIDALL = Variable::pionID(partPiFromD);
      m_pionDKaonIDALL = Variable::kaonID(partPiFromD);
      m_pionDProtonIDALL = Variable::protonID(partPiFromD);
      m_pionDElectronIDnoSVD = Variable::electronID_noSVD(partPiFromD);
      m_pionDPionIDnoSVD = Variable::pionID_noSVD(partPiFromD);
      m_pionDKaonIDnoSVD = Variable::kaonID_noSVD(partPiFromD);
      m_pionDProtonIDnoSVD = Variable::protonID_noSVD(partPiFromD);
      m_pionDElectronIDSVDonly = std::get<double>(electronPIDSVDOnlyFunction(partPiFromD));
      m_pionDPionIDSVDonly = std::get<double>(pionPIDSVDOnlyFunction(partPiFromD));
      m_pionDKaonIDSVDonly = std::get<double>(kaonPIDSVDOnlyFunction(partPiFromD));
      m_pionDProtonIDSVDonly = std::get<double>(protonPIDSVDOnlyFunction(partPiFromD));
      m_pionDElectronLLSVDonly = std::get<double>(electronLLSVDOnlyFunction(partPiFromD));
      m_pionDPionLLSVDonly = std::get<double>(pionLLSVDOnlyFunction(partPiFromD));
      m_pionDKaonLLSVDonly = std::get<double>(kaonLLSVDOnlyFunction(partPiFromD));
      m_pionDProtonLLSVDonly = std::get<double>(protonLLSVDOnlyFunction(partPiFromD));

      m_pionDBinaryPionProtonIDALL = Variable::binaryPID(partPiFromD, {211., 2212.});
      m_pionDBinaryPionKaonIDALL = Variable::binaryPID(partPiFromD, {211., 321.});
      m_pionDBinaryPionElectronIDALL = Variable::binaryPID(partPiFromD, {211., 11.});
      m_pionDBinaryPionProtonIDnoSVD = Variable::binaryPID_noSVD(partPiFromD, {211., 2212});
      m_pionDBinaryPionKaonIDnoSVD = Variable::binaryPID_noSVD(partPiFromD, {211., 321.});
      m_pionDBinaryPionElectronIDnoSVD = Variable::binaryPID_noSVD(partPiFromD, {211., 11.});
      m_pionDBinaryPionProtonIDSVDonly = std::get<double>(binaryPionProtonPIDSVDOnlyFunction(partPiFromD));
      m_pionDBinaryPionKaonIDSVDonly = std::get<double>(binaryPionKaonPIDSVDOnlyFunction(partPiFromD));
      m_pionDBinaryPionElectronIDSVDonly = std::get<double>(binaryPionElectronPIDSVDOnlyFunction(partPiFromD));

      m_pionDBinaryProtonPionIDALL = Variable::binaryPID(partPiFromD, {2212., 211.});
      m_pionDBinaryKaonPionIDALL = Variable::binaryPID(partPiFromD, {321., 211.});
      m_pionDBinaryElectronPionIDALL = Variable::binaryPID(partPiFromD, {11., 211.});
      m_pionDBinaryProtonPionIDnoSVD = Variable::binaryPID_noSVD(partPiFromD, {2212., 211.});
      m_pionDBinaryKaonPionIDnoSVD = Variable::binaryPID_noSVD(partPiFromD, {321., 211.});
      m_pionDBinaryElectronPionIDnoSVD = Variable::binaryPID_noSVD(partPiFromD, {11., 211.});
      m_pionDBinaryProtonPionIDSVDonly = std::get<double>(binaryProtonPionPIDSVDOnlyFunction(partPiFromD));
      m_pionDBinaryKaonPionIDSVDonly = std::get<double>(binaryKaonPionPIDSVDOnlyFunction(partPiFromD));
      m_pionDBinaryElectronPionIDSVDonly = std::get<double>(binaryElectronPionPIDSVDOnlyFunction(partPiFromD));


      m_slowPionElectronIDALL = Variable::electronID(partPiS);
      m_slowPionPionIDALL = Variable::pionID(partPiS);
      m_slowPionKaonIDALL = Variable::kaonID(partPiS);
      m_slowPionProtonIDALL = Variable::protonID(partPiS);
      m_slowPionElectronIDnoSVD = Variable::electronID_noSVD(partPiS);
      m_slowPionPionIDnoSVD = Variable::pionID_noSVD(partPiS);
      m_slowPionKaonIDnoSVD = Variable::kaonID_noSVD(partPiS);
      m_slowPionProtonIDnoSVD = Variable::protonID_noSVD(partPiS);
      m_slowPionElectronIDSVDonly = std::get<double>(electronPIDSVDOnlyFunction(partPiS));
      m_slowPionPionIDSVDonly = std::get<double>(pionPIDSVDOnlyFunction(partPiS));
      m_slowPionKaonIDSVDonly = std::get<double>(kaonPIDSVDOnlyFunction(partPiS));
      m_slowPionProtonIDSVDonly = std::get<double>(protonPIDSVDOnlyFunction(partPiS));
      m_slowPionElectronLLSVDonly = std::get<double>(electronLLSVDOnlyFunction(partPiS));
      m_slowPionPionLLSVDonly = std::get<double>(pionLLSVDOnlyFunction(partPiS));
      m_slowPionKaonLLSVDonly = std::get<double>(kaonLLSVDOnlyFunction(partPiS));
      m_slowPionProtonLLSVDonly = std::get<double>(protonLLSVDOnlyFunction(partPiS));


      m_slowPionBinaryPionProtonIDALL = Variable::binaryPID(partPiS, {211., 2212.});
      m_slowPionBinaryPionKaonIDALL = Variable::binaryPID(partPiS, {211., 321.});
      m_slowPionBinaryPionElectronIDALL = Variable::binaryPID(partPiS, {211., 11.});
      m_slowPionBinaryPionProtonIDnoSVD = Variable::binaryPID_noSVD(partPiS, {211., 2212.});
      m_slowPionBinaryPionKaonIDnoSVD = Variable::binaryPID_noSVD(partPiS, {211., 321.});
      m_slowPionBinaryPionElectronIDnoSVD = Variable::binaryPID_noSVD(partPiS, {211., 11.});
      m_slowPionBinaryPionProtonIDSVDonly = std::get<double>(binaryPionProtonPIDSVDOnlyFunction(partPiS));
      m_slowPionBinaryPionKaonIDSVDonly = std::get<double>(binaryPionKaonPIDSVDOnlyFunction(partPiS));
      m_slowPionBinaryPionElectronIDSVDonly = std::get<double>(binaryPionElectronPIDSVDOnlyFunction(partPiS));

      m_slowPionBinaryProtonPionIDALL = Variable::binaryPID(partPiS, {2212., 211.});
      m_slowPionBinaryKaonPionIDALL = Variable::binaryPID(partPiS, {321., 211.});
      m_slowPionBinaryElectronPionIDALL = Variable::binaryPID(partPiS, {11., 211.});
      m_slowPionBinaryProtonPionIDnoSVD = Variable::binaryPID_noSVD(partPiS, {2212., 211.});
      m_slowPionBinaryKaonPionIDnoSVD = Variable::binaryPID_noSVD(partPiS, {321., 211.});
      m_slowPionBinaryElectronPionIDnoSVD = Variable::binaryPID_noSVD(partPiS, {11., 211.});
      m_slowPionBinaryProtonPionIDSVDonly = std::get<double>(binaryProtonPionPIDSVDOnlyFunction(partPiS));
      m_slowPionBinaryKaonPionIDSVDonly = std::get<double>(binaryKaonPionPIDSVDOnlyFunction(partPiS));
      m_slowPionBinaryElectronPionIDSVDonly = std::get<double>(binaryElectronPionPIDSVDOnlyFunction(partPiS));

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

      m_firstElectronElectronIDALL = Variable::electronID(partE1FromGamma);
      m_firstElectronPionIDALL = Variable::pionID(partE1FromGamma);
      m_firstElectronKaonIDALL = Variable::kaonID(partE1FromGamma);
      m_firstElectronProtonIDALL = Variable::protonID(partE1FromGamma);
      m_firstElectronElectronIDnoSVD = Variable::electronID_noSVD(partE1FromGamma);
      m_firstElectronPionIDnoSVD = Variable::pionID_noSVD(partE1FromGamma);
      m_firstElectronKaonIDnoSVD = Variable::kaonID_noSVD(partE1FromGamma);
      m_firstElectronProtonIDnoSVD = Variable::protonID_noSVD(partE1FromGamma);
      m_firstElectronElectronIDSVDonly = std::get<double>(electronPIDSVDOnlyFunction(partE1FromGamma));
      m_firstElectronPionIDSVDonly = std::get<double>(pionPIDSVDOnlyFunction(partE1FromGamma));
      m_firstElectronKaonIDSVDonly = std::get<double>(kaonPIDSVDOnlyFunction(partE1FromGamma));
      m_firstElectronProtonIDSVDonly = std::get<double>(protonPIDSVDOnlyFunction(partE1FromGamma));
      m_firstElectronElectronLLSVDonly = std::get<double>(electronLLSVDOnlyFunction(partE1FromGamma));
      m_firstElectronPionLLSVDonly = std::get<double>(pionLLSVDOnlyFunction(partE1FromGamma));
      m_firstElectronKaonLLSVDonly = std::get<double>(kaonLLSVDOnlyFunction(partE1FromGamma));
      m_firstElectronProtonLLSVDonly = std::get<double>(protonLLSVDOnlyFunction(partE1FromGamma));

      m_firstElectronBinaryElectronProtonIDALL = Variable::binaryPID(partE1FromGamma, {11., 2212.});
      m_firstElectronBinaryElectronKaonIDALL = Variable::binaryPID(partE1FromGamma, {11., 321.});
      m_firstElectronBinaryElectronPionIDALL = Variable::binaryPID(partE1FromGamma, {11., 211.});
      m_firstElectronBinaryElectronProtonIDnoSVD = Variable::binaryPID_noSVD(partE1FromGamma, {11., 2212.});
      m_firstElectronBinaryElectronKaonIDnoSVD = Variable::binaryPID_noSVD(partE1FromGamma, {11., 321.});
      m_firstElectronBinaryElectronPionIDnoSVD = Variable::binaryPID_noSVD(partE1FromGamma, {11., 211.});
      m_firstElectronBinaryElectronProtonIDSVDonly = std::get<double>(binaryElectronProtonPIDSVDOnlyFunction(partE1FromGamma));
      m_firstElectronBinaryElectronKaonIDSVDonly = std::get<double>(binaryElectronKaonPIDSVDOnlyFunction(partE1FromGamma));
      m_firstElectronBinaryElectronPionIDSVDonly = std::get<double>(binaryElectronPionPIDSVDOnlyFunction(partE1FromGamma));

      m_firstElectronBinaryProtonElectronIDALL = Variable::binaryPID(partE1FromGamma, {2212., 11.});
      m_firstElectronBinaryKaonElectronIDALL = Variable::binaryPID(partE1FromGamma, {321., 11.});
      m_firstElectronBinaryPionElectronIDALL = Variable::binaryPID(partE1FromGamma, {211., 11.});
      m_firstElectronBinaryProtonElectronIDnoSVD = Variable::binaryPID_noSVD(partE1FromGamma, {2212., 11.});
      m_firstElectronBinaryKaonElectronIDnoSVD = Variable::binaryPID_noSVD(partE1FromGamma, {321., 11.});
      m_firstElectronBinaryPionElectronIDnoSVD = Variable::binaryPID_noSVD(partE1FromGamma, {211., 11.});
      m_firstElectronBinaryProtonElectronIDSVDonly = std::get<double>(binaryProtonElectronPIDSVDOnlyFunction(partE1FromGamma));
      m_firstElectronBinaryKaonElectronIDSVDonly = std::get<double>(binaryKaonElectronPIDSVDOnlyFunction(partE1FromGamma));
      m_firstElectronBinaryPionElectronIDSVDonly = std::get<double>(binaryPionElectronPIDSVDOnlyFunction(partE1FromGamma));

      getObjectPtr<TTree>("Gamma")->Fill();
    }
  }
}
