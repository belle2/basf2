/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <cdc/calibration/XTCalibrationAlgorithm.h>
#include <cdc/calibration/XTFunction.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dbobjects/CDCXtRelations.h>

#include <TError.h>
#include <TStopwatch.h>
#include <TROOT.h>
#include <TProfile.h>
#include <TF1.h>
#include <TFile.h>
#include <TTree.h>

#include <iostream>
#include <memory>

#include <framework/database/DBObjPtr.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

typedef std::array<float, 3> array3; /**< angle bin info. */
XTCalibrationAlgorithm::XTCalibrationAlgorithm() : CalibrationAlgorithm("CDCCalibrationCollector")
{
  setDescription(
    " -------------------------- XT Calibration Algorithm -------------------------\n"
  );
}

void XTCalibrationAlgorithm::createHisto()
{

  B2INFO("create and fill histo");
  /*Create histogram*/
  for (int i = 0; i < 56; ++i) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          m_hProf[i][lr][al][th] = new TProfile(Form("m_hProf%d_%d_%d_%d", i, lr, al, th),
                                                Form("(L=%d)-(lr=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                     i, lr, m_iAlpha[al], m_iTheta[th]), 210, -20, 600, 0, 1.2, "i");
          m_hist2d[i][lr][al][th] = new TH2F(Form("h%d_%d_%d_%d", i, lr, al, th),
                                             Form("(L=%d)-(lr=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                  i, lr, m_iAlpha[al], m_iTheta[th]), 210, -20, 600, 110, 0, 1.2);
          if (lr == 1)
            m_hist2dDraw[i][al][th] = new TH2F(Form("h_draw%d_%d_%d", i, al, th),
                                               Form("(L=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                    i, m_iAlpha[al], m_iTheta[th]), 210, -20, 600, 220, -1.2, 1.2);
        }
      }
    }
  }

  /* Read data and make histo*/

  auto tree = getObjectPtr<TTree>("tree");

  UChar_t lay;
  Float_t dt;
  Float_t dx;
  Float_t Pval, alpha, theta;
  Float_t ndf;

  tree->SetBranchAddress("lay", &lay);
  tree->SetBranchAddress("t", &dt);
  tree->SetBranchAddress("x_u", &dx);
  tree->SetBranchAddress("alpha", &alpha);
  tree->SetBranchAddress("theta", &theta);
  tree->SetBranchAddress("Pval", &Pval);
  tree->SetBranchAddress("ndf", &ndf);

  /* Disable unused branch */
  std::vector<TString> list_vars = {"lay", "t", "x_u", "alpha", "theta", "Pval", "ndf"};
  tree->SetBranchStatus("*", 0);

  for (TString brname : list_vars) {
    tree->SetBranchStatus(brname, 1);
  }


  int al = 0;
  int th = 0;
  TStopwatch time;
  time.Start();
  const Long64_t nEntries = tree->GetEntries();
  B2INFO("Number of entries " << nEntries);
  for (Long64_t i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    if (Pval < m_minPval || ndf < m_minNdf) continue;

    for (int k = 0; k < m_nAlphaBins; ++k) {
      if (alpha < m_upperAlpha[k]) {
        al = k;
        break;
      }
    }
    for (int j = 0; j < m_nThetaBins; ++j) {
      if (theta < m_upperTheta[j]) {
        th = j;
        break;
      }
    }
    int lr = dx > 0 ? c_Right : c_Left;
    if (m_LRseparate) {
      m_hProf[lay][lr][al][th]->Fill(dt, abs(dx));
      m_hist2d[lay][lr][al][th]->Fill(dt, abs(dx));
    } else {
      m_hProf[lay][0][al][th]->Fill(dt, abs(dx));
      m_hist2d[lay][0][al][th]->Fill(dt, abs(dx));
      m_hProf[lay][1][al][th]->Fill(dt, abs(dx));
      m_hist2d[lay][1][al][th]->Fill(dt, abs(dx));
    }
    m_hist2dDraw[lay][al][th]->Fill(dt, dx);
  }
  time.Stop();
  B2INFO("Time to fill histograms: " << time.RealTime() << "s");
  //  time.Print();
}

CalibrationAlgorithm::EResult XTCalibrationAlgorithm::calibrate()
{
  gROOT->SetBatch(1);
  gPrintViaErrorHandler = true; // Suppress huge log output from TMinuit
  gErrorIgnoreLevel = 3001;
  B2INFO("Start calibration");


  const auto exprun = getRunList()[0];
  B2INFO("ExpRun used for DB Geometry : " << exprun.first << " " << exprun.second);
  updateDBObjPtrs(1, exprun.second, exprun.first);
  B2INFO("Creating CDCGeometryPar object");
  CDC::CDCGeometryPar::Instance(&(*m_cdcGeo));

  prepare();
  createHisto();

  B2INFO("Start Fitting");
  std::unique_ptr<XTFunction> xt;
  for (int l = 0; l < 56; ++l) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          if (m_hist2d[l][lr][al][th]->GetEntries() < m_minEntriesRequired) {
            m_fitStatus[l][lr][al][th] = FitStatus::c_lowStat;
            continue;
          }
          double p0, p1, tmin;
          TF1* fpol1;
          if (m_useSliceFit) {
            m_hist2d[l][lr][al][th]->FitSlicesY(0, 0, -1, 5);
            m_hist2d_1[l][lr][al][th] = (TH1F*)gDirectory->Get(Form("h%d_%d_%d_%d_1", l, lr, al, th));
            if (!m_hist2d_1[l][lr][al][th]) {
              m_fitStatus[l][lr][al][th] = FitStatus::c_lowStat;
              B2WARNING("Error, not found results of slices fit");
              continue;
            }
            m_hist2d_1[l][lr][al][th]->Fit("pol1", "Q", "", 30, 60);
            fpol1 = (TF1*)m_hProf[l][lr][al][th]->GetFunction("pol1");
          } else {
            /*Set Error for low statistic bin*/
            for (int n = 0; n < m_hProf[l][lr][al][th]->GetNbinsX(); ++n) {
              if (m_hProf[l][lr][al][th]->GetBinEntries(n) < 5 && m_hProf[l][lr][al][th]->GetBinEntries(n) > 1) {
                m_hProf[l][lr][al][th]->SetBinError(n, 0.3 / m_hProf[l][lr][al][th]->GetBinEntries(n));
              }
            }
            m_hProf[l][lr][al][th]->Fit("pol1", "Q", "", 30, 60);
            fpol1 = (TF1*)m_hProf[l][lr][al][th]->GetFunction("pol1");
          }

          if (fpol1) {
            //determine tmin in fitting
            p0 = fpol1->GetParameter(0);
            p1 = fpol1->GetParameter(1);
            tmin = -1 * p0 / p1 + 15;
          } else {
            p0 = 0;
            p1 = 0.005;
            tmin = 12;
          }

          // B2INFO("layer " << l << ", lr " << lr << ", alpha "  << m_iAlpha[al] << ", theta " <<  m_iTheta[th]);
          if (m_useSliceFit) { // if slice fit results exist.
            xt.reset(new XTFunction(m_hist2d_1[l][lr][al][th], m_xtMode));
          } else { // from TProfile.
            xt.reset(new XTFunction(m_hProf[l][lr][al][th], m_xtMode));
          }

          if (m_bField) {
            int ial_old = 0;
            int ith_old = 0;
            for (int k = 0; k < m_nAlphaBins; ++k) {
              if (m_iAlpha[al] < m_upperAlpha[k]) {
                ial_old = k;
                break;
              }
            }
            for (int j = 0; j < m_nThetaBins; ++j) {
              if (m_iTheta[th] < m_upperTheta[j]) {
                ith_old = j;
                break;
              }
            }

            double p6 = m_xtPrior[l][lr][ial_old][ith_old][6];
            if (p6 > 400) {
              p6 = 400;
            }

            if (m_xtMode == m_xtModePrior) {
              xt->setXTParams(m_xtPrior[l][lr][ial_old][ith_old]);
              xt->setP6(p6);
            } else {
              xt->setXTParams(p0, p1, 0., 0., 0., 0., p6, m_xtPrior[l][lr][ial_old][ith_old][7]);
            }
            xt->setFitRange(tmin, p6 + 100);
          } else {
            xt->setXTParams(p0, p1, 0., 0., 0., 0., m_par6[l], 0.0001);
            xt->setFitRange(tmin, m_par6[l] + 100);
          }
          xt->setDebug(m_debug);
          xt->setBField(m_bField);
          xt->fitXT();
          if (xt->isValid() == false) {
            B2WARNING("Empty xt");
            m_fitStatus[l][lr][al][th] = c_fitFailure;
            continue;
          }
          if (xt->getFitStatus() != 1) {
            B2WARNING("Fit failed");
            m_fitStatus[l][lr][al][th] = c_fitFailure;
            continue;
          }
          if (xt->validate() == true) {
            m_fitStatus[l][lr][al][th] = xt->getFitStatus();
            m_xtFunc[l][lr][al][th] = (TF1*)xt->getXTFunction();

            if (m_useSliceFit) {
              m_hist2d_1[l][lr][al][th] = (TH1F*)xt->getFittedHisto();
            } else {
              m_hProf[l][lr][al][th] = (TProfile*)xt->getFittedHisto();
            }
          } else {
            m_fitStatus[l][lr][al][th] = c_fitFailure;
          }
        }
      }
    }
  }
  sanitaryCheck();
  write();
  storeHisto();
  return checkConvergence();
}

