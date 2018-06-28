/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * eclLocalRunAnalyzer                                                    *
 *                                                                        *
 * eclLocalRunCalibAnalizer is a tool designed to obtaind local           *
 * run calibration plots.                                                 *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov (S.S.Gribanov@inp.nsk.su) (BINP)         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// STL
#include <list>
#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <regex>
// BOOST
#include <boost/program_options.hpp>
#include <boost/format.hpp>
// ROOT
#include <TFile.h>
#include <TObject.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TH1.h>
#include <TH1F.h>
#include <TGraph.h>
// ECL
#include <ecl/utility/ECLLocalRunCalibLoader.h>
using namespace Belle2;
namespace po = boost::program_options;
// Program options.
struct commandOptions {
  // Enable central database.
  bool m_centraldb = false;
  // Obtain calibration plots,
  // normalized to reference plots.
  bool m_withref = false;
  // Experiment number.
  int m_exp = -1;
  // Run number.
  int m_run = -1;
  // Cell id.
  int m_cellid = 0;
  // Begin time.
  std::string m_timestart;
  // End time.
  std::string m_timestop;
  // Path to run time table.
  std::string m_timetab = "timetab.cvs";
  // A tag in the central database ot
  // path to a local database.
  std::string m_dbname = "localdb/database.txt";
  // Path to the output file.
  std::string m_path = "ecl_calib_output.root";
};
// Write object to file.
void writeObjsToFile(const std::list<TObject*>& objs,
                     const std::string& fileName)
{
  TFile* fl = TFile::Open(fileName.c_str(), "recreate");
  fl->cd();
  for (const auto& obj : objs) {
    obj->Write();
  }
  fl->Close();
}
// Delete objects.
void deleteObjs(std::list<TObject*>* objs)
{
  for (auto obj : *objs) {
    delete obj;
  }
}
// Fill plots for single run.
void fillSingleRunHistograms(
  const ECLLocalRunCalibLoader& data,
  const int& exp,
  const int& run,
  const bool& withref,
  std::list<TObject*>* objs)
{
  TTree* tree;
  std::vector <
  std::pair<int, int >> runs =
  {{exp, run}};
  data.getTree(&tree, runs, withref);
  int cellid;
  int time_count;
  float time_mean;
  float time_stddev;
  int ampl_count;
  float ampl_mean;
  float ampl_stddev;
  int ref_time_count;
  float ref_time_mean;
  float ref_time_stddev;
  int ref_ampl_count;
  float ref_ampl_mean;
  float ref_ampl_stddev;
  tree->SetBranchAddress("cellid", &cellid);
  tree->SetBranchAddress("time_count", &time_count);
  tree->SetBranchAddress("time_mean", &time_mean);
  tree->SetBranchAddress("time_stddev", &time_stddev);
  tree->SetBranchAddress("ampl_count", &ampl_count);
  tree->SetBranchAddress("ampl_mean", &ampl_mean);
  tree->SetBranchAddress("ampl_stddev", &ampl_stddev);
  if (withref) {
    tree->SetBranchAddress("ref_time_count", &ref_time_count);
    tree->SetBranchAddress("ref_time_mean", &ref_time_mean);
    tree->SetBranchAddress("ref_time_stddev", &ref_time_stddev);
    tree->SetBranchAddress("ref_ampl_count", &ref_ampl_count);
    tree->SetBranchAddress("ref_ampl_mean", &ref_ampl_mean);
    tree->SetBranchAddress("ref_ampl_stddev", &ref_ampl_stddev);
  }
  auto h_time_count = new TH1F("time_count", "Number of time counts",
                               100, 0, 0);
  auto h_time_mean = new TH1F("time_mean", "Mean time", 100, 0, 0);
  auto h_time_stddev = new TH1F("time_stddev", "Time standard deviation",
                                100, 0, 0);
  auto h_ampl_count = new TH1F("ampl_count", "Number of amplitude counts",
                               100, 0, 0);
  auto h_ampl_mean = new TH1F("ampl_mean", "Mean amplitude",
                              100, 0, 0);
  auto h_ampl_stddev = new TH1F("ampl_stddev", "Amplitude standard deviation",
                                100, 0, 0);

  auto h_ref_time_count = new TH1F("ref_time_count", "Number of time counts "
                                   "for refference run",
                                   100, 0, 0);
  auto h_ref_time_mean = new TH1F("ref_time_mean", "Mean time for reference run",
                                  100, 0, 0);
  auto h_ref_time_stddev = new TH1F("ref_time_stddev", "Time standard deviation "
                                    "for reference run",
                                    100, 0, 0);
  auto h_norm_time_mean = new TH1F("norm_time_mean", "Mean time normalized to a "
                                   "reference mean time: "
                                   "norm_time_mean = time_mean - ref_time_mean",
                                   100, 0, 0);
  auto h_norm_time_stddev = new TH1F("norm_time_stddev", "Time standard deviation "
                                     "normalized to a reference time standard deviation: "
                                     "norm_time_stddev = time_stddev / ref_time_stddev",
                                     100, 0, 0);
  auto h_ref_ampl_count = new TH1F("ref_ampl_count", "Number of amplitude counts "
                                   " for reference run",
                                   100, 0, 0);
  auto h_ref_ampl_mean = new TH1F("ref_ampl_mean", "Mean amplitude for reference run",
                                  100, 0, 0);
  auto h_ref_ampl_stddev = new TH1F("ref_ampl_stddev", "Amplitude standard deviation "
                                    "for reference run",
                                    100, 0, 0);
  auto h_norm_ampl_mean = new TH1F("norm_ampl_mean", "Mean amplitude normalized to a "
                                   "a reference mean amplitude: "
                                   "norm_ampl_mean = ampl_mean / ref_ampl_mean",
                                   100, 0, 0);
  auto h_norm_ampl_stddev = new TH1F("norm_ampl_stddev", "Amplitude standard deviation "
                                     "normalized to areference amplitude standard deviation: "
                                     "norm_ampl_stddev = ampl_stddev / ref_ampl_stddev",
                                     100, 0, 0);
  auto g_time_count_c = new TGraph();
  g_time_count_c->SetName("time_count_c");
  g_time_count_c->SetTitle("Number of time counts per cellid");
  g_time_count_c->SetMarkerStyle(20);
  auto g_time_mean_c = new TGraph();
  g_time_mean_c->SetName("time_mean_c");
  g_time_mean_c->SetTitle("Mean time valuses per cellid");
  g_time_mean_c->SetMarkerStyle(20);
  auto g_time_stddev_c = new TGraph();
  g_time_stddev_c->SetName("time_stddev_c");
  g_time_stddev_c->SetTitle("Time standard deviation per cellid");
  g_time_stddev_c->SetMarkerStyle(20);
  auto g_ampl_count_c = new TGraph();
  g_ampl_count_c->SetName("ampl_count_c");
  g_ampl_count_c->SetTitle("Number of amplitude counts per cellid");
  g_ampl_count_c->SetMarkerStyle(20);
  auto g_ampl_mean_c = new TGraph();
  g_ampl_mean_c->SetName("ampl_mean_c");
  g_ampl_mean_c->SetTitle("Mean amplitude per cellid");
  g_ampl_mean_c->SetMarkerStyle(20);
  auto g_ampl_stddev_c = new TGraph();
  g_ampl_stddev_c->SetName("ampl_stddev_c");
  g_ampl_stddev_c->SetTitle("Amplitude standard deviation per cellid");
  g_ampl_stddev_c->SetMarkerStyle(20);
  TGraph* g_ref_time_count_c = nullptr;
  TGraph* g_ref_time_mean_c = nullptr;
  TGraph* g_ref_time_stddev_c = nullptr;
  TGraph* g_norm_time_mean_c = nullptr;
  TGraph* g_norm_time_stddev_c = nullptr;
  TGraph* g_ref_ampl_count_c = nullptr;
  TGraph* g_ref_ampl_mean_c = nullptr;
  TGraph* g_ref_ampl_stddev_c = nullptr;
  TGraph* g_norm_ampl_mean_c = nullptr;
  TGraph* g_norm_ampl_stddev_c = nullptr;
  if (withref) {
    g_ref_time_count_c = new TGraph();
    g_ref_time_count_c->SetName("ref_time_count_c");
    g_ref_time_count_c->SetTitle("Number of time counts per cellid for "
                                 "reference run");
    g_ref_time_count_c->SetMarkerStyle(20);
    g_ref_time_mean_c = new TGraph();
    g_ref_time_mean_c->SetName("ref_time_mean_c");
    g_ref_time_mean_c->SetTitle("Mean time per cellid for "
                                "reference run");
    g_ref_time_mean_c->SetMarkerStyle(20);
    g_ref_time_stddev_c = new TGraph();
    g_ref_time_stddev_c->SetName("ref_time_stddev_c");
    g_ref_time_stddev_c->SetTitle("Time standard deviation per cellid for "
                                  "reference run");
    g_norm_time_mean_c = new TGraph();
    g_norm_time_mean_c->SetName("norm_time_mean_c");
    g_norm_time_mean_c->SetTitle("Mean time normalized to a reference "
                                 "mean time (per cellid): "
                                 "norm_time_mean = time_mean - ref_time_mean");
    g_norm_time_stddev_c = new TGraph();
    g_norm_time_stddev_c->SetName("norm_time_stddev_c");
    g_norm_time_stddev_c->SetTitle("Time standard deviation normalized to a "
                                   "reference time standard deviation (per cellid): "
                                   "norm_time_stddev = time_stddev / ref_time_stddev");
    g_ref_time_stddev_c->SetMarkerStyle(20);
    g_ref_ampl_count_c = new TGraph();
    g_ref_ampl_count_c->SetName("ref_ampl_count_c");
    g_ref_ampl_count_c->SetTitle("Number of amplitude counts per cellid "
                                 "for reference run");
    g_ref_ampl_count_c->SetMarkerStyle(20);
    g_ref_ampl_mean_c = new TGraph();
    g_ref_ampl_mean_c->SetName("ref_ampl_mean_c");
    g_ref_ampl_mean_c->SetTitle("Mean amplitude per cellid for "
                                "reference run");
    g_ref_ampl_mean_c->SetMarkerStyle(20);
    g_ref_ampl_stddev_c = new TGraph();
    g_ref_ampl_stddev_c->SetName("ref_ampl_stddev_c");
    g_ref_ampl_stddev_c->SetTitle("Amplitude standard deviation "
                                  "per cellid for reference run");
    g_norm_ampl_mean_c = new TGraph();
    g_norm_ampl_mean_c->SetName("norm_ampl_mean_c");
    g_norm_ampl_mean_c->SetTitle("Mean amplitude normalized to a reference "
                                 "mean amplitude (per cellid): "
                                 "norm_ampl_mean = ampl_mean / ref_ampl_mean");
    g_norm_ampl_stddev_c = new TGraph();
    g_norm_ampl_stddev_c->SetName("norm_ampl_stddev_c");
    g_norm_ampl_stddev_c->SetTitle("Amplitude standard deviation normalized to a "
                                   "reference amplitude standard deviation "
                                   "(per cellid): "
                                   "norm_ampl_stddev = ampl_stddev / ref_ampl_stddev");
    g_ref_ampl_stddev_c->SetMarkerStyle(20);
  }
  int N = tree->GetEntries();
  for (int k = 0; k < N; ++k) {
    tree->GetEntry(k);
    h_time_count->Fill(time_count);
    h_time_mean->Fill(time_mean);
    h_time_stddev->Fill(time_stddev);
    h_ampl_count->Fill(ampl_count);
    h_ampl_mean->Fill(ampl_mean);
    h_ampl_stddev->Fill(ampl_stddev);
    g_time_count_c->SetPoint(k, cellid, time_count);
    g_time_mean_c->SetPoint(k, cellid, time_mean);
    g_time_stddev_c->SetPoint(k, cellid, time_stddev);
    g_ampl_count_c->SetPoint(k, cellid, ampl_count);
    g_ampl_mean_c->SetPoint(k, cellid, ampl_mean);
    g_ampl_stddev_c->SetPoint(k, cellid, ampl_stddev);
    if (withref) {
      h_ref_time_count->Fill(ref_time_count);
      h_ref_time_mean->Fill(ref_time_mean);
      h_ref_time_stddev->Fill(ref_time_stddev);
      h_norm_time_mean->Fill(time_mean - ref_time_mean);
      h_norm_time_stddev->Fill(time_stddev / ref_time_stddev);
      h_ref_ampl_count->Fill(ref_ampl_count);
      h_ref_ampl_mean->Fill(ref_ampl_mean);
      h_ref_ampl_stddev->Fill(ref_ampl_stddev);
      h_norm_ampl_mean->Fill(ampl_mean / ref_ampl_mean);
      h_norm_ampl_stddev->Fill(ampl_stddev / ref_ampl_stddev);
      g_ref_time_count_c->SetPoint(cellid - 1, cellid, ref_time_count);
      g_ref_time_mean_c->SetPoint(cellid - 1, cellid, ref_time_mean);
      g_ref_time_stddev_c->SetPoint(cellid - 1, cellid, ref_time_stddev);
      g_norm_time_mean_c->SetPoint(cellid - 1, cellid, time_mean - ref_time_mean);
      g_norm_time_stddev_c->SetPoint(cellid - 1, cellid, time_stddev / ref_time_stddev);
      g_ref_ampl_count_c->SetPoint(cellid - 1, cellid, ref_ampl_count);
      g_ref_ampl_mean_c->SetPoint(cellid - 1, cellid, ref_ampl_mean);
      g_ref_ampl_stddev_c->SetPoint(cellid - 1, cellid, ref_ampl_stddev);
      g_norm_ampl_mean_c->SetPoint(cellid - 1, cellid, ampl_mean / ref_ampl_mean);
      g_norm_ampl_stddev_c->SetPoint(cellid - 1, cellid, ampl_stddev / ref_ampl_stddev);
    }
  }
  if (withref) {
    *objs = {h_time_count, h_time_mean, h_time_stddev,
             h_ampl_count, h_ampl_mean, h_ampl_stddev,
             h_ref_time_count, h_ref_time_mean, h_ref_time_stddev,
             h_norm_time_mean, h_norm_time_stddev,
             h_ref_ampl_count, h_ref_ampl_mean, h_ref_ampl_stddev,
             h_norm_ampl_mean, h_norm_ampl_stddev,
             g_time_count_c, g_time_mean_c, g_time_stddev_c,
             g_ampl_count_c, g_ampl_mean_c, g_ampl_stddev_c,
             g_ref_time_count_c, g_ref_time_mean_c, g_ref_time_stddev_c,
             g_norm_time_mean_c, g_norm_time_stddev_c,
             g_ref_ampl_count_c, g_ref_ampl_mean_c, g_ref_ampl_stddev_c,
             g_norm_ampl_mean_c, g_norm_ampl_stddev_c
            };
  } else {
    *objs = {h_time_count, h_time_mean, h_time_stddev,
             h_ampl_count, h_ampl_mean, h_ampl_stddev,
             h_ref_time_count, h_ref_time_mean, h_ref_time_stddev,
             h_ref_ampl_count, h_ref_ampl_mean, h_ref_ampl_stddev
            };
  }
  for (auto obj : *objs) {
    auto cast_th1 =
      dynamic_cast<TH1*>(obj);
    if (cast_th1) {
      cast_th1->SetDirectory(0);
    }
  }
  delete tree;
}
// Fill trend plots.
void fillTrendPlots(
  const ECLLocalRunCalibLoader& data,
  const std::string& timestart,
  const std::string& timestop,
  const int& incellid,
  const bool& withref,
  std::list<TObject*>* objs)
{
  TTree* tree;
  data.getTree(&tree, timestart, timestop, withref);
  int run;
  int exp;
  int ref_run;
  int ref_exp;
  UInt_t run_start;
  int cellid;
  int time_count;
  float time_mean;
  float time_stddev;
  int ampl_count;
  float ampl_mean;
  float ampl_stddev;
  int ref_time_count;
  float ref_time_mean;
  float ref_time_stddev;
  int ref_ampl_count;
  float ref_ampl_mean;
  float ref_ampl_stddev;
  tree->SetBranchAddress("run_start", &run_start);
  tree->SetBranchAddress("run", &run);
  tree->SetBranchAddress("exp", &exp);
  tree->SetBranchAddress("cellid", &cellid);
  tree->SetBranchAddress("time_count", &time_count);
  tree->SetBranchAddress("time_mean", &time_mean);
  tree->SetBranchAddress("time_stddev", &time_stddev);
  tree->SetBranchAddress("ampl_count", &ampl_count);
  tree->SetBranchAddress("ampl_mean", &ampl_mean);
  tree->SetBranchAddress("ampl_stddev", &ampl_stddev);
  if (withref) {
    tree->SetBranchAddress("ref_run", &ref_run);
    tree->SetBranchAddress("ref_exp", &ref_exp);
    tree->SetBranchAddress("ref_time_count", &ref_time_count);
    tree->SetBranchAddress("ref_time_mean", &ref_time_mean);
    tree->SetBranchAddress("ref_time_stddev", &ref_time_stddev);
    tree->SetBranchAddress("ref_ampl_count", &ref_ampl_count);
    tree->SetBranchAddress("ref_ampl_mean", &ref_ampl_mean);
    tree->SetBranchAddress("ref_ampl_stddev", &ref_ampl_stddev);
  }
  TGraph* g_exp_t = new TGraph();
  g_exp_t->SetName("exp_t");
  TGraph* g_run_t = new TGraph();
  g_run_t->SetName("run_t");
  TGraph* g_time_count_t = new TGraph();
  g_time_count_t->SetName("time_count_t");
  TGraph* g_time_mean_t = new TGraph();
  g_time_mean_t->SetName("time_mean_t");
  TGraph* g_time_stddev_t = new TGraph();
  g_time_stddev_t->SetName("time_stddev_t");
  TGraph* g_ampl_count_t = new TGraph();
  g_ampl_count_t->SetName("ampl_count_t");
  TGraph* g_ampl_mean_t = new TGraph();
  g_ampl_mean_t->SetName("ampl_mean_t");
  TGraph* g_ampl_stddev_t = new TGraph();
  g_ampl_stddev_t->SetName("ampl_stddev_t");
  TGraph* g_ref_exp_t = nullptr;
  TGraph* g_ref_run_t = nullptr;
  TGraph* g_ref_time_count_t = nullptr;
  TGraph* g_ref_time_mean_t = nullptr;
  TGraph* g_ref_time_stddev_t = nullptr;
  TGraph* g_norm_time_mean_t = nullptr;
  TGraph* g_norm_time_stddev_t = nullptr;
  TGraph* g_ref_ampl_count_t = nullptr;
  TGraph* g_ref_ampl_mean_t = nullptr;
  TGraph* g_ref_ampl_stddev_t = nullptr;
  TGraph* g_norm_ampl_mean_t = nullptr;
  TGraph* g_norm_ampl_stddev_t = nullptr;
  if (withref) {
    g_ref_exp_t = new TGraph();
    g_ref_exp_t->SetName("ref_exp_t");
    g_ref_run_t = new TGraph();
    g_ref_run_t->SetName("ref_run_t");
    g_ref_time_count_t = new TGraph();
    g_ref_time_count_t->SetName("ref_time_count_t");
    g_ref_time_mean_t = new TGraph();
    g_ref_time_mean_t->SetName("ref_time_mean_t");
    g_ref_time_stddev_t = new TGraph();
    g_ref_time_stddev_t->SetName("ref_time_stddev_t");
    g_norm_time_mean_t = new TGraph();
    g_norm_time_mean_t->SetName("norm_time_mean_t");
    g_norm_time_stddev_t = new TGraph();
    g_norm_time_stddev_t->SetName("norm_time_stddev_t");
    g_ref_ampl_count_t = new TGraph();
    g_ref_ampl_count_t->SetName("ref_ampl_count_t");
    g_ref_ampl_mean_t = new TGraph();
    g_ref_ampl_mean_t->SetName("ref_ampl_mean_t");
    g_ref_ampl_stddev_t = new TGraph();
    g_ref_ampl_stddev_t->SetName("ref_ampl_stddev_t");
    g_norm_ampl_mean_t = new TGraph();
    g_norm_ampl_mean_t->SetName("norm_ampl_mean_t");
    g_norm_ampl_stddev_t = new TGraph();
    g_norm_ampl_stddev_t->SetName("norm_ampl_stddev_t");
    g_ref_exp_t->SetTitle("Number of experiment (reference run)");
    g_ref_run_t->SetTitle("Number of refference run");
  }
  g_exp_t->SetTitle("Number of experiment");
  g_run_t->SetTitle("Number of run");
  if (incellid <= 0) {
    g_time_count_t->SetTitle("Number of time counts");
    g_time_mean_t->SetTitle("Mean time");
    g_time_stddev_t->SetTitle("Time standard deviation");
    g_ampl_count_t->SetTitle("Number of a,plitude counts");
    g_ampl_mean_t->SetTitle("Mean amplitude");
    g_ampl_stddev_t->SetTitle("Amplitude standard deviation");
    if (withref) {
      g_ref_time_count_t->SetTitle("Number of time counts for reference run");
      g_ref_time_mean_t->SetTitle("Mean time for reference run");
      g_ref_time_stddev_t->SetTitle("Time standard deviation for "
                                    "reference run");
      g_norm_time_mean_t->SetTitle("Mean time normalized to a reference mean time: "
                                   "norm_time_mean = time_mean - ref_time_mean");
      g_norm_time_stddev_t->SetTitle("Time standard deviation normalize to a reference "
                                     "time standard deviation: norm_time_stddev = "
                                     "time_stddev / ref_time_stddev");
      g_ref_ampl_count_t->SetTitle("Number of a,plitude counts "
                                   "for reference run");
      g_ref_ampl_mean_t->SetTitle("Mean amplitude for reference run");
      g_ref_ampl_stddev_t->SetTitle("Amplitude standard deviation "
                                    "for reference run");
      g_norm_ampl_mean_t->SetTitle("Mean amplitude normalized to a reference "
                                   "mean amplitude: norm_ampl_mean = "
                                   "ampl_mean / ref_ampl_mean");
    }
  } else {
    std::string tstr;
    tstr = (boost::format("Number of time "
                          "counts (cellid = %d)") % incellid).str();
    g_time_count_t->SetTitle(tstr.c_str());
    tstr = (boost::format("Mean time (cellid = %d)") % incellid).str();
    g_time_mean_t->SetTitle(tstr.c_str());
    tstr = (boost::format("Time standard "
                          "deviation (cellid = %d)") % incellid).str();
    g_time_stddev_t->SetTitle(tstr.c_str());
    tstr = (boost::format("Number of amplitude "
                          "counts (cellid = %d)") % incellid).str();
    g_ampl_count_t->SetTitle(tstr.c_str());
    tstr = (boost::format("Mean amplitude (cellid = %d)") % incellid).str();
    g_ampl_mean_t->SetTitle(tstr.c_str());
    tstr = (boost::format("Amplitude standard deviation "
                          "(cellid = %d)") % incellid).str();
    g_ampl_stddev_t->SetTitle(tstr.c_str());
    if (withref) {
      tstr = (boost::format("Number of time counts for "
                            "teference run (cellid = %d)") % incellid).str();
      g_ref_time_count_t->SetTitle(tstr.c_str());
      tstr = (boost::format("Mean time for refference run "
                            "(cellid = %d)") % incellid).str();
      g_ref_time_mean_t->SetTitle(tstr.c_str());
      tstr = (boost::format("Time standard deviation for "
                            "reference run (cellid = %d)") % incellid).str();
      g_ref_time_stddev_t->SetTitle(tstr.c_str());
      tstr = (boost::format("Mean time normalized to a reference mean time "
                            "(cellid = %d): "
                            "norm_time_mean = time_mean - ref_time_mean") % incellid).str();
      g_norm_time_mean_t->SetTitle(tstr.c_str());
      tstr = (boost::format("Time standard deviation normalized to a "
                            "reference time standard deviation "
                            "(cellid = %d): "
                            "norm_time_stddev = time_stddev / ref_time_stddev") % incellid).str();
      g_norm_time_stddev_t->SetTitle(tstr.c_str());
      tstr = (boost::format("Number of amplitude counts "
                            "for reference run (cellid = %d)") % incellid).str();
      g_ref_ampl_count_t->SetTitle(tstr.c_str());
      tstr = (boost::format("Mean amplitude for reference "
                            "run (cellid = %d)") % incellid).str();
      g_ref_ampl_mean_t->SetTitle(tstr.c_str());
      tstr = (boost::format("Amplitude standard deviation"
                            "for reference run (cellid = %d)") % incellid).str();
      g_ref_ampl_stddev_t->SetTitle(tstr.c_str());
      tstr = (boost::format("Mean amplitude normalized to "
                            "a reference mean amplitude "
                            "(cellid = %d): "
                            "norm_ampl_mean = ampl_mean / ref_ampl_mean") % incellid).str();
      g_norm_ampl_mean_t->SetTitle(tstr.c_str());
      tstr = (boost::format("Amplitude standard deviation normalized to a "
                            "reference amplitude standard deviation "
                            "(cellid = %d): "
                            "norm_ampl_stddev = ampl_stddev / ref_ampl_stddev") % incellid).str();
      g_norm_ampl_stddev_t->SetTitle(tstr.c_str());
    }
  }
  int N = tree->GetEntries();
  int j = 0;
  int p = 0;
  int curexp = -1;
  int currun = -1;
  for (int k = 0; k < N; ++k) {
    tree->GetEntry(k);
    bool crfill = (incellid <= 0) || (incellid == cellid);
    if (crfill) {
      if (curexp != exp || currun != run) {
        g_exp_t->SetPoint(p, run_start, exp);
        g_run_t->SetPoint(p, run_start, run);
        if (withref) {
          g_ref_exp_t->SetPoint(p, run_start, ref_exp);
          g_ref_run_t->SetPoint(p, run_start, ref_run);
        }
        p++;
        curexp = exp;
        currun = run;
      }
      g_time_count_t->SetPoint(j, run_start, time_count);
      g_time_mean_t->SetPoint(j, run_start, time_mean);
      g_time_stddev_t->SetPoint(j, run_start, time_stddev);
      g_ampl_count_t->SetPoint(j, run_start, ampl_count);
      g_ampl_mean_t->SetPoint(j, run_start, ampl_mean);
      g_ampl_stddev_t->SetPoint(j, run_start, ampl_stddev);
      if (withref) {
        g_ref_time_count_t->SetPoint(j, run_start, ref_time_count);
        g_ref_time_mean_t->SetPoint(j, run_start, ref_time_mean);
        g_ref_time_stddev_t->SetPoint(j, run_start, ref_time_stddev);
        g_norm_time_mean_t->SetPoint(j, run_start, time_mean - ref_time_mean);
        g_norm_time_stddev_t->SetPoint(j, run_start, time_stddev / ref_time_stddev);
        g_ref_ampl_count_t->SetPoint(j, run_start, ref_ampl_count);
        g_ref_ampl_mean_t->SetPoint(j, run_start, ref_ampl_mean);
        g_ref_ampl_stddev_t->SetPoint(j, run_start, ref_ampl_stddev);
        g_norm_ampl_mean_t->SetPoint(j, run_start, ampl_mean / ref_ampl_mean);
        g_norm_ampl_stddev_t->SetPoint(j, run_start, ampl_stddev / ref_ampl_stddev);
      }
      j++;
    }
  }
  if (withref) {
    *objs = {
      g_exp_t, g_run_t,
      g_time_count_t, g_time_mean_t, g_time_stddev_t,
      g_ampl_count_t, g_ampl_mean_t, g_ampl_stddev_t,
      g_ref_exp_t, g_ref_run_t,
      g_ref_time_count_t, g_ref_time_mean_t, g_ref_time_stddev_t,
      g_norm_time_mean_t, g_norm_time_stddev_t,
      g_ref_ampl_count_t, g_ref_ampl_mean_t, g_ref_ampl_stddev_t,
      g_norm_ampl_mean_t, g_norm_ampl_stddev_t
    };
  } else {
    *objs = {
      g_exp_t, g_run_t,
      g_time_count_t, g_time_mean_t, g_time_stddev_t,
      g_ampl_count_t, g_ampl_mean_t, g_ampl_stddev_t
    };
  }
  for (auto obj : *objs) {
    auto casted = dynamic_cast<TGraph*>(obj);
    if (casted) {
      casted->GetXaxis()->SetTimeDisplay(1);
      casted->GetXaxis()->SetTimeOffset(0);
      casted->GetXaxis()->SetTimeFormat("#splitline{%y-%m-%d}{%H:%M}");
      casted->GetXaxis()->SetLabelOffset(0.04);
      casted->GetXaxis()->SetNdivisions(510);
      casted->SetMarkerStyle(20);
    }
  }
  delete tree;
}
// Set options.
void setOptions(po::options_description* desc,
                commandOptions* opts)
{
  desc->add_options()
  ("help", "Produce a help message.")
  ("centraldb", po::bool_switch(&(opts->m_centraldb)), "Use a central "
   "database.")
  ("dbname", po::value<std::string>(&(opts->m_dbname)), "A central database "
   "tag or path to a local database.")
  ("withref", po::bool_switch(&(opts->m_withref)), "Obtain results taking into "
   "account refference events")
  ("cellid", po::value<int>(&(opts->m_cellid)), "Cell id")
  ("exp", po::value<int>(&(opts->m_exp)), "Number of experiment.")
  ("run", po::value<int>(&(opts->m_run)), "Number of run.")
  ("timestart", po::value<std::string>(&(opts->m_timestart)),
   "Start time (SQL time format).")
  ("timestop", po::value<std::string>(&(opts->m_timestop)),
   "Stop time (SQL time format).")
  ("timetab", po::value<std::string>(&(opts->m_timetab)), "Table winth run times")
  ("path", po::value<std::string>(&(opts->m_path)),
   "Path to output ROOT file.");
}
int main(int argc, char* argv[])
{
  commandOptions opts;
  po::options_description desc("Allowed options:");
  setOptions(&desc, &opts);
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }
  if (vm.count("timestop") &&
      !vm.count("timestart")) {
    std::cout << "You need to set timestart!" << std::endl;
    return 0;
  }
  if (!vm.count("timestop") &&
      vm.count("timestart")) {
    std::cout << "You need to set timestop!" << std::endl;
    return 0;
  }
  if (vm.count("timestart")) {
    std::regex filter("()(_)()");
    opts.m_timestart = std::regex_replace(opts.m_timestart, filter, "$1 $3");
    opts.m_timestop = std::regex_replace(opts.m_timestop, filter, "$1 $3");
  }
  if (vm.count("exp") &&
      vm.count("run") &&
      !vm.count("timestart") &&
      !vm.count("timestop") &&
      !vm.count("cellid") &&
      !vm.count("timetab")) {
    ECLLocalRunCalibLoader
    data(!opts.m_centraldb,
         opts.m_dbname.c_str());
    std::list<TObject*> objs;
    fillSingleRunHistograms(data, opts.m_exp, opts.m_run,
                            opts.m_withref, &objs);
    writeObjsToFile(objs, opts.m_path);
    deleteObjs(&objs);
    return 0;
  }
  if (vm.count("timestart") &&
      vm.count("timestop") &&
      !vm.count("exp") &&
      !vm.count("run")) {
    ECLLocalRunCalibLoader
    data(!opts.m_centraldb,
         opts.m_dbname.c_str(),
         opts.m_timetab.c_str());
    std::list<TObject*> objs;
    fillTrendPlots(data,
                   opts.m_timestart, opts.m_timestop,
                   opts.m_cellid, opts.m_withref, &objs);
    writeObjsToFile(objs, opts.m_path);
    deleteObjs(&objs);
    return 0;
  }
  std::cout << desc << std::endl;
  return 0;
}
