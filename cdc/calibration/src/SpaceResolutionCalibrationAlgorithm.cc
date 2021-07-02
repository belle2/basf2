/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>
#include <cdc/calibration/SpaceResolutionCalibrationAlgorithm.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dbobjects/CDCSpaceResols.h>

#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>

#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TROOT.h"
#include "TError.h"
#include "TMinuit.h"

using namespace std;
using namespace Belle2;
using namespace CDC;

typedef std::array<float, 3> array3; /**< angle bin info. */
SpaceResolutionCalibrationAlgorithm::SpaceResolutionCalibrationAlgorithm() :
  CalibrationAlgorithm("CDCCalibrationCollector")
{
  setDescription(
    " -------------------------- Position Resolution Calibration Algorithm -------------------------\n"
  );
}
void SpaceResolutionCalibrationAlgorithm::createHisto()
{
  B2INFO("Creating histograms");
  const int np = floor(1 / m_binWidth);

  vector<double> yu;
  vector <double> yb;
  for (int i = 0; i < 50; ++i) {
    yb.push_back(-0.07 + i * (0.14 / 50));
  }
  for (int i = 0; i < 50; ++i) {
    yu.push_back(-0.08 + i * (0.16 / 50));
  }

  vector<double> xbin;
  xbin.push_back(0.);
  xbin.push_back(0.02);
  for (int i = 1; i < np; ++i) {
    xbin.push_back(i * m_binWidth);
  }

  for (int il = 0; il < 56; ++il) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          m_hBiased[il][lr][al][th] = new TH2F(Form("hb_%d_%d_%d_%d", il, lr, al, th),
                                               Form("lay_%d_lr%d_al_%3.0f_th_%3.0f;Drift Length [cm];#DeltaX", il, lr, m_iAlpha[al], m_iTheta[th]),
                                               xbin.size() - 1, &xbin.at(0), yb.size() - 1, &yb.at(0));
          m_hUnbiased[il][lr][al][th] = new TH2F(Form("hu_%d_%d_%d_%d", il, lr, al, th),
                                                 Form("lay_%d_lr%d_al_%3.0f_th_%3.0f;Drift Length [cm];#DeltaX", il, lr, m_iAlpha[al], m_iTheta[th]),
                                                 xbin.size() - 1, &xbin.at(0), yu.size() - 1, &yu.at(0));
        }
      }
    }
  }


  auto tree = getObjectPtr<TTree>("tree");

  UChar_t lay;
  Float_t w;
  Float_t x_u;
  Float_t x_b;
  Float_t x_mea;
  Float_t Pval;
  Float_t alpha;
  Float_t theta;
  Float_t ndf;
  Float_t absRes_u;
  Float_t absRes_b;
  tree->SetBranchAddress("lay", &lay);
  tree->SetBranchAddress("ndf", &ndf);
  tree->SetBranchAddress("Pval", &Pval);
  tree->SetBranchAddress("x_u", &x_u);
  tree->SetBranchAddress("x_b", &x_b);
  tree->SetBranchAddress("x_mea", &x_mea);
  tree->SetBranchAddress("weight", &w);
  tree->SetBranchAddress("alpha", &alpha);
  tree->SetBranchAddress("theta", &theta);

  /* Disable unused branch */
  std::vector<TString> list_vars = {"lay", "ndf", "Pval", "x_u", "x_b", "x_mea", "weight", "alpha", "theta"};
  tree->SetBranchStatus("*", 0);

  for (TString brname : list_vars) {
    tree->SetBranchStatus(brname, 1);
  }


  const Long64_t nEntries = tree->GetEntries();
  B2INFO("Number of entries: " << nEntries);
  int ith = -99;
  int ial = -99;
  for (Long64_t i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);
    if (std::fabs(x_b) < 0.02 || std::fabs(x_u) < 0.02) continue;
    if (Pval < m_minPval || ndf < m_minNdf) continue;

    for (int k = 0; k < m_nAlphaBins; ++k) {
      if (alpha < m_upperAlpha[k]) {
        ial = k;
        break;
      }
    }

    for (int j = 0; j < m_nThetaBins; ++j) {
      if (theta < m_upperTheta[j]) {
        ith = j;
        break;
      }
    }

    int ilr = x_u > 0 ? 1 : 0;

    if (ial == -99 || ith == -99) {
      TString command = Form("Error in alpha=%3.2f and theta = %3.2f>> error", alpha, theta);
      B2FATAL("ERROR" << command);
    }

    absRes_u = fabs(x_mea) - fabs(x_u);
    absRes_b = fabs(x_mea) - fabs(x_b);

    int ilay = static_cast<int>(lay);
    m_hUnbiased[ilay][ilr][ial][ith]->Fill(fabs(x_u), absRes_u, w);
    m_hBiased[ilay][ilr][ial][ith]->Fill(fabs(x_b), absRes_b, w);
  }


  B2INFO("Start to obtain the biased and unbiased sigmas");

  TF1* gb = new TF1("gb", "gausn", -0.05, 0.05);
  TF1* gu = new TF1("gu", "gausn", -0.06, 0.06);
  TF1* g0b = new TF1("g0b", "gausn", -0.015, 0.07);
  TF1* g0u = new TF1("g0u", "gausn", -0.015, 0.08);

  std::vector<double> sigma;
  std::vector<double> dsigma;
  std::vector<double> s2;
  std::vector<double> ds2;
  std::vector<double> xl;
  std::vector<double> dxl;
  std::vector<double> dxl0;

  ofstream ofss("IntReso.dat");
  const int ib1 = int(0.1 / m_binWidth) + 1;
  int firstbin = 1;
  int minEntry = 10;
  for (int il = 0; il < 56; ++il) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {

          B2DEBUG(21, "layer-lr-al-th " << il << " - " << lr << " - " << al << " - " << th);
          if (m_hBiased[il][lr][al][th]->GetEntries() < 5000) {
            m_fitStatus[il][lr][al][th] = -1;
            continue;
          }

          auto* proYb = m_hBiased[il][lr][al][th]->ProjectionY();
          auto* proYu = m_hUnbiased[il][lr][al][th]->ProjectionY();

          g0b->SetParLimits(0, 0, m_hBiased[il][lr][al][th]->GetEntries() * 5);
          g0u->SetParLimits(0, 0, m_hUnbiased[il][lr][al][th]->GetEntries() * 5);
          g0b->SetParLimits(1, -0.01, 0.004);
          g0u->SetParLimits(1, -0.01, 0.004);
          g0b->SetParLimits(2, 0.0, proYb->GetRMS() * 5);
          g0u->SetParLimits(2, 0.0, proYu->GetRMS() * 5);

          g0b->SetParameter(0, m_hBiased[il][lr][al][th]->GetEntries());
          g0u->SetParameter(0, m_hUnbiased[il][lr][al][th]->GetEntries());
          g0b->SetParameter(1, 0);
          g0u->SetParameter(1, 0);
          g0b->SetParameter(2, proYb->GetRMS());
          g0u->SetParameter(2, proYu->GetRMS());

          B2DEBUG(21, "Nentries: " << m_hBiased[il][lr][al][th]->GetEntries());
          m_hBiased[il][lr][al][th]->SetDirectory(0);
          m_hUnbiased[il][lr][al][th]->SetDirectory(0);

          // With biased track fit result

          // Apply slice fit for the region near sense wire
          m_hBiased[il][lr][al][th]->FitSlicesY(g0b, firstbin, ib1, minEntry);

          // mean
          m_hMeanBiased[il][lr][al][th] = (TH1F*)gDirectory->Get(Form("hb_%d_%d_%d_%d_1", il, lr, al, th))->Clone(Form("hb_%d_%d_%d_%d_m", il,
                                          lr, al,
                                          th));
          // sigma
          m_hSigmaBiased[il][lr][al][th] = (TH1F*)gDirectory->Get(Form("hb_%d_%d_%d_%d_2", il, lr, al, th))->Clone(Form("hb_%d_%d_%d_%d_s",
                                           il, lr, al,
                                           th));
          m_hMeanBiased[il][lr][al][th]->SetDirectory(0);
          m_hSigmaBiased[il][lr][al][th]->SetDirectory(0);

          //Apply slice fit for other regions
          m_hBiased[il][lr][al][th]->FitSlicesY(gb, ib1 + 1, np, minEntry);
          // mean
          m_hMeanBiased[il][lr][al][th]->Add((TH1F*)gDirectory->Get(Form("hb_%d_%d_%d_%d_1", il, lr, al, th)));
          //sigma
          m_hSigmaBiased[il][lr][al][th]->Add((TH1F*)gDirectory->Get(Form("hb_%d_%d_%d_%d_2", il, lr, al, th)));
          B2DEBUG(21, "entries (2nd): " << m_hSigmaBiased[il][lr][al][th]->GetEntries());

          // With unbiased track fit result

          // Apply slice fit for the region near sense wire
          m_hUnbiased[il][lr][al][th]->FitSlicesY(g0u, firstbin, ib1, minEntry);
          // mean
          m_hMeanUnbiased[il][lr][al][th] = (TH1F*)gDirectory->Get(Form("hu_%d_%d_%d_%d_1", il, lr, al, th))->Clone(Form("hu_%d_%d_%d_%d_m",
                                            il, lr, al,
                                            th));
          // sigma
          m_hSigmaUnbiased[il][lr][al][th] = (TH1F*)gDirectory->Get(Form("hu_%d_%d_%d_%d_2", il, lr, al, th))->Clone(Form("hu_%d_%d_%d_%d_s",
                                             il, lr, al,
                                             th));
          m_hMeanUnbiased[il][lr][al][th]->SetDirectory(0);
          m_hSigmaUnbiased[il][lr][al][th]->SetDirectory(0);


          //Apply slice fit for other regions
          m_hUnbiased[il][lr][al][th]->FitSlicesY(gu, ib1 + 1, np, minEntry);
          //mean
          m_hMeanUnbiased[il][lr][al][th]->Add((TH1F*)gDirectory->Get(Form("hu_%d_%d_%d_%d_1", il, lr, al, th)));
          //sigma
          m_hSigmaUnbiased[il][lr][al][th]->Add((TH1F*)gDirectory->Get(Form("hu_%d_%d_%d_%d_2", il, lr, al, th)));
          if (!m_hSigmaUnbiased[il][lr][al][th] || !m_hSigmaBiased[il][lr][al][th]) {
            B2WARNING("sliced histo  not found");
            m_fitStatus[il][lr][al][th] = -1;
            continue;
          }
          //clean up container before adding new values.
          xl.clear();
          dxl.clear();
          dxl0.clear();
          sigma.clear();
          dsigma.clear();
          s2.clear();
          ds2.clear();


          for (int j = 1; j < m_hSigmaUnbiased[il][lr][al][th]->GetNbinsX(); j++) {
            if (m_hSigmaUnbiased[il][lr][al][th]->GetBinContent(j) == 0) continue;
            if (m_hSigmaBiased[il][lr][al][th]->GetBinContent(j) == 0) continue;
            double sb = m_hSigmaBiased[il][lr][al][th]->GetBinContent(j);
            double su = m_hSigmaUnbiased[il][lr][al][th]->GetBinContent(j);

            double dsb = m_hSigmaBiased[il][lr][al][th]->GetBinError(j);
            double dsu = m_hSigmaUnbiased[il][lr][al][th]->GetBinError(j);
            double XL = m_hSigmaBiased[il][lr][al][th]->GetXaxis()->GetBinCenter(j);
            double dXL = (m_hSigmaBiased[il][lr][al][th]->GetXaxis()->GetBinWidth(j)) / 2;
            double  s_int = std::sqrt(sb * su);
            double  ds_int = 0.5 * s_int * (dsb / sb + dsu / su);
            if (ds_int > 0.02) continue;
            xl.push_back(XL);
            dxl.push_back(dXL);
            dxl0.push_back(0.);
            sigma.push_back(s_int);
            dsigma.push_back(ds_int);
            s2.push_back(s_int * s_int);
            ds2.push_back(2 * s_int * ds_int);
            ofss << il << "   " << lr << "  " << al << "  " << th << "  " << j << "   " << XL << "   " << dXL << "   " << s_int << "   " <<
                 ds_int << endl;
          }

          if (xl.size() < 7 || xl.size() > Max_np) {
            m_fitStatus[il][lr][al][th] = -1;
            B2WARNING("number of element might out of range"); continue;
          }

          //Intrinsic resolution
          B2DEBUG(21, "Create Histo for layer-lr: " << il << " " << lr);
          m_graph[il][lr][al][th] = new TGraphErrors(xl.size(), &xl.at(0), &sigma.at(0), &dxl.at(0), &dsigma.at(0));
          m_graph[il][lr][al][th]->SetMarkerSize(0.5);
          m_graph[il][lr][al][th]->SetMarkerStyle(8);
          m_graph[il][lr][al][th]->SetTitle(Form("Layer_%d lr%d #alpha = %3.0f #theta = %3.0f", il, lr, m_iAlpha[al], m_iTheta[th]));
          m_graph[il][lr][al][th]->SetName(Form("lay%d_lr%d_al%d_th%d", il, lr, al, th));

          //square of sigma for fitting
          m_gFit[il][lr][al][th] = new TGraphErrors(xl.size(), &xl.at(0), &s2.at(0), &dxl0.at(0), &ds2.at(0));
          m_gFit[il][lr][al][th]->SetMarkerSize(0.5);
          m_gFit[il][lr][al][th]->SetMarkerStyle(8);
          m_gFit[il][lr][al][th]->SetTitle(Form("L%d lr%d #alpha = %3.0f #theta = %3.0f ", il, lr, m_iAlpha[al], m_iTheta[th]));
          m_gFit[il][lr][al][th]->SetName(Form("sigma2_lay%d_lr%d_al%d_th%d", il, lr, al, th));

          gDirectory->Delete("hu_%d_%d_%d_%d_0");
        }
      }
    }
  }
  ofss.close();

}

