#include "trg/grl/modules/trggrlneuralnet/GRLNeuroTrainerModule.h"
#ifdef HAS_OPENMP
#include <parallel_fann.hpp>
#else
#include <fann.h>
#endif

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/ecl/TrgEclMapping.h>
#include <trg/ecl/dataobjects/TRGECLCluster.h>
#include <trg/ecl/dataobjects/TRGECLTrg.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <trg/grl/dataobjects/GRLMLPData.h>
#include "trg/grl/dataobjects/TRGGRLUnpackerStore.h"

#include <cdc/geometry/CDCGeometryPar.h>
#include <framework/gearbox/Unit.h>

#include <fstream>
#include <cmath>
#include <TFile.h>

using namespace Belle2;
using namespace std;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(GRLNeuroTrainer)

GRLNeuroTrainerModule::GRLNeuroTrainerModule() : Module()
{
  setDescription(
    "The NeuroTriggerTrainer module of the GRL.\n"
    "Takes CDC track and ECL cluster to prepare input data\n"
    "for the training of a neural network.\n"
    "Networks are trained after the event loop and saved."
  );
  // parameters for saving / loading
  addParam("TRGECLClusters", m_TrgECLClusterName,
           "Name of the StoreArray holding the information of trigger ecl clusters ",
           string("TRGECLClusters"));
  addParam("2DfinderCollection", m_2DfinderCollectionName,
           "Name of the StoreArray holding the tracks made by the 2D finder to be used as input.",
           string("TRGCDC2DFinderTracks"));
  addParam("GRLCollection", m_GRLCollectionName,
           "Name of the StoreArray holding the tracks made by the GRL to be used as input.",
           string("TRGGRLUnpackerStore"));
  addParam("filename", m_filename,
           "Name of the root file where the NeuroTrigger parameters will be saved.",
           string("GRLNeuroTrigger.root"));
  addParam("trainFilename", m_trainFilename,
           "Name of the root file where the generated training samples will be saved.",
           string("GRLNeuroTrigger.root"));
  //addParam("logFilename", m_logFilename,
  //         "Base name of the text files where the training logs will be saved "
  //         "(two for each sector, named logFilename_BestRun_i.log "
  //         "and logFilename_AllOptima_i.log).",
  //         string("GRLNeuroTrigger"));
  addParam("arrayname", m_arrayname,
           "Name of the TObjArray to hold the NeuroTrigger parameters.",
           string("MLPs"));
  addParam("trainArrayname", m_trainArrayname,
           "Name of the TObjArray to hold the training samples.",
           string("trainSets"));
  addParam("saveDebug", m_saveDebug,
           "If true, save parameter distribution of training data "
           "in train file and training curve in log file.", true);
  addParam("load", m_load,
           "Switch to load saved parameters if existing. "
           "Take care not to duplicate training sets!", false);
  // NeuroTrigger parameters
  addParam("nMLP", m_parameters.nMLP,
           "Number of expert MLPs.", m_parameters.nMLP);
  addParam("n_cdc_sector", m_parameters.n_cdc_sector,
           "Number of expert CDC MLPs.", m_parameters.n_cdc_sector);
  addParam("n_ecl_sector", m_parameters.n_ecl_sector,
           "Number of expert ECL MLPs.", m_parameters.n_ecl_sector);
  addParam("i_cdc_sector", m_parameters.i_cdc_sector,
           "#cdc track   of expert MLPs.", m_parameters.i_cdc_sector);
  addParam("i_ecl_sector", m_parameters.i_ecl_sector,
           "#ecl cluster of expert MLPs.", m_parameters.i_ecl_sector);
  addParam("nHidden", m_parameters.nHidden,
           "Number of nodes in each hidden layer for all networks "
           "or factor to multiply with number of inputs (1 list or nMLP lists). "
           "The number of layers is derived from the shape.", m_parameters.nHidden);
  addParam("multiplyHidden", m_parameters.multiplyHidden,
           "If true, multiply nHidden with number of input nodes.",
           m_parameters.multiplyHidden);
  //addParam("target", m_parameters.target,
  //         "Train one output of MLP to give judgement.", m_parameters.targetZ);
  addParam("outputScale", m_parameters.outputScale,
           "Output scale for all networks (1 value list or nMLP value lists). "
           "Output[i] of the MLP is scaled from [-1, 1] "
           "to [outputScale[2*i], outputScale[2*i+1]]. "
           "(units: z[cm] / theta[degree])", m_parameters.outputScale);
  // parameters for training data preparation
  //addParam("nTrainPrepare", m_nTrainPrepare,
  //         "Number of samples for preparation of relevant ID ranges "
  //         "(0: use default ranges).", 1000);
  //addParam("IDranges", m_IDranges,
  //         "If list is not empty, it will replace the default ranges. "
  //         "1 list or nMLP lists. Set nTrainPrepare to 0 if you use this option.",
  //         {});
  //addParam("relevantCut", m_relevantCut,
  //         "Cut for preparation of relevant ID ranges.", 0.02);
  //addParam("cutSum", m_cutSum,
  //         "If true, relevantCut is applied to the sum over hit counters, "
  //         "otherwise directly on the hit counters.", false);
  addParam("nTrainMin", m_nTrainMin,
           "Minimal number of training samples "
           "or factor to multiply with number of weights. "
           "If the minimal number of samples is not reached, "
           "all samples are saved but no training is started.", 10.);
  addParam("nTrainMax", m_nTrainMax,
           "Maximal number of training samples "
           "or factor to multiply with number of weights. "
           "When the maximal number of samples is reached, "
           "no further samples are added.", 10.);
  addParam("multiplyNTrain", m_multiplyNTrain,
           "If true, multiply nTrainMin and nTrainMax with number of weights.",
           true);
  addParam("nValid", m_nValid,
           "Number of validation samples for training.", 1000);
  addParam("nTest", m_nTest,
           "Number of test samples to get resolution after training.", 5000);
  //addParam("stopLoop", m_stopLoop,
  //         "If true, stop event loop when maximal number of samples "
  //         "is reached for all sectors.", true);
  //addParam("rescaleTarget", m_rescaleTarget,
  //         "If true, set target values > outputScale to 1, "
  //         "else skip them.", true);
  // parameters for training
  addParam("wMax", m_wMax,
           "Weights are limited to [-wMax, wMax] after each training epoch "
           "(for convenience of the FPGA implementation).",
           63.);
  addParam("nThreads", m_nThreads,
           "Number of threads for parallel training.", 1);
  addParam("checkInterval", m_checkInterval,
           "Training is stopped if validation error is higher than "
           "checkInterval epochs ago, i.e. either the validation error is increasing "
           "or the gain is less than the fluctuations.", 500);
  addParam("maxEpochs", m_maxEpochs,
           "Maximum number of training epochs.", 10000);
  addParam("repeatTrain", m_repeatTrain,
           "If >1, training is repeated several times with different start weights. "
           "The weights which give the best resolution on the test samples are kept.", 1);
}