void XTCalibrationAlgorithm::sanitaryCheck()
{
  const double tMax = 500; // max drift time (nsec)
  for (int l = 0; l < 56; ++l) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          if (m_fitStatus[l][lr][al][th] == FitStatus::c_OK) {
            TF1* fun = m_xtFunc[l][lr][al][th];
            double y = fun->Eval(tMax);
            if (y < 0) {
              B2INFO("Strange XT function l " << l << " lr " << lr << " alpha " << al << " theta " << th
                     << ", replaced by initial one");
              fun->SetParameters(m_xtPrior[l][lr][al][th]);
            }
          }
        }
      }
    }
  }
}
CalibrationAlgorithm::EResult XTCalibrationAlgorithm::checkConvergence()
{

  const int nTotal = 56 * 2 * m_nAlphaBins * m_nThetaBins;
  int nFitCompleted = 0;
  for (int l = 0; l < 56; ++l) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          if (m_fitStatus[l][lr][al][th] == FitStatus::c_OK) {
            nFitCompleted++;
          }
        }
      }
    }
  }

  if (static_cast<double>(nFitCompleted) / nTotal < m_threshold) {
    B2WARNING("Less than " << m_threshold * 100 << " % of XTs were fitted.");
    return c_NotEnoughData;
  }
  return c_OK;
}

