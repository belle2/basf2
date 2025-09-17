/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/calibration/CDCdEdx/HadronPrep.h>
using namespace Belle2;

HadronPrep::HadronPrep()
{
  m_bgBins = 10;
  m_bgMin = 2.85;
  m_bgMax = 40;
  m_cosBins = 8;
  m_cosMin = 0.0;
  m_cosMax = 0.95;
  m_cut = 0.5;
}

HadronPrep::HadronPrep(int bgbins, double lowerbg, double upperbg, int cosbins, double lowercos, double uppercos, double cut)
{
  m_bgBins = bgbins;
  m_bgMin = lowerbg;
  m_bgMax = upperbg;
  m_cosBins = cosbins;
  m_cosMin = lowercos;
  m_cosMax = uppercos;
  m_cut = cut;
}

void HadronPrep::prepareSample(std::shared_ptr<TTree> hadron, TFile*& outfile, const std::string& suffix,
                               const std::string& pdg, bool ismakePlots, bool correct)
{

  std::map<int, std::vector<TH1F*>>  hdedx_bgcosth;
  std::vector<TH2F*> hdedxvscosth_bg;

  //define histograms
  for (int i = 0; i < m_bgBins; ++i) {

    std::string title = Form("%s_dedxvscos_bg_%d", pdg.data(), i);
    hdedxvscosth_bg.push_back(new TH2F(title.data(), Form("%s;costh;dEdx", title.data()),
                                       440, -1.1, 1.1, 2600, -1.0, 25));

    defineHisto(hdedx_bgcosth[i], Form("dedx_bg_%d_cos", i), pdg.data());
  }


  // Create some containers to calculate averages
  for (int i = 0; i < m_bgBins; ++i) {
    for (int j = 0; j < m_cosBins; ++j) {
      m_sumcos[i].push_back(0.);
      m_sumbg[i].push_back(0.);
      m_sumsize[i].push_back(0.);
      m_means[i].push_back(0.);
      m_errors[i].push_back(0.);

    }
  }

  // // --------------------------------------------------
  // // LOOP OVER EVENTS AND FILL CONTAINERS
  // // --------------------------------------------------
  // // Fill the histograms to be fitted

  double dedxnosat;    // dE/dx w/o HS correction (use to get new parameters)
  double p;       // track momentum
  double costh;   // cosine of track polar angle
  double timereso;
  int nhit;       // number of hits on this track

  hadron->SetBranchAddress("dedxnosat", &dedxnosat); //must be without any HS correction
  hadron->SetBranchAddress("p", &p);
  hadron->SetBranchAddress("costh", &costh);
  hadron->SetBranchAddress("timereso", &timereso);
  hadron->SetBranchAddress("nhits", &nhit);

  HadronBgPrep prep;
  double mass = prep.getParticleMass(pdg);
  if (mass == 0.0) B2FATAL("Mass of particle " << pdg.data() << " is zero");

  CDCDedxHadSat had;

  if (correct) {
    std::string infile = "sat-pars.fit.txt";
    std::ifstream parfile(infile);

    if (!parfile.fail()) {
      B2INFO("new parameters are using");
      had.setParameters(infile);
    } else {
      B2INFO("default parameters are using");
      had.setParameters();
    }
  }

  for (unsigned int index = 0; index < hadron->GetEntries(); ++index) {

    hadron->GetEntry(index);

    double bg;      // track beta-gamma
    bg = fabs(p) / mass;

    double uncosth = costh;
    costh = fabs(costh);

    // clean up bad events and restrict the momentum range
    if (nhit <  0 || nhit  > 100)continue;
    if (dedxnosat <= 0)continue;
    if (costh != costh)continue;
    if (bg <= m_bgMin || bg >= m_bgMax)continue;
    if (costh <= m_cosMin || costh >= m_cosMax)continue;

    if (pdg == "proton") if ((dedxnosat - 0.45) * abs(p) * abs(p) < m_cut)continue;

    int bgBin = static_cast<int>((bg - m_bgMin) / (m_bgMax - m_bgMin) * m_bgBins);
    bgBin  = std::max(0, std::min(bgBin, m_bgBins - 1));

    int cosBin = static_cast<int>((costh - m_cosMin) / (m_cosMax - m_cosMin) * m_cosBins);
    cosBin = std::max(0, std::min(cosBin, m_cosBins - 1));

    double dedx_new = dedxnosat;

    if (correct) dedx_new = had.D2I(costh, had.I2D(costh, 1.0) * dedxnosat);

    hdedx_bgcosth[bgBin][cosBin]->Fill(dedx_new);
    hdedxvscosth_bg[bgBin]->Fill(uncosth, dedx_new);

    m_sumcos[bgBin][cosBin] += costh;
    m_sumbg[bgBin][cosBin] += bg;
    m_sumsize[bgBin][cosBin] += 1;

  }// end of event loop

  // // --------------------------------------------------
  // // FIT IN BINS OF BETA-GAMMA AND COS(THETA)
  // // --------------------------------------------------
  // // fit the histograms with Gaussian functions
  // // and extract the means and errors

  setPars(outfile, hdedx_bgcosth, pdg.data());

  if (ismakePlots) {

    plotDist(hdedx_bgcosth, Form("fits_dedx_inbg_%s", suffix.data()), pdg.data());
    plotDist(hdedxvscosth_bg, Form("dist_dedx_vscos_scat_%s", suffix.data()), pdg.data());
    plotGraph(suffix.data(), pdg.data());
  }

  hdedxvscosth_bg.clear();
  hdedx_bgcosth.clear();
  clear();

}

