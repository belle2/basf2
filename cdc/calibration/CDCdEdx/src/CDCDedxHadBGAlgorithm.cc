/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/calibration/CDCdEdx/CDCDedxHadBGAlgorithm.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxHadBGAlgorithm::CDCDedxHadBGAlgorithm() :
  CalibrationAlgorithm("CDCDedxHadronCollector"),
  m_ismakePlots(true),
  m_suffix("")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx hadron Beta Gamma curve and resolution fitting");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxHadBGAlgorithm::calibrate()
{

  gROOT->SetBatch();
  getExpRunInfo();

  //existing hadron bg mean and reso payload for merging
  if (!m_DBMeanPars.isValid() || !m_DBSigmaPars.isValid())
    B2FATAL("There is no valid payload for Beta-Gamma saturation");

  // particle list
  std::vector< std::string > particles = {"muon", "kaon", "proton", "pion", "electron"};

  // check we have enough data
  for (int i = 0; i < int(particles.size()); i++) {
    std::string p = particles[i];
    auto tree = getObjectPtr<TTree>(Form("%s", p.data()));
    if (!tree) return c_NotEnoughData;
    std::cout << "Entries:   " << p.data() << "   " << tree->GetEntries() << std::endl;
  }

  gSystem->Exec("mkdir -p plots/HadronPrep");
  gSystem->Exec("mkdir -p plots/HadronCal/BGfits");
  gSystem->Exec("mkdir -p plots/HadronCal/Resofits");
  gSystem->Exec("mkdir -p plots/HadronCal/Monitoring");

  // Write beta-gamma curve mean and resolution parameters in text file
  CDCDedxMeanPred mg;
  mg.setParameters();
  mg.printParameters("parameters.inital.curve");

  CDCDedxSigmaPred sg;
  sg.setParameters();
  sg.printParameters("parameters.inital.sigma");

  m_bgcurve = "parameters.inital.curve";
  m_bgsigma = "parameters.inital.sigma";

  HadronCalibration hadcal;
  std::string filename = " ";

  for (int iter = 0; iter < m_iter; ++iter) {

    std::string sfx = Form("%s_iter%d", m_suffix.data(), iter);
    filename = Form("widget_corrected_NewHSpars_1D_%s.root", sfx.data());

    // prepare sample to perform bg curve fittting and draw the monitoring plots
    prepareSample(particles, filename, sfx);

    // create the HadronCalibration object and fit the prepared samples
    hadcal.plotBGMonitoring(particles, filename, sfx);

    //bg fit
    hadcal.fitBGCurve(particles, filename, m_bgcurve, sfx);
    m_bgcurve = "parameters.bgcurve.fit";

    //dedx reso vs ionz fit
    hadcal.fitSigmavsIonz(particles, filename, m_bgsigma, sfx);
    m_bgsigma = "parameters.ionz.fit";

    //dedx reso vs nHits fit
    SigmaFits(particles, sfx, "nhit");
    m_bgsigma = "parameters.sigmanhit.fit";

    //dedx reso vs costh fit
    SigmaFits(particles, sfx, "costh");
    m_bgsigma = "parameters.sigmacos.fit";

  }

  filename = Form("widget_corrected_NewHSpars_1D_%s_final.root", m_suffix.data());
  prepareSample(particles, filename, Form("%s_final", m_suffix.data()));
  hadcal.plotBGMonitoring(particles, filename, Form("%s_final", m_suffix.data()));

  B2INFO("Saving calibration for: " << m_suffix << "");
  createPayload();

  return c_OK;
}


//------------------------------------
void CDCDedxHadBGAlgorithm::getExpRunInfo()
{
  int cruns = 0;
  for (auto expRun : getRunList()) {
    if (cruns == 0)B2INFO("CDCDedxHadBGAlgorithm: start exp " << expRun.first << " and run " << expRun.second << "");
    cruns++;
  }

  const auto erStart = getRunList()[0];
  int estart = erStart.first;
  int rstart = erStart.second;

  updateDBObjPtrs(1, erStart.second, erStart.first);

  m_suffix = Form("e%dr%d", estart, rstart);
  B2INFO("tool exp = " << estart << ", run = " << rstart << ", m_suffix = " << m_suffix << "");
}

