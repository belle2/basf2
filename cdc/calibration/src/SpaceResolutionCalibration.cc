#include <iostream>
#include <iomanip>
#include <cdc/calibration/SpaceResolutionCalibration.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/calibration/CDCDatabaseImporter.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
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

SpaceResolutionCalibration::SpaceResolutionCalibration():
  m_firstExperiment(0), m_firstRun(0),
  m_lastExperiment(-1), m_lastRun(-1)
{
  /*
   setDescription(
    " -------------------------- Test Calibration Algoritm -------------------------\n"
    "                                                                               \n"
    "  Testing algorithm which just gets mean of a test histogram collected by      \n"
    "  CaTest module and provides a DB object with another histogram with one       \n"
    "  entry at calibrated value.                                                   \n"
    " ------------------------------------------------------------------------------\n"
    );
  */

}
void SpaceResolutionCalibration::createHisto()
{

  B2INFO("createHisto");
  readSigma();
  readProfile();
  const int m_np = floor(1 / m_binWidth);

  TChain* tree = new TChain("tree");
  tree->Add(m_inputRootFileNames.c_str());
  B2INFO(" Open file name: " << m_inputRootFileNames.c_str());
  if (!tree->GetBranch("ndf")) {
    B2FATAL("input data do not exits, please check!");
    gSystem->Exec("echo rootfile do not exits or something wrong >> error");
    return;
  }

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
  for (int i = 1; i < m_np; ++i) {
    xbin.push_back(i * m_binWidth);
  }

  for (int il = 0; il < 56; ++il) {
    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nalpha; ++al) {
        for (int th = 0; th < m_ntheta; ++th) {
          hist_b[il][lr][al][th] = new TH2F(Form("hb_%d_%d_%d_%d", il, lr, al, th),
                                            Form("lay_%d_lr%d_al_%3.0f_th_%3.0f;Drift Length [cm];#DeltaX", il, lr, ialpha[al], itheta[th]),
                                            xbin.size() - 1, &xbin.at(0), yb.size() - 1, &yb.at(0));
          hist_u[il][lr][al][th] = new TH2F(Form("hu_%d_%d_%d_%d", il, lr, al, th),
                                            Form("lay_%d_lr%d_al_%3.0f_th_%3.0f;Drift Length [cm];#DeltaX", il, lr, ialpha[al], itheta[th]),
                                            xbin.size() - 1, &xbin.at(0), yu.size() - 1, &yu.at(0));
        }
      }
    }
  }


  const int nEntries = tree->GetEntries();
  B2INFO("Number of entries: " << nEntries);
  int ith = -99;
  int ial = -99;
  int ilr = -99;
  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);
    //cut
    if (std::fabs(x_b) < 0.02 || std::fabs(x_u) < 0.02) continue;
    if (Pval < m_Pvalmin) continue;
    if (ndf < m_ndfmin) continue;
    for (int k = 0; k < m_nalpha; ++k) {
      if (alpha < u_alpha[k]) {
        ial = k;
        break;
      }
    }

    for (int j = 0; j < m_ntheta; ++j) {
      if (theta < u_theta[j]) {
        ith = j;
        break;
      }
    }

    ilr = x_u > 0 ? 1 : 0;

    if (ial == -99 || ith == -99 || ilr == -99) {
      TString command = Form("Error in alpha=%3.2f and theta = %3.2f>> error", alpha, theta);
      //      gSystem->Exec(command);
      B2FATAL("ERROR" << command);
    }
    absRes_u = fabs(x_mea) - fabs(x_u);
    absRes_b = fabs(x_mea) - fabs(x_b);

    hist_u[lay][ilr][ial][ith]->Fill(fabs(x_u), absRes_u, w);
    hist_b[lay][ilr][ial][ith]->Fill(fabs(x_b), absRes_b, w);

  }

  B2INFO("Finish reading data");

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
      for (int al = 0; al < m_nalpha; ++al) {
        for (int th = 0; th < m_ntheta; ++th) {
          //fit half gaus for first range near sense wire
          B2DEBUG(199, "layer-lr-al-th " << il << " - " << lr << " - " << al << " - " << th);
          if (hist_b[il][lr][al][th]->GetEntries() < 5000) {
            m_fitflag[il][lr][al][th] = -1;
            continue;
          }
          B2DEBUG(199, "Nentries: " << hist_b[il][lr][al][th]->GetEntries());
          hist_b[il][lr][al][th]->SetDirectory(0);
          hist_u[il][lr][al][th]->SetDirectory(0);

          hist_b[il][lr][al][th]->FitSlicesY(g0b, firstbin, ib1, minEntry);

          hb_m[il][lr][al][th] = (TH1F*)gDirectory->Get(Form("hb_%d_%d_%d_%d_1", il, lr, al, th))->Clone(Form("hb_%d_%d_%d_%d_m", il, lr, al,
                                 th));
          hb_s[il][lr][al][th] = (TH1F*)gDirectory->Get(Form("hb_%d_%d_%d_%d_2", il, lr, al, th))->Clone(Form("hb_%d_%d_%d_%d_s", il, lr, al,
                                 th));//sigma

          hb_m[il][lr][al][th]->SetDirectory(0);
          hb_s[il][lr][al][th]->SetDirectory(0);


          //slice other bin with full gaus func
          hist_b[il][lr][al][th]->FitSlicesY(gb, ib1 + 1, m_np, minEntry);
          hb_m[il][lr][al][th]->Add((TH1F*)gDirectory->Get(Form("hb_%d_%d_%d_%d_1", il, lr, al, th))); //mean
          hb_s[il][lr][al][th]->Add((TH1F*)gDirectory->Get(Form("hb_%d_%d_%d_%d_2", il, lr, al, th))); //sigma
          B2DEBUG(199, "entries (2nd): " << hb_s[il][lr][al][th]->GetEntries());
          //fit half gaus for first range near sense wire
          hist_u[il][lr][al][th]->FitSlicesY(g0u, firstbin, ib1, minEntry);
          hu_m[il][lr][al][th] = (TH1F*)gDirectory->Get(Form("hu_%d_%d_%d_%d_1", il, lr, al, th))->Clone(Form("hu_%d_%d_%d_%d_m", il, lr, al,
                                 th));//mean
          hu_s[il][lr][al][th] = (TH1F*)gDirectory->Get(Form("hu_%d_%d_%d_%d_2", il, lr, al, th))->Clone(Form("hu_%d_%d_%d_%d_s", il, lr, al,
                                 th));//sigma
          hu_m[il][lr][al][th]->SetDirectory(0);
          hu_s[il][lr][al][th]->SetDirectory(0);

          //slice other bin with full gaus func
          hist_u[il][lr][al][th]->FitSlicesY(gu, ib1 + 1, m_np, minEntry);
          hu_m[il][lr][al][th]->Add((TH1F*)gDirectory->Get(Form("hu_%d_%d_%d_%d_1", il, lr, al, th))); //mean
          hu_s[il][lr][al][th]->Add((TH1F*)gDirectory->Get(Form("hu_%d_%d_%d_%d_2", il, lr, al, th))); //sigma
          if (!hu_s[il][lr][al][th] || !hb_s[il][lr][al][th]) {
            B2WARNING("slice histo do not found");
            m_fitflag[il][lr][al][th] = -1;
            continue;
          }
          for (Int_t j = 1; j < hu_s[il][lr][al][th]->GetNbinsX(); j++) {
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
            m_fitflag[il][lr][al][th] = -1;
            B2WARNING("number of element might out of range"); continue;
          }

          //Intrinsic resolution
          B2DEBUG(199, "Create Histo for layer-lr: " << il << " " << lr);
          gr[il][lr][al][th] = new TGraphErrors(xl.size(), &xl.at(0), &sigma.at(0), &dxl.at(0), &dsigma.at(0));
          gr[il][lr][al][th]->SetMarkerSize(0.5);
          gr[il][lr][al][th]->SetMarkerStyle(8);
          //          gr[il][lr][al][th]->SetMarkerColor(kBlack);
          //gr[il][lr][al][th]->SetLineColor(1 + lr + al * 2 + th * 3);
          gr[il][lr][al][th]->SetTitle(Form("Layer_%d_lr%d | #alpha = %3.0f | #theta = %3.0f", il, lr, ialpha[al], itheta[th]));
          gr[il][lr][al][th]->SetName(Form("lay%d_lr%d_al%d_th%d", il, lr, al, th));

          //s2 for fitting
          gfit[il][lr][al][th] = new TGraphErrors(xl.size(), &xl.at(0), &s2.at(0), &dxl0.at(0), &ds2.at(0));
          gfit[il][lr][al][th]->SetMarkerSize(0.5);
          gfit[il][lr][al][th]->SetMarkerStyle(8);
          //          gfit[il][lr][al][th]->SetMarkerColor(1 + lr + al * 2 + th * 3);
          //gfit[il][lr][al][th]->SetLineColor(1 + lr + al * 2 + th * 3);
          gfit[il][lr][al][th]->SetTitle(Form("L%d-lr%d | #alpha = %3.0f | #theta = %3.0f ", il, lr, ialpha[al], itheta[th]));
          gfit[il][lr][al][th]->SetName(Form("sigma2_lay%d_lr%d_al%d_th%d", il, lr, al, th));

          xl.clear(); dxl.clear(); dxl0.clear(); sigma.clear(); dsigma.clear(); s2.clear(); ds2.clear();
          gDirectory->Delete("hu_%d_%d_%d_%d_0");
        }
      }
    }
  }
  ofss.close();
}

