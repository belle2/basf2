/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/calibration/CDCdEdx/CDCDedx1DCellAlgorithm.h>
#include <cmath>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
CDCDedx1DCellAlgorithm::CDCDedx1DCellAlgorithm() :
  CalibrationAlgorithm("CDCDedxElectronCollector"),
  m_eaMin(-TMath::Pi() / 2),
  m_eaMax(+TMath::Pi() / 2),
  m_eaBin(316),
  m_dedxMin(0.0),
  m_dedxMax(5.0),
  m_dedxBin(250),
  m_ptMax(8.0),
  m_cosMax(1.0),
  m_truncMin(0.05),
  m_truncMax(0.75),
  m_binSplit(3),
  m_chargeType(0),
  m_adjustFac(1.00),
  isFixTrunc(false),
  isVarBins(true),
  isRotSymm(false),
  isMakePlots(true),
  isPrintLog(false),
  isMerge(true),
  m_suffix("")
{
  // Set module properties
  setDescription("A calibration algorithm for the CDC dE/dx entrance angle cleanup correction");
}

//-----------------------------------------------------------------
//                 Run the calibration
//-----------------------------------------------------------------
CalibrationAlgorithm::EResult CDCDedx1DCellAlgorithm::calibrate()
{

  getExpRunInfo();

  if (!m_DBOneDCell.isValid())
    B2FATAL("There is no valid previous payload for CDCDedx1DCell");

  //reading radiative electron collector TREE
  auto ttree = getObjectPtr<TTree>("tree");
  if (!ttree) return c_NotEnoughData;

  std::vector<double>* dedxhit = 0, *enta = 0;
  std::vector<int>* layer = 0;
  double pt = 0, costh = 0;

  ttree->SetBranchAddress("dedxhit", &dedxhit);
  ttree->SetBranchAddress("entaRS", &enta);
  ttree->SetBranchAddress("layer", &layer);
  ttree->SetBranchAddress("pt", &pt);
  ttree->SetBranchAddress("costh", &costh);

  //repair nbins if they are not divisible accordingly
  m_eaBin = m_eaBin * m_binSplit;
  m_eaBW = (m_eaMax - m_eaMin) / m_eaBin;

  //Settings of variables bins
  CreateBinMapping();

  if (isPrintLog) {
    B2INFO("inner layers bins: " << m_eaBinLocal[0]);
    B2INFO("outer layers bins: " << m_eaBinLocal[1]);
  }

  // dedxhit vector to store dE/dx values for each enta bin
  std::vector<TH1D*> hdedxhit[2];
  TH1D* hdedxlay[2];
  TH1D* hentalay[2];

  TH2D* hptcosth = new TH2D("hptcosth", "pt vs costh dist;pt;costh", 1000, -8.0, 8.0, 1000, -1.0, 1.0);

  defineHisto(hdedxhit, hdedxlay, hentalay);

  //Star filling histogram defined above
  for (int i = 0; i < ttree->GetEntries(); ++i) {

    ttree->GetEvent(i);

    if (std::abs(costh) > m_cosMax) continue;

    // remove wide angle bhabha tracks
    // double mom = pt/sqrt(1-costh*costh);
    // if(abs(pt)<2.4 && abs(mom)>3.6)continue;

    if (std::abs(pt) > m_ptMax) continue;

    //change to random 10%
    int rand = gRandom->Integer(100);
    if (rand < 10) hptcosth->Fill(pt, costh);

    for (unsigned int j = 0; j < dedxhit->size(); ++j) {

      if (dedxhit->at(j) == 0) continue;

      double entaval = enta->at(j);
      //Mapped bin corresponds to entaval
      int ibin = std::floor((entaval - m_eaMin) / m_eaBW);
      if (ibin < 0 || ibin > m_eaBin) continue;

      int mL = -1;
      if (layer->at(j) < 8)mL = 0;
      else mL = 1;

      hdedxlay[mL]->Fill(dedxhit->at(j));
      if (rand < 10) hentalay[mL]->Fill(entaval);

      int jbinea = ibin;
      if (isVarBins) jbinea = m_binIndex[mL].at(ibin);
      hdedxhit[mL][jbinea]->Fill(dedxhit->at(j));
    }
  }

  for (int il = 0; il < 2; il++) {

    int minlay = 0, maxlay = 0;

    if (isFixTrunc) {
      getTruncatedBins(hdedxlay[il], minlay, maxlay);
      hdedxlay[il]->SetTitle(Form("%s;%d;%d", hdedxlay[il]->GetTitle(), minlay, maxlay));
    }

    std::vector<double>tempconst;
    tempconst.reserve(m_eaBinLocal[il]);

    for (int iea = 0; iea < m_eaBinLocal[il]; iea++) {

      int jea = iea;

      // rotation symmtery for 1<->3 and 4<->2 but only symmetric bin
      if (!isVarBins && isRotSymm) jea = rotationalBin(m_eaBinLocal[il], jea);

      TH1D* htemp = (TH1D*)hdedxhit[il][jea]->Clone(Form("h_%s_b%d_c", m_label[il].data(), jea));

      int minbin = 1, maxbin = 1;
      if (isFixTrunc) {
        minbin = minlay;
        maxbin = maxlay;
      } else {
        //extract truncation window per bin
        getTruncatedBins(htemp, minbin, maxbin);
      }

      double dedxmean;
      dedxmean = getTruncationMean(htemp, minbin, maxbin);
      tempconst.push_back(dedxmean);

      hdedxhit[il][iea]->SetTitle(Form("%s, #mu_{truc} = %0.5f;%d;%d", hdedxhit[il][iea]->GetTitle(), dedxmean, minbin, maxbin));
    }

    //Expending constants
    std::vector<double>layerconst;
    layerconst.reserve(m_eaBin);

    for (int iea = 0; iea < m_eaBin; iea++) {
      int jea = iea;
      if (isVarBins)  jea = m_binIndex[il].at(iea);
      layerconst.push_back(tempconst.at(jea));
    }

    // plot the rel constants var/sym bins
    if (isMakePlots)  plotRelConst(tempconst, layerconst, il);
    m_onedcors.push_back(layerconst);

    layerconst.clear();
    tempconst.clear();
  }

  //Saving final constants
  createPayload();

  if (isMakePlots) {

    //1. dE/dx dist. for entrance angle bins
    plotdedxHist(hdedxhit);

    //3. Inner and Outer layer dE/dx distributions
    plotLayerDist(hdedxlay);

    //4. entrance angle distribution sym/var bins
    plotQaPars(hentalay, hptcosth);

    //6. draw the final constants
    plotConstants();

    //7. plot statistics related plots here
    plotEventStats();
  }

  for (int il = 0; il < 2; il++) {
    delete hentalay[il];
    delete hdedxlay[il];
    for (int iea = 0; iea < m_eaBinLocal[il]; iea++)
      delete hdedxhit[il][iea];
  }

  delete hptcosth;
  return c_OK;
}

