/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//THIS MODULE
#include <dqm/analysis/modules/DQMHistAnalysisECLSummary.h>

//ECL
#include <ecl/geometry/ECLGeometryPar.h>

//ROOT
#include <TStyle.h>
#include <TLine.h>

//boost
#include "boost/format.hpp"

using namespace Belle2;

REG_MODULE(DQMHistAnalysisECLSummary);

DQMHistAnalysisECLSummaryModule::DQMHistAnalysisECLSummaryModule()
  : DQMHistAnalysisModule(),
    m_neighbours_obj("F", 0.1)
{

  B2DEBUG(20, "DQMHistAnalysisECLSummary: Constructor done.");
  addParam("pvPrefix", m_pvPrefix, "Prefix to use for PVs registered by this module",
           std::string("ECL:channels_info:"));
  addParam("useChannelMask", m_useChannelMask,
           "Mask Cell IDs based on information from ECL PVs",
           true);
  addParam("maxDeviationForOccupancy", m_maxDeviationForOccupancy,
           "The higher this parameter, the larger differences in occupancy are allowed for adjacent channels", 0.28);
  addParam("maxDeviationForChi2", m_maxDeviationForChi2,
           "The higher this parameter, the larger differences in the number of hits with bad chi2 are allowed for adjacent channels",
           2.5);
  addParam("onlyIfUpdated", m_onlyIfUpdated, "If true (default), update EPICS PVs only if histograms were updated.",
           true);
}


DQMHistAnalysisECLSummaryModule::~DQMHistAnalysisECLSummaryModule()
{
}