bool SpaceResolutionCalibration::calibrate()
{

  B2INFO("calibrate");

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
      for (int al = 0; al < m_nalpha; ++al) {
        for (int th = 0; th < m_ntheta; ++th) {
          //boundary parameters,
          if (m_BField) {
            if (i < 8) {
              upFit = halfCSize[i] + 0.15; intp6 = halfCSize[i] + 0.1;
            } else {
              if (fabs(ialpha[al]) < 25) {
                upFit = halfCSize[i]; intp6 = halfCSize[i];
              } else {
                upFit = halfCSize[i] + 0.2; intp6 = halfCSize[i] + 0.4 ;
              }
            }
            //no B case
          } else {
            if (i < 8) {upFit = halfCSize[i] + 0.1; intp6 = halfCSize[i] + 0.1;}
            else {  upFit = halfCSize[i] - 0.07; intp6 = halfCSize[i];}
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
          B2DEBUG(199, "Fit flag before fit:" << m_fitflag[i][lr][al][th]);
          if (!gfit[i][lr][al][th]) continue;
          //    if(!(gfit[i][lr][al][th]->isValid())) continue;
          if (m_fitflag[i][lr][al][th] != -1) { /*if graph exist, do fitting*/
            //      m_fitflag[i][lr][al][th] = 0;
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
                  m_fitflag[i][lr][al][th] = 0;
                } else {
                  B2DEBUG(199, "Prob of fit: " << func->GetProb());
                  m_fitflag[i][lr][al][th] = 1;
                  break;
                }
              } else {
                m_fitflag[i][lr][al][th] = 0;
                func->SetParameters(5E-6, 0.007, 1E-4, 1E-7, 0.0007, -30, intp6 + 0.05 * j);
                upFit += 0.025;
                if (j == 9) {
                  TCanvas* c1 =  new TCanvas("c1", "", 600, 600);
                  gfit[i][lr][al][th]->Draw();
                  c1->SaveAs(Form("Sigma_Fit_Error_%s_%d_%d_%d_%d.png", Fit_status.c_str(), i, lr, al, th));
                  B2WARNING("Fit error: " << i << " " << lr << " " << al << " " << th);
                  //      fiterror=1;
                  // TString command = Form("echo error fit sigma layer %d- %d- %f- %f>> error", i, lr, ialpha[al], itheta[th]);
                  // gSystem->Exec(command);
                  // B2FATAL("fit error////////////////////////////////////////");
                }
              }
            }
            if (m_fitflag[i][lr][al][th] == 1) {
              B2DEBUG(199, "ProbFit: Lay_lr_al_th: " << i << " " << lr << " " << al << " " << th << func->GetProb());
              hprob->Fill(func->GetProb());
              func->GetParameters(sigma_new[i][lr][al][th]);
            }
          }
        }
      }
    }
  }
  //  ff->Close();
  write();
  storeHisto();

  return true;
}
void SpaceResolutionCalibration::storeHisto()
{
  B2INFO("storeHisto");
  TFile*  ff = new TFile("sigma_histo.root", "RECREATE");
  //  TMultiGraph* mg[56];
  //  TMultiGraph* mgfit[56];
  TDirectory* top = gDirectory;
  TDirectory* Direct[56];

  for (int il = 0; il < 56; ++il) {
    top->cd();
    Direct[il] = gDirectory->mkdir(Form("lay_%d", il));
    Direct[il]->cd();
    /*if(m_draw){
     mg[il] = new TMultiGraph();
     mg[il]->SetMinimum(0.);
     mg[il]->SetMaximum(0.04);}

     mgfit[i]=new TMultiGraph();
     mgfit[i]->SetMinimum(0.);
     mgfit[i]->SetMaximum(0.0015);
     mgfit[i]->SetTitle(Form("Layer_%d; Drift Length [cm];#sigma_{int}^{2} [cm^{2}]",i));
    }
    */

    for (int lr = 0; lr < 2; ++lr) {
      for (int al = 0; al < m_nalpha; ++al) {
        for (int th = 0; th < m_ntheta; ++th) {
          if (!gr[il][lr][al][th]) continue;
          if (!gfit[il][lr][al][th]) continue;
          if (m_fitflag[il][lr][al][th] == 1) {
            hist_b[il][lr][al][th]->Write();
            hist_u[il][lr][al][th]->Write();
            hb_m[il][lr][al][th]->Write();
            hb_s[il][lr][al][th]->Write();
            hu_m[il][lr][al][th]->Write();
            hu_s[il][lr][al][th]->Write();
            gr[il][lr][al][th]->Write();
            gfit[il][lr][al][th]->Write();
          }
          //    if(m_draw){
          //    mgfit[i]->Add(gfit[i][lr][al][th]);
          // mg[il]->Add(gr[il][lr][al][th]);}
        }
      }
    }
  }
  /*
  if(m_draw){
    TH2F* frame[56];
    TPad* pad[8];
    TPad* pad1[6];
    TCanvas* c3 = new TCanvas("c3","c3",1200,800);  c3->Divide(4,2);
    TCanvas* c4 = new TCanvas("c4","c4",1200,800);  c4->Divide(3,2);
    for (int p=0; p<6;++p){
      pad1[p]=(TPad*)(c4->GetPrimitive(Form("c4_%d",p+1)));
    }
    for(int i=0;i<56;++i){
      if (i<8){ c3->cd(i+1);
  pad[i]=(TPad*)(c3->GetPrimitive(Form("c3_%d",i+1)));
  pad[i]->cd();
  pad[i]->SetGrid(kTRUE);
  frame[i] = new TH2F(Form("frame_%d",i),Form("Layer_%d;Position x (cm);#sigma[cm]",i),100,0,1.,300,0,0.04);
  frame[i]->SetDirectory(0);
  frame[i]->Draw();
  mg[i]->Draw("APL same");
  pad[i]->BuildLegend();
      }
      else{ int index=(i-8)%6;
  //cout<<"index: "<<index<<endl;
  pad1[index]->cd();
  pad1[index]->SetGrid(kTRUE);
  frame[i] = new TH2F(Form("frame_%d",i),Form("Layer_%d;Position x (cm);#sigma[cm]",i),100,0,1.,300,0,0.04);
  frame[i]->SetDirectory(0);
  frame[i]->Draw();
  mg[i]->Draw("same");
  pad1[index]->BuildLegend();
      }

      if (i==7){c3->Print("IntResolution.pdf(","Title: intRes_Slayer_0"); delete c3;}
      if ((i-8)%6==5) {
  if ((i-7)/6==8){c4->Print("IntResolution.pdf)",Form("Title:intRes_Slayer_%d",(i-7)/6));}
  else{c4->Print("IntResolution.pdf",Form("Title:intRes_Slayer_%d",(i-7)/6));}
      }

    }
  }
  */

  ff->Close();
  B2INFO("Finish store histogram");
}
void SpaceResolutionCalibration::write()
{

  B2INFO("Exporting parameters...");
  int nfitted = 0;
  int nfailure = 0;
  /* Write the fit params*/

  ofstream ofs(m_outputSigmaFileName.c_str());
  ofs << m_nalpha << endl;
  for (int i = 0; i < m_nalpha; ++i) {
    ofs << std::setprecision(4) << l_alpha[i] << "   " << std::setprecision(4) << u_alpha[i] << "   " << std::setprecision(
          4) << ialpha[i] << endl;
  }

  ofs << m_ntheta << endl;
  for (int i = 0; i < m_ntheta; ++i) {
    ofs << std::setprecision(4) << l_theta[i] << "   " << std::setprecision(4) << u_theta[i] << "   " << std::setprecision(
          4) << itheta[i] << endl;
  }

  ofs << 0 << "  " << 7 << endl; //mode and number of params;
  for (int al = 0; al < m_nalpha; ++al) {
    for (int th = 0; th < m_ntheta; ++th) {
      for (int i = 0; i < 56; ++i) {
        for (int lr = 1; lr >= 0; --lr) {
          //          ffit[i][lr][al][th]->GetParameters(par);
          ofs << i << std::setw(4) << itheta[th] << std::setw(4) << ialpha[al] << std::setw(4) << lr << std::setw(15);
          if (m_fitflag[i][lr][al][th] == 1) {
            nfitted += 1;
            for (int p = 0; p < 7; ++p) {
              if (p != 6) { ofs << std::setprecision(7) << sigma_new[i][lr][al][th][p] << std::setw(15);}
              if (p == 6) { ofs << std::setprecision(7) << sigma_new[i][lr][al][th][p] << std::endl;}
            }
          } else {
            B2WARNING("Fitting error and old sigma will be used. (Layer " << i << ") (lr = " << lr << ") (al = " << al << ") (th = " << th <<
                      ")");
            nfailure += 1;
            int ial_old = 0;
            int ith_old = 0;
            for (int k = 0; k < nalpha_old; ++k) {
              if (ialpha[al] < u_alpha_old[k]) {ial_old = k; break;}
            }
            for (int j = 0; j < ntheta_old; ++j) {
              if (itheta[th] < u_theta_old[j]) {ith_old = j; break;}
            }
            for (int p = 0; p < 7; ++p) {
              if (p != 6) { ofs << std::setprecision(7) << sigma_old[i][lr][ial_old][ith_old][p] << std::setw(15);}
              if (p == 6) { ofs << std::setprecision(7) << sigma_old[i][lr][ial_old][ith_old][p] << std::endl;}
            }
          }
        }
      }
    }
  }
  ofs.close();
  B2RESULT("Number of histogram: " << 56 * 2 * m_nalpha * m_ntheta);
  B2RESULT("Histos succesfully fitted: " << nfitted);
  B2RESULT("Histos fit failure: " << nfailure);
  if (m_useDB) {
    CDCDatabaseImporter import(0, 0, -1, -1);
    import.importSigma(m_outputSigmaFileName.c_str());
  }


}
void SpaceResolutionCalibration::readSigma()
{
  B2INFO("readSigma");
  if (m_useDB) {
    B2INFO("reading sigma from DB");
    m_sResolFromDB = new DBObjPtr<CDCSpaceResols>;
    readSigmaFromDB();
    B2INFO("Number of theta bins from input sigma: " << ntheta_old);
  } else {
    B2INFO("Read sigma from text");
    readSigmaFromText();
    B2INFO("number of alpha bins from input sigma: " << nalpha_old);
  }
}