//--------------------------------------------------
void CDCDedx1DCellAlgorithm::getExpRunInfo()
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
  int eend = erEnd.first;
  int rend = erEnd.second;

  updateDBObjPtrs(1, rstart, estart);

  m_runExp = Form("Range (%d:%d,%d:%d)", estart, rstart, eend, rend);
  if (m_suffix.length() > 0) m_suffix = Form("%s_e%d_r%dr%d", m_suffix.data(), estart, rstart, rend);
  else  m_suffix = Form("e%d_r%dr%d", estart, rstart, rend);
}

//--------------------------------------------------
void CDCDedx1DCellAlgorithm::CreateBinMapping()
{

  std::map<int, std::vector<double>> bounds;
  std::map<int, std::vector<int>> steps;

  const std::array<int, 2> nDev{8, 4};
  bounds[0] = {0, 108, 123, 133, 158, 183, 193, 208, 316}; //il boundries
  steps[0] = {9, 3, 2, 1, 1, 2, 3, 9};  //il steps
  bounds[1] = {0, 38, 158, 278, 316}; //OL boundries
  steps[1] = {2, 1, 1, 2};  //OL steps

  for (int il = 0; il < 2; il++) {

    for (int ibin = 0; ibin <= nDev[il]; ibin++) bounds[il][ibin] = bounds[il][ibin] * m_binSplit;

    int ieaprime = -1, temp = -99, ibin = 0;

    double pastbin = m_eaMin;
    m_binValue[il].push_back(pastbin);

    for (int iea = 0; iea < m_eaBin; iea++) {

      if (isVarBins) {
        if (iea % int(bounds[il][ibin + 1]) == 0 && iea > 0) ibin++;
        int diff = iea - int(bounds[il][ibin]);
        if (diff % steps[il][ibin] == 0) ieaprime++;
      } else ieaprime = iea;

      m_binIndex[il].push_back(ieaprime);

      if (ieaprime != temp) {
        double binwidth = m_eaBW;
        if (isVarBins) binwidth =  m_eaBW * steps[il][ibin];
        double binvalue = pastbin + binwidth;
        pastbin = binvalue;
        if (std::abs(binvalue) < 1e-5)binvalue = 0;
        m_binValue[il].push_back(binvalue);
      }
      temp = ieaprime;
    }
    m_eaBinLocal.push_back(int(m_binValue[il].size()) - 1) ;
  }
  if (isMakePlots) plotMergeFactor(bounds, nDev, steps);
}

