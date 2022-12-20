/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This fiLe is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxWireGainAlgorithm.h>

using namespace Belle2;
using namespace CDC;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxWireGainAlgorithm::CDCDedxWireGainAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  c_nwireCDC(c_nSenseWires),
  isMakePlots(true),
  isMerge(true),
  isWireTruc(false),
  m_dedxBins(250),
  m_dedxMin(0.0),
  m_dedxMax(5.0),
  m_trucMin(0.05),
  m_trucMax(0.75),
  m_suffix("")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx wire gains");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxWireGainAlgorithm::calibrate()
{

  //if not checking then copy function here;
  getExpRunInfo();

  if (!m_DBBadWires.isValid() || !m_DBWireGains.isValid())
    B2FATAL("There is no valid payload for BadWires and/or Wirgain");

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (ttree->GetEntries() < 100)return c_NotEnoughData;

  std::vector<int>* wire = 0;
  ttree->SetBranchAddress("wire", &wire);

  std::vector<double>* dedxhit = 0;
  ttree->SetBranchAddress("dedxhit", &dedxhit);

  // dedxhit vector to store dE/dx values for each wire
  std::map<int, std::vector<double>> wirededx;

  //IL = Inner Layer and OL = Outer Layer
  TH1D* hdedxIL = new TH1D(Form("hdedxIL_%s", m_suffix.data()), "", m_dedxBins, m_dedxMin, m_dedxMax);

  TH1D* hdedxOL = new TH1D(Form("hdedxOL_%s", m_suffix.data()), "", m_dedxBins, m_dedxMin, m_dedxMax);

  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int j = 0; j < wire->size(); ++j) {
      int jwire = wire->at(j);
      double jhitdedx = dedxhit->at(j);
      wirededx[jwire].push_back(jhitdedx);
      //wire # 1279 end of inner layers
      if (jwire < 1280) hdedxIL->Fill(jhitdedx);
      else hdedxOL->Fill(jhitdedx);
    }
  }

  //return if ~10% low stats or dead wires
  int minstat = 0;
  for (unsigned int jw = 0; jw < c_nwireCDC; ++jw)
    if (wirededx[jw].size() <= 100) minstat++;
  if (minstat > 0.10 * c_nwireCDC)  return c_NotEnoughData;

  //25-75 is average bin # for trunction
  unsigned int minbinIL, maxbinIL;
  getTruncatedBins(hdedxIL, minbinIL, maxbinIL);
  hdedxIL->SetTitle(Form("IL(%s);%d;%d", m_suffix.data(), minbinIL, maxbinIL));

  unsigned int minbinOL, maxbinOL;
  getTruncatedBins(hdedxOL, minbinOL, maxbinOL);
  hdedxOL->SetTitle(Form("OL (%s);%d;%d", m_suffix.data(), minbinOL, maxbinOL));

  std::vector<double> vrel_mean;
  std::vector<double> vdedx_means;

  TH1D* hdedxhit[c_nwireCDC];

  B2INFO("Creating CDCGeometryPar object");
  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance(&(*m_cdcGeo));

  std::array<double, 56> layermean;
  int activelayers = 0;
  double layeravg = 0.0;

  int jwire = -1;
  for (unsigned int il = 0; il < c_maxNSenseLayers; ++il) {

    int activewires = 0;
    layermean[il] = 0.0;

    for (unsigned int iw = 0; iw < cdcgeo.nWiresInLayer(il); ++iw) {

      jwire++;
      hdedxhit[jwire] = new TH1D(Form("h%s_w%d", m_suffix.data(), jwire), "", m_dedxBins, m_dedxMin, m_dedxMax);

      for (unsigned int ih = 0; ih < wirededx[jwire].size(); ++ih) {
        hdedxhit[jwire]->Fill(wirededx[jwire][ih]);
      }

      unsigned int minbin, maxbin;
      if (!isWireTruc) {
        getTruncatedBins(hdedxhit[jwire], minbin, maxbin);
      } else {
        if (jwire < 1280) {
          minbin = minbinIL;
          maxbin =  maxbinIL;
        } else {
          minbin = minbinOL;
          maxbin =  maxbinOL;
        }
      }
      hdedxhit[jwire]->SetTitle(Form("dedxhit-dist, wire: %d (%s);%d;%d", jwire, m_suffix.data(), minbin, maxbin));

      double dedxmean;
      if (m_DBBadWires->getBadWireStatus(jwire) == kTRUE) dedxmean = 0.0;
      else dedxmean  = getTruncationMean(hdedxhit[jwire], minbin, maxbin);
      vrel_mean.push_back(dedxmean);

      double prewg = m_DBWireGains->getWireGain(jwire);
      if (prewg > 0.0 && isMerge) {
        vdedx_means.push_back(dedxmean * prewg);
        B2INFO("merged-wireGain: [" << jwire << "], prewgvious = " << prewg << ", rel = " << dedxmean << ", merged = " << vdedx_means.at(
                 jwire));
      } else vdedx_means.push_back(dedxmean);

      //calculate layer average for active wires
      if (vdedx_means.at(jwire) > 0) {
        layermean[il] += vdedx_means.at(jwire);
        activewires++;
      }
    }

    if (activewires > 0) layermean[il] /= activewires;
    else layermean[il] = 1.0;

    //calculate outer layer average for active layer
    if (il >= 8 && layermean[il] > 0) {
      layeravg += layermean[il];
      activelayers++;
    }

  }

  //normalisation of wiregains to get outergain ~1.0
  if (activelayers > 0)layeravg /= activelayers;

  for (unsigned int iw = 0; iw < c_nwireCDC; ++iw) {
    vrel_mean.at(iw) /= layeravg;
    vdedx_means.at(iw) /= layeravg;
  }

  if (isMakePlots) {

    //1. Inner and Outer layer dE/dx distributions
    plotLayerDist(hdedxIL, hdedxOL);

    //2. wiredist -> 14336 (good, bad)
    plotWireDist(hdedxhit, vrel_mean);

    //3. wiregains draw (1D, dist)
    plotWireGain(vdedx_means, vrel_mean, layeravg);

    //4. layer gain plot
    plotLayerGain(layermean, layeravg);

    //5. wiregains per layer
    plotWGPerLayer(vdedx_means, layermean, layeravg);

    //6. Statstics plot
    plotStats();
  }

  createPayload(vdedx_means);
  m_suffix.clear();
  return c_OK;
}