void
GRLNeuroTrainerModule::initialize()
{
  //initialize with input parameter
  m_GRLNeuro.initialize(m_parameters);
  n_cdc_sector = m_parameters.n_cdc_sector;
  n_ecl_sector = m_parameters.n_ecl_sector;
  n_sector = m_parameters.nMLP;
  m_trainSets.clear();
  for (unsigned iMLP = 0; iMLP < (unsigned)n_sector; ++iMLP) {
    m_trainSets.push_back(GRLMLPData());
    scale_bg.push_back(0);
  }
  if (m_nTrainMin > m_nTrainMax) {
    m_nTrainMin = m_nTrainMax;
    B2WARNING("nTrainMin set to " << m_nTrainMin << " (was larger than nTrainMax)");
  }

  //initializa histograms
  for (int isector = 0; isector < n_sector; isector++) {
    h_cdc2d_phi_sig  .push_back(new TH1D(("h_cdc2d_phi_sig_" + to_string(isector)).c_str(),
                                         ("h_cdc2d_phi_sig_" + to_string(isector)).c_str(),     64, -3.2, 3.2));
    h_cdc2d_pt_sig   .push_back(new TH1D(("h_cdc2d_pt_sig_" + to_string(isector)).c_str(),
                                         ("h_cdc2d_pt_sig_" + to_string(isector)).c_str(),       100, -5, 5));
    h_selTheta_sig.push_back(new TH1D(("h_selTheta_sig_" + to_string(isector)).c_str(),
                                      ("h_selTheta_sig_" + to_string(isector)).c_str(), 64, -3.2, 3.2));
    h_selPhi_sig  .push_back(new TH1D(("h_selPhi_sig_" + to_string(isector)).c_str(), ("h_selPhi_sig_" + to_string(isector)).c_str(),
                                      64, -3.2, 3.2));
    h_selE_sig    .push_back(new TH1D(("h_selE_sig_" + to_string(isector)).c_str(), ("h_selE_sig_" + to_string(isector)).c_str(),
                                      100, 0, 10));
    h_result_sig  .push_back(new TH1D(("h_result_sig_" + to_string(isector)).c_str(), ("h_result_sig_" + to_string(isector)).c_str(),
                                      100, -1, 1));
    h_cdc2d_phi_bg   .push_back(new TH1D(("h_cdc2d_phi_bg_" + to_string(isector)).c_str(),
                                         ("h_cdc2d_phi_bg_" + to_string(isector)).c_str(),       64, -3.2, 3.2));
    h_cdc2d_pt_bg    .push_back(new TH1D(("h_cdc2d_pt_bg_" + to_string(isector)).c_str(),
                                         ("h_cdc2d_pt_bg_" + to_string(isector)).c_str(),         100, -5, 5));
    h_selTheta_bg .push_back(new TH1D(("h_selTheta_bg_" + to_string(isector)).c_str(), ("h_selTheta_bg_" + to_string(isector)).c_str(),
                                      64, -3.2, 3.2));
    h_selPhi_bg   .push_back(new TH1D(("h_selPhi_bg_" + to_string(isector)).c_str(), ("h_selPhi_bg_" + to_string(isector)).c_str(),
                                      64, -3.2, 3.2));
    h_selE_bg     .push_back(new TH1D(("h_selE_bg_" + to_string(isector)).c_str(), ("h_selE_bg_" + to_string(isector)).c_str(),
                                      100, 0, 10));
    h_result_bg   .push_back(new TH1D(("h_result_bg_" + to_string(isector)).c_str(), ("h_result_bg_" + to_string(isector)).c_str(),
                                      100, -1, 1));
  }
  h_ncdcf_sig.push_back(new TH1D("h_ncdcf_sig", "h_ncdcf_sig", 10, 0, 10));
  h_ncdcs_sig.push_back(new TH1D("h_ncdcs_sig", "h_ncdcs_sig", 10, 0, 10));
  h_ncdci_sig.push_back(new TH1D("h_ncdci_sig", "h_ncdci_sig", 10, 0, 10));
  h_ncdc_sig.push_back(new TH1D("h_ncdc_sig", "h_ncdc_sig", 10, 0, 10));
  h_necl_sig.push_back(new TH1D("h_necl_sig"  , "h_necl_sig" , 10, 0, 10));
  h_ncdcf_bg.push_back(new TH1D("h_ncdcf_bg"  , "h_ncdcf_bg" , 10, 0, 10));
  h_ncdcs_bg.push_back(new TH1D("h_ncdcs_bg"  , "h_ncdcs_bg" , 10, 0, 10));
  h_ncdci_bg.push_back(new TH1D("h_ncdci_bg"  , "h_ncdci_bg" , 10, 0, 10));
  h_ncdc_bg.push_back(new TH1D("h_ncdc_bg", "h_ncdc_bg", 10, 0, 10));
  h_necl_bg.push_back(new TH1D("h_necl_bg"    , "h_necl_bg"  , 10, 0, 10));

  //..Trigger ThetaID for each trigger cell. Could be replaced by getMaxThetaId() for newer MC
  TrgEclMapping* trgecl_obj = new TrgEclMapping();
  for (int tc = 1; tc <= 576; tc++) {
    TCThetaID.push_back(trgecl_obj->getTCThetaIdFromTCId(tc));
  }

  //-----------------------------------------------------------------------------------------
  //..ECL look up tables
  PCmsLabTransform boostrotate;
  for (int tc = 1; tc <= 576; tc++) {

    //..Four vector of a 1 GeV lab photon at this TC
    TVector3 CellPosition = trgecl_obj->getTCPosition(tc);
    TLorentzVector CellLab(1., 1., 1., 1.);
    CellLab.SetTheta(CellPosition.Theta());
    CellLab.SetPhi(CellPosition.Phi());
    CellLab.SetRho(1.);
    CellLab.SetE(1.);

    //..cotan Theta and phi in lab
    TCPhiLab.push_back(CellPosition.Phi());
    double tantheta = tan(CellPosition.Theta());
    TCcotThetaLab.push_back(1. / tantheta);

    //..Corresponding 4 vector in the COM frame
    TLorentzVector CellCOM = boostrotate.rotateLabToCms() * CellLab;
    TCThetaCOM.push_back(CellCOM.Theta()*radtodeg);
    TCPhiCOM.push_back(CellCOM.Phi()*radtodeg);

    //..Scale to give 1 GeV in the COM frame
    TC1GeV.push_back(1. / CellCOM.E());
  }
  radtodeg = 180. / TMath::Pi();

  delete trgecl_obj;
}

