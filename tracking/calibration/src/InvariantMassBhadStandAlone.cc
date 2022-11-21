/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/



#include <iostream>
#include <iomanip>
#include <vector>
#include <filesystem>

#include <Eigen/Dense>

#include <TROOT.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TLine.h>
#include <Math/Functor.h>
#include <Math/SpecFuncMathCore.h>
#include <Math/DistFunc.h>


#include <RooRealVar.h>
#include <RooDataSet.h>
#include <RooGaussian.h>
#include <RooPlot.h>
#include <RooHist.h>
#include <RooAddPdf.h>
#include <RooPolyVar.h>
#include <RooArgusBG.h>
#include <RooSimultaneous.h>
#include <RooCategory.h>
#include <RooArgSet.h>
#include <RooConstVar.h>

#include <framework/particledb/EvtGenDatabasePDG.h>


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


namespace Belle2::InvariantMassBhadCalib {



  /// read events from TTree to std::vector
  std::vector<Event> getEvents(TTree* tr)
  {

    std::vector<Event> events;
    events.reserve(tr->GetEntries());

    Event evt;

    tr->SetBranchAddress("run", &evt.run);
    tr->SetBranchAddress("exp", &evt.exp);
    tr->SetBranchAddress("event", &evt.evtNo);
    tr->SetBranchAddress("time", &evt.t); //time in hours


    std::vector<double>* pBcms  = new std::vector<double>;
    std::vector<double>* mB     = new std::vector<double>;
    std::vector<int>*    pdg    = new std::vector<int>;
    std::vector<int>*    mode   = new std::vector<int>;
    std::vector<double>* Kpid   = new std::vector<double>;
    std::vector<double>* R2     = new std::vector<double>;
    std::vector<double>* mD     = new std::vector<double>;
    std::vector<double>* dmDstar = new std::vector<double>;



    tr->SetBranchAddress("pBcms", &pBcms);
    tr->SetBranchAddress("mB", &mB);
    tr->SetBranchAddress("pdg", &pdg);
    tr->SetBranchAddress("mode", &mode);
    tr->SetBranchAddress("Kpid", &Kpid);
    tr->SetBranchAddress("R2", &R2);
    tr->SetBranchAddress("mD", &mD);
    tr->SetBranchAddress("dmDstar", &dmDstar);




    for (int i = 0; i < tr->GetEntries(); ++i) {
      tr->GetEntry(i);

      int nCand = mode->size();
      evt.cand.resize(nCand);

      for (int j = 0; j < nCand; ++j) {
        evt.cand[j].pBcms  = pBcms->at(j);
        evt.cand[j].mB     = mB->at(j);
        evt.cand[j].pdg    = pdg->at(j);
        evt.cand[j].mode   = mode->at(j);
        evt.cand[j].Kpid   = Kpid->at(j);
        evt.cand[j].R2     = R2->at(j);
        evt.cand[j].mD     = mD->at(j);
        evt.cand[j].dmDstar = dmDstar->at(j);

        evt.cand[j].isSig = true;
      }

      evt.nBootStrap = 1;
      //evt.isSig = true;
      events.push_back(evt);
    }

    //sort by time
    sort(events.begin(), events.end(), [](Event e1, Event e2) {return e1.t < e2.t;});


    return events;
  }



  void plotArgusFit(RooDataSet* dataE0, RooAddPdf&  sumB0, RooArgusBG& argus, RooGaussian& gauss, RooRealVar& eNow,
                    TString fName = "")
  {
    // switch to the batch mode and store the current setup
    bool isBatch = gROOT->IsBatch();
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

    TLine* l = new TLine(5279.34e-3, 0.0, 5.37, 0.0);
    l->SetLineColor(kBlue);
    l->SetLineWidth(3);
    l->Draw();


    if (fName != "") c1->SaveAs(fName);


    delete hpull;
    delete ll;
    delete l;
    delete pad1;
    delete pad2;

    delete c1;


    gROOT->SetBatch(isBatch);
  }



