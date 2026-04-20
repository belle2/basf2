/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include <iomanip>
#include <cmath>
#include <TFile.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/gearbox/Unit.h>
#include <framework/utilities/FileSystem.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <trg/ecl/TrgEclMapping.h>
#include <trg/ecl/dataobjects/TRGECLCluster.h>
#include <trg/grl/dataobjects/GRLMLPData.h>
#include <trg/grl/dataobjects/TRGGRLInfo.h>
#include "trg/grl/dataobjects/TRGGRLUnpackerStore.h"
#include "trg/grl/modules/trggrlneuralnet/GRLNeuroModule.h"


using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GRLNeuro);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
//
GRLNeuroModule::GRLNeuroModule() : Module()
{
  setDescription(
    "The NeuroTrigger module of the GRL.\n"
    "Takes CDC track and ECL cluster to prepare input data\n"
    "for the training of a neural network.\n"
    "Networks are trained after the event loop and saved."
  );
  setPropertyFlags(c_ParallelProcessingCertified);
  // parameters for saving / loading
  addParam("TrgGrlInformation", m_TrgGrlInformationName,
           "Name of the StoreArray holding the information of tracks and clusters from cdc ecl klm.",
           string("TRGGRLObjects"));
  addParam("HistFileName", m_HistFileName,
           "Name of the root file saving the output histogram.",
           string("hist.root"));
  addParam("SaveHist", m_saveHist,
           "Save the output histogram to root file.",
           false);
  addParam("nMLP", m_parameters.nMLP,
           "Number of expert MLPs.", 1u);
  addParam("n_cdc_sector", m_parameters.n_cdc_sector,
           "Number of expert CDC MLPs.", 0u);
  addParam("n_ecl_sector", m_parameters.n_ecl_sector,
           "Number of expert ECL MLPs.", 1u);
  addParam("i_cdc_sector", m_parameters.i_cdc_sector,
           "#cdc track of expert MLPs.", {0});
  addParam("i_ecl_sector", m_parameters.i_ecl_sector,
           "#ecl cluster of expert MLPs.", {24});
  addParam("nHidden", m_parameters.nHidden,
           "Number of nodes in each hidden layer for all networks "
           "or factor to multiply with number of inputs (1 list or nMLP lists). "
           "The number of layers is derived from the shape.",
  {{24, 24, 24}});
  addParam("multiplyHidden", m_parameters.multiplyHidden,
           "If true, multiply nHidden with number of input nodes.", false);

  addParam("csvThetaPhiFile", m_csvThetaPhiFile,
           "CSV file for theta/phi correction per tcid.",
  {FileSystem::findFile("/data/trg/grl/tcid_correct_theta_phi.csv", true)});

  addParam("TRGECLClusters", m_TrgECLClusterName,
           "Name of the StoreArray holding the information of trigger ecl clusters ",
           string("TRGECLClusters"));
  addParam("MVACut", m_nn_thres,
           "Cut value applied to the MLP output",
  {0});
  addParam("useDB", m_useDB,
           "Flag to use database to set config", true);

  // // TRGGRLInfo must exist for this module
  // addRequired<TRGGRLInfo>(m_TrgGrlInformationName);

}


void
GRLNeuroModule::initialize()
{

  std::string csvPath = FileSystem::findFile(m_csvThetaPhiFile, true);
  if (csvPath.empty()) {
    B2ERROR("Cannot find tcid_correct_theta_phi.csv");
    return;
  }

  std::ifstream infile(csvPath);
  if (!infile.is_open()) {
    B2ERROR(("Failed to open " + csvPath).c_str());
    return;
  }

  std::string line;
  bool isFirstLine = true;
  while (std::getline(infile, line)) {
    if (isFirstLine) {
      isFirstLine = false;
      continue;
    }

    std::stringstream ss(line);
    std::string tc_str, theta_str, phi_str;
    std::getline(ss, tc_str, ',');
    std::getline(ss, theta_str, ',');
    std::getline(ss, phi_str, ',');

    if (tc_str.empty() || theta_str.empty() || phi_str.empty()) continue;

    int tc = std::stoi(tc_str);
    if (tc == 0) continue;
    double theta = std::stod(theta_str) * TMath::RadToDeg();
    double phi   = std::stod(phi_str)   * TMath::RadToDeg();

    thetaMap[tc] = theta;
    phiMap[tc]   = phi;
  }

  infile.close();

  TCThetaLab.clear();
  TCPhiLab.clear();
  TCThetaLab.reserve(576);
  TCPhiLab.reserve(576);

  for (int tc = 1; tc <= 576; ++tc) {
    if (thetaMap.count(tc)) {
      TCThetaLab.push_back(thetaMap[tc]);
      TCPhiLab.push_back(phiMap[tc]);
    } else {

      TCThetaLab.push_back(0.0);
      TCPhiLab.push_back(0.0);
    }
  }


  B2INFO("Loaded TC geometry (theta/phi) from tcid_correct_theta_phi.csv");
  B2INFO(Form("Example: TC 1 => theta = %.3f deg, phi = %.3f deg", TCThetaLab[0], TCPhiLab[0]));


}


