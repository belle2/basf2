//+
// File : StatisticsTimingHLTDQMModule.cc
// Description : Module to monitor process timing on HLT
//
// Author : Chunhua LI
// Date : 04 - July - 2017
//-
#include <hlt/softwaretrigger/modules/HLTDQM/StatisticsTimingHLTDQMModule.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <hlt/softwaretrigger/dataobjects/SoftwareTriggerVariables.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/ProcessStatistics.h>
#include <framework/pcore/ProcHandler.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>

#include <TDirectory.h>

#include <map>
#include <string>
#include <iostream>

using namespace Belle2;
using namespace SoftwareTrigger;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(StatisticsTimingHLTDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

StatisticsTimingHLTDQMModule::StatisticsTimingHLTDQMModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor recontruction of cosmic ray on HLT");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("histogramDirectoryName", m_param_histogramDirectoryName,
           "Cosmic Ray DQM histograms on HLT will be put into this directory", std::string("StatisticsTiming_HLT"));

}

StatisticsTimingHLTDQMModule::~StatisticsTimingHLTDQMModule()
{
}

void StatisticsTimingHLTDQMModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_param_histogramDirectoryName.c_str())->cd();
  h_MeanTime = new TH1F("MeanTime", "Average process time per event", 8, 0, 4);
  h_MeanTime->GetXaxis()->SetBinLabel(1, "Unpacker");
  h_MeanTime->GetXaxis()->SetBinLabel(3, "FastReco");
  h_MeanTime->GetXaxis()->SetBinLabel(5, "PhysicsTrigger");
  h_MeanTime->GetXaxis()->SetBinLabel(7, "Flag Sample");
  h_MeanTime->GetYaxis()->SetTitle("Average Process Time/Event (ms)");

  h_EvtTime = new TH1F("EvtTime", "The processing time of events", 300, 0, 3000);
  h_EvtTime->GetXaxis()->SetTitle("Process Time (ms)");
  h_EvtTime->GetYaxis()->SetTitle("Events");

  h_MeanMem = new TH1F("MeanMem", "Average memory comsumption per event", 8, 0, 4);
  h_MeanMem->GetXaxis()->SetBinLabel(1, "Unpacker");
  h_MeanMem->GetXaxis()->SetBinLabel(3, "FastReco");
  h_MeanMem->GetXaxis()->SetBinLabel(5, "PhysicsTrigger");
  h_MeanMem->GetXaxis()->SetBinLabel(7, "Flag Sample");
  h_MeanMem->GetYaxis()->SetTitle("Average Memory Consumption");

  std::string name_hist_time[] = {"Unpacker", "FastReco", "PhysicsTrigger", "Flag Sample"};
  double uplimit_hist[] = {50., 300., 2000., 200.};
  double bin_hist[] = {100, 300, 200, 200};
  for (int i = 0; i < m_nsubhist; i++) {
    TH1F* h_t = new TH1F(name_hist_time[i].c_str(), ("Processing time of " + name_hist_time[i] + " per event").c_str(), bin_hist[i], 0,
                         uplimit_hist[i]);
    h_t->GetXaxis()->SetTitle("Process Time (ms)");
    h_t->GetYaxis()->SetTitle("Events");
    h_ModuleTime.push_back(h_t);
  }

  //modules with large process time
  m_name_topmodule = {"VXDTF", "VXD_DAFRecoFitter", "CDC_DAFRecoFitter", "Combined_DAFRecoFitter", "V0Finder", "TOPReconstructor", "Muid", "Ext"};
  int size_hmod = m_name_topmodule.size();
  h_MeanTime_TopModule = new TH1F("TopTimeModule", "Average processing time of the mdoules with large time consumption",
                                  2 * size_hmod, 0, size_hmod);
  for (int i = 0; i < size_hmod; i++)
    h_MeanTime_TopModule->GetXaxis()->SetBinLabel(2 * i + 1, m_name_topmodule[i].c_str());
  h_MeanTime_TopModule->GetYaxis()->SetTitle("Processing Time (ms)");
  oldDir->cd();

}


void StatisticsTimingHLTDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  m_fastreco_modules = {
    "TFCDC_WireHitPreparer",
    "TFCDC_ClusterPreparer",
    "TFCDC_SegmentFinderFacetAutomaton",
    "TFCDC_AxialTrackFinderLegendre",
//"TFCDC_TrackQualityAsserter",
    "TFCDC_StereoHitFinder",
    "TFCDC_SegmentTrackCombiner",
//"TFCDC_TrackQualityAsserter",
    "TFCDC_TrackExporter",
//"IPTrackTimeEstimator",
    "ECLDigitCalibrator",
    "ECLCRFinder",
    "ECLLocalMaximumFinder",
    "ECLSplitterN1",
    "ECLSplitterN2",
    "ECLShowerCorrector",
    "ECLShowerCalibrator",
    "ECLShowerShape",
    "ECLCovarianceMatrix",
    "ECLFinalizer"
  };

  m_physfilter_modules = {
    "SetupGenfitExtrapolation",
    "VXDTF",
    "RecoTrackCreator",
    "VXD_DAFRecoFitter",
    "CDC_DAFRecoFitter",
    "VXDCDCTrackMerger",
//"PruneRecoTracks",
//"PruneRecoTracks",
    "TrackFinderMCTruthRecoTracks",
    "MCRecoTracksMatcher",
//"IPTrackTimeEstimator",
    "Combined_DAFRecoFitter",
    "TrackCreator",
    "V0Finder",
    "CDCDedxPID",
    "VXDDedxPID",
//"PruneRecoTracks",
    "PruneGenfitTracks",
    "Ext",
    "TOPChannelMasker",
    "TOPReconstructor",
    "ARICHFillHits",
    "ARICHReconstructor",
    "ECLTrackShowerMatch",
    "ECLElectronId",
    "MCMatcherECLClusters",
    "EKLMReconstructor",
    "BKLMReconstructor",
    "KLMK0LReconstructor",
    "MCMatcherKLMClusters",
    "Muid",
    "MdstPID",
    "ParticleLoader_pi+:HLT",
    "ParticleLoader_gamma:HLT"
  };

}

void StatisticsTimingHLTDQMModule::event()
{
  StoreObjPtr<ProcessStatistics> stats("", DataStore::c_Persistent);
  std::vector<double> mtime_evt;
  std::vector<double> mmem_evt;
  std::vector<double> runtime_mod;
  for (int i = 0; i < m_nsubhist; i++) {
    mmem_evt.push_back(0.0);
    mtime_evt.push_back(0.0);
    runtime_mod.push_back(0.0);
  }

  if (stats.isValid()) {
    std::vector<ModuleStatistics> modules = stats->getAll();
    for (auto& module : modules) {
//double mtime_mod = module.getTimeMean(module.c_Event)*1e-6;
      double mtime_mod = module.getTimeMean(module.c_Event) * 1e-6;
      double stime_mod = module.getTimeSum(module.c_Event) * 1e-6;
      double mmem_mod = module.getMemoryMean();
//double smem_mod = module.getMemorySum();
      std::string name_mod = module.getName();

      auto mod_exit = m_sumtime_module.find(name_mod);
      double runtime_mod_this = 0.0;
      if (mod_exit != m_sumtime_module.end())
        runtime_mod_this = stime_mod - (mod_exit->second);

      for (unsigned int m = 0; m < m_name_topmodule.size(); m++) {
        if (name_mod == m_name_topmodule[m])
          h_MeanTime_TopModule->SetBinContent(2 * m + 1, mtime_mod);
      }

      std::vector<bool> check_module_inlist = {
//0:Unpacker modules
        name_mod.find("Unpacker") != std::string::npos,
//1:fast_reco modules
        std::find(m_fastreco_modules.begin(), m_fastreco_modules.end(), name_mod) != m_fastreco_modules.end(),
//2:phys_reco modules
        std::find(m_physfilter_modules.begin(), m_physfilter_modules.end(), name_mod) != m_physfilter_modules.end(),
//tag samples modules
        name_mod.find(":calib") != std::string::npos || name_mod.find(":dqm") != std::string::npos || name_mod.find(":skim") != std::string::npos
      };

      for (int i = 0; i < m_nsubhist; i++) {
        if (check_module_inlist[i]) {
          mmem_evt[i] += mmem_mod;
          mtime_evt[i] += mtime_mod;
          runtime_mod[i] += runtime_mod_this;
        }
      }
    }

    double total_evt_time = 0.0;
    for (int i = 0; i < m_nsubhist; i++) {
      if (runtime_mod[i] <= 0.0)continue;
      total_evt_time += mtime_evt[i];
      int bin = i * 2 + 1;
      h_MeanTime->SetBinContent(bin, mtime_evt[i]);
      h_MeanMem->SetBinContent(bin, mmem_evt[i]);
      h_ModuleTime[i]->Fill(runtime_mod[i]);
    }
    h_EvtTime->Fill(total_evt_time);

    m_sumtime_module.clear();
    for (auto& module : modules) {
      double stime_mod = module.getTimeSum(module.c_Event) * 1e-6;
      std::string name_mod = module.getName();
//clear the timing of the previous event
//timing of this event for the next one
      m_sumtime_module[name_mod] = stime_mod;
    }
  }

}
