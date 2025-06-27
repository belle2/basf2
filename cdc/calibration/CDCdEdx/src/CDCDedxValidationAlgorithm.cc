/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/calibration/CDCdEdx/CDCDedxValidationAlgorithm.h>

using namespace Belle2;
using namespace CDC;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCDedxValidationAlgorithm::CDCDedxValidationAlgorithm() :
  CalibrationAlgorithm("ElectronValCollector"),
  m_sigmaR(2.0),
  m_dedxBins(600),
  m_dedxMin(0.0),
  m_dedxMax(5.0),
  m_cosBins(100),
  m_cosMin(-1.0),
  m_cosMax(1.0),
  m_momBins(80),
  m_momMin(0.0),
  m_momMax(8.0),
  m_eaBin(316),
  m_eaMin(-TMath::Pi() / 2),
  m_eaMax(+TMath::Pi() / 2),
  m_suffix("")
{
  // Set module properties
  setDescription("A validation algorithm for CDC dE/dx electron");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------

CalibrationAlgorithm::EResult CDCDedxValidationAlgorithm::calibrate()
{

  getExpRunInfo();

  std::vector<std::string> subdirs = {"run", "costh", "mom", "wire", "injection", "oneD"};
  for (const auto& dir : subdirs) {
    gSystem->Exec(Form("mkdir -p plots/%s", dir.c_str()));
  }

  // Get data objects
  auto tBhabha = getObjectPtr<TTree>("tBhabha");

  // require at least 100 tracks (arbitrary for now)
  if (tBhabha->GetEntries() < 100) return c_NotEnoughData;

  // Get data objects
  auto tRadee = getObjectPtr<TTree>("tRadee");

  // require at least 100 tracks (arbitrary for now)
  if (tRadee->GetEntries() < 100) return c_NotEnoughData;

  bhabhaValidation();
  radeeValidation();
  plotEventStats();
  return c_OK;
}

//------------------------------------
void CDCDedxValidationAlgorithm::getExpRunInfo()
{

  int cruns = 0;
  for (auto expRun : getRunList()) {
    if (cruns == 0) B2INFO("start exp " << expRun.first << " and run " << expRun.second << "");
    cruns++;
  }

  const auto erStart = getRunList()[0];
  int estart = erStart.first;
  int rstart = erStart.second;

  updateDBObjPtrs(1, rstart, estart);

  if (m_suffix.length() > 0) m_suffix = Form("%s_e%d_r%d", m_suffix.data(), estart, rstart);
  else  m_suffix = Form("e%d_r%d", estart, rstart);
}


void CDCDedxValidationAlgorithm::radeeValidation()
{
  auto ttree = getObjectPtr<TTree>("tRadee");

  double dedx, costh, p, injtime = 0.0, injring = 1.0;
  int charge;

  std::vector<double>* dedxhit = 0, *enta = 0;
  std::vector<int>* layer = 0;

  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("p", &p);
  ttree->SetBranchAddress("costh", &costh);
  ttree->SetBranchAddress("charge", &charge);
  ttree->SetBranchAddress("injtime", &injtime);
  ttree->SetBranchAddress("injring", &injring);
  ttree->SetBranchAddress("dedxhit", &dedxhit);
  ttree->SetBranchAddress("entaRS", &enta);
  ttree->SetBranchAddress("layer", &layer);

  std::vector<double> vtlocaledges;
  defineTimeBins(vtlocaledges);
  m_tbins = vtlocaledges.size() - 1;
  m_tedges = &vtlocaledges[0];

  std::array<std::array<std::vector<TH1D*>, 2>, 3> hdedx_mom;
  std::array<std::vector<TH1D*>, 2> hdedx_mom_peaks, hdedx_inj, hdedx_oned;
  TH1D* htimes = new TH1D("htimes", "", m_tbins, m_tedges);

  const double momBinWidth = (m_momMax - m_momMin) / m_momBins;
  const double momBinW = (4.0 - m_momMin) / 4;

  std::string scos[3] = {"acos", "posCosth", "negCosth"};
  std::string stype[2] = {"posi", "elec"};
  std::string sLayer[2] = {"IL", "OL"};

  // Define histograms for momentum bins and charge types
  for (int ic = 0; ic < 3; ic++) {
    for (int it = 0; it < 2; ++it) {
      hdedx_mom[ic][it].resize(m_momBins);
      defineHisto(hdedx_mom[ic][it], "mom", Form("%s_%s", scos[ic].data(), stype[it].data()));
    }
  }

  // Define histograms for injection time bins and rings
  for (unsigned int ir = 0; ir < 2; ir++) {
    hdedx_inj[ir].resize(m_tbins);
    hdedx_mom_peaks[ir].resize(4);
    hdedx_oned[ir].resize(m_eaBin);

    defineHisto(hdedx_inj[ir], "inj", m_sring[ir].data());
    defineHisto(hdedx_mom_peaks[ir], "mom_peaks", Form("%s", stype[ir].data()));
    defineHisto(hdedx_oned[ir], "oned", Form("%s", sLayer[ir].data()));
  }

  double eaBW = (m_eaMax - m_eaMin) / m_eaBin;
  double icos[2] = {0, -1};
  double chgtype;

  // Loop over all the entries in the tree
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);

    // Skip invalid events
    if (dedx <= 0 || injtime < 0 || injring < 0) continue;

    // Calculate momentum bin index for hdedx_mom
    int binIndex = static_cast<int>((abs(p) - m_momMin) / momBinWidth);

    // Determine cos(theta) category
    icos[1] = (costh > 0) ? 1 : ((costh < 0) ? 2 : 0);

    // Determine charge type
    chgtype = (charge > 0) ? 0 : 1;

    // Fill momentum histograms (only if binIndex is valid)
    if (binIndex >= 0 && binIndex < m_momBins) {
      hdedx_mom[icos[0]][chgtype][binIndex]->Fill(dedx);
      hdedx_mom[icos[1]][chgtype][binIndex]->Fill(dedx);
    }

    // Add larger times to the last bin
    if (injtime > m_tedges[m_tbins]) injtime = m_tedges[m_tbins] - 10.0;

    // Injection ring type
    int wr = (injring > 0.5) ? 1 : 0;

    // Injection time bin
    unsigned int tb = htimes->GetXaxis()->FindBin(injtime);
    tb = std::min(tb, static_cast<unsigned int>(m_tbins)) - 1;

    // Fill injection time and dE/dx histograms
    htimes->Fill(injtime);
    hdedx_inj[wr][tb]->Fill(dedx);

    // Fill hdedx_mom_peaks with its own binning
    int binI = static_cast<int>((abs(p) - m_momMin) / momBinW);
    if (binI >= 0 && binI < 4) {
      hdedx_mom_peaks[chgtype][binI]->Fill(dedx);
    }

    // Fill dE/dx in enta bins from hits
    for (unsigned int j = 0; j < dedxhit->size(); ++j) {
      if (dedxhit->at(j) == 0) continue;

      double entaval = enta->at(j);
      int ibin = std::floor((entaval - m_eaMin) / eaBW);
      if (ibin < 0 || ibin >= m_eaBin) continue;

      int mL = (layer->at(j) < 8) ? 0 : 1;
      hdedx_oned[mL][ibin]->Fill(dedxhit->at(j));
    }
  }


  for (int ic = 0; ic < 3; ic++)
    for (int it = 0; it < 2; ++it)
      printCanvas(hdedx_mom[ic][it], Form("plots/mom/dedx_vs_mom_%s_%s_%s", scos[ic].data(), stype[it].data(), m_suffix.data()), "mom");

  for (int it = 0; it < 2; ++it) {
    printCanvas(hdedx_inj[it], Form("plots/injection/dedx_vs_inj_%s_%s", m_sring[it].data(), m_suffix.data()), "inj");
    printCanvas(hdedx_oned[it], Form("plots/oneD/dedx_vs_1D_%s_%s", sLayer[it].data(), m_suffix.data()), "oned");
  }

  printCanvasdEdx(hdedx_mom_peaks, Form("plots/mom/dedxpeaks_vs_mom_%s", m_suffix.data()), "mom");

}

