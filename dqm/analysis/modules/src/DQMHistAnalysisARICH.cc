/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <dqm/analysis/modules/DQMHistAnalysisARICH.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TClass.h>
#include <TROOT.h>

#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

REG_MODULE(DQMHistAnalysisARICH);

DQMHistAnalysisARICHModule::DQMHistAnalysisARICHModule()
  : DQMHistAnalysisModule()
{
  // set module description (e.g. insert text)
  setDescription("Modify and analyze the data quality histograms of ARICH");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("debug", m_debug, "debug mode", false);
  addParam("alert", m_enableAlert, "Enable color alert", true);
  addParam("hotLimit", m_hotLimit, "Occupancy limit for hot channels", 0.005);
  addParam("badApdOccLimit", m_badApdOccLimit, "Occupancy limit for bad APDs (in fraction of average occupancy)", 0.3);

}

DQMHistAnalysisARICHModule::~DQMHistAnalysisARICHModule()
{
}

void DQMHistAnalysisARICHModule::initialize()
{
  gROOT->cd();

  //definition of new TObjects for modification and analysis
  for (int i = 0; i < 5; i++) {
    m_LineForMB[i] = new TLine();
    m_LineForMB[i]->SetLineStyle(3);
    m_LineForMB[i]->SetLineWidth(1);
    m_LineForMB[i]->SetLineColor(kBlack);
  }

  m_apdHist = new ARICHChannelHist("tmpChHist", "tmpChHist", 2); /**<ARICH TObject to draw hit map for each APD*/
  m_apdPoly = new TH2Poly();
  m_apdPoly->SetName("ARICH/apdHitMap");
  m_apdPoly->SetTitle("# of hits/APD/event");
  m_apdPoly->SetOption("colz");
  m_c_apdHist = new TCanvas("ARICH/c_apdHist");

  addDeltaPar("ARICH", "bitsPerChannel", HistDelta::c_Events,
              1000000, 1); // update each 1M events (from daq histogram)

  addDeltaPar("ARICH", "theta", HistDelta::c_Events,
              100000, 1); // update each 100k events (from daq histogram)

  // addDeltaPar(m_histogramDirectoryName, m_histogramName, HistDelta::c_Entries, 10000, 1); // update each 10000 entries

  registerEpicsPV("ARI:badAPDs", "badAPDs");
  registerEpicsPV("ARI:deadHAPDs", "deadHAPDs");
  registerEpicsPV("ARI:hotChannels", "hotChannels");

  registerEpicsPV("ARI:hotLimit", "hotLimit");
  registerEpicsPV("ARI:badApdOccLimit", "badApdOccLimit");
  registerEpicsPV("ARI:sigBitFracLimit", "sigBitFracLimit");

  B2DEBUG(20, "DQMHistAnalysisARICH: initialized.");
}

void DQMHistAnalysisARICHModule::beginRun()
{

  // read the alarm limits
  double unused = NAN;
  m_sigBitFracLowWarn = 2.0;
  m_sigBitFracLowAlarm = 1.5;
  requestLimitsFromEpicsPVs("sigBitFracLimit", m_sigBitFracLowAlarm, m_sigBitFracLowWarn, unused, unused);

  double hot = getEpicsPV("hotLimit");
  if (hot != NAN)  m_hotLimit = hot;

  double bad = getEpicsPV("badApdOccLimit");
  if (bad != NAN) m_badApdOccLimit = bad;

}

