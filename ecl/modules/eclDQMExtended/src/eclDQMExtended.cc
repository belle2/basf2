/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 *  ECL Data Quality Monitor (Second Module)                              *
 *                                                                        *
 * This module provides histograms to check out ECL electronics logic     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitry Matvienko (d.v.matvienko@inp.nsk.su)              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//THIS MODULE
#include <ecl/modules/eclDQMExtended/eclDQMExtended.h>

//FRAMEWORK
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

//ECL
#include <ecl/utility/ECLDspUtilities.h>
#include <ecl/utility/ECLDspEmulator.h>

//STL
#include <regex>
#include <map>
#include <vector>
#include <string>
#include <iostream>

//ROOT
#include <TH1F.h>
#include <TH2F.h>
#include <TDirectory.h>

//BOOST
#include <boost/filesystem.hpp>
#include <boost/format.hpp>


//NAMESPACE(S)
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDQMEXTENDED)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDQMEXTENDEDModule::ECLDQMEXTENDEDModule()
  : HistoModule(),
    m_ECLDspDataArray0("ECLDSPPars0"),
    m_ECLDspDataArray1("ECLDSPPars1"),
    m_ECLDspDataArray2("ECLDSPPars2"),
    m_calibrationThrA0("ECL_FPGA_LowAmp"),
    m_calibrationThrAhard("ECL_FPGA_HitThresh"),
    m_calibrationThrAskip("ECL_FPGA_StoreDigit")

{

  //Set module properties
  setDescription("ECL Data Quality Monitor. Logic Test");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "histogram directory in ROOT file", std::string("ECL"));
  addParam("InitKey", m_InitKey,
           "How to initialize DSP coeffs: ''DB'' or ''File'' are acceptable ", std::string("DB"));
  addParam("DSPDirectoryName", m_DSPDirectoryName,
           "directory for DSP coeffs", std::string("/hsm/belle2/bdata/users/dmitry/dsp/"));
  addParam("RunName", m_RunName,
           "Name of run with DSP files", std::string("run0000/"));
  addParam("SaveDetailedFitData", m_SaveDetailedFitData, "Save detailed data "
           "(ampdiff_{cellid,shaper}, timediff_{cellid,shaper} histograms) for "
           "failed fits.", false);
  addParam("AdjustedTiming", m_adjusted_timing, "Use improved procedure for "
           "time determination in ShaperDSP emulator", true);

  std::vector<int> default_skip = {
    // By default, skip delayed bhabha and random trigger events
    TRGSummary::ETimingType::TTYP_DPHY,
    TRGSummary::ETimingType::TTYP_RAND,
    TRGSummary::ETimingType::TTYP_POIS,
  };
  addParam("skipEvents", m_skipEvents, "Skip events that have listed timing "
           "source (from TRGSummary)", default_skip);
}

