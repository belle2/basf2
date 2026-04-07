/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/calibration/CDCdEdx/CDCDedxCosineAlgorithm.h>

#include <TF1.h>
#include <TLine.h>
#include <TCanvas.h>
#include <TH1I.h>
#include <vector>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxCosineAlgorithm::CDCDedxCosineAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  isMethodSep(true),
  isMakePlots(true),
  isMergePayload(true),
  m_sigLim(2.5),
  m_cosBin(100),
  m_cosMin(-1.0),
  m_cosMax(1.0),
  m_dedxBin(250),
  m_dedxMin(0.0),
  m_dedxMax(5.0),
  m_suffix("")

{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx electron cos(theta) dependence");

}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxCosineAlgorithm::calibrate()
{

  getExpRunInfo();

  if (!m_DBCosineCor.isValid())
    B2FATAL("There is no valid previous payload for CDCDedxCosineCor");

  B2INFO("Preparing dE/dx calibration for CDC dE/dx electron saturation");

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (!ttree) {
    B2ERROR("Input tree 'tree' not found");
    return c_Failure;
  }
  if (ttree->GetEntries() < 100)return c_NotEnoughData;

  double dedx, costh; int charge;
  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("costh", &costh);
  ttree->SetBranchAddress("charge", &charge);


  // make histograms to store dE/dx values in bins of cos(theta)
  // bin size can be arbitrary, but for now just make uniform bins
  std::vector<TH1D*>  hDedxCos_neg, hDedxCos_pos, hDedxCos_all;

  const double binW = (m_cosMax - m_cosMin) / m_cosBin;

  defineHisto(hDedxCos_neg, "neg", "e-");
  defineHisto(hDedxCos_pos, "pos", "e+");
  defineHisto(hDedxCos_all, "all", "e-,e+");

  // fill histograms, bin size may be arbitrary
  TH1D* hCosth_neg = defineCosthHist("neg");
  TH1D* hCosth_pos = defineCosthHist("pos");
  TH1D* hCosth_all = defineCosthHist("all");

  for (int i = 0; i < ttree->GetEntries(); ++i) {

    ttree->GetEvent(i);

    //if track is a junk
    if (dedx <= 0 || charge == 0) continue;

    //if track is in CDC accpetance (though it is inbuilt in collector module)
    if (costh < TMath::Cos(150 * TMath::DegToRad()) || costh > TMath::Cos(17 * TMath::DegToRad())) continue;

    int bin = int((costh - m_cosMin) / binW);
    if (bin < 0 || bin >= static_cast<int>(m_cosBin)) continue;

    if (isMethodSep) {
      if (charge < 0) {
        hCosth_neg->Fill(costh);
        hDedxCos_neg[bin]->Fill(dedx);
      } else if (charge > 0) {
        hCosth_pos->Fill(costh);
        hDedxCos_pos[bin]->Fill(dedx);
      }
    } else {
      hCosth_all->Fill(costh);
      hDedxCos_all[bin]->Fill(dedx);
    }
  }

  // fit histograms to get gains in bins of cos(theta)
  std::vector<double> cosine;

  std::vector<std::vector<double>> dedxAll(4);
  std::vector<std::vector<double>> dedxNeg(4);
  std::vector<std::vector<double>> dedxPos(4);

  for (unsigned int i = 0; i < m_cosBin; ++i) {

    double meanDedx = 1.0; //This is what we need for calibration
    double meanDedxErr = 0.0;

    if (!isMethodSep) {
      FitValues fitAll = fitHistogram(hDedxCos_all[i]);

      meanDedx = fitAll.mean;

      dedxAll[0].push_back(fitAll.mean);
      dedxAll[1].push_back(fitAll.meanErr);
      dedxAll[2].push_back(fitAll.sigma);
      dedxAll[3].push_back(fitAll.sigmaErr);

    } else {

      //Fit electron dE/dx in cos bins
      FitValues fitNeg = fitHistogram(hDedxCos_neg[i]);

      //Fit positron dE/dx in cos bins
      FitValues fitPos = fitHistogram(hDedxCos_pos[i]);

      if (fitPos.status != "FitOK" && fitNeg.status == "FitOK") {
        fitPos.mean = fitNeg.mean;
        hDedxCos_pos[i]->SetTitle(Form("%s, mean (manual) = elec left", hDedxCos_pos[i]->GetTitle()));
      } else if (fitNeg.status != "FitOK" && fitPos.status == "FitOK") {
        fitNeg.mean = fitPos.mean;
        hDedxCos_neg[i]->SetTitle(Form("%s, mean (manual) = posi right", hDedxCos_neg[i]->GetTitle()));
      } else if (fitNeg.status != "FitOK" && fitPos.status != "FitOK") {
        fitNeg.mean = 1.0;
        fitPos.mean = 1.0;
      }

      dedxNeg[0].push_back(fitNeg.mean);
      dedxNeg[1].push_back(fitNeg.meanErr);
      dedxNeg[2].push_back(fitNeg.sigma);
      dedxNeg[3].push_back(fitNeg.sigmaErr);

      dedxPos[0].push_back(fitPos.mean);
      dedxPos[1].push_back(fitPos.meanErr);
      dedxPos[2].push_back(fitPos.sigma);
      dedxPos[3].push_back(fitPos.sigmaErr);

      meanDedx = 0.5 * (fitNeg.mean + fitPos.mean);
      if (meanDedx <= 0.0) meanDedx = 1.0;

      meanDedxErr = 0.5 * TMath::Sqrt(fitNeg.meanErr * fitNeg.meanErr +
                                      fitPos.meanErr * fitPos.meanErr);

      dedxAll[0].push_back(meanDedx);
      dedxAll[1].push_back(meanDedxErr);

    }

    cosine.push_back(meanDedx);
  }

  createPayload(cosine);


  if (isMakePlots) {

    //1. dE/dx dist. for cosine bins
    plotdedxHist(hDedxCos_all, hDedxCos_neg, hDedxCos_pos);

    //4. costh distribution
    plotCosThetaDist(hCosth_all, hCosth_pos, hCosth_neg);

    plotFitResults(dedxAll, dedxNeg, dedxPos);

    //7. plot statistics related plots here
    plotEventStats();

    //6. draw the final constants
    plotConstants();
  }

  m_suffix.clear();

  return c_OK;
}

