/*
<header>
<input>GenericB.ntup.root</input>
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
    VarInfo DetectorStatsRec[] = {
        {"nARICHAeroHits",      0,  20},
        {"nARICHLikelihoods",   0,  20},
        {"nCDCHits",            0,  2500},
        {"nDedxLikelihoods",    0,  20},
        {"nECLDsps",            0,  1000},
        {"nECLHitAssignments",  0,  250},
        {"nECLHits",            0,  2500},
        {"nECLShowers",         0,  100},
        {"nECLTrigs",           0,  5},
        {"nEKLMHit2ds",         0,  100},
        {"nEKLMK0Ls",           0,  10},
        {"nGFTracks",           0,  25},
        {"nPIDLikelihoods",     0,  25},
        {"nPXDClusters",        0,  100},
        {"nSVDClusters",        0,  400},
        {"nTOPBarHits",         0,  1000},
        {"nTOPLikelihoods",     0,  20},
        {"nTrackFitResults",    0,  30}
    };
    VarInfo DetectorStatsSim[] = {
        {"nARICHDigits",    0,  200},
        {"nARICHSimHits",   0,  250},
        {"nCDCSimHits",     0,  11000},
        {"nECLDigits",      0,  1500},
        {"nECLSimHits",     0,  20000},
        {"nEKLMDigits",     0,  250},
        {"nEKLMSimHits",    0,  1000},
        {"nPXDDigits",      0,  500},
        {"nPXDSimHits",     0,  200},
        {"nPXDTrueHits",    0,  100},
        {"nSVDDigits",      0,  15000},
        {"nSVDSimHits",     0,  500},
        {"nSVDTrueHits",    0,  200},
        {"nTOPDigits",      0,  1000},
        {"nTOPSimHits",     0,  800},
        {"nTOPSimPhotons",  0,  800}
    };
    static const int NVarsRecoStats = 10;//sizeof(RecoStats)/sizeof(RecoStats[0]);
    static const int NVarsDetectorStatsRec = 18;//sizeof(DetectorStatsRec)/sizeof(DetectorStatsRec[0]);
    static const int NVarsDetectorStatsSim = 16;//sizeof(DetectorStatsSim)/sizeof(DetectorStatsSim[0]);

    TH1F *h_RecoStats[NVarsRecoStats];
    TH1F *h_DetectorStatsRec[NVarsDetectorStatsRec];
    TH1F *h_DetectorStatsSim[NVarsDetectorStatsSim];

    Float_t RecoStatsMeans[NVarsRecoStats];
    Float_t DetectorStatsRecMeans[NVarsDetectorStatsRec];
    Float_t DetectorStatsSimMeans[NVarsDetectorStatsSim];
    
    Float_t RecoStatsRMS[NVarsRecoStats];
    Float_t DetectorStatsRecRMS[NVarsDetectorStatsRec];
    Float_t DetectorStatsSimRMS[NVarsDetectorStatsSim];

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
        RecoStatsRMS[i]=h_RecoStats[i]->GetRMS();
    }
    
    for(int i=0; i<NVarsDetectorStatsRec; i++) {
        tc->cd(i+1);
        int nbins = DetectorStatsRec[i].VarRangeHigh - DetectorStatsRec[i].VarRangeLow;
        if(nbins>50)    nbins/=(int)( (DetectorStatsRec[i].VarRangeHigh - DetectorStatsRec[i].VarRangeLow) / 50);

        h_DetectorStatsRec[i]= new TH1F(Form("h%s",DetectorStatsRec[i].VarName),Form(";%s;Events",DetectorStatsRec[i].                            VarName),nbins,DetectorStatsRec[i].VarRangeLow-0.5,DetectorStatsRec[i].VarRangeHigh-0.5);
        recoTree->Draw(Form("%s>>h%s",DetectorStatsRec[i].VarName,DetectorStatsRec[i].VarName),"","");
        h_DetectorStatsRec[i]->SetLineColor(kBlue);
        h_DetectorStatsRec[i]->Draw("hist");
        tc->Print("test2_Validation_RecoStats_plots.pdf");
        printf( "%s\t\t%.2f\t\t%.2f\n", DetectorStatsRec[i].VarName, h_DetectorStatsRec[i]->GetMean(), h_DetectorStatsRec[i]->GetRMS() );
        DetectorStatsRecMeans[i]=h_DetectorStatsRec[i]->GetMean();
        DetectorStatsRecRMS[i]=h_DetectorStatsRec[i]->GetRMS();
    }
    for(int i=0; i<NVarsDetectorStatsSim; i++) {
        tc->cd(i+1);
        int nbins = DetectorStatsSim[i].VarRangeHigh - DetectorStatsSim[i].VarRangeLow;
        if(nbins>50)    nbins/=(int)( (DetectorStatsSim[i].VarRangeHigh - DetectorStatsSim[i].VarRangeLow) / 50);

        h_DetectorStatsSim[i]= new TH1F(Form("h%s",DetectorStatsSim[i].VarName),Form(";%s;Events",DetectorStatsSim[i].
                                        VarName),nbins,DetectorStatsSim[i].VarRangeLow-0.5,DetectorStatsSim[i].VarRangeHigh-0.5);
        recoTree->Draw(Form("%s>>h%s",DetectorStatsSim[i].VarName,DetectorStatsSim[i].VarName),"","");
        h_DetectorStatsSim[i]->SetLineColor(kBlue);
        h_DetectorStatsSim[i]->Draw("hist");
        tc->Print("test2_Validation_RecoStats_plots.pdf");
        printf( "%s\t\t%.2f\t\t%.2f\n", DetectorStatsSim[i].VarName, h_DetectorStatsSim[i]->GetMean(), h_DetectorStatsSim[i]->GetRMS() );
        DetectorStatsSimMeans[i]=h_DetectorStatsSim[i]->GetMean();
        DetectorStatsSimRMS[i]=h_DetectorStatsSim[i]->GetRMS();
    }

    tc->Print("test2_Validation_RecoStats_plots.pdf]");

    string RecoStatsList;
    for(int i=0; i<NVarsRecoStats; i++) {
        if( i<NVarsRecoStats-1) RecoStatsList = RecoStatsList + Form("%s:",RecoStats[i].VarName);
        else RecoStatsList = RecoStatsList + Form("%s",RecoStats[i].VarName);
    }
    string DetectorStatsRecList;
    for(int i=0; i<NVarsDetectorStatsRec; i++){
        if( i<NVarsDetectorStatsRec-1) DetectorStatsRecList = DetectorStatsRecList + Form("%s:",DetectorStatsRec[i].VarName);
        else DetectorStatsRecList = DetectorStatsRecList + Form("%s",DetectorStatsRec[i].VarName);
    }
    string DetectorStatsSimList;
    for(int i=0; i<NVarsDetectorStatsSim; i++){
        if( i<NVarsDetectorStatsSim-1) DetectorStatsSimList = DetectorStatsSimList + Form("%s:",DetectorStatsSim[i].VarName);
        else DetectorStatsSimList = DetectorStatsSimList + Form("%s",DetectorStatsSim[i].VarName);
    }
    
    TFile * output = TFile::Open("test2_Validation_RecoStats_output.root", "recreate");
    TNtuple* trecostats = new TNtuple("RecoStats", "Reco Stats: N(objects)/Event", Form("%s",string(RecoStatsList).c_str()));
    trecostats->SetAlias("Description", "Average MDST reconstruction object multiplicities per event. Useful to trace back problems to specific input.");
    trecostats->SetAlias("Check", "Look for any MDST classes that are empty, or changed substantially.");
    trecostats->Fill(RecoStatsMeans);

    TNtuple* trecstats = new TNtuple("DetectorStats", "Detector Stats: N(objects)/Event", Form("%s",string(DetectorStatsRecList).c_str()));
    trecstats->SetAlias("Description", "Average MDST reconstruction object multiplicities per event. Useful to trace back problems to specific input.");
    trecstats->SetAlias("Check", "Look for any MDST classes that are empty, or changed substantially. Note that MCParticles may change substantially in the case that secondary particles are preserved by subdetector reculation.");
    trecstats->Fill(DetectorStatsRecMeans);

    TNtuple* tsimstats = new TNtuple("SimStats", "Detector Sim Stats: N(objects)/Event", Form("%s",string(DetectorStatsSimList).c_str()));
    tsimstats->SetAlias("Description", "Average MDST simulation object multiplicities per event. Useful to trace back problems to specific input.");
    tsimstats->SetAlias("Check", "Look for any MDST classes that are empty, or changed substantially. Note that MCParticles may change substantially in the case that secondary particles are preserved by subdetector simulation.");
    tsimstats->Fill(DetectorStatsSimMeans);

    output->cd();
    trecostats->Write();
    trecstats->Write();
    tsimstats->Write();
    output->Close();
}

