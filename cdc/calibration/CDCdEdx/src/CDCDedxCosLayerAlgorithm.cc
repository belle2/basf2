/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/calibration/CDCdEdx/CDCDedxCosLayerAlgorithm.h>

#include <TF1.h>
#include <TLine.h>
#include <TCanvas.h>
#include <TH1I.h>
#include <vector>

using namespace Belle2;
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxCosLayerAlgorithm::CDCDedxCosLayerAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  isMethodSep(true),
  isMakePlots(true),
  isMerge(true),
  isFixTrunc(false),
  isUseTrunc(false),
  m_truncMin(0.05),
  m_truncMax(0.75),
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
CalibrationAlgorithm::EResult CDCDedxCosLayerAlgorithm::calibrate()
{

  getExpRunInfo();

  if (!m_DBCosineCor.isValid())
    B2FATAL("There is no valid previous payload for CDCDedxCosineCor");

  B2INFO("Preparing dE/dx calibration for CDC dE/dx electron saturation");

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (ttree->GetEntries() < 100)return c_NotEnoughData;

  std::vector<double>* lDedx = nullptr;
  std::vector<int>* lLayer = nullptr;
  double costh = 0.0;
  int charge = 0;

  ttree->SetBranchAddress("ldedx", &lDedx);
  ttree->SetBranchAddress("lLayer", &lLayer);
  ttree->SetBranchAddress("costh", &costh);
  ttree->SetBranchAddress("charge", &charge);

  // fill histograms, bin size may be arbitrary
  TH1D* hCosth_neg = defineCosthHist("neg", "e-");
  TH1D* hCosth_pos = defineCosthHist("pos", "e+");
  TH1D* hCosth_all = defineCosthHist("all", "e-,e+");

  std::array<std::vector<double>, m_kNGroups> corrFactor;

  for (int ig = 0; ig < m_kNGroups; ig++) {
    corrFactor[ig].assign(m_cosBin, 1.0);
  }

  for (int iter = 0; iter < 3; iter++) {
    // make histograms to store dE/dx values in bins of cos(theta)
    // bin size can be arbitrary, but for now just make uniform bins
    std::array<std::vector<TH1D*>, m_kNGroups> hDedxCos_neg;
    std::array<std::vector<TH1D*>, m_kNGroups> hDedxCos_pos;
    std::array<std::vector<TH1D*>, m_kNGroups> hDedxCos_all;

    defineHisto(hDedxCos_neg, Form("neg_iter%d", iter), "e-");
    defineHisto(hDedxCos_pos, Form("pos_iter%d", iter), "e+");
    defineHisto(hDedxCos_all, Form("all_iter%d", iter), "e-,e+");

    std::array<TH1D*, m_kNGroups> hDedxGroup{};
    for (int il = 0; il < m_kNGroups; il++) {
      std::string title = Form("dedxhit dist (%s); dedxhit;entries", m_label[il].data());
      hDedxGroup[il] = new TH1D(Form("hDedxGroup_%s_%s_iter%d", m_label[il].data(), m_suffix.data(), iter), "", m_dedxBin, m_dedxMin,
                                m_dedxMax);
      hDedxGroup[il]->SetTitle(title.c_str());
    }

    const double binW = (m_cosMax - m_cosMin) / m_cosBin;

    for (int i = 0; i < ttree->GetEntries(); ++i) {

      ttree->GetEvent(i);

      if (!lDedx || !lLayer) continue;
      if (lDedx->size() != lLayer->size()) continue;
      if (charge == 0) continue;

      if (costh < TMath::Cos(150 * TMath::DegToRad()) ||
          costh > TMath::Cos(17 * TMath::DegToRad())) continue;

      int bin = int((costh - m_cosMin) / binW);
      if (bin < 0 || bin >= int(m_cosBin)) continue;

      for (size_t j = 0; j < lDedx->size(); ++j) {

        double val = lDedx->at(j);
        int lay = lLayer->at(j);

        if (val <= 0) continue;

        int ig = (lay < 8) ? 0 : ((lay < 14) ? 1 : 2);

        val /= corrFactor[ig][bin];

        if (isMethodSep) {
          if (charge < 0)
            hDedxCos_neg[ig][bin]->Fill(val);
          else if (charge > 0)
            hDedxCos_pos[ig][bin]->Fill(val);

        }
        hDedxCos_all[ig][bin]->Fill(val);

        hDedxGroup[ig]->Fill(val);

      }
      if (iter == 0) {
        // costh histo
        if (isMethodSep) {
          if (charge < 0) hCosth_neg->Fill(costh);
          else if (charge > 0) hCosth_pos->Fill(costh);
        }
        hCosth_all->Fill(costh);
      }
    }

    std::array<std::vector<double>, m_kNGroups> cosine;

    for (int il = 0; il < m_kNGroups; ++il) {

      int minGroup = 0, maxGroup = 0;
      std::vector<double> vmean_neg, vmean_pos;

      if (isFixTrunc && isUseTrunc) {
        getTruncatedBins(hDedxGroup[il], minGroup, maxGroup);
        hDedxGroup[il]->SetTitle(
          Form("%s;%d;%d", hDedxGroup[il]->GetTitle(), minGroup, maxGroup));
      }

      cosine[il].reserve(m_cosBin);

      for (unsigned int ibin = 0; ibin < m_cosBin; ++ibin) {

        double mean = 1.0;

        if (isMethodSep) {

          double mean_neg = extractCosMean(hDedxCos_neg[il][ibin], minGroup, maxGroup);
          double mean_pos = extractCosMean(hDedxCos_pos[il][ibin], minGroup, maxGroup);

          bool has_neg = (hDedxCos_neg[il][ibin]->Integral() > 0);
          bool has_pos = (hDedxCos_pos[il][ibin]->Integral() > 0);

          if (has_neg && has_pos) mean = 0.5 * (mean_neg + mean_pos);
          else if (has_neg) mean = mean_neg;
          else if (has_pos) mean = mean_pos;
          vmean_neg.push_back(mean_neg);
          vmean_pos.push_back(mean_pos);

        } else {
          mean = extractCosMean(hDedxCos_all[il][ibin], minGroup, maxGroup);
        }

        cosine[il].push_back(mean);
        if (mean > 0) corrFactor[il][ibin] *= (mean / 1.25);
      }
      if (isMethodSep) {
        std::array<std::vector<double>, 3> cosMeanSets = {vmean_neg, vmean_pos, cosine[il]};
        plotmeanChargeOverlay(cosMeanSets, m_label[il], iter);
      }
    }


    if (isMakePlots) {

      //1. dE/dx dist. for cosine bins

      if (isMethodSep) {
        plotdedxHist(hDedxCos_neg, Form("neg_iter%d", iter));
        plotdedxHist(hDedxCos_pos, Form("pos_iter%d", iter));
      }
      plotdedxHist(hDedxCos_all, Form("all_iter%d", iter));

      //3. Inner and Outer layer dE/dx distributions
      plotLayerDist(hDedxGroup, iter);

      //5. draw the relative constants
      plotRelConst(corrFactor, iter);
    }

  }

  for (int il = 0; il < m_kNGroups; ++il)
    m_coscors.push_back(corrFactor[il]);

  createPayload();

  if (isMakePlots) {
    //4. costh distribution
    plotQaPars(hCosth_all, hCosth_pos, hCosth_neg);

    //6. draw the final constants
    plotConstants();

    //7. plot statistics related plots here
    plotEventStats();

  }

  m_suffix.clear();

  return c_OK;
}