void CDCDedxValidationAlgorithm::bhabhaValidation()
{
  auto ttree = getObjectPtr<TTree>("tBhabha");

  double dedx, costh;
  int run, charge;

  std::vector<int>* wire = 0;
  ttree->SetBranchAddress("wire", &wire);

  std::vector<double>* dedxhit = 0;
  ttree->SetBranchAddress("dedxhit", &dedxhit);

  ttree->SetBranchAddress("dedx", &dedx);
  ttree->SetBranchAddress("run", &run);
  ttree->SetBranchAddress("charge", &charge);
  ttree->SetBranchAddress("costh", &costh);

  std::map<int, TH1D*> hdedx_run;
  std::array<std::vector<TH1D*>, 3> hdedx_cos;
  std::array<std::vector<TH1D*>, 2> hdedx_cos_peaks;
  std::vector<TH1D*> hdedxhit(c_nSenseWires);

  const double cosBinWidth = (m_cosMax - m_cosMin) / m_cosBins;
  const double cosBinW = (m_cosMax - m_cosMin) / 4;

  std::string stype[3] = {"all", "posi", "elec"};

  for (int it = 0; it < 3; ++it) {
    hdedx_cos[it].resize(m_cosBins);
    defineHisto(hdedx_cos[it], "costh", stype[it]);
  }

  for (int ir = 0; ir < 2; ir++) {
    hdedx_cos_peaks[ir].resize(4);
    defineHisto(hdedx_cos_peaks[ir], "cos_peaks", Form("%s", stype[ir + 1].data()));
  }

  defineHisto(hdedxhit, "wire", "wire");

  // Loop over all the entries in the tree
  for (int i = 0; i < ttree->GetEntries(); ++i) {
    ttree->GetEvent(i);
    if (dedx <= 0) continue;

    // Check if a dE/dx histogram for this run number already exists
    if (hdedx_run.find(run) == hdedx_run.end()) {
      std::string histName = Form("hist_dedx_run_%d", run);
      std::string histTitle = Form("dE/dx Histogram for Run %d", run);
      hdedx_run[run] = new TH1D(histName.data(), histTitle.data(), m_dedxBins, m_dedxMin, m_dedxMax);
    }

    // Fill run-specific histogram
    hdedx_run[run]->Fill(dedx);

    // Fill cos(theta) histograms (all charge + by charge sign)
    int binIndex = static_cast<int>((costh - m_cosMin) / cosBinWidth);
    if (binIndex >= 0 && binIndex < m_cosBins) {
      hdedx_cos[0][binIndex]->Fill(dedx);  // All charge

      if (charge > 0)
        hdedx_cos[1][binIndex]->Fill(dedx);
      else if (charge < 0)
        hdedx_cos[2][binIndex]->Fill(dedx);
    }

    // Fill dE/dx for each wire hit
    for (unsigned int j = 0; j < wire->size(); ++j) {
      int jwire = wire->at(j);
      double jhitdedx = dedxhit->at(j);
      hdedxhit[jwire]->Fill(jhitdedx);
    }

    // Fill cos(theta) peaks histograms
    int binI = static_cast<int>((costh - m_cosMin) / cosBinW);
    if (binI >= 0 && binI < 4) {
      if (charge > 0)
        hdedx_cos_peaks[0][binI]->Fill(dedx);
      else if (charge < 0)
        hdedx_cos_peaks[1][binI]->Fill(dedx);
    }
  }

  printCanvasRun(hdedx_run, Form("plots/run/dedx_vs_run_%s", m_suffix.data()));
  printCanvas(hdedx_cos[0], Form("plots/costh/dedx_vs_cos_all_%s", m_suffix.data()), "costh");
  printCanvas(hdedx_cos[1], Form("plots/costh/dedx_vs_cos_positrons_%s", m_suffix.data()), "costh");
  printCanvas(hdedx_cos[2], Form("plots/costh/dedx_vs_cos_electrons_%s", m_suffix.data()), "costh");
  printCanvasdEdx(hdedx_cos_peaks, Form("plots/costh/dedxpeaks_vs_cos_%s", m_suffix.data()), "costh");
  wireGain(hdedxhit);
}