void XTCalibrationAlgorithm::prepare()
{
  B2INFO("Prepare calibration of XT");
  const double rad2deg = 180 / M_PI;

  DBObjPtr<CDCXtRelations> dbXT;
  m_nAlphaBins = dbXT->getNoOfAlphaBins();
  m_nThetaBins = dbXT->getNoOfThetaBins();
  for (unsigned short i = 0; i < m_nAlphaBins; ++i) {
    array3 alpha = dbXT->getAlphaBin(i);
    m_lowerAlpha[i] = alpha[0] * rad2deg;
    m_upperAlpha[i] = alpha[1] * rad2deg;
    m_iAlpha[i] = alpha[2] * rad2deg;
  }

  for (unsigned short i = 0; i < m_nThetaBins; ++i) {
    array3 theta = dbXT->getThetaBin(i);
    m_lowerTheta[i] = theta[0] * rad2deg;
    m_upperTheta[i] = theta[1] * rad2deg;
    m_iTheta[i] = theta[2] * rad2deg;
  }

  m_xtModePrior = dbXT->getXtParamMode();
  if (!(m_xtModePrior == c_Chebyshev || m_xtModePrior == c_Polynomial)) {
    B2FATAL("Function type before calibration is wrong " << m_xtModePrior);
  }

  B2INFO("Number of alpha bins " << m_nAlphaBins);
  B2INFO("Number of theta bins " << m_nThetaBins);
  B2INFO("Function type " << m_xtMode);

  for (unsigned short iCL = 0; iCL < 56; ++iCL) {
    for (unsigned short iLR = 0; iLR < 2; ++iLR) {
      for (unsigned short iA = 0; iA < m_nAlphaBins; ++iA) {
        for (unsigned short iT = 0; iT < m_nThetaBins; ++iT) {
          const std::vector<float> params = dbXT->getXtParams(iCL, iLR, iA, iT);
          unsigned short np = params.size();
          for (unsigned short i = 0; i < np; ++i) {
            m_xtPrior[iCL][iLR][iA][iT][i] = params[i];
          }
        }
      }
    }
  }
}