ECLDQMEXTENDEDModule::~ECLDQMEXTENDEDModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void ECLDQMEXTENDEDModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;

  // Create a separate histogram directory and cd into it.

  TDirectory* dirDAQ = dynamic_cast<TDirectory*>(oldDir->Get(m_histogramDirectoryName.c_str()));
  if (!dirDAQ) dirDAQ = oldDir->mkdir(m_histogramDirectoryName.c_str());
  dirDAQ->cd();

  //1D histograms creation.

  std::vector<TH1F*> h_amp_qualityfail_raw, h_time_qualityfail_raw;

  for (int i = 0; i < 4; i++) {
    std::string h_name_a, h_title_a, h_name_t, h_title_t;
    h_name_a = str(boost::format("amp_timefail_q%1%") % i);
    h_title_a = str(boost::format("Amp for time mismatches w/ FPGA fit qual=%1%") % i);
    h_name_t = str(boost::format("time_ampfail_q%1%") % i);
    h_title_t = str(boost::format("Time for amp mismatches w/ FPGA fit qual=%1%") % i);
    TH1F* h_a = new TH1F(h_name_a.c_str(), h_title_a.c_str(), 1200, 0, 262144);
    TH1F* h_t = new TH1F(h_name_t.c_str(), h_title_t.c_str(), 240, -2050, 2050);
    h_a->SetOption("LIVE");
    h_t->SetOption("LIVE");
    h_amp_timefail.push_back(h_a);
    h_time_ampfail.push_back(h_t);
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (j != i) {
        std::string h_name_a, h_title_a, h_name_t, h_title_t;
        h_name_a = str(boost::format("amp_qf%1%_qd%2%") % i % j);
        h_title_a = str(boost::format("Amp for C++ fit qual=%1% and FPGA fit qual=%2%") % i % j);
        h_name_t = str(boost::format("time_qf%1%_qd%2%") % i % j);
        h_title_t = str(boost::format("Time for C++ fit qual=%1% and FPGA fit qual=%2%") % i % j);
        TH1F* h_a = new TH1F(h_name_a.c_str(), h_title_a.c_str(), 1200, 0, 262144);
        TH1F* h_t = new TH1F(h_name_t.c_str(), h_title_t.c_str(), 240, -2050, 2050);
        h_a->SetOption("LIVE");
        h_t->SetOption("LIVE");
        h_amp_qualityfail_raw.push_back(h_a);
        h_time_qualityfail_raw.push_back(h_t);
      }
    }
    h_amp_qualityfail.push_back(h_amp_qualityfail_raw);
    h_time_qualityfail.push_back(h_time_qualityfail_raw);
    h_amp_qualityfail_raw.clear();
    h_time_qualityfail_raw.clear();
  }

  h_ampfail_quality  = new TH1F("ampfail_quality", "Number of FPGA <-> C++ fitter #bf{amp} inconsistencies vs fit qual", 5, -1, 4);
  h_ampfail_quality->SetFillColor(kPink - 4);
  h_ampfail_quality->GetXaxis()->SetTitle("FPGA fit qual. -1-all evts,0-good,1-int overflow,2-low amp,3-bad chi2");
  h_ampfail_quality->SetDrawOption("hist");
  h_ampfail_quality->SetOption("LIVE");

  h_timefail_quality  = new TH1F("timefail_quality", "Number of FPGA <-> C++ fitter #bf{time} inconsistencies vs fit qual", 5, -1, 4);
  h_timefail_quality->SetFillColor(kPink - 4);
  h_timefail_quality->GetXaxis()->SetTitle("FPGA fit qual. -1-all evts,0-good,1-int overflow,2-low amp,3-bad chi2");
  h_ampfail_quality->SetDrawOption("hist");
  h_timefail_quality->SetOption("LIVE");

  h_ampfail_cellid = new TH1F("ampfail_cellid", "Cell IDs w/ amp inconsistencies", 8736, 1, 8737);
  h_ampfail_cellid->GetXaxis()->SetTitle("Cell ID");
  h_ampfail_cellid->SetOption("LIVE");

  h_timefail_cellid = new TH1F("timefail_cellid", "Cell IDs w/ time inconsistencies", 8736, 1, 8737);
  h_timefail_cellid->GetXaxis()->SetTitle("Cell ID");
  h_timefail_cellid->SetOption("LIVE");

  h_amptimefail_cellid = new TH1F("amptimefail_cellid", "Cell IDs w/ time and amp inconsistencies", 8736, 1, 8737);
  h_amptimefail_cellid->GetXaxis()->SetTitle("Cell ID");
  h_amptimefail_cellid->SetOption("LIVE");

  h_ampfail_shaperid = new TH1F("ampfail_shaperid", "Shaper IDs w/ amp inconsistencies", 624, 1, 625);
  h_ampfail_shaperid->GetXaxis()->SetTitle("Shaper ID");
  h_ampfail_shaperid->SetOption("LIVE");

  h_timefail_shaperid = new TH1F("timefail_shaperid", "Shaper IDs w/ time inconsistencies", 624, 1, 625);
  h_timefail_shaperid->GetXaxis()->SetTitle("Shaper ID");
  h_timefail_shaperid->SetOption("LIVE");

  h_amptimefail_shaperid = new TH1F("amptimefail_shaperid", "Shaper IDs w/ time and amp inconsistencies", 624, 1, 625);
  h_amptimefail_shaperid->GetXaxis()->SetTitle("Shaper ID");
  h_amptimefail_shaperid->SetOption("LIVE");

  h_ampfail_crateid = new TH1F("ampfail_crateid", "Crate IDs w/ amp inconsistencies", 52, 1, 53);
  h_ampfail_crateid->GetXaxis()->SetTitle("Crate ID (same as ECLCollector ID)");
  h_ampfail_crateid->SetOption("LIVE");

  h_timefail_crateid = new TH1F("timefail_crateid", "Crate IDs w/ time inconsistencies", 52, 1, 53);
  h_timefail_crateid->GetXaxis()->SetTitle("Crate ID (same as ECLCollector ID)");
  h_timefail_crateid->SetOption("LIVE");

  h_amptimefail_crateid = new TH1F("amptimefail_crateid", "Crate IDs w/ time and amp inconsistencies", 52, 1, 53);
  h_amptimefail_crateid->GetXaxis()->SetTitle("Crate ID (same as ECLCollector ID)");
  h_amptimefail_crateid->SetOption("LIVE");

  h_qualityfail_cellid = new TH1F("qualityfail_cellid", "Cell IDs w/ fit qual inconsistencies", 8736, 1, 8737);
  h_qualityfail_cellid->GetXaxis()->SetTitle("Cell ID");
  h_qualityfail_cellid->SetOption("LIVE");

  h_qualityfail_shaperid = new TH1F("qualityfail_shaperid", "Shaper IDs w/ fit qual inconsistencies", 624, 1, 625);
  h_qualityfail_shaperid->GetXaxis()->SetTitle("Shaper ID");
  h_qualityfail_shaperid->SetOption("LIVE");

  h_qualityfail_crateid = new TH1F("qualityfail_crateid", "Crate IDs w/ fit qual inconsistencies", 52, 1, 53);
  h_qualityfail_crateid->GetXaxis()->SetTitle("Crate ID (same as ECLCollector ID)");
  h_qualityfail_crateid->SetOption("LIVE");

  h_fail_shaperid = new TH1F("fail_shaperid", "Shaper IDs w/ inconsistencies", 624, 1, 625);
  h_fail_shaperid->GetXaxis()->SetTitle("Shaper ID");
  h_fail_shaperid->SetOption("LIVE");

  h_fail_crateid = new TH1F("fail_crateid", "Crate IDs w/ inconsistencies", 52, 1, 53);
  h_fail_crateid->GetXaxis()->SetTitle("Crate ID (same as ECLCollector ID)");
  h_fail_crateid->SetOption("LIVE");


  //2D histograms creation.

  if (m_SaveDetailedFitData) {
    h_ampdiff_cellid = new TH2F("ampdiff_cellid", "Amp. diff. (Emulator-Data) for amp inconsistencies",
                                8736, 1, 8737, 239, -262143, 262143);
    h_ampdiff_cellid->GetXaxis()->SetTitle("Cell ID");
    h_ampdiff_cellid->GetYaxis()->SetTitle("Amplitude difference");
    h_ampdiff_cellid->SetOption("LIVE");

    h_timediff_cellid = new TH2F("timediff_cellid", "Time diff. (Emulator-Data) for time inconsistencies",
                                 8736, 1, 8737, 239, -4095, 4095);
    h_timediff_cellid->GetXaxis()->SetTitle("Cell ID");
    h_timediff_cellid->GetYaxis()->SetTitle("Time difference");
    h_timediff_cellid->SetOption("LIVE");

    h_ampdiff_shaperid = new TH2F("ampdiff_shaper", "Amp. diff. (Emulator-Data) "
                                  "for amp inconsistencies vs Shaper Id",
                                  624, 1, 625, 239, -262143, 262143);
    h_ampdiff_shaperid->GetXaxis()->SetTitle("Shaper Id");
    h_ampdiff_shaperid->GetYaxis()->SetTitle("Amplitude difference");
    h_ampdiff_shaperid->SetOption("LIVE");

    h_timediff_shaperid = new TH2F("timediff_shaper", "Time diff. (Emulator-Data) "
                                   "for time inconsistencies vs Shaper Id",
                                   624, 1, 625, 239, -4095, 4095);
    h_timediff_shaperid->GetXaxis()->SetTitle("Shaper Id");
    h_timediff_shaperid->GetYaxis()->SetTitle("Time difference");
    h_timediff_shaperid->SetOption("LIVE");
  }

  h_ampdiff_quality = new TH2F("ampdiff_quality", "Amp. diff. (Emulator-Data) for amp. inconsistencies", 4, 0, 4, 239,
                               -262143, 262143);
  h_ampdiff_quality->GetXaxis()->SetTitle("FPGA fit quality. 0-good, 1-int overflow, 2-low amp, 3-bad chi2");
  h_ampdiff_quality->GetYaxis()->SetTitle("Amplitude difference");
  h_ampdiff_quality->SetOption("LIVE");

  h_timediff_quality = new TH2F("timediff_quality", "Time diff. (Emulator-Data) for time inconsistencies", 4, 0, 4, 239,
                                -4095, 4095);
  h_timediff_quality->GetXaxis()->SetTitle("FPGA fit quality. 0-good, 1-int overflow, 2-low amp, 3-bad chi2");
  h_timediff_quality->GetYaxis()->SetTitle("Time difference");
  h_timediff_quality->SetOption("LIVE");

  h_quality_fit_data = new TH2F("quality_fit_data", "C++ fitter vs FPGA, fit quality inconsistencies", 4, 0, 4, 4, 0, 4);
  h_quality_fit_data->GetXaxis()->SetTitle("C++ fit qual. 0-good,1-int overflow,2-low amp,3-bad chi2");
  h_quality_fit_data->GetYaxis()->SetTitle("FPGA fit qual. 0-good,1-int overflow,2-low amp,3-bad chi2");
  h_quality_fit_data->SetOption("LIVE");

  h_ampflag_qualityfail = new TH2F("ampflag_qualityfail", "Amp flag (0/1) for fit qual inconsistencies", 4, 0, 4, 4, -1,
                                   3);
  h_ampflag_qualityfail->GetXaxis()->SetTitle("FPGA fit quality. 0-good,1-int overflow,2-low amp,3-bad chi2");
  h_ampflag_qualityfail->GetYaxis()->SetTitle("Amp flag (0-amp consistent)");
  h_ampflag_qualityfail->SetOption("LIVE");

  h_timeflag_qualityfail = new TH2F("timeflag_qualityfail", "Time flag (0/1) for fit qual inconsistencies", 4, 0, 4, 4,
                                    -1, 3);
  h_timeflag_qualityfail->GetXaxis()->SetTitle("FPGA fit quality. 0-good,1-int overflow,2-low amp,3-bad chi2");
  h_timeflag_qualityfail->GetYaxis()->SetTitle("Time flag (0-time consistent)");
  h_timeflag_qualityfail->SetOption("LIVE");

  oldDir->cd();
}


