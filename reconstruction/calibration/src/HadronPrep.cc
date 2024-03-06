/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/calibration/HadronPrep.h>
using namespace Belle2;

void HadronPrep::prepareSample(std::shared_ptr<TTree> hadron, TFile*& outfile, std::string suffix,
                               std::string pdg, bool ismakePlots, bool correct)
{

  std::map<std::string, std::array<double, 3>> bgpar = {
    {"electron", {18, 600, 9000}},
    {"pion", {4, 2.0, 12.94}},
    {"kaon", {4, 1.10, 3.6}},
    {"muon", {8, 2.83, 28.83}},
    {"proton", {6, 0.46, 0.85}}
  };

  m_bgBins = bgpar[pdg.data()][0];
  m_bgMin = bgpar[pdg.data()][1];
  m_bgMax = bgpar[pdg.data()][2];

  //define histograms
  for (int i = 0; i < m_bgBins; ++i) {

    std::string title = Form("%s_dedxvscos_bg_%d", pdg.data(), i);
    hdedxvscosth_bg.push_back(new TH2F(title.data(), Form("%s;costh;dEdx", title.data()),
                                       440, -1.1, 1.1, 2600, -1.0, 25));

    defineHisto(hdedx_bgcosth[i], Form("dedx_bg_%d_cos", i), pdg.data());
    defineHisto(hchi_bgcosth[i], Form("chi_bg_%d_cos", i), pdg.data());
  }

  defineHisto(hdedx_costh, "dedx_cos", pdg.data());

  // Create some containers to calculate averages

  for (int i = 0; i < m_bgBins; ++i) {
    for (int j = 0; j < m_cosBins; ++j) {
      sumcos[i].push_back(0.);
      sumbg[i].push_back(0.);
      sumres_square[i].push_back(0.);
      sumsize[i].push_back(0.);
      means[i].push_back(0.);
      errors[i].push_back(0.);

    }
  }

  // // --------------------------------------------------
  // // LOOP OVER EVENTS AND FILL CONTAINERS
  // // --------------------------------------------------
  // // Fill the histograms to be fitted
  fillHisto(hadron, pdg.data(), correct);

  // // --------------------------------------------------
  // // FIT IN BINS OF BETA-GAMMA AND COS(THETA)
  // // --------------------------------------------------
  // // fit the histograms with Gaussian functions
  // // and extract the means and errors
  setPars(outfile, pdg.data());

  if (ismakePlots) {

    plotDist(hdedx_bgcosth, Form("fits_dedx_inbg_%s", suffix.data()), pdg.data());
    plotDist(hdedxvscosth_bg, Form("dist_dedx_vscos_scat_%s", suffix.data()), pdg.data());
    plotDist(hchi_bgcosth, Form("fits_chi_inbg_%s", suffix.data()), pdg.data());
    plotDist(hdedx_costh, Form("fits_dedx_incos_%s", suffix.data()), pdg.data());
    plotGraph(suffix.data(), pdg.data());
  }

  // delete histograms
  deleteHisto();

}

//------------------------------------
void HadronPrep::defineHisto(std::vector<TH1F*>& htemp, std::string svar, std::string pdg)
{
  // char *p;
  // p = strtok(svar, "_");
  for (int j = 0; j < m_cosBins; ++j) {
    // initialize the histograms
    std::string title = Form("%s_%s_%d", pdg.data(), svar.data(), j);
    double dedxMax = 4.0;
    int cosbins = int(40 * dedxMax);
    if (pdg == "pion")  {
      dedxMax = 2.0; cosbins = int(50 * dedxMax);
    }
    if (pdg == "proton")  dedxMax = 20.0;
    if (svar.substr(0, svar.find("_")) == "dedx")
      htemp.push_back(new TH1F(title.data(), title.data(), cosbins, 0, dedxMax));
    else
      htemp.push_back(new TH1F(title.data(), title.data(), cosbins, -30, 30));

  }
}

