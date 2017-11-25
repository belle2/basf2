/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Makoto Uchida                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>
#include <iomanip>
#include <cdc/calibration/SpaceResolutionCalibrationAlgorithm.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCSpaceResols.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>

#include "TH1F.h"
#include "TH2F.h"
#include "TMultiGraph.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TChain.h"
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
  readProfile();
  const int np = floor(1 / m_binWidth);
  auto tree = getObjectPtr<TTree>("tree");

  int lay;
  double w;
  double x_u;
  double x_b;
  double x_mea;
  double Pval;
  double alpha;
  double theta;
  double ndf;
  double absRes_u;
  double absRes_b;
  tree->SetBranchAddress("lay", &lay);
  tree->SetBranchAddress("ndf", &ndf);
  tree->SetBranchAddress("Pval", &Pval);
  tree->SetBranchAddress("x_u", &x_u);
  tree->SetBranchAddress("x_b", &x_b);
  tree->SetBranchAddress("x_mea", &x_mea);
  tree->SetBranchAddress("weight", &w);
  tree->SetBranchAddress("alpha", &alpha);
  tree->SetBranchAddress("theta", &theta);

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
          hist_b[il][lr][al][th] = new TH2F(Form("hb_%d_%d_%d_%d", il, lr, al, th),
                                            Form("lay_%d_lr%d_al_%3.0f_th_%3.0f;Drift Length [cm];#DeltaX", il, lr, m_iAlpha[al], m_iTheta[th]),
                                            xbin.size() - 1, &xbin.at(0), yb.size() - 1, &yb.at(0));
          hist_u[il][lr][al][th] = new TH2F(Form("hu_%d_%d_%d_%d", il, lr, al, th),
                                            Form("lay_%d_lr%d_al_%3.0f_th_%3.0f;Drift Length [cm];#DeltaX", il, lr, m_iAlpha[al], m_iTheta[th]),
                                            xbin.size() - 1, &xbin.at(0), yu.size() - 1, &yu.at(0));
        }
      }
    }
  }


  const int nEntries = tree->GetEntries();
  B2INFO("Number of entries: " << nEntries);
  int ith = -99;
  int ial = -99;
  for (int i = 0; i < nEntries; ++i) {
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

    hist_u[lay][ilr][ial][ith]->Fill(fabs(x_u), absRes_u, w);
    hist_b[lay][ilr][ial][ith]->Fill(fabs(x_b), absRes_b, w);

  }


  B2INFO("Start to obtain the biase and unbiased sigmas");

  TF1* gb = new TF1("gb", "gaus", -0.05, 0.05);
  TF1* gu = new TF1("gu", "gaus", -0.06, 0.06);
  TF1* g0b = new TF1("g0b", "gaus", -0.015, 0.07);
  TF1* g0u = new TF1("g0u", "gaus", -0.015, 0.08);
  g0b->SetParLimits(1, -0.005, 0.004);
  g0u->SetParLimits(1, -0.005, 0.004);

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

          B2DEBUG(199, "layer-lr-al-th " << il << " - " << lr << " - " << al << " - " << th);
          if (hist_b[il][lr][al][th]->GetEntries() < 5000) {
            m_fitStatus[il][lr][al][th] = -1;
            continue;
          }
          B2DEBUG(199, "Nentries: " << hist_b[il][lr][al][th]->GetEntries());
          hist_b[il][lr][al][th]->SetDirectory(0);
          hist_u[il][lr][al][th]->SetDirectory(0);

          // With biased track fit result

          // Apply slice fit for the region near sense wire
          hist_b[il][lr][al][th]->FitSlicesY(g0b, firstbin, ib1, minEntry);

          // mean
          hb_m[il][lr][al][th] = (TH1F*)gDirectory->Get(Form("hb_%d_%d_%d_%d_1", il, lr, al, th))->Clone(Form("hb_%d_%d_%d_%d_m", il, lr, al,
                                 th));
          // sigma
          hb_s[il][lr][al][th] = (TH1F*)gDirectory->Get(Form("hb_%d_%d_%d_%d_2", il, lr, al, th))->Clone(Form("hb_%d_%d_%d_%d_s", il, lr, al,
                                 th));
          hb_m[il][lr][al][th]->SetDirectory(0);
          hb_s[il][lr][al][th]->SetDirectory(0);

          //Apply slice fit for other regions
          hist_b[il][lr][al][th]->FitSlicesY(gb, ib1 + 1, np, minEntry);
          // mean
          hb_m[il][lr][al][th]->Add((TH1F*)gDirectory->Get(Form("hb_%d_%d_%d_%d_1", il, lr, al, th)));
          //sigma
          hb_s[il][lr][al][th]->Add((TH1F*)gDirectory->Get(Form("hb_%d_%d_%d_%d_2", il, lr, al, th)));
          B2DEBUG(199, "entries (2nd): " << hb_s[il][lr][al][th]->GetEntries());

          // With unbiased track fit result

          // Apply slice fit for the region near sense wire
          hist_u[il][lr][al][th]->FitSlicesY(g0u, firstbin, ib1, minEntry);
          // mean
          hu_m[il][lr][al][th] = (TH1F*)gDirectory->Get(Form("hu_%d_%d_%d_%d_1", il, lr, al, th))->Clone(Form("hu_%d_%d_%d_%d_m", il, lr, al,
                                 th));
          // sigma
          hu_s[il][lr][al][th] = (TH1F*)gDirectory->Get(Form("hu_%d_%d_%d_%d_2", il, lr, al, th))->Clone(Form("hu_%d_%d_%d_%d_s", il, lr, al,
                                 th));
          hu_m[il][lr][al][th]->SetDirectory(0);
          hu_s[il][lr][al][th]->SetDirectory(0);


          //Apply slice fit for other regions
          hist_u[il][lr][al][th]->FitSlicesY(gu, ib1 + 1, np, minEntry);
          //mean
          hu_m[il][lr][al][th]->Add((TH1F*)gDirectory->Get(Form("hu_%d_%d_%d_%d_1", il, lr, al, th)));
          //sigma
          hu_s[il][lr][al][th]->Add((TH1F*)gDirectory->Get(Form("hu_%d_%d_%d_%d_2", il, lr, al, th)));
          if (!hu_s[il][lr][al][th] || !hb_s[il][lr][al][th]) {
            B2WARNING("slice histo do not found");
            m_fitStatus[il][lr][al][th] = -1;
            continue;
          }

          for (int j = 1; j < hu_s[il][lr][al][th]->GetNbinsX(); j++) {
            if (hu_s[il][lr][al][th]->GetBinContent(j) == 0) continue;
            if (hb_s[il][lr][al][th]->GetBinContent(j) == 0) continue;
            double sb = hb_s[il][lr][al][th]->GetBinContent(j);
            double su = hu_s[il][lr][al][th]->GetBinContent(j);

            double dsb = hb_s[il][lr][al][th]->GetBinError(j);
            double dsu = hu_s[il][lr][al][th]->GetBinError(j);
            double XL = hb_s[il][lr][al][th]->GetXaxis()->GetBinCenter(j);
            double dXL = (hb_s[il][lr][al][th]->GetXaxis()->GetBinWidth(j)) / 2;
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

          if (xl.size() < 8 || xl.size() > Max_np) {
            m_fitStatus[il][lr][al][th] = -1;
            B2WARNING("number of element might out of range"); continue;
          }

          //Intrinsic resolution
          B2DEBUG(199, "Create Histo for layer-lr: " << il << " " << lr);
          gr[il][lr][al][th] = new TGraphErrors(xl.size(), &xl.at(0), &sigma.at(0), &dxl.at(0), &dsigma.at(0));
          gr[il][lr][al][th]->SetMarkerSize(0.5);
          gr[il][lr][al][th]->SetMarkerStyle(8);
          gr[il][lr][al][th]->SetTitle(Form("Layer_%d lr%d | #alpha = %3.0f | #theta = %3.0f", il, lr, m_iAlpha[al], m_iTheta[th]));
          gr[il][lr][al][th]->SetName(Form("lay%d_lr%d_al%d_th%d", il, lr, al, th));

          //s2 for fitting
          gfit[il][lr][al][th] = new TGraphErrors(xl.size(), &xl.at(0), &s2.at(0), &dxl0.at(0), &ds2.at(0));
          gfit[il][lr][al][th]->SetMarkerSize(0.5);
          gfit[il][lr][al][th]->SetMarkerStyle(8);
          //          gfit[il][lr][al][th]->SetMarkerColor(1 + lr + al * 2 + th * 3);
          //gfit[il][lr][al][th]->SetLineColor(1 + lr + al * 2 + th * 3);
          gfit[il][lr][al][th]->SetTitle(Form("L%d-lr%d | #alpha = %3.0f | #theta = %3.0f ", il, lr, m_iAlpha[al], m_iTheta[th]));
          gfit[il][lr][al][th]->SetName(Form("sigma2_lay%d_lr%d_al%d_th%d", il, lr, al, th));

          xl.clear();
          dxl.clear();
          dxl0.clear();
          sigma.clear();
          dsigma.clear();
          s2.clear();
          ds2.clear();
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

  gROOT->SetBatch(1);
  gErrorIgnoreLevel = 3001;
  createHisto();

  //half cell size
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  double halfCSize[56];
  for (int i = 0; i < 56; ++i) {
    double R = cdcgeo.senseWireR(i);
    double nW = cdcgeo.nWiresInLayer(i);
    halfCSize[i] = M_PI * R / nW;
  }

  TF1* func = new TF1("func", "[0]/(x*x + [1])+[2]* x+[3]+[4]*exp([5]*(x-[6])*(x-[6]))", 0, 1.);
  TH1F* hprob = new TH1F("h1", "", 20, 0, 1);
  double upFit; /** Upper limit of fitting. */
  double intp6;

  for (int i = 0; i < 56; ++i) {

    if (i < 8) {
      upFit = halfCSize[i] + 0.1;
      intp6 = halfCSize[i] + 0.1;
    } else {
      upFit = halfCSize[i] - 0.07;
      intp6 = halfCSize[i];
    }

    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          //boundary parameters,
          if (m_bField) {
            if (i < 8) {
              upFit = halfCSize[i] + 0.15;
              intp6 = halfCSize[i] + 0.1;
            } else {
              if (fabs(m_iAlpha[al]) < 25) {
                upFit = halfCSize[i];
                intp6 = halfCSize[i];
              } else {
                upFit = halfCSize[i] + 0.2;
                intp6 = halfCSize[i] + 0.4 ;
              }
            }
            //no B case
          } else {
            if (i < 8) {
              upFit = halfCSize[i] + 0.1;
              intp6 = halfCSize[i] + 0.1;
            } else {
              upFit = halfCSize[i] - 0.07;
              intp6 = halfCSize[i];
            }
          }

          if (upFit > 0.9) upFit = 0.9;

          //          func->SetLineColor(1 + lr + al * 2 + th * 3);
          func->SetParameters(5E-6, 0.007, 1E-4, 1E-5, 0.00008, -30, intp6);
          func->SetParLimits(0, 1E-7, 1E-4);
          func->SetParLimits(1, 0.0045, 0.02);
          func->SetParLimits(2, 1E-6, 0.0005);
          func->SetParLimits(3, 1E-8, 0.0005);
          func->SetParLimits(4, 0., 0.001);
          func->SetParLimits(5, -40, 0.);
          func->SetParLimits(6, intp6 - 0.5, intp6 + 0.2);
          B2DEBUG(199, "FITTING for layer: " << i << "lr: " << lr << " ial" << al << " ith:" << th);
          B2DEBUG(199, "Fit flag before fit:" << m_fitStatus[i][lr][al][th]);
          if (!gfit[i][lr][al][th]) continue;
          if (m_fitStatus[i][lr][al][th] != -1) { /*if graph exist, do fitting*/

            for (int j = 0; j < 10; j++) {

              B2DEBUG(199, "loop: " << j);
              B2DEBUG(199, "Int p6: " << intp6);
              B2DEBUG(199, "Number of Point: " << gfit[i][lr][al][th]->GetN());
              Int_t stat = gfit[i][lr][al][th]->Fit("func", "MQE", "", 0.05, upFit);
              B2DEBUG(199, "stat of fit" << stat);
              std::string Fit_status = gMinuit->fCstatu.Data();
              B2DEBUG(199, "FIT STATUS: " << Fit_status);
              //      stat=gfit[i]->Fit(Form("ffit[%d]",i),"M "+Q,"",0.0,cellsize(i)+0.05+j*0.005);
              if (Fit_status == "OK" || Fit_status == "SUCCESSFUL") {//need to found better way
                if (fabs(func->Eval(0.3)) > 0.00035 || func->Eval(0.3) < 0) {
                  func->SetParameters(5E-6, 0.007, 1E-4, 1E-7, 0.0007, -30, intp6 + 0.05 * j);
                  //    func->SetParameters(defaultparsmall);
                  m_fitStatus[i][lr][al][th] = 0;
                } else {
                  B2DEBUG(199, "Prob of fit: " << func->GetProb());
                  m_fitStatus[i][lr][al][th] = 1;
                  break;
                }
              } else {
                m_fitStatus[i][lr][al][th] = 0;
                func->SetParameters(5E-6, 0.007, 1E-4, 1E-7, 0.0007, -30, intp6 + 0.05 * j);
                upFit += 0.025;
                if (j == 9) {
                  TCanvas* c1 =  new TCanvas("c1", "", 600, 600);
                  gfit[i][lr][al][th]->Draw();
                  c1->SaveAs(Form("Sigma_Fit_Error_%s_%d_%d_%d_%d.png", Fit_status.c_str(), i, lr, al, th));
                  B2WARNING("Fit error: " << i << " " << lr << " " << al << " " << th);
                }
              }
            }
            if (m_fitStatus[i][lr][al][th] == 1) {
              B2DEBUG(199, "ProbFit: Lay_lr_al_th: " << i << " " << lr << " " << al << " " << th << func->GetProb());
              hprob->Fill(func->GetProb());
              func->GetParameters(sigma_new[i][lr][al][th]);
            }
          }
        }
      }
    }
  }

  write();
  storeHisto();

  return c_OK;
}
void SpaceResolutionCalibrationAlgorithm::storeHisto()
{
  B2INFO("storeHisto");

  TFile*  ff = new TFile("sigma_histo.root", "RECREATE");
  TDirectory* top = gDirectory;
  TDirectory* Direct[56];

  for (int il = 0; il < 56; ++il) {
    top->cd();
    Direct[il] = gDirectory->mkdir(Form("lay_%d", il));
    Direct[il]->cd();

    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          if (!gr[il][lr][al][th]) continue;
          if (!gfit[il][lr][al][th]) continue;
          if (m_fitStatus[il][lr][al][th] == 1) {
            hist_b[il][lr][al][th]->Write();
            hist_u[il][lr][al][th]->Write();
            hb_m[il][lr][al][th]->Write();
            hb_s[il][lr][al][th]->Write();
            hu_m[il][lr][al][th]->Write();
            hu_s[il][lr][al][th]->Write();
            gr[il][lr][al][th]->Write();
            gfit[il][lr][al][th]->Write();
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
  B2INFO("write calibrated sigma");
  int nfitted = 0;
  int nfailure = 0;

  ofstream ofs(m_outputSigmaFileName.c_str());
  ofs << m_nAlphaBins << endl;
  for (int i = 0; i < m_nAlphaBins; ++i) {
    ofs << std::setprecision(4) << m_lowerAlpha[i] << "   " << std::setprecision(4) << m_upperAlpha[i] << "   " << std::setprecision(
          4) << m_iAlpha[i] << endl;
  }

  ofs << m_nThetaBins << endl;
  for (int i = 0; i < m_nThetaBins; ++i) {
    ofs << std::setprecision(4) << m_lowerTheta[i] << "   " << std::setprecision(4) << m_upperTheta[i] << "   " << std::setprecision(
          4) << m_iTheta[i] << endl;
  }

  ofs << 0 << "  " << 7 << endl; //mode and number of params;
  for (int al = 0; al < m_nAlphaBins; ++al) {
    for (int th = 0; th < m_nThetaBins; ++th) {
      for (int i = 0; i < 56; ++i) {
        for (int lr = 1; lr >= 0; --lr) {
          ofs << i << std::setw(4) << m_iTheta[th] << std::setw(4) << m_iAlpha[al] << std::setw(4) << lr << std::setw(15);
          if (m_fitStatus[i][lr][al][th] == 1) {
            nfitted += 1;
            for (int p = 0; p < 7; ++p) {
              if (p != 6) { ofs << std::setprecision(7) << sigma_new[i][lr][al][th][p] << std::setw(15);}
              if (p == 6) { ofs << std::setprecision(7) << sigma_new[i][lr][al][th][p] << std::endl;}
            }
          } else {
            B2FATAL("Fitting error and old sigma will be used. (Layer " << i << ") (lr = " << lr << ") (al = " << al << ") (th = " << th <<
                    ")");
          }
        }
      }
    }
  }
  ofs.close();
  B2RESULT("Number of histogram: " << 56 * 2 * m_nAlphaBins * m_nThetaBins);
  B2RESULT("Histos succesfully fitted: " << nfitted);
  B2RESULT("Histos fit failure: " << nfailure);

  //  CDCSpaceResols* dbSigma;
  //  saveCalibration(dbSigmal, "CDCSpaceResols");
}

void SpaceResolutionCalibrationAlgorithm::readProfile()
{
  B2INFO("read Profile");

  const double rad2deg = 180 / M_PI;

  DBObjPtr<CDCSpaceResols> dbSigma;
  DBStore::Instance().update();

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

  m_sigmaParamMode = dbSigma->getSigmaParamMode();
}