void ECLDQMEXTENDEDModule::initialize()
{
  REG_HISTOGRAM;   // required to register histograms to HistoManager

  m_ECLDigits.isRequired();
  m_ECLTrigs.isOptional();
  m_ECLDsps.isOptional();

  if (!mapper.initFromDB()) B2FATAL("ECL DQM logic test FATAL:: Can't initialize eclChannelMapper");

  if (m_InitKey == "DB")  initDspfromDB();
  else if (m_InitKey == "File") initDspfromFile();
  else B2FATAL("ECL DQM logic test FATAL: No way to initialize DSP coeffs!!! Please choose InitKey = DB or InitKey = File");
}

void ECLDQMEXTENDEDModule::callbackCalibration(DBObjPtr<ECLCrystalCalib>& cal, std::vector<short int>& constants)
{
  const std::vector<float> intermediate = cal->getCalibVector();
  constants.resize(intermediate.size());
  for (size_t i = 0; i < constants.size(); i++) constants[i] = (short int)intermediate[i];
}


void ECLDQMEXTENDEDModule::callbackCalibration(const ECLDspData* dspdata,
                                               std::map<std::string, std::vector<short int>>& map1,
                                               std::map<std::string, short int>& map2)
{

  dspdata->getF(map1["F"]);
  dspdata->getF1(map1["F1"]);
  dspdata->getF31(map1["F31"]);
  dspdata->getF32(map1["F32"]);
  dspdata->getF33(map1["F33"]);
  dspdata->getF41(map1["F41"]);
  dspdata->getF43(map1["F43"]);

  map2["k_a"]  = (short int)dspdata->getka();
  map2["k_b"]  = (short int)dspdata->getkb();
  map2["k_c"]  = (short int)dspdata->getkc();
  map2["k_16"] = (short int)dspdata->gety0Startr();
  map2["k_1"]  = (short int)dspdata->getk1();
  map2["k_2"]  = (short int)dspdata->getk2();
  map2["chi_thres"] = dspdata->getchiThresh();
}


