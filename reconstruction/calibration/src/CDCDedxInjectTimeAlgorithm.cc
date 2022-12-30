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
  m_dedxBins(500),
  m_dedxMin(0.0),
  m_dedxMax(2.5),
  isminStat(false),
  ismakePlots(true),
  isMerge(false),
  m_countR(0),
  m_thersE(1000),
  m_prefix("cdcdedx_injcal"),
  m_suffix("")
{
  // Set module properties
  setDescription("A calibration algorithm for CDC dE/dx run gains");
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
  if (!ttree)return c_NotEnoughData;

  double dedx = 0.0, injtime = 0.0, injring = 1.0;
  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("injtime", &injtime);
  ttree->SetBranchAddress("injring", &injring);

  //way to define time bins/edges only once
  if (m_countR == 0) {
    defineTimeBins(); //returns vtlocaledges
    m_tbins = vtlocaledges.size() - 1;
    m_tedges = &vtlocaledges[0];
    m_countR++;
  }

  TH1D* htimes = new TH1D(Form("htimes_%s", m_suffix.data()), "", m_tbins, m_tedges);

  //time bins are changable from out so vector is used
  array<vector<TH1D*>, numdedx::nrings> hdedx, htime;
  defineHisto(hdedx, "dedx");
  defineHisto(htime, "timeinj");

  const double tzedges[4] = {0, 2.0e3, 0.1e6, 20e6};
  array<array<TH1D*, 3>, numdedx::nrings> hztime;
  defineTimeHisto(hztime);

  //fill histograms
  for (int i = 0; i < ttree->GetEntries(); ++i) {

    ttree->GetEvent(i);
    if (dedx <= 0 || injtime < 0 || injring < 0) continue;

    //add larger times to the last bin
    if (injtime > m_tedges[m_tbins])injtime = m_tedges[m_tbins] - 10.0;

    //injection ring
    int wr = 0;
    if (injring > 0.5)wr = 1;

    //injection time bin
    unsigned int tb = htimes->GetXaxis()->FindBin(injtime);
    if (tb > m_tbins)tb = m_tbins; //overflow
    tb = tb - 1;

    htimes->Fill(injtime);
    if (injtime < tzedges[1])hztime[wr][0]->Fill(injtime);
    else if (injtime < tzedges[2])hztime[wr][1]->Fill(injtime);
    else hztime[wr][2]->Fill(injtime);

    hdedx[wr][tb]->Fill(dedx);
    htime[wr][tb]->Fill(injtime);
  }

  //keep merging runs to achive enough stats
  isminStat = false;
  checkStatistics(hdedx);
  if (isminStat) {
    deleteHisto(htime);
    deleteHisto(hdedx);
    deleteTimeHisto(hztime);
    delete htimes;
    return c_NotEnoughData;
  }

  //clear vector of existing constants
  //map<int, vector<double>> m_vmeans, m_vresos, m_vtimes;
  for (unsigned int ir = 0; ir < c_rings; ir++) {
    m_vmeans[ir].clear();
    m_vresos[ir].clear();
    m_vtimes[ir].clear();
    m_vmeanserr[ir].clear();
    m_vresoserr[ir].clear();
    m_vtimeserr[ir].clear();
    for (unsigned int it = 0; it < m_tbins; it++) {
      double mean = 1.00, meanerr = 0.0;
      double reso = 0.07, resoerr = 0.0;
      double avgtime = htime[ir][it]->GetMean();
      double avgtimeerr = htime[ir][it]->GetMeanError();
      if (hdedx[ir][it]->Integral() > 250) {
        string status = "";
        fitGaussianWRange(hdedx[ir][it], status);
        if (status != "FitOK") {
          mean = hdedx[ir][it]->GetMean();
          hdedx[ir][it]->SetTitle(Form("%s, (%s)", hdedx[ir][it]->GetTitle(), status.data()));
        } else {
          mean = hdedx[ir][it]->GetFunction("gaus")->GetParameter(1);
          meanerr = hdedx[ir][it]->GetFunction("gaus")->GetParError(1);
          reso = hdedx[ir][it]->GetFunction("gaus")->GetParameter(2);
          resoerr = hdedx[ir][it]->GetFunction("gaus")->GetParError(2);
          string title = Form("#mu_{fit}: %0.03f, #sigma_{fit}: %0.03f", mean, reso);
          hdedx[ir][it]->SetTitle(Form("%s, %s", hdedx[ir][it]->GetTitle(), title.data()));
        }
      }
      m_vmeans[ir].push_back(mean);
      m_vresos[ir].push_back(reso);
      m_vtimes[ir].push_back(avgtime);
      m_vmeanserr[ir].push_back(meanerr);
      m_vresoserr[ir].push_back(resoerr);
      m_vtimeserr[ir].push_back(avgtimeerr);
    }
  }

  map<int, vector<double>> m_vmeanscorr;
  correctBinBias(m_vmeanscorr, m_vtimes, htimes, "mean");
  map<int, vector<double>> m_vresoscorr;
  correctBinBias(m_vresoscorr, m_vtimes, htimes, "reso");

  //preparing final payload
  vinjPayload.clear();
  vinjPayload.reserve(6);
  for (int ir = 0; ir < 2; ir++) {
    vinjPayload.push_back(vtlocaledges);
    vinjPayload.push_back(m_vmeanscorr[ir]);
    vinjPayload.push_back(m_vresoscorr[ir]);
  }

  //used
  array<double, numdedx::nrings> m_scale;
  createPayload(m_scale);

  if (ismakePlots) {

    //0 plot event track statistics
    plotEventStats();

    //1 plot injection time
    plotInjectionTime(hztime);

    //2. Draw dedxfits
    plotBinLevelDist(hdedx, "dedxfits");

    //3. Draw timedist
    plotBinLevelDist(htime, "timedist");

    //4 plot relative const., bias-bias corrected, avg time
    plotRelConstants(m_vmeanscorr, m_vresoscorr, htime);

    //5 plot final merged const. and comparison to old
    plotFinalConstants(m_scale);

  }

  //delete all histograms
  deleteHisto(htime);
  deleteHisto(hdedx);
  deleteTimeHisto(hztime);
  delete htimes;

  B2INFO("Saving calibration for: " << m_suffix << "");
  return c_OK;
}