//--------------------------------------------------
void CDCDedxCosLayerAlgorithm::getExpRunInfo()
{

  int cruns = 0;
  for (auto expRun : getRunList()) {
    if (cruns == 0) B2INFO("CDCDedxBadWires: start exp " << expRun.first << " and run " << expRun.second << "");
    cruns++;
  }

  const auto erStart = getRunList()[0];
  int estart = erStart.first;
  int rstart = erStart.second;

  const auto erEnd = getRunList()[cruns - 1];
  int eend = erEnd.first;
  int rend = erEnd.second;

  updateDBObjPtrs(1, rstart, estart);

  m_runExp = Form("Range (%d:%d,%d:%d)", estart, rstart, eend, rend);
  if (m_suffix.length() > 0) m_suffix = Form("%s_e%d_r%dr%d", m_suffix.data(), estart, rstart, rend);
  else  m_suffix = Form("e%d_r%dr%d", estart, rstart, rend);
}

//--------------------------------------------------
TH1D* CDCDedxCosLayerAlgorithm::defineCosthHist(const std::string& tag, const std::string& chargeLabel)
{

  TH1D* hist = new TH1D(Form("hCosth_%s_%s", tag.c_str(), m_suffix.data()), " ", m_cosBin, m_cosMin, m_cosMax);
  hist->SetTitle(Form("cos(#theta) dist (%s); cos(#theta); Entries", chargeLabel.c_str()));

  return hist;
}

