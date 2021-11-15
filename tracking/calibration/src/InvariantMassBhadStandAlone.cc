/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/



#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <tuple>
#include <numeric>
#include <filesystem>

#include <TTree.h>
#include <TVector3.h>
#include <Eigen/Dense>


#include "TGraph.h"
#include "TLine.h"
#include "TH1D.h"
#include "Math/Functor.h"
#include "Math/SpecFuncMathCore.h"
#include "Math/DistFunc.h"
#include "TCanvas.h"
#include "TROOT.h"


#include "RooRealVar.h"
#include "RooDataSet.h"
//#include "RooDataHist.h"
#include "RooGaussian.h"
#include "RooPlot.h"
#include "RooHist.h"
#include "RooAddPdf.h"
//#include "RooPolynomial.h"
#include "RooPolyVar.h"
#include "RooArgusBG.h"
#include "RooSimultaneous.h"
#include "RooCategory.h"
#include "RooArgSet.h"
#include "RooConstVar.h"


#include "framework/particledb/EvtGenDatabasePDG.h"


//if compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/InvariantMassBhadStandAlone.h>
#include <tracking/calibration/Splitter.h>
#include <tracking/calibration/tools.h>
#else
#include <InvariantMassBhadStandAlone.h>
#include <Splitter.h>
#include <tools.h>
#endif

using Eigen::MatrixXd;
using Eigen::VectorXd;

using namespace std;

namespace Belle2 {

  namespace InvariantMassBhadCalib {




    /// read events from TTree to std::vector
    vector<Event> getEvents(TTree* tr)
    {

      vector<Event> events;
      events.reserve(tr->GetEntries());

      Event evt;

      tr->SetBranchAddress("run", &evt.run);
      tr->SetBranchAddress("exp", &evt.exp);
      tr->SetBranchAddress("event", &evt.evtNo);
      tr->SetBranchAddress("time", &evt.t); //time in hours


      tr->SetBranchAddress("mBC", &evt.mBC);
      tr->SetBranchAddress("deltaE", &evt.deltaE);
      tr->SetBranchAddress("pdg", &evt.pdg);
      tr->SetBranchAddress("mode", &evt.mode);
      tr->SetBranchAddress("Kpid", &evt.Kpid);
      tr->SetBranchAddress("R2", &evt.R2);
      tr->SetBranchAddress("mD", &evt.mD);
      tr->SetBranchAddress("dmDstar", &evt.dmDstar);





      for (int i = 0; i < tr->GetEntries(); ++i) {
        tr->GetEntry(i);

        evt.nBootStrap = 1;
        evt.isSig = true;
        events.push_back(evt);
      }

      //sort by time
      sort(events.begin(), events.end(), [](Event e1, Event e2) {return e1.t < e2.t;});

      return events;
    }