void
GRLNeuroModule::beginRun()
{
  if (m_useDB) {
    if (not m_db_trggrlconfig.isValid()) {
      StoreObjPtr<EventMetaData> evtMetaData;
      B2FATAL("No database for TRG GRL config. exp " << evtMetaData->getExperiment() << " run "
              << evtMetaData->getRun());
    } else {
      m_parameters.nMLP           = m_db_trggrlconfig->get_ecltaunn_nMLP();
      m_parameters.multiplyHidden = m_db_trggrlconfig->get_ecltaunn_multiplyHidden();
      m_parameters.nHidden        = m_db_trggrlconfig->get_ecltaunn_nHidden();
      m_parameters.nOutput        = m_db_trggrlconfig->get_ecltaunn_nOutput();
      m_parameters.n_cdc_sector   = m_db_trggrlconfig->get_ecltaunn_n_cdc_sector();
      m_parameters.n_ecl_sector   = m_db_trggrlconfig->get_ecltaunn_n_ecl_sector();
      m_parameters.i_cdc_sector   = m_db_trggrlconfig->get_ecltaunn_i_cdc_sector();
      m_parameters.i_ecl_sector   = m_db_trggrlconfig->get_ecltaunn_i_ecl_sector();
      m_nn_thres[0]               = m_db_trggrlconfig->get_ecltaunn_threshold();
      m_parameters.total_bit_bias  = m_db_trggrlconfig->get_ecltaunn_total_bit_bias();
      m_parameters.int_bit_bias    = m_db_trggrlconfig->get_ecltaunn_int_bit_bias();
      m_parameters.is_signed_bias  = m_db_trggrlconfig->get_ecltaunn_is_signed_bias();
      m_parameters.rounding_bias   = m_db_trggrlconfig->get_ecltaunn_rounding_bias();
      m_parameters.saturation_bias = m_db_trggrlconfig->get_ecltaunn_saturation_bias();
      m_parameters.total_bit_accum  = m_db_trggrlconfig->get_ecltaunn_total_bit_accum();
      m_parameters.int_bit_accum    = m_db_trggrlconfig->get_ecltaunn_int_bit_accum();
      m_parameters.is_signed_accum  = m_db_trggrlconfig->get_ecltaunn_is_signed_accum();
      m_parameters.rounding_accum   = m_db_trggrlconfig->get_ecltaunn_rounding_accum();
      m_parameters.saturation_accum = m_db_trggrlconfig->get_ecltaunn_saturation_accum();
      m_parameters.total_bit_weight  = m_db_trggrlconfig->get_ecltaunn_total_bit_weight();
      m_parameters.int_bit_weight    = m_db_trggrlconfig->get_ecltaunn_int_bit_weight();
      m_parameters.is_signed_weight  = m_db_trggrlconfig->get_ecltaunn_is_signed_weight();
      m_parameters.rounding_weight   = m_db_trggrlconfig->get_ecltaunn_rounding_weight();
      m_parameters.saturation_weight = m_db_trggrlconfig->get_ecltaunn_saturation_weight();
      m_parameters.total_bit_relu  = m_db_trggrlconfig->get_ecltaunn_total_bit_relu();
      m_parameters.int_bit_relu    = m_db_trggrlconfig->get_ecltaunn_int_bit_relu();
      m_parameters.is_signed_relu  = m_db_trggrlconfig->get_ecltaunn_is_signed_relu();
      m_parameters.rounding_relu   = m_db_trggrlconfig->get_ecltaunn_rounding_relu();
      m_parameters.saturation_relu = m_db_trggrlconfig->get_ecltaunn_saturation_relu();
      m_parameters.total_bit  = m_db_trggrlconfig->get_ecltaunn_total_bit();
      m_parameters.int_bit    = m_db_trggrlconfig->get_ecltaunn_int_bit();
      m_parameters.is_signed  = m_db_trggrlconfig->get_ecltaunn_is_signed();
      m_parameters.rounding   = m_db_trggrlconfig->get_ecltaunn_rounding();
      m_parameters.saturation = m_db_trggrlconfig->get_ecltaunn_saturation();
      m_parameters.W_input = m_db_trggrlconfig->get_ecltaunn_W_input();
      m_parameters.I_input = m_db_trggrlconfig->get_ecltaunn_I_input();

      B2INFO("DB GRLConfig: nMLP=" << m_db_trggrlconfig->get_ecltaunn_nMLP()
             << " weight=" << m_db_trggrlconfig->get_ecltaunn_weight().size()
             << " bias=" << m_db_trggrlconfig->get_ecltaunn_bias().size()
             << " total_bit_bias=" << m_db_trggrlconfig->get_ecltaunn_total_bit_bias().size());


      m_GRLNeuro.initialize(m_parameters);

      for (unsigned int isector = 0; isector < m_parameters.nMLP; isector++) {
        if (!m_GRLNeuro.load(isector, m_db_trggrlconfig->get_ecltaunn_weight()[isector], m_db_trggrlconfig->get_ecltaunn_bias()[isector])) {
          B2ERROR("weight of GRL ecltaunn could not be loaded correctly.");
        }
      }
    }
  }

  if (m_saveHist) {
    for (unsigned int isector = 0; isector < m_parameters.nMLP; isector++) {
      h_target.push_back(new TH1D(("h_target_" + to_string(isector)).c_str(),
                                  ("h_target_" + to_string(isector)).c_str(),  100, -40.0, 40.0));
    }
  }
}