//--------------------------
void CDCDedxHadBGAlgorithm::createPayload()
{

  std::ifstream fins("parameters.sigmacos.fit"), fin("parameters.bgcurve.fit");

  if (!fin.good()) B2FATAL("\t WARNING: CANNOT FIND parameters.bgcurve.fit!");
  if (!fins.good()) B2FATAL("\tWARNING: CANNOT FIND parameters.sigmacos.fit!");

  int par;
  double meanpars, sigmapars;
  std::vector<double> v_meanpars, v_sigmapars;

  B2INFO("\t --> Curve parameters");
  for (int i = 0; i < 15; ++i) {
    fin >> par >> meanpars;
    v_meanpars.push_back(meanpars);
    B2INFO("\t\t (" << i << ")" << v_meanpars[i]);
  }

  fin.close();

  B2INFO("\t --> Sigma parameters");
  for (int i = 0; i < 17; ++i) {
    fins >> par >> sigmapars;
    v_sigmapars.push_back(sigmapars);
    B2INFO("\t\t (" << i << ")" << v_sigmapars[i]);
  }
  fins.close();

  B2INFO("dE/dx Calibration done for " << v_meanpars.size() << " CDC Beta Gamma curve");
  CDCDedxMeanPars* gains = new CDCDedxMeanPars(0, v_meanpars);
  saveCalibration(gains, "CDCDedxMeanPars");


  B2INFO("dE/dx Calibration done for " << v_sigmapars.size() << " CDC Beta Gamma resolution");
  CDCDedxSigmaPars* sgains = new CDCDedxSigmaPars(0, v_sigmapars);
  saveCalibration(sgains, "CDCDedxSigmaPars");
}

void CDCDedxHadBGAlgorithm::prepareSample(std::vector< std::string > particles, std::string filename, std::string sfx)
{

  TFile* outfile = new TFile(filename.data(), "RECREATE");

  for (int i = 0; i < int(particles.size()); ++i) {

    std::string p =  particles[i];
    auto tree = getObjectPtr<TTree>(Form("%s", p.data()));

    HadronBgPrep prep(m_bgpar[p][0], m_bgpar[p][1], m_bgpar[p][2], 8, -1.0, 1.0, m_injpar[p][0], m_injpar[p][1],
                      m_injpar[p][2], m_nhitBins, m_nhitMin, m_nhitMax, m_cut);

    prep.prepareSample(tree, outfile, sfx, m_bgcurve, m_bgsigma, p, m_ismakePlots);
  }
  outfile->Close();

}

