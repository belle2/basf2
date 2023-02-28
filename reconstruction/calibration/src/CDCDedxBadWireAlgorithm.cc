/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxBadWireAlgorithm.h>

using namespace Belle2;
using namespace CDC;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxBadWireAlgorithm::CDCDedxBadWireAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  c_nwireCDC(c_nSenseWires),
  m_isMakePlots(true),
  m_isADC(false),
  m_varBins(100),
  m_varMin(0.0),
  m_varMax(7.0),
  m_meanThres(1.0),
  m_rmsThres(1.0),
  m_fracThres(1.0),
  m_varName("hitdedx"),
  m_suffix("")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx bad wires");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxBadWireAlgorithm::calibrate()
{

  getExpRunInfo();

  //old wg for book-keeping previous bad+dead
  if (!m_DBBadWires.isValid() || !m_DBWireGains.isValid())
    B2FATAL("There is no valid payload for BadWire and/or Wirgain");

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (ttree->GetEntries() < 1000) return c_NotEnoughData;

  vector<int>* wire = 0;
  ttree->SetBranchAddress("wire", &wire);

  vector<double>* hitvar = 0;
  if (m_isADC) ttree->SetBranchAddress("adccorr", &hitvar);
  else ttree->SetBranchAddress("dedxhit", &hitvar);

  if (m_isADC) m_varName = "hitadc";
  m_suffix = Form("%s_%s", m_varName.data(), m_suffix.data());

  TH1D hvarall(Form("hvarall_%s", m_suffix.data()), "", m_varBins, m_varMin, m_varMax);
  hvarall.SetTitle(Form("dist %s; %s; %s", m_suffix.data(), m_varName.data(), "entries"));

  map<int, vector<double>> vhitvar;

  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int ih = 0; ih < wire->size(); ++ih) {
      double ivalue = hitvar->at(ih);
      vhitvar[wire->at(ih)].push_back(ivalue);
      hvarall.Fill(ivalue);
    }
  }

  m_amean = hvarall.GetMean();
  m_arms = hvarall.GetRMS();

  //return if >5% bad wire or null histogram
  int minstat = 0;
  for (unsigned int jw = 0; jw < c_nwireCDC; ++jw)
    if (vhitvar[jw].size() <= 100) minstat++;

  if (minstat > 0.05 * c_nwireCDC || m_amean == 0 || m_arms == 0)  return c_NotEnoughData;

  map<int, vector<double>> qapars;
  vector<double> vdefectwires, vbadwires, vdeadwires;

  for (unsigned int jw = 0; jw < c_nwireCDC; ++jw) {
    int ncount = 0, tcount = 0;
    double nmean = 0.;
    for (unsigned int jh = 0; jh < vhitvar[jw].size(); ++jh) {
      double jvalue = vhitvar[jw][jh];
      if (jvalue < m_varMax) {
        ncount++;
        nmean += jvalue;
      } else tcount++;
    }

    bool badwire = false;
    if (ncount < 100) {
      qapars[0].push_back(0);
      qapars[1].push_back(0);
      qapars[2].push_back(0);
      badwire = true; //partial dead
    } else {
      nmean = nmean / ncount;
      if (abs(nmean - m_amean) / m_amean > m_meanThres) badwire = true;

      double nrms = 0.;
      for (unsigned int kh = 0; kh < vhitvar[jw].size(); ++kh) {
        double kvalue = vhitvar[jw][kh];
        if (kvalue < m_varMax)  nrms += pow(kvalue - nmean, 2);
      }

      nrms = sqrt(nrms / ncount);
      if (abs(nrms - m_arms) / m_arms > m_rmsThres) badwire = true;

      double badfrac = 0.0;
      if (tcount > 0) badfrac = (1.0 * tcount) / (tcount + ncount);
      if (badfrac > m_fracThres) badwire = true;

      qapars[0].push_back(nmean);
      qapars[1].push_back(nrms);
      qapars[2].push_back(badfrac);
    }

    if (badwire) {
      vdefectwires.push_back(0.0);
      if (ncount == 0) vdeadwires.push_back(jw);
      else vbadwires.push_back(jw);
    } else vdefectwires.push_back(1.0);
  }


  if (m_isMakePlots) {
    //1. plot bad and good wire plots.
    plotWireDist(vbadwires, vhitvar);

    //2. plots wire status map
    plotBadWireMap(vbadwires, vdeadwires);

    //3. plot control parameters histograms
    plotQaPars(qapars);

    //4. plot statistics related histograms
    plotEventStats();
  }

  // Save payloads
  B2INFO("dE/dx Badwire Calibration done: " << vdefectwires.size() << " wires");
  CDCDedxBadWires* c_badwires = new CDCDedxBadWires(vdefectwires);
  saveCalibration(c_badwires, "CDCDedxBadWires");

  m_suffix.clear();

  return c_OK;
}