int ECLDQMEXTENDEDModule::conversion(int cellID)
{
  int iCrate = mapper.getCrateID(cellID);
  int iShaperPosition = mapper.getShaperPosition(cellID);
  return (iCrate - 1) * 12 + iShaperPosition;

}
const short int* ECLDQMEXTENDEDModule::vectorsplit(const std::vector<short int>& vectorFrom, int iChannel)
{
  size_t size = vectorFrom.size();
  if (size % 16) B2ERROR("ECL DQM logic test error: Split is impossible!" << LogVar("Vector size", size));
  return (vectorFrom.data() + (size / 16) * (iChannel - 1));
}


void ECLDQMEXTENDEDModule::initDspfromDB()
{
  size_t iShaper = 0;

  callbackCalibration(m_calibrationThrA0, v_totalthrA0);
  callbackCalibration(m_calibrationThrAhard, v_totalthrAhard);
  callbackCalibration(m_calibrationThrAskip, v_totalthrAskip);

  for (const auto& dspdata : m_ECLDspDataArray0) { //iCrate = 1, ..., 18
    iShaper++;
    callbackCalibration(&dspdata, map_container_vec[iShaper],
                        map_container_coef[iShaper]);
  }

  for (const auto& dspdata : m_ECLDspDataArray1) { //iCrate = 19, ..., 36
    iShaper++;
    callbackCalibration(&dspdata, map_container_vec[iShaper],
                        map_container_coef[iShaper]);
  }

  for (const auto& dspdata : m_ECLDspDataArray2) { //iCrate = 37, ..., 52
    iShaper++;
    if (iShaper < 529) {
      if (iShaper - (iShaper - 1) / 12 * 12 > 10) continue;
    } else {
      if (iShaper - (iShaper - 1) / 12 * 12 > 8) continue;
    }
    callbackCalibration(&dspdata, map_container_vec[iShaper],
                        map_container_coef[iShaper]);
  }
}