//--------------------------------------------------
void CDCDedx1DCellAlgorithm::defineHisto(std::vector<TH1D*> hdedxhit[2],  TH1D* hdedxlay[2], TH1D* hentalay[2])
{
  for (int il = 0; il < 2; il++) {

    std::string title = Form("dedxhit dist (%s): %s ; dedxhit;entries", m_label[il].data(), m_runExp.data());
    hdedxlay[il] = new TH1D(Form("hdedxlay%s", m_label[il].data()), "", m_dedxBin, m_dedxMin, m_dedxMax);
    hdedxlay[il]->SetTitle(Form("%s", title.data()));

    Double_t* nvarBins;
    nvarBins = &m_binValue[il][0];

    if (isVarBins) title = Form("entaRS dist (variable bins): %s: (%s); entaRS (#alpha);entries", m_label[il].data(), m_runExp.data());
    else title = Form("entaRS dist (sym. bins): %s: (%s); entaRS (#alpha);entries", m_label[il].data(), m_runExp.data());

    hentalay[il] = new TH1D(Form("hentalay%s", m_label[il].data()), "", m_eaBinLocal[il], nvarBins);
    hentalay[il]->SetTitle(Form("%s", title.data()));

    for (int iea = 0; iea < m_eaBinLocal[il]; iea++) {

      double min = m_binValue[il].at(iea);
      double max = m_binValue[il].at(iea + 1);
      double width =  max - min;

      if (isPrintLog) B2INFO("bin: " << iea << " ], min:" << min << " , max: " << max << " , width: " << width);

      title = Form("%s: entaRS = (%0.03f to %0.03f)", m_label[il].data(), min, max);
      hdedxhit[il].push_back(new TH1D(Form("hdedxhit_%s_bin%d", m_label[il].data(), iea), "", m_dedxBin, m_dedxMin, m_dedxMax));
      hdedxhit[il][iea]->SetTitle(Form("%s", title.data()));
    }
  }
}

//--------------------------------------------------
void CDCDedx1DCellAlgorithm::getTruncatedBins(TH1D* hist, int& binlow, int& binhigh)
{

  //calculating truncation average
  double sum = hist->Integral();
  if (sum <= 0 || hist->GetNbinsX() <= 0) {
    binlow = 1; binhigh = 1;
    return ;
  }

  binlow = 1.0; binhigh = 1.0;
  double sumPer5 = 0.0, sumPer75 = 0.0;
  for (int ibin = 1; ibin <= hist->GetNbinsX(); ibin++) {
    double bcdedx = hist->GetBinContent(ibin);
    if (sumPer5  <= m_truncMin * sum) {
      sumPer5 += bcdedx;
      binlow = ibin;
    }
    if (sumPer75  <= m_truncMax * sum) {
      sumPer75 += bcdedx;
      binhigh = ibin;
    }
  }
  return;
}

//--------------------------
double CDCDedx1DCellAlgorithm::getTruncationMean(TH1D* hist, int binlow, int binhigh)
{

  //calculating truncation average
  if (hist->Integral() < 100) return 1.0;

  if (binlow <= 0 || binhigh > hist->GetNbinsX())return 1.0;

  double binweights = 0., sumofbc = 0.;
  for (int ibin = binlow; ibin <= binhigh; ibin++) {
    double bcdedx = hist->GetBinContent(ibin);
    if (bcdedx > 0) {
      binweights += (bcdedx * hist->GetBinCenter(ibin));
      sumofbc += bcdedx;
    }
  }
  if (sumofbc > 0) return binweights / sumofbc;
  else return 1.0;
}

