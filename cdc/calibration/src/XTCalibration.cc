//#include <calibration/CalibrationAlgorithm.h>
#include <cdc/calibration/XT.h>
//#include <cdc/utilities/readXT.h>
#include <cdc/calibration/XTCalibration.h>
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

#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/logging/Logger.h>
#include <cdc/calibration/CDCDatabaseImporter.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>


using namespace std;
using namespace Belle2;
using namespace CDC;
typedef std::array<float, 3> array3; /**< angle bin info. */
XTCalibration::XTCalibration():
  m_firstExperiment(0), m_firstRun(0),
  m_lastExperiment(-1), m_lastRun(-1)
{
  /*
   setDescription(
    " -------------------------- Test Calibration Algoritm -------------------------\n"
    "                                                                               \n"
    "  Testing algorithm which just gets mean of a test histogram collected by      \n"
  0    "  CaTest module and provides a DB object with another histogram with one       \n"
    "  entry at calibrated value.                                                   \n"
    " ------------------------------------------------------------------------------\n"
    );
  */
}

void XTCalibration::CreateHisto()
{
  readXT();
  readProfile();
  /* read data from tree, make histo for fit*/
  TChain* tree = new TChain("tree");
  tree->Add(m_inputRootFileNames.c_str());
  B2INFO("Open Files: " << m_inputRootFileNames);
  if (!tree->GetBranch("ndf")) {
    cout << "input data do not exits, please check!" << endl;
    B2FATAL("echo rootfile do not exits or something wrong");
    gSystem->Exec("echo rootfile do not exits or something wrong >> error");
    return;
  }
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
      for (int al = 0; al < m_nalpha; ++al) {
        for (int th = 0; th < m_ntheta; ++th) {
          hprof[i][lr][al][th] = new TProfile(Form("hprof%d_%d_%d_%d", i, lr, al, th),
                                              Form("(L=%d)-(lr=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                   i, lr, ialpha[al], itheta[th]), 210, -20, 600, 0, 1.2, "i");
          hist2d[i][lr][al][th] = new TH2D(Form("h%d_%d_%d_%d", i, lr, al, th),
                                           Form("(L=%d)-(lr=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                i, lr, ialpha[al], itheta[th]), 210, -20, 600, 110, 0, 1.2);
          if (lr == 1)
            hist2d_draw[i][al][th] = new TH2D(Form("h_draw%d_%d_%d", i, al, th),
                                              Form("(L=%d)-(#alpha=%3.0f)-(#theta=%3.0f); Drift time (ns);Drift Length (cm)",
                                                   i, ialpha[al], itheta[th]), 210, -20, 600, 2200, -1.2, 1.2);
        }
      }
    }
  }

  /*Now read data and make histo*/
  int al = 0;
  int th = 0;
  int lr = 0;
  Long64_t nbytes = 0;
  int nEntries = tree->GetEntries();
  cout << "Number of Hit: " << nEntries << endl;

  for (int i = 0; i < nEntries; ++i) {
    nbytes += tree->GetEntry(i);
    /* protect in case |alpha|>90*/
    if (fabs(alpha > 90)) {
      if (alpha < 0) alpha += 180;
      if (alpha > 0) alpha -= 180;
    }

    if (Pval < m_Pvalmin) continue;
    if (ndf < m_ndfmin) continue;

    for (int k = 0; k < m_nalpha; ++k) {
      if (alpha < u_alpha[k]) {al = k; break;}
    }
    for (int j = 0; j < m_ntheta; ++j) {
      if (theta < u_theta[j]) {th = j; break;}
    }
    if (dx > 0)
      lr = 1;
    else
      lr = 0;
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
void XTCalibration::readProfile()
{
  /*Read profile for xt*/
  if (m_useProfileXTFromInputXT) {
    B2INFO("use XT bining from input XT");
    m_nalpha = nalpha_old;
    m_ntheta = ntheta_old;
    B2INFO("Nalpha: " << m_nalpha << "\n Ntheta: " << m_ntheta);
    for (int i = 0; i < m_nalpha; ++i) {
      l_alpha[i] = l_alpha_old[i];
      u_alpha[i] = u_alpha_old[i];
      ialpha[i] = ialpha_old[i];
      B2INFO("-" << l_alpha[i] << " " << u_alpha[i] << " " << ialpha[i]);
    }
    for (int i = 0; i < m_ntheta; ++i) {
      l_theta[i] = l_theta_old[i];
      u_theta[i] = u_theta_old[i];
      itheta[i] = itheta_old[i];
      B2INFO("-" << l_theta[i] << " " << u_theta[i] << " " << itheta[i]);
    }
  } else {
    B2INFO("use XT bining from profile file");
    ifstream proxt(m_profileFileName.c_str());
    if (!proxt) {
      B2FATAL("file not found: " << m_profileFileName);
    }
    double dumy1, dumy2, dumy3;
    proxt >> m_nalpha;
    B2DEBUG(99, "Number of alpha bin" << m_nalpha);
    if (m_nalpha > m_MAXalpha) {
      B2FATAL("number of  alpha bin excess limit; please increse uplimit: " << m_nalpha << " > " << m_MAXalpha);
    }
    for (int i = 0; i < m_nalpha; ++i) {
      proxt >> dumy1 >> dumy2 >> dumy3;
      l_alpha[i] = dumy1;
      u_alpha[i] = dumy2;
      ialpha[i] = dumy3;
    }
    proxt >> m_ntheta;
    B2DEBUG(99, "Number of theta bin" << m_nalpha);
    if (m_ntheta > m_MAXtheta) {B2FATAL("number of  theta bin excess limit; please increse uplimit: " << m_ntheta << " > " << m_MAXtheta);}
    for (int i = 0; i < m_ntheta; ++i) {
      proxt >> dumy1 >> dumy2 >> dumy3;
      l_theta[i] = dumy1;
      u_theta[i] = dumy2;
      itheta[i] = dumy3;
    }
  }
  B2INFO("Finish asssign XT bining");
}

bool XTCalibration::calibrate()
{
  gROOT->SetBatch(1);
  gErrorIgnoreLevel = 3001;

  CreateHisto();
  B2INFO("Start Fitting");
  for (int l = 0; l < 56; ++l) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nalpha; ++al) {
        for (int th = 0; th < m_ntheta; ++th) {

          if (hist2d[l][lr][al][th]->GetEntries() < m_smallestEntryRequire) {
            fitflag[l][lr][al][th] = -1;
            continue;
          }
          double p0, p1, tmin;
          TF1* fpol1;
          if (m_useSliceFit) {
            hist2d[l][lr][al][th]->FitSlicesY(0, 0, -1, 5);
            hist2d_1[l][lr][al][th] = (TH1D*)gDirectory->Get(Form("h%d_%d_%d_%d_1", l, lr, al, th));
            if (!hist2d_1[l][lr][al][th]) {
              fitflag[l][lr][al][th] = -1;
              B2WARNING("error, not found results of slices fit");
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
            p0 = 0; p1 = 0.005;
            tmin = 12;
          }
          XT* xt;
          if (m_useSliceFit) {
            xt = new XT(hist2d_1[l][lr][al][th], m_xtmode);
          } else {
            xt = new XT(hprof[l][lr][al][th], m_xtmode);
          }
          //    xt->setSmallestEntryRequired(m_smallestEntryRequire);
          if (m_BField) {
            int ial_old = 0;
            int ith_old = 0;
            for (int k = 0; k < nalpha_old; ++k) {
              if (ialpha[al] < u_alpha_old[k]) {ial_old = k; break;}
            }
            for (int j = 0; j < ntheta_old; ++j) {
              if (itheta[th] < u_theta_old[j]) {ith_old = j; break;}
            }
            double p6 =  xtold[l][lr][ial_old][ith_old][6];
            if (p6 > 400)
              p6 = 400;
            if (m_xtmode == xtmode_old) {
              xt->setXTParams(xtold[l][lr][ial_old][ith_old]);
              xt->setP6(p6);
            } else {
              xt->setXTParams(p0, p1, 0., 0., 0., 0., p6, xtold[l][lr][ial_old][ith_old][7]);
            }
            xt->setFitRange(tmin, p6 + 100);
          } else {
            xt->setXTParams(p0, p1, 0., 0., 0., 0., m_par6[l], 0.0001);
            xt->setFitRange(tmin, m_par6[l] + 100);
          }
          xt->setDebug(m_debug);
          xt->BField(m_BField);
          xt->FitXT(m_xtmode);
          /*get result*/
          fitflag[l][lr][al][th] = xt->getFitStatus();
          xtf5r[l][lr][al][th] = (TF1*)xt->getXTFunction();
          if (m_useSliceFit) {
            hist2d_1[l][lr][al][th] = (TH1D*)xt->getFittedHisto();
          } else {
            hprof[l][lr][al][th] = (TProfile*)xt->getFittedHisto();
          }
          // TH1D* h1 = (TH1D*)xt->getFittedHisto();

          //    if(m_debug){
          //TCanvas* c1  = new TCanvas("c1","c1",500,500);
          // h1->Draw();
          //xtf5r[l][lr][al][th]->DrawF1(0,400,"same");
          //c1->Print(Form("pic/xt/%i_layer_%i_%i_%i_%i.png",fitflag[l][lr][al][th],l,lr,al,th));}
          /*Revert param for left side to draw*/
          //    if(lr==0 && fitflag[l][lr][al][th]!=0 ){
          //xtf5r[l][lr][al][th]->GetParameters(parN);
          //    xtf5rLeft_draw[l][lr][al][th]->SetParameters(-1*parN[0],-1*parN[1],-1*parN[2],-1*parN[3],-1*parN[4],-1*parN[5],parN[6],-1*parN[7]);
          //}
        }
      }
    }
  }
  Write();
  storeHisto();
  return true;
}
void XTCalibration::Write()
{
  /*Set parameter for layer that fit do not success*/
  /* and then write output file*/
  double par[8];
  ofstream xtout(m_OutputXTFileName.c_str());
  xtout << m_nalpha << endl;
  for (int i = 0; i < m_nalpha; ++i) {
    xtout << std::setprecision(3) << l_alpha[i] << "  "
          << std::setprecision(3) << u_alpha[i] << "  "
          << std::setprecision(3) << ialpha[i] << endl;
  }
  xtout << m_ntheta << endl;
  for (int i = 0; i < m_ntheta; ++i) {
    xtout << std::setprecision(3) << l_theta[i] << "  "
          << std::setprecision(3) << u_theta[i] << "  "
          << std::setprecision(3) << itheta[i]  << endl;
  }
  xtout << m_xtmode << "  " << 8 << endl;

  int nfitted = 0;
  int nfailure = 0;

  for (int th = 0; th < m_ntheta; ++th) {
    for (int al = 0; al < m_nalpha; ++al) {
      for (int l = 0; l < 56; ++l) {
        for (int lr = 0; lr < 2; ++lr) {
          /*Set Parameter for bad fit*/
          if (fitflag[l][lr][al][th] != 1) {
            nfailure += 1;
            printf("fit failure status = %d \n", fitflag[l][lr][al][th]);
            printf("layer %d, r %d, alpha %3.1f, theta %3.1f \n", l, lr, ialpha[al], itheta[th]);
            printf("number of event: %3.2f", hprof[l][lr][al][th]->GetEntries());
            if (fitflag[l][lr][al][th] != -1) {
              printf("Probability of fit: %3.4f", xtf5r[l][lr][al][th]->GetProb());
            }
            if (m_xtmode == xtmode_old) {
              int ial_old = 0;
              int ith_old = 0;
              for (int k = 0; k < nalpha_old; ++k) {
                if (ialpha[al] < u_alpha_old[k]) {ial_old = k; break;}
              }
              for (int j = 0; j < ntheta_old; ++j) {
                if (itheta[th] < u_theta_old[j]) {ith_old = j; break;}
              }
              for (int p = 0; p < 8; ++p) {
                par[p] = xtold[l][lr][ial_old][ith_old][p];
              }
            } else {
              //if mode of input xt is different from output, simple xt is used.
              par[0] = 0; par[1] = 0.004; par[2] = 0; par[3] = 0; par[4] = 0; par[5] = 0; par[6] = m_par6[l]; par[7] = 0.00001;
            }
          } else {
            xtf5r[l][lr][al][th]->GetParameters(par);
            nfitted += 1;
          }
          /*Write params*/
          xtout << l << std::setw(5) << itheta[th] << std::setw(5) << ialpha[al] << std::setw(5) << "0.0" << std::setw(4) << lr << std::setw(
                  15);
          for (int p = 0; p < 8; ++p) {
            if (p != 7) { xtout << std::setprecision(7) << par[p] << std::setw(15);}
            if (p == 7) { xtout << std::setprecision(7) << par[p] << std::endl;}
          }
        }//lr
      }//th
    }//al
  }//lay
  xtout.close();
  B2RESULT(" Total number of xt fit: " << m_nalpha * m_ntheta * 2 * 56);
  B2RESULT(" Successfully Fitted: " << nfitted);
  B2RESULT(" Failure Fit: " << nfailure);
  B2RESULT("Finish export xt to text file");
  if (m_useDB) {
    CDCDatabaseImporter import(0, 0, -1, -1);
    import.importXT(m_OutputXTFileName.c_str());
  }
}

void XTCalibration::storeHisto()
{
  B2INFO("start store histogram");
  TFile* fout = new TFile("XTFIT.root", "RECREATE");
  TDirectory* top = gDirectory;
  TDirectory* Direct[56];
  int nhisto = 0;
  for (int l = 0; l < 56; ++l) {
    top->cd();
    Direct[l] = gDirectory->mkdir(Form("lay_%d", l));
    Direct[l]->cd();
    for (int th = 0; th < m_ntheta; ++th) {
      for (int al = 0; al < m_nalpha; ++al) {
        hist2d_draw[l][al][th]->Write();
        for (int lr = 0; lr < 2; ++lr) {
          hist2d[l][lr][al][th]->Write();
          if (fitflag[l][lr][al][th] != 1) continue;
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

void XTCalibration::readXT()
{
  if (m_useDB) {
    B2INFO("reading xt from DB");

    /*
    ReadXT:readXTFromDB(&xtold,dbXT_old,
       &nalpha_old,l_alpha_old,u_alpha_old,ialpha_old,
       &ntheta_old,l_theta_old,u_theta_old, itheta_old);
    */
    readXTFromDB();
    B2INFO("Number of theta bin from xt: " << ntheta_old);
    B2INFO("Theta 0: " << itheta_old[0]);
    //    if(!a){
    /// B2FATAL("Error reading xt from DB");return;}
  } else {
    B2INFO("Read Xt from text");
    /*
    ReadXT::readXTFromText(xtold,m_xtfile,
            nalpha_old,l_alpha_old,u_alpha_old,ialpha_old,
            ntheta_old,l_theta_old,u_theta_old, itheta_old);
    */
    readXTFromText();
    B2INFO("nalpha: " << nalpha_old);
    // if(!a)
    // {B2FATAL("Error reading xt from text");return;}
  }
}

void XTCalibration::readXTFromText()
{
  std::string fileName1 = "/cdc/data/" + m_xtfile;
  std::string fileName = FileSystem::findFile(fileName1);
  boost::iostreams::filtering_istream ifs;
  if (fileName == "") {
    fileName = FileSystem::findFile(m_xtfile);
  }
  if (fileName == "") {
    B2FATAL("CDCGeometryPar: " << fileName1 << " not exist!");
  } else {
    B2INFO("CDCGeometryPar: open " << fileName1);
    if ((fileName.rfind(".gz") != string::npos) && (fileName.length() - fileName.rfind(".gz") == 3)) {
      ifs.push(boost::iostreams::gzip_decompressor());
    }
    ifs.push(boost::iostreams::file_source(fileName));
    if (!ifs) B2FATAL("CDCGeometryPar: cannot open " << fileName1 << " !");


  }
  int npar = 8;
  //read alpha bin info.
  //  unsigned short nAlphaBins = 0;
  ifs >> nalpha_old;

  for (unsigned short i = 0; i < nalpha_old; ++i) {
    ifs >> l_alpha_old[i] >> u_alpha_old[i] >> ialpha_old[i];
    // ifs >> alpha0 >> alpha1 >> alpha2;
    //m_alphaPoints[i] = alpha2;
  }

  //read theta bin info.
  //  unsigned short nThetaBins = 0;
  ifs >> ntheta_old;

  for (unsigned short i = 0; i < ntheta_old; ++i) {
    ifs >> l_theta_old[i] >> u_theta_old[i] >> itheta_old[i];
    //ifs >> theta0 >> theta1 >> theta2;
    //m_thetaPoints[i] = theta2;
  }

  B2INFO("number of alpha - theta bin" << nalpha_old << " - " << ntheta_old);
  short np = 0;
  unsigned short iCL, iLR;
  //  const unsigned short npx = nXTParams - 1;
  double xtc[npar];
  double theta, alpha, dummy1;
  unsigned nRead = 0;
  //  unsigned m_xtParamMode_old;
  ifs >> xtmode_old >> np;

  const double epsi = 0.1;

  while (ifs >> iCL) {
    ifs >> theta >> alpha >> dummy1 >> iLR;
    for (int i = 0; i < np; ++i) {
      ifs >> xtc[i];
    }
    ++nRead;

    int ith = -99;
    for (unsigned short i = 0; i < ntheta_old; ++i) {
      if (fabs(theta - itheta_old[i]) < epsi) {
        ith = i;
        break;
      }
    }
    if (ith < 0) {
      gSystem->Exec("echo xt_theta error binning>> error");
      return;
    }

    int ial = -99;
    for (unsigned short i = 0; i < nalpha_old; ++i) {
      if (fabs(alpha - ialpha_old[i]) < epsi) {
        ial = i;
        break;
      }
    }
    if (ial < 0) {
      gSystem->Exec("echo xt_alpha error binning>> error");
      return;
    }

    for (int i = 0; i < np; ++i) {
      xtold[iCL][iLR][ial][ith][i] = xtc[i];
    }

  }  //end of while loop

  //convert unit
  /*
  const double degrad = M_PI / 180.;
  for (unsigned i = 0; i < nAlphaBins; ++i) {
    m_alphaPoints[i] *= degrad;
  }
  for (unsigned i = 0; i < nThetaBins; ++i) {
    m_thetaPoints[i] *= degrad;
  }
  */
  //  return true;
}
void XTCalibration::readXTFromDB()
{
  DBObjPtr<CDCXtRelations> dbXT_old;
  nalpha_old = dbXT_old->getNoOfAlphaBins();
  B2INFO("Number of alpha" << nalpha_old);
  double rad2deg = 180 / M_PI;
  for (unsigned short i = 0; i < nalpha_old; ++i) {
    array3 alpha = dbXT_old->getAlphaBin(i);
    l_alpha_old[i] = alpha[0] * rad2deg;
    u_alpha_old[i] = alpha[1] * rad2deg;
    ialpha_old[i] = alpha[2] * rad2deg;
    //    std::cout << m_alphaPoints[i]*180./M_PI << std::endl;
  }

  ntheta_old = dbXT_old->getNoOfThetaBins();
  B2INFO("Ntheta: " << ntheta_old);
  for (unsigned short i = 0; i < ntheta_old; ++i) {
    //    m_thetaPoints[i] = (*dbXT_old).getThetaPoint(i);
    array3 theta = dbXT_old->getThetaBin(i);
    l_theta_old[i] = theta[0] * rad2deg;
    u_theta_old[i] = theta[1] * rad2deg;
    itheta_old[i] = theta[2] * rad2deg;


    //    std::cout << m_thetaPoints[i]*180./M_PI << std::endl;
  }

  xtmode_old = dbXT_old->getXtParamMode();

  for (unsigned short iCL = 0; iCL < 56; ++iCL) {
    for (unsigned short iLR = 0; iLR < 2; ++iLR) {
      for (unsigned short iA = 0; iA < nalpha_old; ++iA) {
        for (unsigned short iT = 0; iT < ntheta_old; ++iT) {
          const std::vector<float> params = dbXT_old->getXtParams(iCL, iLR, iA, iT);
          unsigned short np = params.size();
          //    std::cout <<"np4xt= " << np << std::endl;
          for (unsigned short i = 0; i < np; ++i) {
            xtold[iCL][iLR][iA][iT][i] = params[i];
          }
        }
      }
    }
  }
  //  return true;
}
