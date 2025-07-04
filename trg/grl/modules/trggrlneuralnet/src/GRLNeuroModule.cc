/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


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

#include <fstream>
#include <cmath>
#include <TFile.h>

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
  {{64, 64}});
  addParam("multiplyHidden", m_parameters.multiplyHidden,
           "If true, multiply nHidden with number of input nodes.", false);
  addParam("outputScale", m_parameters.outputScale,
           "Output scale for all networks (1 value list or nMLP value lists). "
           "Output[i] of the MLP is scaled from [-1, 1] "
           "to [outputScale[2*i], outputScale[2*i+1]]. "
           "(units: z[cm] / theta[degree])",
  {{ -1., 1.}});
  addParam("weightFiles", m_weightFileNames,
           "Name of the file where the weights of MLPs are saved. "
           "the default file is $BELLE2_LOCAL_DIR/data/trg/grl/weights.dat",
  {FileSystem::findFile("/data/trg/grl/weights.dat", true)});
  addParam("biasFiles", m_biasFileNames,
           "Name of the file where the biases of MLPs are saved. "
           "the default file is $BELLE2_LOCAL_DIR/data/trg/grl/bias.dat",
  {FileSystem::findFile("/data/trg/grl/bias.dat", true)});
  addParam("TRGECLClusters", m_TrgECLClusterName,
           "Name of the StoreArray holding the information of trigger ecl clusters ",
           string("TRGECLClusters"));
  addParam("MVACut", m_nn_thres,
           "Cut value applied to the MLP output",
  {-1});
  addParam("useDB", m_useDB,
           "Flag to use database to set config", true);
}


void
GRLNeuroModule::initialize()
{

  TrgEclMapping* trgecl_obj = new TrgEclMapping();
  for (int tc = 1; tc <= 576; tc++) {
    TCThetaID.push_back(trgecl_obj->getTCThetaIdFromTCId(tc));
  }

  //-----------------------------------------------------------------------------------------
  //..ECL look up tables
  PCmsLabTransform boostrotate;
  for (int tc = 1; tc <= 576; tc++) {

    //..Four vector of a 1 GeV lab photon at this TC
    ROOT::Math::XYZVector CellPosition = trgecl_obj->getTCPosition(tc);
    ROOT::Math::PxPyPzEVector CellLab;
    CellLab.SetPx(CellPosition.Unit().X());
    CellLab.SetPy(CellPosition.Unit().Y());
    CellLab.SetPz(CellPosition.Unit().Z());
    CellLab.SetE(1.);

    TCThetaLab.push_back(CellLab.Theta()*TMath::RadToDeg());
    TCPhiLab.push_back(CellLab.Phi()*TMath::RadToDeg() + 180.0);

  }

  delete trgecl_obj;

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
      m_parameters.n_cdc_sector   = m_db_trggrlconfig->get_ecltaunn_n_cdc_sector();
      m_parameters.n_ecl_sector   = m_db_trggrlconfig->get_ecltaunn_n_ecl_sector();
      m_parameters.i_cdc_sector   = m_db_trggrlconfig->get_ecltaunn_i_cdc_sector();
      m_parameters.i_ecl_sector   = m_db_trggrlconfig->get_ecltaunn_i_ecl_sector();
      m_nn_thres[0]               = m_db_trggrlconfig->get_ecltaunn_threshold();

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
                                  ("h_target_" + to_string(isector)).c_str(),  100, 0.0, 1.0));
    }
  }
}

void
GRLNeuroModule::event()
{

  //ECL input
  //..Use only clusters within 100 ns of event timing (from ECL).
  //StoreArray<TRGECLTrg> trgArray;
  StoreArray<TRGECLCluster> eclTrgClusterArray(m_TrgECLClusterName);
  StoreObjPtr<TRGGRLInfo> trgInfo(m_TrgGrlInformationName);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  std::vector<std::tuple<float, float, float, float>> eclClusters;
///
  int necl = eclTrgClusterArray.getEntries();
  for (int ic = 0; ic < necl; ic++) {
    int TC = eclTrgClusterArray[ic]->getMaxTCId();
    float energy = eclTrgClusterArray[ic]->getEnergyDep() * 1000.0;
    float theta = TCThetaLab[TC - 1];
    float phi = TCPhiLab[TC - 1];
    float time = eclTrgClusterArray[ic]->getTimeAve();
    eclClusters.emplace_back(energy, theta, phi, time);
  }

  std::sort(eclClusters.begin(), eclClusters.end(),
            [](const std::tuple<float, float, float, float>& a,
  const std::tuple<float, float, float, float>& b) {
    return std::get<0>(a) > std::get<0>(b);
  });

  // MLPinput
  std::vector<float> MLPinput;
  MLPinput.clear();
  MLPinput.assign(24, 0);

  for (size_t i = 0; i < std::min(eclClusters.size(), size_t(6)); i++) {
    MLPinput[i]  = std::get<0>(eclClusters[i]); // E
    MLPinput[6 + i]  = std::get<1>(eclClusters[i]); // Theta
    MLPinput[12 + i] = std::get<2>(eclClusters[i]); // Phi
    MLPinput[18 + i]  = std::get<3>(eclClusters[i]); // time
  }



  // //////////////////////////////////////////////////////////////////////////////////////////////////////////

  //Energy, theta and phi data are the quantized.
  //Energy: LSB = ADC count (5.5MeV)
  //theta : 0 ~ 180∘, LSB = 1.6025∘
  //phi : 0 ~ 360∘, LSB = 1.6025∘
  float LSB_ADC   = 1 / 5.5;
  float LSB_angle = 1 / 1.6025;
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  std::for_each(MLPinput.begin() + 0, MLPinput.begin() + 6, [LSB_ADC](float & x) { x = std::ceil(x * LSB_ADC); });
  std::for_each(MLPinput.begin() + 6,  MLPinput.begin() + 12,  [LSB_angle](float & x) { x = std::ceil(x * LSB_angle);});
  std::for_each(MLPinput.begin() + 12,  MLPinput.begin() + 18, [LSB_angle](float & x) { x = std::ceil(x * LSB_angle); });

//new add
//  float LSB_time = 1.0f;
//  std::for_each(MLPinput.begin() + 18, MLPinput.begin() + 24, [](float &x) {
//      x = std::ceil(x);
//      x = std::min(x, 255.0f);
//     x = std::max(x, 0.0f);
//  });


  float target = m_GRLNeuro.runMLP(0, MLPinput);
  if (m_saveHist) {
    h_target[0]->Fill(target);
  }
  if (target > m_nn_thres[0]) {
    trgInfo->setTauNN(true);
  } else trgInfo->setTauNN(false);

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