//------------------------------------
void HadronPrep::defineHisto(std::vector<TH1F*>& htemp, const std::string& svar, const std::string& pdg)
{
  for (int j = 0; j < m_cosBins; ++j) {
    // initialize the histograms
    std::string title = Form("%s_%s_%d", pdg.data(), svar.data(), j);
    double dedxMax = 4.0;
    int cosbins = int(50 * dedxMax);
    if (pdg == "pion")  {
      dedxMax = 2.0; cosbins = int(100 * dedxMax);
    }
    if (pdg == "proton")  dedxMax = 20.0;
    if (svar.substr(0, svar.find("_")) == "dedx")
      htemp.push_back(new TH1F(title.data(), title.data(), cosbins, 0, dedxMax));
    else
      htemp.push_back(new TH1F(title.data(), title.data(), cosbins, -30, 30));

  }
}

//------------------------------------
void HadronPrep::plotDist(std::map<int, std::vector<TH1F*>>& hist, const std::string& sname, const std::string& pdg)
{

  TCanvas ctmp(Form("cdcdedx_%s_%s", sname.data(), pdg.data()), "", 1200, 1200);
  ctmp.Divide(2, 2);
  ctmp.SetBatch(kTRUE);

  std::stringstream psname;
  psname << Form("plots/HadronSat/%s_%s.pdf[", sname.data(), pdg.data());
  ctmp.Print(psname.str().c_str());
  psname.str("");
  psname << Form("plots/HadronSat/%s_%s.pdf", sname.data(), pdg.data());

  for (int i = 0 ; i < m_bgBins; ++i) {
    for (int j = 0; j < m_cosBins; ++j) {
      hist[i][j]->SetFillColor(kYellow - 9);

      ctmp.cd(j % 4 + 1);
      hist[i][j]->Draw();

      if (m_cosBins <= 4) {
        if (j + 1 == m_cosBins) ctmp.Print(psname.str().c_str());
      } else if ((j + 1) % 4 == 0 || (j + 1 == m_cosBins)) {
        ctmp.Print(psname.str().c_str());
        ctmp.Clear("D");
      }
    }
  }
  psname.str("");
  psname << Form("plots/HadronSat/%s_%s.pdf]", sname.data(), pdg.data());
  ctmp.Print(psname.str().c_str());
}

