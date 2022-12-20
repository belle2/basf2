/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxBadWireAlgorithm.h>

using namespace std;
using namespace Belle2;
using namespace CDC;


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxBadWireAlgorithm::CDCDedxBadWireAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  c_nwireCDC(c_nSenseWires),
  isMakePlots(true),
  isADC(false),
  m_varBins(100),
  m_varMin(0.0),
  m_varMax(7.0),
  m_meanThers(1.0),
  m_rmsThers(1.0),
  m_fracThers(1.0),
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
  if (ttree->GetEntries() < 1000)return c_NotEnoughData;

  std::vector<int>* wire = 0;
  ttree->SetBranchAddress("wire", &wire);

  std::vector<double>* hitvar = 0;
  if (isADC)ttree->SetBranchAddress("adccorr", &hitvar);
  else ttree->SetBranchAddress("dedxhit", &hitvar);

  if (isADC)m_varName = "hitadc";
  m_suffix = Form("%s_%s", m_varName.data(), m_suffix.data());

  TH1D hvarall(Form("hvarall_%s", m_suffix.data()), "", m_varBins, m_varMin, m_varMax);
  hvarall.SetTitle(Form("dist %s; %s; %s", m_suffix.data(), m_varName.data(), "entries"));

  std::map<int, std::vector<double>> vhitvar;

  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    for (unsigned int ih = 0; ih < wire->size(); ++ih) {
      double ivalue = hitvar->at(ih);
      vhitvar[wire->at(ih)].push_back(ivalue);
      hvarall.Fill(ivalue);
    }
  }

  double amean = hvarall.GetMean();
  double arms = hvarall.GetRMS();

  //return if >5% bad wire or null histogram
  int minstat = 0;
  for (unsigned int jw = 0; jw < c_nwireCDC; ++jw)
    if (vhitvar[jw].size() <= 100) minstat++;
  if (minstat > 0.05 * c_nwireCDC || amean == 0 || arms == 0)  return c_NotEnoughData;

  std::map<int, std::vector<double>> qapars;
  std::vector<double> m_vdefectwires, m_badwires, m_deadwires;

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
      if (std::abs(nmean - amean) / amean > m_meanThers)badwire = true;
      double nrms = 0.;
      for (unsigned int kh = 0; kh < vhitvar[jw].size(); ++kh) {
        double kvalue = vhitvar[jw][kh];
        if (kvalue < m_varMax)  nrms += std::pow(kvalue - nmean, 2);
      }

      nrms = sqrt(nrms / ncount);
      if (std::abs(nrms - arms) / arms > m_rmsThers)badwire = true;

      double badfrac = 0.0;
      if (tcount > 0)badfrac = (1.0 * tcount) / (tcount + ncount);
      if (badfrac > m_fracThers)badwire = true;

      qapars[0].push_back(nmean);
      qapars[1].push_back(nrms);
      qapars[2].push_back(badfrac);
    }

    if (badwire) {
      m_vdefectwires.push_back(0.0);
      if (ncount == 0)m_deadwires.push_back(jw);
      else m_badwires.push_back(jw);
    } else m_vdefectwires.push_back(1.0);
  }


  if (isMakePlots) {
    //1. plot bad and good wire plots.
    plotWireDist(m_badwires, vhitvar, amean, arms);

    //2. plots wire status map
    plotBadWireMap(m_badwires, m_deadwires);

    //3. plot control parameters histograms
    plotQaPars(qapars, amean, arms);

    //4. plot statistics related histograms
    plotEventStats();
  }

  // Save payloads
  B2INFO("dE/dx Badwire Calibration done: " << m_vdefectwires.size() << " wires");
  CDCDedxBadWires* c_badwires = new CDCDedxBadWires(m_vdefectwires);
  saveCalibration(c_badwires, "CDCDedxBadWires");

  m_suffix.clear();

  return c_OK;
}