void GRLNeuroModule::event()
{

  //StoreArray<TRGECLTrg> trgArray;
  StoreObjPtr<TRGGRLInfo> trgInfo(m_TrgGrlInformationName);
  // if (!trgInfo.isValid()) {
  //   std::cout << "[WARNING] TRGGRLObjects not found in this event!" << std::endl;
  //   return;
  // }

  StoreArray<TRGECLCluster> eclTrgClusterArray(m_TrgECLClusterName);
  int necl = eclTrgClusterArray.getEntries();

  std::vector<std::tuple<float, float, float, float, int>> eclClusters; //  TC id
  eclClusters.reserve(necl);

  for (int ic = 0; ic < necl; ic++) {
    auto* eclCluster = eclTrgClusterArray[ic];
    int TC = eclCluster->getMaxTCId();

    //float energy = eclCluster->getEnergyDep() ;  // data
    float energy = eclTrgClusterArray[ic]->getEnergyDep() * 1000.0;//MC
    float time   = eclCluster->getTimeAve();
    float theta = TCThetaLab[TC - 1 ];
    float phi   = TCPhiLab[TC - 1  ] + 180;
    float thetaComp = theta;

    eclClusters.emplace_back(energy, thetaComp, phi, time, TC);

  }


  std::sort(eclClusters.begin(), eclClusters.end(),
            [](const std::tuple<float, float, float, float, int>& a,
  const std::tuple<float, float, float, float, int>& b) {
    return std::get<0>(a) > std::get<0>(b);
  });
  // ====quantization
  std::vector<float> MLPinput(24, 0.0f);
  for (size_t i = 0; i < eclClusters.size() && i < 6; i++) {
    float energy, theta, phi, time;
    int TC;
    std::tie(energy, theta, phi, time, TC) = eclClusters[i];
    MLPinput[i]      = energy;
    MLPinput[i + 6]  = theta;
    MLPinput[i + 12] = phi;
    MLPinput[i + 18] = time;
  }


  float LSB_ADC   = 1 / 5.25;
  float LSB_angle = 1 / 1.40625;
  std::for_each(MLPinput.begin() + 0,  MLPinput.begin() + 6,  [LSB_ADC](float & x)   { x = std::ceil(x * LSB_ADC); });
  std::for_each(MLPinput.begin() + 6,  MLPinput.begin() + 12, [LSB_angle](float & x) { x = std::ceil(x * LSB_angle); });
  std::for_each(MLPinput.begin() + 12, MLPinput.begin() + 18, [LSB_angle](float & x) { x = std::ceil(x * LSB_angle); });

  for (size_t i = 0; i < eclClusters.size() && i < 6; i++) {
    float energy, theta, phi, time;
    int TC;
    std::tie(energy, theta, phi, time, TC) = eclClusters[i];

  }

  // Run MLP
  std::vector<float> target = m_GRLNeuro.runMLP(0, MLPinput);
  unsigned num_target = target.size();

  //fill 0th output for debugging
  if (m_saveHist && num_target > 0) {
    h_target[0]->Fill(target[0]);
  }

  //if one of output exceed threshold, put true
  bool target_output = false;
  for (unsigned io = 0; io < num_target; io++) {
    if (target[io] > m_nn_thres[io]) {
      target_output = true;
      break;
    }
  }
  trgInfo->setTauNN(target_output);
}



void
GRLNeuroModule::terminate()
{
  if (m_saveHist) {
    TFile* file = new TFile(TString(m_HistFileName), "recreate");
    for (unsigned int isector = 0; isector < m_parameters.nMLP; isector++) {
      h_target[isector]->Write();
    }
    file->Write();
    file->Close();
  }

}
