/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/calibration/CDCdEdx/HadronBgPrep.h>
using namespace Belle2;

HadronBgPrep::HadronBgPrep()
{
  m_bgBins = 15;
  m_bgMin = 0.1;
  m_bgMax = 40;
  m_cosBins = 18;
  m_cosMin = -1.0;
  m_cosMax = 1.0;
  m_injBins = 20;
  m_injMin = 0;
  m_injMax = 80000;
  m_nhitBins = 10;
  m_nhitMin = 7;
  m_nhitMax = 39;
  m_cut = 0.5;
}
HadronBgPrep::HadronBgPrep(int bgbins, double lowerbg, double upperbg, int cosbins, double lowercos, double uppercos, int injbins,
                           double lowerinj, double upperinj, int nhitbins, double lowernhit, double uppernhit, double cut)
{
  m_bgBins = bgbins;
  m_bgMin = lowerbg;
  m_bgMax = upperbg;
  m_cosBins = cosbins;
  m_cosMin = lowercos;
  m_cosMax = uppercos;
  m_injBins = injbins;
  m_injMin = lowerinj;
  m_injMax = upperinj;
  m_nhitBins = nhitbins;
  m_nhitMin = lowernhit;
  m_nhitMax = uppernhit;
  m_cut = cut;
}

void HadronBgPrep::prepareSample(std::shared_ptr<TTree> hadron, TFile*& outfile, std::string suffix, std::string bgcurvefile,
                                 std::string bgsigmafile, std::string pdg, bool ismakePlots)
{

  std::vector<TH1F*>  hdedx_bg, hchi_bg, hionzsigma_bg;
  std::map<int, std::vector<TH1F*>>  hchi_inj, hchicos_allbg, hchicos_1by3bg, hchicos_2by3bg, hchicos_3by3bg;

  //define histograms
  defineHisto(hdedx_bg, "dedx", "bg", pdg.data());
  defineHisto(hchi_bg, "chi", "bg", pdg.data());
  defineHisto(hionzsigma_bg, "ionzsigma", "bg", pdg.data());
  for (int i = 0; i < 2; ++i) {

    defineHisto(hchi_inj[i], "chi", Form("inj_%d", i), pdg.data());

    std::string charge = "pos";
    if (i == 1) charge = "neg";

    defineHisto(hchicos_allbg[i], "chi", Form("%s_allbg_cos", charge.data()), pdg.data());
    defineHisto(hchicos_1by3bg[i], "chi", Form("%s_1b3bg_cos", charge.data()), pdg.data());
    defineHisto(hchicos_2by3bg[i], "chi", Form("%s_2b3bg_cos", charge.data()), pdg.data());
    defineHisto(hchicos_3by3bg[i], "chi", Form("%s_3b3bg_cos", charge.data()), pdg.data());
  }


  // Create some containers to calculate averages
  for (int i = 0; i < m_bgBins; ++i) {
    m_sumcos.push_back(0.);
    m_sumbg.push_back(0.);
    m_sumres_square.push_back(0.);
    m_sumsize.push_back(0.);
    m_means.push_back(0.);
    m_errors.push_back(0.);
  }

  for (int i = 0; i < m_injBins; ++i) {
    m_injsize.push_back(0.);
    m_suminj.push_back(0.);
  }

  // --------------------------------------------------
  // LOOP OVER EVENTS AND FILL CONTAINERS
  // --------------------------------------------------
  // Fill the histograms to be fitted

  double dedxnosat;    // dE/dx w/o HS correction (use to get new parameters)
  double p;       // track momentum
  double costh;   // cosine of track polar angle
  double timereso;
  int nhit;       // number of hits on this track
  double injtime; //injection time
  double isher;
  int charge;

  hadron->SetBranchAddress("dedxnosat", &dedxnosat); //must be without any HS correction
  hadron->SetBranchAddress("p", &p);
  hadron->SetBranchAddress("costh", &costh);
  hadron->SetBranchAddress("timereso", &timereso);
  hadron->SetBranchAddress("nhits", &nhit);
  hadron->SetBranchAddress("injtime", &injtime);
  hadron->SetBranchAddress("injring", &isher);
  hadron->SetBranchAddress("charge", &charge);

  double mass = getParticleMass(pdg);
  if (mass == 0.0) B2FATAL("Mass of particle " << pdg.data() << " is zero");

  const double cosstep = (m_cosMax - m_cosMin) / m_cosBins;
  const double tstep = (m_injMax - m_injMin) / m_injBins;

  CDCDedxMeanPred mbg;
  mbg.setParameters(bgcurvefile);
  CDCDedxSigmaPred sbg;
  sbg.setParameters(bgsigmafile);

  CDCDedxHadSat had;
  had.setParameters("sat-pars.fit.txt");

  unsigned int entries = hadron->GetEntries();
  // if (pdg == "electron") entries /= 10.;
  for (unsigned int index = 0; index < entries; ++index) {

    hadron->GetEvent(index);

    int chg = (charge < 0) ? 1 : 0;
    double bg = fabs(p) / mass;

    // clean up bad events and restrict the momentum range
    if (nhit <  0 || nhit  > 100)continue;
    if (injtime <= 0 || isher < 0)continue;

    if (dedxnosat <= 0)continue;
    if (costh != costh)continue;
    if (bg < m_bgMin || bg > m_bgMax)continue;

    if (fabs(p) > 7.0)continue;

    if (pdg == "proton")  if ((dedxnosat - 0.45) * abs(p) * abs(p) < m_cut)continue;

    int bgBin = (int)((bg - m_bgMin) / (m_bgMax - m_bgMin) * m_bgBins);

    double dedx_new = had.D2I(costh, had.I2D(costh, 1.0) * dedxnosat);

    hdedx_bg[bgBin]->Fill(dedx_new);

    //get predicted value of dEdx mean
    double dedx_cur = mbg.getMean(bg);
    double dedx_res = sbg.getSigma(dedx_new, nhit, costh, timereso);

    if (dedx_res == 0) {
      B2INFO("RESOLUTION IS ZERO!!!");
      continue;
    }

    //Modified chi_new values
    double chi_new  = (dedx_new - dedx_cur) / dedx_res;

    hchi_bg[bgBin]->Fill(chi_new);

    double ionz_res = sbg.cosPrediction(costh) * sbg.nhitPrediction(nhit) * timereso;

    hionzsigma_bg[bgBin]->Fill((dedx_new - dedx_cur) / ionz_res);

    m_sumcos[bgBin] += costh;
    m_sumbg[bgBin] += bg;
    m_sumres_square[bgBin] += pow(dedx_res, 2);
    m_sumsize[bgBin] += 1;

    // make histograms of dE/dx vs. cos(theta) for validation
    int icos = (int)((costh + 1) / cosstep);
    hchicos_allbg[chg][icos]->Fill(chi_new);

    if (bgBin <= int(m_bgBins / 3)) hchicos_1by3bg[chg][icos]->Fill(chi_new);
    else if (bgBin <= int(2 * m_bgBins / 3)) hchicos_2by3bg[chg][icos]->Fill(chi_new);
    else hchicos_3by3bg[chg][icos]->Fill(chi_new);

    if (injtime > m_injMax) injtime = m_injMax - 10.0;
    int wr = 0;
    if (isher > 0.5) wr = 1;
    int injBin = (int)((injtime - m_injMin) / tstep);
    hchi_inj[wr][injBin]->Fill(chi_new);

    m_suminj[injBin] += injtime;
    m_injsize[injBin] += 1;

  } // end of event loop

  // --------------------------------------------------
  // FIT IN BINS OF BETA-GAMMA AND COS(THETA)
  // --------------------------------------------------
  // fit the histograms with Gaussian functions
  // and extract the means and errors

  setPars(outfile, pdg, hdedx_bg, hchi_bg, hionzsigma_bg, hchi_inj);

  // Plot the histograms
  if (ismakePlots) {

    plotDist(hdedx_bg, Form("fits_dedx_inbg_%s_%s", suffix.data(), pdg.data()), m_bgBins);
    plotDist(hchi_bg, Form("fits_chi_inbg_%s_%s", suffix.data(), pdg.data()), m_bgBins);
    plotDist(hionzsigma_bg, Form("fits_ionzreso_inbg_%s_%s", suffix.data(), pdg.data()), m_bgBins);
    plotDist(hchi_inj, Form("fits_chi_ininj_%s_%s", suffix.data(), pdg.data()), m_injBins);
    printCanvasCos(hchicos_allbg, hchicos_1by3bg, hchicos_2by3bg, hchicos_3by3bg, pdg, suffix);
  }

  // delete histograms
  clearVars();
  deleteHistos(hdedx_bg, m_bgBins);
  deleteHistos(hchi_bg, m_bgBins);
  deleteHistos(hionzsigma_bg, m_bgBins);
  for (int i = 0; i < 2; ++i) {
    deleteHistos(hchi_inj[i], m_injBins);
    deleteHistos(hchicos_allbg[i], m_cosBins);
    deleteHistos(hchicos_1by3bg[i], m_cosBins);
    deleteHistos(hchicos_2by3bg[i], m_cosBins);
    deleteHistos(hchicos_3by3bg[i], m_cosBins);
  }

}

