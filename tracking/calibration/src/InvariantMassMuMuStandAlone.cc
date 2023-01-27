/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <iostream>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <tuple>
#include <numeric>
#include <fstream>

#include <TROOT.h>
#include <TTree.h>
#include <TGraph.h>
#include <TRandom3.h>
#include <TH1D.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TLine.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TPad.h>
#include <Math/Functor.h>
#include <Math/SpecFuncMathCore.h>
#include <Math/DistFunc.h>

#include <Eigen/Dense>

#include <framework/particledb/EvtGenDatabasePDG.h>

//if compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/InvariantMassMuMuStandAlone.h>
#include <tracking/calibration/InvariantMassMuMuIntegrator.h>
#include <tracking/calibration/BoostVectorStandAlone.h>
#include <tracking/calibration/Splitter.h>
#include <tracking/calibration/tools.h>
#include <tracking/calibration/ChebFitter.h>
#else
#include <InvariantMassMuMuStandAlone.h>
#include <InvariantMassMuMuIntegrator.h>
#include <Splitter.h>
#include <tools.h>
#include <ChebFitter.h>
#endif

using Eigen::MatrixXd;
using Eigen::VectorXd;


namespace Belle2::InvariantMassMuMuCalib {




  /// read events from TTree to std::vector
  std::vector<Event> getEvents(TTree* tr, bool is4S)
  {

    std::vector<Event> events;
    events.reserve(tr->GetEntries());

    Event evt;

    tr->SetBranchAddress("run", &evt.run);
    tr->SetBranchAddress("exp", &evt.exp);
    tr->SetBranchAddress("event", &evt.evtNo);

    B2Vector3D* p0 = nullptr;
    B2Vector3D* p1 = nullptr;

    tr->SetBranchAddress("mu0_p", &p0);
    tr->SetBranchAddress("mu1_p", &p1);

    tr->SetBranchAddress("mu0_pid", &evt.mu0.pid);
    tr->SetBranchAddress("mu1_pid", &evt.mu1.pid);


    tr->SetBranchAddress("time", &evt.t); //time in hours

    const double mMu  = EvtGenDatabasePDG::Instance()->GetParticle("mu-")->Mass();  //muon mass, i.e. around 105.66e-3 [GeV]

    for (int i = 0; i < tr->GetEntries(); ++i) {
      tr->GetEntry(i);

      evt.mu0.p = *p0;
      evt.mu1.p = *p1;

      evt.m = sqrt(pow(hypot(evt.mu0.p.Mag(), mMu) + hypot(evt.mu1.p.Mag(), mMu), 2)   - (evt.mu0.p + evt.mu1.p).Mag2());

      evt.nBootStrap = 1;
      evt.isSig = true;
      evt.is4S  = is4S;
      events.push_back(evt);
    }

    //sort by time
    sort(events.begin(), events.end(), [](Event e1, Event e2) {return e1.t < e2.t;});


    return events;
  }





  // Numerical integration using Gauss-Konrod algorithm
  double integrate(std::function<double(double)> f, double a, double b)
  {
    static const std::vector<double> nodes = {
      -0.991455371120813,
        -0.949107912342759,
        -0.864864423359769,
        -0.741531185599394,
        -0.586087235467691,
        -0.405845151377397,
        -0.207784955007898,
        0.000000000000000
      };

    static const std::vector<double> wgts = {
      0.022935322010529,
      0.063092092629979,
      0.104790010322250,
      0.140653259715525,
      0.169004726639267,
      0.190350578064785,
      0.204432940075298,
      0.209482141084728
    };

    if (b < a) B2FATAL("Wrongly defined integration interval");

    double m = (b + a) / 2; //middle of the interval
    double d = (b - a) / 2; //half-width of the interval

    double sum = 0;
    for (unsigned i = 0; i < nodes.size() - 1; ++i) {
      double x1 = m - d * nodes[i];
      double x2 = m + d * nodes[i];
      sum += (f(x1) + f(x2)) * wgts[i];
    }

    //add the central point (which is not in pair)
    sum += f(m) * wgts.back();


    // scale by size of interval, for example a=-1, b=1 -> no scaling
    return sum * d;
  }










  /** Integral of function exp(-c x^2 + 2*d -d*d/c) from a to b */
  double gausInt(double a, double b, double c, double d)
  {
    double res = sqrt(M_PI) / (2 * sqrt(c)) * (TMath::Erf((b * c - d) / sqrt(c)) - TMath::Erf((a * c - d) / sqrt(c)));
    return res;
  }


