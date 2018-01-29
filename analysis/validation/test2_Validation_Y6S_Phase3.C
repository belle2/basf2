/*
<header>
<input>../Y6S_nonbsbs_Phase3.ntup.root</input>
<output>Y6S_Validation_Phase3.root</output>
<contact>Bryan Fulsom, bryan.fulsom@pnnl.gov; Cate MacQueen, cmq.centaurus@gmail.com</contact>
<description>Reconstruction efficiency for single particles for Y(6S) in phase 3 geometry</description>
</header>
*/

// Phillip Urquijo, Luis Pesantez Jul 2013
//                                Oct 2014
// Sam Cunliffe                   Jan 2018

#include <vector>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include <TChain.h>
#include <TCanvas.h>
#include <TDirectory.h>
#include <TError.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TGraphAsymmErrors.h>
#include <TH1.h>
#include <TLegend.h>
#include <TList.h>
#include <TLorentzVector.h>
#include <TROOT.h>
#include <TSelector.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TTree.h>
#include <TVector3.h>

//#include <Belle2Labels.h>

enum hypo {pion=0, photon=1};
const int pid[] = {211, 22};
enum ecl_regions {All=0, FW=1, Barrel=2, BW=3};
const char* regions[]= { "All", "FW", "Barrel", "BW" };

void efficiency_and_plots( TH1* truth, TH1* reco, TGraphAsymmErrors* efficiency, TH1F* h_efficiency, TCanvas* canvas, string sOutput, string sTitle ) {
    TLegend* leg = new TLegend(0.70,0.75,0.90,0.90);
    leg->SetFillColor(0);
    leg->AddEntry(truth,"Truth","l");
    leg->AddEntry(reco,"Reco","l");

    canvas->cd();
    truth->SetMinimum(0);
    truth->SetLineColor(kBlue);
    truth->GetXaxis()->SetRange(truth->FindFirstBinAbove(),truth->FindLastBinAbove());
    reco ->SetMinimum(0);
    reco->SetLineColor(kRed);
    reco->SetLineStyle(2);
    if( truth->GetMaximum()>reco->GetMaximum() ) {
        truth->Draw();
        reco->Draw("same");
    }
    else {
        reco->Draw();
        truth->Draw("same");
    }
    //Belle2Labels(0.4, 0.96, Form( "%s", sTitle.c_str() ) );
    leg->Draw();
    //canvas->Print( Form("%s",sOutput.c_str()), Form("sTitle:%s truth and reco", sTitle.c_str()) );
    canvas->Print( Form("%s",sOutput.c_str()), Form("%s truth and reco", sTitle.c_str()) );

    // This is only for the website, it doesn't accept TGraphs
    //canvas->Clear();
    h_efficiency->SetMarkerColor(kBlue);
    h_efficiency->SetMarkerStyle(21);
    h_efficiency->Divide( reco, truth, 1, 1 );
    string sTrack("Track");
    if (sTitle.find(sTrack) != std::string::npos)   h_efficiency->GetYaxis()->SetTitle("Track efficiency");
    else                                            h_efficiency->GetYaxis()->SetTitle("Photon efficiency");
    //h_efficiency->Draw("");
    //canvas->Print( Form("%s",sOutput.c_str()), Form("%s eff", sTitle.c_str()) );

    canvas->Clear();
    efficiency->Divide(reco ,truth ,"cl=0.683 b(1,1) mode");
    efficiency->SetFillColor(kBlue);
    efficiency->SetMarkerColor(kBlue);
    efficiency->SetMarkerStyle(21);
    efficiency->Draw("2A");
    // Determine Y axis limits
    double max = -1e2;
    double min = 1e2;
    Double_t *y = efficiency->GetY();
    Double_t *yh = efficiency->GetEYhigh();
    Double_t *yl = efficiency->GetEYlow();
    for( int nbin=0; nbin<efficiency->GetN(); nbin++ ) {
        if( max<(y[nbin]+1.5*yh[nbin]) )    max = y[nbin]+1.5*yh[nbin];
        if( min>(y[nbin]-1.5*yl[nbin]) )    min = y[nbin]-1.5*yl[nbin];
    }
    max*=10;
    max=ceil(max);
    max/=10;
    min*=10;
    min=floor(min);
    min/=10;

    efficiency->GetXaxis()->SetRangeUser( truth->GetBinLowEdge(1), truth->GetBinLowEdge( truth->GetNbinsX()+1 ) );
    efficiency->GetYaxis()->SetRangeUser( min,max );
    efficiency->GetXaxis()->SetTitle( truth->GetXaxis()->GetTitle() );
    if (sTitle.find(sTrack) != std::string::npos)   efficiency->GetYaxis()->SetTitle("Track efficiency");
    else                                            efficiency->GetYaxis()->SetTitle("Photon efficiency");

    //Belle2Labels(0.4, 0.96, Form( "%s", sTitle.c_str() ) );
    //canvas->Print( Form("%s",sOutput.c_str()), Form("sTitle:%s efficiency", sTitle.c_str()) );
    canvas->Print( Form("%s",sOutput.c_str()), Form("%s efficiency", sTitle.c_str()) );
    efficiency->SetName(Form("%s", sTitle.c_str()));
}