//------------------------------------
void CDCDedxValidationAlgorithm::defineHisto(std::vector<TH1D*>& htemp, std::string var, std::string stype)
{
  int xbins;
  double xmin, xmax;
  double binWidth = 0.0;

  if (var == "mom") {
    xbins = m_momBins; xmin = m_momMin; xmax = m_momMax;
  } else if (var == "oned") {
    xbins = m_eaBin; xmin = m_eaMin; xmax = m_eaMax; m_dedxBins = 250;
  } else if (var == "costh") {
    xbins = m_cosBins; xmin = m_cosMin; xmax = m_cosMax;
  } else if (var == "inj") {
    xbins  = m_tbins;
  } else if (var == "mom_peaks") {
    xbins = 4; xmin = m_momMin; xmax = 4.0;
  }  else if (var == "cos_peaks") {
    xbins = 4; xmin = m_cosMin; xmax = m_cosMax;
  }  else {
    xbins = c_nSenseWires; m_dedxBins = 250;
  }

  if (var == "costh" || var == "mom" || var == "mom_peaks" || var == "cos_peaks" || var == "oned") {
    binWidth = (xmax - xmin) / xbins;
  }

  for (int ic = 0; ic < xbins; ic++) {
    std::string title = Form("dedxhit-dist, wire:%d", ic);
    std::string name = Form("hdedx_%s_%d", var.data(), ic);

    if (var == "costh" || var == "mom" || var == "mom_peaks" || var == "cos_peaks" || var == "oned") {
      double min = ic * binWidth + xmin;
      double max = min + binWidth;
      title = Form("%s: (%0.02f, %0.02f) %s", var.data(), min, max, stype.data());
      name = Form("hdedx_%s_%s_%d", var.data(), stype.data(), ic);
    } else if (var == "inj") {
      std::string label = getTimeBinLabel(m_tedges[ic], ic);
      title = Form("%s, time(%s)", stype.data(), label.data());
      name = Form("h%s_%s_t%d", var.data(), stype.data(), ic);
    }
    htemp[ic] = new TH1D(name.data(), "", m_dedxBins, m_dedxMin, m_dedxMax);
    htemp[ic]->SetTitle(Form("%s;dedx;entries", title.data()));
  }
}