void ECLDQMEXTENDEDModule::initDspfromFile()
{
  const boost::filesystem::path MainDir(m_DSPDirectoryName);
  const boost::filesystem::path RunSubDir(m_RunName);
  const std::regex Filter(".*(crate)([0-9]{2})/.*(dsp)([0-9]{2})(.dat)");
  if (!exists(MainDir / RunSubDir)) B2FATAL("ECL DQM logic test FATAL: Directory w/ DSP files don't exist" << LogVar("Directory",
                                              MainDir / RunSubDir));
  for (boost::filesystem::directory_entry& x : boost::filesystem::recursive_directory_iterator(MainDir / RunSubDir)) {
    if (!std::regex_match(x.path().string(), Filter) || !boost::filesystem::is_regular_file(x.path())) continue;
    int iCrate = atoi(std::regex_replace(x.path().string(), Filter, "$2").c_str());
    int iShaperPosition = atoi(std::regex_replace(x.path().string(), Filter, "$4").c_str());
    int iShaper = (iCrate - 1) * 12 + iShaperPosition;
    if (iCrate > 36 && iCrate < 45) {
      if (iShaperPosition > 10) continue;
    } else if (iCrate > 44) {
      if (iShaperPosition > 8) continue;
    }
    ECLDspData* dspdata = ECLDspUtilities::readEclDsp(x.path().string().c_str(), iShaperPosition - 1);
    callbackCalibration(dspdata, map_container_vec[iShaper],
                        map_container_coef[iShaper]);
    callbackCalibration(m_calibrationThrA0, v_totalthrA0);
    callbackCalibration(m_calibrationThrAhard, v_totalthrAhard);
    callbackCalibration(m_calibrationThrAskip, v_totalthrAskip);
  }
}

