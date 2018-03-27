/*
<header>
<input>../GenericB.ntup.root</input>
<output>test2_Validation_Resolution_output.root</output>
<contact>Sam Cunliffe (sam.cunliffe@desy.de); Phillip Urquijo (phillip.urquijo@unimelb.edu.au)</contact>
<description>
Check the Reconstruction resolution with single particles, using truth matching
</description>
</header>
*/

// Authors: Phillip Urquijo, Luis Pesantez, Jun 2013
// Updated: Oct 2014
//          Nov 2017

#include <vector>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include <TApplication.h>
#include <TChain.h>
#include <TCanvas.h>
#include <TDirectory.h>
#include <TError.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphAsymmErrors.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TH2.h>
#include <THStack.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TList.h>
#include <TLorentzVector.h>
#include <TPad.h>
#include <TROOT.h>
#include <TSelector.h>
#include <TStyle.h>
#include <TSpectrum.h>
#include <TSystem.h>
#include <TTree.h>

//#include <Belle2Labels.h>

#define NBINSP 25
#define MAXP 2.5
#define MINP 0.0

#define NBINSTH 10
#define MAXTH 1.0
#define MINTH -1.0

#define MINRES -0.05
#define MAXRES 0.05

const float reswidth=(float)MAXP/(float)NBINSP;
const float dzreswidth=1/((float)MAXP/(float)NBINSP);

enum hypo { pion=0, kaon=1, electron=2, muon=3, proton=4, photon=5, ntypes=6};
const int pid[] = {211, 321, 11, 13, 2212, 22};
const float pdgmasses[] = {0.13957, 0.49367, 0.000511, 0.105658, 0.93827, 0.};
const char *names[] = { "pi", "k", "e", "mu", "p", "gamma"};
enum ecl_regions {All=0, FW=1, Barrel=2, BW=3, ECL=4};
const char* regions[]= { "All", "FW", "Barrel", "BW" };

double xvalues[NBINSP], yrms[NBINSP], xe[NBINSP], yerms[NBINSP];
double ymean[NBINSP], yemean[NBINSP];

TCanvas *canvasResolution = new TCanvas ("canvasResolution","canvasResolution",1000,800);

int find_bin( float pt ) {
    int bin = (int)(pt/reswidth);
    if( bin<NBINSP )   return bin;
    else return NBINSP-1;
}

