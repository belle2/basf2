/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ludovico Massaccesi                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <dqm/analysis/modules/DQMHistAnalysisSVDDose.h>
#include <framework/utilities/Utils.h>
#include <TROOT.h>
#include <TText.h>

using namespace std;
using namespace Belle2;

// Utility function
inline double getClockSeconds() { return Utils::getClock() / 1e9; }

REG_MODULE(DQMHistAnalysisSVDDose)

DQMHistAnalysisSVDDoseModule::DQMHistAnalysisSVDDoseModule()
{
  setDescription("Monitoring of SVD Dose with events from Poisson trigger w/o inj. veto. See also SVDDQMDoseModule.");
  // THIS MODULE CAN NOT BE RUN IN PARALLEL
  addParam("pvPrefix", m_pvPrefix, "Prefix for EPICS PVs.", std::string("SVD:DQM:"));
  addParam("useEpics", m_useEpics, "Whether to update EPICS PVs.", true);
  addParam("epicsUpdateSeconds", m_epicsUpdateSeconds,
           "Minimum interval between two successive PV updates (in seconds).", 300.0);
  addParam("pvSuffix", m_pvSuffix, "Suffix for EPICS PVs.", std::string(":OccPois:Avg"));
}

DQMHistAnalysisSVDDoseModule::~DQMHistAnalysisSVDDoseModule()
{
#ifdef _BELLE2_EPICS
  if (m_useEpics && ca_current_context()) ca_context_destroy();
#endif
}

void DQMHistAnalysisSVDDoseModule::initialize()
{
  B2DEBUG(18, "DQMHistAnalysisSVDDose: initialize");

  gROOT->cd(); // Don't know why I need this, but DQMHistAnalysisSVDOnMiraBelle uses it

  m_monObj = getMonitoringObject("svd"); // To write to MiraBelle

  m_c_instOccu.reserve(c_sensorGroups.size());
  m_c_occuLER.reserve(c_sensorGroups.size());
  m_c_occuHER.reserve(c_sensorGroups.size());
  for (const auto& group : c_sensorGroups) {
    TCanvas* c = new TCanvas("svd_instOccupancy_" + group.nameSuffix,
                             "Instantaneous occupancy " + group.titleSuffix,
                             0, 0, 800, 600);
    m_c_instOccu.push_back(c);
    m_monObj->addCanvas(c);

    c = new TCanvas("svd_occuLER_" + group.nameSuffix,
                    "Occupancy vs time since LER inj. " + group.titleSuffix,
                    0, 0, 800, 600);
    m_c_occuLER.push_back(c);
    m_monObj->addCanvas(c);

    c = new TCanvas("svd_occuHER_" + group.nameSuffix,
                    "Occupancy vs time since HER inj. " + group.titleSuffix,
                    0, 0, 800, 600);
    m_c_occuHER.push_back(c);
    m_monObj->addCanvas(c);
  }
  m_h_occuLER.resize(c_sensorGroups.size(), nullptr);
  m_h_occuHER.resize(c_sensorGroups.size(), nullptr);

  // The legend need to be memory-leaked, so we make it once and use it evey time
  m_legend = new TPaveText(0.53, 0.73, 0.68, 0.88, "brNDC");
  m_legend->AddText("LER inj."); ((TText*)m_legend->GetListOfLines()->Last())->SetTextColor(kRed);
  m_legend->AddText("HER inj."); ((TText*)m_legend->GetListOfLines()->Last())->SetTextColor(kAzure);
  m_legend->AddText("No inj."); ((TText*)m_legend->GetListOfLines()->Last())->SetTextColor(kBlack);

#ifdef _BELLE2_EPICS
  if (m_useEpics) {
    if (!ca_current_context())
      SEVCHK(ca_context_create(ca_disable_preemptive_callback), "ca_context_create");
    m_myPVs.resize(c_sensorGroups.size());
    for (unsigned int g = 0; g < c_sensorGroups.size(); g++)
      SEVCHK(ca_create_channel((m_pvPrefix + c_sensorGroups[g].pvMiddle + m_pvSuffix).data(),
                               NULL, NULL, 10, &m_myPVs[g].mychid), "ca_create_channel");
    SEVCHK(ca_pend_io(5.0), "ca_pend_io");
    m_lastPVUpdate = getClockSeconds();
  }
#endif
}