//------------------------------------
void HadronBgPrep::defineHisto(std::vector<TH1F*>& htemp, std::string svar, std::string stype, std::string pdg)
{
  int nbdEdx = 200, nbchi = 200;
  double dedxMax = 20.0;

  if (pdg == "pion") {
    nbdEdx = 400, dedxMax = 4.0;
  } else if (pdg == "kaon") {
    nbdEdx = 500, dedxMax = 5.0;
  } else if (pdg == "proton") {
    nbdEdx = 1500, dedxMax = 30.0;
    nbchi = 350;
  } else if (pdg == "muon") {
    nbdEdx = 300, dedxMax = 3.0;
  } else if (pdg == "electron") {
    nbdEdx = 200, dedxMax = 2.0;
  }

  int bins;
  double min, max;

  if (stype == "bg") bins = m_bgBins, min = m_bgMin, max = m_bgMax ;
  else if (stype == "inj_0" || stype == "inj_1") bins = m_injBins, min = m_injMin, max = m_injMax ;
  else if (stype == "nhit") bins = m_nhitBins, min = m_nhitMin, max = m_nhitMax ;
  else bins = m_cosBins, min = m_cosMin, max = m_cosMax ;

  double step = (max - min) / bins;
  if (stype == "nhit") step = (max - min + 1) / bins;

  for (int j = 0; j < bins; ++j) {

    double start = min + j * step;
    double end = start + step;
    if (stype == "nhit") end = int((start + step) * 0.99999);

    // initialize the histograms
    std::string histname = Form("%s_%s_%s_%d", pdg.data(), svar.data(), stype.data(), j);
    std::string title = Form("%s_%s_%s (%.02f, %.02f)", pdg.data(), svar.data(), stype.data(), start, end);

    if (svar == "dedx")
      htemp.push_back(new TH1F(histname.data(), title.data(), nbdEdx, 0, dedxMax));
    else if (svar == "chi")
      htemp.push_back(new TH1F(histname.data(), title.data(), nbchi, -10.0, 10.0));
    else
      htemp.push_back(new TH1F(histname.data(), title.data(), 300, -3.0, 3.0));

    htemp[j]->GetXaxis()->SetTitle(Form("%s", svar.data()));
    htemp[j]->GetYaxis()->SetTitle("Entries");
  }
}