    void plotArgusFit(RooDataSet* dataE0, RooAddPdf&  sumB0, RooArgusBG& argus, RooGaussian& gauss, RooRealVar& eNow,
                      TString fName = "")
    {
      gROOT->SetBatch(kTRUE);
      using namespace RooFit ;

      // --- Plot toy data and composite PDF overlaid ---
      RooPlot* mbcframe = eNow.frame(40) ;
      dataE0->plotOn(mbcframe) ;

      sumB0.paramOn(mbcframe, dataE0);


      sumB0.plotOn(mbcframe, Components(argus), LineStyle(kDashed)) ;
      sumB0.plotOn(mbcframe, Components(gauss), LineStyle(kDashed), LineColor(kRed)) ;

      sumB0.plotOn(mbcframe);

      mbcframe->GetXaxis()->SetTitleSize(0.0001);
      mbcframe->GetXaxis()->SetLabelSize(0.0001);
      mbcframe->SetTitleSize(0.0001);
      mbcframe->GetYaxis()->SetTitleSize(0.06);
      mbcframe->GetYaxis()->SetTitleOffset(0.7);
      mbcframe->GetYaxis()->SetLabelSize(0.06) ;

      mbcframe->SetTitle("");


      //Double_t chisq = mbcframe->chiSquare();
      RooHist* hpull = mbcframe->pullHist() ;
      hpull->Print();
      RooPlot* frame3 = eNow.frame(Title(".")) ;
      frame3->GetYaxis()->SetTitle("Pull") ;
      frame3->GetYaxis()->SetTitleSize(0.13) ;

      frame3->GetYaxis()->SetNdivisions(504) ;
      frame3->GetYaxis()->SetLabelSize(0.15) ;
      frame3->GetXaxis()->SetTitleSize(0.15) ;
      frame3->GetXaxis()->SetLabelSize(0.15) ;

      frame3->GetYaxis()->SetTitleOffset(0.3) ;
      frame3->addPlotable(hpull, "x0 P E1") ;
      frame3->SetMaximum(5.);
      frame3->SetMinimum(-5.);


      TCanvas* c1 = new TCanvas(rn()) ;
      TPad* pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
      pad1->Draw();             // Draw the upper pad: pad1
      pad1->cd();
      mbcframe->Draw() ;


      TLine* ll = new TLine;
      double mRev = 10579.4e-3 / 2;
      ll->SetLineColor(kGreen);
      ll->DrawLine(mRev, 0, mRev,  mbcframe->GetMaximum());


      c1->cd();          // Go back to the main canvas before defining pad2
      TPad* pad2 = new TPad("pad2", "pad2", 0, 0.00, 1, 0.3);
      pad2->SetTopMargin(0.05);
      pad2->SetBottomMargin(0.35);
      pad2->Draw();
      pad2->cd();
      frame3->Draw() ;
      c1->Update();

      TLine* l = new TLine(5.27, 0.0, 5.37, 0.0);
      l->SetLineColor(kBlue);
      l->SetLineWidth(3);
      l->Draw();


      if (fName != "") c1->SaveAs(fName);
    }






// Analysis itself
    map<TString, pair<double, double>> argusFitConstrained(const vector<Event>& evts, vector<pair<double, double>> limits,
                                                           vector<pair<double, double>> mumuVals)
    {
      // Calculate eCMS/2
      for (auto& el : mumuVals) {
        el.first  /= 2;
        el.second /= 2;
      }


      using namespace RooFit;

      RooRealVar eNow("eNow", "E^{*}_{B} [GeV]", 5.27, 5.37);

      vector<RooDataSet*> dataE0(limits.size());
      vector<RooDataSet*> dataEp(limits.size());

      for (unsigned i = 0; i < limits.size(); ++i) {
        dataE0[i] = new RooDataSet(Form("dataE0_%u", i), Form("dataE0_%u", i), RooArgSet(eNow));
        dataEp[i] = new RooDataSet(Form("dataEp_%u", i), Form("dataEp_%u", i), RooArgSet(eNow));
      }


      TH1D* hDeltaE = new TH1D(rn(), "", 30, -0.05, 0.05);
      TH1D* hMD     = new TH1D(rn(), "", 30, 1.7, 1.9);
      TH1D* hMB      = new TH1D(rn(), "", 30, -0.15, 0.15);


      B2ASSERT("Assert the existence of the Y4S particle data", EvtGenDatabasePDG::Instance()->GetParticle("Upsilon(4S)"));

      for (auto ev : evts) {
        const double cmsE0 = EvtGenDatabasePDG::Instance()->GetParticle("Upsilon(4S)")->Mass(); //Y4S mass
        double E = ev.deltaE + cmsE0 / 2; // energy
        double p = sqrt(pow(cmsE0 / 2, 2) - pow(ev.mBC, 2)); // energy
        double m = sqrt(E * E - p * p);
        double mB = (abs(ev.pdg) == 511) ? 5279.65e-3 : 5279.34e-3;


        //my Filler
        if (1.830 < ev.mD && ev.mD < 1.894)
          if (abs(m - mB) < 0.05)
            if (ev.R2 < 0.3)
              if ((ev.dmDstar < -10)  || (0.143 < ev.dmDstar && ev.dmDstar < 0.147)) {
                double eBC = sqrt(p * p + pow(mB, 2));
                if (eBC > 5.37) continue;

                eNow.setVal(eBC);
                for (unsigned i = 0; i < limits.size(); ++i) {
                  if (limits[i].first <= ev.t && ev.t < limits[i].second) {
                    if (abs(ev.pdg) == 511) dataE0[i]->add(RooArgSet(eNow));
                    else                   dataEp[i]->add(RooArgSet(eNow));
                  }
                }

                hDeltaE->Fill(ev.deltaE);
                hMD->Fill(ev.mD);
                hMB->Fill(m - mB);
              }
      }



      RooCategory Bcharge("sample", "sample") ;
      for (unsigned i = 0; i < limits.size(); ++i) {
        Bcharge.defineType(Form("B0_%u", i));
        Bcharge.defineType(Form("Bp_%u", i));
      }

      RooDataSet* combData = nullptr;

      map<string, RooDataSet*> dataSetMap;
      for (unsigned i = 0; i < limits.size(); ++i) {
        dataSetMap[Form("B0_%u", i)] = dataE0[i];
        dataSetMap[Form("Bp_%u", i)] = dataEp[i];
      }
      combData = new RooDataSet("combData", "combined data", eNow, Index(Bcharge), Import(dataSetMap));


      // --- Build Gaussian signal PDF ---

      //RooRealVar sigmean("Mean","B^{#pm} mass",5.29,5.27,5.30) ;
      RooRealVar sigwidth("#sigma", "B^{#pm} width", 0.00237, 0.0001, 0.030) ;
      //sigwidth.setConstant(kTRUE);

      vector<RooRealVar*> sigmean(limits.size());
      vector<RooGaussian*> gauss(limits.size());



      for (unsigned i = 0; i < limits.size(); ++i) {
        sigmean[i] = new RooRealVar(Form("Mean_%u", i), "B^{#pm} mass", 5.29, 5.27, 5.30) ;
        gauss[i]   = new RooGaussian(Form("gauss_%u", i), "gaussian PDF", eNow, *sigmean[i], sigwidth) ;
      }

      // --- Build Argus background PDF ---
      RooRealVar argpar("Argus_param", "argus shape parameter", -150.7, -300., +50.0) ;
      RooRealVar endpointBp("EndPointBp", "endPoint parameter", 5279.34e-3, 5.27, 5.291) ; //B+ value
      RooRealVar endpointB0("EndPointB0", "endPoint parameter", 5279.65e-3, 5.27, 5.291) ; //B0 value
      endpointB0.setConstant(kTRUE);
      endpointBp.setConstant(kTRUE);

      //argpar.setConstant(kTRUE);

      RooRealVar zero("zero", "", 0);
      RooRealVar two("two", "", 2);
      RooRealVar minus("minus", "", -1);

      //B0 pars
      RooPolyVar shape2B0("EndPoint2B0", "shape parameter", endpointB0, RooArgSet(zero, two));
      RooPolyVar eNowDifB0("eNowDifB0", "eNowDifB0", eNow, RooArgSet(shape2B0, minus));
      RooArgusBG argusB0("argusB0", "Argus PDF", eNowDifB0, endpointB0, argpar) ;

      //Bp pars
      RooPolyVar shape2Bp("EndPoint2Bp", "shape parameter", endpointBp, RooArgSet(zero, two));
      RooPolyVar eNowDifBp("eNowDifBp", "eNowDifBp", eNow, RooArgSet(shape2Bp, minus));
      RooArgusBG argusBp("argusBp", "Argus PDF", eNowDifBp, endpointBp, argpar) ;


      vector<RooRealVar*> nsigB0(limits.size());
      vector<RooRealVar*> nbkgB0(limits.size());

      vector<RooRealVar*> nsigBp(limits.size());
      vector<RooRealVar*> nbkgBp(limits.size());

      vector<RooAddPdf*> sumB0(limits.size());
      vector<RooAddPdf*> sumBp(limits.size());


      // --- Construct signal+background PDF ---
      for (unsigned i = 0; i < limits.size(); ++i) {
        nsigB0[i] = new RooRealVar(Form("nsigB0_%u", i), "#signal events", 100, 0., 100000);
        nbkgB0[i] = new RooRealVar(Form("nbkgB0_%u", i), "#background events", 100, 0., 500000) ;

        nsigBp[i] = new RooRealVar(Form("nsigBp_%u", i), "#signal events", 100, 0., 100000) ;
        nbkgBp[i] = new RooRealVar(Form("nbkgBp_%u", i), "#background events", 100, 0., 500000) ;

        sumB0[i] = new RooAddPdf(Form("sumB0_%u", i), "g0+a0", RooArgList(*gauss[i], argusB0), RooArgList(*nsigB0[i], *nbkgB0[i]));
        sumBp[i] = new RooAddPdf(Form("sumBp_%u", i), "gP+aP", RooArgList(*gauss[i], argusBp), RooArgList(*nsigBp[i], *nbkgBp[i]));

      }


      // Construct a simultaneous pdf using category sample as index
      RooSimultaneous simPdf("simPdf", "simultaneous pdf", Bcharge) ;

      // Associate model with the physics state and model_ctl with the control state
      for (unsigned i = 0; i < limits.size(); ++i) {
        simPdf.addPdf(*sumB0[i],  Form("B0_%u", i));
        simPdf.addPdf(*sumBp[i],  Form("Bp_%u", i));
      }

      RooRealVar shift("shift", "shift to mumu", +6e-3, -30e-3, 30e-3);
      //shift.setConstant(kTRUE);

      vector<RooGaussian*> fconstraint(limits.size());
      vector<RooPolyVar*> shiftNow(limits.size());

      for (unsigned i = 0; i < limits.size(); ++i) {
        shiftNow[i] = new RooPolyVar(Form("shiftNow_%u", i), "shiftShift", shift, RooArgSet(RooConst(mumuVals[i].first), RooConst(1.)));

        fconstraint[i] = new RooGaussian(Form("fconstraint_%u", i), "fconstraint", *sigmean[i], *shiftNow[i],
                                         RooConst(mumuVals[i].second / 1.)) ;
      }


      RooArgSet constraintSet;
      for (auto& c :  fconstraint) {
        constraintSet.add(*c);
      }
      simPdf.fitTo(*combData, ExternalConstraints(constraintSet));


      map<TString, pair<double, double>> resMap;
      for (unsigned i = 0; i < limits.size(); ++i) {
        resMap[Form("sigmean_%u", i)]  = {sigmean[i]->getValV(),  sigmean[i]->getError()};
        resMap[Form("pull_%u", i)]  = {  (sigmean[i]->getValV() - shift.getValV() - mumuVals[i].first) / mumuVals[i].second,  0};
      }

      resMap["sigwidth"] = {sigwidth.getValV(), sigwidth.getError()};
      resMap["argpar"]   = {argpar.getValV(), argpar.getError()};
      resMap["endpoint"] = {endpointB0.getValV(), endpointB0.getError()};
      resMap["shift"]    = {shift.getValV(), shift.getError()};


      namespace fs = std::filesystem;
      fs::create_directories("plotsHadB");

      for (unsigned i = 0; i < limits.size(); ++i) {
        plotArgusFit(dataE0[i], *sumB0[i], argusB0, *gauss[i], eNow, Form("plotsHadB/B0_%d_%u.pdf", int(round(limits[0].first)), i));
        plotArgusFit(dataEp[i], *sumBp[i], argusBp, *gauss[i], eNow, Form("plotsHadB/Bp_%d_%u.pdf", int(round(limits[0].first)), i));
      }


      return resMap;
    }

    vector<vector<double>> doBhadFit(const vector<Event>& evts, vector<pair<double, double>> limits,
                                     vector<pair<double, double>> mumuVals)
    {

      auto r = argusFitConstrained(evts, limits, mumuVals);
      assert(limits.size() == mumuVals.size());

      vector<vector<double>> result(limits.size());


      for (unsigned i = 0; i < limits.size(); ++i) {
        string n  = Form("sigmean_%u", i);
        string np = Form("pull_%u", i);


        //convert to whole eCMS (ecms, ecmsSpread, ecmsShift)
        result[i] = {2 * r.at(n).first,  2 * r.at(n).second,  2 * r.at("sigwidth").first, 2 * r.at("sigwidth").second, 2 * r.at("shift").first, 2 * r.at("shift").second, r.at(np).first};
      }

      return result;

    }


  }
}