bool check_for_duplicate( std::vector<TLorentzVector> all_evt_tracks, TLorentzVector new_track ) {
    // Look at truth 4-momentum for all candidates in the same event. Include the new candidate each time and
    // compare to the rest. If a duplicate is found, return true and skip.
    if( all_evt_tracks.size()==0 ) return false;
    else {
        for( int i=0; i<(int)all_evt_tracks.size(); i++ ) {
            // Measure angle, if == 0 is duplicate, skip
            double angle = new_track.Vect().Angle( all_evt_tracks.at(i).Vect() );
            if( angle<0.001 ) return true;    // Found a duplicate
            //if( (new_track.E()-all_evt_tracks.at(i).E())<0.001 ) return true;
        }
    }
    return false;
}

void test2_Validation_Efficiency_Track_Truth(TString filename, TList* list) {
    TChain* tree = new TChain("chain");
    tree->Add(Form("%s/truthpituple",filename.Data()));

    float pi_P4[4];
    int iCand;
    int exp_no;
    int run_no;
    int evt_no;
    tree->SetBranchAddress("pi_P4", &pi_P4);
    tree->SetBranchAddress("iCand", &iCand);
    tree->SetBranchAddress("exp_no", &exp_no);
    tree->SetBranchAddress("run_no", &run_no);
    tree->SetBranchAddress("evt_no", &evt_no);

    // Run a check on track duplicates, one list for all candidates in event
    std::vector<TLorentzVector> all_evt_tracks;
    for(Int_t iloop=0; iloop<tree->GetEntries(); iloop++) {
        tree->GetEntry(iloop);
        TLorentzVector lv_pi(pi_P4);

        // For track duplicates
        if(iCand==0) {
            all_evt_tracks.clear();
        }

        if(lv_pi.Theta()>2.62 || lv_pi.Theta()<0.3) continue;

        if( check_for_duplicate( all_evt_tracks, lv_pi ) )   continue;
        all_evt_tracks.push_back(lv_pi);

        ( (TH1*)list->FindObject( Form( "pi_TruthP" ) ) )->Fill(lv_pi.Pt());
        if(lv_pi.Pt()<0.25) {
            ( (TH1*)list->FindObject( Form( "pi_TruthTheta_LowPt" ) ) )->Fill(lv_pi.Theta());
            ( (TH1*)list->FindObject( Form( "pi_TruthPhi_LowPt" ) ) )->Fill(lv_pi.Phi());
        }
        else {
            ( (TH1*)list->FindObject( Form( "pi_TruthTheta_HighPt" ) ) )->Fill(lv_pi.Theta());
            ( (TH1*)list->FindObject( Form( "pi_TruthPhi_HighPt" ) ) )->Fill(lv_pi.Phi());
        }
    }
}

void test2_Validation_Efficiency_Track_Reco(TString filename, TList* list) {
    TChain*  tree = new TChain("pituple");
    tree->AddFile(filename);

    float pi_TruthP4[4];
    int pi_mcErrors;
    int iCand;
    tree->SetBranchAddress("pi_mcErrors", &pi_mcErrors);
    tree->SetBranchAddress("pi_TruthP4",  &pi_TruthP4);
    tree->SetBranchAddress("iCand",       &iCand);

    // Run a check on track duplicates, one list for all candidates in event
    std::vector<TLorentzVector> all_evt_tracks;
    for(Int_t iloop=0; iloop<tree->GetEntries(); iloop++) {
        tree->GetEntry(iloop);
        TLorentzVector lv_pi(pi_TruthP4);

        if(lv_pi.Theta()>2.62 || lv_pi.Theta()<0.3) continue;
        // For track duplicates
        if(iCand==0) {
            all_evt_tracks.clear();
        }

        if( check_for_duplicate( all_evt_tracks, lv_pi ) )   continue;
        all_evt_tracks.push_back(lv_pi);

        if( pi_mcErrors>0 ) continue;

        ( (TH1*)list->FindObject( Form( "pi_P" ) ) )->Fill(lv_pi.Pt());
        if(lv_pi.Pt()<0.25) {
            ( (TH1*)list->FindObject( Form( "pi_Theta_LowPt" ) ) )->Fill(lv_pi.Theta());
            ( (TH1*)list->FindObject( Form( "pi_Phi_LowPt" ) ) )->Fill(lv_pi.Phi());
        }
        else {
            ( (TH1*)list->FindObject( Form( "pi_Theta_HighPt" ) ) )->Fill(lv_pi.Theta());
            ( (TH1*)list->FindObject( Form( "pi_Phi_HighPt" ) ) )->Fill(lv_pi.Phi());
        }
    }
}