void DQMHistAnalysisECLSummaryModule::initialize()
{
  m_mapper.initFromFile();

  //=== Set up ECL alarms and corresponding PVs

  m_ecl_alarms = {
    {"dead",     "#splitline{dead}{channels}",         1,  1,  1e5},
    {"cold",     "#splitline{cold}{channels}",         1,  2,  1e5},
    {"hot",      "#splitline{hot}{channels}",          25, 50, 1e5},
    {"bad_chi2", "#splitline{bad #chi^{2}}{channels}", 5,  10, 1e6},
    {"bad_fit",  "#splitline{fit incon-}{sistencies}", 5,  10, 0  },
  };

  // Prepare EPICS PVs
  for (auto& alarm : m_ecl_alarms) {
    // By crate
    for (int crate_id = 1; crate_id <= ECL::ECL_CRATES; crate_id++) {
      std::string pv_name = (boost::format("crate%02d:%s") % crate_id % alarm.name).str();
      registerEpicsPV(m_pvPrefix + pv_name, pv_name);
    }
    // Totals
    for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
      std::string pv_name = (boost::format("%s:%s") % ecl_part % alarm.name).str();
      registerEpicsPV(m_pvPrefix + pv_name, pv_name);
    }
    // Masked Cell IDs
    std::string mask_pv_name = (boost::format("mask:%s") % alarm.name).str();
    registerEpicsPV(m_pvPrefix + mask_pv_name, mask_pv_name);
  }

  m_monObj = getMonitoringObject("ecl");

  //=== Set up the histogram to indicate alarm status

  TString title = "#splitline{ECL errors monitoring}";
  title += "{E - Error, W - Warning, L - Low statistics}";
  title += ";ECLCollector ID (same as Crate ID)";
  h_channels_summary = new TH2F("channels_summary", title,
                                ECL::ECL_CRATES, 1, ECL::ECL_CRATES + 1,
                                m_ecl_alarms.size(), 0, m_ecl_alarms.size());

  // Do not show statistics box.
  h_channels_summary->SetStats(0);
  h_channels_summary->SetMinimum(0);
  h_channels_summary->SetMaximum(1);

  //=== Set X axis labels

  for (int i = 1; i <= ECL::ECL_CRATES; i++) {
    h_channels_summary->GetXaxis()->SetBinLabel(i, std::to_string(i).c_str());
  }
  h_channels_summary->LabelsOption("v", "X"); // Rotate X axis labels 90 degrees
  h_channels_summary->SetTickLength(0, "XY");

  //=== Customize offsets and margins

  h_channels_summary->GetXaxis()->SetTitleOffset(0.95);
  h_channels_summary->GetXaxis()->SetTitleSize(0.05);
  h_channels_summary->GetXaxis()->SetLabelSize(0.04);
  h_channels_summary->GetYaxis()->SetLabelSize(0.06);

  c_channels_summary = new TCanvas("ECL/c_channels_summary_analysis");
  c_channels_summary->SetTopMargin(0.10);
  c_channels_summary->SetLeftMargin(0.20);
  c_channels_summary->SetRightMargin(0.005);
  c_channels_summary->SetBottomMargin(0.10);

  //=== Additional canvases/histograms to display which channels have problems
  c_occupancy = new TCanvas("ECL/c_cid_Thr5MeV_overlaid_analysis");
  c_bad_chi2  = new TCanvas("ECL/c_bad_quality_overlaid_analysis");

  h_bad_occ_overlay = new TH1F("bad_occ_overlay", "", ECL::ECL_TOTAL_CHANNELS,
                               1, ECL::ECL_TOTAL_CHANNELS + 1);
  h_bad_occ_overlay->SetLineColor(kRed);
  h_bad_occ_overlay->SetLineStyle(kDashed);
  h_bad_occ_overlay->SetFillColor(kRed);
  h_bad_occ_overlay->SetFillStyle(3007);

  h_bad_chi2_overlay = new TH1F("bad_chi2_overlay", "", ECL::ECL_TOTAL_CHANNELS,
                                1, ECL::ECL_TOTAL_CHANNELS + 1);
  h_bad_chi2_overlay->SetLineColor(kRed);
  h_bad_chi2_overlay->SetLineStyle(kDashed);
  h_bad_chi2_overlay->SetFillColor(kRed);
  h_bad_chi2_overlay->SetFillStyle(3007);

  h_bad_occ_overlay_green = new TH1F("bad_occ_overlay_green", "", ECL::ECL_TOTAL_CHANNELS,
                                     1, ECL::ECL_TOTAL_CHANNELS + 1);
  h_bad_occ_overlay_green->SetLineColor(kGreen);
  h_bad_occ_overlay_green->SetLineStyle(kDotted);
  h_bad_occ_overlay_green->SetFillColor(kGreen);
  h_bad_occ_overlay_green->SetFillStyle(3013);

  h_bad_chi2_overlay_green = new TH1F("bad_chi2_overlay_green", "", ECL::ECL_TOTAL_CHANNELS,
                                      1, ECL::ECL_TOTAL_CHANNELS + 1);
  h_bad_chi2_overlay_green->SetLineColor(kGreen);
  h_bad_chi2_overlay_green->SetLineStyle(kDotted);
  h_bad_chi2_overlay_green->SetFillColor(kGreen);
  h_bad_chi2_overlay_green->SetFillStyle(3013);

  B2DEBUG(20, "DQMHistAnalysisECLSummary: initialized.");
}

void DQMHistAnalysisECLSummaryModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLSummary: beginRun called.");

  //=== Update m_ecl_alarms based on PV limits

  updateAlarmConfig();

  //=== Set Y axis labels from m_ecl_alarms

  for (size_t i = 0; i < m_ecl_alarms.size(); i++) {
    TString label = m_ecl_alarms[i].title;
    label += " #geq ";
    label += m_ecl_alarms[i].alarm_limit;
    h_channels_summary->GetYaxis()->SetBinLabel(i + 1, label);
  }
}