  /** Convolution of Gaus(sigma=sK, mu=0, x) and  Gaus(sigma=s, mu=m, x)|x=[a,b],
      i.e. the second Gaus is non-zero only for a < x < b */
  double convGausGaus(double sK, double s, double a, double b, double m, double x)
  {
    a -= m;
    b -= m;
    x -= m;


    double c = 1. / 2 * (1. / s / s + 1. / sK / sK);
    double d = 1. / 2 * x / sK / sK;

    double Const = 1. / (2 * M_PI) * 1. / (s * sK)  * exp(-1. / 2 * x * x / (s * s + sK * sK));

    double res = Const * gausInt(a, b, c, d);
    assert(isfinite(res));
    return res;
  }




  /** Convolution of Gaus(sigma=sK, mu=0, x) and  Exp(tau=tau, shift=a, x)|x=[a,inf],
      i.e. the Exp is non-zero only for  for x > a, where it's defined as:
      Exp(tau=tau, shift=a, x) = 1/tau * exp(-(x-a)/tau)  */
  double convExpGaus(double sK, double tau, double a, double x)
  {
    x -= a;

    double A = 1. / sqrt(2) * (-x / sK + sK / tau);
    double B = -x / tau + 1. / 2 * pow(sK / tau, 2);
    double res = 0;
    if (B > 700 || A > 20) { // safety term to deal with 0 * inf limit
      res = 1. / (2 * tau) * 1. / sqrt(M_PI) * exp(-A * A + B) * (1 / A - 1 / 2. / pow(A, 3) + 3. / 4 / pow(A, 5));
    } else {
      res = 1. / (2 * tau) * TMath::Erfc(A) * exp(B);
    }
    assert(isfinite(res));
    return res;
  }


  // convolution of Gaussian with exp tails with the Gaussian smearing kernel
  double gausExpConv(double mean, double sigma, double bMean, double bDelta, double tauL, double tauR, double sigmaK, double x)
  {
    double bL = bMean - bDelta;
    double bR = bMean + bDelta;

    double xL = bL * sigma + mean;
    double xR = bR * sigma + mean;

    double iGaus  = sqrt(2 * M_PI) * sigma * convGausGaus(sigmaK, sigma, xL, xR, mean, x);
    double iRight = exp(-1. / 2 * pow(bR, 2)) * tauR * convExpGaus(sigmaK, tauR, xR, x);
    double iLeft  = exp(-1. / 2 * pow(bL, 2)) * tauL * convExpGaus(sigmaK, tauL, -xL, -x);

    return (iGaus + iLeft + iRight);
  }

  /** gausExpConv with an added Gaus (eCMS resolution function) in the ROOT-like format */
  double gausExpConvRoot(const double* par)
  {
    double x = par[0];      // point where the function is evaluated
    double mean = par[1];   // mean of Gauss in "Crystal Ball" with exp tails instead of pow
    double sigma = par[2];  // sigma of Gauss in "Crystal Ball" with exp tails instead of pow
    double bMean = par[3];  // mean of the transition points between Gaus and exp
    double bDelta = par[4]; // diff/2 of the transition points between Gaus and exp
    double tauL = par[5];   // decay par of the left exp
    double tauR = par[6];   // decay par of the right exp
    double sigmaK = par[7]; // sigma of the Gaussian smearing Kernel
    double sigmaA = par[8]; // sigma of the added Gaussian
    double fA     = par[9]; // fraction of the added Gaussian

    //added Gaussian
    double G = 1. / (sqrt(2 * M_PI) * sigmaA) * exp(-1. / 2 * pow((x - mean) / sigmaA, 2));
    return (1 - fA) * gausExpConv(mean, sigma, bMean, bDelta, tauL, tauR, sigmaK, x) + fA * G;
  }