  std::map<TString, std::pair<double, double>> argusFit(const std::vector<Event>& evts,
                                                        std::vector<std::pair<double, double>> limits)
  {

    const double cMBp = EvtGenDatabasePDG::Instance()->GetParticle("B+")->Mass();
    const double cMB0 = EvtGenDatabasePDG::Instance()->GetParticle("B0")->Mass();

    using namespace RooFit;

    RooRealVar eNow("eNow", "E^{*}_{B} [GeV]", cMBp, 5.37);


    RooDataSet* dataE0 = new RooDataSet("dataE0", "dataE0", RooArgSet(eNow));
    RooDataSet* dataEp = new RooDataSet("dataEp", "dataEp", RooArgSet(eNow));


    TH1D* hDeltaE = new TH1D(rn(), "", 30, -0.05, 0.05);
    TH1D* hMD     = new TH1D(rn(), "", 30, 1.7, 1.9);
    TH1D* hMB      = new TH1D(rn(), "", 30, -0.15, 0.15);



    B2ASSERT("Assert the existence of the Y4S particle data", EvtGenDatabasePDG::Instance()->GetParticle("Upsilon(4S)"));

    const double cmsE0 = EvtGenDatabasePDG::Instance()->GetParticle("Upsilon(4S)")->Mass(); //Y4S mass

    int nCand = 0, nEv = 0;

    for (auto event : evts) {
      int iCand = 0;
      for (auto cand : event.cand) {

        double p = cand.pBcms;
        double mInv = cand.mB;

        //get mass of B+- or B0
        double mB = EvtGenDatabasePDG::Instance()->GetParticle(abs(cand.pdg))->Mass();


        // Filling the events
        if (1.830 < cand.mD && cand.mD < 1.894)
          if (abs(mInv - mB) < 0.05)
            if (cand.R2 < 0.3)
              if ((cand.dmDstar < -10)  || (0.143 < cand.dmDstar && cand.dmDstar < 0.147)) {
                double eBC = sqrt(p * p + pow(mB, 2)); // beam constrained energy
                if (eBC > 5.37) continue;

                for (unsigned i = 0; i < limits.size(); ++i) {
                  if (limits[i].first <= event.t && event.t < limits[i].second) {
                    if (abs(cand.pdg) == 511) {
                      eNow.setVal(eBC);
                      dataE0->add(RooArgSet(eNow));
                    } else {
                      eNow.setVal(eBC);
                      dataEp->add(RooArgSet(eNow));
                    }
                    ++nCand;
                    if (iCand == 0) ++nEv;
                  }
                }

                hDeltaE->Fill(eBC - cmsE0 / 2);
                hMD->Fill(cand.mD);
                hMB->Fill(mInv - mB);
              }
        ++iCand;
      }
    }



    RooCategory Bcharge("sample", "sample") ;
    Bcharge.defineType("B0") ;
    Bcharge.defineType("Bp") ;

    RooDataSet combData("combData", "combined data", eNow, Index(Bcharge), Import("B0", *dataE0), Import("Bp", *dataEp)) ;


    // --- Build Gaussian signal PDF ---
    RooRealVar sigmean("Mean", "B^{#pm} mass", 5.29, 5.27, 5.30) ;
    RooRealVar sigwidth("#sigma", "B^{#pm} width", 0.00237, 0.0001, 0.030) ;
    //sigwidth.setConstant(kTRUE);


    RooPolyVar sigmeanNow("sigmeanNow", "shape parameter", sigmean, RooArgSet(RooConst(0.000), RooConst(1.)));
    RooGaussian gauss("gauss", "gaussian PDF", eNow, sigmeanNow, sigwidth) ;

    // --- Build Argus background PDF ---
    RooRealVar argpar("Argus_param", "argus shape parameter", -150.7, -300., +50.0) ;
    RooRealVar endpointBp("EndPointBp", "endPoint parameter", cMBp, 5.27, 5.291) ; //B+ value
    RooRealVar endpointB0("EndPointB0", "endPoint parameter", cMB0, 5.27, 5.291) ; //B0 value
    endpointB0.setConstant(kTRUE);
    endpointBp.setConstant(kTRUE);


    //B0 pars
    RooPolyVar shape2B0("EndPoint2B0", "shape parameter", endpointB0, RooArgSet(RooConst(0.), RooConst(2.)));
    RooPolyVar eNowDifB0("eNowDifB0", "eNowDifB0", eNow, RooArgSet(shape2B0, RooConst(-1.)));
    RooArgusBG argusB0("argusB0", "Argus PDF", eNowDifB0, endpointB0, argpar) ;

    //Bp pars
    RooPolyVar shape2Bp("EndPoint2Bp", "shape parameter", endpointBp, RooArgSet(RooConst(0.), RooConst(2.)));
    RooPolyVar eNowDifBp("eNowDifBp", "eNowDifBp", eNow, RooArgSet(shape2Bp, RooConst(-1.)));
    RooArgusBG argusBp("argusBp", "Argus PDF", eNowDifBp, endpointBp, argpar) ;


    // --- Construct signal+background PDF ---
    RooRealVar nsigB0("nsigB0", "#signal events", 100, 0., 100000) ;
    RooRealVar nbkgB0("nbkgB0", "#background events", 100, 0., 500000) ;

    RooRealVar nsigBp("nsigBp", "#signal events", 100, 0., 100000) ;
    RooRealVar nbkgBp("nbkgBp", "#background events", 100, 0., 500000) ;



    RooAddPdf sumB0("sumB0", "g0+a0", RooArgList(gauss, argusB0), RooArgList(nsigB0, nbkgB0)) ;
    RooAddPdf sumBp("sumBp", "gP+aP", RooArgList(gauss, argusBp), RooArgList(nsigBp, nbkgBp)) ;


    // Construct a simultaneous pdf using category sample as index
    RooSimultaneous simPdf("simPdf", "simultaneous pdf", Bcharge) ;

    // Associate model with the physics state and model_ctl with the control state
    simPdf.addPdf(sumB0,  "B0") ;
    simPdf.addPdf(sumBp,  "Bp") ;

    simPdf.fitTo(combData);


    std::map<TString, std::pair<double, double>> resMap;
    resMap["sigmean"]  = {sigmean.getValV(),  sigmean.getError()};
    resMap["sigwidth"] = {sigwidth.getValV(), sigwidth.getError()};
    resMap["argpar"]   = {argpar.getValV(), argpar.getError()};

    std::filesystem::create_directories("plotsHadBonly");

    plotArgusFit(dataE0, sumB0, argusB0, gauss, eNow, Form("plotsHadBonly/B0Single_%d.pdf", int(round(limits[0].first))));
    plotArgusFit(dataEp, sumBp, argusBp, gauss, eNow, Form("plotsHadBonly/BpSingle_%d.pdf", int(round(limits[0].first))));


    // Delete rooFit objects
    delete dataE0;
    delete dataEp;


    delete hDeltaE;
    delete hMD;
    delete hMB;

    return resMap;
  }






// Analysis itself
  std::map<TString, std::pair<double, double>> argusFitConstrained(const std::vector<Event>& evts,
                                            std::vector<std::pair<double, double>> limits,
                                            std::vector<std::pair<double, double>> mumuVals,  std::vector<double> startPars)
  {
    // Calculate eCMS/2
    for (auto& el : mumuVals) {
      el.first  /= 2;
      el.second /= 2;
    }

    //starting values are from the Bonly fit
    const double meanInit   = startPars[0] / 2;
    const double sigmaInit  = startPars[2] / 2;
    const double argparInit = startPars[4];

    double s = 0, sw = 0;
    for (auto p : mumuVals) {
      s  += p.first / pow(p.second, 2);
      sw += 1. / pow(p.second, 2);
    }
    const double mumuMean = s / sw;
    const double shiftInit = meanInit - mumuMean;


    const double cMBp = EvtGenDatabasePDG::Instance()->GetParticle("B+")->Mass();
    const double cMB0 = EvtGenDatabasePDG::Instance()->GetParticle("B0")->Mass();

    using namespace RooFit;

    RooRealVar eNow("eNow", "E^{*}_{B} [GeV]", cMBp, 5.37);

    std::vector<RooDataSet*> dataE0(limits.size());
    std::vector<RooDataSet*> dataEp(limits.size());

    for (unsigned i = 0; i < limits.size(); ++i) {
      dataE0[i] = new RooDataSet(Form("dataE0_%u", i), Form("dataE0_%u", i), RooArgSet(eNow));
      dataEp[i] = new RooDataSet(Form("dataEp_%u", i), Form("dataEp_%u", i), RooArgSet(eNow));
    }


    TH1D* hDeltaE = new TH1D(rn(), "", 30, -0.05, 0.05);
    TH1D* hMD     = new TH1D(rn(), "", 30, 1.7, 1.9);
    TH1D* hMB      = new TH1D(rn(), "", 30, -0.15, 0.15);


    B2ASSERT("Assert the existence of the Y4S particle data", EvtGenDatabasePDG::Instance()->GetParticle("Upsilon(4S)"));

    const double cmsE0 = EvtGenDatabasePDG::Instance()->GetParticle("Upsilon(4S)")->Mass(); //Y4S mass

    int nCand = 0, nEv = 0;

    for (auto event : evts) {
      int iCand = 0;
      for (auto cand : event.cand) {

        double p = cand.pBcms;
        double mInv = cand.mB;

        //get mass of B+- or B0
        double mB = EvtGenDatabasePDG::Instance()->GetParticle(abs(cand.pdg))->Mass();


        // Filling the events
        if (1.830 < cand.mD && cand.mD < 1.894)
          if (abs(mInv - mB) < 0.05)
            if (cand.R2 < 0.3)
              if ((cand.dmDstar < -10)  || (0.143 < cand.dmDstar && cand.dmDstar < 0.147)) {
                double eBC = sqrt(p * p + pow(mB, 2)); // beam constrained energy
                if (eBC > 5.37) continue;

                for (unsigned i = 0; i < limits.size(); ++i) {
                  if (limits[i].first <= event.t && event.t < limits[i].second) {
                    if (abs(cand.pdg) == 511) {
                      eNow.setVal(eBC);
                      dataE0[i]->add(RooArgSet(eNow));
                    } else {
                      eNow.setVal(eBC);
                      dataEp[i]->add(RooArgSet(eNow));
                    }
                    ++nCand;
                    if (iCand == 0) ++nEv;
                  }
                }

                hDeltaE->Fill(eBC - cmsE0 / 2);
                hMD->Fill(cand.mD);
                hMB->Fill(mInv - mB);
              }
        ++iCand;
      }
    }



    RooCategory Bcharge("sample", "sample") ;
    for (unsigned i = 0; i < limits.size(); ++i) {
      Bcharge.defineType(Form("B0_%u", i));
      Bcharge.defineType(Form("Bp_%u", i));
    }

    RooDataSet* combData = nullptr;

    std::map<std::string, RooDataSet*> dataSetMap;
    for (unsigned i = 0; i < limits.size(); ++i) {
      dataSetMap[Form("B0_%u", i)] = dataE0[i];
      dataSetMap[Form("Bp_%u", i)] = dataEp[i];
    }
    combData = new RooDataSet("combData", "combined data", eNow, Index(Bcharge), Import(dataSetMap));


    // --- Build Gaussian signal PDF ---

    RooRealVar sigwidth("#sigma", "width of B-meson energy in CMS",  sigmaInit, 0.0001, 0.030) ;

    std::vector<RooRealVar*> sigmean(limits.size());
    std::vector<RooGaussian*> gauss(limits.size());



    for (unsigned i = 0; i < limits.size(); ++i) {
      sigmean[i] = new RooRealVar(Form("Mean_%u", i), "mean B-meson energy in CMS", meanInit, 5.27, 5.30) ;
      gauss[i]   = new RooGaussian(Form("gauss_%u", i), "gaussian PDF", eNow, *sigmean[i], sigwidth) ;
    }

    // --- Build Argus background PDF ---


    RooRealVar argpar("Argus_param", "argus shape parameter", argparInit, -300., +50.0) ;
    RooRealVar endpointBp("EndPointBp", "endPoint parameter", cMBp, 5.27, 5.291) ; //B+ value
    RooRealVar endpointB0("EndPointB0", "endPoint parameter", cMB0, 5.27, 5.291) ; //B0 value
    endpointB0.setConstant(kTRUE);
    endpointBp.setConstant(kTRUE);


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


    std::vector<RooRealVar*> nsigB0(limits.size());
    std::vector<RooRealVar*> nbkgB0(limits.size());

    std::vector<RooRealVar*> nsigBp(limits.size());
    std::vector<RooRealVar*> nbkgBp(limits.size());

    std::vector<RooAddPdf*> sumB0(limits.size());
    std::vector<RooAddPdf*> sumBp(limits.size());


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

    RooRealVar shift("shift", "shift to mumu", shiftInit, -30e-3, 30e-3);

    std::vector<RooGaussian*> fconstraint(limits.size());
    std::vector<RooPolyVar*> shiftNow(limits.size());

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


    std::map<TString, std::pair<double, double>> resMap;
    for (unsigned i = 0; i < limits.size(); ++i) {
      resMap[Form("sigmean_%u", i)]  = {sigmean[i]->getValV(),  sigmean[i]->getError()};
      resMap[Form("pull_%u", i)]  = {  (sigmean[i]->getValV() - shift.getValV() - mumuVals[i].first) / mumuVals[i].second,  0};
    }

    resMap["sigwidth"] = {sigwidth.getValV(), sigwidth.getError()};
    resMap["argpar"]   = {argpar.getValV(), argpar.getError()};
    resMap["shift"]    = {shift.getValV(), shift.getError()};


    namespace fs = std::filesystem;
    fs::create_directories("plotsHadB");

    for (unsigned i = 0; i < limits.size(); ++i) {
      plotArgusFit(dataE0[i], *sumB0[i], argusB0, *gauss[i], eNow, Form("plotsHadB/B0_%d_%u.pdf", int(round(limits[0].first)), i));
      plotArgusFit(dataEp[i], *sumBp[i], argusBp, *gauss[i], eNow, Form("plotsHadB/Bp_%d_%u.pdf", int(round(limits[0].first)), i));
    }


    // Delete rooFit objects
    for (unsigned i = 0; i < limits.size(); ++i) {
      delete dataE0[i];
      delete dataEp[i];

      delete sigmean[i];
      delete gauss[i];

      delete nsigB0[i];
      delete nbkgB0[i];
      delete nsigBp[i];
      delete nbkgBp[i];
      delete sumB0[i];
      delete sumBp[i];

      delete shiftNow[i];
      delete fconstraint[i];
    }

    delete combData;

    delete hDeltaE;
    delete hMD;
    delete hMB;


    return resMap;
  }






