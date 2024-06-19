/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxWireGainAlgorithm.h>

using namespace Belle2;
using namespace CDC;
using namespace std;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxWireGainAlgorithm::CDCDedxWireGainAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  m_isMakePlots(true),
  m_isMerge(true),
  m_isWireTruc(false),
  m_dedxBins(250),
  m_dedxMin(0.0),
  m_dedxMax(5.0),
  m_truncMin(0.05),
  m_truncMax(0.75),
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

  vector<int>* wire = 0;
  ttree->SetBranchAddress("wire", &wire);

  vector<double>* dedxhit = 0;
  ttree->SetBranchAddress("dedxhit", &dedxhit);

  // dedxhit vector to store dE/dx values for each wire
  map<int, vector<double>> wirededx;

  //IL = Inner Layer and OL = Outer Layer
  array<TH1D*, 2> hdedxL;
  string label[2] = {"IL", "OL"};
  for (int il = 0; il < 2; il++)
    hdedxL[il] = new TH1D(Form("hdedx%s_%s", label[il].data(), m_suffix.data()), "", m_dedxBins, m_dedxMin, m_dedxMax);

  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int j = 0; j < wire->size(); ++j) {
      int jwire = wire->at(j);
      double jhitdedx = dedxhit->at(j);
      wirededx[jwire].push_back(jhitdedx);
      //wire # 1279 end of inner layers
      if (jwire < 1280) hdedxL[0]->Fill(jhitdedx);
      else hdedxL[1]->Fill(jhitdedx);
    }
  }

  //return if ~10% low stats or dead wires
  int minstat = 0;
  for (unsigned int jw = 0; jw < c_nwireCDC; ++jw)
    if (wirededx[jw].size() <= 100) minstat++;

  if (minstat > 0.10 * c_nwireCDC) return c_NotEnoughData;

  //25-75 is average bin # for trunction
  array<unsigned int, 2> minbinL, maxbinL;
  for (int il = 0; il < 2; il++) {
    getTruncatedBins(hdedxL[il], minbinL[il], maxbinL[il]);
    hdedxL[il]->SetTitle(Form("%s(%s);%d;%d", label[il].data(), m_suffix.data(), minbinL[il], maxbinL[il]));
  }

  vector<double> vrel_mean;
  vector<double> vdedx_means;

  array<TH1D*, c_nwireCDC>  hdedxhit;

  B2INFO("Creating CDCGeometryPar object");
  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance(&(*m_cdcGeo));

  array<double, c_maxNSenseLayers> layermean;
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
      if (!m_isWireTruc) {
        getTruncatedBins(hdedxhit[jwire], minbin, maxbin);
      } else {
        if (jwire < 1280) {
          minbin = minbinL[0];
          maxbin =  maxbinL[0];
        } else {
          minbin = minbinL[1];
          maxbin =  maxbinL[1];
        }
      }
      hdedxhit[jwire]->SetTitle(Form("dedxhit-dist, wire: %d (%s);%d;%d", jwire, m_suffix.data(), minbin, maxbin));

      double dedxmean;
      if (m_DBBadWires->getBadWireStatus(jwire) == kTRUE) dedxmean = 0.0;

      else dedxmean  = getTruncationMean(hdedxhit[jwire], minbin, maxbin);
      vrel_mean.push_back(dedxmean);

      double prewg = m_DBWireGains->getWireGain(jwire);
      if (prewg > 0.0 && m_isMerge) {
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
  if (activelayers > 0) layeravg /= activelayers;

  for (unsigned int iw = 0; iw < c_nwireCDC; ++iw) {
    vrel_mean.at(iw) /= layeravg;
    vdedx_means.at(iw) /= layeravg;
  }

  if (m_isMakePlots) {

    //1. Inner and Outer layer dE/dx distributions
    plotLayerDist(hdedxL);

    //2. wiredist -> 14336 (good, bad)
    plotWireDist(hdedxhit, vrel_mean);

    //3. wiregains draw (1D, dist)
    plotWireGain(vdedx_means, vrel_mean, layeravg);

    //4. layer gain plot
    plotLayerGain(layermean, layeravg);

    //5. wiregains per layer
    plotWGPerLayer(vdedx_means, layermean, layeravg);

    //6. Statstics plot
    plotEventStats();
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
    if (cruns == 0) B2INFO("CDCDedxWireGain: start exp " << expRun.first << " and run " << expRun.second << "");
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

//--------------------------
void CDCDedxWireGainAlgorithm::createPayload(vector<double>& vdedx_means)
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

  if (binlow <= 0 || binhigh > hdedxhit->GetNbinsX()) return 1.0;

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
void CDCDedxWireGainAlgorithm::plotLayerDist(array<TH1D*, 2> hdedxL)
{

  TCanvas cldedx("cldedx", "IL/OL dedxhit dist", 900, 400);
  cldedx.Divide(2, 1);

  for (int il = 0; il < 2; il++) {
    cldedx.cd(il + 1);

    int minbin = stoi(hdedxL[il]->GetXaxis()->GetTitle());
    int maxbin = stoi(hdedxL[il]->GetYaxis()->GetTitle());
    double lowedge = hdedxL[il]->GetXaxis()->GetBinLowEdge(minbin);
    double upedge = hdedxL[il]->GetXaxis()->GetBinUpEdge(maxbin);

    hdedxL[il]->SetFillColor(kYellow);
    hdedxL[il]->SetTitle(Form("%s, trunc(%0.02f - %0.02f);dedxhit;entries", hdedxL[il]->GetTitle(), lowedge, upedge));
    hdedxL[il]->Draw("histo");

    TH1D* hdedxLC = (TH1D*)hdedxL[il]->Clone(Form("%s_c", hdedxL[il]->GetName()));
    hdedxLC->GetXaxis()->SetRange(minbin, maxbin);
    hdedxLC->SetFillColor(kAzure + 1);
    hdedxLC->Draw("same histo");
  }

  cldedx.SaveAs(Form("cdcdedx_wgcal_layerdedx_%s.pdf", m_suffix.data()));
}

//------------------------------------
void CDCDedxWireGainAlgorithm::plotWireDist(array<TH1D*, c_nwireCDC> hist, vector<double>& vrel_mean)
{

  TCanvas ctmp(Form("cdcdedx_%s", m_suffix.data()), "", 1200, 1200);
  ctmp.Divide(4, 4);
  ctmp.SetBatch(kTRUE);

  stringstream psname;
  psname << Form("cdcdedx_wgcal_%s.pdf[", m_suffix.data());
  ctmp.Print(psname.str().c_str());
  psname.str("");
  psname << Form("cdcdedx_wgcal_%s.pdf", m_suffix.data());

  for (unsigned int iw = 0; iw < c_nwireCDC; iw++) {

    int minbin = stoi(hist[iw]->GetXaxis()->GetTitle());
    int maxbin = stoi(hist[iw]->GetYaxis()->GetTitle());

    hist[iw]->SetFillColor(kYellow - 9);
    hist[iw]->SetTitle(Form("%s, rel. #mu_{trunc} %0.03f;dedxhit;entries", hist[iw]->GetTitle(), vrel_mean.at(iw)));

    if (m_DBBadWires->getBadWireStatus(iw) == kTRUE) {
      hist[iw]->SetLineColor(kRed);
      hist[iw]->SetLineWidth(2);
    }
    ctmp.cd(iw % 16 + 1);
    hist[iw]->DrawCopy();

    TH1D* hdedxhitC = (TH1D*)hist[iw]->Clone(Form("%sC", hist[iw]->GetName()));
    hdedxhitC->GetXaxis()->SetRange(minbin, maxbin);
    hdedxhitC->SetFillColor(kAzure + 1);
    hdedxhitC->DrawCopy("same histo");

    if (((iw + 1) % 16 == 0) || iw == (c_nwireCDC - 1))  {
      ctmp.Print(psname.str().c_str());
      ctmp.Clear("D");
    }
    delete hist[iw];
    delete hdedxhitC;
  }

  psname.str("");
  psname << Form("cdcdedx_wgcal_%s.pdf]", m_suffix.data());
  ctmp.Print(psname.str().c_str());
}

//------------------------------------
void CDCDedxWireGainAlgorithm::plotWireGain(vector<double>& vdedx_means, vector<double>& vrel_mean, double layeravg)
{

  //saving final constants in a histograms for validation
  TCanvas cwconst("cwconst", "", 900, 500);
  TCanvas cwconstvar("cwconstvar", "", 500, 400);

  array<TH1D*, 2> hconstpw, hconstpwvar;

  for (int i = 0; i < 2; i++) {

    hconstpw[i] = new TH1D(Form("hconstpw_%d_%s", i, m_suffix.data()), "", c_nwireCDC, -0.5, 14335.5);
    hconstpw[i]->SetTitle(Form("wiregain const  (%s); wire numbers;<dedxhit>", m_suffix.data()));
    if (m_isMerge
        && i == 0) hconstpw[i]->SetTitle(Form("merged wiregain rel-const  (%s), avg = %0.03f; wire numbers;<dedxhit>", m_suffix.data(),
                                                layeravg));

    hconstpwvar[i] = new TH1D(Form("hconstpwvar_%s", m_suffix.data()), "", 400, -0.5, 2.5);
    hconstpwvar[i]->SetTitle(Form("wiregain const (%s); wire gains; nentries", m_suffix.data()));
    if (m_isMerge
        && i == 0) hconstpwvar[i]->SetTitle(Form("merged wiregain rel-const (%s), avg = %0.03f; wire gains; nentries", m_suffix.data(),
                                                   layeravg));

    for (unsigned int iw = 0; iw < c_nwireCDC; iw++) {

      double gain = vdedx_means.at(iw);
      if (m_isMerge && i == 1) gain =  vrel_mean.at(iw);
      hconstpw[i]->SetBinContent(iw + 1, gain);
      hconstpwvar[i]->Fill(gain);

      if (iw % 500 == 0) hconstpw[i]->GetXaxis()->SetBinLabel(iw + 1, Form("w%d", iw + 1));
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
  if (m_isMerge) hconstpw[1]->Draw("same");

  cwconstvar.cd();
  hconstpwvar[0]->Draw("hist");
  if (m_isMerge) hconstpwvar[1]->Draw("hist same");

  cwconst.SaveAs(Form("cdcdedx_wgcal_wireconst_%s.pdf", m_suffix.data()));
  cwconstvar.SaveAs(Form("cdcdedx_wgcal_wireconstvar_%s.pdf", m_suffix.data()));
}

//------------------------------------
void CDCDedxWireGainAlgorithm::plotLayerGain(array<double, c_maxNSenseLayers>& layermean, double layeravg)
{

  TH1D hlayeravg(Form("hlayeravg_%s", m_suffix.data()), "", c_maxNSenseLayers, -0.5, 55.5);
  hlayeravg.SetTitle(Form("layer gain avg (%s); layer numbers;<dedxhit>", m_suffix.data()));

  for (unsigned int il = 0; il < c_maxNSenseLayers; il++) {
    hlayeravg.SetBinContent(il + 1, layermean[il]);
    if (il % 2 == 0 || il == c_maxNSenseLayers - 1) hlayeravg.GetXaxis()->SetBinLabel(il + 1, Form("L%d", il));
  }

  TCanvas clayeravg("clayeravg", "clayeravg", 800, 500);
  clayeravg.SetGridy(1);
  clayeravg.cd();
  gStyle->SetOptStat("ne");
  hlayeravg.LabelsOption("u", "X");
  hlayeravg.SetLineColor(kBlue);
  hlayeravg.GetYaxis()->SetRangeUser(-0.1, 3.5);
  hlayeravg.SetTitle(Form("%s, avg = %0.03f (abs)", hlayeravg.GetTitle(), layeravg));
  hlayeravg.LabelsDeflate();
  hlayeravg.Draw("");
  TLine* tl = new TLine(-0.5, layeravg, 55.5, layeravg);
  tl->SetLineColor(kRed);
  tl->DrawClone("same");
  clayeravg.SaveAs(Form("cdcdedx_wgcal_layeravg_%s.pdf", m_suffix.data()));
  delete tl;
}

//------------------------------------
void CDCDedxWireGainAlgorithm::plotWGPerLayer(vector<double>& vdedx_means, array<double, c_maxNSenseLayers>& layermean,
                                              double layeravg)
{

  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance(&(*m_cdcGeo));

  TCanvas clconst("clconst", "", 800, 500);
  clconst.Divide(2, 2);
  clconst.SetBatch(kTRUE);

  stringstream psnameL;
  psnameL << Form("cdcdedx_wgcal_layerconst_%s.pdf[", m_suffix.data());
  clconst.Print(psnameL.str().c_str());
  psnameL.str(""); psnameL << Form("cdcdedx_wgcal_layerconst_%s.pdf", m_suffix.data());

  int jwire = 0;

  for (unsigned int il = 0; il < c_maxNSenseLayers; ++il) {

    unsigned int nwires = cdcgeo.nWiresInLayer(il);
    TH1D hconstpl(Form("hconstpwvar_l%d_%s", il, m_suffix.data()), "", nwires, jwire, jwire + nwires);
    hconstpl.SetTitle(Form("abs-const, layer: %d (%s); wire numbers;<dedxhit>", il, m_suffix.data()));

    for (unsigned int iw = 0; iw < nwires; ++iw) {
      hconstpl.SetBinContent(iw + 1, vdedx_means.at(jwire));
      if (il < 32) {
        if (iw % 10 == 0) hconstpl.GetXaxis()->SetBinLabel(iw + 1, Form("w%d", jwire));
      } else {
        if (iw % 15 == 0) hconstpl.GetXaxis()->SetBinLabel(iw + 1, Form("w%d", jwire));
      }
      jwire++;
    }

    double lmean  = layermean.at(il) / layeravg;

    clconst.cd(il % 4 + 1);
    gStyle->SetOptStat("ne");

    hconstpl.SetTitle(Form("%s, avg = %0.03f", hconstpl.GetTitle(), lmean));

    if (il < 8) hconstpl.GetYaxis()->SetRangeUser(-0.1, 4.0);
    else hconstpl.GetYaxis()->SetRangeUser(-0.1, 2.0);
    hconstpl.SetFillColor(kAzure - 1);
    hconstpl.LabelsOption("u", "X");
    hconstpl.DrawCopy("hist");

    TLine* tlc = new TLine(jwire - nwires, lmean, jwire, lmean);
    tlc->SetLineColor(kRed);
    tlc->DrawClone("same");

    if ((il + 1) % 4 == 0 || (il + 1) == c_maxNSenseLayers) {
      clconst.Print(psnameL.str().c_str());
      clconst.Clear("D");
    }

    delete tlc;
  }

  psnameL.str("");
  psnameL << Form("cdcdedx_wgcal_layerconst_%s.pdf]", m_suffix.data());
  clconst.Print(psnameL.str().c_str());
}

//--------------------------
void CDCDedxWireGainAlgorithm::plotEventStats()
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
    htstats->SetName(Form("htstats_%s", m_suffix.data()));
    htstats->SetStats(0);
    htstats->DrawCopy("");
  }

  cstats.Print(Form("cdcdedx_wgcal_stats_%s.pdf", m_suffix.data()));
}