//--------------------------------------------------
void CDCDedx1DCellAlgorithm::createPayload()
{

  B2INFO("dE/dx one cell calibration: Generating payloads");

  for (unsigned int il = 0; il < 2; il++) {
    if (isMerge) {
      unsigned int nbins = m_DBOneDCell->getNBins(il);

      if (int(nbins) != m_eaBin)
        B2ERROR("merging failed because of unmatch bins (old " << m_eaBin << " new " << nbins << ")");

      for (unsigned int iea = 0; iea < nbins; iea++) {
        double prev = m_DBOneDCell->getMean(8 * il + 1, iea);
        m_onedcors[il][iea] *= prev;
        // m_onedcors[il][iea] /= 0.98;
      }
    }

    if (m_chargeType > 0)
      for (int ie = 0; ie < m_eaBin / 2; ie++) m_onedcors[il][ie] *= m_adjustFac;
    if (m_chargeType < 0)
      for (int ie = m_eaBin / 2; ie < m_eaBin; ie++) m_onedcors[il][ie] *= m_adjustFac;

  }
  //Saving constants
  B2INFO("dE/dx Calibration done for CDCDedx1DCell");
  CDCDedx1DCell* gain = new CDCDedx1DCell(0, m_onedcors);
  saveCalibration(gain, "CDCDedx1DCell");
}

//--------------------------------------------------
void CDCDedx1DCellAlgorithm::plotMergeFactor(std::map<int, std::vector<double>> bounds, const std::array<int, 2> nDev,
                                             std::map<int, std::vector<int>> steps)
{

  TCanvas cmfactor("cmfactor", "Merging factors", 800, 400);
  cmfactor.Divide(2, 1);

  for (int il = 0; il < 2; il++) {
    Double_t* nvarBins;
    nvarBins = &bounds[il][0];

    TH1I* hist  = new TH1I(Form("hist_%s", m_label[il].data()), "", nDev[il], nvarBins);
    hist->SetTitle(Form("Merging factor for %s bins;binindex;merge-factors", m_label[il].data()));

    for (int ibin = 0; ibin < nDev[il]; ibin++) hist->SetBinContent(ibin + 1, steps[il][ibin]);

    cmfactor.cd(il + 1);
    hist->SetFillColor(kYellow);
    hist->Draw("hist");
    delete hist;
  }

  cmfactor.SaveAs(Form("cdcdedx_1dcell_mergefactor%s.pdf", m_suffix.data()));
  cmfactor.SaveAs(Form("cdcdedx_1dcell_mergefactor%s.root", m_suffix.data()));
}

//--------------------------------------------------
void CDCDedx1DCellAlgorithm::plotdedxHist(std::vector<TH1D*> hdedxhit[2])
{

  TCanvas ctmp("tmp", "tmp", 1200, 1200);
  ctmp.Divide(4, 4);
  std::stringstream psname;

  psname << Form("cdcdedx_1dcell_dedxhit%s.pdf[", m_suffix.data());
  ctmp.Print(psname.str().c_str());
  psname.str("");
  psname << Form("cdcdedx_1dcell_dedxhit%s.pdf", m_suffix.data());

  for (int il = 0; il < 2; il++) {

    for (int jea = 0; jea < m_eaBinLocal[il]; jea++) {

      int minbin = std::stoi(hdedxhit[il][jea]->GetXaxis()->GetTitle());
      int maxbin = std::stoi(hdedxhit[il][jea]->GetYaxis()->GetTitle());

      ctmp.cd(jea % 16 + 1);
      hdedxhit[il][jea]->SetFillColor(4 + il);

      hdedxhit[il][jea]->SetTitle(Form("%s;dedxhit;entries", hdedxhit[il][jea]->GetTitle()));
      hdedxhit[il][jea]->DrawClone("hist");
      TH1D* htempC = (TH1D*)hdedxhit[il][jea]->Clone(Form("%sc2", hdedxhit[il][jea]->GetName()));
      htempC->GetXaxis()->SetRange(minbin, maxbin);
      htempC->SetFillColor(kGray);
      htempC->DrawClone("same hist");

      if (jea % 16 == 15 || (jea == m_eaBinLocal[il] - 1)) {
        ctmp.Print(psname.str().c_str());
        gPad->Clear("D");
        ctmp.Clear("D");
      }
      delete htempC;
    }
  }
  psname.str("");
  psname << Form("cdcdedx_1dcell_dedxhit%s.pdf]", m_suffix.data());
  ctmp.Print(psname.str().c_str());
}

