/*
<header>
<input>../GenericB.ntup.root</input>
<output>test2_Validation_PID_output.root</output>
<contact>Jan Strube (jan.strube@desy,de)</contact>
<description>
Check the PID efficiency from single particles with truth matching.
Note this is only PID efficiency, not tracking efficiency.
</description>
</header>
*/

// Authors: Phillip Urquijo, Luis Pesantez, May 2013
// Updated: Oct 2014
//          Nov 2017

#include <vector>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include <TChain.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphAsymmErrors.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <THStack.h>
#include <TLegend.h>
#include <TList.h>
#include <TLorentzVector.h>
#include <TMultiGraph.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TTree.h>

//#include <Belle2Labels.h>

void plot( TH1F** histo, TCanvas* canvas ) {
    THStack* stack = new THStack;
    int maxHisto = -1;
    double max = 0.;
    for(int j=0; j<5; j++) {
        stack->Add(histo[j]);
        if( histo[j]->GetMaximum()>max ) {
            max=histo[j]->GetMaximum();      // This is because THStack messes up the axis titles
            maxHisto=j;
        }
    }
    canvas->cd();
    histo[maxHisto]->Draw();
    stack->Draw("nostacksame");
    histo[maxHisto]->Draw("sameaxis");
}

void test2_Validation_PID(int region=0, bool runOffline=false) { //0=all,1=forward,2=barrel,3=backward,4=endcap
    TFile* output = new TFile("test2_Validation_PID_output.root", "recreate");
    output->Close();

    TCanvas *c_pidvalidation = new TCanvas ("PIDvalidation","PIDvalidation",1000,800);
    c_pidvalidation->Print("test2_Validation_PID_plots.pdf[");

    TChain * recoTree = new TChain("pituple");
    recoTree->AddFile("../GenericB.ntup.root");

    const char *contact = "jan.strube@desy.de";

    enum hypo {pion=0, kaon=1, electron=2, muon=3, proton=4, ntypes=5};
    const int pid[] = {211,321,11,13,2212};
    const float pdgmasses[] = {0.13957,0.49367,0.000511,0.105658,0.93827};
    const char *names[] = { "pi", "K", "e", "mu", "p"};
    Color_t colors[]= {kRed, kBlue, kGreen, kOrange, kMagenta};

    const int nthbins=20;
    const float thlow=-1.;
    const float thhigh=1.;

    const int npbins=18;
    const float plow=0.;
    const float phigh=3.6;

    const int npidbins=101;
    const float pidlow=0.;
    const float pidhigh=1.01;

    TH1F* h_trk_P[ntypes];
    TH1F* h_trk_P_pass[ntypes];
    TH1F* h_trk_CosTh[ntypes];
    TH1F* h_trk_CosTh_pass[ntypes];
    TH1F* h_trk_PID[ntypes];
    TH3F* h_ROC[ntypes];

    TList* list_web = new TList;

    for(int Hypo=0; Hypo<ntypes; Hypo++) { //Hypo counter
        TList* list = new TList;
        TLegend* legHist = new TLegend(0.5,0.7,0.7,0.9);
        legHist->SetFillColor(0);

        for(int Hypo2=0; Hypo2<ntypes; Hypo2++) { //Hypo counter
            h_trk_P[Hypo2]          = new TH1F(Form("%s_%s_P",names[Hypo],names[Hypo2]),         Form(";p(%s) GeV;N",names[Hypo]),
                                               npbins,plow,phigh);
            h_trk_P_pass[Hypo2]      = new TH1F(Form("%s_%s_P_pass",names[Hypo],names[Hypo2]),    Form(";Pass PID, p(%s) GeV;N",names[Hypo]),
                                               npbins,plow,phigh);
            h_trk_CosTh[Hypo2]      = new TH1F(Form("%s_%s_CosTh",names[Hypo],names[Hypo2]),     Form(";cos#theta(%s);N",names[Hypo]),
                                               nthbins,thlow,thhigh);
            h_trk_CosTh_pass[Hypo2] = new TH1F(Form("%s_%s_CosTh_pass",names[Hypo],names[Hypo2]),Form(";Pass PID, cos#theta(%s) GeV;N",names[Hypo]),
                                               nthbins,thlow,thhigh);
            h_trk_PID[Hypo2]        = new TH1F(Form("%s_%s_pass",names[Hypo],names[Hypo2]),       Form(";PID(%s) GeV;N",names[Hypo]),
                                               npidbins,pidlow,pidhigh);
            h_trk_P[Hypo2]->SetLineColor( colors[Hypo2] );
            h_trk_P_pass[Hypo2]->SetLineColor( colors[Hypo2] );
            h_trk_CosTh[Hypo2]->SetLineColor( colors[Hypo2] );
            h_trk_CosTh_pass[Hypo2]->SetLineColor( colors[Hypo2] );
            h_trk_PID[Hypo2]->SetLineColor( colors[Hypo2] );
            legHist->AddEntry( h_trk_P[Hypo2], Form("%s",names[Hypo2]), "l" );
        }
        h_ROC[Hypo] = new TH3F(Form("ROC_%s",names[Hypo]), Form(";PID(%s);N;p (GeV)",names[Hypo]),5,0.,5.,npidbins,pidlow,pidhigh,                                npbins,plow,phigh);
        list->Add(h_ROC[Hypo]);

        float pi_TruthP;
        recoTree->SetBranchAddress("pi_TruthP", &pi_TruthP);
        float pi_P4[4];
        recoTree->SetBranchAddress("pi_P4",     &pi_P4);
        float pi_pionID;
        recoTree->SetBranchAddress("pi_pionID",  &pi_pionID);
        float pi_kaonID;
        recoTree->SetBranchAddress("pi_kaonID",   &pi_kaonID);
        float pi_electronID;
        recoTree->SetBranchAddress("pi_electronID",   &pi_electronID);
        float pi_muonID;
        recoTree->SetBranchAddress("pi_muonID",  &pi_PIDmu);
        float pi_protonID;
        recoTree->SetBranchAddress("pi_protonID",   &pi_protonID);
        int pi_mcPDG;
        recoTree->SetBranchAddress("pi_mcPDG",  &pi_mcPDG);

        for(Int_t iloop=0; iloop<recoTree->GetEntries(); iloop++) {
            recoTree->GetEntry(iloop);
            //Recalculate the particle 4 vector for different hypotheses
            TLorentzVector lv_trk_temp(pi_P4);
            TLorentzVector lv_trk(lv_trk_temp.Vect(), sqrt(lv_trk_temp.P()*lv_trk_temp.P() + pdgmasses[Hypo]*pdgmasses[Hypo]));

            int pdgid=abs(pi_mcPDG);

            if(region==1 && (lv_trk.Theta()<0.21 || lv_trk.Theta()>0.58) ) continue; //forward
            if(region==2 && (lv_trk.Theta()<0.58 || lv_trk.Theta()>2.23) ) continue; //barrel
            if(region==3 && (lv_trk.Theta()<2.23 || lv_trk.Theta()>2.71) ) continue; //backward
            if(region==4 && (lv_trk.Theta()<0.21 || lv_trk.Theta()>2.71 || (lv_trk.Theta()>0.58 && lv_trk.Theta()<2.23) ) ) continue; //backward
            if(region==5 && (lv_trk.Theta()<0.58 || lv_trk.Theta()>2.23) && lv_trk.Rho()>0.5&&lv_trk.Rho()<1.5 ) continue; //special case for presentations

            //PID cut
            bool passPID = false;
            if     (Hypo==pion     && pi_pionID>0.5) passPID=true;
            else if(Hypo==kaon     && pi_kaonID >0.5) passPID=true;
            else if(Hypo==electron && pi_electronID >0.5) passPID=true;
            else if(Hypo==muon     && pi_muonID>0.5) passPID=true;
            else if(Hypo==proton   && pi_protonID >0.5) passPID=true;

            float PID = -2;
            if     (Hypo==pion    ) PID=pi_pionID;
            else if(Hypo==kaon    ) PID=pi_kaonID;
            else if(Hypo==electron) PID=pi_electronID;
            else if(Hypo==muon    ) PID=pi_muonID;
            else if(Hypo==proton  ) PID=pi_protonID;

            int type=-1;
            for(int Hypo2=0; Hypo2<ntypes; Hypo2++) { //Hypo counter
                if(pdgid==pid[Hypo2])   type=Hypo2;
            }
            h_ROC[Hypo]->Fill( type, PID, lv_trk.Rho() );

            for(int Hypo2=0; Hypo2<ntypes; Hypo2++) { //Hypo counter
                if(pdgid!=pid[Hypo2]) continue;
                h_trk_P[Hypo2]    ->Fill(lv_trk.Rho());
                h_trk_CosTh[Hypo2]->Fill(lv_trk.CosTheta());
                h_trk_PID[Hypo2]  ->Fill(PID);
                if(passPID) {
                    h_trk_P_pass[Hypo2]     ->Fill(lv_trk.Rho());
                    h_trk_CosTh_pass[Hypo2]->Fill(lv_trk.CosTheta());
                }
            }
        }

        const int nPSteps = 7;
        float PBoundary[nPSteps+1] = { 0.0, 4.0, 0.0, 0.5, 1.0, 1.5, 2.0, 4.0 };

        for( int Hypo2=0; Hypo2<ntypes; Hypo2++ ) {
            TMultiGraph* multigraph_ROC = new TMultiGraph;
            TMultiGraph* multigraph_Rejection = new TMultiGraph;
            TLegend* legROC = new TLegend(0.2,0.7,0.4,0.9);
            legROC->SetFillColor(0);

            for( int PStep=0; PStep<nPSteps; PStep++ ) {
                if( PStep==1 )  continue;// skip region 4<p<0, defined just for drawing purposes
                int Pmin = h_ROC[Hypo]->GetZaxis()->FindBin(PBoundary[PStep]);
                int Pmax = h_ROC[Hypo]->GetZaxis()->FindBin(PBoundary[PStep+1]);

                //skip first few bins, have non-linear behavior (singularity at 0)
                const int nPIDSteps=npidbins-5;
                float vPIDStart=h_ROC[Hypo]->GetYaxis()->GetBinLowEdge(3); // Start on 3rd bin
                float FakeRate[nPIDSteps];
                float SignalEff[nPIDSteps];
                float Rejection[nPIDSteps];

                for( int PIDStep=0; PIDStep<nPIDSteps; PIDStep++ ) {
                    float PIDVal = vPIDStart+0.01*PIDStep;
                    int PIDmin = h_ROC[Hypo]->GetYaxis()->FindBin(PIDVal);
                    int PIDmax = h_ROC[Hypo]->GetNbinsY();

                    float NPassHypo   = h_ROC[Hypo]->Integral( Hypo+1,Hypo+1,PIDmin,PIDmax,Pmin,Pmax );
                    float NTotalHypo  = h_ROC[Hypo]->Integral( Hypo+1,Hypo+1,1,PIDmax,Pmin,Pmax );
                    float NPassHypo2  = h_ROC[Hypo]->Integral( Hypo2+1,Hypo2+1,PIDmin,PIDmax,Pmin,Pmax );
                    float NTotalHypo2 = h_ROC[Hypo]->Integral( Hypo2+1,Hypo2+1,1,PIDmax,Pmin,Pmax );
                    SignalEff[PIDStep] = NPassHypo/NTotalHypo;
                    FakeRate[PIDStep]  = NPassHypo2/NTotalHypo2;
                    Rejection[PIDStep] = 1./FakeRate[PIDStep];
                }
                TGraph* GraphROC = new TGraph( nPIDSteps, SignalEff, FakeRate );
                GraphROC->SetLineColor( PStep+1 );
                GraphROC->SetLineWidth(2);
                legROC->AddEntry( GraphROC, Form("%.2f - %.2f",PBoundary[PStep],PBoundary[PStep+1]), "l" );
                multigraph_ROC->Add( GraphROC );

                TGraph* GraphRejection = new TGraph( nPIDSteps, SignalEff, Rejection );
                GraphRejection->SetLineColor( PStep+1 );
                GraphRejection->SetLineWidth(2);
                multigraph_Rejection->Add( GraphRejection );
            }
            multigraph_ROC->Draw("al");
            multigraph_ROC->GetXaxis()->SetTitle( Form("(%s #rightarrow %s) Efficiency",names[Hypo],names[Hypo] ) );
            multigraph_ROC->GetYaxis()->SetTitle(Form("(%s #rightarrow %s) Fake rate",names[Hypo2],names[Hypo]));
            legROC->Draw();
            //Belle2Labels(0.5,0.95,Form("PID(%s)",names[Hypo]));
            c_pidvalidation->Print("test2_Validation_PID_plots.pdf",Form("Title: ROC %s #rightarrow %s",names[Hypo2],names[Hypo]));
            c_pidvalidation->Clear();

            multigraph_Rejection->Draw("al");
            multigraph_Rejection->GetXaxis()->SetTitle( Form("(%s #rightarrow %s) Efficiency",names[Hypo],names[Hypo] ) );
            multigraph_Rejection->GetYaxis()->SetTitle(Form("(%s #rightarrow %s) Rejection",names[Hypo2],names[Hypo]));
            legROC->Draw();
            //Belle2Labels(0.5,0.95,Form("PID(%s)",names[Hypo]));
            c_pidvalidation->Print("test2_Validation_PID_plots.pdf",Form("Title: Rejection %s #rightarrow %s",names[Hypo2],names[Hypo]));
            c_pidvalidation->Clear();
        }

        plot( h_trk_P, c_pidvalidation );
        //if(runOffline)Belle2Labels(0.5,0.9,"Belle II Validation");
        legHist->Draw();
        c_pidvalidation->Print("test2_Validation_PID_plots.pdf",Form("Title: P noPID, %s hypothesis",names[Hypo]));

        plot( h_trk_CosTh, c_pidvalidation );
        //if(runOffline)Belle2Labels(0.5,0.9,"Belle II Validation");
        legHist->Draw();
        c_pidvalidation->Print("test2_Validation_PID_plots.pdf",Form("Title: cos#theta noPID, %s hypothesis",names[Hypo]));

        plot( h_trk_P_pass, c_pidvalidation );
        //if(runOffline)Belle2Labels(0.5,0.9,"Belle II Validation");
        legHist->Draw();
        c_pidvalidation->Print("test2_Validation_PID_plots.pdf",Form("Title: P PID, %s hypothesis",names[Hypo]));

        plot( h_trk_CosTh_pass, c_pidvalidation );
        //if(runOffline)Belle2Labels(0.5,0.9,"Belle II Validation");
        legHist->Draw();
        c_pidvalidation->Print("test2_Validation_PID_plots.pdf",Form("Title: cos#theta PID, %s hypothesis",names[Hypo]));

        c_pidvalidation->SetLogy(1);
        plot( h_trk_PID, c_pidvalidation );
        //if(runOffline)Belle2Labels(0.5,0.9,"Belle II Validation");
        legHist->Draw();
        c_pidvalidation->Print("test2_Validation_PID_plots.pdf",Form("Title: PID, %s hypothesis",names[Hypo]));
        c_pidvalidation->SetLogy(0);

        if(runOffline) {
            c_pidvalidation->SetLogy(1);
            h_trk_PID[pion]->SetFillColor(kYellow);
            h_trk_PID[pion]->SetLineColor(kYellow);
            h_trk_PID[pion]->SetMinimum(10);
            h_trk_PID[pion]->SetMaximum(1000000);
            h_trk_PID[pion]->Draw();
            h_trk_PID[kaon]->SetLineColor(kBlack);
            h_trk_PID[kaon]->SetFillColor(kBlack);
            h_trk_PID[kaon]->SetFillStyle(3004);
            h_trk_PID[kaon]->Draw("same");
            c_pidvalidation->Print("test2_Validation_PID_plots.pdf",Form("Title: PID, %s hypothesis",names[Hypo]));
            c_pidvalidation->Clear();
            c_pidvalidation->SetLogy(0);
        }

        //TGraph for efficiencies;
        TGraphAsymmErrors *Eff_P[ntypes];
        TH1F *h_Eff_P[ntypes];
        TMultiGraph* multigraph_P = new TMultiGraph;
        multigraph_P->SetName(Form("Efficiency_P_%s",names[Hypo]));
        for( int Hypo2=0; Hypo2<ntypes; Hypo2++ ) {
            h_Eff_P[Hypo2] = (TH1F*)h_trk_P[Hypo2]->Clone( Form("h_Eff_P_%s_%s",names[Hypo],names[Hypo2]) );
            h_Eff_P[Hypo2]->GetYaxis()->SetTitle("Efficiency");
            h_Eff_P[Hypo2]->Divide(h_trk_P_pass[Hypo2], h_trk_P[Hypo2],1,1);

            Eff_P[Hypo2] = new TGraphAsymmErrors;
            Eff_P[Hypo2]->Divide(h_trk_P_pass[Hypo2], h_trk_P[Hypo2],"cl=0.683 b(1,1) mode");
            Eff_P[Hypo2]->SetMarkerColor(colors[Hypo2]);
            Eff_P[Hypo2]->SetLineColor(colors[Hypo2]);
            Eff_P[Hypo2]->SetName( Form("Efficiency_P_%s_%s",names[Hypo],names[Hypo2]) );

            multigraph_P->Add(Eff_P[Hypo2]);
            Eff_P[Hypo2]->GetXaxis()->SetTitle(h_trk_CosTh[0]->GetXaxis()->GetTitle());
            Eff_P[Hypo2]->GetYaxis()->SetTitle("Efficiency");

            h_Eff_P[Hypo2]->GetListOfFunctions()->Add(new TNamed("Description", Form("PID(%s)>0.5 efficiency of truth-matched %s tracks in bins of lab momentum. A Generic BBbar sample is used.",names[Hypo],names[Hypo2])));
            h_Eff_P[Hypo2]->GetListOfFunctions()->Add(new TNamed("Check", "Stable, continuous efficiency."));
            h_Eff_P[Hypo2]->GetListOfFunctions()->Add(new TNamed("Contact", contact));
            list->Add(Eff_P[Hypo2]);
            if( Hypo2==Hypo ) list_web->Add(h_Eff_P[Hypo2]);
        }
        multigraph_P->Draw("ap");
        multigraph_P->GetXaxis()->SetTitle(h_trk_P[0]->GetXaxis()->GetTitle());
        multigraph_P->GetYaxis()->SetTitle("Efficiency");
        //Belle2Labels(0.5,0.95,Form("PID(%s)>0.5",names[Hypo]));
        c_pidvalidation->Print("test2_Validation_PID_plots.pdf",Form("Title: Efficiency, %s hypothesis",names[Hypo]));

        if(runOffline) {
            c_pidvalidation->cd()->SetGridy(1);
            c_pidvalidation->cd()->SetGridx(1);
            TMultiGraph* multigraph_offline_P = new TMultiGraph;
            multigraph_offline_P->Add(Eff_P[pion]);
            multigraph_offline_P->Add(Eff_P[kaon]);
            multigraph_offline_P->Draw("ap");
            multigraph_offline_P->GetXaxis()->SetTitle(h_trk_P[0]->GetXaxis()->GetTitle());
            multigraph_offline_P->GetYaxis()->SetTitle("Efficiency");
            //if(region==4)Belle2Labels(0.5,0.9,"Endcaps");
            //if(region==2)Belle2Labels(0.5,0.9,"Barrel");
            c_pidvalidation->Print("test2_Validation_PID_plots.pdf",Form("Title: Efficiency, %s hypothesis",names[Hypo]));
        }

        TGraphAsymmErrors *Eff_CosTh[ntypes];
        TH1F *h_Eff_CosTh[ntypes];
        TMultiGraph* multigraph_CosTh = new TMultiGraph;
        multigraph_CosTh->SetName(Form("Efficiency_CosTh_%s",names[Hypo]));
        for( int Hypo2=0; Hypo2<ntypes; Hypo2++ ) {
            h_Eff_CosTh[Hypo2] = (TH1F*)h_trk_CosTh[Hypo2]->Clone( Form("h_Eff_CosTh_%s_%s",names[Hypo],names[Hypo2]) );
            h_Eff_CosTh[Hypo2]->GetYaxis()->SetTitle("Efficiency");
            h_Eff_CosTh[Hypo2]->Divide(h_trk_CosTh_pass[Hypo2], h_trk_CosTh[Hypo2],1,1);

            Eff_CosTh[Hypo2] = new TGraphAsymmErrors;
            Eff_CosTh[Hypo2]->Divide(h_trk_CosTh_pass[Hypo2], h_trk_CosTh[Hypo2],"cl=0.683 b(1,1) mode");
            Eff_CosTh[Hypo2]->SetMarkerColor(colors[Hypo2]);
            Eff_CosTh[Hypo2]->SetLineColor(colors[Hypo2]);
            Eff_CosTh[Hypo2]->SetName( Form("Efficiency_CosTh_%s_%s",names[Hypo],names[Hypo2]) );

            multigraph_CosTh->Add(Eff_CosTh[Hypo2]);
            Eff_CosTh[Hypo2]->GetXaxis()->SetTitle(h_trk_CosTh[0]->GetXaxis()->GetTitle());
            Eff_CosTh[Hypo2]->GetYaxis()->SetTitle("Efficiency");

            h_Eff_CosTh[Hypo2]->GetListOfFunctions()->Add(new TNamed("Description", Form("PID(%s)>0.5 efficiency of truth-matched %s tracks in bins of lab momentum. A Generic BBbar sample is used.",names[Hypo],names[Hypo])));
            h_Eff_CosTh[Hypo2]->GetListOfFunctions()->Add(new TNamed("Check", "Stable, continuous efficiency."));
            h_Eff_CosTh[Hypo2]->GetListOfFunctions()->Add(new TNamed("Contact", contact));

            list->Add(Eff_CosTh[Hypo2]);
            if( Hypo2==Hypo ) list_web->Add(h_Eff_CosTh[Hypo2]);
        }
        multigraph_CosTh->Draw("ap");
        multigraph_CosTh->GetXaxis()->SetTitle(h_trk_CosTh[0]->GetXaxis()->GetTitle());
        multigraph_CosTh->GetYaxis()->SetTitle("Efficiency");
        c_pidvalidation->Print("test2_Validation_PID_plots.pdf",Form("Title: Efficiency, %s hypothesis",names[Hypo]));

        if(runOffline) {
            c_pidvalidation->cd()->SetGridy(1);
            c_pidvalidation->cd()->SetGridx(1);
            TMultiGraph* multigraph_offline_CosTh = new TMultiGraph;
            multigraph_offline_CosTh->Add(Eff_CosTh[pion]);
            multigraph_offline_CosTh->Add(Eff_CosTh[kaon]);
            multigraph_offline_CosTh->Draw("ap");
            multigraph_offline_CosTh->GetXaxis()->SetTitle(h_trk_CosTh[0]->GetXaxis()->GetTitle());
            multigraph_offline_CosTh->GetYaxis()->SetTitle("Efficiency");
            //if(region==4)Belle2Labels(0.5,0.9,"Endcaps");
            //if(region==2)Belle2Labels(0.5,0.9,"Barrel");
            c_pidvalidation->Print("test2_Validation_PID_plots.pdf",Form("Title: Efficiency, %s hypothesis",names[Hypo]));
        }
        TFile* outFile = new TFile("test2_Validation_PID_output.root", "update");
        TDirectory* dir = outFile->mkdir(Form("%s",names[Hypo]));
        dir->cd();
        list->Write();
        multigraph_P->Write();
        multigraph_CosTh->Write();
        outFile->Close();
    }
    TFile* outFile_web = new TFile("test2_Validation_PID_output.root", "update");
    list_web->Write();
    outFile_web->Close();

    c_pidvalidation->Clear();
    c_pidvalidation->Print("test2_Validation_PID_plots.pdf","");
    c_pidvalidation->Print("test2_Validation_PID_plots.pdf]");
}

