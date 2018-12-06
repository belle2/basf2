/*
<header>
<input>../GenericB.ntup.root</input>
<output>test2_Validation_RecoStats_output.root</output>
<contact>Luis Pesantez, pesantez@uni-bonn.de</contact>
</header>
*/
////////////////////////////////////////////////////////////
// Validation_RecoStats.cc
// Check the detector sim and reco stats
//
// Phillip Urquijo, Luis Pesantez
// May 3, 2013
// Update: Oct 11, 2014
//
#include <vector>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include <TChain.h>
#include <TCanvas.h>
#include <TError.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TNtuple.h>
#include <TROOT.h>
#include <TTree.h>

void test2_Validation_RecoStats() {
    TChain * recoTree = new TChain("eventtuple");
    recoTree->AddFile("../GenericB.ntup.root");

    struct VarInfo {
        const char* VarName;
        int VarRangeLow;
        int VarRangeHigh;
    };

    VarInfo RecoStats[] = {
        {"nECLClusters",          0,  100},
        {"nNeutralECLClusters",   0,  100},
        {"nChargedECLClusters",   0,  40},
        {"nGoodNeutralECLClusters",0, 40},
        {"neutralECLEnergy",      0,  10},
        {"chargedECLEnergy",      0,  10},
        {"goodNeutralECLEnergy",  0,  10},
        {"nTracks",               0,  50},
        {"nParticles",            0,  500},
        {"nMCParticles",          0,  500}
    };

    static const int NVarsRecoStats = 10;//sizeof(RecoStats)/sizeof(RecoStats[0]);

    TH1F *h_RecoStats[NVarsRecoStats];

    Float_t RecoStatsMeans[NVarsRecoStats];
    
    // Float_t RecoStatsRMS[NVarsRecoStats];

    TCanvas *tc = new TCanvas ("tcReco","tcReco",1000,800);
    //tc->Divide(6,4);
    tc->Print("test2_Validation_RecoStats_plots.pdf[");

    for(int i=0; i<NVarsRecoStats; i++) {
        tc->cd(i+1);
        int nbins = RecoStats[i].VarRangeHigh - RecoStats[i].VarRangeLow;
        if(nbins>50)    nbins/=(int)( (RecoStats[i].VarRangeHigh - RecoStats[i].VarRangeLow) / 50);

        h_RecoStats[i]= new TH1F(Form("h%s",RecoStats[i].VarName),Form(";%s;Events",RecoStats[i].VarName),
                                 nbins,RecoStats[i].VarRangeLow-0.5,RecoStats[i].VarRangeHigh-0.5);
        recoTree->Draw(Form("%s>>h%s",RecoStats[i].VarName,RecoStats[i].VarName),"","");
        h_RecoStats[i]->SetLineColor(kBlue);
        h_RecoStats[i]->Draw("hist");
        tc->Print("test2_Validation_RecoStats_plots.pdf");
        printf( "%s\t\t%.2f\t\t%.2f\n", RecoStats[i].VarName, h_RecoStats[i]->GetMean(), h_RecoStats[i]->GetRMS() );
        RecoStatsMeans[i]=h_RecoStats[i]->GetMean();
        // RecoStatsRMS[i]=h_RecoStats[i]->GetRMS();
    }
    
    tc->Print("test2_Validation_RecoStats_plots.pdf]");

    string RecoStatsList;
    for(int i=0; i<NVarsRecoStats; i++) {
        if( i<NVarsRecoStats-1) RecoStatsList = RecoStatsList + Form("%s:",RecoStats[i].VarName);
        else RecoStatsList = RecoStatsList + Form("%s",RecoStats[i].VarName);
    }
    
    TFile * output = TFile::Open("test2_Validation_RecoStats_output.root", "recreate");
    TNtuple* trecostats = new TNtuple("RecoStats", "Reco Stats: N(objects)/Event", Form("%s",string(RecoStatsList).c_str()));
    trecostats->SetAlias("Description", "Average MDST reconstruction object multiplicities per event. Useful to trace back problems to specific input.");
    trecostats->SetAlias("Check", "Look for any MDST classes that are empty, or changed substantially.");
    trecostats->SetAlias("Contact", "sam.cunliffe@desy.de");
    trecostats->Fill(RecoStatsMeans);

    output->cd();
    trecostats->Write();
    output->Close();
}