void ECLDQMEXTENDEDModule::emulator(int cellID, int trigger_time, std::vector<int> adc_data)
{
  int iShaper = conversion(cellID);
  int iChannelPosition = mapper.getShaperChannel(cellID);

  const auto& map_vec = map_container_vec[iShaper];
  const short int* f    = vectorsplit(map_vec.at("F"),   iChannelPosition);
  const short int* f1   = vectorsplit(map_vec.at("F1"),  iChannelPosition);
  const short int* fg31 = vectorsplit(map_vec.at("F31"), iChannelPosition);
  const short int* fg32 = vectorsplit(map_vec.at("F32"), iChannelPosition);
  const short int* fg33 = vectorsplit(map_vec.at("F33"), iChannelPosition);
  const short int* fg41 = vectorsplit(map_vec.at("F41"), iChannelPosition);
  const short int* fg43 = vectorsplit(map_vec.at("F43"), iChannelPosition);

  const auto& map_coef = map_container_coef[iShaper];
  int k_a = map_coef.at("k_a");
  int k_b = map_coef.at("k_b");
  int k_c = map_coef.at("k_c");
  int k_1 = map_coef.at("k_1");
  int k_2 = map_coef.at("k_2");
  int k_16 = map_coef.at("k_16");
  int chi_thres = map_coef.at("chi_thres");

  int A0 = (int)v_totalthrA0[cellID - 1];
  int Ahard = (int)v_totalthrAhard[cellID - 1];
  int Askip = (int)v_totalthrAskip[cellID - 1];

  int* y = adc_data.data();
  int ttrig2 = trigger_time - 2 * (trigger_time / 8);

  auto result = lftda_(f, f1, fg41, fg43, fg31, fg32, fg33, y, ttrig2, A0,
                       Ahard, Askip, k_a, k_b, k_c, k_16, k_1, k_2, chi_thres,
                       m_adjusted_timing);
  m_AmpFit = result.amp;
  m_TimeFit = result.time;
  m_QualityFit = result.quality;

  if (result.skip_thr || result.hit_thr) m_QualityFit += 4;
}

void ECLDQMEXTENDEDModule::beginRun()
{
  for (int i = 0; i < 4; i++) {
    h_amp_timefail[i]->Reset();
    h_time_ampfail[i]->Reset();
  }
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      h_amp_qualityfail[i][j]->Reset();
      h_time_qualityfail[i][j]->Reset();
    }
  }
  h_ampfail_quality->Reset();
  h_timefail_quality->Reset();
  h_ampfail_cellid->Reset();
  h_timefail_cellid->Reset();
  h_amptimefail_cellid->Reset();
  h_ampfail_shaperid->Reset();
  h_timefail_shaperid->Reset();
  h_amptimefail_shaperid->Reset();
  h_ampfail_crateid->Reset();
  h_timefail_crateid->Reset();
  h_amptimefail_crateid->Reset();
  h_qualityfail_cellid->Reset();
  h_qualityfail_shaperid->Reset();
  h_qualityfail_crateid->Reset();
  if (m_SaveDetailedFitData) {
    h_ampdiff_cellid->Reset();
    h_timediff_cellid->Reset();
    h_ampdiff_shaperid->Reset();
    h_timediff_shaperid->Reset();
  }
  h_ampdiff_quality->Reset();
  h_timediff_quality->Reset();
  h_quality_fit_data->Reset();
  h_ampflag_qualityfail->Reset();
  h_timeflag_qualityfail->Reset();
}