CalibrationAlgorithm::EResult SpaceResolutionCalibrationAlgorithm::calibrate()
{

  B2INFO("Start calibration");
  gPrintViaErrorHandler = true; // Suppress huge log output from TMinuit
  gROOT->SetBatch(1);
  gErrorIgnoreLevel = 3001;

  const auto exprun = getRunList()[0];
  B2INFO("ExpRun used for DB Geometry : " << exprun.first << " " << exprun.second);
  updateDBObjPtrs(1, exprun.second, exprun.first);
  //  B2INFO("Creating CDCGeometryPar object");
  //  CDC::CDCGeometryPar::Instance(&(*m_cdcGeo));

  prepare();
  createHisto();

  TF1* func = new TF1("func", "[0]/(x*x + [1])+[2]* x+[3]+[4]*exp([5]*(x-[6])*(x-[6]))", 0, 1.);
  TH1F* hprob = new TH1F("h1", "", 20, 0, 1);
  double upFit; /** Upper limit of fitting. */
  double intp6;

  for (int i = 0; i < 56; ++i) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          if (!m_gFit[i][lr][al][th]) continue;
          if (m_fitStatus[i][lr][al][th] != -1) { /*if graph exist, do fitting*/
            upFit = getUpperBoundaryForFit(m_gFit[i][lr][al][th]);
            intp6 = upFit + 0.2;
            B2DEBUG(199, "xmax for fitting: " << upFit);

            func->SetParameters(5E-6, 0.007, 1E-4, 1E-5, 0.00008, -30, intp6);
            func->SetParLimits(0, 1E-7, 1E-4);
            func->SetParLimits(1, 0.0045, 0.02);
            func->SetParLimits(2, 1E-6, 0.0005);
            func->SetParLimits(3, 1E-8, 0.0005);
            func->SetParLimits(4, 0., 0.001);
            func->SetParLimits(5, -40, 0.);
            func->SetParLimits(6, intp6 - 0.5, intp6 + 0.2);

            B2DEBUG(21, "Fitting for layer: " << i << "lr: " << lr << " ial" << al << " ith:" << th);
            B2DEBUG(21, "Fit status before fit:" << m_fitStatus[i][lr][al][th]);

            for (int j = 0; j < 10; j++) {

              B2DEBUG(21, "loop: " << j);
              B2DEBUG(21, "Int p6: " << intp6);
              B2DEBUG(21, "Number of Point: " << m_gFit[i][lr][al][th]->GetN());
              Int_t stat = m_gFit[i][lr][al][th]->Fit("func", "MQE", "", 0.05, upFit);
              B2DEBUG(21, "stat of fit" << stat);
              std::string Fit_status = gMinuit->fCstatu.Data();
              B2DEBUG(21, "FIT STATUS: " << Fit_status);
              if (Fit_status == "OK" || Fit_status == "SUCCESSFUL" || Fit_status == "CALL LIMIT"
                  || Fit_status == "PROBLEMS") {//need to found better way
                if (fabs(func->Eval(0.3)) > 0.00035 || func->Eval(0.3) < 0) {
                  func->SetParameters(5E-6, 0.007, 1E-4, 1E-7, 0.0007, -30, intp6 + 0.05 * j);
                  func->SetParLimits(6, intp6 + 0.05 * j - 0.5, intp6 + 0.05 * j + 0.2);
                  //    func->SetParameters(defaultparsmall);
                  m_fitStatus[i][lr][al][th] = 0;
                } else {
                  B2DEBUG(21, "Prob of fit: " << func->GetProb());
                  m_fitStatus[i][lr][al][th] = 1;
                  break;
                }
              } else {
                m_fitStatus[i][lr][al][th] = 0;
                func->SetParameters(5E-6, 0.007, 1E-4, 1E-7, 0.0007, -30, intp6 + 0.05 * j);
                func->SetParLimits(6, intp6 + 0.05 * j - 0.5, intp6 + 0.05 * j + 0.2);
                upFit += 0.025;
                if (j == 9) {
                  // TCanvas* c1 =  new TCanvas("c1", "", 600, 600);
                  // m_gFit[i][lr][al][th]->Draw();
                  // c1->SaveAs(Form("Sigma_Fit_Error_%s_%d_%d_%d_%d.png", Fit_status.c_str(), i, lr, al, th));
                  // B2WARNING("Fit error: " << i << " " << lr << " " << al << " " << th);
                }
              }
            }
            if (m_fitStatus[i][lr][al][th] == 1) {
              B2DEBUG(21, "ProbFit: Lay_lr_al_th: " << i << " " << lr << " " << al << " " << th << func->GetProb());
              hprob->Fill(func->GetProb());
              func->GetParameters(m_sigma[i][lr][al][th]);
            }
          }
        }
      }
    }
  }

  write();
  storeHisto();

  const int nTotal = 56 * 2 * m_nAlphaBins * m_nThetaBins;
  int nFitCompleted = 0;
  for (int l = 0; l < 56; ++l) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          if (m_fitStatus[l][lr][al][th] == 1) {
            nFitCompleted += 1;
          }
        }
      }
    }
  }

  if (static_cast<double>(nFitCompleted) / nTotal < m_threshold) {
    B2WARNING("Less than " << m_threshold * 100 << " % of Sigmas were fitted.");
    return c_NotEnoughData;
  }

  return c_OK;
}