void test2_Validation_Efficiency_Photon_Truth(TString filename, TList* list) {
    TChain*  tree = new TChain("truthgammatuple");
    tree->AddFile(filename);

    float gamma_P4[4];
    int iCand;
    int evt_no;
    tree->SetBranchAddress("gamma_P4", &gamma_P4);
    tree->SetBranchAddress("iCand",    &iCand);
    tree->SetBranchAddress("evt_no",   &evt_no);

    // Run a check on cluster duplicates
    std::vector<TLorentzVector> all_evt_clusters;
    int region=0;
    for(Int_t iloop=0; iloop<tree->GetEntries(); iloop++) {
        tree->GetEntry(iloop);
        TLorentzVector lv_gamma(gamma_P4);
        if(iCand==0) all_evt_clusters.clear();

        if(lv_gamma.E()<0.02) continue;
        if(lv_gamma.Theta()>2.71 || lv_gamma.Theta()<0.21) continue;
        if((lv_gamma.Theta()<0.58 && lv_gamma.Theta()>=0.21) ) region=1; //forward
        if((lv_gamma.Theta()<2.23 && lv_gamma.Theta()>=0.58) ) region=2; //barrel
        if((lv_gamma.Theta()<2.71 && lv_gamma.Theta()>=2.23) ) region=3; //backward

        if( check_for_duplicate( all_evt_clusters, lv_gamma ) )    continue;
        all_evt_clusters.push_back( lv_gamma );

        ( (TH1*)list->FindObject( Form( "gamma_TruthP_%s", regions[0] ) ) )->Fill(lv_gamma.E());
        ( (TH1*)list->FindObject( Form( "gamma_TruthP_%s", regions[region] ) ) )->Fill(lv_gamma.E());
        if(lv_gamma.E()<0.5) {
            ( (TH1*)list->FindObject( Form( "gamma_TruthTheta_LowE_%s", regions[0] ) ) )->Fill(lv_gamma.Theta());
            ( (TH1*)list->FindObject( Form( "gamma_TruthTheta_LowE_%s", regions[region] ) ) )->Fill(lv_gamma.Theta());
            ( (TH1*)list->FindObject( Form( "gamma_TruthPhi_LowE_%s", regions[0] ) ) )->Fill(lv_gamma.Phi());
            ( (TH1*)list->FindObject( Form( "gamma_TruthPhi_LowE_%s", regions[region] ) ) )->Fill(lv_gamma.Phi());
        }
        else {
            ( (TH1*)list->FindObject( Form( "gamma_TruthTheta_HighE_%s", regions[0] ) ) )->Fill(lv_gamma.Theta());
            ( (TH1*)list->FindObject( Form( "gamma_TruthTheta_HighE_%s", regions[region] ) ) )->Fill(lv_gamma.Theta());
            ( (TH1*)list->FindObject( Form( "gamma_TruthPhi_HighE_%s", regions[0] ) ) )->Fill(lv_gamma.Phi());
            ( (TH1*)list->FindObject( Form( "gamma_TruthPhi_HighE_%s", regions[region] ) ) )->Fill(lv_gamma.Phi());
        }
    }
}