void HadronPrep::fillHisto(std::shared_ptr<TTree> hadron, std::string pdg, bool correct)
{

  // auto hadron = getObjectPtr<TTree>(Form("%s", pdg.data()));

  double dedxnosat;    // dE/dx w/o HS correction (use to get new parameters)
  double p;       // track momentum
  double bg;      // track beta-gamma
  double costh;   // cosine of track polar angle
  double timereso;
  int nhit;       // number of hits on this track

  hadron->SetBranchAddress("dedx", &dedxnosat); //must be without any HS correction
  hadron->SetBranchAddress("p", &p);
  hadron->SetBranchAddress("costh", &costh);
  hadron->SetBranchAddress("timereso", &timereso);
  hadron->SetBranchAddress("nhits", &nhit);

  double mass = 0.0;
  if (pdg == "pion") mass = Const::pion.getMass();
  else if (pdg == "kaon") mass = Const::kaon.getMass();
  else if (pdg == "proton") mass = Const::proton.getMass();
  else if (pdg == "muon") mass = Const::muon.getMass();
  else if (pdg == "electron") mass = Const::electron.getMass();
  if (mass == 0.0) exit(1);

  double alpha, gamma, delta, power, ratio;
  double par[5];
  CDCDedxMeanPred m;
  CDCDedxSigmaPred s;
  HadronSaturation had;
  if (correct) {
    std::ifstream parfile("sat-pars.fit.txt");

    if (!parfile.fail()) {
      std::cout << "new parameters are using" << std::endl;
      for (int i = 0; i < 5; ++i) {
        parfile >> par[i];
      }
      parfile.close();
      alpha = par[0];
      gamma = par[1];
      delta = par[2];
      power = par[3];
      ratio = par[4];

    } else {
      std::cout << "default parameters are using" << std::endl;

      for (int i = 0; i < 5; ++i) {
        // alpha = m_DBHadronCor->getHadronPar(0);
        // gamma = m_DBHadronCor->getHadronPar(1);
        // delta = m_DBHadronCor->getHadronPar(2);
        // power = m_DBHadronCor->getHadronPar(3);
        // ratio = m_DBHadronCor->getHadronPar(4);
        alpha = 0.165016;
        gamma = 0.02078;
        delta = 0.02;
        power = 1.18098;
        ratio = 1;
      }
    }
  }
  for (unsigned int index = 0; index < hadron->GetEntries(); ++index) {

    hadron->GetEvent(index);

    bg = fabs(p) / mass;
    double uncosth = costh;
    costh = fabs(costh);

    // clean up bad events and restrict the momentum range
    if (nhit <  0 || nhit  > 100)continue;
    if (dedxnosat <= 0)continue;
    if (costh != costh)continue;
    if (bg <= m_bgMin || bg >= m_bgMax)continue;
    if (costh <= m_cosMin || costh >= m_cosMax)continue;

    if (pdg == "proton") {
      if ((dedxnosat - 0.45)*abs(p)*abs(p) < 0.5)continue;
    }

    int bgBin = (int)((bg - m_bgMin) / (m_bgMax - m_bgMin) * m_bgBins);
    int cosBin = (int)((costh - m_cosMin) / (m_cosMax - m_cosMin) * m_cosBins);


    double dedx_new = dedxnosat;

    if (correct) {
      dedx_new = had.D2I(costh, had.I2D(costh, 1.0, alpha, gamma, delta, power, ratio) * dedxnosat,
                         alpha, gamma, delta, power, ratio);
    }

    hdedx_bgcosth[bgBin][cosBin]->Fill(dedx_new);
    hdedx_costh[cosBin]->Fill(dedx_new);
    hdedxvscosth_bg[bgBin]->Fill(uncosth, dedx_new);

    //get predicted value of dEdx mean
    double dedx_cur = m.getMean(bg);
    double dedx_res = s.nhitPrediction(nhit) * s.ionzPrediction(dedx_new) * s.cosPrediction(costh) * timereso;

    if (dedx_res == 0) {
      std::cout << "RESOLUTION IS ZERO!!!" << std::endl;
      continue;
    }

    //Modified chi_new values
    double chi_new  = (dedx_new - dedx_cur) / dedx_res;

    hchi_bgcosth[bgBin][cosBin]->Fill(chi_new);

    sumcos[bgBin][cosBin] += costh;
    sumbg[bgBin][cosBin] += bg;
    sumres_square[bgBin][cosBin] += pow(dedx_res, 2);
    sumsize[bgBin][cosBin] += 1;

  }// end of event loop
}