void SpaceResolutionCalibrationAlgorithm::storeHisto()
{
  B2INFO("saving histograms");

  TFile*  ff = new TFile(m_histName.c_str(), "RECREATE");
  TDirectory* top = gDirectory;
  TDirectory* Direct[56];

  auto hNDF =   getObjectPtr<TH1F>("hNDF");
  auto hPval =   getObjectPtr<TH1F>("hPval");
  auto hEvtT0 =   getObjectPtr<TH1F>("hEventT0");
  //store NDF, P-val. EventT0 histogram for monitoring during calibration
  if (hNDF && hPval && hEvtT0) {
    hEvtT0->Write();
    hPval->Write();
    hNDF->Write();
  }


  for (int il = 0; il < 56; ++il) {
    top->cd();
    Direct[il] = gDirectory->mkdir(Form("lay_%d", il));
    Direct[il]->cd();

    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          if (!m_graph[il][lr][al][th]) continue;
          if (!m_gFit[il][lr][al][th]) continue;
          if (m_fitStatus[il][lr][al][th] == 1) {
            m_hBiased[il][lr][al][th]->Write();
            m_hUnbiased[il][lr][al][th]->Write();
            m_hMeanBiased[il][lr][al][th]->Write();
            m_hSigmaBiased[il][lr][al][th]->Write();
            m_hMeanUnbiased[il][lr][al][th]->Write();
            m_hSigmaUnbiased[il][lr][al][th]->Write();
            m_graph[il][lr][al][th]->Write();
            m_gFit[il][lr][al][th]->Write();
          }
        }
      }
    }
  }
  ff->Close();
  B2INFO("Finish store histogram");

}
void SpaceResolutionCalibrationAlgorithm::write()
{
  B2INFO("Writing calibrated sigma's");
  int nfitted = 0;
  int nfailure = 0;

  CDCSpaceResols* dbSigma = new CDCSpaceResols();

  const float deg2rad = M_PI / 180.0;

  for (unsigned short i = 0; i < m_nAlphaBins; ++i) {
    std::array<float, 3> alpha3 = {m_lowerAlpha[i]* deg2rad,
                                   m_upperAlpha[i]* deg2rad,
                                   m_iAlpha[i]*   deg2rad
                                  };
    dbSigma->setAlphaBin(alpha3);
  }


  for (unsigned short i = 0; i < m_nThetaBins; ++i) {
    std::array<float, 3> theta3 = {m_lowerTheta[i]* deg2rad,
                                   m_upperTheta[i]* deg2rad,
                                   m_iTheta[i]* deg2rad
                                  };
    dbSigma->setThetaBin(theta3);
  }

  dbSigma->setSigmaParamMode(m_sigmaParamMode);
  for (int ialpha = 0; ialpha < m_nAlphaBins; ++ialpha) {
    for (int itheta = 0; itheta < m_nThetaBins; ++itheta) {
      for (int iCL = 0; iCL < 56; ++iCL) {
        for (int iLR = 1; iLR >= 0; --iLR) {
          std::vector<float> sgbuff;
          if (m_fitStatus[iCL][iLR][ialpha][itheta] == 1) {
            nfitted += 1;  // inclement number of successfully fitted sigma's
            for (int i = 0; i < 7; ++i) {
              sgbuff.push_back(m_sigma[iCL][iLR][ialpha][itheta][i]);
            }
          } else {
            //B2WARNING("Fitting error and old sigma will be used. (Layer " << iCL << ") (lr = " << iLR <<
            //                      ") (al = " << ialpha << ") (th = " << itheta << ")");
            nfailure += 1; // inclement number of fit failed sigma's
            for (int i = 0; i < 7; ++i) {
              sgbuff.push_back(m_sigmaPost[iCL][iLR][ialpha][itheta][i]);
            }
          }
          dbSigma->setSigmaParams(iCL, iLR, ialpha, itheta, sgbuff);
        }
      }
    }
  }

  if (m_textOutput == true) {
    dbSigma->outputToFile(m_outputFileName);
  }

  saveCalibration(dbSigma, "CDCSpaceResols");

  B2RESULT("Number of histogram: " << 56 * 2 * m_nAlphaBins * m_nThetaBins);
  B2RESULT("Histos succesfully fitted: " << nfitted);
  B2RESULT("Histos fit failure: " << nfailure);


}

