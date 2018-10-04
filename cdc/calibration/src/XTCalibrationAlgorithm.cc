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
          m_hist2d[i][lr][al][th] = new TH2D(Form("h%d_%d_%d_%d", i, lr, al, th),
                                             Form("(L=%d)-(lr=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                  i, lr, m_iAlpha[al], m_iTheta[th]), 210, -20, 600, 110, 0, 1.2);
          if (lr == 1)
            m_hist2dDraw[i][al][th] = new TH2D(Form("h_draw%d_%d_%d", i, al, th),
                                               Form("(L=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                    i, m_iAlpha[al], m_iTheta[th]), 210, -20, 600, 2200, -1.2, 1.2);
        }
      }
    }
  }

  /* Read data and make histo*/

  auto tree = getObjectPtr<TTree>("tree");

  int lay;
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
  tree->SetBranchAddress("ndf", &ndf);

  int al = 0;
  int th = 0;
  const int nEntries = tree->GetEntries();
  B2INFO("Number of entries " << nEntries);
  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);
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
}

CalibrationAlgorithm::EResult XTCalibrationAlgorithm::calibrate()
{
  gROOT->SetBatch(1);
  gErrorIgnoreLevel = 3001;
  B2INFO("Start calibration");

  StoreObjPtr<EventMetaData> evtPtr;
  if (!evtPtr.isValid()) {
    // Construct an EventMetaData object in the Datastore so that the DB objects in CDCGeometryPar can work
    DataStore::Instance().setInitializeActive(true);
    B2INFO("Registering EventMetaData object in DataStore");
    evtPtr.registerInDataStore();
    DataStore::Instance().setInitializeActive(false);
    B2INFO("Creating EventMetaData object");
    const auto exprun = getRunList()[0];
    evtPtr.construct(1,  exprun.second, exprun.first);

    //    evtPtr.create();
  } else {
    B2INFO("A valid EventMetaData object already exists.");
  }
  DBObjPtr<CDCGeometry> cdcGeometry;
  CDC::CDCGeometryPar::Instance(&(*cdcGeometry));
  B2INFO("ExpRun at init : " << evtPtr->getExperiment() << " " << evtPtr->getRun());

  prepare(evtPtr);
  createHisto();


  B2INFO("Start Fitting");
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
            m_hist2d_1[l][lr][al][th] = (TH1D*)gDirectory->Get(Form("h%d_%d_%d_%d_1", l, lr, al, th));
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


          XTFunction* xt;
          if (m_useSliceFit) { // if slice fit results exist.
            xt = new XTFunction(m_hist2d_1[l][lr][al][th], m_xtMode);
          } else { // from TProfile.
            xt = new XTFunction(m_hProf[l][lr][al][th], m_xtMode);
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

            double p6 = m_xtPost[l][lr][ial_old][ith_old][6];
            if (p6 > 400) {
              p6 = 400;
            }

            if (m_xtMode == m_xtModePost) {
              xt->setXTParams(m_xtPost[l][lr][ial_old][ith_old]);
              xt->setP6(p6);
            } else {
              xt->setXTParams(p0, p1, 0., 0., 0., 0., p6, m_xtPost[l][lr][ial_old][ith_old][7]);
            }
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
            m_hist2d_1[l][lr][al][th] = (TH1D*)xt->getFittedHisto();
          } else {
            m_hProf[l][lr][al][th] = (TProfile*)xt->getFittedHisto();
          }
        }
      }
    }
  }

  write();
  storeHisto();
  for (int l = 0; l < 56; ++l) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nAlphaBins; ++al) {
        for (int th = 0; th < m_nThetaBins; ++th) {
          if (m_fitStatus[l][lr][al][th] != FitStatus::c_OK) {
            return c_Iterate;
          }
        }
      }
    }
  }

  return c_OK;
}

void XTCalibrationAlgorithm::prepare(StoreObjPtr<EventMetaData>& evtPtr)
{
  B2INFO("Prepare calibration of XT");
  const double rad2deg = 180 / M_PI;

  const auto exprun =  getRunList();
  B2INFO("Changed ExpRun to: " << exprun[0].first << " " << exprun[0].second);
  evtPtr->setExperiment(exprun[0].first);
  evtPtr->setRun(exprun[0].second);
  DBStore::Instance().update();

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
  m_xtModePost = dbXT->getXtParamMode();
  if (!(m_xtModePost == c_Chebyshev || m_xtModePost == c_Polynomial)) {
    B2FATAL("Function type before calibration is wrong " << m_xtModePost);
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
            m_xtPost[iCL][iLR][iA][iT][i] = params[i];
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
          if (m_fitStatus[l][lr][al][th] != FitStatus::c_OK) {
            nfailure += 1;
            printf("fit failure status = %d \n", m_fitStatus[l][lr][al][th]);
            printf("layer %d, r %d, alpha %3.1f, theta %3.1f \n", l, lr, m_iAlpha[al], m_iTheta[th]);
            printf("number of event: %3.2f \n", m_hProf[l][lr][al][th]->GetEntries());
            if (m_fitStatus[l][lr][al][th] != FitStatus::c_lowStat) {
              printf("Probability of fit: %3.4f \n", m_xtFunc[l][lr][al][th]->GetProb());
            }
            par[0] = 0; par[1] = 0.004; par[2] = 0; par[3] = 0; par[4] = 0; par[5] = 0; par[6] = m_par6[l]; par[7] = 0.00001;
          } else {
            m_xtFunc[l][lr][al][th]->GetParameters(par);
            nfitted += 1;
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
  B2INFO("saving histograms");
  TFile* fout = new TFile("histXT.root", "RECREATE");
  TDirectory* top = gDirectory;
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