//--------------------------------------------------
void CDCDedx1DCellAlgorithm::plotLayerDist(TH1D* hdedxlay[2])
{

  TCanvas cdedxlayer("layerdedxhit", "Inner and Outer Layer dedxhit dist", 900, 400);
  cdedxlayer.Divide(2, 1);

  for (int il = 0; il < 2; il++) {
    int minlay = 0, maxlay = 0;
    if (isFixTrunc) {
      minlay = std::stoi(hdedxlay[il]->GetXaxis()->GetTitle());
      maxlay = std::stoi(hdedxlay[il]->GetYaxis()->GetTitle());
      double lowedge = hdedxlay[il]->GetXaxis()->GetBinLowEdge(minlay);
      double upedge = hdedxlay[il]->GetXaxis()->GetBinUpEdge(maxlay);
      hdedxlay[il]->SetTitle(Form("%s, trunc #rightarrow: %0.02f - %0.02f;dedxhit;entries", hdedxlay[il]->GetTitle(), lowedge, upedge));
    }

    cdedxlayer.cd(il + 1);
    hdedxlay[il]->SetFillColor(kYellow);
    hdedxlay[il]->Draw("histo");

    if (isFixTrunc) {
      TH1D* hdedxlayC = (TH1D*)hdedxlay[il]->Clone(Form("hdedxlayC%d", il));
      hdedxlayC->GetXaxis()->SetRange(minlay, maxlay);
      hdedxlayC->SetFillColor(kAzure + 1);
      hdedxlayC->Draw("same histo");
    }
  }

  cdedxlayer.SaveAs(Form("cdcdedx_1dcell_dedxlay%s.pdf", m_suffix.data()));
  cdedxlayer.SaveAs(Form("cdcdedx_1dcell_dedxlay%s.root", m_suffix.data()));
}

//--------------------------------------------------
void CDCDedx1DCellAlgorithm::plotQaPars(TH1D* hentalay[2], TH2D* hptcosth)
{

  TCanvas ceadist("ceadist", "Enta distributions", 800, 400);
  ceadist.Divide(2, 1);

  for (int il = 0; il < 2; il++) {

    ceadist.cd(il + 1);
    gPad->SetLogy();
    hentalay[il]->SetFillColor(kYellow);
    hentalay[il]->Draw("hist");
  }

  TCanvas cptcos("cptcos", "pt vs costh dist.", 400, 400);
  cptcos.cd();
  hptcosth->Draw("colz");

  cptcos.SaveAs(Form("cdcdedx_ptcosth_%s.pdf", m_suffix.data()));
  ceadist.SaveAs(Form("cdcdedx_1dcell_enta%s.pdf", m_suffix.data()));
  ceadist.SaveAs(Form("cdcdedx_1dcell_enta%s.root", m_suffix.data()));
}