void DQMHistAnalysisSVDDoseModule::event()
{
  // Update PVs ("write" to EPICS)
#ifdef _BELLE2_EPICS
  if (m_useEpics && getClockSeconds() >= m_lastPVUpdate + m_epicsUpdateSeconds) {
    for (unsigned int g = 0; g < c_sensorGroups.size(); g++) {
      const auto& group = c_sensorGroups[g];
      double nHits = 0.0, nEvts = 0.0;
      for (TString dir : {"SVDDoseLERInj", "SVDDoseHERInj", "SVDDoseNoInj"}) {
        auto hHits = findHistT<TH2F>(dir + "/SVDHitsVsTime_" + group.nameSuffix);
        auto hEvts = findHistT<TH2F>(dir + "/SVDEvtsVsTime");
        if (!hHits || !hEvts) {
          B2WARNING("Histograms needed for Average Poisson Occupancy U-side not found.");
          nEvts = 0.0;
          break;
        }
        nHits += hHits->GetEntries();
        nEvts += hEvts->GetEntries();
      }

      B2DEBUG(19, "DQMHistAnalysisSVDDose: PV write"
              << LogVar("group", group.nameSuffix.Data())
              << LogVar("nEvts", nEvts) << LogVar("nHits", nHits));

      auto& pv = m_myPVs[g];
      double delta_nHits = nHits - pv.lastNHits;
      double delta_nEvts = nEvts - pv.lastNEvts;
      double occ = delta_nEvts > 0.0 ? (delta_nHits / delta_nEvts * 100.0 / group.nStrips) : -1.0;
      if (pv.mychid)
        SEVCHK(ca_put(DBR_DOUBLE, pv.mychid, (void*)&occ), "ca_put");

      pv.lastNEvts = nEvts;
      pv.lastNHits = nHits;
    }
    SEVCHK(ca_pend_io(5.0), "ca_pend_io");
    m_lastPVUpdate = getClockSeconds();
  }
#endif

  updateCanvases();
}

void DQMHistAnalysisSVDDoseModule::endRun()
{
  B2DEBUG(18, "DQMHistAnalysisSVDDose: endRun");

  // EPICS: reset the counters used for the delta computation
  for (auto& pv : m_myPVs)
    pv.lastNEvts = pv.lastNHits = 0.0;

  // Write to MiraBelle
  for (unsigned int g = 0; g < c_sensorGroups.size(); g++) {
    const auto& group = c_sensorGroups[g];
    double nHits = 0.0, nEvts = 0.0;
    for (TString dir : {"SVDDoseLERInj", "SVDDoseHERInj", "SVDDoseNoInj"}) {
      auto hHits = findHistT<TH2F>(dir + "/SVDHitsVsTime_" + group.nameSuffix);
      auto hEvts = findHistT<TH2F>(dir + "/SVDEvtsVsTime");
      if (!hHits || !hEvts) {
        B2WARNING("Histograms needed for Average Poisson Occupancy U-side not found.");
        nEvts = 0.0;
        break;
      }
      nHits += hHits->GetEntries();
      nEvts += hEvts->GetEntries();
    }

    B2DEBUG(19, "DQMHistAnalysisSVDDose: MonObj write"
            << LogVar("group", group.nameSuffix.Data())
            << LogVar("nEvts", nEvts) << LogVar("nHits", nHits));

    double occ = nEvts ? (nHits / nEvts * 100.0 / group.nStrips) : -1.0;
    // TODO is this the best name for the MonitoringObject variable?
    TString vName = group.nameSuffix + "OccPoisAvg"; // e.g. L3XXUOccPoisAvg
    m_monObj->setVariable(vName.Data(), occ);
  }

  updateCanvases();
}