//------------------------------------
void HadronBgPrep::plotDist(std::map<int, std::vector<TH1F*>>& hist, std::string sname, int bins)
{

  TCanvas ctmp(Form("cdcdedx_%s", sname.data()), "", 1200, 600);
  ctmp.Divide(2, 1);
  ctmp.SetBatch(kTRUE);

  std::stringstream psname;
  psname << Form("plots/HadronPrep/%s.pdf[", sname.data());
  ctmp.Print(psname.str().c_str());
  psname.str("");
  psname << Form("plots/HadronPrep/%s.pdf", sname.data());
  for (int j = 0; j < bins; ++j) {

    for (int i = 0 ; i < 2; ++i) {
      ctmp.cd(i + 1);
      hist[i][j]->SetFillColorAlpha(i + 5, 0.25);
      hist[i][j]->Draw();
    }
    ctmp.Print(psname.str().c_str());
  }
  psname.str("");
  psname << Form("plots/HadronPrep/%s.pdf]", sname.data());
  ctmp.Print(psname.str().c_str());
}

//------------------------------------
void HadronBgPrep::plotDist(std::vector<TH1F*>& hist, std::string sname, int nbins)
{

  TCanvas ctmp(Form("cdcdedx_%s", sname.data()), "", 1200, 1200);
  ctmp.Divide(2, 2);
  ctmp.SetBatch(kTRUE);

  std::stringstream psname;
  psname << Form("plots/HadronPrep/%s.pdf[", sname.data());
  ctmp.Print(psname.str().c_str());
  psname.str("");
  psname << Form("plots/HadronPrep/%s.pdf", sname.data());

  for (int i = 0 ; i < nbins; ++i) {
    ctmp.cd(i % 4 + 1);
    hist[i]->SetFillColor(kYellow - 9);
    hist[i]->Draw();

    if ((i + 1) % 4 == 0 || (i + 1) == nbins) {
      ctmp.Print(psname.str().c_str());
      ctmp.Clear("D");
    }
  }
  psname.str("");
  psname << Form("plots/HadronPrep/%s.pdf]", sname.data());
  ctmp.Print(psname.str().c_str());
}