//--------------------------------------------------
void CDCDedx1DCellAlgorithm::plotRelConst(std::vector<double>tempconst, std::vector<double>layerconst, int il)
{

  TH1D* hconst, *hconstvar;

  Double_t* nvarBins;
  nvarBins = &m_binValue[il][0];

  hconst = new TH1D(Form("hconst%s", m_label[il].data()), "", m_eaBin, m_eaMin, m_eaMax);
  std::string title = Form("calibration const dist: %s: (%s); entaRS (#alpha); entries", m_label[il].data(), m_runExp.data());
  hconst->SetTitle(Form("%s", title.data()));

  hconstvar = new TH1D(Form("hconstvar%s", m_label[il].data()), "", m_eaBinLocal[il], nvarBins);
  title = Form("calibration const dist (var bins): %s: (%s); entaRS (#alpha);entries", m_label[il].data(), m_runExp.data());
  hconstvar->SetTitle(Form("%s", title.data()));

  if (isVarBins) {
    for (int iea = 0; iea < m_eaBinLocal[il]; iea++)
      hconstvar->SetBinContent(iea + 1, tempconst.at(iea));
  }

  for (int jea = 0; jea < m_eaBin; jea++) hconst->SetBinContent(jea + 1, layerconst.at(jea));

  gStyle->SetOptStat("ne");
  TCanvas cconst("cconst", "Calirbation Constants", 800, 400);
  if (isVarBins) {
    cconst.Divide(2, 1);
    cconst.SetWindowSize(1000, 800);
  }

  cconst.cd(1);
  hconst->SetFillColor(kYellow);
  hconst->Draw("histo");
  if (isVarBins) {
    cconst.cd(2);
    hconstvar->SetFillColor(kBlue);
    hconstvar->Draw("hist");
  }
  cconst.SaveAs(Form("cdcdedx_1dcell_relconst%s_%s.pdf", m_label[il].data(), m_suffix.data()));
  cconst.SaveAs(Form("cdcdedx_1dcell_relconst%s_%s.root", m_label[il].data(), m_suffix.data()));

  delete hconst;
  delete hconstvar;
}

//--------------------------------------------------
void CDCDedx1DCellAlgorithm::plotConstants()
{

  //Draw New/Old final constants
  TH1D* hnewconst[2], *holdconst[2];
  double min[2], max[2];

  for (unsigned int il = 0; il < 2; il++) {
    unsigned int nbins = m_DBOneDCell->getNBins(il);

    std::string title = Form("final calibration const dist (%s): %s; entaRS (#alpha); entries", m_label[il].data(), m_runExp.data());
    hnewconst[il] = new TH1D(Form("hnewconst_%s", m_label[il].data()), "", m_eaBin, m_eaMin, m_eaMax);
    hnewconst[il]->SetTitle(Form("%s", title.data()));

    title = Form("old calibration const dist (%s): %s; entaRS (#alpha); entries", m_label[il].data(), m_runExp.data());
    holdconst[il] = new TH1D(Form("holdconst_%s", m_label[il].data()), "", m_eaBin, m_eaMin, m_eaMax);
    holdconst[il]->SetTitle(Form("%s", title.data()));

    for (unsigned int iea = 0; iea < nbins; iea++) {
      double prev = m_DBOneDCell->getMean(8 * il + 1, iea);
      holdconst[il]->SetBinContent(iea + 1, prev);
      hnewconst[il]->SetBinContent(iea + 1, m_onedcors[il][iea]);
    }
    min[il] = hnewconst[il]->GetMinimum();
    max[il] = hnewconst[il]->GetMaximum();
  }

  //Ploting final constants
  if (max[1] < max[0])max[1] = max[0];
  if (min[1] > min[0])min[1] = min[0];

  gStyle->SetOptStat("ne");
  TCanvas cfconst("cfconst", "Final calirbation constants", 800, 400);
  cfconst.Divide(2, 1);

  for (int il = 0; il < 2; il++) {
    cfconst.cd(il + 1);
    hnewconst[il]->GetYaxis()->SetRangeUser(min[1] * 0.95, max[1] * 1.05);
    hnewconst[il]->SetLineColor(kBlack);
    hnewconst[il]->Draw("histo");
    holdconst[il]->SetLineColor(kRed);
    holdconst[il]->Draw("histo same");

    auto legend = new TLegend(0.4, 0.75, 0.56, 0.85);
    legend->AddEntry(holdconst[il], "Old", "lep");
    legend->AddEntry(hnewconst[il], "New", "lep");
    legend->Draw();
  }

  cfconst.SaveAs(Form("cdcdedx_1dcell_fconsts%s.pdf", m_suffix.data()));
  cfconst.SaveAs(Form("cdcdedx_1dcell_fconsts%s.root", m_suffix.data()));

  for (int il = 0; il < 2; il++) {
    delete hnewconst[il];
    delete holdconst[il];
  }
}

//------------------------------------
void CDCDedx1DCellAlgorithm::plotEventStats()
{

  TCanvas cstats("cstats", "cstats", 1000, 500);
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
    hestats->SetName(Form("htstats_%s", m_suffix.data()));
    htstats->DrawCopy("");
    hestats->SetStats(0);
  }
  cstats.Print(Form("cdcdedx_1dcell_stats_%s.pdf", m_suffix.data()));
}