void CDCDedxHadBGAlgorithm::SigmaFits(std::vector< std::string > particles, std::string sfx, std::string svar)
{
  // only the muon samples are used for the sigma fits
  HadronCalibration hadcal;

  std::string filename = Form("widget_%s_1D_%s.root", svar.data(), sfx.data());

  TFile* outfile = new TFile(filename.data(), "RECREATE");
  outfile->cd();

  for (int ip = 0; ip < int(particles.size()); ++ip) {

    std::string particle =  particles[ip];
    auto hadron = getObjectPtr<TTree>(Form("%s", particle.data()));

    HadronBgPrep prep(m_bgpar[particle][0], m_bgpar[particle][1], m_bgpar[particle][2], m_cospar[particle], m_cosMin, m_cosMax,
                      m_injpar[particle][0],
                      m_injpar[particle][1], m_injpar[particle][2], m_nhitBins, m_nhitMin, m_nhitMax, m_cut);

    double mass = prep.getParticleMass(particle);
    if (mass == 0.0) B2FATAL("Mass of particle " << particle.data() << " is zero");
    // --------------------------------------------------
    // INITIALIZE CONTAINERS
    // --------------------------------------------------
    double dedxnosat;    // dE/dx without hadron saturation correction
    double p;       // track momentum
    double bg;      // track beta-gamma
    double costh;   // cosine of track polar angle
    double timereso;
    int nhits;       // number of hits on this track

    hadron->SetBranchAddress("dedxnosat", &dedxnosat);
    hadron->SetBranchAddress("p", &p);
    hadron->SetBranchAddress("costh", &costh);
    hadron->SetBranchAddress("timereso", &timereso);
    hadron->SetBranchAddress("nhits", &nhits);

    int  nbins = m_nhitBins;
    double lower = m_nhitMin;
    double upper = m_nhitMax;
    double nstep = (upper - lower + 1) / nbins;

    if (svar == "costh") {
      nbins = m_cospar[particle];
      lower = m_cosMin, upper = m_cosMax;
      nstep = (upper - lower) / nbins;
    }

    // Create the histograms to be fit
    std::vector<TH1F*>   hdedx_var;

    //define histograms
    prep.defineHisto(hdedx_var, "chi", svar, particle.data());

    // Create some containers to calculate averages
    double sumvar[nbins];
    int sumsize[nbins];
    for (int i = 0; i < nbins; ++i) {
      sumvar[i] = 0;
      sumsize[i] = 0;
    }

    // get the hadron saturation parameters
    CDCDedxMeanPred mgpar;
    CDCDedxSigmaPred sgpar;

    mgpar.setParameters(m_bgcurve.data());
    sgpar.setParameters(m_bgsigma.data());

    CDCDedxHadSat had;
    had.setParameters("sat-pars.fit.txt");
    // --------------------------------------------------
    // LOOP OVER EVENTS AND FILL CONTAINERS
    // --------------------------------------------------
    // Fill the histograms to be fitted

    for (unsigned int index = 0; index < hadron->GetEntries(); ++index) {

      hadron->GetEvent(index);

      bg = fabs(p) / mass;

      if (fabs(p) > 8.0)continue;   //unphysical tracks

      // clean up bad events and restrict the momentum range
      if (svar == "nhit") {if (nhits <  lower || nhits  > upper) continue;}
      else if (svar == "costh") {if (costh > upper || costh < lower)continue;}

      if (dedxnosat <= 0)continue;
      if (costh != costh)continue;

      if (particle == "proton") {if ((dedxnosat - 0.45)*abs(p)*abs(p) < m_cut) continue;}

      if (particle == "electron" || particle == "muon") {if (fabs(p) > 2.0) continue;}

      double dedx_new = dedxnosat;
      dedx_new = had.D2I(costh, had.I2D(costh, 1.0) * dedxnosat);

      double dedx_cur = mgpar.getMean(bg);

      if (svar == "nhit") {
        double res_cor = sgpar.cosPrediction(costh) * sgpar.ionzPrediction(dedx_cur) * timereso;
        int nhitBin = (int)((fabs(nhits) - lower) / nstep);
        hdedx_var[nhitBin]->Fill((dedx_new - dedx_cur) / res_cor);
        sumvar[nhitBin] += nhits;
        sumsize[nhitBin] += 1;
      } else if (svar == "costh") {

        double res_cor = sgpar.nhitPrediction(nhits) * sgpar.ionzPrediction(dedx_cur) * timereso;
        int cosBin = (int)((costh - lower) / (upper - lower) * nbins);

        hdedx_var[cosBin]->Fill((dedx_new - dedx_cur) / res_cor);

        sumvar[cosBin] += costh;
        sumsize[cosBin] += 1;
      }
    }// end of event loop

    // --------------------------------------------------
    // FIT IN BINS OF NHIT
    // --------------------------------------------------
    // fit the histograms with Gaussian functions
    // and extract the means and errors


    TTree* tTree = new TTree(Form("%s_%s", particle.data(), svar.data()), "chi m_means and m_errors");
    double avg,  mean, mean_err, sigma, sigma_err;

    tTree->Branch("avg", &avg, "avg/D");
    tTree->Branch("chimean", &mean, "chimean/D");
    tTree->Branch("chimean_err", &mean_err, "chimean_err/D");
    tTree->Branch("chisigma", &sigma, "chisigma/D");
    tTree->Branch("chisigma_err", &sigma_err, "chisigma_err/D");

    double avg_sigma = 0.0;
    double var[nbins], varres[nbins], varreserr[nbins];

    int count_bins = 0;
    for (int i = 0; i < nbins; ++i) {

      varres[i] = 0.0;
      varreserr[i] = 0.0;
      var[i] = sumvar[i] / sumsize[i];

      // fit the dE/dx distribution in bins of injection time'
      if (hdedx_var[i]->Integral() > 100) {
        prep.fit(hdedx_var[i],  particle.data());
        varres[i] = hdedx_var[i]->GetFunction("gaus")->GetParameter(2);;
        varreserr[i] = hdedx_var[i]->GetFunction("gaus")->GetParError(2);
        count_bins++;
        avg_sigma += varres[i];

      }
    }
    if (count_bins > 0) avg_sigma = avg_sigma / count_bins;
    for (int i = 0; i < nbins; ++i) {

      if (avg_sigma > 0) {
        sigma = varres[i] = varres[i] / avg_sigma;
        sigma_err = varreserr[i] = varreserr[i] / avg_sigma;
      }
      mean =  hdedx_var[i]->GetFunction("gaus")->GetParameter(1);
      mean_err =  hdedx_var[i]->GetFunction("gaus")->GetParError(1);
      avg = var[i];
      tTree->Fill();
    }

    tTree->Write();

    prep.plotDist(hdedx_var, Form("fits_chi_%s_%s_%s", svar.data(), sfx.data(), particle.data()), nbins);

    prep.deleteHistos(hdedx_var, nbins);
  }
  outfile->Close();

  if (svar == "costh") hadcal.fitSigmaVsCos(particles, filename, m_bgsigma, sfx);
  else hadcal.fitSigmaVsNHit(particles, filename, m_bgsigma, sfx);

}