void CDCDedxValidationAlgorithm::printCanvasdEdx(std::array<std::vector<TH1D*>, 2>& htemp, std::string namesfx, std::string svar)
{
  int xbins = 4;
  double xmin, xmax;

  if (svar == "mom") {
    xmin = m_momMin; xmax = 4.0;
  } else if (svar == "costh") {
    xmin = m_cosMin; xmax = m_cosMax;
  } else {
    B2FATAL("wrong input");
  }
  double binWidth = (xmax - xmin) / xbins;

  // Set up the TCanvas with 2x2 grid
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 1200, 1200);
  ctmp->Divide(2, 2); // Divide into 2x2 grid

  // Prepare the PDF output
  std::stringstream psname;
  psname << Form("%s.pdf[", namesfx.data());
  ctmp->Print(psname.str().c_str());
  psname.str("");
  psname << Form("%s.pdf", namesfx.data());

  // Iterate through the histograms and plot them in the canvas

  for (int i = 0; i < xbins; ++i) {

    ctmp->cd(i % 4 + 1);

    double emean, emeanErr, esigma, esigmaErr;
    double pmean, pmeanErr, psigma, psigmaErr;

    fit(htemp[0][i], emean, emeanErr, esigma, esigmaErr);
    fit(htemp[1][i], pmean, pmeanErr, psigma, psigmaErr);

    double min = i * binWidth + xmin;
    double max = min + binWidth;

    TPaveText pt(0.6, 0.63, 0.85, 0.89, "NBNDC");
    setTextCosmetics(pt, kBlack);
    pt.AddText("e+");
    pt.AddText(Form("#mu_{fit}: %0.03f#pm%0.03f", emean, emeanErr));
    pt.AddText(Form("#sigma_{fit}: %0.03f#pm%0.03f", esigma, esigmaErr));

    pt.AddText("e-");
    pt.AddText(Form("#mu_{fit}: %0.03f#pm%0.03f", pmean, pmeanErr));
    pt.AddText(Form("#sigma_{fit}: %0.03f#pm%0.03f", psigma, psigmaErr));

    htemp[0][i]->SetStats(0);
    htemp[1][i]->SetStats(0);
    htemp[0][i]->SetFillColor(0);
    htemp[1][i]->SetFillColor(0);
    htemp[0][i]->SetLineColor(8);
    htemp[1][i]->SetLineColor(9);
    htemp[0][i]->SetTitle(Form("%s: (%0.02f, %0.02f)", svar.data(), min, max));
    if (htemp[0][i]->GetEntries() > 0)
      htemp[0][i]->Scale(1 / htemp[0][i]->GetEntries());
    if (htemp[1][i]->GetEntries() > 0)
      htemp[1][i]->Scale(1 / htemp[1][i]->GetEntries());

    if (htemp[1][i]->GetMaximum() > htemp[0][i]->GetMaximum())
      htemp[0][i]->SetMaximum(htemp[1][i]->GetMaximum());

    htemp[0][i]->DrawCopy("HIST");
    htemp[1][i]->DrawCopy("same HIST");
    pt.DrawClone("same");

    TLegend* lego = new TLegend(0.15, 0.67, 0.3, 0.8);
    lego->AddEntry(htemp[0][i], "e+", "l");
    lego->AddEntry(htemp[1][i], "e-", "l");
    lego->Draw("same");

    if ((i + 1) % 4 == 0 || i == xbins - 1) {
      ctmp->SetBatch(kTRUE);
      ctmp->Print(psname.str().c_str());
      if ((i + 1) % 4 == 0) ctmp->Clear("D");
    }
  }

  psname.str("");
  psname << Form("%s.pdf]", namesfx.data());
  ctmp->Print(psname.str().c_str());

  delete ctmp;
}