//----------------------------------------
void CDCDedxInjectTimeAlgorithm::fitGaussianWRange(TH1D*& temphist, string& status)
{
  double histmean = temphist->GetMean();
  double histrms = temphist->GetRMS();
  temphist->GetXaxis()->SetRangeUser(histmean - 5.0 * histrms, histmean + 5.0 * histrms);

  int fs = temphist->Fit("gaus", "Q0");
  if (fs != 0) {
    B2INFO(Form("\tFit (round 1) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
    status = "FitFailed";
    return;
  } else {
    double mean = temphist->GetFunction("gaus")->GetParameter(1);
    double width = temphist->GetFunction("gaus")->GetParameter(2);
    temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
    fs = temphist->Fit("gaus", "QR", "", mean - m_sigmaR * width, mean + m_sigmaR * width);
    if (fs != 0) {
      B2INFO(Form("\tFit (round 2) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
      status = "FitFailed";
      return;
    } else {
      temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
      B2INFO(Form("\tFit for hist (%s) sucessfull (status = %d)", temphist->GetName(), fs));
      status = "FitOK";
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

  if (isminStat) {
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
  if (!vtlocaledges.empty())vtlocaledges.clear();
  if (vtedges.empty()) {
    double fixedges[69];
    // vtlocaledges.reserve(69);
    for (int ib = 0; ib < 69; ib++) {
      fixedges[ib] = ib * 0.5 * 1e3;
      if (ib > 40 && ib <= 60)fixedges[ib] = fixedges[ib - 1] + 1.0 * 1e3;
      else if (ib > 60 && ib <= 64)fixedges[ib] = fixedges[ib - 1] + 10.0 * 1e3;
      else if (ib > 64 && ib <= 65)fixedges[ib] = fixedges[ib - 1] + 420.0 * 1e3;
      else if (ib > 65 && ib <= 66)fixedges[ib] = fixedges[ib - 1] + 500.0 * 1e3;
      else if (ib > 66)fixedges[ib] = fixedges[ib - 1] + 2e6;
      //cout << ib << ", value " << fixedges[ib] << endl;
      vtlocaledges.push_back(fixedges[ib]);
    }
  } else {
    // vtlocaledges.reserve(vtedges.size());
    for (unsigned int ib = 0; ib < vtedges.size(); ib++)
      vtlocaledges.push_back(vtedges.at(ib));
  }
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::defineHisto(array<vector<TH1D*>, numdedx::nrings>& htemp, string var)
{
  for (unsigned int ir = 0; ir < c_rings; ir++) {
    htemp[ir].resize(m_tbins);
    for (unsigned int it = 0; it < m_tbins; it++) {
      string label = getTimeBinLabel(m_tedges[it], it);
      string title = Form("%s(%s), time(%s)", m_suffix.data(), m_sring[ir].data(), label.data());
      string hname = Form("h%s_%s_%s_t%d", var.data(), m_sring[ir].data(), m_suffix.data(), it);
      if (var == "dedx")htemp[ir][it] = new TH1D(hname.data(), "", 250, 0.0, 2.5);
      else htemp[ir][it] = new TH1D(hname.data(), "", 50, m_tedges[it], m_tedges[it + 1]);
      htemp[ir][it]->SetTitle(Form("%s;%s;entries", title.data(), var.data()));
    }
  }
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::defineTimeHisto(array<array<TH1D*, 3>, numdedx::nrings>& htemp)
{
  const int tzoom = 3;
  const int nt[tzoom] = {50, 500, 1000};
  double tzedges[tzoom + 1] = {0, 2.0e3, 0.1e6, 20e6};
  string stname[tzoom] = {"early", "mid", "later"};
  string stlabel[tzoom] = {"zoom <2ms", "early time <= 100ms", "later time >100ms"};
  for (unsigned int ir = 0; ir < c_rings; ir++) {
    for (int wt = 0; wt < tzoom; wt++) {
      string title = Form("inject time (%s), %s (%s)", stlabel[wt].data(), m_sring[ir].data(), m_suffix.data());
      string hname = Form("htimezoom_%s_%s_%s", m_sring[ir].data(), stname[wt].data(), m_suffix.data());
      htemp[ir][wt] = new TH1D(Form("%s", hname.data()), "", nt[wt], tzedges[wt], tzedges[wt + 1]);
      htemp[ir][wt]->SetTitle(Form("%s;injection time(#mu-sec);entries", title.data()));
    }
  }
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::checkStatistics(array<vector<TH1D*>, numdedx::nrings>& hvar)
{
  for (unsigned int ir = 0; ir < c_rings; ir++) {
    for (unsigned int it = 3; it < m_tbins; it++) {
      //check statiscs from 1-40ms
      if (m_tedges[it] < 4e4 && hvar[ir][it]->Integral() < m_thersE) {
        isminStat = true;
        break;
      } else continue;
    }
  }
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::correctBinBias(map<int, vector<double>>& m_varcorr, map<int, vector<double>>& m_time,
                                                TH1D*& htimes, string svar)
{
  //Deep copy OK
  if (svar == "mean")m_varcorr = m_vmeans;
  else if (svar == "reso")m_varcorr = m_vresos;

  for (int ir = 0; ir < 2; ir++) {

    for (int ix = m_varcorr[ir].size(); ix -- > 0;) {

      double var_thisbin = 1.0;
      if (svar == "mean")var_thisbin = m_vmeans[ir].at(ix);
      else if (svar == "reso")var_thisbin = m_vresos[ir].at(ix);

      double atime_thisbin = m_time[ir].at(ix);
      double ctime_thisbin = htimes->GetBinCenter(ix + 1);

      if (atime_thisbin > 0 && atime_thisbin < 4e4 * 0.99) {

        double var_nextbin = 1.0;
        if (svar == "mean")var_nextbin = m_vmeans[ir].at(ix + 1);
        else if (svar == "reso")var_nextbin = m_vresos[ir].at(ix + 1);
        double var_diff = var_nextbin - var_thisbin;

        double atime_nextbin = m_time[ir].at(ix + 1);
        double atime_diff = atime_nextbin - atime_thisbin;

        double slope = (atime_diff > 0) ? var_diff / atime_diff : -1.0;

        //extrapolation after veto only
        if (var_diff > 0 && slope > 0)m_varcorr[ir].at(ix) = var_thisbin + (ctime_thisbin - atime_thisbin) * (slope);
        printf("\t %s ix = %d, center = %0.2f(%0.3f), var = %0.5f(%0.5f) \n", m_sring[ir].data(), ix, ctime_thisbin, atime_thisbin,
               var_thisbin, m_varcorr[ir].at(ix));
      } else {
        printf("\t %s --> ix = %d, center = %0.2f(%0.3f), var = %0.5f(%0.5f) \n", m_sring[ir].data(), ix, ctime_thisbin, atime_thisbin,
               var_thisbin, m_varcorr[ir].at(ix));
      }
    }
  }
}

//-------------------------------------
void CDCDedxInjectTimeAlgorithm::createPayload(array<double, numdedx::nrings>& m_scale)
{
  if (isMerge) {
    //merge only no change in payload structure
    bool incomp_bin = false;
    vector<vector<double>> oldvectors;
    if (m_DBInjectTime)oldvectors = m_DBInjectTime->getConstVector();
    int vsize = oldvectors.size();
    if (vsize != 6) incomp_bin = true;
    else {
      for (int iv = 0; iv < vsize; iv++) {
        if (oldvectors[iv].size() != vinjPayload[iv].size())incomp_bin = true;
      }
    }
    if (!incomp_bin) {
      B2INFO("CDCDedxInjectTimeAlgorithm: started merging relative constants");
      for (int ir = 0; ir < 2; ir++) {//merging only means
        unsigned int msize = vinjPayload[ir * 3 + 1].size();
        for (unsigned int im = 0; im < msize; im++) {
          double relvalue = vinjPayload[ir * 3 + 1].at(im);
          double oldvalue = oldvectors[ir * 3 + 1].at(im);
          double merged = oldvalue * relvalue;
          printf("%s: rel %0.03f, old %0.03f, merged %0.03f\n", m_suffix.data(), relvalue, oldvalue, merged);
          vinjPayload[ir * 3 + 1].at(im) *= oldvectors[ir * 3 + 1].at(im) ;
        }
      }
    } else B2ERROR("CDCDedxInjectTimeAlgorithm: found incompatible bins for merging");
  } else B2INFO("CDCDedxInjectTimeAlgorithm: saving final (abs) calibration");

  B2INFO("CDCDedxInjectTimeAlgorithm: normalising constants with plateau");
  for (unsigned int ir = 0; ir < c_rings; ir++) {
    //scaling means with time >40ms
    unsigned int msize = vinjPayload[ir * 3 + 1].size();
    int countsum = 0;
    m_scale[ir] = 0;
    for (unsigned int im = 0; im < msize; im++) {
      double time = vinjPayload[ir * 3].at(im);
      double mean = vinjPayload[ir * 3 + 1].at(im);
      if (time > 4e4 && mean > 0) {
        m_scale[ir]  += mean;
        countsum++;
      }
    }
    if (countsum > 0 && m_scale[ir] > 0) {
      m_scale[ir] /= countsum;
      for (unsigned int im = 0; im < msize; im++) {
        vinjPayload[ir * 3 + 1].at(im) /= m_scale[ir];
      }
    }
  }
  //saving payloads;
  CDCDedxInjectTime* gains = new CDCDedxInjectTime(vinjPayload);
  saveCalibration(gains, "CDCDedxInjectTime");
  B2INFO("dE/dx Injection time calibration done");
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::plotBinLevelDist(array<vector<TH1D*>, numdedx::nrings>& hvar, string var)
{
  TCanvas* cfit = new TCanvas("cfit", "cfit", 1000, 500);
  cfit->Divide(2, 1);
  stringstream psname_fit;
  psname_fit << Form("%s_%s_%s.pdf[", m_prefix.data(), var.data(), m_suffix.data());
  cfit->Print(psname_fit.str().c_str());
  psname_fit.str("");
  psname_fit << Form("%s_%s_%s.pdf", m_prefix.data(), var.data(), m_suffix.data());
  for (unsigned int it = 0; it < m_tbins; it++) {
    for (unsigned int ir = 0; ir < c_rings; ir++) {
      cfit->cd(ir + 1);
      hvar[ir][it]->SetFillColorAlpha(ir + 5, 0.25);
      hvar[ir][it]->Draw();
    }
    cfit->Print(psname_fit.str().c_str());
  }
  psname_fit.str("");
  psname_fit << Form("%s_%s_%s.pdf]", m_prefix.data(), var.data(), m_suffix.data());
  cfit->Print(psname_fit.str().c_str());
  delete cfit;
}

//----------------------------------------
void CDCDedxInjectTimeAlgorithm::plotEventStats()
{
  // draw event and track statistics
  TCanvas* cestat = new TCanvas("cestat", "cestat", 1000, 500);
  cestat->SetBatch(kTRUE);
  cestat->Divide(2, 1);

  cestat->cd(1);
  auto hestats = getObjectPtr<TH1I>("hestats");
  if (hestats) {
    hestats->SetName(Form("hestats_%s", m_suffix.data()));
    hestats->SetStats(0);
    hestats->Draw("hist text");
  }
  cestat->cd(2);
  auto htstats = getObjectPtr<TH1I>("htstats");
  if (htstats) {
    htstats->SetName(Form("htstats_%s", m_suffix.data()));
    htstats->SetStats(0);
    htstats->Draw("hist text");
  }
  cestat->Print(Form("%s_eventstat_%s.pdf", m_prefix.data(), m_suffix.data()));
  delete cestat;
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::plotInjectionTime(array<array<TH1D*, 3>, numdedx::nrings>& hvar)
{
  TCanvas* ctzoom = new TCanvas("ctzoom", "ctzoom", 1500, 450);
  ctzoom->SetBatch(kTRUE);
  ctzoom->Divide(3, 1);
  for (int wt = 0; wt < 3; wt++) {
    ctzoom->cd(wt + 1);
    if (wt == 2)gPad->SetLogy();
    for (unsigned int ir = 0; ir < c_rings; ir++) {
      hvar[ir][wt]->SetStats(0);
      hvar[ir][wt]->SetFillColorAlpha(5 + ir, 0.20);
      if (ir == 0) {
        double max1 = hvar[ir][wt]->GetMaximum();
        double max2 = hvar[c_rings - 1][wt]->GetMaximum();
        if (max2 > max1)hvar[ir][wt]->SetMaximum(max2 * 1.05);
        hvar[ir][wt]->Draw("");
      } else hvar[ir][wt]->Draw("same");
    }
  }
  ctzoom->Print(Form("%s_timezoom_%s.pdf]", m_prefix.data(), m_suffix.data()));
  delete ctzoom;
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::plotRelConstants(map<int, vector<double>>& m_meancorr, map<int, vector<double>>& m_resocorr,
                                                  array<vector<TH1D*>, numdedx::nrings>& htime)
{
  string sname[3] = {"mean", "reso", "timestat"};
  const int lcolors[c_rings] = {2, 4};

  TH1D* htimestat[c_rings];
  TH1D* hmean[c_rings], *hmeancorr[c_rings];
  TH1D* hreso[c_rings], *hresocorr[c_rings];

  TCanvas* cconst[3];
  for (int ic = 0; ic < 3; ic++) {
    cconst[ic] = new TCanvas(Form("c%sconst", sname[ic].data()), "", 900, 500);
    cconst[ic]->SetGridy(1);
  }

  TLegend* mleg = new TLegend(0.50, 0.54, 0.80, 0.75, NULL, "brNDC");
  mleg->SetBorderSize(0);
  mleg->SetFillStyle(0);

  TLegend* rleg = new TLegend(0.40, 0.60, 0.80, 0.72, NULL, "brNDC");
  rleg->SetBorderSize(0);
  rleg->SetFillStyle(0);

  for (unsigned int ir = 0; ir < c_rings; ir++) {

    string title = Form("#mu(dedx), relative const. compare, (%s)", m_suffix.data());
    hmean[ir] = new TH1D(Form("hmean_%s_%s", m_suffix.data(), m_sring[ir].data()), "", m_tbins, 0, m_tbins);
    hmean[ir]->SetTitle(Form("%s;injection time(#mu-second);#mu (dedx-fit)", title.data()));

    hmeancorr[ir] = new TH1D(Form("hmeancorr_%s_%s", m_suffix.data(), m_sring[ir].data()), "", m_tbins, 0, m_tbins);
    hmeancorr[ir]->SetTitle(Form("%s;injection time(#mu-second);#mu (dedx-fit, bin-bais-corr)", title.data()));

    title = Form("#sigma/#mu(dedx), relative const. compare, (%s)", m_suffix.data());
    hreso[ir] = new TH1D(Form("hreso_%s_%s", m_suffix.data(), m_sring[ir].data()), "", m_tbins, 0, m_tbins);
    hreso[ir]->SetTitle(Form("%s;injection time(#mu-second);#sigma/#mu (dedx-fit)", title.data()));

    hresocorr[ir] = new TH1D(Form("hresocorr_%s_%s", m_suffix.data(), m_sring[ir].data()), "", m_tbins, 0, m_tbins);
    hresocorr[ir]->SetTitle(Form("%s;injection time(#mu-second);#sigma/#mu (dedx-fit bin-bais-corr)", title.data()));

    title = Form("injection time, her-ler comparision, (%s)", m_suffix.data());
    htimestat[ir] = new TH1D(Form("htimestat_%s_%s", m_suffix.data(), m_sring[ir].data()), "", m_tbins, 0, m_tbins);
    htimestat[ir]->SetTitle(Form("%s;injection time(#mu-second);norm. entries", title.data()));

    for (unsigned int it = 0; it < m_tbins; it++) {

      string label = getTimeBinLabel(m_tedges[it], it);

      hmean[ir]->SetBinContent(it + 1, m_vmeans[ir].at(it));
      hmean[ir]->SetBinError(it + 1, m_vmeanserr[ir].at(it));
      hmean[ir]->GetXaxis()->SetBinLabel(it + 1, label.data());

      hreso[ir]->SetBinContent(it + 1, m_vresos[ir].at(it) / m_vmeans[ir].at(it));
      hreso[ir]->SetBinError(it + 1, m_vresoserr[ir].at(it));
      hreso[ir]->GetXaxis()->SetBinLabel(it + 1, label.data());

      hmeancorr[ir]->SetBinContent(it + 1, m_meancorr[ir].at(it));
      hmeancorr[ir]->SetBinError(it + 1, m_vmeanserr[ir].at(it));
      hmeancorr[ir]->GetXaxis()->SetBinLabel(it + 1, label.data());

      hresocorr[ir]->SetBinContent(it + 1, m_resocorr[ir].at(it) / m_meancorr[ir].at(it));
      hresocorr[ir]->SetBinError(it + 1, m_vresoserr[ir].at(it));
      hresocorr[ir]->GetXaxis()->SetBinLabel(it + 1, label.data());

      htimestat[ir]->SetBinContent(it + 1, htime[ir][it]->Integral());
      htimestat[ir]->SetBinError(it + 1, 0);
      htimestat[ir]->GetXaxis()->SetBinLabel(it + 1, label.data());
    }

    cconst[0]->cd();
    mleg->AddEntry(hmean[ir], Form("%s", m_sring[ir].data()), "lep");
    mleg->AddEntry(hmeancorr[ir], Form("%s (bin-bias-corr)", m_sring[ir].data()), "lep");
    setHistStyle(hmean[ir], lcolors[ir], ir + 24, 0.60, 1.10);
    setHistStyle(hmeancorr[ir], lcolors[ir], ir + 20, 0.60, 1.10);
    if (ir == 0)hmean[ir]->Draw("");
    else hmean[ir]->Draw("same");
    hmeancorr[ir]->Draw("same");
    if (ir == 1)mleg->Draw("same");

    cconst[1]->cd();
    setHistStyle(hreso[ir], lcolors[ir], ir + 24, 0.01, 0.20);
    setHistStyle(hresocorr[ir], lcolors[ir], ir + 20, 0.01, 0.20);
    if (ir == 0)hreso[ir]->Draw("");
    else hreso[ir]->Draw("same");
    hresocorr[ir]->Draw("same");
    if (ir == 1)mleg->Draw("same");

    cconst[2]->cd();
    double norm = htimestat[ir]->GetMaximum();
    rleg->AddEntry(htimestat[ir], Form("%s (scaled with %0.02f)", m_sring[ir].data(), norm), "lep");
    htimestat[ir]->Scale(1.0 / norm);
    setHistStyle(htimestat[ir], lcolors[ir], ir + 20, 0.0, 1.10);
    htimestat[ir]->SetFillColorAlpha(lcolors[ir], 0.30);
    if (ir == 0) htimestat[ir]->Draw("hist");
    else htimestat[ir]->Draw("hist same");
    if (ir == 1)rleg->Draw("same");
  }

  for (int ic = 0; ic < 3; ic++) {
    cconst[ic]->SaveAs(Form("%s_relconst_%s_%s.pdf", m_prefix.data(), sname[ic].data(), m_suffix.data()));
    cconst[ic]->SaveAs(Form("%s_relconst_%s_%s.root", m_prefix.data(), sname[ic].data(), m_suffix.data()));
    delete cconst[ic];
  }
}

//------------------------------------
void CDCDedxInjectTimeAlgorithm::plotFinalConstants(array<double, numdedx::nrings>& m_scale)
{
  vector<vector<double>> oldvectors;
  if (m_DBInjectTime)oldvectors = m_DBInjectTime->getConstVector();

  const int c_type = 2; //old and new
  string sname[c_rings] = {"mean", "reso"};
  string stype[c_type] = {"new", "old"};
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

  for (unsigned int ip = 0; ip < c_type; ip++) {

    for (unsigned int ir = 0; ir < c_rings; ir++) {

      string title = Form("#mu(dedx), final-mean-compare (%s)", m_suffix.data());
      string hname = Form("hfmean_%s_%s_%s", m_sring[ir].data(), stype[ip].data(), m_suffix.data());
      hmean[ir][ip] = new TH1D(hname.data(), "", m_tbins, 0, m_tbins);
      hmean[ir][ip]->SetTitle(Form("%s;injection time(#mu-second);#mu (dedx-fit)", title.data()));

      title = Form("#sigma/#mu(dedx), final-reso-compare (%s)", m_suffix.data());
      hname = Form("hfreso_%s_%s_%s", m_sring[ir].data(), stype[ip].data(), m_suffix.data());
      hreso[ir][ip] = new TH1D(hname.data(), "", m_tbins, 0, m_tbins);
      hreso[ir][ip]->SetTitle(Form("%s;injection time(#mu-second);#sigma/#mu (dedx-fit)", title.data()));

      for (unsigned int it = 0; it < m_tbins; it++) {

        string label = getTimeBinLabel(m_tedges[it], it);
        double mean = 0.0, reso = 0.0;
        if (ip == 0) {
          mean = vinjPayload[ir * 3 + 1].at(it);
          //reso is reso/mu (reso is relative so mean needs to be relative)
          reso = m_vresos[ir].at(it) / m_vmeans[ir].at(it);
        } else {
          mean = oldvectors[ir * 3 + 1].at(it);
          reso = oldvectors[ir * 3 + 2].at(it) / oldvectors[ir * 3 + 1].at(it);
        }

        //old payloads
        hmean[ir][ip]->SetBinContent(it + 1, mean);
        hmean[ir][ip]->SetBinError(it + 1, m_vmeanserr[ir].at(it));
        hmean[ir][ip]->GetXaxis()->SetBinLabel(it + 1, label.data());

        hreso[ir][ip]->SetBinContent(it + 1, reso);
        hreso[ir][ip]->SetBinError(it + 1, m_vresoserr[ir].at(it));
        hreso[ir][ip]->GetXaxis()->SetBinLabel(it + 1, label.data());
      }
      cconst[0]->cd();
      if (ip == 1)mleg->AddEntry(hmean[ir][ip], Form("%s, %s", m_sring[ir].data(), stype[ip].data()), "lep");
      else mleg->AddEntry(hmean[ir][ip], Form("%s, %s (scaled by %0.03f)", m_sring[ir].data(), stype[ip].data(), m_scale[ir]), "lep");
      setHistStyle(hmean[ir][ip], lcolors[ir], lmarker[ip] + ir * 2, 0.60, 1.05);
      if (ir == 0 && ip == 0)hmean[ir][ip]->Draw("");
      else hmean[ir][ip]->Draw("same");
      if (ir == 1 && ip == 1)mleg->Draw("same");

      cconst[1]->cd();
      setHistStyle(hreso[ir][ip], lcolors[ir], lmarker[ip] + ir * 2, 0.01, 0.15);
      if (ir == 0  && ip == 0)hreso[ir][ip]->Draw("");
      else hreso[ir][ip]->Draw("same");
      if (ir == 1 && ip == 1)mleg->Draw("same");
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
