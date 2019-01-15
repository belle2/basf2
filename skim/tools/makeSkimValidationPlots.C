

#include "TPad.h"
#include "TGaxis.h"
#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TCanvas.h"
#include <vector>
using namespace std;




void makeSkimValidationPlots(){

vector<TString> skims={"ALP3Gamma", "BottomoniumEtabExclusive", "BottomoniumUpsilon", "TauGeneric", "SystematicsRadMuMu", "SystematicsRadEE", "LFVZpInvisible", "LFVZpVisible" ,"SinglePhotonDark", "SystematicsTracking",  "SystematicsLambda", " Systematics", "ISRpipicc", "BtoDh_Kspipipi0", "BtoPi0Pi0", "feiSLB0WithOneLep",  "feiHadronicB0", "feiHadronicBplus" ,  "BtoDh_Kspi0",  "BtoDh_hh", "TauGeneric",  "PRsemileptonicUntagged","SLUntagged", "LeptonicUntagged", "TCPV", "CharmRare", "BtoXll", "BtoXgamma",  "TauLFV"}; 
vector<Double_t> Rel3_retention={0.000492,0.000128,0.0,0.144492,0.028371,0.004014,0.012242,0.142264,0.000114,0.008414,0.0,0.038471,0.000135,0.006635,0.000378,0.045271,0.034292,0.06695,0.003807,0.017128,0.144492,0.010978,0.0083,0.021985,0.031657,0.006335,0.022892,0.046742,0.002235};
vector<Double_t> Rel3_evtSize={10.19831,11.12507,0.0,7.427172,15.81789,24.82281,15.32312,7.943524,26.76620,17.36287,0.0,8.760236,40.02806,20.31377,18.97929,15.41394,15.80385,11.75710,18.15921,10.19796,7.427172,12.05886,19.92752,8.438124,12.03479,14.03856,21.06394,11.39889,51.92646};
vector<Double_t> Rel3_time={0.025418,0.023024,0.019019,0.029172,0.020699,0.016961,0.030245,0.595546,0.021711,0.036659,0.014717,0.066039,0.032575,0.0,0.814409,1.120394,1.084213,1.190391,0.0,0.0,0.029172,0.027223,0.064351,0.231159,1.038901,0.199146,0.383116,0.326240,0.270773};


vector<Double_t> Rel2_Retention={0.000471,0.000142,0.000742,0.146221,0.034635,0.003992,0.012242,0.142264,0.000121,0,0.013050,0.051835,0.021614,0.006635,0.000378,0.045078,0.0329,0.066185,0.003807,0.017128,0.146221,0.010978,0.010864,0.021985,0.031592,0.047314,0.0,0.047978,0.005200};
vector<Double_t> Rel2_EvtSize={10.10244,10.51767,17.11329,7.517369,14.59925,24.48618,15.17643,8.125865,17.84204,0,13.43906,9.957251,11.59387,20.22167,18.81661,12.11161,12.48399,11.87566,18.06592,10.18103,7.517369,12.01894,15.33952,8.427725,11.91536,10.05296,0.0,11.54691,15.67407};
vector<Double_t> Rel2_Time={0.019835,0.025768,0.027662,0.261987,0.038358,0.032624,0.033819,0.572852,0.035567,0,0.052625,0.047081,0.047331,0.799448,0.776370,1.113844,1.079201,1.170873,0.203038,0.039925,0.261987,0.040218,0.074386,0.234856,0.999871,0.198030,0.0,0.333820,0.291357};
Int_t r= Rel3_retention.size();
Int_t r1= Rel2_Retention.size();
cout<<"Rel3 "<<r<<" and rel2 "<<r1<<endl; 

Int_t nSkims=skims.size();
Int_t nBins=nSkims+2;
TH1F * h1=new TH1F("RetentionRel3","Retention\% Prerelease-03-00-00b", nBins,0,nBins);
TH1F * h2= new TH1F("EventSizeRel3","Event Size Prerelease-03-00-00b",nBins, 0, nBins);
TH1F * h3 =new TH1F("ProcessingTimeRel3","Processing Time Prerelease-03-00-00b",nBins,0,nBins);
h1->SetMarkerStyle(23);
h2->SetMarkerStyle(23);
h3->SetMarkerStyle(23);
h1->SetLineColor(kRed);
h1->SetMarkerColor(kRed);
h2->SetMarkerColor(kRed);
h3->SetMarkerColor(kRed);
TH1F * h4=new TH1F("RetentionRel2","Retention\% release-02-00-02", nBins,0,nBins);
TH1F * h5= new TH1F("EventSizeRel2","Event Size release-02-00-02",nBins, 0, nBins);
TH1F * h6 =new TH1F("ProcessingTimeRel2","Processing Time release-02-00-002",nBins,0,nBins);
h4->SetMarkerStyle(23);
h5->SetMarkerStyle(23);
h6->SetMarkerStyle(23);
h4->SetMarkerColor(kBlue);
h4->SetLineColor(kBlue);
h5->SetMarkerColor(kBlue);
h6->SetMarkerColor(kBlue);

for (int i=0;i<skims.size();i++)
{
h1->Fill(i+1,Rel3_retention[i]*10000);
h2->Fill(i+1,Rel3_evtSize[i]*100);
h3->Fill(i+1,Rel3_time[i]*100); 
h1->GetXaxis()->SetBinLabel(i+1,skims[i]);
h2->GetXaxis()->SetBinLabel(i+1,skims[i]);
h3->GetXaxis()->SetBinLabel(i+1,skims[i]);

h4->Fill(i+1,Rel2_Retention[i]*10000);
h5->Fill(i+1,Rel2_EvtSize[i]*100);
h6->Fill(i+1,Rel2_Time[i]*100); 
h4->GetXaxis()->SetBinLabel(i+1,skims[i]);
h5->GetXaxis()->SetBinLabel(i+1,skims[i]);
h6->GetXaxis()->SetBinLabel(i+1,skims[i]);


}
h1->GetYaxis()->SetTitle("Retention x100 (\%)");
h2->GetYaxis()->SetTitle("Event size x100 (KB)");
h3->GetYaxis()->SetTitle("Processing Time /event x100 (HEPSEC)");
h4->GetYaxis()->SetTitle("Retention x100 (\%)");
h5->GetYaxis()->SetTitle("Event size x100(KB)");
h6->GetYaxis()->SetTitle("Processing Time /event x100(HEPSEC)");


TLegend * leg= new TLegend(0.76,0.7,0.9,0.9);
leg->AddEntry(h1,"Prerelease-03","f");
leg->AddEntry(h4,"Release-02","f");

TCanvas *c1 =new TCanvas("c1","c1",600,700);
c1->cd();
h1->Draw("el");
h4->Draw("elsame");
leg->Draw();

TCanvas *c2 =new TCanvas("c2","c2",600,700);
c2->cd();
h2->Draw("pl");
h5->Draw("plsame");
leg->Draw();


TCanvas *c3 =new TCanvas("c3","c3",600,700);
c3->cd();
h3->Draw("el");
h6->Draw("elsame");
leg->Draw();


TFile * outputFile =new TFile("performance.root","RECREATE");



outputFile->cd();
h1->Write();
h2->Write();
h3->Write();
h4->Write();
h5->Write();
h6->Write();
c1->Write();
c2->Write();
c3->Write();
outputFile->Close();

}