void pi_plots(bool runOffline) {
    const char *contact = "sam.cunliffe@desy.de";
    TString infile (Form( "../GenericB.ntup.root" ) );
    TChain* recoTree = new TChain("pituple");
    recoTree->AddFile(infile);

    TList* list = new TList;
    TList* list_web = new TList;
    //Lab momentum
    TH1F* h_PPi     = new TH1F("PPi", ";p_{T}(#pi) GeV;N", NBINSP, MINP, MAXP);
    list->Add(h_PPi);
    TH1F* h_PTruthPi= new TH1F("PTruthPi", ";p_{T}(#pi) truth GeV;N", NBINSP, MINP, MAXP);
    list->Add(h_PTruthPi);
    TH1F* h_PResolutionPi= new TH1F("PResolutionPi", ";#sigma(p)/p;N", NBINSP, MINRES, MAXRES);
    list->Add(h_PResolutionPi);
    //Lab cos(theta)
    TH1F* h_CosThPi      = new TH1F("CosThPi", ";cos#theta(#pi) ;N", NBINSTH, MINTH, MAXTH);
    list->Add(h_CosThPi);
    TH1F* h_CosThTruthPi = new TH1F("CosThTruthPi", ";cos#theta(#pi) truth ;N", NBINSTH, MINTH, MAXTH);
    list->Add(h_CosThTruthPi);

    TH1F* h_PResPi[NBINSP];
    TH1F* h_dzResPi[NBINSP];
    for(int ipbin=0; ipbin<NBINSP; ipbin++) {
        h_PResPi[ipbin]= new TH1F(Form("PResPi_%d", ipbin), ";p_{T}(#pi) (Rec-True) GeV;Arbitrary Normalisation", 200, -0.05, .05);
        list->Add(h_PResPi[ipbin]);
        h_dzResPi[ipbin] = new TH1F(Form("dzResPi_%d", ipbin), ";dz;Arbitrary Normalisation",     100, 0., 1000.);
        list->Add(h_dzResPi[ipbin]);
    }

    TH2F* h_Resolution_2D  = new TH2F("Resolution_2D", ";p_{T}(#pi) Rec GeV;p_{T}(#pi) Truth GeV", NBINSP, MINP, MAXP, NBINSP, MINP, MAXP);
    TH2F* h_dzResolution  = new TH2F("dzResolution", ";p_{T};#sigma(dz) #mu m", NBINSP, MINP, MAXP, 10, 0., 200);

    Float_t pi_P;
    Float_t pi_P4[4];
    Float_t pi_z0;
    Int_t   pi_mcErrors;
    Float_t pi_TruthP;
    Float_t pi_TruthP4[4];

    recoTree->SetBranchAddress("pi_P", &pi_P);
    recoTree->SetBranchAddress("pi_P4", pi_P4);
    recoTree->SetBranchAddress("pi_z0", &pi_z0);
    recoTree->SetBranchAddress("pi_mcErrors", &pi_mcErrors);
    recoTree->SetBranchAddress("pi_TruthP", &pi_TruthP);
    recoTree->SetBranchAddress("pi_TruthP4", pi_TruthP4);

    for(Int_t iloop=0; iloop<recoTree->GetEntries(); iloop++) {
        recoTree->GetEntry(iloop);

        if(pi_mcErrors>0)   continue;
        TLorentzVector p4_pi(pi_P4);
        TLorentzVector p4_truth_pi(pi_TruthP4);;
        float pBias = (p4_pi.Pt()-p4_truth_pi.Pt())/p4_truth_pi.Pt();
        int pt_bin = find_bin( p4_pi.Pt() );

        h_PPi->Fill(p4_pi.Pt());
        h_PTruthPi->Fill(p4_truth_pi.Pt());
        h_PResolutionPi->Fill(pBias);
        h_Resolution_2D->Fill(p4_pi.Pt(), p4_truth_pi.Pt());

        h_CosThPi->Fill(p4_pi.CosTheta());
        h_CosThTruthPi->Fill(p4_truth_pi.CosTheta());

        h_PResPi[pt_bin]->Fill(p4_pi.Pt()-p4_truth_pi.Pt());
        h_dzResPi[pt_bin]->Fill(pi_z0*1000);
        h_dzResolution->Fill(p4_pi.Pt(), pi_z0);
    }

    TLegend* leg = new TLegend(0.3,0.2,0.5,0.35);
    leg->SetFillColor(0);

    h_PTruthPi->SetLineColor(kRed);
    leg->AddEntry(h_PTruthPi,"Truth","l");
    leg->AddEntry(h_PPi,"Reco","l");

    h_PPi->DrawNormalized("");
    h_PTruthPi->DrawNormalized("same");
    leg->Draw();
    canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"),"Momentum");

    canvasResolution->Clear();
    h_Resolution_2D->Draw("box");
    canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"),"Detector response for pions");

    canvasResolution->Clear();
    h_CosThTruthPi->SetLineColor(kRed);
    h_CosThPi->DrawNormalized();
    h_CosThTruthPi->DrawNormalized("same");
    leg->Draw();
    canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"),"CosTheta");

    canvasResolution->Clear();
    h_PResolutionPi->Draw();
    //if(runOffline)Belle2Labels(0.4,0.9,"Simulation (Preliminary)");
    canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"),"Momentum Resolution");

    canvasResolution->Clear();

    for(int i=0; i<NBINSP; i++) {
        if( h_PResPi[i]->Integral()>0. ) h_PResPi[i]->Scale(1/h_PResPi[i]->Integral());
        xvalues[i] = (float)i*reswidth+reswidth/2.;
        yrms[i] = h_PResPi[i]->GetRMS()/xvalues[i];
        ymean[i] = h_PResPi[i]->GetMean()/xvalues[i];
        xe[i] = reswidth/2.;
        yerms[i] = h_PResPi[i]->GetRMSError()/xvalues[i];
        yemean[i] = h_PResPi[i]->GetMeanError()/xvalues[i];
        h_PResPi[i]->Draw();
        //if(runOffline)Belle2Labels(0.4,0.9,"Simulation (Preliminary)");
        //Belle2Labels(0.1, 0.96, Form( "%.1f < p_{T}(#pi)(GeV) < %.1f", i*0.1, (i+1)*0.1 ) );
        canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"),"1/pT in bins of pT");
    }
    for(int i=0; i<NBINSP; i++) {
        h_dzResPi[i]->Draw();
        //if(runOffline)Belle2Labels(0.4,0.9,"Simulation (Preliminary)");
        //Belle2Labels(0.1, 0.96, Form( "%.1f < p_{T}(#pi)(GeV) < %.1f", i*0.1, (i+1)*0.1 ) );
        canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"),"dz");
    }

    canvasResolution->Clear();
    canvasResolution->cd()->SetGridy(1);
    canvasResolution->cd()->SetGridx(1);

    h_Resolution_2D->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction, reconstructed vs truth transverse momentum of truth-matched pion tracks with a pi hypothesis. A Generic BBbar sample is used."));
    h_Resolution_2D->GetListOfFunctions()->Add(new TNamed("Check", "Stable resolution."));
    h_Resolution_2D->GetListOfFunctions()->Add(new TNamed("Contact", contact));
    list_web->Add(h_Resolution_2D);

    TH1F *h_Resolution = new TH1F("h_Resolution", ";p_{T} (#pi) GeV;#sigma(p_{T})/p_{T}", NBINSP, MINP, MAXP);
    h_Resolution->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction PT resolution of truth-matched pion tracks with a pi hypothesis in bins of transverse momentum. Sigma is determined from getRMS. A Generic BBbar sample is used."));
    h_Resolution->GetListOfFunctions()->Add(new TNamed("Check", "Stable resolution."));
    h_Resolution->GetListOfFunctions()->Add(new TNamed("Contact", contact));
    h_Resolution->SetFillColor(kBlue);  h_Resolution->SetMarkerColor(kBlue);
    h_Resolution->SetMinimum(0.00);     h_Resolution->SetMaximum(0.03);
    for( int bin=0; bin<NBINSP; bin++ ){
        h_Resolution->SetBinContent( bin+1, yrms[bin] );
        h_Resolution->SetBinError( bin+1, yerms[bin] );
    }
    list->Add(h_Resolution);
    list_web->Add(h_Resolution);
    h_Resolution->Draw("e2");
    h_Resolution->SetOption("e2");
    //if(runOffline)Belle2Labels(0.4,0.9,"Simulation (Preliminary)");
    canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"),"Resolution in bins of pT");
    canvasResolution->Clear();

    TH1F *h_Bias = new TH1F("h_Bias", ";p_{T} (#pi) GeV;Mean Bias (Rec-True)/Mean", NBINSP, MINP, MAXP);
    h_Bias->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction pT bias of truth-matched pion tracks with a pi hypothesis in bins of transverse momentum. The relative bias, i.e. (Mean(Rec)-Mean(True))/Mean is shown. A Generic BBbar sample is used."));
    h_Bias->GetListOfFunctions()->Add(new TNamed("Check", "Stable resolution. Low pT tracks < 100 MeV exhibit larger relative bias."));
    h_Bias->GetListOfFunctions()->Add(new TNamed("Contact", contact));
    h_Bias->SetFillColor(kBlue);    h_Bias->SetMarkerColor(kBlue);
    h_Bias->SetMinimum(-0.002);     h_Bias->SetMaximum(0.002);
    for( int bin=0; bin<NBINSP; bin++ ){
        h_Bias->SetBinContent( bin+1, ymean[bin] );
        h_Bias->SetBinError( bin+1, yemean[bin] );
    }
    h_Bias->Draw("2a");
    list->Add(h_Bias);
    list_web->Add(h_Bias);
    h_Bias->Draw("e2");
    h_Bias->SetOption("e2");
    //if(runOffline)Belle2Labels(0.4,0.9,"Simulation (Preliminary)");
    canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"),"Bias in bins of pT");
    canvasResolution->Clear();


    for(int i=0; i<NBINSP; i++) {
        if( h_dzResPi[i]->Integral()>0. )    h_dzResPi[i]->Scale(1/h_dzResPi[i]->Integral());
        xvalues[i] = (float)i*reswidth+reswidth/2.;
        yrms[i] = h_dzResPi[i]->GetRMS();
        xe[i] = reswidth/2.;
        yerms[i] = h_dzResPi[i]->GetRMSError();
    }

    //z0 Bias in bins of pT
    TH1F *h_ZResolution = new TH1F("h_ZResolution", ";p_{T}(#pi) GeV;#sigma(z)", NBINSP, MINP, MAXP);
    h_ZResolution->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction dz of truth-matched pion tracks with a pi hypothesis in bins of transverse momentum. Sigma is determined from getRMS. A Generic BBbar sample is used."));
    h_ZResolution->GetListOfFunctions()->Add(new TNamed("Check", "Stable resolution."));
    h_ZResolution->GetListOfFunctions()->Add(new TNamed("Contact", contact));
    list->Add(h_ZResolution);
    h_ZResolution->SetFillColor(kBlue); h_ZResolution->SetMarkerColor(kBlue);
    h_ZResolution->SetMinimum(0.00);    h_ZResolution->SetMaximum(250);
    for( int bin=0; bin<NBINSP; bin++ ){
        h_ZResolution->SetBinContent( bin+1, yrms[bin] );
        h_ZResolution->SetBinError( bin+1, yerms[bin] );
    }
    list->Add(h_ZResolution);
    list_web->Add(h_ZResolution);
    h_ZResolution->Draw("e2");
    h_ZResolution->SetOption("e2");
    //if(runOffline)Belle2Labels(0.4,0.9,"Simulation (Preliminary)");
    canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"),"z Resolution in bins of pT");
    canvasResolution->Clear();

    TFile* output = new TFile(Form("test2_Validation_Resolution_output.root"), "update");
    TDirectory* dir = output->mkdir("Tracks");
    dir->cd();
    list->Write();

    output->cd();
    list_web->Write();
    output->Close();
}