//------------------------------------
void CDCDedxBadWireAlgorithm::getExpRunInfo()
{

  int cruns = 0;
  for (auto expRun : getRunList()) {
    if (cruns == 0)B2INFO("CDCDedxBadWires: start exp " << expRun.first << " and run " << expRun.second << "");
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
void CDCDedxBadWireAlgorithm::plotWireDist(std::vector<double> m_inwires, std::map<int, std::vector<double>> vhitvar, double amean,
                                           double arms)
{

  TList* bdlist = new TList();
  bdlist->SetName("badwires");

  TList* goodlist = new TList();
  goodlist->SetName("goodwires");

  TList* hflist = new TList();
  hflist->SetName("highfracwires");

  for (unsigned int jw = 0; jw < c_nwireCDC; ++jw) {

    TH1D* hvar = new TH1D(Form("%s_wire%d", m_suffix.data(), jw), "", m_varBins, m_varMin, m_varMax);
    hvar->SetTitle(Form("%s, wire = %d; %s; entries", m_suffix.data(), jw, m_varName.data()));

    TH1D* hvarhf = new TH1D(Form("hf%s_wire%d", m_suffix.data(), jw), "", m_varBins, m_varMin, m_varMax);
    hvarhf->SetTitle(Form("%s, wire = %d; %s; entries", m_suffix.data(), jw, m_varName.data()));

    for (unsigned int jh = 0; jh < vhitvar[jw].size(); ++jh) {
      double jvalue = vhitvar[jw][jh];
      if (jvalue < m_varMax) hvar->Fill(jvalue);
      else if (jvalue < m_varMax * 10.) hvarhf->Fill(jvalue / 10.);
    }

    bool isbad = false;
    if (std::count(m_inwires.begin(), m_inwires.end(), jw)) isbad = true;

    double oldwg = m_DBWireGains->getWireGain(jw);
    if (oldwg == 0) {
      hvar->SetLineWidth(2);
      hvar->SetLineColor(kRed);
    }

    if (isbad) {
      bdlist->Add(hvar);
      hflist->Add(hvarhf);
    } else {
      if (hvar->Integral() > 100)goodlist->Add(hvar);
    }
  }

  printCanvas(bdlist, hflist, kYellow - 9, amean, arms);
  printCanvas(goodlist, hflist, kGreen, amean, arms);

  delete bdlist;
  delete goodlist;
  delete hflist;
}

//------------------------------------
void CDCDedxBadWireAlgorithm::printCanvas(TList* list, TList* hflist, Color_t color, double amean, double arms)
{

  std::string listname = list->GetName();
  std::string sfx = Form("%s_%s", listname.data(), m_suffix.data());

  TCanvas* ctmp = new TCanvas(Form("cdcdedx_%s", sfx.data()), "", 1200, 1200);
  ctmp->Divide(4, 4);
  ctmp->SetBatch(kTRUE);

  std::stringstream psname;
  psname << Form("cdcdedx_bdcal_%s.pdf[", sfx.data());
  ctmp->Print(psname.str().c_str());
  psname.str("");
  psname << Form("cdcdedx_bdcal_%s.pdf", sfx.data());

  for (int ih = 0; ih < list->GetSize(); ih++) {

    TH1D* hist = (TH1D*)list->At(ih);

    TPaveText* pinfo = new TPaveText(0.35, 0.63, 0.83, 0.89, "NBNDC");
    pinfo->AddText(Form("#mu: %0.2f(%0.2f#pm%0.2f)", hist->GetMean(), amean, m_meanThers * amean));
    pinfo->AddText(Form("#sigma: %0.2f(%0.2f#pm%0.2f)", hist->GetRMS(), arms, m_rmsThers * arms));
    pinfo->AddText(Form("N: %0.00f", hist->Integral()));
    setTextCosmetics(pinfo, 0.04258064);

    ctmp->cd(ih % 16 + 1);
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
      ctmp->Print(psname.str().c_str());
      ctmp->Clear("D");
    }
  }

  psname.str("");
  psname << Form("cdcdedx_bdcal_%s.pdf]", sfx.data());
  ctmp->Print(psname.str().c_str());
  delete ctmp;
}


//------------------------------------
void CDCDedxBadWireAlgorithm::plotBadWireMap(std::vector<double> m_badwires, std::vector<double> m_deadwires)
{

  TCanvas* cmap = new TCanvas(Form("cmap_%s", m_suffix.data()), "", 800, 800);
  cmap->SetTitle("CDC dE/dx bad wire status");

  const std::vector<double> m_allwires;
  int total = 0;
  TH2F* hxyAll = getHistoPattern(m_allwires, "all", total);
  hxyAll->SetTitle(Form("wire status map (%s)", m_suffix.data()));
  setHistCosmetics(hxyAll, kGray);
  hxyAll->Draw();

  int nbad = 0.0;
  TH2F* hxyBad = getHistoPattern(m_badwires, "bad", nbad);
  if (hxyBad) {
    setHistCosmetics(hxyBad, kRed);
    hxyBad->Draw("same");
  }

  int ndead = 0.0;
  TH2F* hxyDead = getHistoPattern(m_deadwires, "dead", ndead);
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

  cmap->SaveAs(Form("cdcdedx_bdcal_wiremap_%s.pdf", m_suffix.data()));
  delete cmap;
}

//------------------------------------
TH2F* CDCDedxBadWireAlgorithm::getHistoPattern(std::vector<double> m_inwires, const std::string& suffix, int& total)
{

  B2INFO("Creating CDCGeometryPar object");
  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance(&(*m_cdcGeo));

  std::ofstream outfile;
  std::string outname = Form("cdcdedx_bdcal_%slist_%s.txt", suffix.data(), m_suffix.data());
  outfile.open(outname);

  TH2F* temp = new TH2F(Form("temp_%s_%s", m_suffix.data(), suffix.data()), "", 2400, -1.2, 1.2, 2400, -1.2, 1.2);

  Int_t jwire = -1;
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
        outfile << jwire << std::endl;
      } else {
        if (std::count(m_inwires.begin(), m_inwires.end(), jwire)) {
          temp->Fill(x, y);
          total++;
          outfile << jwire << std::endl;;
        }
      }
    }
  }
  outfile.close();
  return temp;
}