void DQMHistAnalysisECLSummaryModule::event()
{
  TH1* h_total_events = findHist("ECL/event", m_onlyIfUpdated);
  if (!h_total_events) return;
  m_total_events = h_total_events->GetEntries();

  c_channels_summary->Clear();
  h_channels_summary->Reset();

  // [alarm_type][crate_id - 1] -> number of problematic channels in that crate
  std::vector< std::vector<int> > alarm_counts = updateAlarmCounts();

  //=== Set warning and error indicators on the histogram

  const double HISTCOLOR_RED    = 0.9;
  const double HISTCOLOR_GREEN  = 0.45;
  const double HISTCOLOR_ORANGE = 0.65;
  const double HISTCOLOR_GRAY   = 0.00;

  // TODO: Currently, I am explicitly removing
  // TText labels from the histogram. However,
  // normally I would assume that canvas->Clear()
  // should have already removed them. So
  // it might indicate a memory leak or some
  // problem with ROOT->JSON->ROOT conversion
  for (auto& text : m_labels) {
    delete text;
  }
  m_labels.clear();

  //== Set correct warning/error colors for each bin based on alarm thresholds

  bool enough     = false;
  bool warn_flag  = false;
  bool error_flag = false;

  for (size_t alarm_idx = 0; alarm_idx < alarm_counts.size(); alarm_idx++) {
    for (size_t crate = 0; crate < alarm_counts[alarm_idx].size(); crate++) {
      double color;
      char label_text[2] = {0};
      int alarm_limit   = m_ecl_alarms[alarm_idx].alarm_limit;
      int warning_limit = m_ecl_alarms[alarm_idx].warning_limit;

      int alarms = alarm_counts[alarm_idx][crate];

      if (m_total_events < m_ecl_alarms[alarm_idx].required_statistics || alarms < 0) {
        // `alarms < 0` means that we could not get the relevant histogram.
        // We assume that the histogram is not yet available due to low
        // statistics but this is not a guarantee.
        color = HISTCOLOR_GRAY;
        label_text[0] = 'L';
      } else if (alarms >= alarm_limit) {
        color = HISTCOLOR_RED;
        label_text[0] = 'E';
        error_flag = true;
        enough = true;
      } else if (alarms >= warning_limit) {
        color = HISTCOLOR_ORANGE;
        label_text[0] = 'W';
        warn_flag = true;
        enough = true;
      } else {
        color = HISTCOLOR_GREEN;
        enough = true;
      }
      if (label_text[0] == 'E' || label_text[0] == 'W') {
        B2DEBUG(100, "Non-zero (" << alarm_counts[alarm_idx][crate]
                << ") for alarm_idx, crate = " << alarm_idx << ", " <<  crate);
      }
      h_channels_summary->SetBinContent(crate + 1, alarm_idx + 1, color);
      if (label_text[0] != 0) {
        auto text = new TText((crate + 1.5), (alarm_idx + 0.5), label_text);
        if (label_text[0] != 'L') text->SetTextColor(kWhite);
        text->SetTextSize(0.03);
        text->SetTextAlign(22); // centered
        m_labels.push_back(text);
      }
    }
  }

  //=== Draw histogram, labels and grid

  // Customize title
  auto gstyle_title_h = gStyle->GetTitleH();
  auto gstyle_title_x = gStyle->GetTitleX();
  auto gstyle_title_y = gStyle->GetTitleY();
  gStyle->SetTitleH(0.04);
  gStyle->SetTitleX(0.60);
  gStyle->SetTitleY(1.00);

  //=== Set background color based on combined status.
  c_channels_summary->cd();
  colorizeCanvas(c_channels_summary, makeStatus(enough, warn_flag, error_flag));
  c_channels_summary->SetFrameFillColor(DQMHistAnalysisModule::c_ColorTooFew);

  //=== Prepare special style objects to use correct color palette
  //    and use it only for this histogram
  // Based on https://root-forum.cern.ch/t/different-color-palettes-for-different-plots-with-texec/5250/3
  // and https://root.cern/doc/master/multipalette_8C.html

  if (!m_ecl_style) delete m_ecl_style;
  if (!m_default_style) delete m_default_style;

  m_ecl_style     = new TExec("ecl_style",
                              "gStyle->SetPalette(kRainBow);"
                              "if (channels_summary) channels_summary->SetDrawOption(\"col\");");
  h_channels_summary->GetListOfFunctions()->Add(m_ecl_style);
  m_default_style = new TExec("default_style",
                              "gStyle->SetPalette(kBird);");

  //=== Draw with special style
  //    https://root.cern.ch/js/latest/examples.htm#th2_colpal77
  h_channels_summary->Draw("");
  h_channels_summary->Draw("colpal55;same");
  for (auto& text : m_labels) {
    text->Draw();
  }
  drawGrid(h_channels_summary);
  m_default_style->Draw("same");

  //
  c_channels_summary->Modified();
  c_channels_summary->Update();
  c_channels_summary->Draw();

  gStyle->SetTitleH(gstyle_title_h);
  gStyle->SetTitleX(gstyle_title_x);
  gStyle->SetTitleY(gstyle_title_y);
}

void DQMHistAnalysisECLSummaryModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLSummary: endRun called");

  updateAlarmCounts(true);
}


void DQMHistAnalysisECLSummaryModule::terminate()
{
  B2DEBUG(20, "terminate called");
  delete c_channels_summary;
  delete c_occupancy;
  delete c_bad_chi2;
  delete h_bad_occ_overlay;
  delete h_bad_chi2_overlay;
}

std::pair<int, DQMHistAnalysisECLSummaryModule::ECLAlarmType> DQMHistAnalysisECLSummaryModule::getAlarmByName(std::string name)
{
  int index = 0;
  for (auto& alarm_info : m_ecl_alarms) {
    if (alarm_info.name == name) return {index, alarm_info};
    index++;
  }
  B2FATAL("Could not get ECL alarm " + name);
  return {-1, m_ecl_alarms[0]};
}

void DQMHistAnalysisECLSummaryModule::updateAlarmConfig()
{
  //===== Get alarm limits

  for (auto& alarm : m_ecl_alarms) {
    // In the current version, use only first crate PV to get alarm limits
    int crate_id = 1;

    std::string pv_name = (boost::format("crate%02d:%s") % crate_id % alarm.name).str();
    double unused              = 0;
    double upper_warning_limit = 0;
    double upper_alarm_limit   = 0;
    bool accessed = requestLimitsFromEpicsPVs(pv_name, unused, unused, upper_warning_limit, upper_alarm_limit);

    if (!accessed || upper_alarm_limit <= 0 || upper_warning_limit <= 0) {
      B2WARNING("Failed to get alarm limits");
      return;
    }

    alarm.alarm_limit   = upper_alarm_limit;
    alarm.warning_limit = upper_warning_limit;
  }

  //===== Get masked channels

  // alarm_type -> array_of_masked_channels
  static std::map<std::string, dbr_sts_long_array> mask_info;
  if (!getMaskedChannels(mask_info)) {
    B2WARNING("Failed to get arrays of masked channels");
    return;
  }

  for (auto& alarm : m_ecl_alarms) {
    // Update the list of masked channels
    m_mask[alarm.name].clear();
    for (int i = 0; i < c_max_masked_channels; i++) {
      int cell_id = mask_info[alarm.name].value[i];
      // The array of masked channels goes like this: [15, 22, 716, 0, 0, 0, 0, 0]
      // So if 0 (invalid channel ID) is encountered, stop reading further.
      if (cell_id == 0) break;
      m_mask[alarm.name].insert(cell_id);
    }
  }
}