void photon_plots(bool runOffline) {
    const char *contact = "sam.cunliffe@desy.de; torben.ferber@desy.de";
    TString infile (Form( "../GenericB.ntup.root" ) );
    TChain* recoTree = new TChain("gammatuple");
    recoTree->AddFile(infile);
    TH1F* h_PGamm[ECL];
    TH1F* h_PTruthGamm[ECL];
    TH1F* h_PResolutionGamm[ECL];
    TH1F* h_EResGamm[ECL][NBINSP];
    TH2F* h_Resolution_2D[ECL];
    //Energy
    TList* list = new TList;
    TList* list_web = new TList;
    for( int i=0; i<ECL; i++ ) {
        h_PGamm[i]= new TH1F(Form("PGamm_%s",regions[i]), ";E(#gamma) GeV;N", NBINSP, MINP, MAXP);
        list->Add( h_PGamm[i] );
        h_PTruthGamm[i]= new TH1F(Form("PTruthGamm_%s",regions[i]), ";E(#gamma) truth GeV;N", NBINSP, MINP, MAXP);
        list->Add( h_PTruthGamm[i] );
        h_PResolutionGamm[i]= new TH1F(Form("PResolutionGamm_%s",regions[i]), ";#sigma(E)/E;N", 50, -2, 1);
        list->Add( h_PResolutionGamm[i] );

        for(int j=0; j<NBINSP; j++) {
            h_EResGamm[i][j]= new TH1F(Form("EResGamm_%s_%d",regions[i],j), ";E(#gamma) Rec-Truth;N", 100, -0.35, 0.15);
            list->Add( h_EResGamm[i][j] );
        }
        h_Resolution_2D[i] = new TH2F(Form("Resolution_2D_%s",regions[i]), ";E(#gamma) Rec GeV;E(#gamma) Truth GeV", NBINSP, MINP, MAXP, NBINSP, MINP, MAXP);
        h_Resolution_2D[i]->GetListOfFunctions()->Add(new TNamed("Description", Form("Photon reconstruction, reconstructed vs truth energy of truth-matched photons. A Generic BBbar sample is used. ECL %s",names[i]) ) );
        h_Resolution_2D[i]->GetListOfFunctions()->Add(new TNamed("Check", "Large number of true photons mapped to E(reco)=0."));
        h_Resolution_2D[i]->GetListOfFunctions()->Add(new TNamed("Contact", contact));
        list->Add( h_Resolution_2D[i] );
        if(i==0)    list_web->Add( h_Resolution_2D[i] );
    }

    Float_t gamma_P;
    Float_t gamma_P4[4];
    Int_t   gamma_mcErrors;
    Float_t gamma_TruthP;
    Float_t gamma_TruthP4[4];

    recoTree->SetBranchAddress("gamma_P", &gamma_P);
    recoTree->SetBranchAddress("gamma_P4", gamma_P4);
    recoTree->SetBranchAddress("gamma_mcErrors", &gamma_mcErrors);
    recoTree->SetBranchAddress("gamma_TruthP", &gamma_TruthP);
    recoTree->SetBranchAddress("gamma_TruthP4", gamma_TruthP4);

    int region=0;
    for(Int_t iloop=0; iloop<recoTree->GetEntries(); iloop++) {
        recoTree->GetEntry(iloop);
        TLorentzVector lv_gamma(gamma_TruthP4);
        if(lv_gamma.Theta()>2.71 || lv_gamma.Theta()<0.21) continue;
        if((lv_gamma.Theta()<0.58 && lv_gamma.Theta()>=0.21) ) region=1; //forward
        if((lv_gamma.Theta()<2.23 && lv_gamma.Theta()>=0.58) ) region=2; //barrel
        if((lv_gamma.Theta()<2.71 && lv_gamma.Theta()>=2.23) ) region=3; //backward

        if(gamma_mcErrors>0)    continue;
        float pBias = (gamma_P-gamma_TruthP)/gamma_TruthP;
        int p_bin = find_bin( gamma_P );

        h_PGamm[0]->Fill(gamma_P);
        h_PGamm[region]->Fill(gamma_P);
        h_PTruthGamm[0]->Fill(gamma_TruthP);
        h_PTruthGamm[region]->Fill(gamma_TruthP);
        h_PResolutionGamm[0]->Fill(pBias);
        h_PResolutionGamm[region]->Fill(pBias);

        h_EResGamm[0][p_bin]->Fill(gamma_P-gamma_TruthP);
        h_EResGamm[region][p_bin]->Fill(gamma_P-gamma_TruthP);
        h_Resolution_2D[0]->Fill(gamma_P, gamma_TruthP);
        h_Resolution_2D[region]->Fill(gamma_P, gamma_TruthP);
    }
    canvasResolution->Divide(2,2);
    for( int i=0; i<ECL; i++ ) {
        h_PTruthGamm[i]->SetLineColor(kRed);

        TLegend* leg = new TLegend(0.3,0.2,0.5,0.35);
        leg->SetFillColor(0);
        leg->AddEntry(h_PTruthGamm[i],"Truth","l");
        leg->AddEntry(h_PGamm[i],"Reco","l");

        canvasResolution->cd(i+1);
        h_PGamm[i]->DrawNormalized("");
        h_PTruthGamm[i]->DrawNormalized("same");
        //Belle2Labels(0.1, 0.96, Form( "ECL: %s", regions[i] ) );
        if(i==0)    leg->Draw();
    }
    canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"), Form("Photon energy"));

    for(int j=0; j<NBINSP; j++) {
        for( int i=0; i<ECL; i++ ) {
            //------------------------------------------------------------------------------
            // This is only to find weird peaks in the resolution plots
            // I found peaks for Ereco=0 and no BG
            // All events in the peaks have:
            // - flag "SeenInECL"==1
            // - TruthP<0.2
            // - P<0.05
            //------------------------------------------------------------------------------
            //TSpectrum *s = new TSpectrum(5);
            //Int_t nfound = s->Search(h_EResGamm[i][j],2,"new");
            //Float_t *xpeaks = s->GetPositionX();
            //for( int n=0; n<nfound; n++ )   cout << xpeaks[n] << "\t";
            //cout << endl;
            //------------------------------------------------------------------------------
            canvasResolution->cd(i+1);
            h_EResGamm[i][j]->Draw("histo");
            //if(i==0)Belle2Labels(0.3, 0.96, Form( "ECL: %s %10.1f < E(#gamma) GeV < %.1f",regions[i], j*0.1, (j+1)*0.1 ) );
            //else Belle2Labels(0.3, 0.96, Form( "ECL: %s",regions[i] ) );
        }
        //if(runOffline)Belle2Labels(0.4, 0.9, "Simulation (Preliminary)");
        canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"), Form("Photon energy resolution %d", j));
    }

    for( int i=0; i<ECL; i++ ) {
        canvasResolution->cd(i+1);
        h_Resolution_2D[i]->Draw("box");
        //Belle2Labels(0.3, 0.96, Form( "ECL: %s", regions[i] ) );
    }
    //if(runOffline)Belle2Labels(0.4, 0.9, "Simulation (Preliminary)");
    canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"),"Detector response for photons");

    canvasResolution->Clear();
    canvasResolution->SetGridx(1);
    canvasResolution->SetGridy(1);
    for( int i=0; i<ECL; i++ ) {
        for(int j=0; j<NBINSP; j++) {
            if( h_EResGamm[i][j]->Integral()>0. )    h_EResGamm[i][j]->Scale(1/h_EResGamm[i][j]->Integral());
            xvalues[j] = (float)j*reswidth+reswidth/2.;
            yrms[j]  = h_EResGamm[i][j]->GetRMS()/xvalues[j];
            ymean[j] = h_EResGamm[i][j]->GetMean()/xvalues[j];

            xe[j] = reswidth/2.;
            yerms[j]  = h_EResGamm[i][j]->GetRMSError()/xvalues[j];
            yemean[j] = h_EResGamm[i][j]->GetMeanError()/xvalues[j];
            // If too small statistics, some ymean become nan
            if(yrms[j]!=yrms[j])    yrms[j] = 0.;
            if(ymean[j]!=ymean[j])  ymean[j] = 0.;
        }

        TH1F *h_Resolution = new TH1F(Form("h_Resolution_%s",regions[i]), ";E (#gamma) GeV;#sigma(E)/E", NBINSP, MINP, MAXP);
        h_Resolution->GetListOfFunctions()->Add(new TNamed("Description", Form( "Single photon reconstruction resolution of truth-matched photons in bins of true energy. Sigma is determined from getRMS. A Generic BBbar sample is used. ECL %s", names[i]) ) );
        h_Resolution->GetListOfFunctions()->Add(new TNamed("Check", "Large number of true photons mapped to E(reco)=0"));
        h_Resolution->GetListOfFunctions()->Add(new TNamed("Contact", contact));
        h_Resolution->SetFillColor(kBlue);  h_Resolution->SetMarkerColor(kBlue);
        h_Resolution->SetMinimum(0.);       h_Resolution->SetMaximum(0.4);
        for( int bin=0; bin<NBINSP; bin++ ){
            h_Resolution->SetBinContent( bin+1, yrms[bin] );
            h_Resolution->SetBinError( bin+1, yerms[bin] );
        }
        list->Add(h_Resolution);
        if( i==0 )  list_web->Add(h_Resolution);
        
        h_Resolution->Draw("e2");
        h_Resolution->SetOption("e2");
        //if(runOffline)Belle2Labels(0.4, 0.9, "Simulation (Preliminary)");
        canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"),"Detector resolution for photons");


        TH1F *h_Bias = new TH1F(Form("h_Bias_%s",regions[i]),";E (#gamma) GeV;Mean Bias (Rec-Truth)/Mean",NBINSP,MINP,MAXP);
        h_Bias->GetListOfFunctions()->Add(new TNamed("Description", Form("Single photon reconstruction Energy bias of truth-matched photons in bins of true energy. The relative bias, i.e. (Mean(Rec)-Mean(True))/Mean is shown. A Generic BBbar sample is used. ECL %s", names[i])));
        h_Bias->GetListOfFunctions()->Add(new TNamed("Check", "Large number of true photons mapped to E(reco)=0"));
        h_Bias->GetListOfFunctions()->Add(new TNamed("Contact", contact));
        list->Add(h_Bias);
        h_Bias->SetFillColor(kBlue);  h_Bias->SetMarkerColor(kBlue);
        h_Bias->SetMinimum(-0.5);     h_Bias->SetMaximum(0.1);
        for( int bin=0; bin<NBINSP; bin++ ){
            h_Bias->SetBinContent( bin+1, ymean[bin] );
            h_Bias->SetBinError( bin+1, yemean[bin] );
        }
        list->Add(h_Bias);
        if( i==0 )    list_web->Add(h_Bias);
        
        h_Bias->Draw("e2");
        h_Bias->SetOption("e2");
        //if(runOffline)Belle2Labels(0.4, 0.9, "Simulation (Preliminary)");
        canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf"),"Detector resolution for photons");
    }

    ////////////////////////////
    TFile* output = new TFile(Form("test2_Validation_Resolution_output.root"), "update");
    TDirectory* dir = output->mkdir("Photons");
    dir->cd();
    list->Write();

    output->cd();
    list_web->Write();
    output->Close();
}

void test2_Validation_Resolution(bool runOffline=false) {
    TFile* output = new TFile(Form("test2_Validation_Resolution_output.root"), "recreate");
    output->Close();

    canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf["));
    pi_plots(runOffline);
    photon_plots(runOffline);
    canvasResolution->Print(Form("test2_Validation_Resolution_plots.pdf]"));
}