void SpaceResolutionCalibration::readSigmaFromText()
{
  ifstream ifs;
  std::string fileName1 = "/cdc/data/" + m_sigmafile;
  std::string fileName = FileSystem::findFile(fileName1);
  if (fileName == "") {
    fileName = FileSystem::findFile(m_sigmafile);
  }
  if (fileName == "") {
    cout << "CDCGeometryPar: " << fileName << " not exist!" << endl;
  } else {
    cout << "CDCGeometryPar: " << fileName << " exists." << endl;
    ifs.open(fileName);
    if (!ifs) cout << "CDCGeometryPar: cannot open " << fileName << " !" << endl;
  }

  //read alpha bin info.
  if (ifs >> nalpha_old) {
    if (nalpha_old == 0 || nalpha_old > 18) cout << "Fail to read alpha bins !" << endl;
  } else {
    cout << "Fail to read alpha bins !" << endl; return;
  }
  double alpha0, alpha1, alpha2;
  for (unsigned short i = 0; i < nalpha_old; ++i) {
    ifs >> alpha0 >> alpha1 >> alpha2;
    l_alpha_old[i] = alpha0;
    u_alpha_old[i] = alpha1;
    ialpha_old[i] = alpha2;
  }

  //read theta bin info.
  if (ifs >> ntheta_old) {
    if (ntheta_old == 0 || ntheta_old > 7) cout << "CDCGeometryPar: fail to read theta bins !" << endl;
  } else {
    cout << "CDCGeometryPar: fail to read theta bins !" << endl;
  }
  double theta0, theta1, theta2;
  for (unsigned short i = 0; i < ntheta_old; ++i) {
    ifs >> theta0 >> theta1 >> theta2;
    l_theta_old[i] = theta0;
    u_theta_old[i] = theta1;
    itheta_old[i] = theta2;
  }

  unsigned short np = 0;
  unsigned short iCL, iLR;
  double theta, alpha;
  unsigned nRead = 0;

  ifs >> m_sigmaParamMode_old >> np;
  double sigma[8];
  //  if (m_sigmaParamMode < 0 || m_sigmaParamMode > 1) cout<<"CDCGeometryPar: invalid sigma-parameterization mode read !"<<endl;
  //if (m_sigmaParamMode == 1) cout<<"CDCGeometryPar: sigma-parameterization mode=1 not ready yet"<<endl;
  //  if (np <= 0 || np > nSigmaParams) cout<<"CDCGeometryPar: no. of sigma-params. outside limits !"<<endl;
  const double epsi = 0.1;
  while (ifs >> iCL) {
    ifs >> theta >> alpha >> iLR;
    for (int i = 0; i < np; ++i) {
      ifs >> sigma[i];
    }
    ++nRead;

    int ith = -99;
    for (unsigned short i = 0; i < ntheta_old; ++i) {
      if (fabs(theta - itheta_old[i]) < epsi) {
        ith = i;
        break;
      }
    }
    if (ith < 0) cout << "CDCGeometryPar: thetas in sigma.dat are inconsistent !" << endl;

    int ial = -99;
    for (unsigned short i = 0; i < nalpha_old; ++i) {
      if (fabs(alpha - ialpha_old[i]) < epsi) {
        ial = i;
        break;
      }
    }
    if (ial < 0) cout << "CDCGeometryPar: alphas in sigma.dat are inconsistent !" << endl;

    for (int i = 0; i < np; ++i) {
      sigma_old[iCL][iLR][ial][ith][i] = sigma[i];
    }
  }  //end of while loop
  ifs.close();
}
void SpaceResolutionCalibration::readSigmaFromDB()
{
  typedef std::array<float, 3> array3; /**< angle bin info. */
  //  std::cout <<"setSResol called" << std::endl;
  nalpha_old = (*m_sResolFromDB)->getNoOfAlphaBins();
  double rad2deg = 180 / M_PI;
  for (unsigned short i = 0; i < nalpha_old; ++i) {
    //    m_alphaPoints[i] = (*dbXT_old)->getAlphaPoint(i);
    array3 alpha = (*m_sResolFromDB)->getAlphaBin(i);
    l_alpha_old[i] = alpha[0] * rad2deg;
    u_alpha_old[i] = alpha[1] * rad2deg;
    ialpha_old[i] = alpha[2] * rad2deg;
    //    std::cout << m_alphaPoints[i]*180./M_PI << std::endl;
  }

  ntheta_old = (*m_sResolFromDB)->getNoOfThetaBins();
  B2INFO("Ntheta: " << ntheta_old);
  for (unsigned short i = 0; i < ntheta_old; ++i) {
    //    m_thetaPoints[i] = (*dbXT_old).getThetaPoint(i);
    array3 theta = (*m_sResolFromDB)->getThetaBin(i);
    l_theta_old[i] = theta[0] * rad2deg;
    u_theta_old[i] = theta[1] * rad2deg;
    itheta_old[i] = theta[2] * rad2deg;
  }
  m_sigmaParamMode_old = (*m_sResolFromDB)->getSigmaParamMode();

  for (unsigned short iCL = 0; iCL < MAX_N_SLAYERS; ++iCL) {
    for (unsigned short iLR = 0; iLR < 2; ++iLR) {
      for (unsigned short iA = 0; iA < nalpha_old; ++iA) {
        for (unsigned short iT = 0; iT < ntheta_old; ++iT) {
          const std::vector<float> params = (*m_sResolFromDB)->getSigmaParams(iCL, iLR, iA, iT);
          unsigned short np = params.size();
          //    std::cout <<"np4sigma= " << np << std::endl;
          for (unsigned short i = 0; i < np; ++i) {
            sigma_old[iCL][iLR][iA][iT][i] = params[i];
          }
        }
      }
    }
  }

}
void SpaceResolutionCalibration::readProfile()
{
  B2INFO("readProfile");
  /*Read profile for xt*/
  if (m_useProfileFromInputSigma) {
    B2INFO("use Sigma bining from input Sigma");
    m_nalpha = nalpha_old;
    m_ntheta = ntheta_old;
    B2INFO("Number of alpha bins: " << m_nalpha);
    for (int i = 0; i < m_nalpha; ++i) {
      l_alpha[i] = l_alpha_old[i];    u_alpha[i] = u_alpha_old[i];     ialpha[i] = ialpha_old[i];
      B2INFO("" << i << " | " << l_alpha[i] << " " << u_alpha[i] << " " << ialpha[i]);
    }
    B2INFO("Number of theta bins: " << m_ntheta);
    for (int i = 0; i < m_ntheta; ++i) {
      l_theta[i] = l_theta_old[i];    u_theta[i] = u_theta_old[i];     itheta[i] = itheta_old[i];
      B2INFO("" << i << " |" << l_theta[i] << " " << u_theta[i] << " " << itheta[i]);
    }
  } else {
    B2INFO("use Sigma bining from profile file");
    ifstream proxt(m_ProfileFileName.c_str());
    if (!proxt) {
      B2FATAL("file not found: " << m_ProfileFileName);
    }
    double dumy1, dumy2, dumy3;
    proxt >> m_nalpha;
    B2INFO("Number of alpha bins: " << m_nalpha);
    if (m_nalpha > Max_nalpha) {B2FATAL("number of  alpha bin excess limit; please increse uplimit: " << m_nalpha << " > " << Max_nalpha);}
    for (int i = 0; i < m_nalpha; ++i) {
      proxt >> dumy1 >> dumy2 >> dumy3;
      l_alpha[i] = dumy1;    u_alpha[i] = dumy2;     ialpha[i] = dumy3;
      B2INFO("" << i << " | " << l_alpha[i] << " " << u_alpha[i] << " " << ialpha[i]);
    }
    proxt >> m_ntheta;
    B2INFO("Number of theta bins: " << m_ntheta);
    if (m_ntheta > Max_ntheta) {B2FATAL("number of  theta bin excess limit; please increse uplimit: " << m_ntheta << " > " << Max_ntheta);}
    for (int i = 0; i < m_ntheta; ++i) {
      proxt >> dumy1 >> dumy2 >> dumy3;
      l_theta[i] = dumy1;    u_theta[i] = dumy2;     itheta[i] = dumy3;
      B2INFO("" << i << " |" << l_theta[i] << " " << u_theta[i] << " " << itheta[i]);
    }
  }
  B2INFO("Finish asssign sigma bining");
}