//------------------------------------
void CDCDedxBadWireAlgorithm::getExpRunInfo()
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
  int rend = erEnd.second;

  updateDBObjPtrs(1, rstart, estart);

  if (m_suffix.length() > 0) m_suffix = Form("%s_e%d_r%dr%d", m_suffix.data(), estart, rstart, rend);
  else  m_suffix = Form("e%d_r%dr%d", estart, rstart, rend);
}


//------------------------------------
void CDCDedxBadWireAlgorithm::plotWireDist(const vector<double>& inwires,
                                           map<int, vector<double>>& vhitvar)
{

  TList* bdlist = new TList();
  bdlist->SetName("badwires");

  TList* goodlist = new TList();
  goodlist->SetName("goodwires");

  TList* hflist = new TList();
  hflist->SetName("highfracwires");

  for (unsigned int jw = 0; jw < c_nwireCDC; ++jw) {

    TH1D* hvar = new TH1D(Form("%s_wire%d", m_suffix.data(), jw), "", m_varBins, m_varMin, m_varMax);

    TH1D* hvarhf = new TH1D(Form("hf%s_wire%d", m_suffix.data(), jw), "", m_varBins, m_varMin, m_varMax);
    hvarhf->SetTitle(Form("%s, wire = %d; %s; entries", m_suffix.data(), jw, m_varName.data()));

    int ncount = 0, tcount = 0;

    for (unsigned int jh = 0; jh < vhitvar[jw].size(); ++jh) {
      double jvalue = vhitvar[jw][jh];
      if (jvalue < m_varMax) {
        ncount++;
        hvar->Fill(jvalue);
      } else {
        tcount++;
        if (jvalue < m_varMax * 10.) hvarhf->Fill(jvalue / 10.);
      }
    }

    double badfrac = 0.0;
    if (tcount > 0) badfrac = (1.0 * tcount) / (tcount + ncount);
    hvar->SetTitle(Form("%s, wire = %d; %s; %0.01f", m_suffix.data(), jw, m_varName.data(), badfrac * 100));

    bool isbad = false;
    if (count(inwires.begin(), inwires.end(), jw)) isbad = true;

    double oldwg = m_DBWireGains->getWireGain(jw);
    if (oldwg == 0) {
      hvar->SetLineWidth(2);
      hvar->SetLineColor(kRed);
    }

    if (isbad) {
      bdlist->Add(hvar);
      hflist->Add(hvarhf);
    } else {
      if (hvar->Integral() > 100) goodlist->Add(hvar);
    }
  }

  printCanvas(bdlist, hflist, kYellow - 9);
  printCanvas(goodlist, hflist, kGreen);

  delete bdlist;
  delete goodlist;
  delete hflist;
}