void XTCalibrationAlgorithm::write()
{
  B2INFO("write calibrated XT");
  double par[8];


  int nfitted = 0;
  int nfailure = 0;

  //
  // Save to the localDB
  //

  CDCXtRelations* xtRel = new CDCXtRelations();
  const float deg2rad = static_cast<float>(Unit::deg);

  for (int i = 0; i < m_nAlphaBins; ++i) {
    std::array<float, 3> alpha3 = {m_lowerAlpha[i]* deg2rad,
                                   m_upperAlpha[i]* deg2rad,
                                   m_iAlpha[i]* deg2rad
                                  };
    xtRel->setAlphaBin(alpha3);
  }

  for (int i = 0; i < m_nThetaBins; ++i) {
    std::array<float, 3> theta3 = {m_lowerTheta[i]* deg2rad,
                                   m_upperTheta[i]* deg2rad,
                                   m_iTheta[i]* deg2rad
                                  };
    xtRel->setThetaBin(theta3);
  }

  xtRel->setXtParamMode(m_xtMode);

  for (int th = 0; th < m_nThetaBins; ++th) {
    for (int al = 0; al < m_nAlphaBins; ++al) {
      for (int l = 0; l < 56; ++l) {
        for (int lr = 0; lr < 2; ++lr) {
          if (m_fitStatus[l][lr][al][th] != FitStatus::c_OK) {
            nfailure += 1;
            B2DEBUG(21, "fit failure status = " <<  m_fitStatus[l][lr][al][th]);
            B2DEBUG(21, "layer " << l << ", r " << lr << ", alpha "  << m_iAlpha[al] << ", theta " <<  m_iTheta[th]);
            B2DEBUG(21, "number of event: " <<  m_hProf[l][lr][al][th]->GetEntries());
            if (m_fitStatus[l][lr][al][th] != FitStatus::c_lowStat) {
              if (m_xtFunc[l][lr][al][th]) {
                B2DEBUG(21, "Probability of fit: " <<  m_xtFunc[l][lr][al][th]->GetProb());
              }
            }
            // If fit is failed
            // and mode of input xt (prior) is same as output, previous xt is used.
            if (m_xtMode == m_xtModePrior) {
              for (int i = 0; i < 8; ++i) {
                par[i] = m_xtPrior[l][lr][al][th][i];
              }
            } else {
              B2FATAL("XT mode before/after calibration is different!");
            }

          } else {
            if (par[1] < 0) { // if negative c1, privious xt is kept.
              for (int i = 0; i < 8; ++i) {
                par[i] = m_xtPrior[l][lr][al][th][i];
              }
            } else {
              m_xtFunc[l][lr][al][th]->GetParameters(par);
              nfitted += 1;
            }
          }
          std::vector<float> xtbuff;
          for (int i = 0; i < 8; ++i) {
            xtbuff.push_back(par[i]);
          }
          xtRel->setXtParams(l, lr, al, th, xtbuff);
        }//lr
      }//layer
    }//alpha
  }//theta

  if (m_textOutput == true) {
    xtRel->outputToFile(m_outputFileName);
  }

  saveCalibration(xtRel, "CDCXtRelations");

  B2RESULT("Total number of xt fit: " << m_nAlphaBins * m_nThetaBins * 2 * 56);
  B2RESULT("Successfully Fitted: " << nfitted);
  B2RESULT("Failure Fit: " << nfailure);

}

void XTCalibrationAlgorithm::storeHisto()
{

  auto hNDF =   getObjectPtr<TH1F>("hNDF");
  auto hPval =   getObjectPtr<TH1F>("hPval");
  auto hEvtT0 =   getObjectPtr<TH1F>("hEventT0");
  B2INFO("saving histograms");
  TFile* fout = new TFile(m_histName.c_str(), "RECREATE");
  TDirectory* top = gDirectory;
  //store NDF, P-val. EventT0 histogram for monitoring during calibration
  if (hNDF && hPval && hEvtT0) {
    hEvtT0->Write();
    hPval->Write();
    hNDF->Write();
  }
  // for each layer

  TDirectory* Direct[56];
  int nhisto = 0;
  for (int l = 0; l < 56; ++l) {
    top->cd();
    Direct[l] = gDirectory->mkdir(Form("lay_%d", l));
    Direct[l]->cd();
    for (int th = 0; th < m_nThetaBins; ++th) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        m_hist2dDraw[l][al][th]->Write();
        for (int lr = 0; lr < 2; ++lr) {
          m_hist2d[l][lr][al][th]->Write();
          if (m_fitStatus[l][lr][al][th] != 1) continue;
          if (m_useSliceFit) {
            if (m_hist2d_1[l][lr][al][th]) {
              m_hist2d_1[l][lr][al][th]->Write();
              nhisto += 1;
            }
          } else {
            m_hProf[l][lr][al][th]->Write();
            nhisto += 1;
          }
        }
      }
    }
  }
  top->cd();

  fout->Close();
  B2RESULT("  " << nhisto << " histograms was stored.");
}