bool DQMHistAnalysisECLSummaryModule::getMaskedChannels(std::map<std::string, dbr_sts_long_array>& mask_info)
{
  for (auto& alarm : m_ecl_alarms) {
    std::string mask_pv_name = (boost::format("mask:%s") % alarm.name).str();
    chid mask_chid = getEpicsPVChID(mask_pv_name);

    if (mask_chid == nullptr) return false;
    // ca_array_get data is NOT VALID here.
    // It will only be valid after ca_pend_io returns ECA_NORMAL
    auto r = ca_array_get(DBR_STS_LONG, c_max_masked_channels, mask_chid, &mask_info[alarm.name]);
    if (r != ECA_NORMAL) return false;
  }

  // ca_pend_io has to be called here,
  // see https://epics.anl.gov/base/R3-14/12-docs/CAref.html#ca_get

  auto r = ca_pend_io(5.0);
  if (r != ECA_NORMAL) return false;

  return true;
}

std::vector< std::vector<int> > DQMHistAnalysisECLSummaryModule::updateAlarmCounts(bool update_mirabelle)
{
  std::vector< std::vector<int> > alarm_counts(
    m_ecl_alarms.size(), std::vector<int>(ECL::ECL_CRATES));

  //=== Get number of fit inconsistencies

  TH1* h_fail_crateid = findHist("ECL/fail_crateid", false);

  const int fit_alarm_index = getAlarmByName("bad_fit").first;
  for (int crate_id = 1; crate_id <= ECL::ECL_CRATES; crate_id++) {
    int errors_count = 0;
    if (h_fail_crateid) {
      errors_count = h_fail_crateid->GetBinContent(crate_id);
    } else {
      errors_count = -1;
    }

    alarm_counts[fit_alarm_index][crate_id - 1] = errors_count;
  }

  // [Cell ID] -> error_bitmask
  std::map<int, int> error_bitmasks;

  //=== Get number of dead/cold/hot channels
  // cppcheck-suppress unassignedVariable
  for (auto& [cell_id, error_bitmask] : getChannelsWithOccupancyProblems()) {
    error_bitmasks[cell_id] |= error_bitmask;
  }
  //=== Get number of channels with bad_chi2
  // cppcheck-suppress unassignedVariable
  for (auto& [cell_id, error_bitmask] : getChannelsWithChi2Problems()) {
    error_bitmasks[cell_id] |= error_bitmask;
  }

  //=== Combine the information

  if (!update_mirabelle) {
    h_bad_chi2_overlay->Reset();
    h_bad_occ_overlay->Reset();
    h_bad_chi2_overlay_green->Reset();
    h_bad_occ_overlay_green->Reset();
  }

  static std::vector<std::string> indices = {"dead", "cold", "hot", "bad_chi2"};
  for (auto& index_name : indices) {
    int alarm_index = getAlarmByName(index_name).first;
    int alarm_bit   = 1 << alarm_index;

    for (auto& [cid, error_bitmask] : error_bitmasks) {
      if ((error_bitmask & alarm_bit) == 0) continue;

      bool masked = (m_mask[index_name].find(cid) != m_mask[index_name].end());

      if (!masked) {
        int crate_id = m_mapper.getCrateID(cid);
        // If channel is not masked, increase the alarm counter.
        alarm_counts[alarm_index][crate_id - 1] += 1;
      }

      if (update_mirabelle) continue;

      // Unless it is the end of run, add problematic channels
      // to the detailed 1D histogram.

      if (index_name == "bad_chi2") {
        if (!masked) h_bad_chi2_overlay->SetBinContent(cid, 1);
        else h_bad_chi2_overlay_green->SetBinContent(cid, 1);
      } else if (index_name == "dead" || index_name == "cold" || index_name == "hot") {
        if (!masked) h_bad_occ_overlay->SetBinContent(cid, 1);
        else h_bad_occ_overlay_green->SetBinContent(cid, 1);
      }
    }

    if (update_mirabelle) continue;

    //=== Prepare the histogram to display the list of all bad channels
    if (index_name == "hot" || index_name == "bad_chi2") {
      TCanvas* current_canvas;
      TH1* main_hist;
      TH1F* overlay_hist;
      TH1F* overlay_hist_green;
      if (index_name == "hot") {
        main_hist          = findHist("ECL/cid_Thr5MeV", m_onlyIfUpdated);
        overlay_hist       = h_bad_occ_overlay;
        overlay_hist_green = h_bad_occ_overlay_green;
        current_canvas     = c_occupancy;
      } else {
        main_hist          = findHist("ECL/bad_quality", m_onlyIfUpdated);
        overlay_hist       = h_bad_chi2_overlay;
        overlay_hist_green = h_bad_chi2_overlay_green;
        current_canvas     = c_bad_chi2;
      }

      if (main_hist && main_hist->GetEntries() > 0) {
        for (auto& overlay : {overlay_hist, overlay_hist_green}) {
          for (int bin_id = 1; bin_id <= ECL::ECL_TOTAL_CHANNELS; bin_id++) {
            if (overlay->GetBinContent(bin_id) == 0) continue;
            // Do not adjust bin height for dead channels
            if (main_hist->GetBinContent(bin_id) == 0) continue;
            overlay->SetBinContent(bin_id, main_hist->GetBinContent(bin_id));
          }
        }

        current_canvas->Clear();
        current_canvas->cd();
        main_hist->Draw("hist");
        overlay_hist->Draw("hist;same");
        overlay_hist_green->Draw("hist;same");
        current_canvas->Modified();
        current_canvas->Update();
        current_canvas->Draw();
      }
    }
  }

  //== Update EPICS PVs or MiraBelle monObjs

  for (size_t alarm_idx = 0; alarm_idx < alarm_counts.size(); alarm_idx++) {
    auto& alarm = m_ecl_alarms[alarm_idx];
    std::map<std::string, int> total;
    // Convert values per crate to totals
    for (size_t crate = 0; crate < alarm_counts[alarm_idx].size(); crate++) {
      int crate_id = crate + 1;
      int value    = alarm_counts[alarm_idx][crate];

      if (!update_mirabelle) {
        std::string pv_name = (boost::format("crate%02d:%s") % crate_id % alarm.name).str();
        setEpicsPV(pv_name, value);
      }

      total["All"] += value;
      if (crate_id <= ECL::ECL_BARREL_CRATES) {
        total["Barrel"] += value;
      } else if (crate_id <= ECL::ECL_BARREL_CRATES + ECL::ECL_FWD_CRATES) {
        total["FWDEndcap"] += value;
      } else {
        total["BWDEndcap"] += value;
      }
    }
    // Export totals
    for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
      std::string pv_name = (boost::format("%s:%s") % ecl_part % alarm.name).str();
      if (update_mirabelle) {
        std::string var_name = pv_name;
        std::replace(var_name.begin(), var_name.end(), ':', '_');
        B2DEBUG(100, var_name << " = " << total[ecl_part]);
        m_monObj->setVariable(var_name, total[ecl_part]);
      } else {
        setEpicsPV(pv_name, total[ecl_part]);
      }
    }
  }

  return alarm_counts;
}