//--------------------------------------------------
void CDCDedxCosLayerAlgorithm::defineHisto(std::array<std::vector<TH1D*>, m_kNGroups>& hdedx, const std::string& tag,
                                           const std::string& chargeLabel)
{

  const double binW = (m_cosMax - m_cosMin) / m_cosBin;

  for (int il = 0; il < m_kNGroups; il++) {
    hdedx[il].reserve(m_cosBin);

    for (unsigned int i = 0; i < m_cosBin; ++i) {
      double coslow = i * binW + m_cosMin;
      double coshigh = coslow + binW;

      hdedx[il].push_back(new TH1D(Form("hDedxCos_%s_g%d_bin%d_%s", tag.c_str(), il, i, m_suffix.data()),
                                   "", m_dedxBin, m_dedxMin, m_dedxMax));

      hdedx[il][i]->SetTitle(Form("%s dE/dx dist (%s) in costh (%0.02f, %0.02f)",
                                  m_label[il].c_str(), chargeLabel.c_str(), coslow, coshigh));

      // hdedx[il][i]->GetXaxis()->SetTitle("dE/dx");
      // hdedx[il][i]->GetYaxis()->SetTitle("Entries");
    }
  }
}

//--------------------------------------------------
void CDCDedxCosLayerAlgorithm::createPayload()
{

  for (unsigned int il = 0; il < m_kNGroups; il++) {
    if (isMerge) {
      unsigned int nbins = m_DBCosineCor->getSize(getRepresentativeLayer(il));
      if (nbins != m_cosBin)
        B2ERROR("merging failed because of unmatch bins (old " << m_cosBin << " new " << nbins << ")");

      for (unsigned int ibin = 0; ibin < nbins; ibin++) {
        double prev = m_DBCosineCor->getMean(getRepresentativeLayer(il), ibin);
        B2INFO("Cosine Corr for " << m_label[il] << " Bin # " << ibin << ", Previous = " << prev << ", Relative = " << m_coscors[il][ibin]
               << ", Merged = " << prev * m_coscors[il][ibin]);
        m_coscors[il][ibin] *= prev;

      }
    }
  }

  //Saving constants
  B2INFO("dE/dx calibration done for CDC dE/dx _eltron saturation");

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
void CDCDedxCosLayerAlgorithm::plotdedxHist(std::array<std::vector<TH1D*>, 3>& hdedx, const std::string& tag)
{

  TCanvas ctmp("tmp", "tmp", 1200, 1200);
  int nx = 2;
  int ny = 2;
  unsigned int nPads = nx * ny;

  ctmp.Divide(nx, ny);
  std::stringstream psname;

  psname << Form("cdcdedx_coscorr_ldedx_%s_%s.pdf[", tag.c_str(), m_suffix.data());
  ctmp.Print(psname.str().c_str());
  psname.str("");
  psname << Form("cdcdedx_coscorr_ldedx_%s_%s.pdf", tag.c_str(), m_suffix.data());

  for (int il = 0; il < m_kNGroups; il++) {

    for (unsigned int ic = 0; ic < m_cosBin; ic++) {

      ctmp.cd(ic % nPads + 1);
      hdedx[il][ic]->SetFillColor(4 + il);

      hdedx[il][ic]->SetTitle(Form("%s;ldedx;entries", hdedx[il][ic]->GetTitle()));
      hdedx[il][ic]->DrawClone("hist");

      if (ic % nPads == nPads - 1 || ic == m_cosBin - 1) {
        ctmp.Print(psname.str().c_str());
        gPad->Clear("D");
        ctmp.Clear("D");
      }
    }
  }
  psname.str("");
  psname << Form("cdcdedx_coscorr_ldedx_%s_%s.pdf]", tag.c_str(), m_suffix.data());
  ctmp.Print(psname.str().c_str());
}

//--------------------------------------------------
void CDCDedxCosLayerAlgorithm::plotLayerDist(std::array<TH1D*, 3>& hDedxGroup, int iter)
{

  TCanvas cdedxlayer(Form("layerdedxhit_iter%d", iter), "Inner and Outer Layer dedxhit dist", 2400, 800);
  cdedxlayer.Divide(3, 1);

  for (int il = 0; il < m_kNGroups; il++) {
    int minlay = 0, maxlay = 0;
    if (isFixTrunc) {
      minlay = std::stoi(hDedxGroup[il]->GetXaxis()->GetTitle());
      maxlay = std::stoi(hDedxGroup[il]->GetYaxis()->GetTitle());
      double lowedge = hDedxGroup[il]->GetXaxis()->GetBinLowEdge(minlay);
      double upedge = hDedxGroup[il]->GetXaxis()->GetBinUpEdge(maxlay);
      hDedxGroup[il]->SetTitle(Form("%s, trunc #rightarrow: %0.02f - %0.02f;dedxhit;entries", hDedxGroup[il]->GetTitle(), lowedge,
                                    upedge));
    }

    cdedxlayer.cd(il + 1);
    hDedxGroup[il]->SetFillColor(kYellow);
    hDedxGroup[il]->Draw("histo");

    if (isFixTrunc) {
      TH1D* hDedxGroupC = (TH1D*)hDedxGroup[il]->Clone(Form("hDedxGroupC%d", il));
      hDedxGroupC->GetXaxis()->SetRange(minlay, maxlay);
      hDedxGroupC->SetFillColor(kAzure + 1);
      hDedxGroupC->Draw("same histo");
    }
  }

  cdedxlayer.SaveAs(Form("cdcdedx_coscorr_dedxlay%s_iter%d.pdf", m_suffix.data(), iter));
  cdedxlayer.SaveAs(Form("cdcdedx_coscorr_dedxlay%s_iter%d.root", m_suffix.data(), iter));
}

//--------------------------------------------------
void CDCDedxCosLayerAlgorithm::plotQaPars(TH1D* hCosth_all, TH1D* hCosth_pos, TH1D* hCosth_neg)
{

  TCanvas ceadist("ceadist", "Cosine distributions", 800, 600);
  ceadist.cd();

  TLegend* leg = new TLegend(0.6, 0.7, 0.8, 0.9);

  // Always draw ALL first
  if (hCosth_all) {
    hCosth_all->SetFillColor(kYellow);
    hCosth_all->SetLineColor(kBlack);
    hCosth_all->SetStats(0);
    hCosth_all->Draw("hist");
    leg->AddEntry(hCosth_all, "all", "f");
  }

  // If method separation, overlay pos/neg
  if (isMethodSep) {

    if (hCosth_pos) {
      hCosth_pos->SetLineColor(kRed);
      hCosth_pos->SetFillStyle(0);
      hCosth_pos->SetStats(0);
      hCosth_pos->Draw("hist same");
      leg->AddEntry(hCosth_pos, "pos", "l");
    }

    if (hCosth_neg) {
      hCosth_neg->SetLineColor(kBlue);
      hCosth_neg->SetFillStyle(0);
      hCosth_neg->SetStats(0);
      hCosth_neg->Draw("hist same");
      leg->AddEntry(hCosth_neg, "neg", "l");
    }
  }

  leg->Draw();

  ceadist.SaveAs(Form("cdcdedx_coscorr_cosine_%s.pdf", m_suffix.data()));
  ceadist.SaveAs(Form("cdcdedx_coscorr_cosine_%s.root", m_suffix.data()));
}

//--------------------------------------------------
void CDCDedxCosLayerAlgorithm::plotRelConst(const std::array<std::vector<double>, m_kNGroups>& cosine, int iter)
{
  TCanvas cconst("cconst", "calibration Constants", 800, 600);
  cconst.cd();

  TLegend* leg = new TLegend(0.6, 0.8, 0.9, 0.9);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);

  std::vector<TH1D*> hists;
  std::vector<int> colors = {kRed, kBlue, kGreen + 2};
  double ymax = 0.0;

  for (int il = 0; il < m_kNGroups; il++) {

    TH1D* h = new TH1D(Form("hconst_%d_%s", il, m_suffix.data()), "Relative constants; cos(#theta); constant", m_cosBin, m_cosMin,
                       m_cosMax);

    // fill histogram
    for (unsigned int jea = 0; jea < m_cosBin; jea++) {
      if (jea < cosine[il].size())
        h->SetBinContent(jea + 1, cosine[il].at(jea));
    }

    double hmax = h->GetMaximum();
    if (hmax > ymax) ymax = hmax;


    hists.push_back(h);
  }

  for (int il = 0; il < m_kNGroups; il++) {

    hists[il]->SetLineColor(colors[il]);
    hists[il]->SetStats(0);
    hists[il]->SetLineWidth(2);

    // draw
    if (il == 0) {
      hists[il]->SetMaximum(ymax + 0.01);
      hists[il]->Draw("hist");
    } else {
      hists[il]->Draw("hist same");
    }

    leg->AddEntry(hists[il], m_label[il].data(), "l");
  }

  leg->Draw();

  cconst.SaveAs(Form("cdcdedx_coscorr_relconst_%s_iter%d.pdf", m_suffix.data(), iter));
  cconst.SaveAs(Form("cdcdedx_coscorr_relconst_%s_iter%d.root", m_suffix.data(), iter));

  // cleanup
  for (auto h : hists) delete h;
}