//------------------------------------
void HadronPrep::plotDist(std::map<int, std::vector<TH1F*>> hist, std::string sname, std::string pdg)
{

  TCanvas ctmp(Form("cdcdedx_%s_%s", sname.data(), pdg.data()), "", 1200, 1200);
  ctmp.Divide(2, 2);
  ctmp.SetBatch(kTRUE);

  std::stringstream psname;
  psname << Form("cdcdedx_hadcal_%s_%s.pdf[", sname.data(), pdg.data());
  ctmp.Print(psname.str().c_str());
  psname.str("");
  psname << Form("cdcdedx_hadcal_%s_%s.pdf", sname.data(), pdg.data());

  for (int i = 0 ; i < m_bgBins; ++i) {
    for (int j = 0; j < m_cosBins; ++j) {
      hist[i][j]->SetFillColor(kYellow - 9);

      ctmp.cd(j % 4 + 1);
      hist[i][j]->Draw();

      if (m_cosBins <= 4) {
        if (j + 1 == m_cosBins)ctmp.Print(psname.str().c_str());
      } else {
        if ((j + 1) % 4 == 0) {
          ctmp.Print(psname.str().c_str());
          ctmp.Clear("D");
        } else if (((j + 1) % 4 != 0) && (j + 1 == m_cosBins)) {
          ctmp.Print(psname.str().c_str());
          ctmp.Clear("D");
        }
      }
    }
  }
  psname.str("");
  psname << Form("cdcdedx_hadcal_%s_%s.pdf]", sname.data(), pdg.data());
  ctmp.Print(psname.str().c_str());
}

//------------------------------------
void HadronPrep::plotDist(std::vector<TH2F*> hist, std::string sname, std::string pdg)
{

  TCanvas ctmp(Form("cdcdedx_%s_%s", sname.data(), pdg.data()), "", 1200, 1200);
  ctmp.Divide(2, 2);
  ctmp.SetBatch(kTRUE);

  std::stringstream psname;
  psname << Form("cdcdedx_hadcal_%s_%s.pdf[", sname.data(), pdg.data());
  ctmp.Print(psname.str().c_str());
  psname.str("");
  psname << Form("cdcdedx_hadcal_%s_%s.pdf", sname.data(), pdg.data());
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
  psname << Form("cdcdedx_hadcal_%s_%s.pdf]", sname.data(), pdg.data());
  ctmp.Print(psname.str().c_str());
}

//------------------------------------
void HadronPrep::plotDist(std::vector<TH1F*> hist, std::string sname, std::string pdg)
{

  TCanvas ctmp(Form("cdcdedx_%s_%s", sname.data(), pdg.data()), "", 1200, 1200);
  ctmp.Divide(2, 2);
  ctmp.SetBatch(kTRUE);

  std::stringstream psname;
  psname << Form("cdcdedx_hadcal_%s_%s.pdf[", sname.data(), pdg.data());
  ctmp.Print(psname.str().c_str());
  psname.str("");
  psname << Form("cdcdedx_hadcal_%s_%s.pdf", sname.data(), pdg.data());

  for (int i = 0 ; i < m_cosBins; ++i) {
    ctmp.cd(i % 4 + 1);

    hist[i]->SetFillColor(kYellow - 9);

    hist[i]->SetTitle(Form("%s", hist[i]->GetTitle()));
    hist[i]->DrawCopy();
    if ((i + 1) % 4 == 0 || (i + 1) == m_cosBins) {
      ctmp.Print(psname.str().c_str());
      ctmp.Clear("D");
    }
  }
  psname.str("");
  psname << Form("cdcdedx_hadcal_%s_%s.pdf]", sname.data(), pdg.data());
  ctmp.Print(psname.str().c_str());
}