void test2_Validation_Efficiency_Photon_Reco(TString filename, TList* list) {
    TChain*  tree = new TChain("gammatuple");
    tree->AddFile(filename);

    float gamma_clusterTiming;
    float gamma_TruthP4[4];
    float gamma_TruthP;
    int gamma_mcErrors;
    int iCand;
    tree->SetBranchAddress("gamma_clusterTiming",  &gamma_clusterTiming);
    tree->SetBranchAddress("gamma_TruthP4", &gamma_TruthP4);
    tree->SetBranchAddress("gamma_TruthP",  &gamma_TruthP);
    tree->SetBranchAddress("gamma_mcErrors",   &gamma_mcErrors);
    tree->SetBranchAddress("iCand",         &iCand);

    // Run a check on cluster duplicates
    std::vector<TLorentzVector> all_evt_clusters;
    int region=0;
    for(Int_t iloop=0; iloop<tree->GetEntries(); iloop++) {
        tree->GetEntry(iloop);
        TLorentzVector lv_gamma(gamma_TruthP4);
        if(iCand==0) all_evt_clusters.clear();

        if( gamma_mcErrors>0 ) continue;
        //if( abs(gamma_mcPDG)!=pid[photon] ) continue;
        if(lv_gamma.E()<0.02) continue;
        if(lv_gamma.Theta()>2.71 || lv_gamma.Theta()<0.21) continue;
        if((lv_gamma.Theta()<0.58 && lv_gamma.Theta()>=0.21) ) region=1; //forward
        if((lv_gamma.Theta()<2.23 && lv_gamma.Theta()>=0.58) ) region=2; //barrel
        if((lv_gamma.Theta()<2.71 && lv_gamma.Theta()>=2.23) ) region=3; //backward
        // TODO: Implement timing selection criteria
        // if( gamma_clusterTiming<0.75e3 || gamma_clusterTiming>2.5e3 ) continue;

        // For cluster duplicates
        if( check_for_duplicate( all_evt_clusters, lv_gamma ) )    continue;
        all_evt_clusters.push_back( lv_gamma );

        ( (TH1*)list->FindObject( Form( "gamma_P_%s", regions[0] ) ) )->Fill(gamma_TruthP);
        ( (TH1*)list->FindObject( Form( "gamma_P_%s", regions[region] ) ) )->Fill(gamma_TruthP);
        if(lv_gamma.E()<0.5) {
            ( (TH1*)list->FindObject( Form( "gamma_Theta_LowE_%s", regions[0] ) ) )->Fill(lv_gamma.Theta());
            ( (TH1*)list->FindObject( Form( "gamma_Theta_LowE_%s", regions[region] ) ) )->Fill(lv_gamma.Theta());
            ( (TH1*)list->FindObject( Form( "gamma_Phi_LowE_%s", regions[0] ) ) )->Fill(lv_gamma.Phi());
            ( (TH1*)list->FindObject( Form( "gamma_Phi_LowE_%s", regions[region] ) ) )->Fill(lv_gamma.Phi());
        }
        else {
            ( (TH1*)list->FindObject( Form( "gamma_Theta_HighE_%s", regions[0] ) ) )->Fill(lv_gamma.Theta());
            ( (TH1*)list->FindObject( Form( "gamma_Theta_HighE_%s", regions[region] ) ) )->Fill(lv_gamma.Theta());
            ( (TH1*)list->FindObject( Form( "gamma_Phi_HighE_%s", regions[0] ) ) )->Fill(lv_gamma.Phi());
            ( (TH1*)list->FindObject( Form( "gamma_Phi_HighE_%s", regions[region] ) ) )->Fill(lv_gamma.Phi());
        }
    }
}