void ECLDQMEXTENDEDModule::event()
{
  if (m_TRGSummary.isValid()) {
    // Skip events for several types of timing sources
    // to reduce CPU time usage on HLT (random trigger, delayed bhahba).
    int timing_type = m_TRGSummary->getTimType();
    for (auto& skipped_timing_type : m_skipEvents) {
      if (timing_type == skipped_timing_type) return;
    }
  }

  int iAmpflag_qualityfail = 0;
  int iTimeflag_qualityfail = 0;

  for (auto& aECLDsp : m_ECLDsps) {
    m_CellId = aECLDsp.getCellId();
    std::vector<int> DspArray = aECLDsp.getDspA();
    if (!m_ECLTrigs.isValid()) B2FATAL("ECL DQM logic test FATAL: Trigger time information is not available");
    for (auto& aECLTrig : m_ECLTrigs) {
      if (aECLTrig.getTrigId() == mapper.getCrateID(m_CellId)) {
        m_TrigTime = aECLTrig.getTimeTrig();
        break;
      }
    }

    emulator(m_CellId, m_TrigTime, DspArray);
    ECLDigit* aECLDigit = aECLDsp.getRelated<ECLDigit>();

    if ((m_AmpFit >= (int)v_totalthrAskip[m_CellId - 1]) && m_QualityFit < 4 && !aECLDigit)
      B2ERROR("ECL DQM logic test error: ECL Digit does not exist for A_emulator > Thr_skip"
              << LogVar("Thr_skip", (int)v_totalthrAskip[m_CellId - 1])
              << LogVar("A_emulator", m_AmpFit)
              << LogVar("Quality_emulator", m_QualityFit));

    if ((m_AmpFit >= (int)v_totalthrAskip[m_CellId - 1]) && aECLDigit) {

      m_AmpData = aECLDigit->getAmp();
      m_TimeData = aECLDigit->getTimeFit();
      m_QualityData = aECLDigit->getQuality();

      if (m_AmpFit != m_AmpData) {
        for (int i = 0; i < 4; i++) if (m_QualityData == i && m_TimeFit == m_TimeData) h_time_ampfail[i]->Fill(m_TimeData);
        h_ampfail_quality->Fill(m_QualityData);
        h_ampfail_cellid->Fill(m_CellId);
        h_ampfail_shaperid->Fill(conversion(m_CellId));
        h_ampfail_crateid->Fill(mapper.getCrateID(m_CellId));
        if (m_SaveDetailedFitData) {
          h_ampdiff_cellid->Fill(m_CellId, m_AmpFit - m_AmpData);
          h_ampdiff_shaperid->Fill(conversion(m_CellId), m_AmpFit - m_AmpData);
        }
        h_ampdiff_quality->Fill(m_QualityData, m_AmpFit - m_AmpData);
      }
      if (m_TimeFit != m_TimeData) {
        for (int i = 0; i < 4; i++) if (m_QualityData == i && m_AmpFit == m_AmpData) h_amp_timefail[i]->Fill(m_AmpData);
        h_timefail_quality->Fill(m_QualityData);
        h_timefail_cellid->Fill(m_CellId);
        h_timefail_shaperid->Fill(conversion(m_CellId));
        h_timefail_crateid->Fill(mapper.getCrateID(m_CellId));
        if (m_SaveDetailedFitData) {
          h_timediff_cellid->Fill(m_CellId, m_TimeFit - m_TimeData);
          h_timediff_shaperid->Fill(conversion(m_CellId), m_TimeFit - m_TimeData);
        }
        h_timediff_quality->Fill(m_QualityData, m_TimeFit - m_TimeData);
      }
      if (m_AmpFit != m_AmpData && m_TimeFit != m_TimeData) {
        h_amptimefail_cellid->Fill(m_CellId);
        h_amptimefail_shaperid->Fill(conversion(m_CellId));
        h_amptimefail_crateid->Fill(mapper.getCrateID(m_CellId));
      }
      if (m_QualityFit != m_QualityData) {
        for (int i = 0; i < 4; i++) {
          for (int j = 0; j < 3; j++) {
            int k = 0;
            if (i == 0) k = j + 1;
            if (i == 1) { if (j == 0) k = j; else k = j + 1; }
            if (i == 2) { if (j == 2) k = j + 1; else k = j; }
            if (i == 3) k = j;
            if (m_QualityFit == i && m_QualityData == k) { h_amp_qualityfail[i][j]->Fill(m_AmpData); h_time_qualityfail[i][j]->Fill(m_TimeData); }
          }
        }
        if (m_AmpFit != m_AmpData) iAmpflag_qualityfail = 1;
        if (m_TimeFit != m_TimeData) iTimeflag_qualityfail = 1;
        h_qualityfail_cellid->Fill(m_CellId);
        h_qualityfail_shaperid->Fill(conversion(m_CellId));
        h_qualityfail_crateid->Fill(mapper.getCrateID(m_CellId));
        h_ampflag_qualityfail->Fill(m_QualityData, iAmpflag_qualityfail);
        h_timeflag_qualityfail->Fill(m_QualityData, iTimeflag_qualityfail);
        h_quality_fit_data->Fill(m_QualityFit, m_QualityData);
      }
      if (m_AmpFit != m_AmpData || m_TimeFit != m_TimeData || m_QualityFit != m_QualityData) {
        h_fail_shaperid->Fill(conversion(m_CellId));
        h_fail_crateid->Fill(mapper.getCrateID(m_CellId));
      }
      h_ampfail_quality->Fill(-1);
      h_timefail_quality->Fill(-1);
    } //aECLDigit
  }  //aECLDsp
} //event


void ECLDQMEXTENDEDModule::endRun()
{
}



void ECLDQMEXTENDEDModule::terminate()
{
}