  /** gausExpConv with an added Gaus (eCMS resolution function) convoluted with eCMS gen-level spectrum
      in the ROOT-like format */
  double gausExpPowConvRoot(const double* par)
  {
    double x = par[0];      // point where the function is evaluated
    double mean = par[1];   // mean of Gauss in "Crystal Ball" with exp tails instead of pow
    double sigma = par[2];  // sigma of Gauss in "Crystal Ball" with exp tails instead of pow
    double bMean = par[3];  // mean of the transition points between Gaus and exp
    double bDelta = par[4]; // diff/2 of the transition points between Gaus and exp
    double tauL = par[5];   // decay par of the left exp
    double tauR = par[6];   // decay par of the right exp
    double sigmaK = par[7]; // sigma of the Gaussian smearing Kernel
    double sigmaA = par[8]; // sigma of the added Gaussian
    double fA     = par[9]; // fraction of the added Gaussian



    double eCMS  = par[10]; // center of mass energy of the collisions
    double slope = par[11]; // power-slope of gen-level spectra from ISR
    double K = par[12];     // normalisation of the part without photon ISR

    double step = 0.10;     // step size in the integration
    int N = 800 * 1. / step;

    double sum = 0;

    //calculation of the convolution using trapezium rule
    for (int i = 0; i < N; ++i) {

      double t = eCMS - i * step;

      double y = x - t;
      double G = 1. / (sqrt(2 * M_PI) * sigmaA) * exp(-1. / 2 * pow((y - mean) / sigmaA, 2));
      double Core = (1 - fA) * gausExpConv(mean, sigma, bMean, bDelta, tauL, tauR, sigmaK, y) + fA * G;

      double C = (i == 0 || i == N - 1) ? 0.5 : 1;

      double Kernel;
      if (i == 0)
        Kernel = K * pow(step, -slope);
      else
        Kernel = pow(eCMS - t, -slope);


      sum += Kernel * Core * step * C;
    }

    return sum;
  }






  /** plot convolution of Gaussian with exp tails with the Gaussian smearing kernel
      the parametrization of the eCMS resolution function  */
  void plotTest()
  {
    double mean = 0;
    double bMean = 0;
    double bDelta = 5;
    double tauL = 30;
    double tauR = 30;
    double sigma = 10;
    double sigmaK = 40;

    TGraph* gr = new TGraph;
    for (double x = -300; x < 300; x += 1) {

      double v = gausExpConv(mean, sigma, bMean, bDelta, tauL, tauR, sigmaK, x);

      gr->SetPoint(gr->GetN(), x, v);
    }

    gr->Draw();
  }


  /** Gaussian with the exponential tails */
  double gausExp(const double* par)
  {
    double x = par[0];      // point where the function is evaluated
    double mean = par[1];   // mean of Gaus
    double sigma = par[2];  // sigma of Gaus
    double bMean = par[3];  // mean of the transition points between Gaus and exp
    double bDelta = par[4]; // diff/2 of the transition points between Gaus and exp
    double tauL = par[5];   // decay par of the left exp
    double tauR = par[6];   // decay par of the right exp

    double bL = bMean - bDelta;
    double bR = bMean + bDelta;


    double r = (x - mean) / sigma;
    if (bL <= r && r <= bR) {
      return exp(-1. / 2 * pow(r, 2));
    } else if (r < bL) {
      double bp = exp(-1. / 2 * pow(bL, 2));
      double xb = mean + bL * sigma;
      return exp((x - xb) / tauL) * bp;
    } else {
      double bp = exp(-1. / 2 * pow(bR, 2));
      double xb = mean + bR * sigma;
      return exp(-(x - xb) / tauR) * bp;
    }
  }






  /** plot the integrand connected to pdfGen conv resFun and the
      function pdfGen conv resFun itself */
  void plotInt()
  {
    double mean = 4;
    double sigma  = 30;
    double sigmaK = 30;
    double bMean = 0;
    double bDelta = 2.6;
    double tau = 60;

    double x = 10300;

    TGraph* gr    = new TGraph;
    TGraph* grE   = new TGraph;
    TGraph* grRat = new TGraph;
    TGraph* grR   = new TGraph;

    double m0 = 10500;
    double slope = 0.95;
    double eps = 0.01;

    double frac = 0.2;
    double sigmaE = 30;

    for (double t = eps; t < 5000; t += 1.00) {
      double Core = gausExpConv(mean, sigma, bMean, bDelta, tau, tau, sigmaK, x + t - m0);
      double K = (+t) >= eps ? pow(+ t, -slope) : 0;

      double fun = Core * K;
      gr->SetPoint(gr->GetN(), t, fun);

      double s = exp(-t / tau);

      double funE = exp(-abs(x - m0 + t) / tau) * 1 / t;
      grE->SetPoint(grE->GetN(), t, funE);
      if (funE > 0) {
        grRat->SetPoint(grE->GetN(), t, fun / funE);
        grR->SetPoint(grR->GetN(), s, fun / funE);
      }
    }


    InvariantMassMuMuIntegrator integrator;

    TGraph* grM = new TGraph; // the PDF used in the fit

    double C = 16;

    for (double xNow = 10000; xNow <= 11000; xNow += 0.1) {

      integrator.init(mean,
                      sigma,
                      sigmaK,
                      bMean,
                      bDelta,
                      tau,
                      sigmaE,
                      frac,
                      m0,
                      eps,
                      C,
                      slope,
                      xNow);


      grM->SetPoint(grM->GetN(), xNow, integrator.integralKronrod(2000));
    }

    grM->Draw();


    delete gr;
    delete grE;
    delete grRat;
    delete grR;
    delete grM;

  }