void
GRLNeuroTrainerModule::event()
{
  //inputs and outputs
  std::vector<float> input;
  std::vector<float> output;

  ////CDC input
  //StoreArray<CDCTriggerTrack> cdc2DTrkArray(m_2DfinderCollectionName);
  std::vector<float> cdc2d_phi;
  std::vector<float> cdc2d_pt;
  //for (int itrk = 0; itrk < cdc2DTrkArray.getEntries(); itrk++) {
  //  cdc2d_phi.push_back(cdc2DTrkArray[itrk]->getPhi0());
  //  cdc2d_pt.push_back(cdc2DTrkArray[itrk]->getPt());
  //  input.push_back(cdc2DTrkArray[itrk]->getPhi0());
  //  input.push_back(cdc2DTrkArray[itrk]->getPt());
  //}

  //GRL input
  StoreObjPtr<TRGGRLUnpackerStore> GRLStore(m_GRLCollectionName);
  int n_cdcf = 0;
  int n_cdcs = 0;
  int n_cdci = 0;
  int n_cdc = 0;
  int map_cdcf[36];
  int map_cdcs[36];
  int map_cdci[36];
  for (int i = 0; i < 36; i++) {
    map_cdcf[i] = 0;
    map_cdcs[i] = 0;
    map_cdci[i] = 0;
  }

  //full track
  for (int i = 0; i < 36; i++) {
    if (GRLStore->m_phi_CDC[i]) {
      map_cdcf[i] = 1;
    }
  }

  //short track
  for (int i = 0; i < 64; i++) {
    if (GRLStore->m_map_ST2[i]) {
      int j = i * (36. / 64.);
      map_cdcs[j] = 1;
    }
  }

  //inner track
  for (int i = 0; i < 64; i++) {
    if (GRLStore->m_map_TSF0[i]) {
      int j = i * (36. / 64.);
      int j1 = i - 4;
      if (j1 < 0) j1 = j1 + 64;
      int j2 = i - 3;
      if (j2 < 0) j2 = j2 + 64;
      int j3 = i - 2;
      if (j3 < 0) j3 = j3 + 64;
      int j4 = i - 1;
      if (j4 < 0) j4 = j4 + 64;
      int j5 = i;
      int j6 = i + 1;
      if (j6 > 63) j6 = j6 - 64;
      int j7 = i + 2;
      if (j7 > 63) j7 = j7 - 64;
      if (
        (GRLStore->m_map_TSF1[j1] || GRLStore->m_map_TSF1[j2] || GRLStore->m_map_TSF1[j3] || GRLStore->m_map_TSF1[j4]
         || GRLStore->m_map_TSF1[j5])
        &&
        (GRLStore->m_map_TSF2[j3] || GRLStore->m_map_TSF2[j4] || GRLStore->m_map_TSF2[j5] || GRLStore->m_map_TSF2[j6]
         || GRLStore->m_map_TSF2[j7])
      )
        map_cdci[j] = 1;
    }
  }

  //avoid overlap
  for (int i = 0; i < 36; i++) {
    if (map_cdcf[i] == 1) {
      int i1 = i - 2;
      if (i1 < 0) i1 = i1 + 36;
      int i2 = i - 1;
      if (i2 < 0) i2 = i2 + 36;
      int i3 = i;
      int i4 = i + 1;
      if (i4 > 36) i4 = i4 - 36;
      int i5 = i + 2;
      if (i5 > 36) i5 = i5 - 36;
      //map_cdcs[i1]=0;
      map_cdcs[i2] = 0;
      map_cdcs[i3] = 0;
      map_cdcs[i4] = 0;
      //map_cdcs[i5]=0;
      //map_cdci[i1]=0;
      map_cdci[i2] = 0;
      map_cdci[i3] = 0;
      map_cdci[i4] = 0;
      //map_cdci[i5]=0;
    }
  }
  for (int i = 0; i < 36; i++) {
    if (map_cdcs[i] == 1) {
      int i1 = i - 2;
      if (i1 < 0) i1 = i1 + 36;
      int i2 = i - 1;
      if (i2 < 0) i2 = i2 + 36;
      int i3 = i;
      int i4 = i + 1;
      if (i4 > 36) i4 = i4 - 36;
      int i5 = i + 2;
      if (i5 > 36) i5 = i5 - 36;
      //map_cdci[i1]=0;
      map_cdci[i2] = 0;
      map_cdci[i3] = 0;
      map_cdci[i4] = 0;
      //map_cdci[i5]=0;
    }
  }

  ////cout
  //for (int i = 0; i < 36; i++) {
  //  std::cout << map_cdcf[i] << " " ;
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 36; i++) {
  //  std::cout << map_cdcs[i] << " " ;
  //}
  //std::cout << std::endl;
  //for (int i = 0; i < 36; i++) {
  //  std::cout << map_cdci[i] << " " ;
  //}
  //std::cout << std::endl;

  //count
  for (int i = 0; i < 36; i++) {
    if (map_cdcf[i] == 1) {n_cdcf++; n_cdc++;}
    if (map_cdcs[i] == 1) {n_cdcs++; n_cdc++;}
    if (map_cdci[i] == 1) {n_cdci++; n_cdc++;}
  }

  //input
  for (int i = 0; i < 36; i++) {
    input.push_back((map_cdcf[i] - 0.5) * 2);
  }
  for (int i = 0; i < 36; i++) {
    input.push_back((map_cdcs[i] - 0.5) * 2);
  }
  for (int i = 0; i < 36; i++) {
    input.push_back((map_cdci[i] - 0.5) * 2);
  }

  //ECL input
  //..Use only clusters within 100 ns of event timing (from ECL).
  StoreArray<TRGECLTrg> trgArray;
  StoreArray<TRGECLCluster> eclTrgClusterArray(m_TrgECLClusterName);
  int ntrgArray = trgArray.getEntries();
  double EventTiming = -9999.;
  if (ntrgArray > 0) {EventTiming = trgArray[0]->getEventTiming();}
  std::vector<int> selTC;
  std::vector<float> selTheta;
  std::vector<float> selPhi;
  std::vector<float> selE;
  for (int ic = 0; ic < eclTrgClusterArray.getEntries(); ic++) {
    double tcT = abs(eclTrgClusterArray[ic]->getTimeAve() - EventTiming);
    //if (tcT < 100.) {
    int TC = eclTrgClusterArray[ic]->getMaxTCId();
    selTC.push_back(TC);
    selTheta.push_back(TCcotThetaLab[TC - 1]);
    selPhi.push_back(TCPhiLab[TC - 1]);
    selE.push_back(eclTrgClusterArray[ic]->getEnergyDep() * 0.001);
    input.push_back(TCcotThetaLab[TC - 1] / TMath::Pi());
    input.push_back(TCPhiLab[TC - 1] / TMath::Pi());
    input.push_back((eclTrgClusterArray[ic]->getEnergyDep() * 0.001 - 3.5) / 3.5);
    //}
    //B2DEBUG(50,"InputECL " << ic << " " << tcT << " " << TC << " " << TCcotThetaLab[TC-1] << " " << TCPhiLab[TC-1] << " " << eclTrgClusterArray[ic]->getEnergyDep() << " " << EventTiming );
  }

  //output
  bool accepted_signal = false;
  bool accepted_bg     = false;
  bool accepted_hadron = false;
  bool accepted_filter = false;
  bool accepted_bhabha = false;
  StoreObjPtr<SoftwareTriggerResult> result_soft;
  if (result_soft.isValid()) {
    const std::map<std::string, int>& skim_map = result_soft->getResults();
    //if (skim_map.find("software_trigger_cut&skim&accept_tau_tau") != skim_map.end()) {
    //  accepted = (result_soft->getResult("software_trigger_cut&skim&accept_tau_tau") == SoftwareTriggerCutResult::c_accept);
    //}
    if (skim_map.find("software_trigger_cut&skim&accept_hadronb2") != skim_map.end()) {
      accepted_hadron = (result_soft->getResult("software_trigger_cut&skim&accept_hadronb2") == SoftwareTriggerCutResult::c_accept);
    }
    if (skim_map.find("software_trigger_cut&filter&total_result") != skim_map.end()) {
      accepted_filter = (result_soft->getResult("software_trigger_cut&filter&total_result") == SoftwareTriggerCutResult::c_accept);
    }
    if (skim_map.find("software_trigger_cut&skim&accept_bhabha") != skim_map.end()) {
      accepted_bhabha = (result_soft->getResult("software_trigger_cut&skim&accept_bhabha") == SoftwareTriggerCutResult::c_accept);
    }
  }

  accepted_signal = accepted_hadron && accepted_filter;
  accepted_bg     = !accepted_filter;

  //input and output for NN training
  int cdc_sector = cdc2d_phi.size();
  int ecl_sector = selTC.size();
  int isector = cdc_sector * n_ecl_sector + ecl_sector;
  //B2DEBUG(50,"Input " << cdc_sector << " " << ecl_sector << " " << accepted_signal << " " << accepted_bg);
  //if(accepted_signal && !accepted_filter)B2DEBUG(50,"Input " << cdc_sector << " " << ecl_sector << " " << accepted_signal << " " << accepted_filter << " " << accepted_bhabha);

  if (accepted_signal) {
    output.push_back(1);
  } else if (accepted_bg) {
    scale_bg[isector]++;
    if (isector == 3) {
      if (scale_bg[isector] == 100) {
        output.push_back(-1);
        scale_bg[isector] = 1;
      } else return;
    }
    if (isector == 4) {
      if (scale_bg[isector] == 5) {
        output.push_back(-1);
        scale_bg[isector] = 1;
      } else return;
    } else {
      output.push_back(-1);
    }
  } else {
    return;
  }



  if (cdc_sector < n_cdc_sector && ecl_sector < n_ecl_sector) {
    m_trainSets[isector].addSample(input, output);
    if (m_saveDebug) {
      if (accepted_signal) {
        for (int i = 0; i < cdc_sector; i++)     h_cdc2d_phi_sig[isector]->Fill(cdc2d_phi[i]);
        for (int i = 0; i < cdc_sector; i++)     h_cdc2d_pt_sig[isector]->Fill(cdc2d_pt[i]);
        for (int i = 0; i < ecl_sector; i++)     h_selTheta_sig[isector]->Fill(selTheta[i]);
        for (int i = 0; i < ecl_sector; i++)     h_selPhi_sig[isector]->Fill(selPhi[i]);
        for (int i = 0; i < ecl_sector; i++)     h_selE_sig[isector]->Fill(selE[i]);
        h_ncdcf_sig[0]->Fill(n_cdcf);
        h_ncdcs_sig[0]->Fill(n_cdcs);
        h_ncdci_sig[0]->Fill(n_cdci);
        h_ncdc_sig[0]->Fill(n_cdc);
        h_necl_sig[0]->Fill(ecl_sector);
      } else if (accepted_bg) {
        for (int i = 0; i < cdc_sector; i++)     h_cdc2d_phi_bg[isector]->Fill(cdc2d_phi[i]);
        for (int i = 0; i < cdc_sector; i++)     h_cdc2d_pt_bg[isector]->Fill(cdc2d_pt[i]);
        for (int i = 0; i < ecl_sector; i++)     h_selTheta_bg[isector]->Fill(selTheta[i]);
        for (int i = 0; i < ecl_sector; i++)     h_selPhi_bg[isector]->Fill(selPhi[i]);
        for (int i = 0; i < ecl_sector; i++)     h_selE_bg[isector]->Fill(selE[i]);
        h_ncdcf_bg[0]->Fill(n_cdcf);
        h_ncdcs_bg[0]->Fill(n_cdcs);
        h_ncdci_bg[0]->Fill(n_cdci);
        h_ncdc_bg[0]->Fill(n_cdc);
        h_necl_bg[0]->Fill(ecl_sector);
      }
    }
  }
}