//------------------------------------
void CDCDedxBadWireAlgorithm::plotQaPars(std::map<int, std::vector<double>> qapars, double amean, double arms)
{

  TH1D* histqa[3];
  std::string qaname[3] = {"mean", "rms", "high fraction"};

  for (int i = 0; i < 3; i++) histqa[i] = new TH1D(Form("%s_%s", qaname[i].data(), m_suffix.data()), "", c_nwireCDC, -0.5, 14335.5);

  for (unsigned int jw = 0; jw < c_nwireCDC; jw++) {
    histqa[0]->SetBinContent(jw + 1, qapars[0][jw]);
    histqa[1]->SetBinContent(jw + 1, qapars[1][jw]);
    histqa[2]->SetBinContent(jw + 1, qapars[2][jw]);
  }

  TCanvas c_pars("c_pars", "", 600, 1200);
  c_pars.Divide(1, 3);
  double linemin[3] = {amean* (1 - m_meanThers), arms* (1 - m_rmsThers), m_fracThers};
  double linemax[3] = {amean* (1 + m_meanThers), arms* (1 + m_rmsThers), m_fracThers};

  for (int iqa = 0; iqa < 3; iqa++) {
    c_pars.cd(iqa + 1);
    gPad->SetGridy();
    histqa[iqa]->SetTitle(Form("%s vs wires (%s); wire ; %s", qaname[iqa].data(), m_suffix.data(), qaname[iqa].data()));
    histqa[iqa]->SetStats(0);
    histqa[iqa]->Draw();
    TLine* lmin = new TLine(-0.5, linemin[iqa], 14335.5, linemin[iqa]);
    lmin->SetLineColor(kRed);
    lmin->Draw("same");
    TLine* lmax = new TLine(-0.5, linemax[iqa], 14335.5, linemax[iqa]);
    lmax->SetLineColor(kRed);
    lmax->Draw("same");
  }
  c_pars.Print(Form("cdcdedx_bdcal_qapars_%s.pdf", m_suffix.data()));
  c_pars.Print(Form("cdcdedx_bdcal_qapars_%s.root", m_suffix.data()));
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
    hestats->SetName(Form("htstats_%s", m_suffix.data()));
    htstats->DrawCopy("");
    hestats->SetStats(0);
  }

  cstats.Print(Form("cdcdedx_bdcal_qastats_%s.pdf", m_suffix.data()));
}