void CDCDedxValidationAlgorithm::printCanvas(std::vector<TH1D*>& htemp, std::string namesfx, std::string svar)
{
  int xbins;
  double xmin, xmax;

  if (svar == "mom") {
    xbins = m_momBins; xmin = m_momMin; xmax = m_momMax;
  } else if (svar == "oned") {
    xbins = m_eaBin; xmin = m_eaMin; xmax = m_eaMax;
  } else if (svar == "costh") {
    xbins = m_cosBins; xmin = m_cosMin; xmax = m_cosMax;
  }  else if (svar == "inj") {
    xbins  = m_tbins;
  } else if (svar == "mom_peaks") {
    xbins = 4; xmin = m_momMin; xmax = 4.0;
  } else {
    B2FATAL("wrong input");
  }

  // Set up the TCanvas with 4x4 grid
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 1200, 1200);
  ctmp->Divide(4, 4); // Divide into 4x4 grid

  // Prepare the PDF output
  std::stringstream psname;
  psname << Form("%s.pdf[", namesfx.data());
  ctmp->Print(psname.str().c_str());
  psname.str("");
  psname << Form("%s.pdf", namesfx.data());

  std::ofstream outFile;
  outFile.open(Form("%s.txt", namesfx.data()));
  CDCDedxWireGainAlgorithm wireg;

  // Iterate through the histograms and plot them in the canvas
  for (int i = 0; i < xbins; ++i) {

    ctmp->cd(i % 16 + 1);
    TPaveText pt(0.6, 0.73, 0.85, 0.89, "NBNDC");
    setTextCosmetics(pt, kBlack);

    if (svar == "oned") {
      unsigned int minbin, maxbin;
      wireg.getTruncatedBins(htemp[i], minbin, maxbin);
      htemp[i]->SetTitle(Form("dedxhit-dist, entabin: %d ;%d;%d", i, minbin, maxbin));

      double dedxmean  = wireg.getTruncationMean(htemp[i], minbin, maxbin);

      const double binWidth = (xmax - xmin) / xbins;
      double binCenter = xmin + (i + 0.5) * binWidth; // Calculate bin center for cos(theta) or mom

      outFile << binCenter << " " <<  dedxmean << std::endl;
    } else {
      double mean, meanErr, sigma, sigmaErr;
      fit(htemp[i], mean, meanErr, sigma, sigmaErr);

      if (svar == "mom" || svar == "costh" || svar == "mom_peaks") {
        const double binWidth = (xmax - xmin) / xbins;
        double binCenter = xmin + (i + 0.5) * binWidth; // Calculate bin center for cos(theta) or mom

        outFile << binCenter << " " <<  mean << " " << meanErr << " " << sigma << " " << sigmaErr << std::endl;
      } else {
        std::string label = getTimeBinLabel(m_tedges[i], i);
        outFile << i << " " << label << " " <<  mean << " " << meanErr << " " << sigma << " " << sigmaErr << std::endl;
      }

      pt.AddText(Form("#mu_{fit}: %0.03f#pm%0.03f", mean, meanErr));
      pt.AddText(Form("#sigma_{fit}: %0.03f#pm%0.03f", sigma, sigmaErr));
    }
    htemp[i]->SetStats(0);
    htemp[i]->DrawCopy("");
    pt.DrawClone("same");

    if ((i + 1) % 16 == 0) {
      ctmp->SetBatch(kTRUE);
      ctmp->Print(psname.str().c_str());
      ctmp->Clear("D");
    }
  }

  ctmp->Print(psname.str().c_str());
  psname.str("");
  psname << Form("%s.pdf]", namesfx.data());
  ctmp->Print(psname.str().c_str());

  outFile.close();

  delete ctmp;
}

void CDCDedxValidationAlgorithm::fit(TH1D*& hist, double& mean, double& meanErr, double& sigma, double& sigmaErr)
{

  std::string status = "";

  if (hist->Integral() > 100)
    fitGaussianWRange(hist, status);

  if (status != "fitOK") {
    hist->SetFillColor(kOrange);
    mean = 0.0, meanErr = 0.0, sigma = 0.0, sigmaErr = 0.0;
  } else {
    mean = hist->GetFunction("gaus")->GetParameter(1);
    meanErr = hist->GetFunction("gaus")->GetParError(1);
    sigma = hist->GetFunction("gaus")->GetParameter(2);
    sigmaErr = hist->GetFunction("gaus")->GetParError(2);
    hist->SetFillColor(kYellow);
  }
}