void SpaceResolutionCalibrationAlgorithm::prepare()
{
  B2INFO("Prepare calibration of space resolution");

  const double rad2deg = 180 / M_PI;

  DBObjPtr<CDCSpaceResols> dbSigma;

  m_nAlphaBins = dbSigma->getNoOfAlphaBins();
  m_nThetaBins = dbSigma->getNoOfThetaBins();

  B2INFO("Number of alpha bins: " << m_nAlphaBins);
  for (int i = 0; i < m_nAlphaBins; ++i) {
    array3 alpha = dbSigma->getAlphaBin(i);
    m_lowerAlpha[i] = alpha[0] * rad2deg;
    m_upperAlpha[i] = alpha[1] * rad2deg;
    m_iAlpha[i] = alpha[2] * rad2deg;
  }

  B2INFO("Number of theta bins: " << m_nThetaBins);
  for (int i = 0; i < m_nThetaBins; ++i) {
    array3 theta = dbSigma->getThetaBin(i);
    m_lowerTheta[i] = theta[0] * rad2deg;
    m_upperTheta[i] = theta[1] * rad2deg;
    m_iTheta[i] = theta[2] * rad2deg;
  }
  m_sigmaParamModePost = dbSigma->getSigmaParamMode();

  for (unsigned short iCL = 0; iCL < 56; ++iCL) {
    for (unsigned short iLR = 0; iLR < 2; ++iLR) {
      for (unsigned short iA = 0; iA < m_nAlphaBins; ++iA) {
        for (unsigned short iT = 0; iT < m_nThetaBins; ++iT) {
          const std::vector<float> params = dbSigma->getSigmaParams(iCL, iLR, iA, iT);
          unsigned short np = params.size();
          //    std::cout <<"np4sigma= " << np << std::endl;
          for (unsigned short i = 0; i < np; ++i) {
            m_sigmaPost[iCL][iLR][iA][iT][i] = params[i];
          }
        }
      }
    }
  }
}