void DQMHistAnalysisSVDDoseModule::updateCanvases()
{
  B2DEBUG(18, "DQMHistAnalysisSVDDose: updating canvases");

  for (unsigned int g = 0; g < c_sensorGroups.size(); g++) {
    const auto& group = c_sensorGroups[g];

    auto c = m_c_instOccu[g];
    auto hLER = findHistT<TH1F>("SVDDoseLERInj/SVDInstOccu_" + group.nameSuffix);
    auto hHER = findHistT<TH1F>("SVDDoseHERInj/SVDInstOccu_" + group.nameSuffix);
    auto hNo = findHistT<TH1F>("SVDDoseNoInj/SVDInstOccu_" + group.nameSuffix);
    if (hLER && hHER && hNo) {
      hLER->SetLineColor(kRed);
      hHER->SetLineColor(kAzure);
      hNo->SetLineColor(kBlack);
      carryOverflowOver(hLER);
      carryOverflowOver(hHER);
      carryOverflowOver(hNo);
      c->Clear();
      c->cd(0);
      hNo->Draw("hist"); // hNo usually has the larger maximum by far
      hLER->Draw("hist same");
      hHER->Draw("hist same");
      c->SetLogy();
      m_legend->Draw();
    }

    c = m_c_occuLER[g];
    auto hHits = findHistT<TH2F>("SVDDoseLERInj/SVDHitsVsTime_" + group.nameSuffix);
    auto hEvts = findHistT<TH2F>("SVDDoseLERInj/SVDEvtsVsTime");
    if (hHits && hEvts) {
      if (m_h_occuLER[g]) delete m_h_occuLER[g];
      auto hOccu = m_h_occuLER[g] = divide(hHits, hEvts, 100.0f / group.nStrips);
      hOccu->SetTitle("SVD Occupancy " + group.titleSuffix + " - LER inj."
                      ";Time since last injection [#mus];Time in beam cycle [#mus]"
                      ";Occupancy [%]");
      hOccu->SetMinimum(1e-6);
      hOccu->SetMaximum(10);
      c->Clear();
      c->cd(0);
      c->SetRightMargin(0.16); // For the colorbar
      hOccu->Draw("COLZ");
      c->SetLogz();
    }

    c = m_c_occuHER[g];
    hHits = findHistT<TH2F>("SVDDoseHERInj/SVDHitsVsTime_" + group.nameSuffix);
    hEvts = findHistT<TH2F>("SVDDoseHERInj/SVDEvtsVsTime");
    if (hHits && hEvts) {
      if (m_h_occuHER[g]) delete m_h_occuHER[g];
      auto hOccu = m_h_occuHER[g] = divide(hHits, hEvts, 100.0f / group.nStrips);
      hOccu->SetTitle("SVD Occupancy " + group.titleSuffix + " - HER inj."
                      ";Time since last injection [#mus];Time in beam cycle [#mus]"
                      ";Occupancy [%]");
      hOccu->SetMinimum(1e-6);
      hOccu->SetMaximum(10);
      c->Clear();
      c->cd(0);
      c->SetRightMargin(0.16); // For the colorbar
      hOccu->Draw("COLZ");
      c->SetLogz();
    }
  }
}

TH2F* DQMHistAnalysisSVDDoseModule::divide(TH2F* num, TH2F* den, float scale)
{
  TString name = TString("occu_from_") + num->GetName();
  TH2F* res = (TH2F*)num->Clone(name);
  for (int i = 0; i < num->GetNcells(); i++) {
    float n = num->GetBinContent(i), d = den->GetBinContent(i), e = num->GetBinError(i);
    res->SetBinContent(i, d ? scale * n / d : 0.0f);
    res->SetBinError(i, d ? scale * e / d : 0.0f);
  }
  return res;
}

void DQMHistAnalysisSVDDoseModule::carryOverflowOver(TH1F* h)
{
  int i = h->GetNbinsX();
  float t = h->GetBinContent(i) + h->GetBinContent(i + 1);
  h->SetBinContent(i, t);
  h->SetBinContent(i + 1, 0);
}

const vector<DQMHistAnalysisSVDDoseModule::SensorGroup> DQMHistAnalysisSVDDoseModule::c_sensorGroups = {
  {"L31XU", "L3.1", "L3:1", 768 * 2},
  {"L32XU", "L3.2", "L3:2", 768 * 2},
  {"L3XXU", "L3 avg.", "L3", 768 * 14},
  {"L4XXU", "L4 avg.", "L4", 768 * 30},
  {"L5XXU", "L5 avg.", "L5", 768 * 48},
  {"L6XXU", "L6 avg.", "L6", 768 * 80}
};