void CDCDedxValidationAlgorithm::printCanvasRun(std::map<int, TH1D*>& htemp, std::string namesfx)
{
  // Set up the TCanvas with 4x4 grid
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 1200, 1200);
  ctmp->Divide(4, 4); // Divide into 4x4 grid

  // Prepare the PDF output
  std::stringstream psname;
  psname << Form("%s.pdf[", namesfx.data());
  ctmp->Print(psname.str().c_str());
  psname.str("");
  psname << Form("%s.pdf", namesfx.data());

  std::ofstream outFile;
  outFile.open(Form("%s.txt", namesfx.data()));

  // Iterate through the histograms and plot them in the canvas
  int irun = 0;
  for (const auto& entry : htemp) {
    int run = entry.first;
    TH1D* hist = entry.second;

    ctmp->cd(irun % 16 + 1);

    TPaveText pt(0.6, 0.73, 0.85, 0.89, "NBNDC");
    setTextCosmetics(pt, kBlack);

    double mean, meanErr, sigma, sigmaErr;
    fit(hist, mean, meanErr, sigma, sigmaErr);

    outFile << run << " " <<  mean << " " << meanErr << " " << sigma << " " << sigmaErr << std::endl;

    pt.AddText(Form("#mu_{fit}: %0.03f#pm%0.03f", mean, meanErr));
    pt.AddText(Form("#sigma_{fit}: %0.03f#pm%0.03f", sigma, sigmaErr));

    hist->SetStats(0);
    hist->DrawCopy("");
    pt.DrawClone("same");

    if ((irun + 1) % 16 == 0 || irun == int(htemp.size() - 1)) {
      ctmp->SetBatch(kTRUE);
      ctmp->Print(psname.str().c_str());
      ctmp->Clear("D");
    }
    irun++;
  }

  ctmp->Print(psname.str().c_str());
  psname.str("");
  psname << Form("%s.pdf]", namesfx.data());
  ctmp->Print(psname.str().c_str());

  outFile.close();

  delete ctmp;
}