//------------------------------------
void CDCDedxBadWireAlgorithm::printCanvas(TList* list, TList* hflist, Color_t color)
{

  string listname = list->GetName();
  string sfx = Form("%s_%s", listname.data(), m_suffix.data());

  TCanvas ctmp(Form("cdcdedx_%s", sfx.data()), "", 1200, 1200);
  ctmp.Divide(4, 4);
  ctmp.SetBatch(kTRUE);

  stringstream psname;
  psname << Form("cdcdedx_bdcal_%s.pdf[", sfx.data());
  ctmp.Print(psname.str().c_str());
  psname.str("");
  psname << Form("cdcdedx_bdcal_%s.pdf", sfx.data());

  for (int ih = 0; ih < list->GetSize(); ih++) {

    TH1D* hist = (TH1D*)list->At(ih);

    double frac = stod(hist->GetYaxis()->GetTitle());

    TPaveText* pinfo = new TPaveText(0.40, 0.63, 0.89, 0.89, "NBNDC");
    pinfo->AddText(Form("#mu: %0.2f(%0.2f#pm%0.2f)", hist->GetMean(), m_amean, m_meanThres * m_amean));
    pinfo->AddText(Form("#sigma: %0.2f(%0.2f#pm%0.2f)", hist->GetRMS(), m_arms, m_rmsThres * m_arms));
    pinfo->AddText(Form("N: %0.00f", hist->Integral()));
    pinfo->AddText(Form("hf: %0.00f%%(%0.00f%%)", frac, m_fracThres * 100));
    setTextCosmetics(pinfo, 0.04258064);

    ctmp.cd(ih % 16 + 1);
    hist->GetYaxis()->SetTitle("entries");
    hist->SetFillColor(color);
    hist->SetStats(0);
    hist->Draw();
    pinfo->Draw("same");

    if (listname == "badwires") {
      TH1D* histhf = (TH1D*)hflist->At(ih);
      if (hist->GetMaximum() < histhf->GetMaximum()) hist->SetMaximum(histhf->GetMaximum() * 1.05);
      histhf->SetFillColor(kGray);
      histhf->SetStats(0);
      histhf->Draw("same");
    }

    if (((ih + 1) % 16 == 0) || ih == (list->GetSize() - 1))  {
      ctmp.Print(psname.str().c_str());
      ctmp.Clear("D");
    }
  }

  psname.str("");
  psname << Form("cdcdedx_bdcal_%s.pdf]", sfx.data());
  ctmp.Print(psname.str().c_str());
}

//------------------------------------
void CDCDedxBadWireAlgorithm::plotBadWireMap(const vector<double>& vbadwires, const vector<double>& vdeadwires)
{

  TCanvas cmap(Form("cmap_%s", m_suffix.data()), "", 800, 800);
  cmap.SetTitle("CDC dE/dx bad wire status");

  int total = 0;
  TH2F* hxyAll = getHistoPattern(vbadwires, "all", total);
  hxyAll->SetTitle(Form("wire status map (%s)", m_suffix.data()));
  setHistCosmetics(hxyAll, kGray);
  hxyAll->Draw();

  int nbad = 0.0;
  TH2F* hxyBad = getHistoPattern(vbadwires, "bad", nbad);
  if (hxyBad) {
    setHistCosmetics(hxyBad, kRed);
    hxyBad->Draw("same");
  }

  int ndead = 0.0;
  TH2F* hxyDead = getHistoPattern(vdeadwires, "dead", ndead);
  if (hxyDead) {
    setHistCosmetics(hxyDead, kBlack);
    hxyDead->Draw("same");
  }

  int ndefect = nbad + ndead;
  auto leg = new TLegend(0.68, 0.80, 0.90, 0.92);
  leg->SetBorderSize(0);
  leg->SetLineWidth(3);
  leg->SetHeader(Form("total defective: %d (~%0.02f%%)", ndefect, 100.*(ndefect) / c_nwireCDC));
  leg->AddEntry(hxyBad, Form("bad #rightarrow %d", nbad), "p");
  leg->AddEntry(hxyDead, Form("dead #rightarrow %d", ndead), "p");
  leg->Draw();

  gStyle->SetLegendTextSize(0.025);
  TPaveText* pt = new TPaveText(-0.30, -1.47, -0.31, -1.30, "br");
  setTextCosmetics(pt, 0.02258064);

  TText* text = pt->AddText("CDC-wire map: counter-clockwise and start from +x");
  text->SetTextColor(kGray + 1);
  pt->Draw("same");

  cmap.SaveAs(Form("cdcdedx_bdcal_wiremap_%s.pdf", m_suffix.data()));

  delete hxyAll;
  delete hxyBad;
  delete hxyDead;
}

