#include <cdc/calibration/XTCalibrationAlgorithm.h>
#include <cdc/calibration/XTFunction.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCXtRelations.h>

#include <TError.h>
#include <TROOT.h>
#include <TH1D.h>
#include <TGraphErrors.h>
#include <TProfile.h>
#include <TF1.h>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
#include <TSystem.h>
#include <iostream>
#include <iomanip>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>


using namespace std;
using namespace Belle2;
using namespace CDC;

typedef std::array<float, 3> array3; /**< angle bin info. */
XTCalibrationAlgorithm::XTCalibrationAlgorithm() :  CalibrationAlgorithm("CDCCalibrationCollector")
{
  setDescription(
    " -------------------------- XT Calibration Algoritm -------------------------\n"
  );
}

void XTCalibrationAlgorithm::createHisto()
{
  B2INFO("creating and filling histo");
  readXTProfile();

  auto tree = getObjectPtr<TTree>("tree");

  int lay, trighit;
  double dt;
  double dx;
  double Pval, alpha, theta;
  double ndf;

  tree->SetBranchAddress("lay", &lay);
  tree->SetBranchAddress("t", &dt);
  tree->SetBranchAddress("x_u", &dx);
  tree->SetBranchAddress("alpha", &alpha);
  tree->SetBranchAddress("theta", &theta);
  tree->SetBranchAddress("Pval", &Pval);
  tree->SetBranchAddress("trighit", &trighit);
  tree->SetBranchAddress("ndf", &ndf);

  /*Create histogram*/
  for (int i = 0; i < 56; ++i) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          hprof[i][lr][al][th] = new TProfile(Form("hprof%d_%d_%d_%d", i, lr, al, th),
                                              Form("(L=%d)-(lr=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                   i, lr, m_iAlpha[al], m_iTheta[th]), 210, -20, 600, 0, 1.2, "i");
          hist2d[i][lr][al][th] = new TH2D(Form("h%d_%d_%d_%d", i, lr, al, th),
                                           Form("(L=%d)-(lr=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                i, lr, m_iAlpha[al], m_iTheta[th]), 210, -20, 600, 110, 0, 1.2);
          if (lr == 1)
            hist2d_draw[i][al][th] = new TH2D(Form("h_draw%d_%d_%d", i, al, th),
                                              Form("(L=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                   i, m_iAlpha[al], m_iTheta[th]), 210, -20, 600, 2200, -1.2, 1.2);
        }
      }
    }
  }

  /* Read data and make histo*/
  int al = 0;
  int th = 0;
  Long64_t nbytes = 0;
  const int nEntries = tree->GetEntries();
  B2INFO("Number of events: " << nEntries);

  for (int i = 0; i < nEntries; ++i) {
    nbytes += tree->GetEntry(i);
    /* protect in case |alpha|>90 */
    if (fabs(alpha > 90)) {
      if (alpha < 0) alpha += 180;
      if (alpha > 0) alpha -= 180;
    }

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
      hprof[lay][lr][al][th]->Fill(dt, abs(dx));
      hist2d[lay][lr][al][th]->Fill(dt, abs(dx));
    } else {
      hprof[lay][0][al][th]->Fill(dt, abs(dx));
      hist2d[lay][0][al][th]->Fill(dt, abs(dx));
      hprof[lay][1][al][th]->Fill(dt, abs(dx));
      hist2d[lay][1][al][th]->Fill(dt, abs(dx));
    }
    hist2d_draw[lay][al][th]->Fill(dt, dx);
  }
}

CalibrationAlgorithm::EResult XTCalibrationAlgorithm::calibrate()
{
  gROOT->SetBatch(1);
  gErrorIgnoreLevel = 3001;
  createHisto();
  B2INFO("Start calibration");

  B2INFO("Start Fitting");
  for (int l = 0; l < 56; ++l) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          if (hist2d[l][lr][al][th]->GetEntries() < m_minEntriesRequired) {
            m_fitStatus[l][lr][al][th] = -1;
            continue;
          }
          double p0, p1, tmin;
          TF1* fpol1;
          if (m_useSliceFit) {
            hist2d[l][lr][al][th]->FitSlicesY(0, 0, -1, 5);
            hist2d_1[l][lr][al][th] = (TH1D*)gDirectory->Get(Form("h%d_%d_%d_%d_1", l, lr, al, th));
            if (!hist2d_1[l][lr][al][th]) {
              m_fitStatus[l][lr][al][th] = -1;
              B2WARNING("Error, not found results of slices fit");
              continue;
            }
            hist2d_1[l][lr][al][th]->Fit("pol1", "Q", "", 30, 60);
            fpol1 = (TF1*)hprof[l][lr][al][th]->GetFunction("pol1");
          } else {
            /*Set Error for low statistic bin*/
            for (int n = 0; n < hprof[l][lr][al][th]->GetNbinsX(); ++n) {
              if (hprof[l][lr][al][th]->GetBinEntries(n) < 5 && hprof[l][lr][al][th]->GetBinEntries(n) > 1) {
                hprof[l][lr][al][th]->SetBinError(n, 0.3 / hprof[l][lr][al][th]->GetBinEntries(n));
              }
            }
            hprof[l][lr][al][th]->Fit("pol1", "Q", "", 30, 60);
            fpol1 = (TF1*)hprof[l][lr][al][th]->GetFunction("pol1");
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


          XTFunction* xt;
          if (m_useSliceFit) { // if slice fit results exist.
            xt = new XTFunction(hist2d_1[l][lr][al][th], m_xtMode);
          } else { // from TProfile.
            xt = new XTFunction(hprof[l][lr][al][th], m_xtMode);
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
            double p6 = m_xtOld[l][lr][ial_old][ith_old][6];
            if (p6 > 400) {
              p6 = 400;
            }
            xt->setXTParams(m_xtOld[l][lr][ial_old][ith_old]);
            xt->setP6(p6);
            xt->setFitRange(tmin, p6 + 100);
          } else {
            xt->setXTParams(p0, p1, 0., 0., 0., 0., m_par6[l], 0.0001);
            xt->setFitRange(tmin, m_par6[l] + 100);
          }
          xt->setDebug(m_debug);
          xt->setBField(m_bField);
          xt->fitXT();
          m_fitStatus[l][lr][al][th] = xt->getFitStatus();
          m_xtFunc[l][lr][al][th] = (TF1*)xt->getXTFunction();
          if (m_useSliceFit) {
            hist2d_1[l][lr][al][th] = (TH1D*)xt->getFittedHisto();
          } else {
            hprof[l][lr][al][th] = (TProfile*)xt->getFittedHisto();
          }
        }
      }
    }
  }
  write();
  storeHisto();
  return c_OK;
}
void XTCalibrationAlgorithm::write()
{
  B2INFO("write calibrated XT");
  double par[8];
  ofstream xtout(m_outputXTFileName.c_str());
  xtout << m_nAlphaBins << endl;
  for (int i = 0; i < m_nAlphaBins; ++i) {
    xtout << std::setprecision(3) << m_lowerAlpha[i] << "  "
          << std::setprecision(3) << m_upperAlpha[i] << "  "
          << std::setprecision(3) << m_iAlpha[i] << endl;
  }
  xtout << m_nThetaBins << endl;
  for (int i = 0; i < m_nThetaBins; ++i) {
    xtout << std::setprecision(3) << m_lowerTheta[i] << "  "
          << std::setprecision(3) << m_upperTheta[i] << "  "
          << std::setprecision(3) << m_iTheta[i]  << endl;
  }
  xtout << m_xtMode << "  " << 8 << endl;

  int nfitted = 0;
  int nfailure = 0;

  for (int th = 0; th < m_nThetaBins; ++th) {
    for (int al = 0; al < m_nAlphaBins; ++al) {
      for (int l = 0; l < 56; ++l) {
        for (int lr = 0; lr < 2; ++lr) {
          /*Set Parameter for bad fit*/
          if (m_fitStatus[l][lr][al][th] != 1) {
            nfailure += 1;
            printf("fit failure status = %d \n", m_fitStatus[l][lr][al][th]);
            printf("layer %d, r %d, alpha %3.1f, theta %3.1f \n", l, lr, m_iAlpha[al], m_iTheta[th]);
            printf("number of event: %3.2f \n", hprof[l][lr][al][th]->GetEntries());
            if (m_fitStatus[l][lr][al][th] != -1) {
              printf("Probability of fit: %3.4f \n", m_xtFunc[l][lr][al][th]->GetProb());
            }
            par[0] = 0; par[1] = 0.004; par[2] = 0; par[3] = 0; par[4] = 0; par[5] = 0; par[6] = m_par6[l]; par[7] = 0.00001;
          } else {
            m_xtFunc[l][lr][al][th]->GetParameters(par);
            nfitted += 1;
          }
          /*Write params*/
          xtout << l << std::setw(5) << m_iTheta[th] << std::setw(5) << m_iAlpha[al] << std::setw(5) << "0.0" << std::setw(
                  4) << lr << std::setw(
                  15);
          for (int p = 0; p < 8; ++p) {
            if (p != 7) { xtout << std::setprecision(7) << par[p] << std::setw(15);}
            if (p == 7) { xtout << std::setprecision(7) << par[p] << std::endl;}
          }
        }//lr
      }//layer
    }//alpha
  }//theta
  xtout.close();


  //
  // Save to the localDB
  //

  CDCXtRelations* xtRel = new CDCXtRelations();
  const float deg2rad = M_PI / 180.0;

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
          if (m_fitStatus[l][lr][al][th] != 1) {
            par[0] = 0; par[1] = 0.004; par[2] = 0; par[3] = 0; par[4] = 0; par[5] = 0; par[6] = m_par6[l]; par[7] = 0.00001;
          } else {
            m_xtFunc[l][lr][al][th]->GetParameters(par);
          }
          std::vector<float> xtbuff;
          for (int i = 0; i < 8; ++i) {
            xtbuff.push_back(par[i]);
          }
          xtRel->setXtParams(l, lr, al * deg2rad, th * deg2rad, xtbuff);
        }//lr
      }//layer
    }//alpha
  }//theta

  saveCalibration(xtRel, "CDCXtRelations");

  B2RESULT("Total number of xt fit: " << m_nAlphaBins * m_nThetaBins * 2 * 56);
  B2RESULT("Successfully Fitted: " << nfitted);
  B2RESULT("Failure Fit: " << nfailure);

}