  /** the function which is used to fit M(mu,mu) spectrum */
  double mainFunction(double xx, Pars par)
  {
    InvariantMassMuMuIntegrator fun;

    fun.init(par.at("mean"),   // mean
             par.at("sigma"), // sigma
             par.at("sigma"), // sigmaK
             par.at("bMean"), // bMean
             par.at("bDelta"),// bDelta
             par.at("tau"),   // tau=tauL=tauR
             par.at("sigma"), // sigmaE
             par.at("frac"),  // frac
             par.at("m0"),    // m0
             0.1,             // eps
             par.at("C"),     // C
             par.at("slope"), // slope
             xx);             // x

    return fun.integralKronrod(2000);
  }


  /** read the mass from events and filter on mu PID and invariant mass range, i.e. a < mass < b
    a and b are in MeV */
  std::vector<double> readEvents(const std::vector<Event>& evts, double pidCut, double a, double b)
  {

    std::vector<double> vMass;
    for (const auto& ev : evts) {

      //Keep only muons
      if (ev.mu0.pid < pidCut || ev.mu1.pid < pidCut) {
        continue;
      }

      double m = 1e3 * ev.m; // from GeV to MeV
      if (a < m && m < b)
        vMass.push_back(m);
    }

    return vMass;
  }


/// plots the result of the fit to the Mmumu, i.e. data and the fitted curve, the base function
  static void plotMuMuFitBase(TH1D* hData, TGraph* gr, TH1D* hPull, Pars pars, Eigen::MatrixXd mat, int time)
  {
    bool isBatch = gROOT->IsBatch();
    gROOT->SetBatch(kTRUE);

    gStyle->SetOptStat(0);

    TCanvas* can = new TCanvas(Form("canMuMu_%d", time), "");

    TPad* pad1 = new TPad(Form("pad1_%d", time), "", 0, 0.3, 1, 1.0);
    TPad* pad2 = new TPad(Form("pad2_%d", time), "", 0, 0,   1, 0.3);

    pad1->SetBottomMargin(0.05);
    pad2->SetTopMargin(0.05);
    pad2->SetBottomMargin(0.35);

    pad1->Draw();
    pad2->Draw();

    ///////////////////
    // Main plot
    ///////////////////

    pad1->cd();

    hData->SetMarkerStyle(kFullCircle);
    hData->Draw();
    gr->SetLineColor(kRed);
    gr->SetLineWidth(2);
    gr->Draw("same");
    hData->GetXaxis()->SetLabelSize(0.0001);
    hData->GetYaxis()->SetLabelSize(0.05);
    hData->GetYaxis()->SetTitle("Number of events");
    hData->GetYaxis()->SetTitleSize(0.05);
    hData->GetYaxis()->SetTitleOffset(0.9);

    double mY4S = 10579.4;
    double y = gr->Eval(mY4S);
    TLine* line = new TLine(mY4S, 0, mY4S, y);
    line->SetLineColor(kGreen);
    line->SetLineWidth(2);
    line->Draw();



    TLegend* leg = new TLegend(.15, .4, .35, .87);
    int i = 0, nPars = 0;
    for (auto p : pars) {
      double err = sqrt(mat(i, i));
      if (err != 0) {
        int nDig = log10(p.second / err) + 2;

        TString s   = "%s = %." + TString(Form("%d", nDig)) + "g";
        TString dig = "%." + TString(Form("%d", nDig)) + "g";
        TString digE = "%.2g";
        leg->AddEntry((TObject*)0, Form("%s = " + dig + " #pm " + digE, p.first.c_str(), p.second, err), "h");
        ++nPars;
      }
      ++i;
    }
    leg->SetTextSize(0.05);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->Draw();


    double chi2 = 0;
    for (int j = 1; j <= hPull->GetNbinsX(); ++j)
      chi2 += pow(hPull->GetBinContent(j), 2);
    int ndf = hPull->GetNbinsX() - nPars - 1;


    TLegend* leg2 = new TLegend(.73, .75, .93, .87);
    leg2->AddEntry((TObject*)0, Form("chi2/ndf = %.2f", chi2 / ndf), "h");
    leg2->AddEntry((TObject*)0, Form("p = %.2g",   TMath::Prob(chi2, ndf)), "h");

    leg2->SetTextSize(0.05);
    leg2->SetBorderSize(0);
    leg2->SetFillStyle(0);
    leg2->Draw();


    double mFit = pars.at("m0");
    double yF = gr->Eval(mFit);
    TLine* lineR = new TLine(mFit, 0, mFit, yF);
    lineR->SetLineColor(kRed);
    lineR->SetLineWidth(2);
    lineR->Draw();



    ///////////////////
    // Ratio plot
    ///////////////////

    pad2->cd();
    hPull->SetMarkerStyle(kFullCircle);
    hPull->Draw("p");

    hPull->GetXaxis()->SetTitle("M (#mu#mu) [MeV]");
    hPull->GetYaxis()->SetTitle("pull");
    hPull->GetXaxis()->SetTitleSize(0.13);
    hPull->GetXaxis()->SetTitleOffset(1.25);
    hPull->GetXaxis()->SetLabelSize(0.13);
    hPull->GetXaxis()->SetLabelOffset(0.05);
    hPull->GetXaxis()->SetTickSize(0.07);


    hPull->GetYaxis()->SetTitleSize(0.13);
    hPull->GetYaxis()->SetLabelSize(0.13);
    hPull->GetYaxis()->SetTitleOffset(0.2);
    hPull->GetYaxis()->CenterTitle();


    hPull->GetYaxis()->SetNdivisions(404);

    hPull->GetYaxis()->SetRangeUser(-5, 5);

    TGraph* grLine = new TGraph(2);
    grLine->SetPoint(0, hPull->GetBinLowEdge(1), 0);
    grLine->SetPoint(1, hPull->GetBinLowEdge(hPull->GetNbinsX()) + hPull->GetBinWidth(hPull->GetNbinsX()), 0);
    grLine->SetLineWidth(2);
    grLine->SetLineColor(kRed);
    grLine->Draw("same");

    std::filesystem::create_directories("plotsMuMu");

    can->SaveAs(Form("plotsMuMu/mumu_%d.pdf", time));


    delete leg;
    delete leg2;
    delete line;
    delete lineR;
    delete grLine;

    delete pad1;
    delete pad2;
    delete can;


    gROOT->SetBatch(isBatch);
  }

/// plots the result of the fit to the Mmumu, i.e. data and the fitted curve
  static void plotMuMuFit(const std::vector<double>& data, const Pars& pars, Eigen::MatrixXd mat, double mMin, double mMax, int time)
  {
    const int nBins = 100;

    // Fill the data histogram
    TH1D::SetDefaultSumw2();
    TH1D* hData = new TH1D("hData", "", nBins, mMin, mMax);
    TH1D* hFit  = new TH1D("hFit", "", nBins, mMin, mMax);
    TH1D* hPull = new TH1D("hPull", "", nBins, mMin, mMax);
    hData->SetDirectory(nullptr);
    hFit->SetDirectory(nullptr);
    hPull->SetDirectory(nullptr);

    // fill histogram with data
    for (auto d : data)
      hData->Fill(d);


    // construct the fitted function
    TGraph* gr = new TGraph();
    const double step = (mMax - mMin) / (nBins);

    for (int i = 0; i <= 2 * nBins; ++i) {
      double m = mMin + 0.5 * step * i;
      double V = mainFunction(m, pars);
      gr->SetPoint(gr->GetN(), m, V);
    }


    // Calculate integrals of the fitted function within each bin
    for (int i = 0; i < nBins; ++i) {
      double lV = gr->GetPointY(2 * i + 0);
      double cV = gr->GetPointY(2 * i + 1);
      double rV = gr->GetPointY(2 * i + 2);

      double I = step / 6 * (lV + 4 * cV + rV);
      hFit->SetBinContent(i + 1, I);
    }

    //Normalization factor
    double F = hData->Integral() / hFit->Integral();

    hFit->Scale(F);

    // Normalize the curve
    for (int i = 0; i < gr->GetN(); ++i)
      gr->SetPointY(i, gr->GetPointY(i) * F * step);


    // calculate pulls
    for (int i = 1; i <= nBins; ++i) {
      double pull = (hData->GetBinContent(i) - hFit->GetBinContent(i)) / sqrt(hFit->GetBinContent(i));
      hPull->SetBinContent(i, pull);
    }


    plotMuMuFitBase(hData, gr, hPull, pars, mat, time);

    delete hData;
    delete hFit;
    delete hPull;
    delete gr;
  }