//------------------------------------
void HadronPrep::plotDist(std::vector<TH2F*>& hist, const std::string& sname, const std::string& pdg)
{

  TCanvas ctmp(Form("cdcdedx_%s_%s", sname.data(), pdg.data()), "", 1200, 1200);
  ctmp.Divide(2, 2);
  ctmp.SetBatch(kTRUE);

  std::stringstream psname;
  psname << Form("plots/HadronSat/%s_%s.pdf[", sname.data(), pdg.data());
  ctmp.Print(psname.str().c_str());
  psname.str("");
  psname << Form("plots/HadronSat/%s_%s.pdf", sname.data(), pdg.data());
  double bgstep = (m_bgMax - m_bgMin) / m_bgBins;
  double dmax = 2.5;
  if (pdg == "kaon")dmax = 3.5;
  else if (pdg == "proton")dmax = 15.5;

  for (int i = 0 ; i < m_bgBins; ++i) {
    ctmp.cd(i % 4 + 1);
    double frombg = m_bgMin + i * bgstep;
    double tobg = frombg + bgstep;

    hist[i]->SetTitle(Form("%s, pF=(%0.02f,%0.02f)", hist[i]->GetTitle(), frombg, tobg));
    hist[i]->GetYaxis()->SetRangeUser(-0.5, dmax);
    hist[i]->DrawCopy("colz");
    if ((i + 1) % 4 == 0 || (i + 1) == m_bgBins) {
      ctmp.Print(psname.str().c_str());
      ctmp.Clear("D");
    }
  }
  psname.str("");
  psname << Form("plots/HadronSat/%s_%s.pdf]", sname.data(), pdg.data());
  ctmp.Print(psname.str().c_str());
}

//----------------------------------------
void HadronPrep::setPars(TFile*& outfile, std::map<int, std::vector<TH1F*>>& hdedx_bgcosth, const std::string& pdg)
{
  outfile->cd();
  TTree* satTree = new TTree(Form("%s", pdg.data()), "dE/dx means and errors");
  double satbg;          // beta-gamma value for this bin
  double satcosth;       // cos(theta) value for this bin
  double satdedx;        // mean dE/dx value for this bin
  double satdedxerr;     // error on ^
  double satdedxwidth;// width of ^ distribution
  double satbg_avg;      // average beta-gamma value for this sample
  double satcosth_avg;   // average cos(theta) value for this sample

  satTree->Branch("bg", &satbg, "bg/D");
  satTree->Branch("costh", &satcosth, "costh/D");

  //dEdx related variables
  satTree->Branch("dedx", &satdedx, "dedx/D");
  satTree->Branch("dedxerr", &satdedxerr, "dedxerr/D");
  satTree->Branch("dedxwidth", &satdedxwidth, "dedxwidth/D");

  // Other variables
  satTree->Branch("bg_avg", &satbg_avg, "bg_avg/D");
  satTree->Branch("costh_avg", &satcosth_avg, "costh_avg/D");

  double bgstep = (m_bgMax - m_bgMin) / m_bgBins;
  double cosstep = (m_cosMax - m_cosMin) / m_cosBins;

  for (int i = 0; i < m_bgBins; ++i) {
    for (int j = 0; j < m_cosBins; ++j) {

      // fill some details for this bin
      satbg = m_bgMin + 0.5 * bgstep + i * bgstep;
      satcosth = m_cosMin + 0.5 * cosstep + j * cosstep;

      if (m_sumsize[i][j] > 0) {
        satbg_avg = m_sumbg[i][j] / m_sumsize[i][j];
        satcosth_avg = m_sumcos[i][j] / m_sumsize[i][j];
      } else {
        satbg_avg = satcosth_avg = 0.0;
      }

      //1. -------------------------
      // fit the dE/dx distribution in bins of beta-gamma and cosine
      HadronBgPrep prep;
      gstatus stats;
      prep.fit(hdedx_bgcosth[i][j],  pdg.data(), stats);
      if (stats == OK) {
        satdedx = m_means[i][j] = hdedx_bgcosth[i][j]->GetFunction("gaus")->GetParameter(1);
        satdedxerr = m_errors[i][j] = hdedx_bgcosth[i][j]->GetFunction("gaus")->GetParError(1);
        satdedxwidth = hdedx_bgcosth[i][j]->GetFunction("gaus")->GetParameter(2);
      } else { satdedx = 0.0; satdedxerr = 0.0; satdedxwidth = 0.0;}


      //Be careful to not set for every particle
      if (satdedx > m_dedxmax) m_dedxmax = satdedx;
      if (satdedx < m_dedxmin) m_dedxmin = satdedx;

      // fill the tree for this bin
      satTree->Fill();
    }
  }
  outfile->Write();
}