void XTCalibrationAlgorithm::storeHisto()
{
  B2INFO("saving histograms");
  TFile* fout = new TFile("XTFIT.root", "RECREATE");
  TDirectory* top = gDirectory;
  TDirectory* Direct[56];
  int nhisto = 0;
  for (int l = 0; l < 56; ++l) {
    top->cd();
    Direct[l] = gDirectory->mkdir(Form("lay_%d", l));
    Direct[l]->cd();
    for (int th = 0; th < m_nThetaBins; ++th) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        hist2d_draw[l][al][th]->Write();
        for (int lr = 0; lr < 2; ++lr) {
          hist2d[l][lr][al][th]->Write();
          if (m_fitStatus[l][lr][al][th] != 1) continue;
          if (m_useSliceFit) {
            if (hist2d_1[l][lr][al][th]) {
              hist2d_1[l][lr][al][th]->Write();
              nhisto += 1;
            }
          } else {
            hprof[l][lr][al][th]->Write();
            nhisto += 1;
          }
        }
      }
    }
  }
  fout->Close();
  B2RESULT("  " << nhisto << " histograms was stored.");
}

void XTCalibrationAlgorithm::readXTProfile()
{
  B2INFO("readXTProfile");
  const double rad2deg = 180 / M_PI;

  DBObjPtr<CDCXtRelations> dbXT;
  DBStore::Instance().update();


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
  m_xtMode = dbXT->getXtParamMode();

  B2INFO("Number of alpha bins " << m_nAlphaBins);
  B2INFO("Number of theta bins " << m_nThetaBins);
}