  /** run the collision invariant mass calibration,
      it returns (eCMS, eCMSstatUnc, 0) */
  std::pair<Pars, MatrixXd> getInvMassPars(const std::vector<Event>& evts, Pars pars, double mMin, double mMax, int bootStrap = 0)
  {
    bool is4S = evts[0].is4S;

    std::vector<double> dataNow = readEvents(evts, 0.9/*PIDcut*/, mMin, mMax);


    // do bootStrap
    std::vector<double> data;
    TRandom3* rand = nullptr;
    if (bootStrap) rand = new TRandom3(bootStrap);
    for (auto d : dataNow) {
      int nP = bootStrap ? rand->Poisson(1) : 1;
      for (int i = 0; i < nP; ++i)
        data.push_back(d);
    }

    if (bootStrap)
      delete rand;

    ChebFitter fitter;
    fitter.setDataAndFunction(mainFunction, data);
    fitter.init(256 + 1, mMin, mMax);


    Pars pars0_4S = {
      {"C", 15        },
      {"bDelta", 1.60307        },
      {"bMean", 0        },
      {"frac", 0.998051        },
      {"m0", 10570.2        },
      {"mean", 4.13917        },
      {"sigma", 37.0859        },
      {"slope", 0.876812        },
      {"tau", 99.4225}
    };

    Pars pars0_Off = {
      {"C", 15        },
      {"bDelta", 2.11        },
      {"bMean", 0        },
      {"frac", 0.9854        },
      {"m0", mMax - 230     },
      {"mean", 4.13917        },
      {"sigma", 36.4         },
      {"slope", 0.892           },
      {"tau", 64.9}
    };

    if (pars.empty()) {
      pars = is4S ? pars0_4S : pars0_Off;
    }



    Limits limits = {
      {"mean",   std::make_pair(0, 0)},
      {"sigma",  std::make_pair(10, 120)},
      {"bMean",  std::make_pair(0, 0)},
      {"bDelta", std::make_pair(0.01, 10.)},
      {"tau",    std::make_pair(20, 250)},
      {"frac",   std::make_pair(0.00, 1.0)},

      {"m0",    std::make_pair(10450, 10950)},
      {"slope", std::make_pair(0.3, 0.999)},
      {"C",     std::make_pair(0, 0)}
    };


    return fitter.fitData(pars, limits, true/*useCheb*/);

  }