//------------------------------------
void HadronPrep::plotGraph(const std::string& sname, const std::string& pdg)
{
  std::vector<double> cbcenters(m_cosBins), cberrors(m_cosBins);
  double cosstep = (m_cosMax - m_cosMin) / m_cosBins;
  double bgstep = (m_bgMax - m_bgMin) / m_bgBins;

  for (int j = 0; j < m_cosBins; ++j) {
    cbcenters[j] = m_cosMin + 0.5 * cosstep + j * cosstep;
    cberrors[j] = 0;
  }

  // Plot the dE/dx means vs. cos(theta) for validation
  TCanvas ctmp6(Form("cdcdedx_costh_%s_%s", sname.data(), pdg.data()), "", 500, 500);
  ctmp6.SetGridy(1);

  TH1F base("base", "bla-bla", 25, 0, 1.0);
  base.SetTitle(Form("%s: dE/dx fit means vs. cos(#theta);cos(#theta);dE/dx-fit mean", pdg.data()));
  base.GetXaxis()->SetTitleOffset(1.2);

  double margin = 0.1;
  if (pdg == "proton") margin = 0.95;

  base.SetMaximum(m_dedxmax + margin);
  base.SetMinimum(m_dedxmin - margin);
  base.SetStats(0);
  base.DrawCopy("");

  TLegend legend(0.75, 0.75, 0.95, 0.9);

  std::vector<TGraphErrors*> gdedx_costh(m_bgBins);

  legend.SetBorderSize(0);

  for (int i = 0; i < m_bgBins; ++i) {
    std::vector<double> mean_d(m_cosBins);
    std::vector<double> error_d(m_cosBins);

    for (int j = 0; j < m_cosBins; ++j) {
      mean_d [j] = m_means[i][j];
      error_d[j] = m_errors[i][j];
    }
    gdedx_costh[i] = new TGraphErrors(m_cosBins, cbcenters.data(), mean_d.data(), cberrors.data(), error_d.data());

    gdedx_costh[i]->SetMarkerSize(0.9);
    gdedx_costh[i]->SetMarkerColor(50 + i * 3);
    gdedx_costh[i]->SetLineColor(i + 1);
    gdedx_costh[i]->SetLineWidth(1);
    gdedx_costh[i]->Draw("same");
    legend.AddEntry(gdedx_costh[i], Form("bg = (%0.03f - %0.03f)", m_bgMin + i * bgstep, m_bgMin + (i + 1) * bgstep), "lep");
  }

  legend.Draw("same");

  std::stringstream psname;
  psname.str("");
  psname << Form("plots/HadronSat/gr_dedx_vscosth_%s_%s.pdf", sname.data(), pdg.data());
  ctmp6.SaveAs(psname.str().c_str());

  for (int i = 0; i < m_bgBins; ++i)
    delete gdedx_costh[i];
}


void HadronPrep::clear()
{
  m_sumcos.clear();
  m_sumbg.clear();
  m_sumsize.clear();
  m_means.clear();
  m_errors.clear();
}