//----------------------------------------
void HadronPrep::setPars(TFile*& outfile, std::string pdg)
{
  outfile->cd();
  TTree* satTree = new TTree(Form("%s", pdg.data()), "dE/dx means and errors");
  double satbg;          // beta-gamma value for this bin
  double satcosth;       // cos(theta) value for this bin
  double satdedx;        // mean dE/dx value for this bin
  double dedxcos;        // mean dE/dx value for this bin
  double satdedxerr;     // error on ^
  double satdedxwidth;// width of ^ distribution
  double satchi;         // mean chi value for this bin
  double satchierr;      // error on ^
  double satchiwidth;    // width of ^ distribution
  double satbg_avg;      // average beta-gamma value for this sample
  double satcosth_avg;   // average cos(theta) value for this sample
  double satdedxres_avg; // average dE/dx error squared for this sample
  double ratio;          // ratio of the predicted mean to that of the average

  satTree->Branch("bg", &satbg, "bg/D");
  satTree->Branch("costh", &satcosth, "costh/D");

  //dEdx related variables
  satTree->Branch("dedx", &satdedx, "dedx/D");
  satTree->Branch("dedxcos", &dedxcos, "dedxcos/D");
  satTree->Branch("dedxerr", &satdedxerr, "dedxerr/D");
  satTree->Branch("dedxwidth", &satdedxwidth, "dedxwidth/D");

  // Chi related variables
  satTree->Branch("chi", &satchi, "chi/D");
  satTree->Branch("chierr", &satchierr, "chierr/D");
  satTree->Branch("chiwidth", &satchiwidth, "chiwidth/D");

  // Other variables
  satTree->Branch("bg_avg", &satbg_avg, "bg_avg/D");
  satTree->Branch("costh_avg", &satcosth_avg, "costh_avg/D");
  satTree->Branch("dedxres_avg", &satdedxres_avg, "dedxres_avg/D");
  satTree->Branch("ratio", &ratio, "ratio/D");


  double bgstep = (m_bgMax - m_bgMin) / m_bgBins;
  double cosstep = (m_cosMax - m_cosMin) / m_cosBins;

  for (int i = 0; i < m_bgBins; ++i) {
    for (int j = 0; j < m_cosBins; ++j) {

      // fill some details for this bin
      std::cout << "\t" << " Bin in (bg ,costh) = (" << i << ", " << j << ") ---------------" << std::endl;
      satbg = m_bgMin + 0.5 * bgstep + i * bgstep;
      satcosth = m_cosMin + 0.5 * cosstep + j * cosstep;

      satbg_avg = sumbg[i][j] / sumsize[i][j];
      satcosth_avg = sumcos[i][j] / sumsize[i][j];
      satdedxres_avg = sumres_square[i][j] / sumsize[i][j];

      //1. -------------------------
      // fit the dE/dx distribution in bins of beta-gamma and cosine
      fit(hdedx_bgcosth[i][j],  pdg.data());
      satdedx = means[i][j] = hdedx_bgcosth[i][j]->GetFunction("gaus")->GetParameter(1);
      satdedxerr = errors[i][j] = hdedx_bgcosth[i][j]->GetFunction("gaus")->GetParError(1);
      satdedxwidth = hdedx_bgcosth[i][j]->GetFunction("gaus")->GetParameter(2);

      //2. -------------------------
      // fit the dE/dx distribution in bins of cosine
      if (m_bgBins == 0) {
        fit(hdedx_costh[j],  pdg.data());
        dedxcos = hdedx_costh[j]->GetFunction("gaus")->GetParameter(1);
      }

      //Be careful to not set for every particle
      if (satdedx > dedxmax) dedxmax = satdedx;
      if (satdedx < dedxmin) dedxmin = satdedx;

      //3. -------------------------
      // fit the chi distribution  in bins of beta-gamma and cosine
      fit(hchi_bgcosth[i][j], pdg.data());
      satchi = hchi_bgcosth[i][j]->GetFunction("gaus")->GetParameter(1);
      satchierr  = hchi_bgcosth[i][j]->GetFunction("gaus")->GetParError(1);
      satchiwidth = hchi_bgcosth[i][j]->GetFunction("gaus")->GetParameter(2);

      // determine the ratio of the predicted mean at a given bg to that of the average
      // ratio = m_gpar.dedxPrediction(satbg_avg) / m_gpar.dedxPrediction(satbg);

      // fill the tree for this bin
      satTree->Fill();
    }
  }
  outfile->Write();
}

//----------------------------------------
void HadronPrep::fit(TH1F*& hist, std::string pdg)
{
  gstatus status;
  if (pdg == "pion") fitGaussianWRange(hist, status, 1.5);
  else fitGaussianWRange(hist, status, 2.0);

  hist->SetFillColorAlpha(kAzure + 1, 0.30);

  double mean = hist->GetFunction("gaus")->GetParameter(1);
  double meanerr = hist->GetFunction("gaus")->GetParError(1);
  double width = hist->GetFunction("gaus")->GetParameter(2);

  std::string title = Form("#mu_{fit}: %0.03f #pm %0.03f, #sigma_{fit}: %0.03f", mean, meanerr, width);
  hist->SetTitle(Form("%s, %s", hist->GetTitle(), title.data()));

}