  // Returns tuple with the invariant mass parameters (cmsEnergy in GeV)
  std::tuple<std::vector<VectorXd>, std::vector<MatrixXd>, MatrixXd>  runMuMuInvariantMassAnalysis(std::vector<Event> evts,
      const std::vector<double>& splitPoints)
  {
    int n = splitPoints.size() + 1;

    std::vector<VectorXd>     invMassVec(n);
    std::vector<MatrixXd>  invMassVecUnc(n);
    MatrixXd          invMassVecSpred;

    std::ofstream mumuTextOut("mumuEcalib.txt", std::ios::app);
    static int iPrint = 0;
    if (iPrint == 0)
      mumuTextOut << "n   id    t1   t2    exp1   run1     exp2  run2    Ecms   EcmsUnc   state" << std::endl;
    ++iPrint;

    for (int iDiv = 0; iDiv < n; ++iDiv) {


      invMassVec[iDiv].resize(1);       //1D vector for center of the 1D Gauss
      invMassVecUnc[iDiv].resize(1, 1); //1x1 matrix covariance mat of the center
      invMassVecSpred.resize(1, 1);  //1x1 matrix for spread of the 1D Gauss

      std::vector<Event> evtsNow;
      for (auto ev :  evts) {
        double tMin = (iDiv != 0)   ? splitPoints[iDiv - 1] : -1e40;
        double tMax = (iDiv != n - 1) ? splitPoints[iDiv]   :  1e40;
        if (tMin <= ev.t && ev.t < tMax)
          evtsNow.push_back(ev);

      }



      // default fitting range for the mumu invariant mass
      double mMin = 10.2e3, mMax = 10.8e3;

      // in case of offResonance runs adjust limits from median
      if (!evtsNow[0].is4S) {
        std::vector<double> dataNow;
        for (const auto& ev : evtsNow)
          dataNow.push_back(ev.m);
        double mMedian = 1e3 * Belle2::BoostVectorCalib::median(dataNow.data(), dataNow.size());
        double est = mMedian + 30;
        mMax = est + 220;
        mMin = est - 380;
      }




      // number of required successful bootstrap replicas
      const int nRep = 25;


      std::vector<double> vals, errs;
      for (int rep = 0; rep < 200; ++rep) {
        double errEst = 50. / sqrt(evtsNow.size());

        Pars resP, inDummy;
        MatrixXd resM;


        // fit using bootstrap replica replicas, rep=0 is no replica
        tie(resP, resM) =  getInvMassPars(evtsNow, inDummy, mMin, mMax, rep);

        int ind = distance(resP.begin(), resP.find("m0"));
        double mass = resP.at("m0");
        double err  = sqrt(resM(ind, ind));


        // if there are problems with fit, try again with diffrent bootstrap replica
        if (!(errEst < err && err < 4 * errEst))
          continue;

        vals.push_back(mass);
        errs.push_back(err);

        // if now bootstrapping needed, plot & break
        if (rep == 0) {
          plotMuMuFit(readEvents(evtsNow, 0.9/*PIDcut*/, mMin, mMax), resP, resM, mMin, mMax, int(round(evtsNow[0].t)));
          break;
        }

        // try fit with different input parameters, but without bootstrapping
        Pars resP0;
        MatrixXd resM0;
        tie(resP0, resM0) =  getInvMassPars(evtsNow, resP, mMin, mMax, 0);

        int ind0 = distance(resP0.begin(), resP0.find("m0"));
        double mass0 = resP0.at("m0");
        double err0  = sqrt(resM0(ind0, ind0));

        // if successful, plot & break
        if (errEst < err0 && err0 < 4 * errEst) {
          vals = {mass0};
          errs = {err0};
          plotMuMuFit(readEvents(evtsNow, 0.9/*PIDcut*/, mMin, mMax), resP0, resM0, mMin, mMax, int(round(evtsNow[0].t)));
          break;
        }


        // if the fit was sucesfull several times only on replicas, plot & break
        if (vals.size() >= nRep) {
          plotMuMuFit(readEvents(evtsNow, 0.9/*PIDcut*/, mMin, mMax), resP, resM, mMin, mMax, int(round(evtsNow[0].t)));
          break;
        }

      }

      if (vals.size() != 1 && vals.size() != nRep)
        B2FATAL("Inconsistency of number of results with number of replicas");

      double meanMass    = accumulate(vals.begin(), vals.end(), 0.) / vals.size();
      double meanMassUnc = accumulate(errs.begin(), errs.end(), 0.) / errs.size();

      double sum2 = 0;
      for (auto v : vals)
        sum2 += pow(v - meanMass, 2);
      double errBootStrap = vals.size() > 1 ? sqrt(sum2 / (vals.size() - 1)) : 0;

      mumuTextOut << n << " " << iDiv << " " << std::setprecision(14) << evtsNow.front().t << " " << evtsNow.back().t << " " <<
                  evtsNow.front().exp << " " << evtsNow.front().run << " " << evtsNow.back().exp << " " << evtsNow.back().run  <<   "  " << meanMass
                  <<
                  "   " << meanMassUnc << " " << errBootStrap <<   std::endl;

      // Convert to GeV
      invMassVec[iDiv](0) = meanMass / 1e3;
      invMassVecUnc[iDiv](0, 0) = meanMassUnc / 1e3;
      invMassVecSpred(0, 0) = 0;
    }

    mumuTextOut.close();

    return std::make_tuple(invMassVec, invMassVecUnc, invMassVecSpred);
  }

}
