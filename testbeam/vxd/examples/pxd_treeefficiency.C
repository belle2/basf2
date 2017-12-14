{ 

gROOT->Reset(); 
gROOT->SetStyle("Plain"); 

gStyle->SetOptFit();
gStyle->SetPalette(1);
gStyle->SetCanvasColor(0);
gStyle->SetTitleFillColor(0);
gStyle->SetPaintTextFormat("1f");
gStyle->SetOptStat(0);



//
// event data 
TFile *ftb = new TFile("run000096_PXDEfficiency.root");
TTree *ttbtrack = (TTree*) ftb->Get("PXDEffiTree");


TCanvas * c1  = new TCanvas("c1","c1",600,400);
c1->SetLeftMargin(0.2);
c1->SetRightMargin(0.1);
c1->SetTopMargin(0.1);
c1->SetBottomMargin(0.16);


ttbtrack->Draw(" vcell_fit_1_1_2 >> htotal_vcell_1_1_2(100,0,768)" , "ucell_fit_1_1_2 < 150 && otherpxd_cluster_matched_1_1_2==0" );
ttbtrack->Draw(" vcell_fit_1_1_2 >> hmatch_vcell_1_1_2(100,0,768)" , "ucell_fit_1_1_2 < 150 && cluster_matched_1_1_2==0 && otherpxd_cluster_matched_1_1_2==0" );

ttbtrack->Draw(" vcell_fit_1_1_2 >> htotal_roi_vcell_1_1_2(100,0,768)" , "ucell_fit_1_1_2 < 150 " );
ttbtrack->Draw(" vcell_fit_1_1_2 >> hmatch_roi_vcell_1_1_2(100,0,768)" , "ucell_fit_1_1_2 < 150 && cluster_matched_1_1_2==0 " );

TGraphAsymmErrors * effi_vcell_1_1_2 = new TGraphAsymmErrors ((TH1*) hmatch_vcell_1_1_2,(TH1*) htotal_vcell_1_1_2, "w" ); 
TGraphAsymmErrors * ROIeffi_vcell_1_1_2 = new TGraphAsymmErrors ((TH1*) hmatch_roi_vcell_1_1_2,(TH1*) htotal_roi_vcell_1_1_2, "w" ); 

effi_vcell_1_1_2->SetTitle("");
effi_vcell_1_1_2->GetXaxis()->SetTitle("vCell Sensor 1.1.2");
effi_vcell_1_1_2->GetYaxis()->SetTitle("efficiency");
//effi_vcell_1_1_2->GetXaxis()->SetRangeUser(0,64);
//effi_vcell_1_1_2->GetYaxis()->SetRangeUser(0.98,1.0);
effi_vcell_1_1_2->GetXaxis()->SetTitleSize(0.06);
effi_vcell_1_1_2->GetYaxis()->SetTitleSize(0.06);
effi_vcell_1_1_2->GetXaxis()->SetLabelSize(0.06);
effi_vcell_1_1_2->GetYaxis()->SetLabelSize(0.06);
effi_vcell_1_1_2->GetYaxis()->SetTitleOffset(1.7);


effi_vcell_1_1_2->Draw(); 
ROIeffi_vcell_1_1_2->Draw("same"); 


TCanvas * c2  = new TCanvas("c2","c2",600,400);
c2->SetLeftMargin(0.2);
c2->SetRightMargin(0.1);
c2->SetTopMargin(0.1);
c2->SetBottomMargin(0.16);

ttbtrack->Draw(" ucell_fit_1_1_2 >> htotal_ucell_1_1_2(25,0,250)" , "vcell_fit_1_1_2 > 300 && otherpxd_cluster_matched_1_1_2==0" );
ttbtrack->Draw(" ucell_fit_1_1_2 >> hmatch_ucell_1_1_2(25,0,250)" , "vcell_fit_1_1_2 > 300 && cluster_matched_1_1_2==0 && otherpxd_cluster_matched_1_1_2==0" );

ttbtrack->Draw(" ucell_fit_1_1_2 >> htotal_roi_ucell_1_1_2(25,0,250)" , "vcell_fit_1_1_2 > 300 " );
ttbtrack->Draw(" ucell_fit_1_1_2 >> hmatch_roi_ucell_1_1_2(25,0,250)" , "vcell_fit_1_1_2 > 300 && cluster_matched_1_1_2==0 " );

TGraphAsymmErrors * effi_ucell_1_1_2 = new TGraphAsymmErrors ((TH1*) hmatch_ucell_1_1_2,(TH1*) htotal_ucell_1_1_2, "w" ); 
TGraphAsymmErrors * ROIeffi_ucell_1_1_2 = new TGraphAsymmErrors ((TH1*) hmatch_roi_ucell_1_1_2,(TH1*) htotal_roi_ucell_1_1_2, "w" ); 

effi_ucell_1_1_2->SetTitle("");
effi_ucell_1_1_2->GetXaxis()->SetTitle("uCell Sensor 1.1.2");
effi_ucell_1_1_2->GetYaxis()->SetTitle("efficiency");
//effi_ucell_1_1_2->GetXaxis()->SetRangeUser(0,64);
//effi_ucell_1_1_2->GetYaxis()->SetRangeUser(0.98,1.0);
effi_ucell_1_1_2->GetXaxis()->SetTitleSize(0.06);
effi_ucell_1_1_2->GetYaxis()->SetTitleSize(0.06);
effi_ucell_1_1_2->GetXaxis()->SetLabelSize(0.06);
effi_ucell_1_1_2->GetYaxis()->SetLabelSize(0.06);
effi_ucell_1_1_2->GetYaxis()->SetTitleOffset(1.7);


effi_ucell_1_1_2->Draw(); 
ROIeffi_ucell_1_1_2->Draw("same"); 

TCanvas * c3  = new TCanvas("c3","c3",600,400);
c3->SetLeftMargin(0.2);
c3->SetRightMargin(0.1);
c3->SetTopMargin(0.1);
c3->SetBottomMargin(0.16);


ttbtrack->Draw(" vcell_fit_2_1_2 >> htotal_vcell_2_1_2(100,0,768)" , "ucell_fit_2_1_2 < 150 && otherpxd_cluster_matched_2_1_2==0" );
ttbtrack->Draw(" vcell_fit_2_1_2 >> hmatch_vcell_2_1_2(100,0,768)" , "ucell_fit_2_1_2 < 150 && cluster_matched_2_1_2==0 && otherpxd_cluster_matched_2_1_2==0" );

ttbtrack->Draw(" vcell_fit_2_1_2 >> htotal_roi_vcell_2_1_2(100,0,768)" , "ucell_fit_2_1_2 < 150 " );
ttbtrack->Draw(" vcell_fit_2_1_2 >> hmatch_roi_vcell_2_1_2(100,0,768)" , "ucell_fit_2_1_2 < 150 && cluster_matched_2_1_2==0 " );

TGraphAsymmErrors * effi_vcell_2_1_2 = new TGraphAsymmErrors ((TH1*) hmatch_vcell_2_1_2,(TH1*) htotal_vcell_2_1_2, "w" ); 
TGraphAsymmErrors * ROIeffi_vcell_2_1_2 = new TGraphAsymmErrors ((TH1*) hmatch_roi_vcell_2_1_2,(TH1*) htotal_roi_vcell_2_1_2, "w" ); 

effi_vcell_2_1_2->SetTitle("");
effi_vcell_2_1_2->GetXaxis()->SetTitle("vCell Sensor 2.1.2");
effi_vcell_2_1_2->GetYaxis()->SetTitle("efficiency");
//effi_vcell_2_1_2->GetXaxis()->SetRangeUser(0,64);
//effi_vcell_2_1_2->GetYaxis()->SetRangeUser(0.98,1.0);
effi_vcell_2_1_2->GetXaxis()->SetTitleSize(0.06);
effi_vcell_2_1_2->GetYaxis()->SetTitleSize(0.06);
effi_vcell_2_1_2->GetXaxis()->SetLabelSize(0.06);
effi_vcell_2_1_2->GetYaxis()->SetLabelSize(0.06);
effi_vcell_2_1_2->GetYaxis()->SetTitleOffset(1.7);


effi_vcell_2_1_2->Draw(); 
ROIeffi_vcell_2_1_2->Draw("same"); 


TCanvas * c4  = new TCanvas("c4","c4",600,400);
c4->SetLeftMargin(0.2);
c4->SetRightMargin(0.1);
c4->SetTopMargin(0.1);
c4->SetBottomMargin(0.16);

ttbtrack->Draw(" ucell_fit_2_1_2 >> htotal_ucell_2_1_2(25,0,250)" , "vcell_fit_2_1_2 > 300 && otherpxd_cluster_matched_2_1_2==0" );
ttbtrack->Draw(" ucell_fit_2_1_2 >> hmatch_ucell_2_1_2(25,0,250)" , "vcell_fit_2_1_2 > 300 && cluster_matched_2_1_2==0 && otherpxd_cluster_matched_2_1_2==0" );

ttbtrack->Draw(" ucell_fit_2_1_2 >> htotal_roi_ucell_2_1_2(25,0,250)" , "vcell_fit_2_1_2 > 300 " );
ttbtrack->Draw(" ucell_fit_2_1_2 >> hmatch_roi_ucell_2_1_2(25,0,250)" , "vcell_fit_2_1_2 > 300 && cluster_matched_2_1_2==0 " );

TGraphAsymmErrors * effi_ucell_2_1_2 = new TGraphAsymmErrors ((TH1*) hmatch_ucell_2_1_2,(TH1*) htotal_ucell_2_1_2, "w" ); 
TGraphAsymmErrors * ROIeffi_ucell_2_1_2 = new TGraphAsymmErrors ((TH1*) hmatch_roi_ucell_2_1_2,(TH1*) htotal_roi_ucell_2_1_2, "w" ); 

effi_ucell_2_1_2->SetTitle("");
effi_ucell_2_1_2->GetXaxis()->SetTitle("uCell Sensor 2.1.2");
effi_ucell_2_1_2->GetYaxis()->SetTitle("efficiency");
//effi_ucell_2_1_2->GetXaxis()->SetRangeUser(0,64);
//effi_ucell_2_1_2->GetYaxis()->SetRangeUser(0.98,1.0);
effi_ucell_2_1_2->GetXaxis()->SetTitleSize(0.06);
effi_ucell_2_1_2->GetYaxis()->SetTitleSize(0.06);
effi_ucell_2_1_2->GetXaxis()->SetLabelSize(0.06);
effi_ucell_2_1_2->GetYaxis()->SetLabelSize(0.06);
effi_ucell_2_1_2->GetYaxis()->SetTitleOffset(1.7);


effi_ucell_2_1_2->Draw(); 
ROIeffi_ucell_2_1_2->Draw("same"); 


}