void test2_Validation_Y6S_Phase3(bool runOffline=false) {
    char const *contact_for_all_plots = "Bryan Fulsom (bryan.fulsom@pnnl.gov), Cate MacQueen (cmq.centaurus@gmail.com)";
    //If running offline study then use finer binning
    int binfactor=1;
    if(runOffline)binfactor=2;
    int nthbins=16*binfactor;
    float thlow=0;
    float thhigh=3.2;
    float phlow=-3.2;
    float phhigh=3.2;

    int npbins=13;
    float plow=0.;
    float phigh=2.6;

    int nthgbins=25;
    float thglow=0.21;
    float thghigh=2.71;

    int nptbins=26*binfactor;
    float ptlow=0.;
    float pthigh=2.6;

    //Set the Binning, clones for truth and reco
    //Lab momentum
    TList* list_hist_tracks = new TList;
    TH1F* h_pi_TruthP = new TH1F("pi_TruthP" ,"All p_{T};p_{T}(#pi) GeV;N" ,nptbins,ptlow,pthigh);
    TH1F* h_pi_P = (TH1F*)h_pi_TruthP->Clone( "pi_P" );
    list_hist_tracks->Add(h_pi_TruthP);
    list_hist_tracks->Add(h_pi_P);

    TH1F* h_pi_TruthTheta_LowPt = new TH1F("pi_TruthTheta_LowPt", "p_{T}<250 MeV;#theta(#pi) ;N" ,nthbins,thlow,thhigh);
    TH1F* h_pi_Theta_LowPt = (TH1F*)h_pi_TruthTheta_LowPt->Clone( "pi_Theta_LowPt" );
    list_hist_tracks->Add(h_pi_TruthTheta_LowPt);
    list_hist_tracks->Add(h_pi_Theta_LowPt);

    TH1F* h_pi_TruthTheta_HighPt = new TH1F("pi_TruthTheta_HighPt","p_{T} #geq 250 MeV;#theta(#pi) ;N" ,nthbins,thlow,thhigh);
    TH1F* h_pi_Theta_HighPt = (TH1F*)h_pi_TruthTheta_HighPt->Clone( "pi_Theta_HighPt" );
    list_hist_tracks->Add(h_pi_TruthTheta_HighPt);
    list_hist_tracks->Add(h_pi_Theta_HighPt);

    TH1F* h_pi_TruthPhi_LowPt = new TH1F("pi_TruthPhi_LowPt", "p_{T}<250 MeV;#phi(#pi) ;N" ,nthbins,phlow,phhigh);
    TH1F* h_pi_Phi_LowPt = (TH1F*)h_pi_TruthPhi_LowPt->Clone( "pi_Phi_LowPt" );
    list_hist_tracks->Add(h_pi_TruthPhi_LowPt);
    list_hist_tracks->Add(h_pi_Phi_LowPt);

    TH1F* h_pi_TruthPhi_HighPt = new TH1F("pi_TruthPhi_HighPt","p_{T} #geq 250 MeV;#phi(#pi) ;N" ,nthbins,phlow,phhigh);
    TH1F* h_pi_Phi_HighPt = (TH1F*)h_pi_TruthPhi_HighPt->Clone( "pi_Phi_HighPt" );
    list_hist_tracks->Add(h_pi_TruthPhi_HighPt);
    list_hist_tracks->Add(h_pi_Phi_HighPt);

    //Photons
    TList* list_hist_photons = new TList;
    TH1F* h_gamma_TruthP[4];
    TH1F* h_gamma_TruthTheta_LowE[4];
    TH1F* h_gamma_TruthTheta_HighE[4];
    TH1F* h_gamma_TruthPhi_LowE[4];
    TH1F* h_gamma_TruthPhi_HighE[4];
    TH1F* h_gamma_P[4];
    TH1F* h_gamma_Theta_LowE[4];
    TH1F* h_gamma_Theta_HighE[4];
    TH1F* h_gamma_Phi_LowE[4];
    TH1F* h_gamma_Phi_HighE[4];
    for( int i=0; i<4; i++ ) {
        h_gamma_TruthP[i] = new TH1F(Form("gamma_TruthP_%s",regions[i]) ,Form("%s;E(#gamma) GeV;N",regions[i]) ,npbins,plow,phigh);
        h_gamma_P[i] = (TH1F*)h_gamma_TruthP[i]->Clone(Form("gamma_P_%s",regions[i]));
        list_hist_photons->Add(h_gamma_TruthP[i]);
        list_hist_photons->Add(h_gamma_P[i]);

        h_gamma_TruthTheta_LowE[i] = new TH1F(Form("gamma_TruthTheta_LowE_%s",regions[i]) ,Form("%s, E<500 MeV;#theta(#gamma) ;N",regions[i]) ,nthgbins,thglow,thghigh);
        h_gamma_Theta_LowE[i] = (TH1F*)h_gamma_TruthTheta_LowE[i]->Clone(Form("gamma_Theta_LowE_%s",regions[i]));
        list_hist_photons->Add(h_gamma_TruthTheta_LowE[i]);
        list_hist_photons->Add(h_gamma_Theta_LowE[i]);

        h_gamma_TruthTheta_HighE[i] = new TH1F(Form("gamma_TruthTheta_HighE_%s",regions[i]),Form("%s, E #geq 500 MeV;#theta(#gamma) ;N",regions[i]) ,nthgbins,thglow,thghigh);
        h_gamma_Theta_HighE[i] = (TH1F*)h_gamma_TruthTheta_HighE[i]->Clone(Form("gamma_Theta_HighE_%s",regions[i]));
        list_hist_photons->Add(h_gamma_TruthTheta_HighE[i]);
        list_hist_photons->Add(h_gamma_Theta_HighE[i]);

        h_gamma_TruthPhi_LowE[i] = new TH1F(Form("gamma_TruthPhi_LowE_%s",regions[i]) ,Form("%s, E<500 MeV;#phi(#gamma) ;N",regions[i]) ,nthbins,phlow,phhigh);
        h_gamma_Phi_LowE[i] = (TH1F*)h_gamma_TruthPhi_LowE[i]->Clone(Form("gamma_Phi_LowE_%s",regions[i]));
        list_hist_photons->Add(h_gamma_TruthPhi_LowE[i]);
        list_hist_photons->Add(h_gamma_Phi_LowE[i]);

        h_gamma_TruthPhi_HighE[i] = new TH1F(Form("gamma_TruthPhi_HighE_%s",regions[i]) ,Form("%s, E #geq 500 MeV;#phi(#gamma) ;N",regions[i]) ,nthbins,phlow,phhigh);
        h_gamma_Phi_HighE[i] = (TH1F*)h_gamma_TruthPhi_HighE[i]->Clone(Form("gamma_Phi_HighE_%s",regions[i]));
        list_hist_photons->Add(h_gamma_TruthPhi_HighE[i]);
        list_hist_photons->Add(h_gamma_Phi_HighE[i]);
    }


    TString filename ("../Y6S_nonbsbs_Phase3.ntup.root");

    TCanvas* maincanvas = new TCanvas ("maincanvas","maincanvas");
    maincanvas->Print("Y6S_Validation_plots3.pdf[");

    TList* list_web = new TList;


    ///////////////////// Tracks
    //Fill histograms
    test2_Validation_Efficiency_Track_Truth(filename, list_hist_tracks);
    test2_Validation_Efficiency_Track_Reco(filename, list_hist_tracks);

    list_web->Add( h_pi_P );

    TList* list_eff_tracks = new TList;
    TGraphAsymmErrors* Eff_Track = new TGraphAsymmErrors();
    TH1F* h_Eff_Track = (TH1F*)h_pi_TruthP->Clone( "h_Eff_Track" );
    efficiency_and_plots( h_pi_TruthP, h_pi_P, Eff_Track, h_Eff_Track, maincanvas,
                          string("Y6S_Validation_plots3.pdf"), string( "Track p_{T}" ) );
    h_Eff_Track->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction efficiency of truth-matched pion tracks with a pi hypothesis in bins of transverse momentum. input: Y(6S) generic sample in phase 3 geometry."));
    h_Eff_Track->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency. Steep rise in efficiency up to 90 percent at 0.5 GeV. p_{T}<0.5 GeV more sensitive to tracking algorithm changes."));
    h_Eff_Track->GetListOfFunctions()->Add(new TNamed("Contact", contact_for_all_plots));
    list_eff_tracks->Add( Eff_Track );
    list_web->Add( h_Eff_Track );

    TGraphAsymmErrors* Eff_TrackTheta_LowPt = new TGraphAsymmErrors();
    TH1F* h_Eff_TrackTheta_LowPt = (TH1F*)h_pi_Theta_LowPt->Clone( "h_Eff_TrackTheta_LowPt" );
    efficiency_and_plots( h_pi_TruthTheta_LowPt, h_pi_Theta_LowPt, Eff_TrackTheta_LowPt, h_Eff_TrackTheta_LowPt, maincanvas,
                          string("Y6S_Validation_plots3.pdf"), string("Track #theta_{lab}, p_{T}<250 MeV") );
    h_Eff_TrackTheta_LowPt->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction efficiency of truth-matched pion tracks with a pi hypothesis in bins of theta_lab. input: Y(6S) generic sample in phase 3 geometry. Track cut p_{T}<250 MeV"));
    h_Eff_TrackTheta_LowPt->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume."));
    h_Eff_TrackTheta_LowPt->GetListOfFunctions()->Add(new TNamed("Contact", contact_for_all_plots));
    list_eff_tracks->Add( Eff_TrackTheta_LowPt );
    list_web->Add( h_Eff_TrackTheta_LowPt );

    TGraphAsymmErrors* Eff_TrackTheta_HighPt = new TGraphAsymmErrors();
    TH1F* h_Eff_TrackTheta_HighPt = (TH1F*)h_pi_Theta_HighPt->Clone( "h_Eff_TrackTheta_HighPt" );
    efficiency_and_plots( h_pi_TruthTheta_HighPt, h_pi_Theta_HighPt, Eff_TrackTheta_HighPt, h_Eff_TrackTheta_HighPt, maincanvas,
                          string("Y6S_Validation_plots3.pdf"), string("Track #theta_{lab}, p_{T} #geq 250 MeV") );
    h_Eff_TrackTheta_HighPt->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction efficiency of truth-matched pion tracks with a pi hypothesis in bins of theta_lab. input: Y(6S) generic sample in phase 3 geometry. Track cut p_{T} #geq 250 MeV"));
    h_Eff_TrackTheta_HighPt->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume."));
    h_Eff_TrackTheta_HighPt->GetListOfFunctions()->Add(new TNamed("Contact", contact_for_all_plots));
    list_eff_tracks->Add( Eff_TrackTheta_HighPt );
    list_web->Add( h_Eff_TrackTheta_HighPt );

    TGraphAsymmErrors* Eff_TrackPhi_LowPt = new TGraphAsymmErrors();
    TH1F* h_Eff_TrackPhi_LowPt = (TH1F*)h_pi_Phi_LowPt->Clone( "h_Eff_TrackPhi_LowPt" );
    efficiency_and_plots( h_pi_TruthPhi_LowPt, h_pi_Phi_LowPt, Eff_TrackPhi_LowPt, h_Eff_TrackPhi_LowPt, maincanvas,
                          string("Y6S_Validation_plots3.pdf"), string("Track #phi_{lab}, p_{T}<250 MeV") );
    h_Eff_TrackPhi_LowPt->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction efficiency of truth-matched pion tracks with a pi hypothesis in bins of phi_lab. input: Y(6S) generic sample in phase 3 geometry. Track cut p_{T}<250 MeV"));
    h_Eff_TrackPhi_LowPt->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume. Periodic dip at #pi/2 modulo."));
    h_Eff_TrackPhi_LowPt->GetListOfFunctions()->Add(new TNamed("Contact", contact_for_all_plots));
    list_eff_tracks->Add( Eff_TrackPhi_LowPt );
    list_web->Add( h_Eff_TrackPhi_LowPt );

    TGraphAsymmErrors* Eff_TrackPhi_HighPt = new TGraphAsymmErrors();
    TH1F* h_Eff_TrackPhi_HighPt = (TH1F*)h_pi_Phi_HighPt->Clone( "h_Eff_TrackPhi_HighPt" );
    efficiency_and_plots( h_pi_TruthPhi_HighPt, h_pi_Phi_HighPt, Eff_TrackPhi_HighPt, h_Eff_TrackPhi_HighPt, maincanvas,
                          string("Y6S_Validation_plots3.pdf"), string("Track #phi_{lab}, p_{T} #geq 250 MeV") );
    h_Eff_TrackPhi_HighPt->GetListOfFunctions()->Add(new TNamed("Description", "Single track reconstruction efficiency of truth-matched pion tracks with a pi hypothesis in bins of phi_lab. input: Y(6S) generic sample in phase 3 geometry. Track cut p_{T} #geq 250 MeV"));
    h_Eff_TrackPhi_HighPt->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume. Periodic dip at #pi/2 modulo."));
    h_Eff_TrackPhi_HighPt->GetListOfFunctions()->Add(new TNamed("Contact", contact_for_all_plots));
    list_eff_tracks->Add( Eff_TrackPhi_HighPt );
    list_web->Add( h_Eff_TrackPhi_HighPt );

    ///////////////////// Photons
    // Fill histograms
    test2_Validation_Efficiency_Photon_Truth(filename, list_hist_photons);
    test2_Validation_Efficiency_Photon_Reco(filename, list_hist_photons);


    TList* list_eff_photons = new TList;
    TGraphAsymmErrors* Eff_Photon[4];
    TGraphAsymmErrors* Eff_PhotonTheta_LowE[4];
    TGraphAsymmErrors* Eff_PhotonTheta_HighE[4];
    TGraphAsymmErrors* Eff_PhotonPhLowE[4];
    TGraphAsymmErrors* Eff_PhotonPhHighE[4];

    TH1F* h_Eff_Photon[4];
    TH1F* h_Eff_PhotonTheta_LowE[4];
    TH1F* h_Eff_PhotonTheta_HighE[4];
    TH1F* h_Eff_PhotonPhLowE[4];
    TH1F* h_Eff_PhotonPhHighE[4];
    for( int i=0; i<4; i++ ) {
        Eff_Photon[i] = new TGraphAsymmErrors();
        h_Eff_Photon[i] = (TH1F*)h_gamma_P[i]->Clone( Form( "h_Eff_Photon_%s", regions[i] ) );
        efficiency_and_plots( h_gamma_TruthP[i], h_gamma_P[i], Eff_Photon[i], h_Eff_Photon[i], maincanvas,
                              string("Y6S_Validation_plots3.pdf"), string(Form("%s: Photon E", regions[i])) );
        h_Eff_Photon[i]->GetListOfFunctions()->Add(new TNamed("Description", Form("Single photon reconstruction efficiency of truth-matched photons in bins of energy. Input: Y(6S) generic sample in phase 3 geometry. ECL: %s", regions[i])));
        h_Eff_Photon[i]->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency, particularly at low E. Steep rise to 80 percent at 1 GeV."));
        h_Eff_Photon[i]->GetListOfFunctions()->Add(new TNamed("Contact", contact_for_all_plots));
        list_eff_photons->Add( Eff_Photon[i] );
        if(i==0) list_web->Add( h_Eff_Photon[i] );

        Eff_PhotonTheta_LowE[i] = new TGraphAsymmErrors();
        h_Eff_PhotonTheta_LowE[i] = (TH1F*)h_gamma_Theta_LowE[i]->Clone( Form( "h_Eff_PhotonTheta_LowE_%s", regions[i] ) );
        efficiency_and_plots( h_gamma_TruthTheta_LowE[i], h_gamma_Theta_LowE[i], Eff_PhotonTheta_LowE[i], h_Eff_PhotonTheta_LowE[i], maincanvas,
                              string("Y6S_Validation_plots3.pdf"), string(Form("%s: Photon #theta_{lab}, E<500 MeV", regions[i])) );
        h_Eff_PhotonTheta_LowE[i]->GetListOfFunctions()->Add(new TNamed("Description", Form("Single photon reconstruction efficiency of truth-matched photons in bins of theta_lab. Input: Y(6S) generic sample in phase 3 geometry. Photon cut E<500 MeV. ECL: %s", regions[i])));
        h_Eff_PhotonTheta_LowE[i]->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume. Dips at 0.6 and 2.3."));
        h_Eff_PhotonTheta_LowE[i]->GetListOfFunctions()->Add(new TNamed("Contact", contact_for_all_plots));
        list_eff_photons->Add( Eff_PhotonTheta_LowE[i] );
        if(i==0) list_web->Add( h_Eff_PhotonTheta_LowE[i] );

        Eff_PhotonTheta_HighE[i] = new TGraphAsymmErrors();
        h_Eff_PhotonTheta_HighE[i] = (TH1F*)h_gamma_Theta_HighE[i]->Clone( Form( "h_Eff_PhotonTheta_HighE_%s", regions[i] ) );
        efficiency_and_plots( h_gamma_TruthTheta_HighE[i], h_gamma_Theta_HighE[i], Eff_PhotonTheta_HighE[i], h_Eff_PhotonTheta_HighE[i], maincanvas,
                              string("Y6S_Validation_plots3.pdf"), string(Form("%s: Photon #theta_{lab}, E #geq 500 MeV", regions[i])) );
        h_Eff_PhotonTheta_HighE[i]->GetListOfFunctions()->Add(new TNamed("Description", Form("Single photon reconstruction efficiency of truth-matched photons in bins of theta_lab. Input: Y(6S) generic sample in phase 3 geometry. Photon cut E #geq 500 MeV. ECL: %s", regions[i])));
        h_Eff_PhotonTheta_HighE[i]->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume. Dips at 0.6 and 2.3."));
        h_Eff_PhotonTheta_HighE[i]->GetListOfFunctions()->Add(new TNamed("Contact", contact_for_all_plots));
        list_eff_photons->Add( Eff_PhotonTheta_HighE[i] );
        if(i==0) list_web->Add( h_Eff_PhotonTheta_HighE[i] );

        Eff_PhotonPhLowE[i] = new TGraphAsymmErrors();
        h_Eff_PhotonPhLowE[i] = (TH1F*)h_gamma_Phi_LowE[i]->Clone( Form( "h_Eff_PhotonPhLowE_%s", regions[i] ) );
        efficiency_and_plots( h_gamma_TruthPhi_LowE[i], h_gamma_Phi_LowE[i], Eff_PhotonPhLowE[i], h_Eff_PhotonPhLowE[i], maincanvas,
                              string("Y6S_Validation_plots3.pdf"), string(Form("%s: Photon #phi_{lab}, E<500 MeV", regions[i])) );
        h_Eff_PhotonPhLowE[i]->GetListOfFunctions()->Add(new TNamed("Description", Form("Single photon reconstruction efficiency of truth-matched photons in bins of phi_lab. Input: Y(6S) generic sample in phase 3 geometry. Photon cut E<500 MeV. ECL: %s", regions[i])));
        h_Eff_PhotonPhLowE[i]->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume. Flat across the spectrum."));
        h_Eff_PhotonPhLowE[i]->GetListOfFunctions()->Add(new TNamed("Contact", contact_for_all_plots));
        list_eff_photons->Add( Eff_PhotonPhLowE[i] );
        if(i==0) list_web->Add( h_Eff_PhotonPhLowE[i] );

        Eff_PhotonPhHighE[i] = new TGraphAsymmErrors();
        h_Eff_PhotonPhHighE[i] = (TH1F*)h_gamma_Phi_HighE[i]->Clone( Form( "h_Eff_PhotonPhHighE_%s", regions[i] ) );
        efficiency_and_plots( h_gamma_TruthPhi_HighE[i], h_gamma_Phi_HighE[i], Eff_PhotonPhHighE[i], h_Eff_PhotonPhHighE[i], maincanvas,
                              string("Y6S_Validation_plots3.pdf"), string(Form("%s: Photon #phi_{lab}, E #geq 500 MeV", regions[i]) ) );
        h_Eff_PhotonPhHighE[i]->GetListOfFunctions()->Add(new TNamed("Description", Form("Single photon reconstruction efficiency of truth-matched photons in bins of phi_lab. Input: Y(6S) generic sample in phase 3 geometry. Photon cut E #geq 500 MeV. ECL: %s", regions[i])));
        h_Eff_PhotonPhHighE[i]->GetListOfFunctions()->Add(new TNamed("Check", "Stable efficiency through the detector volume. Flat across the spectrum."));
        h_Eff_PhotonPhHighE[i]->GetListOfFunctions()->Add(new TNamed("Contact", contact_for_all_plots));
        list_eff_photons->Add( Eff_PhotonPhHighE[i] );
        if(i==0) list_web->Add( h_Eff_PhotonPhHighE[i] );
    }

    TFile* output = new TFile("Y6S_Validation_Phase3.root", "recreate");
    TDirectory* dir_tracks = output->mkdir("Tracks");
    dir_tracks->cd();
    list_eff_tracks->Write();
    TDirectory* dir_photons = output->mkdir("Photons");
    dir_photons->cd();
    list_eff_photons->Write();

    output->cd();
    list_web->Write();
    output->Close();

    maincanvas->Print("Y6S_Validation_plots3.pdf]");
}

