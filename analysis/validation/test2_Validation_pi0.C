/*
<header>
<input>../GenericB.ntup.root</input>
<output>test2_Validation_pi0_output.root</output>
<contact>Sam Cunliffe (sam.cunliffe@desy.de), Mario Merola (mario.merola@na.infn.it)</contact>
<description>
Check the calibration of the ECL in the MC by determining the measured pi0 invariant mass,
Fit parameter constraints need some tuning.
</description>
</header>
*/

// Authors: Phillip Urquijo, Luis Pesantez, May 2013
// Updated: Oct 2014
//          Nov 2017

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include <TChain.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TF2.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TLorentzVector.h>
#include <TMinuit.h>
#include <TNtuple.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TTree.h>

#ifndef __CINT__
#include <RooGlobalFunc.h>
#endif
#include <RooRealVar.h>
#include <RooAddPdf.h>
#include <RooCBShape.h>
#include <RooChebychev.h>
#include <RooDataHist.h>
#include <RooDataSet.h>
#include <RooGaussian.h>
#include <RooMsgService.h>
#include <RooPlot.h>

using namespace RooFit ;

void test2_Validation_pi0() {
    const char *contact = "mario.merola@na.infn.it; sam.cunliffe@desy.de";

    /* Take the pi0tuple prepared by the NtupleMaker */
    TChain * recoTree = new TChain("pi0tuple");
    recoTree->AddFile("../GenericB.ntup.root");

    //Plots for online/web validation
    TFile* output = new TFile("test2_Validation_pi0_output.root","recreate");

    //Plots used in offline validation
    /* Invariant mass after Egamma>0.05 GeV criterion */
    TH1F * h_pi0_m_cut = new TH1F("pi0_m_cut",";Mass with photon energy cut m(#pi^{0}) [GeV];N",40,0.08,0.18);
    h_pi0_m_cut->GetListOfFunctions()->Add(new TNamed("Description","pi0 invariant mass after Egamma>0.05 GeV cut. A Generic BBbar sample is used."));
    h_pi0_m_cut->GetListOfFunctions()->Add(new TNamed("Check","Stable S/B,non-empty (i.e. pi0 import to analysis modules is working),consistent mean."));
    h_pi0_m_cut->GetListOfFunctions()->Add(new TNamed("Contact",contact));

    /* Mass constrained fit value,as stored in Particle */
    TH1F * h_pi0_m_fit = new TH1F("pi0_m_fit",";Mass constrained fit m(#pi^{0}) [GeV];N",40,0.133,0.137);
    h_pi0_m_fit->GetListOfFunctions()->Add(new TNamed("Description","pi0 Mass constrained fit mass,with background. A Generic BBbar sample is used. Test may be replaced with analysis mode validation with pi0."));
    h_pi0_m_fit->GetListOfFunctions()->Add(new TNamed("Check","Stable S/B,non-empty (i.e. pi0 import to analysis modules is working),consistent mean."));
    h_pi0_m_fit->GetListOfFunctions()->Add(new TNamed("Contact",contact));

    /* Invariant mass determined from the two photon daughters */
    TH1F * h_pi0_m    = new TH1F("pi0_m",";Mass without photon energy cut m(#pi^{0}) [GeV];N",40,0.08,0.18);
    h_pi0_m->GetListOfFunctions()->Add(new TNamed("Description","pi0 Mass,with background. A Generic BBbar sample is used. Test may be replaced with analysis mode validation with pi0."));
    h_pi0_m->GetListOfFunctions()->Add(new TNamed("Check","Stable S/B,non-empty (i.e. pi0 import to analysis modules is working),consistent mean."));
    h_pi0_m->GetListOfFunctions()->Add(new TNamed("Contact",contact));
    
    TH1F * h_pi0_m_truth    = new TH1F("pi0_m",";Mass without cut m(#pi^{0}) [GeV];N",40,0.08,0.18);
    h_pi0_m_truth->GetListOfFunctions()->Add(new TNamed("Description","pi0 mass from photons,with mcErrors==0. A Generic BBbar sample is used."));
    h_pi0_m_truth->GetListOfFunctions()->Add(new TNamed("Check","Check if mean is correct. Currently photon energy is wrong and gives wrong mean."));
    h_pi0_m_truth->GetListOfFunctions()->Add(new TNamed("Contact",contact));

    /* Access the Photons and pi0 M*/
    float pi0_gamma0_P4[4];
    float pi0_gamma1_P4[4];
    float pi0_M;
    int pi0_mcErrors;
    recoTree->SetBranchAddress("pi0_gamma0_P4",&pi0_gamma0_P4);
    recoTree->SetBranchAddress("pi0_gamma1_P4",&pi0_gamma1_P4);
    recoTree->SetBranchAddress("pi0_M",&pi0_M);
    recoTree->SetBranchAddress("pi0_mcErrors",&pi0_mcErrors);

    for(Int_t iloop=0; iloop<recoTree->GetEntries(); iloop++) {
        recoTree->GetEntry(iloop);
        TLorentzVector lv_pi0_gamma0(pi0_gamma0_P4);
        TLorentzVector lv_pi0_gamma1(pi0_gamma1_P4);
        TLorentzVector lv_pi0_raw = lv_pi0_gamma0+lv_pi0_gamma1;
        float pi0_raw_M = lv_pi0_raw.M();
        h_pi0_m_fit->Fill(pi0_M);
        h_pi0_m->Fill(pi0_raw_M);
        if(lv_pi0_gamma0.E()>0.05&&lv_pi0_gamma1.E()>0.05)h_pi0_m_cut->Fill(pi0_raw_M);
        if( pi0_mcErrors<1 )h_pi0_m_truth->Fill(pi0_raw_M);
    }

    TCanvas *canvas = new TCanvas ("canvas","canvasReco",1000,800);
    canvas->Print("test2_Validation_pi0_plots.pdf[");

    // Mass constrained fit mass
    h_pi0_m_truth->SetLineColor(kGreen);
    h_pi0_m_truth->SetMinimum(0.);
    h_pi0_m_truth->Draw();
    canvas->Print("test2_Validation_pi0_plots.pdf");
    
    // Mass constrained fit mass
    h_pi0_m_fit->SetLineColor(kRed);
    h_pi0_m_fit->SetMinimum(0.);
    h_pi0_m_fit->Draw();
    canvas->Print("test2_Validation_pi0_plots.pdf");

    // Raw masses (unfit)
    h_pi0_m->SetLineColor(kRed);
    h_pi0_m->Draw();
    h_pi0_m->SetMinimum(0.);
    h_pi0_m_cut->SetLineColor(kBlue);
    h_pi0_m_cut->Draw("same");
    canvas->Print("test2_Validation_pi0_plots.pdf");

    RooRealVar *mass  =  new RooRealVar("mass","m(#pi^{0}) GeV" ,0.08,0.18);
    RooDataHist h_pi0_cut("h_pi0_cut","h_pi0_cut",*mass,h_pi0_m_cut);
    RooDataHist h_pi0_nocut("h_pi0_nocut","h_pi0_nocut",*mass,h_pi0_m);

    //pi0 signal PDF is a Crystal Ball (Gaussian also listed in case we want to switch)
    RooRealVar mean("mean","mean",0.14,0.11,0.16);
    RooRealVar sig1("#sigma","sig",0.05,0.002,1.);
    RooGaussian gau1("gau1","gau1",*mass,mean,sig1);

    RooRealVar alphacb("alphacb","alpha",1.4,0.1,1.8);
    RooRealVar ncb("ncb","n",8,2.,15);
    RooCBShape sigcb("sigcb","sig",*mass,mean,sig1,alphacb,ncb);

    //pi0 background PDF is a 2nd order Chebyshev
    RooRealVar b1("b1","b1",-3.0021e-01,-3,3);
    RooRealVar a1("a1","a1",-3.0021e-01,-3,3);
    RooArgList bList(a1,b1);
    RooChebychev bkg("bkg","pol 2",*mass,bList);

    RooArgList shapes;
    //shapes.add(gau1);
    shapes.add(sigcb);
    shapes.add(bkg);

    // Perform the extended fit to get a rough idea of yields.
    RooRealVar nsig("Nsig","Nsig",100,0,1000000);
    RooRealVar nbkg("Nbkg","Nbkg",100,0,1000000);
    RooArgList yields;
    yields.add(nsig);
    yields.add(nbkg);

    RooAddPdf totalPdf("totalpdf","",shapes,yields);

    totalPdf.fitTo(h_pi0_cut,RooFit::Extended(kTRUE),Minos(1));
    /* Fit to the unfit mass with cuts */
    RooPlot *framex = mass->frame();
    h_pi0_cut.plotOn(framex,Binning(40),Name("Hist"));
    framex->SetMaximum(framex->GetMaximum());
    totalPdf.plotOn(framex,Normalization(1.0,RooAbsReal::RelativeExpected),Name("curve"));
    //  totalPdf.plotOn(framex,Components(RooArgSet(gau1)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
    totalPdf.plotOn(framex,Components(RooArgSet(sigcb)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
    totalPdf.plotOn(framex,Components(RooArgSet(bkg)),LineStyle(3),LineColor(kBlue),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
    totalPdf.paramOn(framex,Parameters(RooArgSet(sig1,mean,nsig)),Format("NELU",AutoPrecision(2)),Layout(0.4,0.95,0.95) );
    framex->getAttText()->SetTextSize(0.03);
    framex->SetMaximum(h_pi0_m_cut->GetMaximum()*1.5);

    float meancut   = mean.getVal();
    float meanerror = mean.getError();
    float width = sig1.getVal();
    float widtherror = sig1.getError();

    framex->Draw("");
    canvas->Print("test2_Validation_pi0_plots.pdf");

    /* Fit to the unfit mass with cuts */
    totalPdf.fitTo(h_pi0_nocut,RooFit::Extended(kTRUE),Minos(1));
    RooPlot *framey = mass->frame();
    h_pi0_nocut.plotOn(framey,Binning(40),Name("Hist"));
    framey->SetMaximum(framey->GetMaximum());
    totalPdf.plotOn(framey,Normalization(1.0,RooAbsReal::RelativeExpected),Name("curve"));
    //  totalPdf.plotOn(framey,Components(RooArgSet(gau1)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
    totalPdf.plotOn(framey,Components(RooArgSet(sigcb)),LineStyle(kDashed),LineColor(kRed),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
    totalPdf.plotOn(framey,Components(RooArgSet(bkg)),LineStyle(3),LineColor(kBlue),Normalization(1.0,RooAbsReal::RelativeExpected),ProjectionRange("sigreg"));
    totalPdf.paramOn(framey,Parameters(RooArgSet(sig1,mean,nsig)),Format("NELU",AutoPrecision(2)),Layout(0.4,0.95,0.95) );
    framey->getAttText()->SetTextSize(0.03);
    framey->SetMaximum(h_pi0_m->GetMaximum()*1.5);
    framey->Draw();
    canvas->Print("test2_Validation_pi0_plots.pdf");

    /* Save the numerical fit results to a validation ntuple */
    string namelist;
    TNtuple* tvalidation = new TNtuple("pi0mass","tree","mean:meanerror:width:widtherror");
    tvalidation->Fill(meancut*1000.,meanerror*1000.,width*1000.,widtherror*1000.);
    tvalidation->SetAlias("Description","Fit to the pi0 mass in background conditions. Note this test may be replaced due to overlap with ECL specific tests.");
    tvalidation->SetAlias("Check","Consistent numerical fit results.");
    tvalidation->SetAlias("Contact",contact);

    canvas->Print("test2_Validation_pi0_plots.pdf]");

    output->Write();
    output->Close();
}


