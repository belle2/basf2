/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxCosEdgeAlgorithm.h>
using namespace Belle2;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxCosEdgeAlgorithm::CDCDedxCosEdgeAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  isMakePlots(true),
  isMerge(true),
  m_sigLim(2.5),
  m_npBins(20),
  m_negMin(-0.870),
  m_negMax(-0.850),
  m_posMin(0.950),
  m_posMax(0.960),
  m_dedxBins(600),
  m_dedxMin(0.0),
  m_dedxMax(3.0),
  m_suffix("")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx electron cos(theta) dependence");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxCosEdgeAlgorithm::calibrate()
{

  getExpRunInfo();

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (ttree->GetEntries() < 100)return c_NotEnoughData;

  double dedx, costh; int charge;
  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("costh", &costh);
  ttree->SetBranchAddress("charge", &charge);

  // make histograms to store dE/dx values in bins of cos(theta)
  TH1D* hdedx_negi[m_npBins], *hdedx_posi[m_npBins];

  const double bwnegi = (m_negMax - m_negMin) / m_npBins;
  const double bwposi = (m_posMax - m_posMin) / m_npBins;

  for (unsigned int i = 0; i < m_npBins; ++i) {

    double mincos = i * bwposi + m_posMin;
    double maxcos = mincos + bwposi;
    std::string title = Form("costh: %0.04f, %0.04f(%s)", mincos, maxcos, m_suffix.data());
    hdedx_posi[i] = new TH1D(Form("hdedx_posi%d_%s", i, m_suffix.data()), "", m_dedxBins, m_dedxMin, m_dedxMax);
    hdedx_posi[i]->SetTitle(Form("%s;dedx;entries", title.data()));

    mincos = i * bwnegi + m_negMin;
    maxcos = mincos + bwnegi;
    title = Form("costh: %0.04f, %0.04f(%s)", mincos, maxcos, m_suffix.data());
    hdedx_negi[i] = new TH1D(Form("hdedx_negi%d_%s", i, m_suffix.data()), "", m_dedxBins, m_dedxMin, m_dedxMax);
    hdedx_negi[i]->SetTitle(Form("%s;dedx;entries", title.data()));
  }

  int icosbin = -99.;
  for (int i = 0; i < ttree->GetEntries(); ++i) {

    ttree->GetEvent(i);

    //if track is a junk
    if (dedx <= 0 || charge == 0) continue;
    if (costh > -0.850 && costh < 0.950)continue;

    if (costh > 0) {
      icosbin = int((costh - m_posMin) / bwposi);
      hdedx_posi[icosbin]->Fill(dedx);
    } else if (costh < 0) {
      icosbin = int((costh - m_negMin) / bwnegi);
      hdedx_negi[icosbin]->Fill(dedx);
    }

  }

  std::map<int, std::vector<double>> vneg_fitpars;
  std::map<int, std::vector<double>> vpos_fitpars;

  std::vector<double> vneg_const, vpos_const;
  std::vector<std::vector<double>> vfinal_const;

  for (unsigned int i = 0; i < m_npBins; ++i) {

    std::string fitstatus = "";

    //Fit dedx in negative cos bins
    FitGaussianWRange(hdedx_negi[i], fitstatus);
    if (fitstatus != "FitOK") {
      vneg_fitpars[0].push_back(1.0);
      vneg_fitpars[1].push_back(0.0);
      vneg_fitpars[2].push_back(0.0);
      vneg_fitpars[3].push_back(0.0);
      hdedx_negi[i]->SetTitle(Form("%s, Fit(%s)", hdedx_negi[i]->GetTitle(), fitstatus.data()));
    } else {
      vneg_fitpars[0].push_back(hdedx_negi[i]->GetFunction("gaus")->GetParameter(1));
      vneg_fitpars[1].push_back(hdedx_negi[i]->GetFunction("gaus")->GetParError(1));
      vneg_fitpars[2].push_back(hdedx_negi[i]->GetFunction("gaus")->GetParameter(2));
      vneg_fitpars[3].push_back(hdedx_negi[i]->GetFunction("gaus")->GetParError(2));
    }

    vneg_const.push_back(vneg_fitpars[0][i]);

    //Fit dedx in positive cos bins
    FitGaussianWRange(hdedx_posi[i], fitstatus);
    if (fitstatus != "FitOK") {
      vpos_fitpars[0].push_back(1.0);
      vpos_fitpars[1].push_back(0.0);
      vpos_fitpars[2].push_back(0.0);
      vpos_fitpars[3].push_back(0.0);
      hdedx_posi[i]->SetTitle(Form("%s, Fit(%s)", hdedx_posi[i]->GetTitle(), fitstatus.data()));
    } else {
      vpos_fitpars[0].push_back(hdedx_posi[i]->GetFunction("gaus")->GetParameter(1));
      vpos_fitpars[1].push_back(hdedx_posi[i]->GetFunction("gaus")->GetParError(1));
      vpos_fitpars[2].push_back(hdedx_posi[i]->GetFunction("gaus")->GetParameter(2));
      vpos_fitpars[3].push_back(hdedx_posi[i]->GetFunction("gaus")->GetParError(2));
    }

    vpos_const.push_back(vpos_fitpars[0][i]);
  }

  vfinal_const.push_back(vneg_const);
  vfinal_const.push_back(vpos_const);

  createPayload(vfinal_const);

  if (isMakePlots) {

    //1. draw dedx dist of individual bins
    plotHist(hdedx_posi, vpos_fitpars, "pos");
    plotHist(hdedx_negi, vneg_fitpars, "neg");

    //2. plot relative const or fit parameters
    plotFitPars(vneg_fitpars, vpos_fitpars);

    //3. compare new/old calibration constants
    plotConstants(vfinal_const);

    //4. plot statistics related plots here
    plotStats();
  }

  m_suffix.clear();
  return c_OK;
}