//------------------------------------
void CDCDedxWireGainAlgorithm::getExpRunInfo()
{

  int cruns = 0;
  for (auto expRun : getRunList()) {
    if (cruns == 0)B2INFO("CDCDedxWireGain: start exp " << expRun.first << " and run " << expRun.second << "");
    cruns++;
  }

  const auto erStart = getRunList()[0];
  int estart = erStart.first;
  int rstart = erStart.second;

  const auto erEnd = getRunList()[cruns - 1];
  int rend = erEnd.second;

  updateDBObjPtrs(1, rstart, estart);

  if (m_suffix.length() > 0)m_suffix = Form("%s_e%d_r%dr%d", m_suffix.data(), estart, rstart, rend);
  else  m_suffix = Form("e%d_r%dr%d", estart, rstart, rend);
}

//--------------------------
void CDCDedxWireGainAlgorithm::createPayload(std::vector<double> vdedx_means)
{

  B2INFO("dE/dx Calibration done for " << vdedx_means.size() << " CDC wires");
  CDCDedxWireGain* gains = new CDCDedxWireGain(vdedx_means);
  saveCalibration(gains, "CDCDedxWireGain");
}

//--------------------------
double CDCDedxWireGainAlgorithm::getTruncationMean(TH1D* hdedxhit, int binlow, int binhigh)
{

  //calculating truncation average
  if (hdedxhit->Integral() < 100) return 1.0;

  if (binlow <= 0 || binhigh > hdedxhit->GetNbinsX())return 1.0;

  double binweights = 0., sumofbc = 0.;
  for (int ibin = binlow; ibin <= binhigh; ibin++) {
    double bcdedx = hdedxhit->GetBinContent(ibin);
    if (bcdedx > 0) {
      binweights += (bcdedx * hdedxhit->GetBinCenter(ibin));
      sumofbc += bcdedx;
    }
  }
  if (sumofbc > 0) return binweights / sumofbc;
  else return 1.0;
}