std::map<int, int> DQMHistAnalysisECLSummaryModule::getChannelsWithOccupancyProblems()
{
  static std::vector< std::vector<short> > neighbours(ECL::ECL_TOTAL_CHANNELS);
  if (neighbours[0].size() == 0) {
    for (int cid0 = 0; cid0 < ECL::ECL_TOTAL_CHANNELS; cid0++) {
      // [0]First is the crystal itself.
      // [1]Second is PHI neighbour, phi+1.
      // [2]Third is PHI neighbour, phi-1.
      // [3]Next one (sometimes two) are THETA neighbours in theta-1.
      // [4]Next one (sometimes two) are THETA neighbours in theta+1.
      neighbours[cid0] = m_neighbours_obj.getNeighbours(cid0 + 1);
      // Remove first element (the crystal itself)
      neighbours[cid0].erase(neighbours[cid0].begin());
    }
  }

  TH1* h_occupancy = findHist("ECL/cid_Thr5MeV", m_onlyIfUpdated);
  const double max_deviation = m_maxDeviationForOccupancy;
  return getSuspiciousChannels(h_occupancy, m_total_events, neighbours,
                               max_deviation, true);
}

std::map<int, int> DQMHistAnalysisECLSummaryModule::getChannelsWithChi2Problems()
{
  ECL::ECLGeometryPar* geom = ECL::ECLGeometryPar::Instance();

  static std::vector< std::vector<short> > neighbours(ECL::ECL_TOTAL_CHANNELS);
  if (neighbours[0].size() == 0) {
    for (int cid_center = 1; cid_center <= ECL::ECL_TOTAL_CHANNELS; cid_center++) {
      geom->Mapping(cid_center - 1);
      const int theta_id_center  = geom->GetThetaID();
      int phi_id_center          = geom->GetPhiID();
      const int crystals_in_ring = m_neighbours_obj.getCrystalsPerRing(theta_id_center);
      phi_id_center              = phi_id_center * 144 / crystals_in_ring;
      for (int cid0 = 0; cid0 < 8736; cid0++) {
        if (cid0 == cid_center - 1) continue;
        geom->Mapping(cid0);
        int theta_id = geom->GetThetaID();
        int phi_id   = geom->GetPhiID();
        phi_id       = phi_id * 144 / m_neighbours_obj.getCrystalsPerRing(theta_id);
        if (std::abs(theta_id - theta_id_center) <= 2 &&
            std::abs(phi_id   - phi_id_center)   <= 2) {
          neighbours[cid_center - 1].push_back(cid0);
        }
      }
    }
  }

  TH1* h_bad_chi2 = findHist("ECL/bad_quality", m_onlyIfUpdated);
  const double max_deviation = m_maxDeviationForChi2;
  return getSuspiciousChannels(h_bad_chi2, m_total_events, neighbours,
                               max_deviation, false);
}