//----------------------------------------
void HadronBgPrep::setPars(TFile*& outfile, std::string& pdg, std::vector<TH1F*>& hdedx_bg, std::vector<TH1F*>& hchi_bg,
                           std::vector<TH1F*>& hionzsigma_bg, std::map<int, std::vector<TH1F*>>& hchi_inj)
{
  outfile->cd();

  TTree* satTree = new TTree(Form("%s", pdg.data()), "dE/dx m_means and m_errors");
  double satbg;          // beta-gamma value for this bin
  double satcosth;       // cos(theta) value for this bin
  double satdedx;        // mean dE/dx value for this bin
  double satdedxerr;     // error on ^
  double satdedxwidth;// width of ^ distribution

  double satchi;         // mean chi value for this bin
  double satchierr;      // error on ^
  double satchiwidth;    // width of ^ distribution
  double satchiwidth_err;
  double sationzres;   // width of dedx reso

  double satbg_avg;      // average beta-gamma value for this sample
  double satcosth_avg;   // average cos(theta) value for this sample
  double satdedxres_avg; // average dE/dx error squared for this sample

  satTree->Branch("bg", &satbg, "bg/D");
  satTree->Branch("costh", &satcosth, "costh/D");

  //dEdx related variables
  satTree->Branch("dedx", &satdedx, "dedx/D");
  satTree->Branch("dedxerr", &satdedxerr, "dedxerr/D");
  satTree->Branch("dedxwidth", &satdedxwidth, "dedxwidth/D");

  // Chi related variables
  satTree->Branch("chimean", &satchi, "chimean/D");
  satTree->Branch("chimean_err", &satchierr, "chimean_err/D");
  satTree->Branch("chisigma", &satchiwidth, "chisigma/D");
  satTree->Branch("chisigma_err", &satchiwidth_err, "chisigma_err/D");

  satTree->Branch("ionzres", &sationzres, "ionzres/D");

  // Other variables
  satTree->Branch("bg_avg", &satbg_avg, "bg_avg/D");
  satTree->Branch("costh_avg", &satcosth_avg, "costh_avg/D");
  satTree->Branch("dedxres_avg", &satdedxres_avg, "dedxres_avg/D");

  double bgstep = (m_bgMax - m_bgMin) / m_bgBins;

  for (int i = 0; i < m_bgBins; ++i) {

    // fill some details for this bin
    satbg = m_bgMin + 0.5 * bgstep + i * bgstep;
    satcosth = 0.0;

    satbg_avg = m_sumbg[i] / m_sumsize[i];
    satcosth_avg = m_sumcos[i] / m_sumsize[i];
    satdedxres_avg = m_sumres_square[i] / m_sumsize[i];

    //1. -------------------------
    // fit the dE/dx distribution in bins of beta-gamma
    fit(hdedx_bg[i],  pdg.data());
    satdedx = m_means[i] = hdedx_bg[i]->GetFunction("gaus")->GetParameter(1);
    satdedxerr = m_errors[i] = hdedx_bg[i]->GetFunction("gaus")->GetParError(1);
    satdedxwidth = hdedx_bg[i]->GetFunction("gaus")->GetParameter(2);

    //2. -------------------------
    // fit the chi distribution  in bins of beta-gamma
    fit(hchi_bg[i], pdg.data());
    satchi = hchi_bg[i]->GetFunction("gaus")->GetParameter(1);
    satchierr  = hchi_bg[i]->GetFunction("gaus")->GetParError(1);
    satchiwidth = hchi_bg[i]->GetFunction("gaus")->GetParameter(2);
    satchiwidth_err = hchi_bg[i]->GetFunction("gaus")->GetParError(2);

    //3. -------------------------
    // fit the chi distribution  in bins of beta-gamma
    fit(hionzsigma_bg[i], pdg.data());
    sationzres = hionzsigma_bg[i]->GetFunction("gaus")->GetParameter(2);

    // fill the tree for this bin
    satTree->Fill();
  }
  // write out the data to file
  satTree->Write();

  // --------------------------------------------------
  // FIT IN BINS OF INJECTION TIME FOR VALIDATION
  // --------------------------------------------------

  std::string svar = "ler";
  for (int ir = 0; ir < 2; ir++) {

    if (ir == 1) svar = "her";

    TTree* injTree = new TTree(Form("%s_%s", pdg.data(), svar.data()), "chi m_means and m_errors");
    double inj_avg;
    double mean;
    double mean_err;
    double sigma;
    double sigma_err;

    injTree->Branch("inj_avg", &inj_avg, "inj_avg/D");
    injTree->Branch("chimean", &mean, "chimean/D");
    injTree->Branch("chimean_err", &mean_err, "chimean_err/D");
    injTree->Branch("chisigma", &sigma, "chisigma/D");
    injTree->Branch("chisigma_err", &sigma_err, "chisigma_err/D");

    for (int i = 0; i < m_injBins; ++i) {

      inj_avg = m_suminj[i] / m_injsize[i];

      // fit the dE/dx distribution in bins of injection time'
      fit(hchi_inj[ir][i],  pdg.data());

      mean = hchi_inj[ir][i]->GetFunction("gaus")->GetParameter(1);
      mean_err = hchi_inj[ir][i]->GetFunction("gaus")->GetParError(1);
      sigma = hchi_inj[ir][i]->GetFunction("gaus")->GetParameter(2);
      sigma_err = hchi_inj[ir][i]->GetFunction("gaus")->GetParError(2);

      injTree->Fill();
    }

    injTree->Write();
  }
  outfile->Write();
}