//--------------------------
void CDCDedxWireGainAlgorithm::getTruncatedBins(TH1D* hdedxhit, unsigned int& binlow, unsigned int& binhigh)
{

  //calculating truncation average
  double sum = hdedxhit->Integral();
  if (sum <= 0 || hdedxhit->GetNbinsX() <= 0) {
    binlow = 1; binhigh = 1;
    return ;
  }

  binlow = 1.0; binhigh = 1.0;
  double sumPer5 = 0.0, sumPer75 = 0.0;
  for (int ibin = 1; ibin <= hdedxhit->GetNbinsX(); ibin++) {
    double bcdedx = hdedxhit->GetBinContent(ibin);
    if (sumPer5  <= m_trucMin * sum) {
      sumPer5 += bcdedx;
      binlow = ibin;
    }
    if (sumPer75  <= m_trucMax * sum) {
      sumPer75 += bcdedx;
      binhigh = ibin;
    }
  }
  return;
}

//--------------------------
void CDCDedxWireGainAlgorithm::plotLayerDist(TH1D*& hdedxIL, TH1D*& hdedxOL)
{

  TCanvas cldedx("cldedx", "IL/OL dedxhit dist", 900, 400);
  cldedx.Divide(2, 1);

  cldedx.cd(1);
  int minbin = std::stoi(hdedxIL->GetXaxis()->GetTitle());
  int maxbin = std::stoi(hdedxIL->GetYaxis()->GetTitle());
  double lowedge = hdedxIL->GetXaxis()->GetBinLowEdge(minbin);
  double upedge = hdedxIL->GetXaxis()->GetBinUpEdge(maxbin);
  hdedxIL->SetFillColor(kYellow);
  hdedxIL->SetTitle(Form("%s, trunc(%0.02f - %0.02f);dedxhit;entries", hdedxIL->GetTitle(), lowedge, upedge));
  hdedxIL->Draw("histo");
  TH1D* hdedxILC = (TH1D*)hdedxIL->Clone(Form("%s_c", hdedxIL->GetTitle()));
  hdedxILC->GetXaxis()->SetRange(minbin, maxbin);
  hdedxILC->SetFillColor(kAzure + 1);
  hdedxILC->Draw("same histo");

  cldedx.cd(2);
  minbin = std::stoi(hdedxOL->GetXaxis()->GetTitle());
  maxbin = std::stoi(hdedxOL->GetYaxis()->GetTitle());
  lowedge = hdedxOL->GetXaxis()->GetBinLowEdge(minbin);
  upedge = hdedxOL->GetXaxis()->GetBinUpEdge(maxbin);
  hdedxOL->SetTitle(Form("%s trunc(%0.02f - %0.02f);dedxhit;entries", hdedxOL->GetTitle(), lowedge, upedge));
  hdedxOL->SetFillColor(kYellow);
  hdedxOL->Draw("histo");
  TH1D* hdedxOLC = (TH1D*)hdedxOL->Clone(Form("%s_c", hdedxOL->GetTitle()));
  hdedxOLC->GetXaxis()->SetRange(minbin, maxbin);
  hdedxOLC->SetFillColor(kAzure + 1);
  hdedxOLC->Draw("same histo");

  cldedx.SaveAs(Form("cdcdedx_wgcal_layerdedx_%s.pdf", m_suffix.data()));
}

//------------------------------------
void CDCDedxWireGainAlgorithm::plotWireDist(TH1D* hist[], std::vector<double>vrel_mean)
{

  TCanvas* ctmp = new TCanvas(Form("cdcdedx_%s", m_suffix.data()), "", 1200, 1200);
  ctmp->Divide(4, 4);
  ctmp->SetBatch(kTRUE);

  std::stringstream psname;
  psname << Form("cdcdedx_wgcal_%s.pdf[", m_suffix.data());
  ctmp->Print(psname.str().c_str());
  psname.str("");
  psname << Form("cdcdedx_wgcal_%s.pdf", m_suffix.data());

  for (unsigned int iw = 0; iw < c_nwireCDC; iw++) {

    int minbin = std::stoi(hist[iw]->GetXaxis()->GetTitle());
    int maxbin = std::stoi(hist[iw]->GetYaxis()->GetTitle());

    hist[iw]->SetFillColor(kYellow - 9);
    hist[iw]->SetTitle(Form("%s, rel. #mu_{trunc} %0.03f;dedxhit;entries", hist[iw]->GetTitle(), vrel_mean.at(iw)));

    if (m_DBBadWires->getBadWireStatus(iw) == kTRUE) {
      hist[iw]->SetLineColor(kRed);
      hist[iw]->SetLineWidth(2);
    }
    ctmp->cd(iw % 16 + 1);
    hist[iw]->Draw();

    TH1D* hdedxhitC = (TH1D*)hist[iw]->Clone(Form("%sC", hist[iw]->GetName()));
    hdedxhitC->GetXaxis()->SetRange(minbin, maxbin);
    hdedxhitC->SetFillColor(kAzure + 1);
    hdedxhitC->DrawCopy("same histo");

    if (((iw + 1) % 16 == 0) || iw == (c_nwireCDC - 1))  {
      ctmp->Print(psname.str().c_str());
      ctmp->Clear("D");
    }
  }

  psname.str("");
  psname << Form("cdcdedx_wgcal_%s.pdf]", m_suffix.data());
  ctmp->Print(psname.str().c_str());
  delete ctmp;
}