//----------------------------------------
void HadronPrep::fitGaussianWRange(TH1F*& temphist, gstatus& status, double sigmaR)
{
  double histmean = temphist->GetMean();
  double histrms = temphist->GetRMS();
  temphist->GetXaxis()->SetRangeUser(histmean - 5.0 * histrms, histmean + 5.0 * histrms);

  int fs = temphist->Fit("gaus", "Q0");
  if (fs != 0) {
    B2INFO(Form("\tFit (round 1) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
    status = Failed;
    return;
  } else {
    double mean = temphist->GetFunction("gaus")->GetParameter(1);
    double width = temphist->GetFunction("gaus")->GetParameter(2);
    temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
    fs = temphist->Fit("gaus", "QR", "", mean - sigmaR * width, mean + sigmaR * width);
    if (fs != 0) {
      B2INFO(Form("\tFit (round 2) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
      status = Failed;
      return;
    } else {
      temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
      B2INFO(Form("\tFit for hist (%s) sucessfull (status = %d)", temphist->GetName(), fs));
      status = OK;
    }
  }
}

//------------------------------------
void HadronPrep::plotGraph(std::string sname, std::string pdg)
{
  double cbcenters[m_cosBins], cberrors[m_cosBins];
  double cosstep = (m_cosMax - m_cosMin) / m_cosBins;
  double bgstep = (m_bgMax - m_bgMin) / m_bgBins;

  for (int j = 0; j < m_cosBins; ++j) {
    cbcenters[j] = m_cosMin + 0.5 * cosstep + j * cosstep;
    cberrors[j] = 0;
  }

  // Plot the dE/dx means vs. cos(theta) for validation
  TCanvas ctmp6("tmp6", "tmp6", 500, 500);
  ctmp6.SetGridy(1);

  TH1F base("base", "bla-bla", 25, 0, 1.0);
  base.SetTitle(Form("%s: dE/dx fit means vs. cos(#theta);cos(#theta);dE/dx-fit mean", pdg.data()));
  base.GetXaxis()->SetTitleOffset(1.2);

  double margin = 0.1;
  if (pdg == "proton") margin = 0.95;

  base.SetMaximum(dedxmax + margin);
  base.SetMinimum(dedxmin - margin);
  base.SetStats(0);
  base.DrawCopy("");

  auto legend = new TLegend(0.75, 0.75, 0.95, 0.9);

  TGraphErrors* gdedx_costh[m_bgBins];
  legend->SetBorderSize(0);

  for (int i = 0; i < m_bgBins; ++i) {
    double mean_d [m_cosBins];
    double error_d[m_cosBins];
    for (int j = 0; j < m_cosBins; ++j) {
      mean_d [j] = means[i][j];
      error_d[j] = errors[i][j];
    }
    gdedx_costh[i] = new TGraphErrors(m_cosBins, cbcenters, mean_d, cberrors, error_d);
    gdedx_costh[i]->SetMarkerSize(0.9);
    gdedx_costh[i]->SetMarkerColor(50 + i * 3);
    gdedx_costh[i]->SetLineColor(i + 1);
    gdedx_costh[i]->SetLineWidth(1);
    gdedx_costh[i]->Draw("same");
    legend->AddEntry(gdedx_costh[i], Form("bg = (%0.03f - %0.03f)", m_bgMin + i * bgstep, m_bgMin + (i + 1) * bgstep), "lep");
  }

  legend->Draw("same");

  std::stringstream psname;
  psname.str("");
  psname << Form("cdcdedx_hadcal_gr_dedx_vscosth_%s_%s.pdf", sname.data(), pdg.data());
  ctmp6.SaveAs(psname.str().c_str());

  delete legend;
}


void HadronPrep::deleteHisto()
{
  hdedxvscosth_bg.clear();
  hdedx_bgcosth.clear();
  hchi_bgcosth.clear();
  sumcos.clear();
  sumbg.clear();
  sumres_square.clear();
  sumsize.clear();
  means.clear();
  errors.clear();
  hdedx_costh.clear();
}