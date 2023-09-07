/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/calibration/CDCDedxInjectTimeAlgorithm.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedxInjectTimeAlgorithm::CDCDedxInjectTimeAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  m_sigmaR(2.0),
  m_dedxBins(250),
  m_dedxMin(0.0),
  m_dedxMax(2.5),
  m_chiBins(250),
  m_chiMin(-10.0),
  m_chiMax(10.0),
  m_countR(0),
  m_thersE(1000),
  m_isminStat(false),
  m_ismakePlots(true),
  m_isMerge(true),
  m_prefix("cdcdedx_injcal"),
  m_suffix("")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx injection time gain and reso");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedxInjectTimeAlgorithm::calibrate()
{

  getExpRunInfo();

  //existing inject time payload for merging
  if (!m_DBInjectTime.isValid())
    B2FATAL("There is no valid payload for Injection time");

  // Get data objects
  auto ttree = getObjectPtr<TTree>("tree");
  if (!ttree) return c_NotEnoughData;

  double dedx = 0.0, injtime = 0.0, injring = 1.0, costh, mom;
  int nhits;
  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("injtime", &injtime);
  ttree->SetBranchAddress("injring", &injring);
  ttree->SetBranchAddress("costh", &costh);
  ttree->SetBranchAddress("p", &mom);
  ttree->SetBranchAddress("nhits", &nhits);

  //way to define time bins/edges only once
  if (m_countR == 0) {
    defineTimeBins(); //returns m_vtlocaledges
    m_tbins = m_vtlocaledges.size() - 1;
    m_tedges = &m_vtlocaledges[0];
    m_countR++;
  }

  TH1D* htimes = new TH1D(Form("htimes_%s", m_suffix.data()), "", m_tbins, m_tedges);

  //time bins are changable from out so vector is used
  std::array<std::vector<TH1D*>, numdedx::nrings> hdedx, htime, hchi, hdedx_corr;
  defineHisto(hdedx, "dedx");
  defineHisto(hdedx_corr, "dedx_corr");
  defineHisto(htime, "timeinj");
  defineHisto(hchi, "chi");

  const double tzedges[4] = {0, 2.0e3, 0.1e6, 20e6};
  std::array<std::array<TH1D*, 3>, numdedx::nrings> hztime;
  defineTimeHisto(hztime);

  //fill histograms
  for (int i = 0; i < ttree->GetEntries(); ++i) {

    ttree->GetEvent(i);
    if (dedx <= 0 || injtime < 0 || injring < 0) continue;

    //add larger times to the last bin
    if (injtime > m_tedges[m_tbins]) injtime = m_tedges[m_tbins] - 10.0;

    //injection ring
    int wr = 0;
    if (injring > 0.5) wr = 1;

    //injection time bin
    unsigned int tb = htimes->GetXaxis()->FindBin(injtime);
    if (tb > m_tbins)tb = m_tbins; //overflow
    tb = tb - 1;

    htimes->Fill(injtime);
    if (injtime < tzedges[1]) hztime[wr][0]->Fill(injtime);
    else if (injtime < tzedges[2]) hztime[wr][1]->Fill(injtime);
    else hztime[wr][2]->Fill(injtime);

    hdedx[wr][tb]->Fill(dedx);
    htime[wr][tb]->Fill(injtime);
  }

  //keep merging runs to achive enough stats
  m_isminStat = false;
  checkStatistics(hdedx);
  if (m_isminStat) {
    deleteHisto(htime);
    deleteHisto(hdedx);
    deleteHisto(hchi);
    deleteTimeHisto(hztime);
    delete htimes;
    return c_NotEnoughData;
  }

  //clear vector of existing constants
  std::map<int, std::vector<double>> vmeans, vresos, vtimes;

  // get time vector
  for (unsigned int ir = 0; ir < c_rings; ir++) {
    for (unsigned int it = 0; it < m_tbins; it++) {
      double avgtime = htime[ir][it]->GetMean();
      double avgtimeerr = htime[ir][it]->GetMeanError();
      vtimes[ir * 2].push_back(avgtime);
      vtimes[ir * 2 + 1].push_back(avgtimeerr);
    }
  }

  //Fit dedx to get mean and resolution
  getMeanReso(hdedx, vmeans, vresos);

  //Bin-bias correction to mean
  std::map<int, std::vector<double>> vmeanscorr;
  correctBinBias(vmeanscorr, vmeans, vtimes, htimes);

  //scale the mean and merge with old constants
  std::array<double, numdedx::nrings> scale;
  std::map<int, std::vector<double>> vmeanscal;
  createPayload(scale, vmeanscorr, vmeanscal, "mean");

  //................................................
  // Do the calibration for resolution
  //................................................
  CDCDedxMeanPred m;
  CDCDedxSigmaPred s;
  //fill histograms for the resolution
  for (int i = 0; i < ttree->GetEntries(); ++i) {

    ttree->GetEvent(i);
    if (dedx <= 0 || injtime < 0 || injring < 0) continue;

    double corrcetion = getCorrection(injring, injtime, vmeanscal);
    double old_cor = m_DBInjectTime->getCorrection("mean", injring, injtime);

    dedx = (dedx * old_cor) / corrcetion;
    //add larger times to the last bin
    if (injtime > m_tedges[m_tbins]) injtime = m_tedges[m_tbins] - 10.0;

    //injection ring
    int wr = 0;
    if (injring > 0.5) wr = 1;

    //injection time bin
    unsigned int tb = htimes->GetXaxis()->FindBin(injtime);
    if (tb > m_tbins)tb = m_tbins; //overflow
    tb = tb - 1;

    double predmean = m.getMean(mom / Const::electron.getMass());
    double predsigma = s.nhitPrediction(nhits) * s.ionzPrediction(dedx) * s.cosPrediction(costh);

    double chi = (dedx - predmean) / predsigma;
    hdedx_corr[wr][tb]->Fill(dedx);
    hchi[wr][tb]->Fill(chi);
  }

  // fit chi to get mean and resolution
  std::map<int, std::vector<double>> vmeans_chi, vresos_chi;
  getMeanReso(hchi, vmeans_chi, vresos_chi);

  //bin-bias correction to the resolution
  std::map<int, std::vector<double>> vresoscorr;
  correctBinBias(vresoscorr, vresos_chi, vtimes, htimes);

  // scale the resolution
  std::map<int, std::vector<double>> vresoscal;
  std::array<double, numdedx::nrings> scale_reso;
  createPayload(scale_reso, vresoscorr, vresoscal, "reso");

  //Fit the corrected mean to check for consistency
  std::map<int, std::vector<double>> vmeans_corr, vresos_corr;
  getMeanReso(hdedx_corr, vmeans_corr, vresos_corr);

  //................................................
  //preparing final payload
  //................................................
  m_vinjPayload.clear();
  m_vinjPayload.reserve(6);
  for (int ir = 0; ir < 2; ir++) {
    m_vinjPayload.push_back(m_vtlocaledges);
    m_vinjPayload.push_back(vmeanscal[ir * 2]);
    m_vinjPayload.push_back(vresoscal[ir * 2]);
  }

  if (m_ismakePlots) {

    //0 plot event track statistics
    plotEventStats();

    //1 plot injection time
    plotInjectionTime(hztime);

    //2. Draw dedxfits and chifits
    plotBinLevelDist(hdedx, "dedxfits");

    //3. Draw chifits
    plotBinLevelDist(hchi, "chifits");

    //4. Draw timedist
    plotBinLevelDist(htime, "timedist");

    //5. plot relative const., bias-bias corrected for dedx
    plotRelConstants(vmeans, vresos, vmeanscorr, "dedx");

    //6. plot relative const., bias-bias corrected for chi
    plotRelConstants(vmeans_chi, vresos_chi, vresoscorr, "chi");

    //6. plot mean and resolution of corrected dedx to check for consistency
    plotRelConstants(vmeans_corr, vresos_corr, vresoscorr, "dedx_corr");

    //7. plot time statistics dist
    plotTimeStat(htime);

    //8. plot final merged const. and comparison to old
    plotFinalConstants(vmeanscal, vresoscal, scale, scale_reso);
  }

  //saving payloads;
  CDCDedxInjectionTime* gains = new CDCDedxInjectionTime(m_vinjPayload);
  saveCalibration(gains, "CDCDedxInjectionTime");
  B2INFO("dE/dx Injection time calibration done");

  //delete all histograms
  deleteHisto(htime);
  deleteHisto(hdedx);
  deleteHisto(hdedx_corr);
  deleteHisto(hchi);
  deleteTimeHisto(hztime);
  delete htimes;

  B2INFO("Saving calibration for: " << m_suffix << "");
  return c_OK;
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::getMeanReso(std::array<std::vector<TH1D*>, numdedx::nrings>& hvar,
                                             std::map<int, std::vector<double>>& vmeans, std::map<int, std::vector<double>>& vresos)
{
  for (unsigned int ir = 0; ir < c_rings; ir++) {

    for (unsigned int it = 0; it < m_tbins; it++) {
      double mean = 1.00, meanerr = 0.0;
      double reso = 1.00, resoerr = 0.0;
      if (hvar[ir][it]->Integral() > 250) {
        fstatus status;
        fitGaussianWRange(hvar[ir][it], status);
        if (status != fitOK) {
          mean = hvar[ir][it]->GetMean();
          hvar[ir][it]->SetTitle(Form("%s, (%d)", hvar[ir][it]->GetTitle(), status));
        } else {
          mean = hvar[ir][it]->GetFunction("gaus")->GetParameter(1);
          meanerr = hvar[ir][it]->GetFunction("gaus")->GetParError(1);
          reso = hvar[ir][it]->GetFunction("gaus")->GetParameter(2);
          resoerr = hvar[ir][it]->GetFunction("gaus")->GetParError(2);
          std::string title = Form("#mu_{fit}: %0.03f, #sigma_{fit}: %0.03f", mean, reso);
          hvar[ir][it]->SetTitle(Form("%s, %s", hvar[ir][it]->GetTitle(), title.data()));
        }
      }

      vmeans[ir * 2].push_back(mean);
      vresos[ir * 2].push_back(reso);
      vmeans[ir * 2 + 1].push_back(meanerr);
      vresos[ir * 2 + 1].push_back(resoerr);
    }
  }
}

//----------------------------------------
void CDCDedxInjectTimeAlgorithm::fitGaussianWRange(TH1D*& temphist, fstatus& status)
{
  double histmean = temphist->GetMean();
  double histrms = temphist->GetRMS();
  temphist->GetXaxis()->SetRangeUser(histmean - 5.0 * histrms, histmean + 5.0 * histrms);

  int fs = temphist->Fit("gaus", "Q0");
  if (fs != 0) {
    B2INFO(Form("\tFit (round 1) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
    status = fitFailed;
    return;
  } else {
    double mean = temphist->GetFunction("gaus")->GetParameter(1);
    double width = temphist->GetFunction("gaus")->GetParameter(2);
    temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
    fs = temphist->Fit("gaus", "QR", "", mean - m_sigmaR * width, mean + m_sigmaR * width);
    if (fs != 0) {
      B2INFO(Form("\tFit (round 2) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
      status = fitFailed;
      return;
    } else {
      temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
      B2INFO(Form("\tFit for hist (%s) sucessfull (status = %d)", temphist->GetName(), fs));
      status = fitOK;
    }
  }
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::getExpRunInfo()
{
  int cruns = 0;
  for (auto expRun : getRunList()) {
    if (cruns == 0)B2INFO("CDCDedxInjectTimeAlgorithm: start exp " << expRun.first << " and run " << expRun.second << "");
    cruns++;
  }

  const auto erStart = getRunList()[0];
  int estart = erStart.first;
  int rstart = erStart.second;

  const auto erEnd = getRunList()[cruns - 1];
  int rend = erEnd.second;

  updateDBObjPtrs(1, erStart.second, erStart.first);

  if (m_isminStat) {
    m_suffix = Form("e%dr%dto%d_nruns%d", estart, rstart, rend, cruns);
    B2INFO("\t+ run = " << rend << ", m_suffix = " << m_suffix << "");
  } else {
    m_suffix = Form("e%dr%d", estart, rstart);
    B2INFO("tool run = " << estart << ", exp = " << estart << ", m_suffix = " << m_suffix << "");
  }
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::defineTimeBins()
{
  //empty local vector or find a way to execulate this function
  //only once
  if (!m_vtlocaledges.empty()) m_vtlocaledges.clear();
  if (m_vtedges.empty()) {
    double fixedges[69];
    for (int ib = 0; ib < 69; ib++) {
      fixedges[ib] = ib * 0.5 * 1e3;
      if (ib > 40 && ib <= 60) fixedges[ib] = fixedges[ib - 1] + 1.0 * 1e3;
      else if (ib > 60 && ib <= 64) fixedges[ib] = fixedges[ib - 1] + 10.0 * 1e3;
      else if (ib > 64 && ib <= 65) fixedges[ib] = fixedges[ib - 1] + 420.0 * 1e3;
      else if (ib > 65 && ib <= 66) fixedges[ib] = fixedges[ib - 1] + 500.0 * 1e3;
      else if (ib > 66) fixedges[ib] = fixedges[ib - 1] + 2e6;
      m_vtlocaledges.push_back(fixedges[ib]);
    }
  } else {
    for (unsigned int ib = 0; ib < m_vtedges.size(); ib++)
      m_vtlocaledges.push_back(m_vtedges.at(ib));
  }
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::defineHisto(std::array<std::vector<TH1D*>, numdedx::nrings>& htemp, std::string var)
{
  for (unsigned int ir = 0; ir < c_rings; ir++) {
    htemp[ir].resize(m_tbins);
    for (unsigned int it = 0; it < m_tbins; it++) {
      std::string label = getTimeBinLabel(m_tedges[it], it);
      std::string title = Form("%s(%s), time(%s)", m_suffix.data(), m_sring[ir].data(), label.data());
      std::string hname = Form("h%s_%s_%s_t%d", var.data(), m_sring[ir].data(), m_suffix.data(), it);
      if (var == "dedx" or var == "dedx_corr") htemp[ir][it] = new TH1D(hname.data(), "", m_dedxBins, m_dedxMin, m_dedxMax);
      else if (var == "chi") htemp[ir][it] = new TH1D(hname.data(), "", m_chiBins, m_chiMin, m_chiMax);
      else htemp[ir][it] = new TH1D(hname.data(), "", 50, m_tedges[it], m_tedges[it + 1]);
      htemp[ir][it]->SetTitle(Form("%s;%s;entries", title.data(), var.data()));
    }
  }
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::defineTimeHisto(std::array<std::array<TH1D*, 3>, numdedx::nrings>& htemp)
{
  const int tzoom = 3;
  const int nt[tzoom] = {50, 500, 1000};
  double tzedges[tzoom + 1] = {0, 2.0e3, 0.1e6, 20e6};
  std::string stname[tzoom] = {"early", "mid", "later"};
  std::string stlabel[tzoom] = {"zoom <2ms", "early time <= 100ms", "later time >100ms"};
  for (unsigned int ir = 0; ir < c_rings; ir++) {
    for (int wt = 0; wt < tzoom; wt++) {
      std::string title = Form("inject time (%s), %s (%s)", stlabel[wt].data(), m_sring[ir].data(), m_suffix.data());
      std::string hname = Form("htimezoom_%s_%s_%s", m_sring[ir].data(), stname[wt].data(), m_suffix.data());
      htemp[ir][wt] = new TH1D(Form("%s", hname.data()), "", nt[wt], tzedges[wt], tzedges[wt + 1]);
      htemp[ir][wt]->SetTitle(Form("%s;injection time(#mu-sec);entries", title.data()));
    }
  }
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::checkStatistics(std::array<std::vector<TH1D*>, numdedx::nrings>& hvar)
{
  for (unsigned int ir = 0; ir < c_rings; ir++) {
    for (unsigned int it = 3; it < m_tbins; it++) {
      //check statiscs from 1-40ms
      if (m_tedges[it] < 4e4 && hvar[ir][it]->Integral() < m_thersE) {
        m_isminStat = true;
        break;
      } else continue;
    }
  }
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::correctBinBias(std::map<int, std::vector<double>>& varcorr,
                                                std::map<int, std::vector<double>>& var,
                                                std::map<int, std::vector<double>>& time, TH1D*& htimes)
{
  //Deep copy OK
  varcorr = var;

  for (int ir = 0; ir < 2; ir++) {

    for (int ix = varcorr[ir * 2].size(); ix -- > 0;) {

      double var_thisbin = 1.0;
      var_thisbin = var[ir * 2].at(ix);

      double atime_thisbin = time[ir * 2].at(ix);
      double ctime_thisbin = htimes->GetBinCenter(ix + 1);

      if (atime_thisbin > 0 && atime_thisbin < 4e4 * 0.99) {

        double var_nextbin = 1.0;
        var_nextbin = var[ir * 2].at(ix + 1);
        double var_diff = var_nextbin - var_thisbin;

        double atime_nextbin = time[ir * 2].at(ix + 1);
        double atime_diff = atime_nextbin - atime_thisbin;

        double slope = (atime_diff > 0) ? var_diff / atime_diff : -1.0;

        //extrapolation after veto only
        if (var_diff > 0 && slope > 0)varcorr[ir * 2].at(ix) = var_thisbin + (ctime_thisbin - atime_thisbin) * (slope);
        printf("\t %s ix = %d, center = %0.2f(%0.3f), var = %0.5f(%0.5f) \n", m_sring[ir].data(), ix, ctime_thisbin, atime_thisbin,
               var_thisbin, varcorr[ir * 2].at(ix));
      } else {
        printf("\t %s --> ix = %d, center = %0.2f(%0.3f), var = %0.5f(%0.5f) \n", m_sring[ir].data(), ix, ctime_thisbin, atime_thisbin,
               var_thisbin, varcorr[ir * 2].at(ix));
      }
    }
  }
}

//-------------------------------------
void CDCDedxInjectTimeAlgorithm::createPayload(std::array<double, numdedx::nrings>& scale,
                                               std::map<int, std::vector<double>>& var,
                                               std::map<int, std::vector<double>>& varscal, std::string svar)
{
  varscal = var;

  B2INFO("CDCDedxInjectTimeAlgorithm: normalising constants with plateau");
  for (unsigned int ir = 0; ir < c_rings; ir++) {
    //scaling means with time >40ms
    unsigned int msize = varscal[ir * 2].size();
    int countsum = 0;
    scale[ir] = 0;
    for (unsigned int im = 0; im < msize; im++) {
      double time = m_vtlocaledges.at(im);
      double mean = varscal[ir * 2].at(im);
      if (time > 4e4 && mean > 0) {
        scale[ir]  += mean;
        countsum++;
      }
    }
    if (countsum > 0 && scale[ir] > 0) {
      scale[ir] /= countsum;
      for (unsigned int im = 0; im < msize; im++) {
        varscal[ir * 2].at(im) /= scale[ir];
      }
    }
  }
  if (m_isMerge && svar == "mean") {
    //merge only no change in payload structure
    bool incomp_bin = false;
    std::vector<std::vector<double>> oldvectors;
    if (m_DBInjectTime) oldvectors = m_DBInjectTime->getConstVector();
    int vsize = oldvectors.size();
    if (vsize != 6) incomp_bin = true;
    else {
      for (int iv = 0; iv < 2; iv++) {
        if (oldvectors[iv * 3 + 1].size() != varscal[iv * 2].size()) incomp_bin = true;
      }
    }
    if (!incomp_bin) {
      B2INFO("CDCDedxInjectTimeAlgorithm: started merging relative constants");
      for (int ir = 0; ir < 2; ir++) {//merging only means
        unsigned int msize = varscal[ir * 2].size();
        for (unsigned int im = 0; im < msize; im++) {
          double relvalue = varscal[ir * 2].at(im);
          double oldvalue = oldvectors[ir * 3 + 1].at(im);
          double merged = oldvalue * relvalue;
          printf("%s: rel %0.03f, old %0.03f, merged %0.03f\n", m_suffix.data(), relvalue, oldvalue, merged);
          varscal[ir * 2].at(im)  *= oldvectors[ir * 3 + 1].at(im) ;
        }
      }
    } else B2ERROR("CDCDedxInjectTimeAlgorithm: found incompatible bins for merging");
  } else B2INFO("CDCDedxInjectTimeAlgorithm: saving final (abs) calibration");
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::plotBinLevelDist(std::array<std::vector<TH1D*>, numdedx::nrings>& hvar, std::string var)
{
  TCanvas cfit("cfit", "cfit", 1000, 500);
  cfit.Divide(2, 1);
  std::stringstream psname_fit;
  psname_fit << Form("%s_%s_%s.pdf[", m_prefix.data(), var.data(), m_suffix.data());
  cfit.Print(psname_fit.str().c_str());
  psname_fit.str("");
  psname_fit << Form("%s_%s_%s.pdf", m_prefix.data(), var.data(), m_suffix.data());
  for (unsigned int it = 0; it < m_tbins; it++) {
    for (unsigned int ir = 0; ir < c_rings; ir++) {
      cfit.cd(ir + 1);
      hvar[ir][it]->SetFillColorAlpha(ir + 5, 0.25);
      hvar[ir][it]->Draw();
    }
    cfit.Print(psname_fit.str().c_str());
  }
  psname_fit.str("");
  psname_fit << Form("%s_%s_%s.pdf]", m_prefix.data(), var.data(), m_suffix.data());
  cfit.Print(psname_fit.str().c_str());
}

//----------------------------------------
void CDCDedxInjectTimeAlgorithm::plotEventStats()
{
  // draw event and track statistics
  TCanvas cestat("cestat", "cestat", 1000, 500);
  cestat.SetBatch(kTRUE);
  cestat.Divide(2, 1);

  cestat.cd(1);
  auto hestats = getObjectPtr<TH1I>("hestats");
  if (hestats) {
    hestats->SetName(Form("hestats_%s", m_suffix.data()));
    hestats->SetStats(0);
    hestats->Draw("hist text");
  }
  cestat.cd(2);
  auto htstats = getObjectPtr<TH1I>("htstats");
  if (htstats) {
    htstats->SetName(Form("htstats_%s", m_suffix.data()));
    htstats->SetStats(0);
    htstats->Draw("hist text");
  }
  cestat.Print(Form("%s_eventstat_%s.pdf", m_prefix.data(), m_suffix.data()));
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::plotInjectionTime(std::array<std::array<TH1D*, 3>, numdedx::nrings>& hvar)
{
  TCanvas ctzoom("ctzoom", "ctzoom", 1500, 450);
  ctzoom.SetBatch(kTRUE);
  ctzoom.Divide(3, 1);
  for (int wt = 0; wt < 3; wt++) {
    ctzoom.cd(wt + 1);
    if (wt == 2) gPad->SetLogy();
    for (unsigned int ir = 0; ir < c_rings; ir++) {
      hvar[ir][wt]->SetStats(0);
      hvar[ir][wt]->SetFillColorAlpha(5 + ir, 0.20);
      if (ir == 0) {
        double max1 = hvar[ir][wt]->GetMaximum();
        double max2 = hvar[c_rings - 1][wt]->GetMaximum();
        if (max2 > max1) hvar[ir][wt]->SetMaximum(max2 * 1.05);
        hvar[ir][wt]->Draw("");
      } else hvar[ir][wt]->Draw("same");
    }
  }
  ctzoom.Print(Form("%s_timezoom_%s.pdf]", m_prefix.data(), m_suffix.data()));
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::plotRelConstants(std::map<int, std::vector<double>>& vmeans,
                                                  std::map<int, std::vector<double>>& vresos, std::map<int, std::vector<double>>& corr, std::string svar)
{
  std::string sname[3] = {"mean", "reso"};
  const int lcolors[c_rings] = {2, 4};

  TH1D* hmean[c_rings], *hcorr[c_rings];
  TH1D* hreso[c_rings];

  TCanvas* cconst[2];
  for (int ic = 0; ic < 2; ic++) {
    cconst[ic] = new TCanvas(Form("c%sconst", sname[ic].data()), "", 900, 500);
    cconst[ic]->SetGridy(1);
  }

  TLegend* mleg = new TLegend(0.50, 0.54, 0.80, 0.75, NULL, "brNDC");
  mleg->SetBorderSize(0);
  mleg->SetFillStyle(0);

  for (unsigned int ir = 0; ir < c_rings; ir++) {

    std::string mtitle = Form("#mu(dedx), relative const. compare, (%s)", m_suffix.data());
    hmean[ir] = new TH1D(Form("hmean_%s_%s", m_suffix.data(), m_sring[ir].data()), "", m_tbins, 0, m_tbins);

    std::string rtitle = Form("#sigma(#chi), relative const. compare, (%s)", m_suffix.data());
    hreso[ir] = new TH1D(Form("hreso_%s_%s", m_suffix.data(), m_sring[ir].data()), "", m_tbins, 0, m_tbins);

    hcorr[ir] = new TH1D(Form("hcorr_%s_%s", m_suffix.data(), m_sring[ir].data()), "", m_tbins, 0, m_tbins);
    if (svar == "chi") {
      hmean[ir]->SetTitle(Form("%s;injection time(#mu-second);#mu (#chi-fit)", rtitle.data()));
      hreso[ir]->SetTitle(Form("%s;injection time(#mu-second);#sigma (#chi-fit)", rtitle.data()));
      hcorr[ir]->SetTitle(Form("%s;injection time(#mu-second);#sigma (#chi-fit bin-bais-corr)", rtitle.data()));

    } else {
      hmean[ir]->SetTitle(Form("%s;injection time(#mu-second);#mu (dedx-fit)", mtitle.data()));
      hreso[ir]->SetTitle(Form("%s;injection time(#mu-second);#sigma (dedx-fit)", mtitle.data()));
      hcorr[ir]->SetTitle(Form("%s;injection time(#mu-second);#mu (dedx-fit, bin-bais-corr)", mtitle.data()));
    }


    for (unsigned int it = 0; it < m_tbins; it++) {

      std::string label = getTimeBinLabel(m_tedges[it], it);

      hmean[ir]->SetBinContent(it + 1, vmeans[ir * 2].at(it));
      hmean[ir]->SetBinError(it + 1, vmeans[ir * 2 + 1].at(it));
      hmean[ir]->GetXaxis()->SetBinLabel(it + 1, label.data());

      hreso[ir]->SetBinContent(it + 1, vresos[ir * 2].at(it));
      hreso[ir]->SetBinError(it + 1, vresos[ir * 2 + 1].at(it));
      hreso[ir]->GetXaxis()->SetBinLabel(it + 1, label.data());

      hcorr[ir]->SetBinContent(it + 1, corr[ir * 2].at(it));
      hcorr[ir]->SetBinError(it + 1, corr[ir * 2 + 1].at(it));
      hcorr[ir]->GetXaxis()->SetBinLabel(it + 1, label.data());
    }

    mleg->AddEntry(hmean[ir], Form("%s", m_sring[ir].data()), "lep");
    cconst[0]->cd();
    if (svar == "chi") setHistStyle(hmean[ir], lcolors[ir], ir + 24, -0.60, 0.60);
    else if (svar == "dedx")  setHistStyle(hmean[ir], lcolors[ir], ir + 24, 0.60, 1.10);
    else setHistStyle(hmean[ir], lcolors[ir], ir + 24, 0.9, 1.10);

    if (ir == 0)hmean[ir]->Draw("");
    else hmean[ir]->Draw("same");
    if (svar == "dedx") {
      mleg->AddEntry(hcorr[ir], Form("%s (bin-bias-corr)", m_sring[ir].data()), "lep");
      setHistStyle(hcorr[ir], lcolors[ir], ir + 20, 0.60, 1.10);
      hcorr[ir]->Draw("same");
    }
    if (ir == 1)mleg->Draw("same");

    cconst[1]->cd();
    if (svar == "chi") setHistStyle(hreso[ir], lcolors[ir], ir + 24, 0.5, 1.50);
    else setHistStyle(hreso[ir], lcolors[ir], ir + 24, 0.0, 0.15);
    if (ir == 0)hreso[ir]->Draw("");
    else hreso[ir]->Draw("same");
    if (svar == "chi") {
      mleg->AddEntry(hcorr[ir], Form("%s (bin-bias-corr)", m_sring[ir].data()), "lep");
      setHistStyle(hcorr[ir], lcolors[ir], ir + 20, 0.5, 1.50);
      hcorr[ir]->Draw("same");
    }
    if (ir == 1)mleg->Draw("same");
  }

  for (int ic = 0; ic < 2; ic++) {
    cconst[ic]->SaveAs(Form("%s_relconst_%s_%s_%s.pdf", m_prefix.data(), sname[ic].data(), svar.data(), m_suffix.data()));
    cconst[ic]->SaveAs(Form("%s_relconst_%s_%s_%s.root", m_prefix.data(), sname[ic].data(), svar.data(), m_suffix.data()));
    delete cconst[ic];
  }
  for (int ic = 0; ic < 2; ic++) {

    delete hmean[ic];
    delete hreso[ic];
    delete hcorr[ic];
  }
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::plotTimeStat(std::array<std::vector<TH1D*>, numdedx::nrings>& htime)
{
  const int lcolors[c_rings] = {2, 4};

  TH1D* htimestat[c_rings];

  TCanvas* cconst = new TCanvas("ctimestatconst", "", 900, 500);
  cconst->SetGridy(1);

  TLegend* rleg = new TLegend(0.40, 0.60, 0.80, 0.72, NULL, "brNDC");
  rleg->SetBorderSize(0);
  rleg->SetFillStyle(0);

  for (unsigned int ir = 0; ir < c_rings; ir++) {

    std::string title = Form("injection time, her-ler comparision, (%s)", m_suffix.data());
    htimestat[ir] = new TH1D(Form("htimestat_%s_%s", m_suffix.data(), m_sring[ir].data()), "", m_tbins, 0, m_tbins);
    htimestat[ir]->SetTitle(Form("%s;injection time(#mu-second);norm. entries", title.data()));

    for (unsigned int it = 0; it < m_tbins; it++) {
      std::string label = getTimeBinLabel(m_tedges[it], it);
      htimestat[ir]->SetBinContent(it + 1, htime[ir][it]->Integral());
      htimestat[ir]->SetBinError(it + 1, 0);
      htimestat[ir]->GetXaxis()->SetBinLabel(it + 1, label.data());
    }

    cconst->cd();
    double norm = htimestat[ir]->GetMaximum();
    rleg->AddEntry(htimestat[ir], Form("%s (scaled with %0.02f)", m_sring[ir].data(), norm), "lep");
    htimestat[ir]->Scale(1.0 / norm);
    setHistStyle(htimestat[ir], lcolors[ir], ir + 20, 0.0, 1.10);
    htimestat[ir]->SetFillColorAlpha(lcolors[ir], 0.30);
    if (ir == 0) htimestat[ir]->Draw("hist");
    else htimestat[ir]->Draw("hist same");
    if (ir == 1)rleg->Draw("same");
  }

  cconst->SaveAs(Form("%s_relconst_timestat_%s.pdf", m_prefix.data(), m_suffix.data()));
  cconst->SaveAs(Form("%s_relconst_timestat_%s.root", m_prefix.data(), m_suffix.data()));
  delete cconst;
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::plotFinalConstants(std::map<int, std::vector<double>>& vmeans,
                                                    std::map<int, std::vector<double>>& vresos,
                                                    std::array<double, numdedx::nrings>& scale, std::array<double, numdedx::nrings>& scale_reso)
{

  std::vector<std::vector<double>> oldvectors;
  if (m_DBInjectTime)oldvectors = m_DBInjectTime->getConstVector();

  const int c_type = 2; //old and new
  std::string sname[c_rings] = {"mean", "reso"};
  std::string stype[c_type] = {"new", "old"};
  const int lcolors[c_rings] = {2, 4};
  const int lmarker[c_type] = {20, 24}; //+2 for different rings

  TH1D* hmean[c_rings][c_type], *hreso[c_rings][c_type];

  TCanvas* cconst[c_rings];
  for (int ic = 0; ic < 2; ic++) {
    cconst[ic] = new TCanvas(Form("c%sconst", sname[ic].data()), "", 900, 500);
    cconst[ic]->SetGridy(1);
  }

  TLegend* mleg = new TLegend(0.50, 0.54, 0.80, 0.75, NULL, "brNDC");
  mleg->SetBorderSize(0);
  mleg->SetFillStyle(0);

  TLegend* rleg = new TLegend(0.50, 0.54, 0.80, 0.75, NULL, "brNDC");
  rleg->SetBorderSize(0);
  rleg->SetFillStyle(0);

  for (unsigned int ip = 0; ip < c_type; ip++) {

    for (unsigned int ir = 0; ir < c_rings; ir++) {

      std::string hname = Form("hfmean_%s_%s_%s", m_sring[ir].data(), stype[ip].data(), m_suffix.data());
      hmean[ir][ip] = new TH1D(hname.data(), "", m_tbins, 0, m_tbins);
      std::string title = Form("#mu(dedx), final-mean-compare (%s)", m_suffix.data());
      hmean[ir][ip]->SetTitle(Form("%s;injection time(#mu-second);#mu (dedx-fit)", title.data()));

      hname = Form("hfreso_%s_%s_%s", m_sring[ir].data(), stype[ip].data(), m_suffix.data());
      hreso[ir][ip] = new TH1D(hname.data(), "", m_tbins, 0, m_tbins);
      title = Form("#sigma(#chi), final-reso-compare (%s)", m_suffix.data());
      hreso[ir][ip]->SetTitle(Form("%s;injection time(#mu-second);#sigma (#chi-fit)", title.data()));

      for (unsigned int it = 0; it < m_tbins; it++) {

        std::string label = getTimeBinLabel(m_tedges[it], it);
        double mean = 0.0, reso = 0.0;
        if (ip == 0) {
          mean = m_vinjPayload[ir * 3 + 1].at(it);
          //reso is reso/mu (reso is relative so mean needs to be relative)
          reso = m_vinjPayload[ir * 3 + 2].at(it);

        } else {
          mean = oldvectors[ir * 3 + 1].at(it);
          reso = oldvectors[ir * 3 + 2].at(it);

        }

        //old payloads
        hmean[ir][ip]->SetBinContent(it + 1, mean);
        hmean[ir][ip]->SetBinError(it + 1, vmeans[ir * 2 + 1].at(it));
        hmean[ir][ip]->GetXaxis()->SetBinLabel(it + 1, label.data());

        hreso[ir][ip]->SetBinContent(it + 1, reso);
        hreso[ir][ip]->SetBinError(it + 1, vresos[ir * 2 + 1].at(it));
        hreso[ir][ip]->GetXaxis()->SetBinLabel(it + 1, label.data());
      }

      cconst[0]->cd();
      if (ip == 1)mleg->AddEntry(hmean[ir][ip], Form("%s, %s", m_sring[ir].data(), stype[ip].data()), "lep");
      else mleg->AddEntry(hmean[ir][ip], Form("%s, %s (scaled by %0.03f)", m_sring[ir].data(), stype[ip].data(), scale[ir]), "lep");
      setHistStyle(hmean[ir][ip], lcolors[ir], lmarker[ip] + ir * 2, 0.60, 1.05);
      if (ir == 0 && ip == 0)hmean[ir][ip]->Draw("");
      else hmean[ir][ip]->Draw("same");
      if (ir == 1 && ip == 1)mleg->Draw("same");

      cconst[1]->cd();
      if (ip == 1)rleg->AddEntry(hreso[ir][ip], Form("%s, %s", m_sring[ir].data(), stype[ip].data()), "lep");
      else rleg->AddEntry(hreso[ir][ip], Form("%s, %s (scaled by %0.03f)", m_sring[ir].data(), stype[ip].data(), scale_reso[ir]), "lep");
      setHistStyle(hreso[ir][ip], lcolors[ir], lmarker[ip] + ir * 2, 0.6, 1.9);
      if (ir == 0  && ip == 0)hreso[ir][ip]->Draw("");
      else hreso[ir][ip]->Draw("same");
      if (ir == 1 && ip == 1)rleg->Draw("same");
    }
  }

  for (int ic = 0; ic < 2; ic++) {
    cconst[ic]->SaveAs(Form("%s_finalconst_%s_%s.pdf", m_prefix.data(), sname[ic].data(), m_suffix.data()));
    cconst[ic]->SaveAs(Form("%s_finalconst_%s_%s.root", m_prefix.data(), sname[ic].data(), m_suffix.data()));
    delete cconst[ic];
  }

  for (unsigned int ir = 0; ir < c_rings; ir++) {
    for (unsigned int ip = 0; ip < c_type; ip++) {
      delete hmean[ir][ip];
      delete hreso[ir][ip];
    }
  }
}

//------------------------------------
double CDCDedxInjectTimeAlgorithm::getCorrection(unsigned int ring, unsigned int time,
                                                 std::map<int, std::vector<double>>&  vmeans)
{

  unsigned int iv = ring * 2;

  unsigned int sizet = m_vtlocaledges.size(); //time

  std::vector<unsigned int> tedges(sizet); //time edges array
  std::copy(m_vtlocaledges.begin(), m_vtlocaledges.end(), tedges.begin());

  if (time >= 5e6) time = 5e6 - 10;
  unsigned int it = m_DBInjectTime->getTimeBin(tedges, time);

  double center = 0.5 * (m_vtlocaledges.at(it) + m_vtlocaledges.at(it + 1));

  //no corr before veto bin (usually one or two starting bin)
  //intrapolation for entire range except
  //--extrapolation (for first half and last half of intended bin)
  int thisbin = it, nextbin = it;
  if (center != time && it > 0) {

    if (time < center) {
      thisbin = it - 1;
    } else {
      if (it < sizet - 2)nextbin = it + 1;
      else thisbin = it - 1;
    }

    if (it <= 2) {
      double diff = vmeans[iv].at(2) - vmeans[iv].at(1) ;
      if (diff < -0.015) { //difference above 1.0%
        thisbin = it;
        if (it == 1) nextbin = it;
        else nextbin = it + 1;
      } else {
        if (it == 1) {
          thisbin = it;
          nextbin = it + 1;
        }
      }
    }
  }

  double thisdedx = vmeans[iv].at(thisbin);
  double nextdedx = vmeans[iv].at(nextbin);

  double thistime = 0.5 * (m_vtlocaledges.at(thisbin) + m_vtlocaledges.at(thisbin + 1));
  double nexttime = 0.5 * (m_vtlocaledges.at(nextbin) + m_vtlocaledges.at(nextbin + 1));

  double newdedx = vmeans[iv].at(it);
  if (thisbin != nextbin)
    newdedx = thisdedx + ((nextdedx - thisdedx) / (nexttime - thistime)) * (time - thistime);

  return newdedx;
}