  std::vector<std::vector<double>> doBhadFit(const std::vector<Event>& evts, std::vector<std::pair<double, double>> limits,
                                             std::vector<std::pair<double, double>> mumuVals,  const std::vector<double>& startPars)
  {

    auto r = argusFitConstrained(evts, limits, mumuVals, startPars);
    assert(limits.size() == mumuVals.size());

    std::vector<std::vector<double>> result(limits.size());


    for (unsigned i = 0; i < limits.size(); ++i) {
      std::string n  = Form("sigmean_%u", i);
      std::string np = Form("pull_%u", i);

      //convert to whole eCMS (ecms, ecmsSpread, ecmsShift)
      result[i] = {2 * r.at(n).first,  2 * r.at(n).second,  2 * r.at("sigwidth").first, 2 * r.at("sigwidth").second, 2 * r.at("shift").first, 2 * r.at("shift").second, r.at(np).first};
    }

    return result;

  }


  std::vector<double> doBhadOnlyFit(const std::vector<Event>& evts, const std::vector<std::pair<double, double>>& limits)
  {

    auto r = argusFit(evts, limits);

    return {2 * r.at("sigmean").first,  2 * r.at("sigmean").second,  2 * r.at("sigwidth").first, 2 * r.at("sigwidth").second, r.at("argpar").first};

  }



}