//----------------------------------------
void HadronBgPrep::fit(TH1F*& hist, std::string pdg)
{
  gstatus status;
  if (pdg == "pion") fitGaussianWRange(hist, status, 1.0);
  else fitGaussianWRange(hist, status, 2.0);

  hist->SetFillColorAlpha(kAzure + 1, 0.30);

  if (status == OK) {
    double mean = hist->GetFunction("gaus")->GetParameter(1);
    double meanerr = hist->GetFunction("gaus")->GetParError(1);
    double width = hist->GetFunction("gaus")->GetParameter(2);

    std::string title = Form("#mu_{fit}: %0.03f #pm %0.03f, #sigma_{fit}: %0.03f", mean, meanerr, width);
    hist->SetTitle(Form("%s, %s", hist->GetTitle(), title.data()));
  }

}

//----------------------------------------
void HadronBgPrep::fitGaussianWRange(TH1F*& temphist, gstatus& status, double sigmaR)
{
  double histmean = temphist->GetMean();
  double histrms = temphist->GetRMS();
  temphist->GetXaxis()->SetRangeUser(histmean - 5.0 * histrms, histmean + 5.0 * histrms);

  int fs = temphist->Fit("gaus", "Q");
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


// --------------------------------------------------
// FIT IN BINS OF COS(THETA) FOR VALIDATION
// --------------------------------------------------
void HadronBgPrep::printCanvasCos(std::map<int, std::vector<TH1F*>>& hchicos_allbg,
                                  std::map<int, std::vector<TH1F*>>& hchicos_1by3bg, std::map<int, std::vector<TH1F*>>& hchicos_2by3bg,
                                  std::map<int, std::vector<TH1F*>>& hchicos_3by3bg, std::string pdg, std::string suffix)
{

  double chicos[2][m_cosBins], sigmacos[2][m_cosBins];
  double chicoserr[2][m_cosBins], sigmacoserr[2][m_cosBins];

  double chicos_1b3bg[2][m_cosBins], sigmacos_1b3bg[2][m_cosBins];
  double chicos_1b3bgerr[2][m_cosBins], sigmacos_1b3bgerr[2][m_cosBins];

  double chicos_2b3bg[2][m_cosBins], sigmacos_2b3bg[2][m_cosBins];
  double chicos_2b3bgerr[2][m_cosBins], sigmacos_2b3bgerr[2][m_cosBins];

  double chicos2[2][m_cosBins], sigmacos2[2][m_cosBins];
  double chicos2err[2][m_cosBins], sigmacos2err[2][m_cosBins];

  for (int c = 0; c < 2; ++c) {
    for (int i = 0; i < m_cosBins; ++i) {
      if (hchicos_allbg[c][i]->Integral() > 100) {
        fit(hchicos_allbg[c][i],  pdg.data());
        chicos[c][i] = hchicos_allbg[c][i]->GetFunction("gaus")->GetParameter(1);
        chicoserr[c][i] = hchicos_allbg[c][i]->GetFunction("gaus")->GetParError(1);
        sigmacos[c][i] = hchicos_allbg[c][i]->GetFunction("gaus")->GetParameter(2);
        sigmacoserr[c][i] = hchicos_allbg[c][i]->GetFunction("gaus")->GetParError(2);
      }

      if (hchicos_1by3bg[c][i]->Integral() > 100) {
        fit(hchicos_1by3bg[c][i],  pdg.data());
        chicos_1b3bg[c][i] = hchicos_1by3bg[c][i]->GetFunction("gaus")->GetParameter(1);
        chicos_1b3bgerr[c][i] = hchicos_1by3bg[c][i]->GetFunction("gaus")->GetParError(1);
        sigmacos_1b3bg[c][i] = hchicos_1by3bg[c][i]->GetFunction("gaus")->GetParameter(2);
        sigmacos_1b3bgerr[c][i] = hchicos_1by3bg[c][i]->GetFunction("gaus")->GetParError(2);
      }

      if (hchicos_2by3bg[c][i]->Integral() > 100) {
        fit(hchicos_2by3bg[c][i],  pdg.data());
        chicos_2b3bg[c][i] = hchicos_2by3bg[c][i]->GetFunction("gaus")->GetParameter(1);
        chicos_2b3bgerr[c][i] = hchicos_2by3bg[c][i]->GetFunction("gaus")->GetParError(1);
        sigmacos_2b3bg[c][i] = hchicos_2by3bg[c][i]->GetFunction("gaus")->GetParameter(2);
        sigmacos_2b3bgerr[c][i] = hchicos_2by3bg[c][i]->GetFunction("gaus")->GetParError(2);
      }

      if (hchicos_3by3bg[c][i]->Integral() > 100) {
        fit(hchicos_3by3bg[c][i],  pdg.data());
        chicos2[c][i] = hchicos_3by3bg[c][i]->GetFunction("gaus")->GetParameter(1);
        chicos2err[c][i] = hchicos_3by3bg[c][i]->GetFunction("gaus")->GetParError(1);
        sigmacos2[c][i] = hchicos_3by3bg[c][i]->GetFunction("gaus")->GetParameter(2);
        sigmacos2err[c][i] = hchicos_3by3bg[c][i]->GetFunction("gaus")->GetParError(2);
      }
    }
  }


  plotDist(hchicos_allbg,  Form("fits_chi_vscos_allbg_%s_%s", pdg.data(), suffix.data()), m_cosBins);
  plotDist(hchicos_1by3bg, Form("fits_chi_vscos_1by3bg_%s_%s", pdg.data(), suffix.data()), m_cosBins);
  plotDist(hchicos_2by3bg,  Form("fits_chi_vscos_2by3bg_%s_%s", pdg.data(), suffix.data()), m_cosBins);
  plotDist(hchicos_3by3bg, Form("fits_chi_vscos_3by3bg_%s_%s", pdg.data(), suffix.data()), m_cosBins);

  const double cosstep = 2.0 / m_cosBins;
  double cosArray[m_cosBins], cosArrayErr[m_cosBins];
  for (int i = 0; i < m_cosBins; ++i) {
    cosArray[i] = -1.0 + (i * cosstep + cosstep / 2.0); //finding bin centre
    cosArrayErr[i] = 0.0;
  }

  TGraphErrors grchicos(m_cosBins, cosArray, chicos[0], cosArrayErr, chicoserr[0]);
  TGraphErrors grchicos_1b3bg(m_cosBins, cosArray, chicos_1b3bg[0], cosArrayErr, chicos_1b3bgerr[0]);
  TGraphErrors grchicos_2b3bg(m_cosBins, cosArray, chicos_2b3bg[0], cosArrayErr, chicos_2b3bgerr[0]);
  TGraphErrors grchicos2(m_cosBins, cosArray, chicos2[0], cosArrayErr, chicos2err[0]);

  TGraphErrors grchicosn(m_cosBins, cosArray, chicos[1], cosArrayErr, chicoserr[1]);
  TGraphErrors grchicos_1b3bgn(m_cosBins, cosArray, chicos_1b3bg[1], cosArrayErr, chicos_1b3bgerr[1]);
  TGraphErrors grchicos_2b3bgn(m_cosBins, cosArray, chicos_2b3bg[1], cosArrayErr, chicos_2b3bgerr[1]);
  TGraphErrors grchicos2n(m_cosBins, cosArray, chicos2[1], cosArrayErr, chicos2err[1]);

  TGraphErrors grsigmacos(m_cosBins, cosArray, sigmacos[0], cosArrayErr, sigmacoserr[0]);
  TGraphErrors grsigmacos_1b3bg(m_cosBins, cosArray, sigmacos_1b3bg[0], cosArrayErr, sigmacos_1b3bgerr[0]);
  TGraphErrors grsigmacos_2b3bg(m_cosBins, cosArray, sigmacos_2b3bg[0], cosArrayErr, sigmacos_2b3bgerr[0]);
  TGraphErrors grsigmacos2(m_cosBins, cosArray, sigmacos2[0], cosArrayErr, sigmacos2err[0]);

  TGraphErrors grsigmacosn(m_cosBins, cosArray, sigmacos[1], cosArrayErr, sigmacoserr[1]);
  TGraphErrors grsigmacos_1b3bgn(m_cosBins, cosArray, sigmacos_1b3bg[1], cosArrayErr, sigmacos_1b3bgerr[1]);
  TGraphErrors grsigmacos_2b3bgn(m_cosBins, cosArray, sigmacos_2b3bg[1], cosArrayErr, sigmacos_2b3bgerr[1]);
  TGraphErrors grsigmacos2n(m_cosBins, cosArray, sigmacos2[1], cosArrayErr, sigmacos2err[1]);

  TLine line0(-1, 0, 1, 0);
  line0.SetLineStyle(kDashed);
  line0.SetLineColor(kRed);

  TLine line1(-1, 1, 1, 1);
  line1.SetLineStyle(kDashed);
  line1.SetLineColor(kRed);

  TString ptype("");
  double mass = 0.0;
  if (pdg == "pion") {ptype += "#pi"; mass = Const::pion.getMass();}
  else if (pdg == "kaon") {ptype += "K"; mass = Const::kaon.getMass();}
  else if (pdg == "proton") { ptype += "p"; mass = Const::proton.getMass();}
  else if (pdg == "muon") {ptype += "#mu"; mass = Const::muon.getMass();}
  else if (pdg == "electron") {ptype += "e"; mass = Const::electron.getMass();}
  if (mass == 0.0) B2FATAL("Mass of particle " << pdg.data() << " is zero");

  int bglow = 0, bghigh = 0;
  double bgstep = (m_bgMax - m_bgMin) / m_bgBins;

  TLegend lchi(0.7, 0.75, 0.8, 0.85);
  lchi.SetBorderSize(0);
  lchi.SetFillColor(kYellow);
  lchi.AddEntry(&grchicos, ptype + "^{+}", "pc");
  lchi.AddEntry(&grchicosn, ptype + "^{-}", "pc");

  TCanvas cchi(Form("cchi_%s", suffix.data()), "cchi", 700, 600);
  cchi.Divide(2, 2);
  cchi.cd(1);
  FormatGraph(grchicos, 0, Form("all (%s) bg bins, p =(%0.02f, %0.02f)", pdg.data(), m_bgMin * mass, m_bgMax * mass));
  FormatGraph(grchicosn, 1);
  grchicos.Draw("AP");
  grchicosn.Draw("P,same");
  line0.Draw("same");
  lchi.Draw("same");

  cchi.cd(2);
  bghigh = int(m_bgBins / 3);
  FormatGraph(grchicos_1b3bg, 0, Form("first 1/3 bg bins, p =(%0.02f, %0.02f)", m_bgMin * mass, bghigh * bgstep * mass));
  FormatGraph(grchicos_1b3bgn, 1);
  grchicos_1b3bg.Draw("AP");
  grchicos_1b3bgn.Draw("P,same");
  line0.Draw("same");
  lchi.Draw("same");

  cchi.cd(3);
  bglow = int(m_bgBins / 3);
  bghigh = int(2 * m_bgBins / 3);
  FormatGraph(grchicos_2b3bg, 0, Form("second 1/3 bg bins, p =(%0.02f, %0.02f)", bglow * bgstep * mass, bghigh * bgstep * mass));
  FormatGraph(grchicos_2b3bgn, 1);
  grchicos_2b3bg.Draw("AP");
  grchicos_2b3bgn.Draw("P,same");
  line0.Draw("same");
  lchi.Draw("same");

  cchi.cd(4);
  bglow = int(2 * m_bgBins / 3);
  FormatGraph(grchicos2, 0, Form("third 1/3 bg bins, p =(%0.02f, %0.02f)", bglow * bgstep * mass, m_bgMax * mass));
  FormatGraph(grchicos2n, 1);
  grchicos2.Draw("AP");
  grchicos2n.Draw("P,same");
  line0.Draw("same");
  lchi.Draw("same");
  cchi.SaveAs(Form("plots/HadronCal/Monitoring/mean_chi_vscos_inbg_%s_%s.pdf", pdg.data(), suffix.data()));

  cchi.cd(1);
  FormatGraph(grsigmacos, 2, Form("all (%s) bg bins, p =(%0.02f, %0.02f)", pdg.data(), m_bgMin * mass, m_bgMax * mass));
  FormatGraph(grsigmacosn, 1);
  grsigmacos.Draw("AP");
  grsigmacosn.Draw("P,same");
  line1.Draw("same");
  lchi.Draw("same");

  cchi.cd(2);
  bghigh = int(m_bgBins / 3);
  FormatGraph(grsigmacos_1b3bg, 2, Form("first 1/3 bg bins, p =(%0.02f, %0.02f)", m_bgMin * mass, bghigh * bgstep * mass));
  FormatGraph(grsigmacos_1b3bgn, 1);
  grsigmacos_1b3bg.Draw("AP");
  grsigmacos_1b3bgn.Draw("P,same");
  line1.Draw("same");
  lchi.Draw("same");

  cchi.cd(3);
  bglow = int(m_bgBins / 3);
  bghigh = int(2 * m_bgBins / 3);
  FormatGraph(grsigmacos_2b3bg, 2, Form("second 1/3 bg bins, p =(%0.02f, %0.02f)", bglow * bgstep * mass, bghigh * bgstep * mass));
  FormatGraph(grsigmacos_2b3bgn, 1);
  grsigmacos_2b3bg.Draw("AP");
  grsigmacos_2b3bgn.Draw("P,same");
  line1.Draw("same");
  lchi.Draw("same");

  cchi.cd(4);
  bglow = int(2 * m_bgBins / 3);
  FormatGraph(grsigmacos2, 2, Form("third 1/3 bg bins, p =(%0.02f, %0.02f)", bglow * bgstep * mass, m_bgMax * mass));
  FormatGraph(grsigmacos2n, 1);
  grsigmacos2.Draw("AP");
  grsigmacos2n.Draw("P,same");
  line1.Draw("same");
  lchi.Draw("same");
  cchi.SaveAs(Form("plots/HadronCal/Monitoring/sigma_chi_vscos_inbg_%s_%s.pdf", pdg.data(), suffix.data()));

}