void
GRLNeuroTrainerModule::terminate()
{
  // do training for all sectors with sufficient training samples
  for (unsigned isector = 0; isector < m_GRLNeuro.nSectors(); ++isector) {
    // skip sectors that have already been trained
    if (m_GRLNeuro[isector].isTrained())
      continue;
    float nTrainMin = m_multiplyNTrain ? m_nTrainMin * m_GRLNeuro[isector].nWeights() : m_nTrainMin;
    std::cout << m_nTrainMin << " " << m_nValid << " " << m_nTest << std::endl;
    if (m_trainSets[isector].nSamples() < (nTrainMin + m_nValid + m_nTest)) {
      B2WARNING("Not enough training samples for sector " << isector << " (" << (nTrainMin + m_nValid + m_nTest)
                << " requested, " << m_trainSets[isector].nSamples() << " found)");
      continue;
    }
    train(isector);
    m_GRLNeuro[isector].trained = true;
    // save all networks (including the newly trained)
    m_GRLNeuro.save(m_filename, m_arrayname);
  }

  // save the training data
  saveTraindata(m_trainFilename, m_trainArrayname);
}



void
GRLNeuroTrainerModule::train(unsigned isector)
{
#ifdef HAS_OPENMP
  B2INFO("Training network for sector " << isector << " with OpenMP");
#else
  B2INFO("Training network for sector " << isector << " without OpenMP");
#endif
  // initialize network
  unsigned nLayers = m_GRLNeuro[isector].nLayers();
  unsigned* nNodes = new unsigned[nLayers];
  for (unsigned il = 0; il < nLayers; ++il) {
    nNodes[il] = m_GRLNeuro[isector].nNodesLayer(il);
  }
  struct fann* ann = fann_create_standard_array(nLayers, nNodes);
  // initialize training and validation data
  GRLMLPData currentData = m_trainSets[isector];
  // train set
  unsigned nTrain = m_trainSets[isector].nSamples() - m_nValid - m_nTest;
  struct fann_train_data* train_data =
    fann_create_train(nTrain, nNodes[0], nNodes[nLayers - 1]);
  for (unsigned i = 0; i < nTrain; ++i) {
    vector<float> input = currentData.getInput(i);
    for (unsigned j = 0; j < input.size(); ++j) {
      train_data->input[i][j] = input[j];
    }
    vector<float> target = currentData.getTarget(i);
    for (unsigned j = 0; j < target.size(); ++j) {
      train_data->output[i][j] = target[j];
    }
  }
  // validation set
  struct fann_train_data* valid_data =
    fann_create_train(m_nValid, nNodes[0], nNodes[nLayers - 1]);
  for (unsigned i = nTrain; i < nTrain + m_nValid; ++i) {
    vector<float> input = currentData.getInput(i);
    for (unsigned j = 0; j < input.size(); ++j) {
      valid_data->input[i - nTrain][j] = input[j];
    }
    vector<float> target = currentData.getTarget(i);
    for (unsigned j = 0; j < target.size(); ++j) {
      valid_data->output[i - nTrain][j] = target[j];
    }
  }
  // set network parameters
  fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
  fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
  fann_set_training_algorithm(ann, FANN_TRAIN_RPROP);
  // keep full train error curve for best run
  vector<double> bestTrainLog = {};
  vector<double> bestValidLog = {};
  // keep train error of optimum for all runs
  vector<double> trainOptLog = {};
  vector<double> validOptLog = {};
  // repeat training several times with different random start weights
  for (int irun = 0; irun < m_repeatTrain; ++irun) {
    double bestValid = 999.;
    vector<double> trainLog = {};
    vector<double> validLog = {};
    trainLog.assign(m_maxEpochs, 0.);
    validLog.assign(m_maxEpochs, 0.);
    int breakEpoch = 0;
    int bestEpoch = 0;
    vector<fann_type> bestWeights = {};
    bestWeights.assign(m_GRLNeuro[isector].nWeights(), 0.);
    fann_randomize_weights(ann, -0.1, 0.1);
    // train and save the network
    for (int epoch = 1; epoch <= m_maxEpochs; ++epoch) {
#ifdef HAS_OPENMP
      double mse = parallel_fann::train_epoch_irpropm_parallel(ann, train_data, m_nThreads);
#else
      double mse = fann_train_epoch(ann, train_data);
#endif
      trainLog[epoch - 1] = mse;
      // reduce weights that got too large
      for (unsigned iw = 0; iw < ann->total_connections; ++iw) {
        if (ann->weights[iw] > m_wMax)
          ann->weights[iw] = m_wMax;
        else if (ann->weights[iw] < -m_wMax)
          ann->weights[iw] = -m_wMax;
      }
      // evaluate validation set
      fann_reset_MSE(ann);
#ifdef HAS_OPENMP
      double valid_mse = parallel_fann::test_data_parallel(ann, valid_data, m_nThreads);
#else
      double valid_mse = fann_test_data(ann, valid_data);
#endif
      validLog[epoch - 1] = valid_mse;
      // keep weights for lowest validation error
      if (valid_mse < bestValid) {
        bestValid = valid_mse;
        for (unsigned iw = 0; iw < ann->total_connections; ++iw) {
          bestWeights[iw] = ann->weights[iw];
        }
        bestEpoch = epoch;
      }
      // break when validation error increases
      if (epoch > m_checkInterval && valid_mse > validLog[epoch - m_checkInterval]) {
        B2INFO("Training run " << irun << " stopped in epoch " << epoch);
        B2INFO("Train error: " << mse << ", valid error: " << valid_mse <<
               ", best valid: " << bestValid);
        breakEpoch = epoch;
        break;
      }
      // print current status
      if (epoch == 1 || (epoch < 100 && epoch % 10 == 0) || epoch % 100 == 0) {
        B2INFO("Epoch " << epoch << ": Train error = " << mse <<
               ", valid error = " << valid_mse << ", best valid = " << bestValid);
      }
    }
    if (breakEpoch == 0) {
      B2INFO("Training run " << irun << " finished in epoch " << m_maxEpochs);
      breakEpoch = m_maxEpochs;
    }
    trainOptLog.push_back(trainLog[bestEpoch - 1]);
    validOptLog.push_back(validLog[bestEpoch - 1]);
    vector<float> oldWeights = m_GRLNeuro[isector].getWeights();
    m_GRLNeuro[isector].weights = bestWeights;
  }
  if (m_saveDebug) {
    for (unsigned i = nTrain + m_nValid; i < m_trainSets[isector].nSamples(); ++i) {
      vector<float> output = m_GRLNeuro.runMLP(isector, m_trainSets[isector].getInput(i));
      vector<float> target = m_trainSets[isector].getTarget(i);
      for (unsigned iout = 0; iout < output.size(); ++iout) {
        if (((int)target[0]) == 1)h_result_sig[isector]->Fill(output[iout]);
        else                    h_result_bg[isector]->Fill(output[iout]);
      }
    }
  }
  // free memory
  fann_destroy_train(train_data);
  fann_destroy_train(valid_data);
  fann_destroy(ann);
  delete[] nNodes;
}