//--------------------------------------------------
void CDCDedxCosineAlgorithm::getExpRunInfo()
{

  int cruns = 0;
  for (auto expRun : getRunList()) {
    if (cruns == 0) B2INFO("CDCDedxCosineCor: start exp " << expRun.first << " and run " << expRun.second << "");
    cruns++;
  }

  const auto erStart = getRunList()[0];
  int estart = erStart.first;
  int rstart = erStart.second;

  const auto erEnd = getRunList()[cruns - 1];
  int rend = erEnd.second;

  updateDBObjPtrs(1, rstart, estart);

  if (m_suffix.length() > 0) m_suffix = Form("%s_e%d_r%dr%d", m_suffix.data(), estart, rstart, rend);
  else  m_suffix = Form("e%d_r%dr%d", estart, rstart, rend);
}

//--------------------------------------------------
void CDCDedxCosineAlgorithm::defineHisto(std::vector<TH1D*>& hdedx, const std::string& tag,
                                         const std::string& chargeLabel)
{

  const double binW = (m_cosMax - m_cosMin) / m_cosBin;

  hdedx.reserve(m_cosBin);

  for (unsigned int i = 0; i < m_cosBin; ++i) {
    double coslow = i * binW + m_cosMin;
    double coshigh = coslow + binW;

    hdedx.push_back(new TH1D(Form("hDedxCos_%s_bin%d_%s", tag.c_str(), i, m_suffix.data()), "", m_dedxBin, m_dedxMin, m_dedxMax));

    hdedx[i]->SetTitle(Form("dE/dx dist (%s) in costh (%0.02f, %0.02f);dE/dx (no had sat, for %s);Entries", chargeLabel.c_str(), coslow,
                            coshigh, chargeLabel.c_str()));

  }
}