//------------------------------------
void CDCDedxWireGainAlgorithm::plotWireGain(std::vector<double>vdedx_means, std::vector<double>vrel_mean, double layeravg)
{

  //saving final constants in a histograms for validation
  TCanvas cwconst("cwconst", "", 900, 500);
  TCanvas cwconstvar("cwconstvar", "", 500, 400);

  TH1D* hconstpw[2], *hconstpwvar[2];

  for (int i = 0; i < 2; i++) {

    hconstpw[i] = new TH1D(Form("hconstpw_%d_%s", i, m_suffix.data()), "", c_nwireCDC, -0.5, 14335.5);
    hconstpw[i]->SetTitle(Form("wiregain const  (%s); wire numbers;<dedxhit>", m_suffix.data()));
    if (isMerge
        && i == 0) hconstpw[i]->SetTitle(Form("merged wiregain rel-const  (%s), avg = %0.03f; wire numbers;<dedxhit>", m_suffix.data(),
                                                layeravg));

    hconstpwvar[i] = new TH1D(Form("hconstpwvar_%s", m_suffix.data()), "", 400, -0.5, 2.5);
    hconstpwvar[i]->SetTitle(Form("wiregain const (%s); wire gains; nentries", m_suffix.data()));
    if (isMerge
        && i == 0) hconstpwvar[i]->SetTitle(Form("merged wiregain rel-const (%s), avg = %0.03f; wire gains; nentries", m_suffix.data(),
                                                   layeravg));

    for (unsigned int iw = 0; iw < c_nwireCDC; iw++) {

      double gain = vdedx_means.at(iw);
      if (isMerge && i == 1) gain =  vrel_mean.at(iw);
      hconstpw[i]->SetBinContent(iw + 1, gain);
      hconstpwvar[i]->Fill(gain);

      if (iw % 500 == 0)hconstpw[i]->GetXaxis()->SetBinLabel(iw + 1, Form("w%d", iw + 1));
    }

    hconstpw[i]->SetLineColor(i * 2 + 2);
    hconstpw[i]->LabelsOption("u", "X");
    hconstpw[i]->GetYaxis()->SetRangeUser(-0.1, 3.5);
    hconstpw[i]->LabelsDeflate();

    hconstpwvar[i]->SetFillColor(i * 2 + 2);
    hconstpwvar[i]->Scale(1 / hconstpwvar[i]->GetMaximum());

  }
  cwconst.cd();
  cwconst.SetGridy(1);
  hconstpw[0]->Draw("");
  if (isMerge) hconstpw[1]->Draw("same");

  cwconstvar.cd();
  hconstpwvar[0]->Draw("hist");
  if (isMerge) hconstpwvar[1]->Draw("hist same");

  cwconst.SaveAs(Form("cdcdedx_wgcal_wireconst_%s.pdf", m_suffix.data()));
  cwconstvar.SaveAs(Form("cdcdedx_wgcal_wireconstvar_%s.pdf", m_suffix.data()));

}