void
GRLNeuroTrainerModule::saveTraindata(const string& filename, const string& arrayname)
{
  B2INFO("Saving traindata to file " << filename << ", array " << arrayname);
  TFile datafile(filename.c_str(), "RECREATE");
  //TObjArray* trainSets = new TObjArray(m_trainSets.size());
  for (int isector = 0; isector < n_sector; ++isector) {
    //trainSets->Add(&m_trainSets[isector]);
    if (m_saveDebug) {
      h_cdc2d_phi_sig[isector]->Write();
      h_cdc2d_pt_sig[isector]->Write();
      h_selTheta_sig[isector]->Write();
      h_selPhi_sig[isector]->Write();
      h_selE_sig[isector]->Write();
      h_result_sig[isector]->Write();
      h_cdc2d_phi_bg[isector]->Write();
      h_cdc2d_pt_bg[isector]->Write();
      h_selTheta_bg[isector]->Write();
      h_selPhi_bg[isector]->Write();
      h_selE_bg[isector]->Write();
      h_result_bg[isector]->Write();
    }
    h_ncdcf_sig[0]->Write();
    h_ncdcs_sig[0]->Write();
    h_ncdci_sig[0]->Write();
    h_ncdc_sig[0]->Write();
    h_necl_sig[0]->Write();
    h_ncdcf_bg[0]->Write();
    h_ncdcs_bg[0]->Write();
    h_ncdci_bg[0]->Write();
    h_ncdc_bg[0]->Write();
    h_necl_bg[0]->Write();
  }
  //trainSets->Write(arrayname.c_str(), TObject::kSingleKey | TObject::kOverwrite);
  //datafile.Close();
  //trainSets->Clear();
  //delete trainSets;
  for (int isector = 0; isector < n_sector; ++ isector) {
    delete h_cdc2d_phi_sig[isector];
    delete h_cdc2d_pt_sig[isector];
    delete h_selTheta_sig[isector];
    delete h_selPhi_sig[isector];
    delete h_selE_sig[isector];
    delete h_result_sig[isector];
    delete h_cdc2d_phi_bg[isector];
    delete h_cdc2d_pt_bg[isector];
    delete h_selTheta_bg[isector];
    delete h_selPhi_bg[isector];
    delete h_selE_bg[isector];
    delete h_result_bg[isector];
  }
  delete h_ncdcf_sig[0];
  delete h_ncdcs_sig[0];
  delete h_ncdci_sig[0];
  delete h_ncdc_sig[0];
  delete h_necl_sig[0];
  delete h_ncdcf_bg[0];
  delete h_ncdcs_bg[0];
  delete h_ncdci_bg[0];
  delete h_ncdc_bg[0];
  delete h_necl_bg[0];
  h_cdc2d_phi_sig.clear();
  h_cdc2d_pt_sig.clear();
  h_selTheta_sig.clear();
  h_selPhi_sig.clear();
  h_selE_sig.clear();
  h_result_sig.clear();
  h_cdc2d_phi_bg.clear();
  h_cdc2d_pt_bg.clear();
  h_selTheta_bg.clear();
  h_selPhi_bg.clear();
  h_selE_bg.clear();
  h_result_bg.clear();
  h_ncdcf_sig.clear();
  h_ncdcs_sig.clear();
  h_ncdci_sig.clear();
  h_ncdc_sig.clear();
  h_necl_sig.clear();
  h_ncdcf_bg.clear();
  h_ncdcs_bg.clear();
  h_ncdci_bg.clear();
  h_necl_bg.clear();
}

//bool
//GRLNeuroTrainerModule::loadTraindata(const string& filename, const string& arrayname)
//{
//  TFile datafile(filename.c_str(), "READ");
//  if (!datafile.IsOpen()) {
//    B2WARNING("Could not open file " << filename);
//    return false;
//  }
//  TObjArray* trainSets = (TObjArray*)datafile.Get(arrayname.c_str());
//  if (!trainSets) {
//    datafile.Close();
//    B2WARNING("File " << filename << " does not contain key " << arrayname);
//    return false;
//  }
//  m_trainSets.clear();
//  for (int isector = 0; isector < trainSets->GetEntriesFast(); ++isector) {
//    CDCTriggerMLPData* samples = dynamic_cast<CDCTriggerMLPData*>(trainSets->At(isector));
//    if (samples) m_trainSets.push_back(*samples);
//    else B2WARNING("Wrong type " << trainSets->At(isector)->ClassName() << ", ignoring this entry.");
//  }
//  trainSets->Clear();
//  delete trainSets;
//  datafile.Close();
//  B2DEBUG(100, "loaded " << m_trainSets.size() << " training sets");
//  return true;
//}