//------------------------------------
void CDCDedxCosEdgeAlgorithm::getExpRunInfo()
{
  int cruns = 0;
  for (auto expRun : getRunList()) {
    if (cruns == 0)B2INFO("CDCDedxCosEdge: start exp " << expRun.first << " and run " << expRun.second << "");
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

//------------------------------------
void CDCDedxCosEdgeAlgorithm::createPayload(std::vector<std::vector<double>>& vfinalconst)
{
  if (isMerge) {
    if (m_DBCosineCor->getSize(-1) != int(m_npBins) || m_DBCosineCor->getSize(1) != int(m_npBins))
      B2FATAL("CDCDedxCosEdgeAlgorithm: Can't merge paylaods with different size");

    for (unsigned int ibin = 0; ibin < m_npBins; ibin++) {

      //costh < 0
      double prevg = m_DBCosineCor->getMean(-1, ibin);
      double relg = vfinalconst[0].at(ibin);
      double newg = prevg * relg;
      B2INFO("CosEdge Const (<0), bin# " << ibin << ", rel " << relg << ", previous " << prevg << ", merged " <<  newg);
      vfinalconst[0].at(ibin) *= (double)m_DBCosineCor->getMean(-1, ibin);
      vfinalconst[0].at(ibin) /= (0.5 * (vfinalconst[0].at(m_npBins - 1) + vfinalconst[0].at(m_npBins - 2)));
      printf("hello\n ");
      //costh > 0
      prevg = m_DBCosineCor->getMean(1, ibin);
      relg = vfinalconst[1].at(ibin);
      newg = prevg * relg;
      B2INFO("CosEdge Const (>0), bin# " << ibin << ", rel " << relg << ", previous " << prevg << ", merged " <<  newg);
      vfinalconst[1].at(ibin) *= (double)m_DBCosineCor->getMean(1, ibin);
      vfinalconst[1].at(ibin) /= (0.5 * (vfinalconst[1].at(0) + vfinalconst[1].at(1)));
    }
  }

  B2INFO("CDCDedxCosineEdge calibration done");
  CDCDedxCosineEdge* gain = new CDCDedxCosineEdge(vfinalconst);
  saveCalibration(gain, "CDCDedxCosineEdge");
}

//------------------------------------
void CDCDedxCosEdgeAlgorithm::FitGaussianWRange(TH1D*& temphist, std::string& status)
{
  if (temphist->Integral() < 500) { //atleast 1k bhabha events
    B2INFO(Form("\t insufficient fit stats (%0.00f) for (%s)", temphist->Integral(), temphist->GetName()));
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
      double fdEdxMean = temphist->GetFunction("gaus")->GetParameter(1);
      double width = temphist->GetFunction("gaus")->GetParameter(2);
      temphist->GetXaxis()->SetRangeUser(fdEdxMean - 5.0 * width, fdEdxMean + 5.0 * width);
      fs = temphist->Fit("gaus", "QR", "", fdEdxMean - m_sigLim * width, fdEdxMean + m_sigLim * width);
      if (fs != 0) {
        B2INFO(Form("\tFit (round 2) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
        status = "FitFailed";
        return;
      } else {
        temphist->GetXaxis()->SetRangeUser(fdEdxMean - 5.0 * width, fdEdxMean + 5.0 * width);
        B2INFO(Form("\tFit for hist (%s) sucessfull (status = %d)", temphist->GetName(), fs));
        status = "FitOK";
      }
    }
  }
}

//------------------------------------
void CDCDedxCosEdgeAlgorithm::plotHist(TH1D* hdedx[], std::map<int, std::vector<double>> vpars, std::string type)
{
  TCanvas* ctmp = new TCanvas("ctmp", "ctmp", 1200, 1200);
  ctmp->Divide(5, 4);

  std::stringstream psname;
  psname << Form("cdcdedx_cosedgecal_fits_%s_%s.pdf[", type.data(), m_suffix.data());
  ctmp->Print(psname.str().c_str());
  psname.str("");
  psname << Form("cdcdedx_cosedgecal_fits_%s_%s.pdf", type.data(), m_suffix.data());

  for (unsigned int i = 0; i < m_npBins; ++i) {

    ctmp->cd(i % 20 + 1); // each canvas is 2x2
    hdedx[i]->SetStats(0);
    hdedx[i]->SetFillColor(kAzure - 9);
    hdedx[i]->DrawCopy();

    TPaveText* pt = new TPaveText(0.5, 0.73, 0.8, 0.89, "NBNDC");
    setTextCosmetics(pt, 0.04258064);
    pt->AddText(Form("#mu_{fit}: %0.03f#pm%0.03f", vpars[0][i], vpars[1][i]));
    pt->AddText(Form("#sigma_{fit}: %0.03f", vpars[2][i]));
    pt->Draw("same");

    if ((i + 1) % 20 == 0 || (i + 1) == m_npBins) {
      ctmp->Print(psname.str().c_str());
      ctmp->Clear("D");
    }
  }
  psname.str("");
  psname << Form("cdcdedx_cosedgecal_fits_%s_%s.pdf]", type.data(), m_suffix.data());
  ctmp->Print(psname.str().c_str());
  delete ctmp;
}

//------------------------------------
void CDCDedxCosEdgeAlgorithm::plotFitPars(std::map<int, std::vector<double>> vneg_fitpars,
                                          std::map<int, std::vector<double>> vpos_fitpars)
{
  // For qa pars
  TCanvas* cQa = new TCanvas("cQa", "cQa", 1200, 1200);
  cQa->Divide(2, 2);

  double min[2] = {0.85, 0.04};
  double max[2] = {1.05, 0.3};

  std::string vars[2] = {"#mu_{fit}", "#sigma_{fit}"};
  std::string side[2] = {"pcos", "ncos"};

  TH1D* htemp = 0x0;
  for (int is = 0; is < 2; is++) {
    for (int iv = 0; iv < 2; iv++) {
      std::string hname = Form("hpar_%s_%s_%s", vars[iv].data(), side[is].data(), m_suffix.data());
      if (is == 0) {
        htemp  = new TH1D(Form("%s", hname.data()), "", m_npBins, m_posMin, m_posMax);
        htemp->SetTitle(Form("Constant (%s), cos#theta:(%0.02f, %0.02f);cos(#theta);const", vars[iv].data(), m_posMin, m_posMax));
      } else {
        htemp  = new TH1D(Form("%s", hname.data()), "", m_npBins, m_negMin, m_negMax);
        htemp->SetTitle(Form("Constant (%s), cos#theta:(%0.02f, %0.02f);cos(#theta);const", vars[iv].data(), m_negMin, m_negMax));
      }

      for (unsigned int ib = 0; ib < m_npBins; ib++) {
        if (is == 0) {
          htemp->SetBinContent(ib + 1, vpos_fitpars[2 * iv][ib]);
          htemp->SetBinError(ib + 1, vpos_fitpars[2 * iv + 1][ib]);
        } else {
          htemp->SetBinContent(ib + 1, vneg_fitpars[2 * iv][ib]);
          htemp->SetBinError(ib + 1, vneg_fitpars[2 * iv + 1][ib]);
        }
      }
      setHistCosmetics(htemp, iv * 2 + 2, min[iv], max[iv], 0.03);
      cQa->cd(2 * iv + 1 + is); //1,3,2,4
      gPad->SetGridx(1);
      gPad->SetGridy(1);
      htemp->DrawCopy("");
    }
  }
  cQa->SaveAs(Form("cdcdedx_cosedgecal_relconst_%s.pdf", m_suffix.data()));
  cQa->SaveAs(Form("cdcdedx_cosedgecal_relconst_%s.root", m_suffix.data()));
  delete htemp;
}

//------------------------------------
void CDCDedxCosEdgeAlgorithm::plotConstants(std::vector<std::vector<double>> vfinalconst)
{

  TCanvas ctmp_const("ctmp_const", "ctmp_const", 900, 450);
  ctmp_const.Divide(2, 1);

  TH1D* holdconst[2], *hnewconst[2];

  for (int i = 0; i < 2; i++) {

    double min = m_negMin, max = m_negMax;
    if (i == 1) {
      min = m_posMin;
      max = m_posMax;
    }

    holdconst[i] = new TH1D(Form("holdconst%d_%s", i, m_suffix.data()), "", m_npBins, min, max);
    holdconst[i]->SetTitle(Form("constant comparision, cos#theta:(%0.02f, %0.02f);cos(#theta);const", min, max));

    hnewconst[i] = new TH1D(Form("hnewconst%d_%s", i, m_suffix.data()), "", m_npBins, min, max);

    int iside = 2 * i - 1; //-1 or +1 for neg and pos cosine side
    for (int ibin = 0; ibin < m_DBCosineCor->getSize(iside); ibin++) {
      holdconst[i]->SetBinContent(ibin + 1, (double)m_DBCosineCor->getMean(iside, ibin));
      hnewconst[i]->SetBinContent(ibin + 1, vfinalconst[i].at(ibin));
    }

    ctmp_const.cd(i + 1);
    gPad->SetGridx(1);
    gPad->SetGridy(1);
    setHistCosmetics(holdconst[i], kBlack, 0., 1.10, 0.025);
    holdconst[i]->DrawCopy("");
    setHistCosmetics(hnewconst[i], kRed, 0., 1.10, 0.025);
    hnewconst[i]->DrawCopy("same");

    auto legend = new TLegend(0.4, 0.75, 0.56, 0.85);
    legend->AddEntry(holdconst[i], "Old", "lep");
    legend->AddEntry(hnewconst[i], "New", "lep");
    legend->Draw();
  }

  ctmp_const.SaveAs(Form("cdcdedx_cosedgecal_constcomp_%s.pdf", m_suffix.data()));
  ctmp_const.SaveAs(Form("cdcdedx_cosedgecal_constcomp_%s.root", m_suffix.data()));
}

//------------------------------------
void CDCDedxCosEdgeAlgorithm::plotStats()
{

  TCanvas* cstats = new TCanvas("cstats", "cstats", 1000, 500);
  cstats->SetBatch(kTRUE);
  cstats->Divide(2, 1);

  cstats->cd(1);
  auto hestats = getObjectPtr<TH1I>("hestats");
  if (hestats) {
    hestats->SetName(Form("hestats_%s", m_suffix.data()));
    hestats->SetStats(0);
    hestats->DrawCopy("");
  }

  cstats->cd(2);
  auto htstats = getObjectPtr<TH1I>("htstats");
  if (htstats) {
    hestats->SetName(Form("htstats_%s", m_suffix.data()));
    htstats->DrawCopy("");
    hestats->SetStats(0);
  }
  cstats->Print(Form("cdcdedx_cosedgecal_stats_%s.pdf", m_suffix.data()));
  delete cstats;
}