std::map<int, int> DQMHistAnalysisECLSummaryModule::getSuspiciousChannels(
  TH1* hist, double total_events,
  const std::vector< std::vector<short> >& neighbours,
  double max_deviation, bool occupancy_histogram)
{
  std::map<int, int> retval;

  if (!hist) return retval;
  //== A bit hacky solution to skip incorrectly
  //   filled histograms
  if (hist->Integral() <= 0) return retval;

  //=== Extract alarm details
  // cppcheck-suppress unassignedVariable
  const auto& [dead_index, dead_alarm] = getAlarmByName("dead");
  // cppcheck-suppress unassignedVariable
  const auto& [cold_index, cold_alarm] = getAlarmByName("cold");
  // cppcheck-suppress unassignedVariable
  const auto& [hot_index,  hot_alarm ] = getAlarmByName("hot");
  // cppcheck-suppress unassignedVariable
  const auto& [chi2_index, chi2_alarm] = getAlarmByName("bad_chi2");

  double min_required_events;

  if (occupancy_histogram) {
    min_required_events = std::min({
      dead_alarm.required_statistics,
      cold_alarm.required_statistics,
      hot_alarm.required_statistics,
    });
  } else {
    min_required_events = chi2_alarm.required_statistics;
  }

  if (total_events < min_required_events) return retval;

  int dead_bit = 1 << dead_index;
  int cold_bit = 1 << cold_index;
  int hot_bit  = 1 << hot_index;
  int chi2_bit = 1 << chi2_index;

  // == Search for dead channels

  if (occupancy_histogram) {
    // This indicates that DQMHistAnalysisECL module is not included in the path
    bool not_normalized = (findCanvas("ECL/c_cid_Thr5MeV_analysis") == nullptr);
    if (total_events >= dead_alarm.required_statistics) {
      double min_occupancy;
      if (getRunType() == "null") {
        // For null runs, occupancy should be higher than 0.0001%
        min_occupancy = 1e-6;
      } else {
        // For cosmic runs, occupancy should be higher than 0.01%
        min_occupancy = 1e-4;
        // (for physics runs, as opposed to cosmics, this can actually be set to higher value)
      }
      if (not_normalized) {
        // The histogram is not normalized, multiply the threshold by evt count
        min_occupancy *= total_events;
      }
      for (int cid = 1; cid <= ECL::ECL_TOTAL_CHANNELS; cid++) {
        if (hist->GetBinContent(cid) > min_occupancy) continue;
        retval[cid] |= dead_bit;
      }
    }
    if (total_events >= hot_alarm.required_statistics) {
      // Number of hits with E > threshold should be less than 70%
      double max_occupancy = 0.7;
      if (not_normalized) {
        // The histogram is not normalized, multiply the threshold by evt count
        max_occupancy *= total_events;
      }
      for (int cid = 1; cid <= ECL::ECL_TOTAL_CHANNELS; cid++) {
        if (hist->GetBinContent(cid) < max_occupancy) continue;
        retval[cid] |= hot_bit;
      }
    }
  }

  // == Search for cold and hot channels (or channels with bad chi2)

  for (int cid = 1; cid <= ECL::ECL_TOTAL_CHANNELS; cid++) {
    double actual_value = hist->GetBinContent(cid);

    std::vector<short> neighb = neighbours[cid - 1];
    std::multiset<double> values_sorted;
    for (auto& neighbour_cid : neighb) {
      values_sorted.insert(hist->GetBinContent(neighbour_cid));
    }

    double expected_value;
    if (occupancy_histogram) {
      // Use "lower 25%" value:
      expected_value = *std::next(values_sorted.begin(), 1 * neighb.size() / 4);
    } else {
      // Use "upper ~75%" value (48 is the expected number of neighbours)
      expected_value = *std::next(values_sorted.begin(), 35 * neighb.size() / 48);
    }
    double deviation = std::abs((actual_value - expected_value) /
                                expected_value);

    if (!occupancy_histogram) {
      // Min occupancy for high-energy (> 1GeV) hits
      double min_occupancy = 1.41e-5;
      if (findCanvas("ECL/c_bad_quality_analysis") == nullptr) {
        // The histogram is not normalized, multiply the threshold by evt count
        min_occupancy *= total_events;
      }

      if (actual_value < min_occupancy) continue;
    }

    if (deviation < max_deviation) continue;

    if (occupancy_histogram) {
      if (actual_value <     expected_value) retval[cid] |= cold_bit;
      if (actual_value > 2 * expected_value) retval[cid] |= hot_bit;
    } else {
      if (actual_value > expected_value) retval[cid] |= chi2_bit;
    }
  }

  return retval;
}

void DQMHistAnalysisECLSummaryModule::drawGrid(TH2* hist)
{
  static std::map<std::string, std::vector<TLine*> > lines;
  std::string name = hist->GetName();
  if (lines[name].empty()) {
    int x_min = hist->GetXaxis()->GetXmin();
    int x_max = hist->GetXaxis()->GetXmax();
    int y_min = hist->GetYaxis()->GetXmin();
    int y_max = hist->GetYaxis()->GetXmax();
    for (int x = x_min + 1; x < x_max; x++) {
      auto l = new TLine(x, 0, x, 5);
      l->SetLineStyle(kDashed);
      lines[name].push_back(l);
    }
    for (int y = y_min + 1; y < y_max; y++) {
      auto l = new TLine(1, y, ECL::ECL_CRATES + 1, y);
      l->SetLineStyle(kDashed);
      lines[name].push_back(l);
    }
  }
  for (auto line : lines[name]) {
    line->Draw();
  }
}

