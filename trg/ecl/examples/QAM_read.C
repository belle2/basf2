/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
void draw_QAM(TString m_file,TString m_pdf, TString m_title,TString m_Xaxis, TString m_Yaxis, Int_t m_startrun, Int_t m_endrun, Double_t m_min, Double_t m_max);

void QAM_read(TString m_file){

  gStyle->SetOptTitle(1);
  gStyle->SetOptStat(0);
  gStyle->SetPadTopMargin(0.08);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetLabelSize(0.05,"X");
  gStyle->SetLabelSize(0.05,"Y");
  gStyle->SetTitleSize(0.1,"X");
  gStyle->SetTitleSize(0.1,"Y");
  gStyle->SetTitleOffset(1.0,"X");
  gStyle->SetTitleOffset(1.5,"Y");

  //  draw_QAM(m_file,"HighE.pdf","m_psnm8","run#","ratio",0,6000,0,1);
  // draw_QAM(m_file,"clster3.pdf","m_psnm13","run#","ratio",0,6000,0,1);
  // draw_QAM(m_file,"BelleI_Physics.pdf","m_psnm8_13","run#","ratio",0,6000,0,1);
  // draw_QAM(m_file,"eclbhabha_logic1.pdf","m_psnm7_15","run#","ratio",0,6000,0,1);
  // draw_QAM(m_file,"eclbhabha_logic2.pdf","m_psnm8_15","run#","ratio",0,6000,0,1);
  // draw_QAM(m_file,"eclbhabha_logic3.pdf","m_psnm13_15","run#","ratio",0,6000,0,1);
  draw_QAM(m_file,"plots/FWETC_error.pdf","m_FWD","run#","#ofTC",0,2300,0,81);
  draw_QAM(m_file,"plots/BarrelTC_error.pdf","m_BAR","run#","#ofTC",0,2300,0,432);
  draw_QAM(m_file,"plots/BWETC_error.pdf","m_BWD","run#","#ofTC",0,2300,0,65);
  draw_QAM(m_file,"plots/ALLTC_error.pdf","m_ALL","run#","#ofTC",0,2300,0,580);

  draw_QAM(m_file,"plots/Energy_Total.pdf","m_etot_mean","run#","Etot[ADC]",0,2300,800,2200); // Bhabha Peak : ~1900ADC
  draw_QAM(m_file,"plots/Energy_Total_sigma.pdf","m_etot_sigma","run#","sigma",0,2300,50,200);

  draw_QAM(m_file,"plots/Caltime_mean.pdf","m_caltime_mean","run#","Time[ns]",0,2300,-5,+5); // Caltime Peak : ~+-3ns
  draw_QAM(m_file,"plots/Caltime_sigma.pdf","m_caltime_sigma","run#","sigma",0,2300,5,20);

  draw_QAM(m_file,"plots/ClusterE_mean.pdf","m_clusterE_mean","run#","Ec1+Ec2[ADC]",0,2300,1500,2500);  // Bhabha Peak : ~1900ADC
  draw_QAM(m_file,"plots/ClusterE_sigma.pdf","m_clusterE_sigma","run#","sigma",0,2300,50,200); 



}
void draw_QAM(TString m_file,TString m_pdf, TString m_title,TString m_Xaxis, TString m_Yaxis, Int_t m_startrun, Int_t m_endrun, Double_t m_min, Double_t m_max){

  TH2F* hhh = new TH2F("hhh","QAM Histogram", m_endrun-m_startrun, m_startrun, m_endrun, (m_max-m_min)*100, m_min, m_max);
  
  TFile* file = TFile::Open(m_file);
  TTree *tree = (TTree*)file->Get("tree");

  Int_t n1 = tree->GetEntries();
  
  for( int ii = 0 ; ii<n1 ; ii++){

    tree->GetEntry(ii);

    Double_t run = tree->GetLeaf("m_nRun")->GetValue();
    Double_t value = tree->GetLeaf(m_title)->GetValue();
    Double_t nevent = tree->GetLeaf("m_nevent")->GetValue();
    if( nevent < 1000000)continue;

    hhh->Fill(run,value);
  }

  TCanvas *c1 = new TCanvas("c1","c1",300,300);
  TPDF *pdf1 = new TPDF(m_pdf,-111);

  hhh->SetTitle(m_title);
  hhh->SetTitleSize(1);
  hhh->SetLabelSize(0.025,"X");
  hhh->SetLabelSize(0.025,"Y");
  hhh->SetTitleOffset(1.0,"X");
  hhh->SetTitleOffset(1.25,"Y");
  hhh->GetXaxis()->SetTitleSize(0.03);
  hhh->GetYaxis()->SetTitleSize(0.03);

  hhh->GetXaxis()->SetLabelSize(0.04);
  hhh->GetXaxis()->SetNdivisions(5);
  hhh->GetXaxis()->SetTitle(m_Xaxis);
  //  hhh->GetXaxis()->Set(300,0,300);
  hhh->GetYaxis()->SetTitle(m_Yaxis);
  hhh->SetLineColor(2);
  hhh->SetMarkerStyle(7);
  hhh->SetMarkerColor(1);
  hhh->SetMarkerSize(1);
  hhh->Draw("PL");

  pdf1->Close();
  c1->Close();

  file->Close();
}
  

     
	