//--------------------------------------------------
TH1D* CDCDedxCosineAlgorithm::defineCosthHist(const std::string& tag)
{

  TH1D* hist = new TH1D(Form("hCosth_%s_%s", tag.c_str(), m_suffix.data()), " ", m_cosBin, m_cosMin, m_cosMax);
  hist->SetTitle("cos(#theta) dist (e- and e+); cos(#theta); Entries");

  return hist;
}

//----------------------------------------
CDCDedxCosineAlgorithm::FitValues CDCDedxCosineAlgorithm::fitHistogram(TH1D*& hist)
{
  FitValues fitValues;

  fitGaussianWithRange(hist, fitValues.status);

  hist->SetFillColorAlpha(kAzure + 1, 0.30);

  if (fitValues.status == "FitOK") {
    TF1* fitFunc = hist->GetFunction("gaus");
    if (fitFunc) {
      fitValues.mean = fitFunc->GetParameter(1);
      fitValues.meanErr = fitFunc->GetParError(1);
      fitValues.sigma = fitFunc->GetParameter(2);
      fitValues.sigmaErr = fitFunc->GetParError(2);

      std::string fitSummary = Form("#mu_{fit}: %0.03f #pm %0.03f, #sigma_{fit}: %0.03f",
                                    fitValues.mean, fitValues.meanErr, fitValues.sigma);

      hist->SetTitle(Form("%s, %s", hist->GetTitle(), fitSummary.data()));
    }
  }

  return fitValues;
}

