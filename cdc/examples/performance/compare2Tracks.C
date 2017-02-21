#include "TVector3.h"
#include "TTree.h"
#include "TChain.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TStyle.h"
#include "TText.h"
#include "TPaveStats.h"

void fit(TH1D* h1){
  double p1 = h1->GetMaximum();
  double p3 = h1->GetRMS();
  cout<<"RMS::"<<p3<<endl;
  Double_t par[6];
  TF1* f1 = new TF1("f1","gaus(0)+gaus(3)",2,2);
  f1->SetLineColor(kRed);
  f1->SetParameters(p1,0,p3*0.75,p1/3.5,0,p3*1.5);
  TF1* g1 =new TF1("g1","gaus",-2,2);g1->SetLineColor(kBlue);
  TF1* g2 =new TF1("g2","gaus",-2,2);g2->SetLineColor(kGreen);
  h1->Fit("f1","MQ","",-4*p3,4*p3);
  f1->GetParameters(par);
  g1->SetParameters(&par[0]);
  g2->SetParameters(&par[3]);
  g1->Draw("same");
  g2->Draw("same");
}

void compare2Tracks(string name = "201607"){
  //  SetBelle2Style();
  gStyle->SetOptFit(1111);
  string filename = name+"/output_*.root";
  const string output = "twotracks"+name+".eps";
  TChain *tree = new TChain("tree");
  tree->Add(filename.c_str());
  cout<<"Open TChain: "<<filename.c_str()<<endl;

  double ndf1, Pval1, Phi01, tanLambda1, D01, Z01;
  double ndf2, Pval2, Phi02, tanLambda2, D02, Z02;
  TVector3* posSeed1=0;
  TVector3*  posSeed2=0;
  tree->SetBranchAddress("ndf1",&ndf1);
  tree->SetBranchAddress("Pval1",&Pval1);
  tree->SetBranchAddress("Phi01",&Phi01);
  tree->SetBranchAddress("tanLambda1",&tanLambda1);
  tree->SetBranchAddress("D01",&D01);
  tree->SetBranchAddress("Z01",&Z01);
  tree->SetBranchAddress("posSeed1",&posSeed1);

  tree->SetBranchAddress("ndf2",&ndf2);
  tree->SetBranchAddress("Pval2",&Pval2);
  tree->SetBranchAddress("Phi02",&Phi02);
  tree->SetBranchAddress("tanLambda2",&tanLambda2);
  tree->SetBranchAddress("D02",&D02);
  tree->SetBranchAddress("Z02",&Z02);
  tree->SetBranchAddress("posSeed2",&posSeed2);
  /** Track parameters histogram*/
  TH1D* hNDF1 = new TH1D("hNDF1","Degree of Freedom;ndf;#tracks",60,0,60);
  TH1D* hNDF2 = new TH1D("hNDF2","Degree of Freedom;ndf;#tracks",60,0,60);

  TH1D* hPval1 = new TH1D("hPval1","Fit Probability; Pval;#tracks",600,-0.1,1.1);
  TH1D* hPval2 = new TH1D("hPval2","Fit Probability; Pval;#tracks",600,-0.1,1.1);

  TH1D* hPhi01 = new TH1D("hPhi01","#Phi0;#Phi0 [rad]; #Tracks",360,-180,180);
  TH1D* hPhi02 = new TH1D("hPhi02","#Phi0;#Phi0 [rad]; #Tracks",360,-180,180);

  TH1D* htanLambda1 = new TH1D("htanLambda1","tan#lambda;tan#lambda;#tracks",100,-5,5);
  TH1D* htanLambda2 = new TH1D("htanLambda2","tan#lambda;tan#lambda;#tracks",100,-5,5);
  TH1D* hD01 = new TH1D("hD01","D0",200,-10,10);
  TH1D* hD02 = new TH1D("hD02","D0",200,-10,10);
  TH1D* hZ01 = new TH1D("hZ01","Z0",200,-40,100);
  TH1D* hZ02 = new TH1D("hZ02","Z0",200,-40,100);

  /**Different of track param Histogram*/
  TH1D* hdD0 = new TH1D("hdD0","D01-D02;#Delta d_{0} [cm]",100,-0.2,0.2);
  TH1D* hdZ0 = new TH1D("hdZ0","Z01-Z02;#Delta z_{0} [cm]",100,-2,2);
  //  TH1D* hdPhi0 = new TH1D("hdPhi0","#Phi01-#Phi02;#Phi_{0} [rad]",100,-0.05,0.05);
  TH1D* hdPhi0 = new TH1D("hdPhi0","#Phi01-#Phi02;#Delta #Phi_{0} [deg]",100,-1,1);
  TH1D* hdtanL = new TH1D("hdtanL","tan#lambda1-tan#lambda2;#Delta tan#lambda",100,-0.05,0.05);

  TH2D*hdD0TanL1 = new TH2D("hdD0TanL1","#DeltaD0 vs Tan#lambda1; tan#lambda1;#DeltaD0",100,-1,1,100,-0.15,0.15);
  TH2D*hdZ0TanL1 = new TH2D("hdZ0TanL1","#DeltaZ0 vs Tan#lambda1; tan#lambda1;#DeltaZ0",100,-1,1,100,-1.5,1.5);
  //  TH2D*hdD0TanL2 = new TH2D("hdD0TanL2","#DeltaD0 vs Tan#lambda2; tan#lambda2;dD0",100,-1,1,100,-0.15,0.15);
  // TH2D*hdZ0TanL2 = new TH2D("hdZ0TanL2","#DeltaZ0 vs Tan#lambda2; tan#lambda2;dZ0",100,-1,1,100,-1.5,1.5);
  TH2D*hdtanLTanL1 = new TH2D("hdtanLTanL1","#Delta tan#lambda vs Tan#lambda1; tan#lambda1;#Delta tan#lambda",100,-1,1,100,-0.1,0.1);
  TH2D*hdPhi0TanL1 = new TH2D("hdPhi0TanL1","#Delta#Phi_{0} vs Tan#lambda1; tan#lambda1;#Delta #Phi_{0} [rad]",100,-1,1,100,-0.5,0.5);
  TH2D*hdD0Z0 = new TH2D("hdD0Z0","#DeltaD0 vs Z0; Z0;#Delta d_0;",200,-40,100,100,-0.2,0.2);
  TH2D*hdZ0Z0 = new TH2D("hdZ0Z0","#DeltaZ0 vs Z0; Z0;#Delta z_0;",200,-40,100,100,-2,2);

  double ndf;
  double Phi0;
  double tanLambda;
  double D0;
  double Z0;
  double Pval;
  hNDF1->SetLineColor(kRed);
  hPval1->SetLineColor(kRed);
  hPhi01->SetLineColor(kRed);
  htanLambda1->SetLineColor(kRed);
  hD01->SetLineColor(kRed);
  hZ01->SetLineColor(kRed);
  Long64_t nbytes = 0;
  for(Long64_t i=0; i< tree->GetEntries();++i){
    nbytes += tree->GetEntry(i);    
    if(posSeed1->Y() * posSeed2->Y() > 0) continue;
    if(posSeed1->Y() < 0){
      Phi0 = Phi01; Phi01 =Phi02; Phi02 =Phi0;
      tanLambda = tanLambda1; tanLambda1 =tanLambda2; tanLambda2 =tanLambda;
      Z0 = Z01; Z01 =Z02; Z02 =Z0;
      D0 = D01; D01 =D02; D02 =D0;
      ndf = ndf1; ndf1 =ndf2; ndf2 =ndf;
      Pval = Pval1; Pval1 =Pval2; Pval2 =Pval;
    }
    Phi01 *=180/M_PI;
    Phi02 *=180/M_PI;
    //    if(abs(Z01)>5  || abs(Z02)>5  ) continue;
    //    if(abs(D01)>2.5  || abs(D02)>2.5  ) continue;
    //    if(abs(tanLambda1)>0.1  || abs(tanLambda2)>0.1  ) continue;
    hNDF1->Fill(ndf1);   hPval1->Fill(Pval1); hPhi01->Fill(Phi01);  hD01->Fill(D01); hZ01->Fill(Z01); htanLambda1->Fill(tanLambda1);
    hNDF2->Fill(ndf2);   hPval2->Fill(Pval2); hPhi02->Fill(Phi02);  hD02->Fill(D02); hZ02->Fill(Z02); htanLambda2->Fill(tanLambda2);

    //    if(abs(D01)>1  || abs(D02)>1  ) continue;
    //    if(abs(abs(Z01)-abs(Z02))>0.5) continue;
    if(ndf1<25||ndf2<25) continue;

    double dD0=D01-D02;
    double dZ0 = Z01-Z02;
    double dPhi0=Phi01-Phi02;
    double dtanLambda = tanLambda1-tanLambda2;
    if(fabs(dPhi0)>0.15) continue;
    if(fabs(dtanLambda)>0.01) continue;

    hdD0->Fill(dD0);
    hdZ0->Fill(dZ0);
    hdPhi0->Fill(dPhi0);
    hdtanL->Fill(dtanLambda);
    hdtanLTanL1->Fill(tanLambda1,dtanLambda);
    hdPhi0TanL1->Fill(tanLambda1,dPhi0);
    hdD0TanL1->Fill(tanLambda1, dD0);
    hdZ0TanL1->Fill(tanLambda1,Z01-Z02);
    if(fabs(tanLambda1) <0.5){
      hdD0Z0->Fill(Z01,dD0);
      hdZ0Z0->Fill(Z01,dZ0);}
    //hdD0TanL2->Fill(tanLambda2, dD0);
    //hdZ0TanL2->Fill(tanLambda2,Z01-Z02);
  }
  /** First canvas for track params**/
  TCanvas* c1 = new TCanvas("c1","",1200,800);
  c1->Divide(3,2);
  c1->cd(1); hPhi01->Draw(); hPhi02->Draw("same");
  c1->cd(2); hNDF1->Draw(); hNDF2->Draw("same");
  c1->cd(3); hPval1->Draw(); hPval2->Draw("same");

  c1->cd(4); htanLambda1->Draw(); htanLambda2->Draw("same");
  c1->cd(5); hD01->Draw(); hD02->Draw("same");
  c1->cd(6); hZ01->Draw(); hZ02->Draw("same");

  //  c1->Print(outputb.c_str(),"Title:Params");

  /** Second canvas for track params different**/
  TCanvas* c2 = new TCanvas("c2","",900,700);
  c2->Divide(2,2);
  c2->cd(1);
  hdD0->Draw();
  fit(hdD0);
  TText *t = new TText();
  t->DrawTextNDC(0.2,0.85,"(a)");
  t->Draw();
  c2->cd(2);hdZ0->Draw();fit(hdZ0);
  t->DrawTextNDC(0.2,0.85,"(b)");
  t->Draw();

  c2->cd(3);hdPhi0->Draw();fit(hdPhi0);
  t->DrawTextNDC(0.2,0.85,"(c)");
  t->Draw();
  c2->cd(4);hdtanL->Draw();fit(hdtanL);
  t->DrawTextNDC(0.2,0.85,"(d)");
  t->Draw();
  c2->Print(output.c_str());

  //  const string output = "twotracks"+name+".pdf";
  //  c2->Print(output.c_str(),"Title:diff");

  /** third canvas for the dependence of diff with params*/
  TCanvas* c3 = new TCanvas("c3","",900,700);
  c3->Divide(3,2);
  c3->cd(1);hdD0TanL1->Draw("colz");
  hdD0TanL1->FitSlicesY();
  TH1D* m_D0TanL1 = (TH1D*)gDirectory->Get("hdD0TanL1_1");
  TH1D* s_D0TanL1 = (TH1D*)gDirectory->Get("hdD0TanL1_2");
  m_D0TanL1->SetMinimum(-0.15);
  m_D0TanL1->SetMaximum(0.15);
  s_D0TanL1->SetMinimum(0.);
  s_D0TanL1->SetMaximum(0.05);

  hdZ0TanL1->FitSlicesY();
  TH1D* m_Z0TanL1 = (TH1D*)gDirectory->Get("hdZ0TanL1_1");
  TH1D* s_Z0TanL1 = (TH1D*)gDirectory->Get("hdZ0TanL1_2");
  m_Z0TanL1->SetMinimum(-1.5);
  m_Z0TanL1->SetMaximum(1.5);
  s_Z0TanL1->SetMinimum(0.);
  s_Z0TanL1->SetMaximum(0.5);

  if(m_D0TanL1)
    {  c3->cd(2);
      m_D0TanL1->SetYTitle("Mean of dD0 [cm]");
      m_D0TanL1->Draw();cout<<"HEHEHEHE"<<endl;
      m_D0TanL1->Fit("pol1");
      c3->cd(3);
      s_D0TanL1->SetYTitle("sigma of dD0 [cm]");
      s_D0TanL1->Draw();
    }

  c3->cd(4);hdZ0TanL1->Draw("colz");
  if(m_Z0TanL1){
    c3->cd(5);
    m_Z0TanL1->SetYTitle("Mean of dZ0 [cm]");
    m_Z0TanL1->Draw();cout<<"HEHEHEHE"<<endl;
    m_Z0TanL1->Fit("pol1");
    c3->cd(6);
    s_Z0TanL1->SetYTitle("Sigma of dZ0 [cm]");
    s_Z0TanL1->Draw();
  }
  //  c3->Print(output.c_str(),"Title:diff_TanL");
  TCanvas* c4 = new TCanvas("c4","",900,700);
  c4->Divide(2,2);
  c4->cd(1); hdD0Z0->Draw("colz");
  c4->cd(2); hdZ0Z0->Draw("colz");
  c4->cd(3); hdtanLTanL1->Draw("colz");
  c4->cd(4); hdPhi0TanL1->Draw("colz");
  //  c2->Print(outpute.c_str(),"Title:diff2_TanL");
}