void DQMHistAnalysisARICHModule::event()
{

  //Show alert by empty bins = red and strange entries = yellow
  //Draw lines on mergerHits histogram for shifters to divide sectors
  TH1* m_h_mergerHit = findHist("ARICH/mergerHit");/**<The number of hits in each Merger Boards*/
  m_c_mergerHit = findCanvas("ARICH/c_mergerHit");
  if (m_h_mergerHit != NULL && m_c_mergerHit != NULL) {
    m_c_mergerHit->Clear();
    m_c_mergerHit->cd();
    m_h_mergerHit->SetMinimum(0);
    m_h_mergerHit->Draw("hist");
    gPad->Update();

    int alertMerger = 0;/**<Alert level variable for shifter plot (0:no problem, 1:need to check, 2:contact experts immediately)*/
    double mean = m_h_mergerHit->Integral() / 72;
    for (int i = 0; i < 72; i++) {
      int hit = m_h_mergerHit->GetBinContent(i + 1);
      if ((bool)hit ^ (bool)m_h_mergerHit->GetEntries()) {
        //only if the empty bin is not a masked merger, show alert.
        auto itr = std::find(maskedMergers.begin(), maskedMergers.end(), i + 1);
        if (itr == maskedMergers.end()) {
          alertMerger = 2;
          break;
        }
      }
      if (hit > mean * 100 && alertMerger < 1) alertMerger = 1;
    }
    if (m_enableAlert && m_minStats < m_h_mergerHit->GetEntries()) m_c_mergerHit->SetFillColor(alertColor[alertMerger]);

    //Draw lines divide the sectors
    for (int i = 0; i < 5; i++) {
      m_LineForMB[i]->DrawLine(12 * (i + 1) + 0.5, 0, 12 * (i + 1) + 0.5, gPad->GetUymax());
    }

    m_c_mergerHit->Modified();
  } else {
    B2INFO("Histogram/canvas named mergerHit is not found.");
  }


  //Show alert by the ratio of center 2 bins to side 2bins. <1.5 = red, <2 = yellow
  TH1* m_h_bits = findHist("ARICH/bits");/**<The number of hits in each timing bit*/
  m_c_bits = findCanvas("ARICH/c_bits");
  if (m_h_bits != NULL && m_c_bits != NULL) {
    m_c_bits->Clear();
    m_c_bits->cd();
    m_h_bits->SetMinimum(0);
    m_h_bits->Draw("hist");
    gPad->Update();

    double side = m_h_bits->GetBinContent(2) + m_h_bits->GetBinContent(5);
    double center = m_h_bits->GetBinContent(3) + m_h_bits->GetBinContent(4);
    EStatus status = c_StatusGood;
    if (center / side < m_sigBitFracLowWarn) status = c_StatusWarning;
    if (center / side < m_sigBitFracLowAlarm) status = c_StatusError;
    if (m_enableAlert && m_minStats < m_h_bits->GetEntries()) colorizeCanvas(m_c_bits, status);

    m_c_bits->Modified();
  } else {
    B2INFO("Histogram/canvas named bits is not found.");
  }

  //Show alert by no entry = red and 0 peak = yellow
  TH1* m_h_hitsPerEvent = findHist("ARICH/hitsPerEvent");/**<The number of hits in each triggered event*/
  m_c_hitsPerEvent = findCanvas("ARICH/c_hitsPerEvent");
  int nEvents = 0;
  if (m_h_hitsPerEvent != NULL && m_c_hitsPerEvent  != NULL) {
    m_c_hitsPerEvent->Clear();
    m_c_hitsPerEvent->cd();
    m_h_hitsPerEvent->SetMinimum(0);
    m_h_hitsPerEvent->Draw("hist");
    gPad->Update();
    nEvents = m_h_hitsPerEvent->GetEntries();
    int alertHitsPerEvent = 0;/**<Alert level variable for shifter plot (0:no problem, 1:need to check, 2:contact experts immediately)*/
    double mean = m_h_hitsPerEvent->GetMean();
    if (mean < 1) alertHitsPerEvent = 1;
    double entry = m_h_hitsPerEvent->GetEntries();
    if (entry == 0) alertHitsPerEvent = 2;
    if (m_enableAlert) m_c_hitsPerEvent->SetFillColor(alertColor[alertHitsPerEvent]);

    m_c_hitsPerEvent->Modified();
  } else {
    B2INFO("Histogram/canvas named hitsPerEvent is not found.");
  }

  //Draw 2D hit map of channels and APDs
  TH1* m_h_chHit = findHist("ARICH/chipHit");/**<The number of hits in each chip */
  if (m_h_chHit != NULL) {
    int nevt = 0;
    TH1* htmp = findHist("ARICH/hitsPerEvent");
    if (htmp) nevt = htmp->GetEntries();
    m_apdHist->fillFromTH1(m_h_chHit);
    if (nevt) m_apdHist->Scale(1. / float(nevt));
    m_apdPoly->SetMaximum(0.1);
    m_apdHist->setPoly(m_apdPoly);
    m_apdPoly->SetMinimum(0.0001);
    m_c_apdHist->Clear();
    m_c_apdHist->cd();
    m_apdPoly->Draw("colz");
    m_apdPoly->GetXaxis()->SetTickLength(0);
    m_apdPoly->GetYaxis()->SetTickLength(0);
    gPad->SetLogz();
    m_c_apdHist->Update();
  } else {
    B2INFO("Histogram named chipHit is not found.");
  }

  TH1F* chDigit = (TH1F*)findHist("ARICH/chDigit");
  int nhot = 0;
  double avgOcc = 0;
  if (chDigit != NULL && nEvents != 0) {
    avgOcc = chDigit->GetEntries() / 60480.;
    if (avgOcc > 100.) {
      for (int i = 0; i < chDigit->GetNbinsX(); i++) {
        int nhit = chDigit->GetBinContent(i + 1);
        if ((nhit - 3. * sqrt(nhit)) / float(nEvents) > m_hotLimit) nhot++;
      }
    }
  }
  setEpicsPV("hotChannels", nhot);

  int ndeadHapd = 0;
  TH1F* hapdDigit = (TH1F*)findHist("ARICH/hapdDigit");
  if (hapdDigit != NULL && avgOcc * 144. > 100.) {
    for (int i = 0; i < hapdDigit->GetNbinsX(); i++) {
      if (hapdDigit->GetBinContent(i + 1) == 0) ndeadHapd++;
    }
  }
  setEpicsPV("deadHAPDs", ndeadHapd);

  auto h_theta =  getDelta("ARICH", "theta", 0, false); // change this to false
  auto c_theta = findCanvas("ARICH/c_theta");
  auto h_thetaInt = (TH1F*)findHist("ARICH/theta");
  if (h_theta != NULL && c_theta != NULL && h_thetaInt != NULL) {
    int binmax = h_theta->GetMaximumBin();
    double x = h_theta->GetXaxis()->GetBinCenter(binmax);
    EStatus status = c_StatusGood;
    if (x < 0.3 || x > 0.35) status = c_StatusError;
    c_theta->Clear();
    c_theta->cd();
    h_theta->Scale(h_thetaInt->GetEntries() / double(h_theta->GetEntries()));
    h_thetaInt->Draw();
    h_theta->Draw("same");
    if (m_enableAlert && h_theta->GetEntries() > 10000) colorizeCanvas(c_theta, status);
    c_theta->Modified();
  }


  if (avgOcc * 36. < 100.) {
    setEpicsPV("badAPDs", 0);
    return;
  }

  auto h_bitsPerChannel =  getDelta("ARICH", "bitsPerChannel", 0, true);
  if (h_bitsPerChannel == NULL) return;
  TH1F* apdHits = new TH1F("apdHits", "nSigHits/nevt for all apds", 1680, -0.5, 1679.5);
  double avg = 0.;
  for (int i = 1; i < 60481; i++) {
    int nnoise = h_bitsPerChannel->GetBinContent(1, i) + h_bitsPerChannel->GetBinContent(4, i);
    int nsig = h_bitsPerChannel->GetBinContent(2, i) + h_bitsPerChannel->GetBinContent(3, i) - nnoise;
    double sig2noise = float(nsig) / float(nsig + nnoise);
    if (sig2noise > 0.2) avg += nsig;
    if (i % 36 == 0 && i > 1) {
      apdHits->SetBinContent(i / 36 + 1, avg);
      avg = 0;
    }
  }

  // for convenience I make this independent of the DB payloads... (it will never change)
  double ringApdAvg[7] = {0.};

  const int hapdInRing[7] = {42, 48, 54, 60, 66, 72, 78};

  // average in hapd ring
  for (int i = 1; i < 1681; i++) {
    int hapd = (i - 1) / 4 + 1;
    ringApdAvg[getRing(hapd)] += apdHits->GetBinContent(i);
  }

  for (int i = 0; i < 7; i++) {
    ringApdAvg[i] /= float(4.*hapdInRing[i]);
  }
  int nbadApd = 0;
  for (int i = 0; i < 1680; i++) {
    int ring = getRing(i / 4 + 1);
    int nhits = apdHits->GetBinContent(i + 1);
    if (nhits + 3. * sqrt(nhits) < ringApdAvg[ring] * m_badApdOccLimit) nbadApd++;
  }
  setEpicsPV("badAPDs", nbadApd);

  delete apdHits;

}


int DQMHistAnalysisARICHModule::getRing(int modID)
{
  if (modID <= 42) return 0;
  if (modID <= 90) return 1;
  if (modID <= 144) return 2;
  if (modID <= 204) return 3;
  if (modID <= 270) return 4;
  if (modID <= 342) return 5;
  if (modID <= 420) return 6;
  return -1; // -1 if invalid input
}

void DQMHistAnalysisARICHModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisARICH : endRun called");
}

void DQMHistAnalysisARICHModule::terminate()
{

  B2DEBUG(20, "terminate called");
}