//--------------------------------------------------
void CDCDedxCosineAlgorithm::fitGaussianWithRange(TH1D*& temphist, TString& status)
{
  if (temphist->Integral() < 2000) { //atleast 1k bhabha events
    B2INFO(Form("\tThis hist (%s) have insufficient entries to perform fit (%0.03f)", temphist->GetName(), temphist->Integral()));
    status = "LowStats";
    return;
  } else {
    temphist->GetXaxis()->SetRange(temphist->FindFirstBinAbove(0, 1), temphist->FindLastBinAbove(0, 1));
    int fs = temphist->Fit("gaus", "QR");
    if (fs != 0) {
      B2INFO(Form("\tFit (round 1) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
      status = "FitFailed";
      return;
    } else {
      double meanDedx = temphist->GetFunction("gaus")->GetParameter(1);
      double width = temphist->GetFunction("gaus")->GetParameter(2);
      temphist->GetXaxis()->SetRangeUser(meanDedx - 5.0 * width, meanDedx + 5.0 * width);
      fs = temphist->Fit("gaus", "QR", "", meanDedx - m_sigLim * width, meanDedx + m_sigLim * width);
      if (fs != 0) {
        B2INFO(Form("\tFit (round 2) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
        status = "FitFailed";
        return;
      } else {
        temphist->GetXaxis()->SetRangeUser(meanDedx - 5.0 * width, meanDedx + 5.0 * width);
        B2INFO(Form("\tFit for hist (%s) sucessfull (status = %d)", temphist->GetName(), fs));
        status = "FitOK";
      }
    }
  }
}

//--------------------------------------------------
void CDCDedxCosineAlgorithm::createPayload(std::vector<double> cosine)
{
  m_coscors.resize(m_kNGroups);

  for (unsigned int il = 0; il < m_kNGroups; il++) {

    unsigned int nbins = m_DBCosineCor->getSize(getRepresentativeLayer(il));

    if (nbins != m_cosBin)
      B2ERROR("merging failed because of unmatch bins (old "
              << nbins << " new " << m_cosBin << ")");

    m_coscors[il].reserve(nbins);

    for (unsigned int ibin = 0; ibin < nbins; ibin++) {

      double value = cosine[ibin];

      if (isMergePayload) {
        double prev = m_DBCosineCor->getMean(getRepresentativeLayer(il), ibin);

        value *= prev;

        B2INFO("Cosine Corr for " << m_label[il]
               << " Bin # " << ibin
               << ", Previous = " << prev
               << ", Relative = " << cosine[ibin]
               << ", Merged = " << value);
      }

      m_coscors[il].push_back(value);
    }
  }

  //Saving constants
  B2INFO("dE/dx calibration done for CDC dE/dx electron saturation");

  std::vector<unsigned int> layerToGroup(56);

  for (unsigned int layer = 0; layer < 56; layer++) {
    if (layer < 8) layerToGroup[layer] = 0;        // SL0
    else if (layer < 14) layerToGroup[layer] = 1;  // SL1
    else layerToGroup[layer] = 2;                  // SL2-8
  }

  CDCDedxCosineCor* gain = new CDCDedxCosineCor(m_coscors, layerToGroup);
  saveCalibration(gain, "CDCDedxCosineCor");
}

//--------------------------------------------------
void CDCDedxCosineAlgorithm::plotdedxHist(std::vector<TH1D*>& hDedxCos_all,
                                          std::vector<TH1D*>& hDedxCos_neg,
                                          std::vector<TH1D*>& hDedxCos_pos)
{

  TCanvas ctmp("tmp", "tmp", 1200, 1200);
  int nx = isMethodSep ? 2 : 2;
  int ny = isMethodSep ? 1 : 2;
  unsigned int nPads = nx * ny;
  if (isMethodSep) ctmp.SetCanvasSize(1200, 600);
  ctmp.Divide(nx, ny);
  std::stringstream psname;

  psname << Form("cdcdedx_coscorr_dedx_%s.pdf[",  m_suffix.data());
  ctmp.Print(psname.str().c_str());
  psname.str("");
  psname << Form("cdcdedx_coscorr_dedx_%s.pdf",  m_suffix.data());

  for (unsigned int ic = 0; ic < m_cosBin; ic++) {
    if (!isMethodSep) {
      ctmp.cd(ic % nPads + 1);
      hDedxCos_all[ic]->SetStats(0);
      hDedxCos_all[ic]->SetFillColorAlpha(kYellow, 0.25);
      hDedxCos_all[ic]->DrawClone("hist");

      if (ic % nPads == nPads - 1 || ic == m_cosBin - 1) {
        ctmp.Print(psname.str().c_str());
        ctmp.Clear();
        ctmp.Divide(nx, ny);
      }
    } else {

      // left: electron
      ctmp.cd(1);
      hDedxCos_neg[ic]->SetFillColorAlpha(kRed, 0.25);
      hDedxCos_neg[ic]->DrawClone("hist");


      // right: positron
      ctmp.cd(2);
      hDedxCos_pos[ic]->SetFillColorAlpha(kBlue, 0.25);
      hDedxCos_pos[ic]->DrawClone("hist");

      ctmp.Print(psname.str().c_str());
      ctmp.Clear();
      ctmp.Divide(nx, ny);

    }

  }
  psname.str("");
  psname << Form("cdcdedx_coscorr_dedx_%s.pdf]",  m_suffix.data());
  ctmp.Print(psname.str().c_str());
}

//--------------------------------------------------
void CDCDedxCosineAlgorithm::plotCosThetaDist(TH1D* hCosth_all, TH1D* hCosth_pos, TH1D* hCosth_neg)
{

  TCanvas ceadist("ceadist", "Cosine distributions", 800, 600);
  ceadist.cd();


  // If method separation, overlay pos/neg
  if (isMethodSep) {
    TLegend* leg = new TLegend(0.6, 0.7, 0.8, 0.9);

    if (hCosth_neg) {
      hCosth_neg->SetLineColor(kRed);
      hCosth_neg->SetFillColorAlpha(kYellow, 0.55);
      hCosth_neg->SetStats(0);
      hCosth_neg->Draw("hist");
      leg->AddEntry(hCosth_neg, "neg", "f");
    }
    if (hCosth_pos) {
      hCosth_pos->SetLineColor(kBlue);
      hCosth_pos->SetFillColorAlpha(kGray, 0.35);
      hCosth_pos->SetStats(0);
      hCosth_pos->Draw("hist same");
      leg->AddEntry(hCosth_pos, "pos", "f");
    }
    leg->Draw();

  } else {
    // Always draw ALL first
    if (hCosth_all) {
      hCosth_all->SetFillColorAlpha(kGray, 0.25);
      hCosth_all->SetLineColor(kGray);
      hCosth_all->SetStats(0);
      hCosth_all->Draw("hist");
    }
  }

  ceadist.SaveAs(Form("cdcdedx_coscorr_cosine_%s.pdf", m_suffix.data()));
  ceadist.SaveAs(Form("cdcdedx_coscorr_cosine_%s.root", m_suffix.data()));
}


//--------------------------------------------------
void CDCDedxCosineAlgorithm::plotFitResults(const std::vector<std::vector<double>>& dedxAll,
                                            const std::vector<std::vector<double>>& dedxNeg,
                                            const std::vector<std::vector<double>>& dedxPos)
{
  // Fill histograms

  TH1D* hMean_all = new TH1D("hMean_all", "mean vs cos#theta;cos#theta;mean",  m_cosBin, m_cosMin, m_cosMax);

  TH1D* hMean_el  = new TH1D("hMean_el",  "mean (e-);cos#theta;mean",  m_cosBin, m_cosMin, m_cosMax);

  TH1D* hMean_po  = new TH1D("hMean_po",  "mean (e+);cos#theta;mean",  m_cosBin, m_cosMin, m_cosMax);


  TH1D* hSig_all = new TH1D("hSig_all", "sigma vs cos#theta;cos#theta;#sigma", m_cosBin, m_cosMin, m_cosMax);

  TH1D* hSig_el  = new TH1D("hSig_el",  "sigma (e-);cos#theta;#sigma", m_cosBin, m_cosMin, m_cosMax);

  TH1D* hSig_po  = new TH1D("hSig_po",  "sigma (e+);cos#theta;#sigma", m_cosBin, m_cosMin, m_cosMax);


  for (unsigned int i = 0; i < m_cosBin; i++) {
    hMean_all->SetBinContent(i + 1, dedxAll[0][i]);
    hMean_all->SetBinError(i + 1, dedxAll[1][i]);

    if (isMethodSep) {
      hMean_el->SetBinContent(i + 1, dedxNeg[0][i]);
      hMean_el->SetBinError(i + 1, dedxNeg[1][i]);
      hSig_el->SetBinContent(i + 1, dedxNeg[2][i]);
      hSig_el->SetBinError(i + 1, dedxNeg[3][i]);

      hMean_po->SetBinContent(i + 1, dedxPos[0][i]);
      hMean_po->SetBinError(i + 1, dedxPos[1][i]);
      hSig_po->SetBinContent(i + 1, dedxPos[2][i]);
      hSig_po->SetBinError(i + 1, dedxPos[3][i]);
    } else {
      hSig_all->SetBinContent(i + 1, dedxAll[2][i]);
      hSig_all->SetBinError(i + 1, dedxAll[3][i]);
    }

  }

  TCanvas* ctmp = new TCanvas("c_fit", "Mean & Sigma", 1000, 500);
  ctmp->Divide(2, 1);
  ctmp->cd(1);
  gPad->SetGridy(1);

  setHist(hMean_all, kBlack, "dedx rel(#mu_{fit}) for e- and e+ combined", 0.97, 1.04);

  if (isMethodSep) {

    setHist(hMean_el, kRed, "comparison of dedx #mu_{fit}^{rel}", 0.96, 1.04);
    setHist(hMean_po, kBlue, "", 0.96, 1.04);

    hMean_el->Draw("E1");
    hMean_po->Draw("E1 same");
    hMean_all->Draw("E1 same");

  } else {
    hMean_all->Draw("E1");
  }

  ctmp->cd(2);
  gPad->SetGridy(1);

  setHist(hSig_all, kBlack, "dedx rel(#sigma_{fit}) for e- and e+ combined", 0.12, 0.4);

  if (isMethodSep) {

    setHist(hSig_el, kRed, "comparison of dedx #sigma_{fit}^{rel}", 0.12, 0.4, 24);
    setHist(hSig_po, kBlue, "", 0.12, 0.4, 25);

    hSig_el->Draw("E1");
    hSig_po->Draw("E1 same");

  } else {
    hSig_all->Draw("E1");
  }
  B2INFO("Plotting finished ");


  ctmp->SaveAs(Form("cdcdedx_coscorr_fit_%s.pdf", m_suffix.data()));
  delete hMean_all;
  delete hMean_el;
  delete hMean_po;
  delete hSig_all;
  delete hSig_el;
  delete hSig_po;
  delete ctmp;
}

//--------------------------------------------------
void CDCDedxCosineAlgorithm::plotConstants()
{

  for (int il = 0; il < m_kNGroups; il++) {

    unsigned int nbins = m_DBCosineCor->getSize(getRepresentativeLayer(il));

    // --- Create histograms ---
    TH1D* hnew = new TH1D(Form("hnew_%s", m_label[il].data()), Form("Final const: %s;cos(#theta);dedx #mu_{fit}", m_label[il].data()),
                          m_cosBin, m_cosMin,
                          m_cosMax);

    TH1D* hold = new TH1D(Form("hold_%s", m_label[il].data()), Form("Final const: %s;cos(#theta);dedx #mu_{fit}", m_label[il].data()),
                          m_cosBin, m_cosMin,
                          m_cosMax);

    for (unsigned int iea = 0; iea < nbins; iea++) {
      double oldv = m_DBCosineCor->getMean(getRepresentativeLayer(il), iea);
      double newv = m_coscors[il][iea];

      hold->SetBinContent(iea + 1, oldv);
      hnew->SetBinContent(iea + 1, newv);
    }

    // --- Ratio ---
    TH1D* hratio = (TH1D*)hnew->Clone(Form("hratio_%s", m_label[il].data()));
    hratio->Divide(hold);

    TCanvas c(Form("c_%s", m_label[il].data()), Form("Final constants %s", m_label[il].data()), 1000, 500);
    c.Divide(2, 1);
    c.cd(1);
    gPad->SetGridy(1);
    gPad->SetGridx(1);

    hnew->SetLineColor(kBlack);
    hnew->SetStats(0);
    hold->SetLineColor(kRed);
    hold->SetStats(0);

    double min = std::min(hnew->GetMinimum(), hold->GetMinimum());
    double max = std::max(hnew->GetMaximum(), hold->GetMaximum());
    hnew->GetYaxis()->SetRangeUser(min * 0.95, max * 1.05);

    hnew->Draw("hist");
    hold->Draw("hist same");

    auto leg = new TLegend(0.6, 0.75, 0.85, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(hnew, "New", "l");
    leg->AddEntry(hold, "Old", "l");
    leg->Draw();

    c.cd(2);
    gPad->SetGridy(1);
    hratio->SetLineColor(kBlue);
    hratio->SetStats(0);
    hratio->SetTitle(Form("Ratio: new/old, %s;cos(#theta); New / Old", m_label[il].data()));
    hratio->GetYaxis()->SetRangeUser(0.2, 1.2);
    hratio->Draw("hist");

    TLine* line = new TLine(m_cosMin, 1.0, m_cosMax, 1.0);
    line->SetLineStyle(2);
    line->Draw();

    c.SaveAs(Form("cdcdedx_coscorr_fconsts_%s_%s.pdf", m_label[il].data(), m_suffix.data()));
    c.SaveAs(Form("cdcdedx_coscorr_fconsts_%s_%s.root", m_label[il].data(), m_suffix.data()));

    // cleanup
    delete hnew;
    delete hold;
    delete hratio;
    delete line;
  }
}

//------------------------------------
void CDCDedxCosineAlgorithm::plotEventStats()
{

  TCanvas cstats("cstats", "cstats", 1000, 500);
  cstats.SetBatch(kTRUE);
  cstats.Divide(2, 1);

  cstats.cd(1);
  auto hestats = getObjectPtr<TH1I>("hestats");
  if (hestats) {
    hestats->SetName(Form("hestats_%s", m_suffix.data()));
    hestats->SetStats(0);
    hestats->DrawCopy("");
  }

  cstats.cd(2);
  auto htstats = getObjectPtr<TH1I>("htstats");
  if (htstats) {
    htstats->SetName(Form("htstats_%s", m_suffix.data()));
    htstats->SetStats(0);
    htstats->DrawCopy("");
  }
  cstats.Print(Form("cdcdedx_coscorr_stats_%s.pdf", m_suffix.data()));
}