//------------------------------------
TH2F* CDCDedxBadWireAlgorithm::getHistoPattern(const vector<double>& inwires, const string& suffix, int& total)
{

  B2INFO("Creating CDCGeometryPar object");
  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance(&(*m_cdcGeo));

  TH2F* temp = new TH2F(Form("temp_%s_%s", m_suffix.data(), suffix.data()), "", 2400, -1.2, 1.2, 2400, -1.2, 1.2);

  int jwire = -1;
  total = 0;
  for (unsigned int ilay = 0; ilay < c_maxNSenseLayers; ++ilay) {
    for (unsigned int iwire = 0; iwire < cdcgeo.nWiresInLayer(ilay); ++iwire) {
      jwire++;
      double phi = 2.*TMath::Pi() * (float(iwire) / float(cdcgeo.nWiresInLayer(ilay)));
      double radius = cdcgeo.senseWireR(ilay) / 100.;
      double x = radius * cos(phi);
      double y = radius * sin(phi);
      if (suffix == "all") {
        total++;
        temp->Fill(x, y);
      } else {
        if (count(inwires.begin(), inwires.end(), jwire)) {
          temp->Fill(x, y);
          total++;
        }
      }
    }
  }
  return temp;
}

//------------------------------------
void CDCDedxBadWireAlgorithm::plotQaPars(map<int, vector<double>>& qapars)
{

  string qaname[3] = {"mean", "rms", "high_fraction"};

  double linemin[3] = {m_amean* (1 - m_meanThres), m_arms* (1 - m_rmsThres), m_fracThres * 100};
  double linemax[3] = {m_amean* (1 + m_meanThres), m_arms* (1 + m_rmsThres), m_fracThres * 100};

  for (int iqa = 0; iqa < 3; iqa++) {

    TH1D histqa(Form("%s_%s", qaname[iqa].data(), m_suffix.data()), "", c_nwireCDC, -0.5, 14335.5);

    for (unsigned int jw = 0; jw < c_nwireCDC; jw++) {
      if (iqa == 2) histqa.SetBinContent(jw + 1, qapars[iqa][jw] * 100);
      else histqa.SetBinContent(jw + 1, qapars[iqa][jw]);
    }

    TCanvas c_pars(Form("c_pars_%d", iqa), "", 800, 600);
    c_pars.cd();
    gPad->SetGridy();

    histqa.SetTitle(Form("%s vs wires (%s); wire ; %s", qaname[iqa].data(), m_suffix.data(), qaname[iqa].data()));
    histqa.SetStats(0);
    histqa.Draw();

    TLine* lmin = new TLine(-0.5, linemin[iqa], 14335.5, linemin[iqa]);
    lmin->SetLineColor(kRed);
    lmin->Draw("same");
    TLine* lmax = new TLine(-0.5, linemax[iqa], 14335.5, linemax[iqa]);
    lmax->SetLineColor(kRed);
    lmax->Draw("same");

    c_pars.Print(Form("cdcdedx_bdcal_%s_%s.root", qaname[iqa].data(), m_suffix.data()));
    c_pars.Print(Form("cdcdedx_bdcal_%s_%s.pdf", qaname[iqa].data(), m_suffix.data()));

    delete lmax;
    delete lmin;
  }
}

//------------------------------------
void CDCDedxBadWireAlgorithm::plotEventStats()
{

  TCanvas cstats("cstats", "cstats", 1000, 500);
  cstats.SetBatch(kTRUE);
  cstats.Divide(2, 1);

  cstats.cd(1);
  auto hestats = getObjectPtr<TH1I>("hestats");
  if (hestats) {
    hestats->SetName(Form("htstats_%s", m_suffix.data()));
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

  cstats.Print(Form("cdcdedx_bdcal_qastats_%s.pdf", m_suffix.data()));
}