//----------------------------------------
void CDCDedxValidationAlgorithm::fitGaussianWRange(TH1D*& temphist, std::string& status)
{
  double histmean = temphist->GetMean();
  double histrms = temphist->GetRMS();
  temphist->GetXaxis()->SetRangeUser(histmean - 5.0 * histrms, histmean + 5.0 * histrms);

  int fs = temphist->Fit("gaus", "Q0");
  if (fs != 0) {
    B2INFO(Form("\tFit (round 1) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
    status = "fitFailed";
    return;
  } else {
    double mean = temphist->GetFunction("gaus")->GetParameter(1);
    double width = temphist->GetFunction("gaus")->GetParameter(2);
    temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
    fs = temphist->Fit("gaus", "QR", "", mean - m_sigmaR * width, mean + m_sigmaR * width);
    if (fs != 0) {
      B2INFO(Form("\tFit (round 2) for hist (%s) failed (status = %d)", temphist->GetName(), fs));
      status = "fitFailed";
      return;
    } else {
      temphist->GetXaxis()->SetRangeUser(mean - 5.0 * width, mean + 5.0 * width);
      B2INFO(Form("\tFit for hist (%s) successful (status = %d)", temphist->GetName(), fs));
      status = "fitOK";
    }
  }
}

void CDCDedxValidationAlgorithm::wireGain(std::vector<TH1D*>& hdedxhit)
{

  std::vector<double> vdedx_means;
  std::vector<double> layermean(c_maxNSenseLayers);
  std::vector<double> lgmean(c_maxNSenseLayers);

  std::ofstream outFile, outFileLayer, outFileAvg, outFilebdwire;
  outFile.open(Form("plots/wire/dedx_mean_gwire_%s.txt", m_suffix.data()));
  outFilebdwire.open(Form("plots/wire/dedx_mean_badwire_%s.txt", m_suffix.data()));
  outFileLayer.open(Form("plots/wire/dedx_mean_layer_%s.txt", m_suffix.data()));
  outFileAvg.open(Form("plots/wire/dedx_mean_layer_avg_%s.txt", m_suffix.data()));

  int activelayers = 0;
  double layeravg = 0.0;

  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance(&(*m_cdcGeo));
  CDCDedxWireGainAlgorithm wireg;

  DBObjPtr<CDCDedxBadWires> Badwire;

  int jwire = -1;
  for (unsigned int il = 0; il < c_maxNSenseLayers; ++il) {

    int activewires = 0, goodwires = 0;
    layermean[il] = 0.0;
    lgmean[il] = 0.0;

    for (unsigned int iw = 0; iw < cdcgeo.nWiresInLayer(il); ++iw) {
      jwire++;

      unsigned int minbin, maxbin;
      wireg.getTruncatedBins(hdedxhit[jwire], minbin, maxbin);
      hdedxhit[jwire]->SetTitle(Form("dedxhit-dist, wire: %d ;%d;%d", jwire, minbin, maxbin));

      double dedxmean  = wireg.getTruncationMean(hdedxhit[jwire], minbin, maxbin);
      vdedx_means.push_back(dedxmean);

      if (Badwire->getBadWireStatus(jwire) == kTRUE)
        outFilebdwire << jwire << " " << dedxmean << std::endl;
      else
        outFile << jwire << " " << dedxmean << std::endl;

      if (vdedx_means.at(jwire) > 0) {
        layermean[il] += vdedx_means.at(jwire);
        activewires++;
        if (Badwire->getBadWireStatus(jwire) != kTRUE) {
          lgmean[il] += vdedx_means.at(jwire);
          goodwires++;
        }
      }
    }

    if (activewires > 0) layermean[il] /= activewires;
    else layermean[il] = 1.0;

    if (goodwires > 0) lgmean[il] /= goodwires;
    else lgmean[il] = 1.0;

    outFileLayer << il << " " << layermean[il] << " " << lgmean[il] << std::endl;

    //calculate outer layer average for active layer
    if (il >= 8 && layermean[il] > 0) {
      layeravg += layermean[il];
      activelayers++;
    }
  }

  //normalisation of wiregains to get outergain ~1.0
  if (activelayers > 0) layeravg /= activelayers;
  outFileAvg << layeravg << std::endl;

  outFile.close();
  outFilebdwire.close();
  outFileLayer.close();
  outFileAvg.close();
  printCanvasWire(hdedxhit, Form("plots/wire/dedx_vs_wire_%s", m_suffix.data()), vdedx_means);
}

void CDCDedxValidationAlgorithm::printCanvasWire(std::vector<TH1D*> temp, std::string namesfx,
                                                 const std::vector<double>& vdedx_mean)
{
  TCanvas* ctmp = new TCanvas("tmp", "tmp", 900, 900);
  ctmp->Divide(4, 4);

  std::stringstream psname;
  psname << Form("%s.pdf[", namesfx.data());
  ctmp->Print(psname.str().c_str());
  psname.str("");
  psname << Form("%s.pdf", namesfx.data());

  for (unsigned int ip = 0; ip < c_nwireCDC; ip++) {
    int minbin = std::stoi(temp[ip]->GetXaxis()->GetTitle());
    int maxbin = std::stoi(temp[ip]->GetYaxis()->GetTitle());
    temp[ip]->SetFillColor(kYellow - 9);
    temp[ip]->SetTitle(Form("%s, #mu_{trunc} %0.03f;dedxhit;entries", temp[ip]->GetTitle(), vdedx_mean.at(ip)));

    ctmp->cd(ip % 16 + 1);
    gPad->cd();
    temp[ip]->DrawCopy("hist");
    TH1D* hdedxhitC = (TH1D*)temp[ip]->Clone(Form("%sC", temp[ip]->GetName()));
    hdedxhitC->GetXaxis()->SetRange(minbin, maxbin);
    hdedxhitC->SetFillColor(kAzure + 1);
    hdedxhitC->DrawCopy("same histo");

    if ((ip + 1) % 16 == 0) {
      ctmp->SetBatch(kTRUE);
      ctmp->Print(psname.str().c_str());
      ctmp->Clear("D");
    }
    delete temp[ip];
    delete hdedxhitC;

  }

  psname.str("");
  psname << Form("%s.pdf]",  namesfx.data());
  ctmp->Print(psname.str().c_str());
  delete ctmp;
}

void CDCDedxValidationAlgorithm::defineTimeBins(std::vector<double>& vtlocaledges)
{
  double fixedges[69];
  for (int ib = 0; ib < 69; ib++) {
    fixedges[ib] = ib * 0.5 * 1e3;
    if (ib > 40 && ib <= 60) fixedges[ib] = fixedges[ib - 1] + 1.0 * 1e3;
    else if (ib > 60 && ib <= 64) fixedges[ib] = fixedges[ib - 1] + 10.0 * 1e3;
    else if (ib > 64 && ib <= 65) fixedges[ib] = fixedges[ib - 1] + 420.0 * 1e3;
    else if (ib > 65 && ib <= 66) fixedges[ib] = fixedges[ib - 1] + 500.0 * 1e3;
    else if (ib > 66) fixedges[ib] = fixedges[ib - 1] + 2e6;
    vtlocaledges.push_back(fixedges[ib]);
  }
}

//--------------------------
void CDCDedxValidationAlgorithm::plotEventStats()
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

  cstats.Print(Form("cdcdedx_stats_%s.pdf", m_suffix.data()));
}

void CDCDedxValidationAlgorithm::DatabaseIN(int experiment, int run)
{
  if (m_EventMetaData.isValid()) {
    m_EventMetaData->setExperiment(experiment);
    m_EventMetaData->setRun(run);
  }

  auto& dbConfiguration = Conditions::Configuration::getInstance();
  dbConfiguration.setGlobalTags({"online"});
  if (!m_testingPayloadName.empty() && m_GlobalTagName.empty()) {
    dbConfiguration.prependTestingPayloadLocation(m_testingPayloadName);
  } else if (m_testingPayloadName.empty() && !m_GlobalTagName.empty()) {
    dbConfiguration.prependGlobalTag(m_GlobalTagName);
  } else
    B2FATAL("Setting both testing payload and Global Tag or setting no one of them.");

  /* Mimic a module initialization. */
  DataStore::Instance().setInitializeActive(true);
  m_EventMetaData.registerInDataStore();
  DataStore::Instance().setInitializeActive(false);
  if (!m_EventMetaData.isValid())
    m_EventMetaData.construct(1, run, experiment);

  /* Database instance and configuration. */
  DBStore& dbStore = DBStore::Instance();
  dbStore.update();
  dbStore.updateEvent();
}

WireGainData CDCDedxValidationAlgorithm::getwiregain(int experiment, int run)
{

  DatabaseIN(experiment, run);

  std::vector<double> wiregain;
  std::vector<double> layermean(c_maxNSenseLayers);

  DBObjPtr<CDCDedxWireGain> DBWireGains;
  if (!DBWireGains.isValid())  B2FATAL("Wire gain data are not valid.");

  CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance(&(*m_cdcGeo));

  int jwire = -1;
  for (unsigned int il = 0; il < c_maxNSenseLayers; ++il) {

    int activewires = 0;
    layermean[il] = 0.0;

    for (unsigned int iw = 0; iw < cdcgeo.nWiresInLayer(il); ++iw) {
      jwire++;

      wiregain.push_back(DBWireGains->getWireGain(jwire));

      if (wiregain.at(jwire) > 0) {
        layermean[il] += wiregain.at(jwire);
        activewires++;
      }
    }

    if (activewires > 0) layermean[il] /= activewires;
    else layermean[il] = 1.0;
  }

  resetDatabase();
  return { wiregain, layermean };
}

CosGainData CDCDedxValidationAlgorithm::getcosgain(int experiment, int run)
{

  DatabaseIN(experiment, run);

  std::vector<double> cosgain, cos;

  DBObjPtr<CDCDedxCosineCor> DBCosineCor;
  if (!DBCosineCor.isValid())  B2FATAL("Cosine gain data are not valid.");

  unsigned int nCosBins = DBCosineCor->getSize();

  for (unsigned int il = 0; il < nCosBins; ++il) {
    double costh = -1.0 + (il + 0.5) * 2.0 / nCosBins;
    costh += .000001;
    cosgain.push_back(DBCosineCor->getMean(il));
    cos.push_back(costh);
  }

  resetDatabase();
  return {cosgain, cos};
}

OnedData CDCDedxValidationAlgorithm::getonedgain(int experiment, int run)
{

  DatabaseIN(experiment, run);

  std::vector<double> inner1D, outer1D, Enta;

  DBObjPtr<CDCDedx1DCell> DBOneDCell;
  if (!DBOneDCell.isValid())  B2FATAL("OneD cell gain data are not valid.");

  for (int i = 0; i < 2; i++) {

    unsigned int nBins = DBOneDCell->getNBins(i);
    double binSize = TMath::Pi() / nBins;

    for (unsigned int nbin = 0; nbin < nBins; nbin++) {

      double enta = (-1 * TMath::Pi() / 2.0) + binSize * nbin;
      if (i == 0) {
        Enta.push_back(enta);
        inner1D.push_back(DBOneDCell->getMean(0, nbin));
      } else
        outer1D.push_back(DBOneDCell->getMean(17, nbin));
    }
  }
  resetDatabase();
  return {inner1D, outer1D, Enta};

}

double CDCDedxValidationAlgorithm::getrungain(int experiment, int run)
{

  DatabaseIN(experiment, run);

  DBObjPtr<CDCDedxRunGain> RunGain;
  if (!RunGain.isValid())  B2FATAL("Run gain data are not valid.");
  double gain = RunGain->getRunGain();
  return gain;
}

void CDCDedxValidationAlgorithm::resetDatabase()
{
  /* Reset both DataStore and Database. */
  DataStore::Instance().reset();
  Database::Instance().reset(false);
  DBStore::Instance().reset(false);
}