//--------------------------------------------------
void CDCDedxCosLayerAlgorithm::plotConstants()
{

  for (int il = 0; il < m_kNGroups; il++) {

    unsigned int nbins = m_DBCosineCor->getSize(getRepresentativeLayer(il));

    // --- Create histograms ---
    TH1D* hnew = new TH1D(Form("hnew_%s", m_label[il].data()), Form("Final const: %s", m_label[il].data()), m_cosBin, m_cosMin,
                          m_cosMax);

    TH1D* hold = new TH1D(Form("hold_%s", m_label[il].data()), Form("Final const: %s", m_label[il].data()), m_cosBin, m_cosMin,
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

//--------------------------------------------------
void CDCDedxCosLayerAlgorithm::plotmeanChargeOverlay(const std::array<std::vector<double>, 3>& mean, const std::string& sltag,
                                                     int iter)
{
  TCanvas cconst(Form("cconst_%s_iter%d", sltag.c_str(), iter), "calibration Constants", 800, 600);
  cconst.cd();

  TLegend* leg = new TLegend(0.6, 0.8, 0.9, 0.9);
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);

  std::vector<TH1D*> hists;

  std::array<std::string, 3> labels = {"e^{+}", "e^{-}", "Average"};
  std::vector<int> colors = {kRed, kBlue, kBlack + 2};

  for (int il = 0; il < 3; il++) {

    TH1D* h = new TH1D(Form("hconst_%d_%s_%s_iter%d", il, sltag.c_str(), m_suffix.data(), iter),
                       Form("Relative mean %s, iter %d; cos(#theta); constant", sltag.c_str(), iter),
                       m_cosBin, m_cosMin, m_cosMax);

    for (unsigned int jea = 0; jea < m_cosBin; jea++) {
      if (jea < mean[il].size())
        h->SetBinContent(jea + 1, mean[il].at(jea));
    }

    h->SetLineColor(colors[il]);
    h->SetStats(0);
    h->SetLineWidth(2);

    if (il == 0) h->Draw("hist");
    else h->Draw("hist same");

    leg->AddEntry(h, labels[il].c_str(), "l");
    hists.push_back(h);
  }

  leg->Draw();

  cconst.SaveAs(Form("cdcdedx_coscorr_relmean_iter%d_%s_%s.pdf", iter, sltag.c_str(), m_suffix.data()));
  cconst.SaveAs(Form("cdcdedx_coscorr_relmean_iter%d_%s_%s.root", iter, sltag.c_str(), m_suffix.data()));

  for (auto h : hists) delete h;
  delete leg;
}

//------------------------------------
void CDCDedxCosLayerAlgorithm::plotEventStats()
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

//--------------------------------------------------
void CDCDedxCosLayerAlgorithm::getTruncatedBins(TH1D* hist, int& binlow, int& binhigh)
{

  //calculating truncation average
  double sum = hist->Integral();
  if (sum <= 0 || hist->GetNbinsX() <= 0) {
    binlow = 1; binhigh = 1;
    return ;
  }

  binlow = 1.0; binhigh = 1.0;
  double sumPer5 = 0.0, sumPer75 = 0.0;
  for (int ibin = 1; ibin <= hist->GetNbinsX(); ibin++) {
    double bcdedx = hist->GetBinContent(ibin);
    if (sumPer5  <= m_truncMin * sum) {
      sumPer5 += bcdedx;
      binlow = ibin;
    }
    if (sumPer75  <= m_truncMax * sum) {
      sumPer75 += bcdedx;
      binhigh = ibin;
    }
  }
  return;
}

//--------------------------
double CDCDedxCosLayerAlgorithm::getTruncationMean(TH1D* hist, int binlow, int binhigh)
{

  //calculating truncation average
  if (hist->Integral() < 100) return 1.0;

  if (binlow <= 0 || binhigh > hist->GetNbinsX())return 1.0;

  double binweights = 0., sumofbc = 0.;
  for (int ibin = binlow; ibin <= binhigh; ibin++) {
    double bcdedx = hist->GetBinContent(ibin);
    if (bcdedx > 0) {
      binweights += (bcdedx * hist->GetBinCenter(ibin));
      sumofbc += bcdedx;
    }
  }
  if (sumofbc > 0) return binweights / sumofbc;
  else return 1.0;
}

double CDCDedxCosLayerAlgorithm::extractCosMean(TH1D*& hist, int fixedLow, int fixedHigh)
{
  if (!hist || hist->Integral() <= 0) return 1.0;

  // Default for SL0, SL1: simple mean
  if (!isUseTrunc) {

    hist->SetTitle(Form("%s, mean = %0.5f", hist->GetTitle(), hist->GetMean()));
    return hist->GetMean();
  }

  int minbin = 1, maxbin = 1;
  if (isFixTrunc) {
    minbin = fixedLow;
    maxbin = fixedHigh;
  } else {
    getTruncatedBins(hist, minbin, maxbin);
  }

  double mean = getTruncationMean(hist, minbin, maxbin);

  hist->SetTitle(Form("%s, mean = %0.5f;%d;%d", hist->GetTitle(), mean, minbin, maxbin));

  return mean;
}