//------------------------------------
void CDCDedxWireGainAlgorithm::plotLayerGain(std::array<double, 56> layermean, double layeravg)
{

  TH1D* hlayeravg = new TH1D(Form("hlayeravg_%s", m_suffix.data()), "", c_maxNSenseLayers, -0.5, 55.5);
  hlayeravg->SetTitle(Form("layer gain avg (%s); layer numbers;<dedxhit>", m_suffix.data()));

  for (unsigned int iL = 0; iL < c_maxNSenseLayers; iL++) {
    hlayeravg->SetBinContent(iL + 1, layermean[iL]);
    if (iL % 2 == 0 || iL == c_maxNSenseLayers - 1)hlayeravg->GetXaxis()->SetBinLabel(iL + 1, Form("L%d", iL));
  }

  TCanvas clayeravg("clayeravg", "clayeravg", 800, 500);
  clayeravg.SetGridy(1);
  clayeravg.cd();
  gStyle->SetOptStat("ne");
  hlayeravg->LabelsOption("u", "X");
  hlayeravg->SetLineColor(kBlue);
  hlayeravg->GetYaxis()->SetRangeUser(-0.1, 3.5);
  hlayeravg->SetTitle(Form("%s, avg = %0.03f (abs)", hlayeravg->GetTitle(), layeravg));
  hlayeravg->LabelsDeflate();
  hlayeravg->Draw("");
  TLine* tl = new TLine(-0.5, layeravg, 55.5, layeravg);
  tl->SetLineColor(kRed);
  tl->DrawClone("same");
  clayeravg.SaveAs(Form("cdcdedx_wgcal_layeravg_%s.pdf", m_suffix.data()));
}


//------------------------------------
void CDCDedxWireGainAlgorithm::plotWGPerLayer(std::vector<double>vdedx_means, std::array<double, 56> layermean, double layeravg)
{

  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance(&(*m_cdcGeo));

  TCanvas* clconst = new TCanvas("clconst", "", 800, 500);
  clconst->Divide(2, 2);
  clconst->SetBatch(kTRUE);

  std::stringstream psnameL;
  psnameL << Form("cdcdedx_wgcal_layerconst_%s.pdf[", m_suffix.data());
  clconst->Print(psnameL.str().c_str());
  psnameL.str(""); psnameL << Form("cdcdedx_wgcal_layerconst_%s.pdf", m_suffix.data());

  int jwire = 0;

  for (unsigned int il = 0; il < c_maxNSenseLayers; ++il) {

    unsigned int nwires = cdcgeo.nWiresInLayer(il);
    TH1D* hconstpl = new TH1D(Form("hconstpwvar_l%d_%s", il, m_suffix.data()), "", nwires, jwire, jwire + nwires);
    hconstpl->SetTitle(Form("abs-const, layer: %d (%s); wire numbers;<dedxhit>", il, m_suffix.data()));

    for (unsigned int iw = 0; iw < nwires; ++iw) {
      hconstpl->SetBinContent(iw + 1, vdedx_means.at(jwire));
      if (il < 32) {
        if (iw % 10 == 0)hconstpl->GetXaxis()->SetBinLabel(iw + 1, Form("w%d", jwire));
      } else {
        if (iw % 15 == 0)hconstpl->GetXaxis()->SetBinLabel(iw + 1, Form("w%d", jwire));
      }
      jwire++;
    }

    double lmean  = layermean.at(il) / layeravg;

    clconst->cd(il % 4 + 1);
    gStyle->SetOptStat("ne");

    hconstpl->SetTitle(Form("%s, avg = %0.03f", hconstpl->GetTitle(), lmean));

    if (il < 8)hconstpl->GetYaxis()->SetRangeUser(-0.1, 4.0);
    else hconstpl->GetYaxis()->SetRangeUser(-0.1, 2.0);
    hconstpl->SetFillColor(kAzure - 1);
    hconstpl->LabelsOption("u", "X");
    hconstpl->DrawCopy("hist");

    TLine* tlc = new TLine(jwire - nwires, lmean, jwire, lmean);
    tlc->SetLineColor(kRed);
    tlc->DrawClone("same");
    delete tlc;

    if ((il + 1) % 4 == 0)clconst->Print(psnameL.str().c_str());
    hconstpl->Reset();
  }

  psnameL.str("");
  psnameL << Form("cdcdedx_wgcal_layerconst_%s.pdf]", m_suffix.data());
  clconst->Print(psnameL.str().c_str());
  delete clconst;
}

//--------------------------
void CDCDedxWireGainAlgorithm::plotStats()
{

  TCanvas cstats("cstats", "cstats", 800, 400);
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
    hestats->SetName(Form("htstats_%s", m_suffix.data()));
    htstats->DrawCopy("");
    hestats->SetStats(0);
  }

  cstats.Print(Form("cdcdedx_wgcal_stats_%s.pdf", m_suffix.data()));
}