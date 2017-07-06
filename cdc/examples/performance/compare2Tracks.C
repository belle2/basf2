#include "TROOT.h"
#include "TGraphErrors.h"
void fit(TH1D* h1){
  gStyle->SetOptFit(1111);
  double p1 = h1->GetMaximum();
  double p3 = h1->GetRMS();
  cout<<"RMS::"<<p3<<endl;
  Double_t par[6];
  TF1* f1 = new TF1("f1","gaus(0)+gaus(3)",2,2);
  f1->SetLineColor(kRed);
  f1->SetParameters(p1*0.9, 0, p3*0.7, p1*0.3, 0, p3*1.5);
  TF1* g1 =new TF1("g1","gaus",-2,2);g1->SetLineColor(kBlue);
  TF1* g2 =new TF1("g2","gaus",-2,2);g2->SetLineColor(kGreen);
  h1->Fit("f1","Q","",-3*p3,3*p3);
  f1->GetParameters(par);
  g1->SetParameters(&par[0]);
  g2->SetParameters(&par[3]);
  g1->Draw("same");
  g2->Draw("same");

}
void compare2Tracks(){
  //  gStyle->SetTitleOffset(1.5);
  //  gStyle->SetOptStat(0000);
  //  gStyle->SetOptTitleOffse(1.4);
  gStyle->SetPalette(1);
  gROOT->SetBatch(1);
  TGaxis::SetMaxDigits(4);
  double binWidth=45;
  int ndfmin=25;
  double D0min = 10;
  int nbin = floor(180/binWidth);
  string filename = "output/output_*";
  TChain *tree = new TChain("tree");
  const double pi=3.141592653589793;
  tree->Add(filename.c_str());
  if(!tree->GetBranch("Pval1")) return;
  cout<<"Open TChain: "<<filename.c_str()<<endl;
  cout<<" Number of entry"<<tree->GetEntries()<<endl;
  double ndf1, Pval1, Phi01, tanLambda1, D01, Z01;
  double ndf2, Pval2, Phi02, tanLambda2, D02, Z02;
  TVector3* posSeed1=0;
  TVector3* posSeed2=0;
  TVector3* Mom1=0;
  TVector3* Mom2=0;
  double Omega1, Omega2;


  double binwidth_phi0 = 45;
  tree->SetBranchAddress("ndf1",&ndf1);
  tree->SetBranchAddress("Pval1",&Pval1);
  tree->SetBranchAddress("Phi01",&Phi01);
  tree->SetBranchAddress("tanLambda1",&tanLambda1);
  tree->SetBranchAddress("D01",&D01);
  tree->SetBranchAddress("Z01",&Z01);
  tree->SetBranchAddress("posSeed1",&posSeed1);
  tree->SetBranchAddress("Omega1",&Omega1);
  tree->SetBranchAddress("Mom1",&Mom1);

  tree->SetBranchAddress("ndf2",&ndf2);
  tree->SetBranchAddress("Pval2",&Pval2);
  tree->SetBranchAddress("Phi02",&Phi02);
  tree->SetBranchAddress("tanLambda2",&tanLambda2);
  tree->SetBranchAddress("D02",&D02);
  tree->SetBranchAddress("Z02",&Z02);
  tree->SetBranchAddress("posSeed2",&posSeed2);
  tree->SetBranchAddress("Omega2",&Omega2);
  tree->SetBranchAddress("Mom2",&Mom2);
  TH1D* hNDF1 = new TH1D("hNDF1","Degree of Freedom;ndf;#tracks",100,0,100);
  TH1D* hNDF2 = new TH1D("hNDF2","Degree of Freedom;ndf;#tracks",100,0,100);
  TH1D* hPval1 = new TH1D("hPval1","Prob(#chi^{2}); Pval;#tracks",600,-0.1,1.1);
  TH1D* hPval2 = new TH1D("hPval2","Fit Probability; Pval;#tracks",600,-0.1,1.1);
  TH1D* hPhi01 = new TH1D("hPhi01","#varphi_{0};#varphi_{0}; #Tracks",360,-180,180);
  TH1D* hPhi02 = new TH1D("hPhi02","#varphi_{0};#varphi_{0}; #Tracks",360,-180,180);
  TH1D* htanLambda1 = new TH1D("htanLambda1","tan#lambda;tan#lambda;#tracks",100,-5,5);
  TH1D* htanLambda2 = new TH1D("htanLambda2","tan#lambda;tan#lambda;#tracks",100,-5,5);
  TH1D* hD01 = new TH1D("hD01","d_{0}",200,-110,110);
  TH1D* hD02 = new TH1D("hD02","d_{0}",200,-110,110);
  TH1D* hZ01 = new TH1D("hZ01","z_{0}",200,-80,150);
  TH1D* hZ02 = new TH1D("hZ02","z_{0}",200,-80,150);

  TH1D* hOmega1 = new TH1D("hOmega1","Omega;Omega;#Tracks",200,-0.05,0.05);
  TH1D* hOmega2 = new TH1D("hOmega2","Omega;Omega;#Tracks",200,-0.05,0.05);

  TH1D* hPt1 = new TH1D("hPt1","P_{t};P_{t};#Tracks",200,0,15);
  TH1D* hPt2 = new TH1D("hPt2","P_{t};P_{t};#Tracks",200,0,15);

  /**Different of track param Histogram*/
  TH1D* hdPt = new TH1D("hdPt","#DeltaP_{t};DeltaP_{t};#Tracks",200,-0.15,0.15);
  TH1D* hdOmega = new TH1D("hdOmega","#DeltaOmega;#DeltaOmega;#Tracks",200,-0.0003,0.0003);


  TH1D* hdD0 = new TH1D("hdD0","#Deltad_{0};[cm];#Tracks",75,-0.15,0.15);
  TH1D* hdZ0 = new TH1D("hdZ0","#Deltaz_{0}; [cm];#Tracks",75,-1.5,1.5);
  TH1D* hdPhi0 = new TH1D("hdPhi0","#Delta#varphi;#varphi_{0}[Degree]",100,-1,1);
  TH1D* hdtanL = new TH1D("hdtanL","#Deltatan#lambda",100,-0.05,0.05);
  //Tanlambda dependence
  TH2D* hdD0TanPhi0[nbin];
  TH2D* hdZ0TanPhi0[nbin];
  TH2D*hdD0TanL1 = new TH2D("hdD0TanL1","#Deltad_{0} vs tan#lambda; tan#lambda;#Deltad_{0} [cm]",100,-1,1,100,-0.15,0.15);
  TH2D*hdZ0TanL1 = new TH2D("hdZ0TanL1","#Deltaz_{0} vs tan#lambda; tan#lambda;#Deltaz_{0} [cm]",100,-1,1,100,-1.5,1.5);
  for(int i =0;i<nbin;++i){
    hdD0TanPhi0[i] = new TH2D(Form("hdD0TanPhi0_%d",i),"#Deltad_{0} vs tan#lambda; tan#lambda;#Deltad_{0} [cm]",100,-1,1,100,-0.15,0.15);
    hdZ0TanPhi0[i] = new TH2D(Form("hdZ0TanPhi0_%d",i),"#Deltaz_{0} vs tan#lambda; tan#lambda;#Deltaz_{0} [cm]",100,-1,1,100,-1.5,1.5);
  }
  TH2D*hdtanLTanL1 = new TH2D("hdtanLTanL1","#Deltatan#lambda vs Tan#lambda1; tan#lambda1;#Deltatan#lambda",100,-1,1,100,-0.05,0.05);
  TH2D*hdPhi0TanL1 = new TH2D("hdPhi0TanL1","#Delta#phi_{0} vs tan#lambda1; tan#lambda1;#Delta #phi_{0}",100,-1,1,100,-1,1);
  //Z0 dependence
  TH2D*hdD0Z0 = new TH2D("hdD0Z0","#Deltad_{0} vs z_{0}; z_{0};#Deltad_{0};",100,-80,150,100,-0.15,0.15);
  TH2D*hdZ0Z0 = new TH2D("hdZ0Z0","#Deltaz_{0} vs z_{0}; z_{0};#Deltaz_{0};",100,-80,150,100,-1.5,1.5);
  // D0 dependence
  TH2D*hdD0D0 = new TH2D("hdD0D0","#Deltad_{0} vs d_{0}; d_{0};#Deltad_{0};",50,-50,50,100,-0.15,0.15);
  TH2D*hdZ0D0 = new TH2D("hdZ0D0","#Deltaz_{0} vs d_{0}; d_{0};#Deltaz_{0};",50,-50,50,100,-1.5,1.5);
  // Phi0 dependence
 
  TH2D*hdD0Phi0 = new TH2D("hdD0Phi0","#Deltad_{0} vs #varphi_{0}; #varphi_{0};#Deltad_{0};",90,-180,0,100,-0.15,0.15);
  TH2D*hdZ0Phi0 = new TH2D("hdZ0Phi0","#Deltaz_{0} vs #varphi_{0}; #varphi_{0};#Deltaz_{0};",90,-180,0,100,-1.5,1.5);
  // Pt dependence
  TH2D* hdPtPt = new TH2D("hdPtPt","#DeltaP_{t} vs P_{t};P_{t};#DeltaP_{t}",10,0,5,100,-0.1,0.1);
  TH2D* hdD0Pt = new TH2D("hdD0Pt","#Deltad_{0} vs P_{t};P_{t};#Deltad_{0}",10,0,5,100,-0.15,0.15);
  TH2D* hdZ0Pt = new TH2D("hdZ0Pt","#Deltaz_{0} vs P_{t};P_{t};#Deltaz_{0}",10,0,5,100,-1.5,1.5);
  TH2D* hdPhi0Pt = new TH2D("hdPhi0Pt","#Delta#Phi0_{0} vs P_{t};P_{t};#Delta#Phi0_{0} (rad)",10,0,5,100,-0.01,0.01);
  TH2D* hdtanLPt = new TH2D("hdtanLPt","#Deltatan#lambda  vs P_{t};P_{t};#Deltatan#lambda",10,0,5,100,-0.05,0.05);

  double ndf, Phi0, tanLambda, D0,Z0, Pval;
  hNDF1->SetLineColor(kRed);
  hPval1->SetLineColor(kRed);
  hPhi01->SetLineColor(kRed);
  htanLambda1->SetLineColor(kRed);
  hD01->SetLineColor(kRed);
  hZ01->SetLineColor(kRed);
  hOmega1->SetLineColor(kRed);
  hPt1->SetLineColor(kRed);

  hdD0TanL1->GetYaxis()->SetTitleOffset(1.4);
  hdZ0TanL1->GetYaxis()->SetTitleOffset(1.4);
  hdD0TanL1->GetYaxis()->SetTitleSize(0.05);
  hdZ0TanL1->GetYaxis()->SetTitleSize(0.05);
  hdD0TanL1->GetYaxis()->SetLabelSize(0.05);
  hdZ0TanL1->GetYaxis()->SetLabelSize(0.05);
  Long64_t nbytes=0;
  for(int i=0; i< tree->GetEntries();++i){
    nbytes += tree->GetEntry(i);    
    //    if(ndf1<25 ||ndf2<25) continue;
    //    if(posSeed1->Y() * posSeed2->Y() > 0) continue;
    if(posSeed1->Y() <  posSeed2->Y() ){
      Phi0 = Phi01; Phi01 =Phi02; Phi02 =Phi0;
      tanLambda = tanLambda1; tanLambda1 =tanLambda2; tanLambda2 =tanLambda;
      Z0 = Z01; Z01 =Z02; Z02 =Z0;
      D0 = D01; D01 =D02; D02 =D0;
      ndf = ndf1; ndf1 =ndf2; ndf2 =ndf;
      Pval = Pval1; Pval1 =Pval2; Pval2 =Pval;
    }
    //    if(fabs(D02)>D0min||fabs(D01)>D0min) continue;

    Phi01 *=180/pi;
    Phi02 *=180/pi;
    /** convert signg of D0*/
    /*    double a = fabs(D01)*D02/fabs(D02);
    D01=a;
    double b = fabs(tanLambda1)*tanLambda2/fabs(tanLambda2);
    tanLambda1=b;
    */
    //    if(D01>2.5  || D02>2.5) continue;
    //if(abs(Z01)>10  || abs(Z02)>10  ) continue;
    //    if(abs(abs(Z01)-abs(Z02))>0.5) continue;


    hNDF1->Fill(ndf1);   hPval1->Fill(Pval1); hPhi01->Fill(Phi01);  hD01->Fill(D01); hZ01->Fill(Z01); htanLambda1->Fill(tanLambda1); hOmega1->Fill(Omega1); hPt1->Fill(Mom1->Perp());
    hNDF2->Fill(ndf2);   hPval2->Fill(Pval2); hPhi02->Fill(Phi02);  hD02->Fill(D02); hZ02->Fill(Z02); htanLambda2->Fill(tanLambda2); hOmega2->Fill(Omega2); hPt2->Fill(Mom2->Perp());


    if(ndf1<ndfmin||ndf2<ndfmin) continue;
    //    if(Mom1->Perp()<1 || Mom2->Perp()<1) continue;
    double dD0=D01-D02;
    double dZ0 = Z01-Z02;
    double dPhi0=Phi01-Phi02;
    double dtanLambda = tanLambda1-tanLambda2;
    double dOmega = Omega1 - Omega2;
    double dPt = Mom1->Perp() - Mom2->Perp();
    double D0m = (D01+D02)/2;
    double Z0m = (Z01+Z02)/2;
    double Phi0m = (Phi01+Phi02)/2;
    double tanLm = (tanLambda1+tanLambda2)/2;
    //    if(fabs(dPhi0)>0.15) continue;
    //if(fabs(dtanLambda)>0.01) continue;
    //    if(fabs(Z0)>10) continue;
    //if(fabs(D0)>3) continue;

    hdD0->Fill(dD0);
    hdZ0->Fill(dZ0);
    hdPhi0->Fill(dPhi0);
    hdtanL->Fill(dtanLambda);
    hdPt->Fill(dPt);
    hdOmega->Fill(dOmega);
    double Pt = (Mom1->Perp() + Mom2->Perp());
    double sigma = sqrt(2)*dPt/Pt;
    //tanL dependece
    hdPtPt->Fill(Pt/2,sigma);
    hdD0Pt->Fill(Pt/2,dD0);
    hdZ0Pt->Fill(Pt/2,dZ0);
    hdPhi0Pt->Fill(Pt/2,dPhi0);
    hdtanLPt->Fill(Pt/2,dtanLambda);

    hdtanLTanL1->Fill(tanLm,dtanLambda);
    hdPhi0TanL1->Fill(tanLm,dPhi0);
    hdD0TanL1->Fill(tanLm, dD0);
    hdZ0TanL1->Fill(tanLm, dZ0);
    int index = floor(fabs(Phi01)/binWidth);
    if(index>=0 && index<nbin){
      hdD0TanPhi0[index]->Fill(tanLm, dD0);
      hdZ0TanPhi0[index]->Fill(tanLm, dZ0);
    }


    //    if(fabs(tanLambda1)<0.35 & fabs(tanLambda2)<0.35){
      hdD0Z0->Fill(Z0m, dD0);
      hdZ0Z0->Fill(Z0m, dZ0);    
      //    }
    hdD0D0->Fill(D0m, dD0);
    hdZ0D0->Fill(D0m, dZ0);
    
    hdD0Phi0->Fill(Phi0m,dD0);
    hdZ0Phi0->Fill(Phi0m,dZ0);
  }

  //Draw.
  // First is track fit quality
  TCanvas* c1 = new TCanvas("c1","",1200,800);
  c1->Divide(2,1);

  c1->cd(1); hNDF1->Draw(); hNDF2->Draw("same");
  c1->cd(2); hPval1->Draw(); hPval2->Draw("same");
  c1->Print("TwoTrack_real.pdf(","Title:Params");
  //Second is track parameters

  c1 = new TCanvas("c1","",1200,800);
  c1->Divide(3,2);
  c1->cd(1); hPhi01->Draw(); hPhi02->Draw("same");
  c1->cd(2); hOmega1->Draw();hOmega2->Draw("same");
  c1->cd(3); hPt1->Draw();hPt2->Draw("same");

  c1->cd(4); htanLambda1->Draw(); htanLambda2->Draw("same");
  c1->cd(5); hD01->Draw(); hD02->Draw("same");
  c1->cd(6); hZ01->Draw(); hZ02->Draw("same");
  c1->Print("TwoTrack_real.pdf","Title:Params");

  // Draw difference of track params
  TCanvas* c2 = new TCanvas("c2","",1200,800);
  c2->Divide(3,2);
  c2->cd(1);hdD0->Draw();fit(hdD0);
  c2->cd(2);hdZ0->Draw();fit(hdZ0);
  c2->cd(3);hdPhi0->Draw();fit(hdPhi0);
  c2->cd(4);hdtanL->Draw();fit(hdtanL);
  c2->cd(5);hdOmega->Draw();fit(hdOmega);
  c2->cd(6);hdPt->Draw();fit(hdPt);
  c2->Print("TwoTrack_real.pdf","Title:diff");

  //  c3->cd(3);hdD0TanL2->Draw("colz");
  // c3->cd(4);hdZ0TanL2->Draw("colz");

  //TanL dependece of D0, Z0
  TCanvas* c3 = new TCanvas("c3","",1200,800);
  c3->Divide(3,2);
  c3->cd(1);
  hdD0TanL1->GetXaxis()->SetNdivisions(8,9);
  hdD0TanL1->Draw("colz");
  hdD0TanL1->FitSlicesY(0,0,-1,20);
  TH1D* m_D0TanL1 = (TH1D*)gDirectory->Get("hdD0TanL1_1");
  TH1D* s_D0TanL1 = (TH1D*)gDirectory->Get("hdD0TanL1_2");
  m_D0TanL1->SetMinimum(-0.15);
  m_D0TanL1->SetMaximum(0.15);
  m_D0TanL1->GetYaxis()->SetTitleOffset(1.4);
  s_D0TanL1->SetMinimum(0.);
  s_D0TanL1->SetMaximum(0.05);

  hdZ0TanL1->FitSlicesY(0,0,-1,20);
  TH1D* m_Z0TanL1 = (TH1D*)gDirectory->Get("hdZ0TanL1_1");
  TH1D* s_Z0TanL1 = (TH1D*)gDirectory->Get("hdZ0TanL1_2");
  m_Z0TanL1->SetMinimum(-1.5);
  m_Z0TanL1->SetMaximum(1.5);
  m_Z0TanL1->GetYaxis()->SetTitleOffset(1.4);
  s_Z0TanL1->SetMinimum(0.);
  s_Z0TanL1->SetMaximum(0.5);

  if(m_D0TanL1 && m_D0TanL1->GetEntries()>10)
    {  c3->cd(2);
      TPad* pad2 = (TPad*)c3->GetPrimitive("c3_2");
      pad2->cd();pad2->SetGrid();
      m_D0TanL1->SetYTitle("#Deltad_{0} [cm]");
      m_D0TanL1->Draw();cout<<"HEHEHEHE"<<endl;
      //      m_D0TanL1->Fit("pol1");
      //      m_D0TanL1->GetFunction("pol1")->SetLineColor(kRed);
      m_D0TanL1->GetXaxis()->SetNdivisions(8,9);
      c3->cd(3);
      s_D0TanL1->SetYTitle("sigma of dD0 [cm]");
      s_D0TanL1->Draw();
    }

  c3->cd(4);hdZ0TanL1->Draw("colz");
  hdZ0TanL1->GetXaxis()->SetNdivisions(8,9);
  if(m_Z0TanL1  && m_Z0TanL1->GetEntries()>10){
    c3->cd(5);
    TPad* pad5 = (TPad*)c3->GetPrimitive("c3_5");
    pad5->cd();pad5->SetGrid();

    m_Z0TanL1->SetYTitle("#Deltaz_{0} [cm]");
    m_Z0TanL1->Draw();
    //    m_Z0TanL1->Fit("pol1");
    //    m_Z0TanL1->GetFunction("pol1")->SetLineColor(kRed);
    m_Z0TanL1->GetXaxis()->SetNdivisions(8,9);
    c3->cd(6);
    s_Z0TanL1->SetYTitle("Sigma of dZ0 [cm]");
    s_Z0TanL1->Draw();
  }
  c3->Print("TwoTrack_real.pdf","Title:diff_TanL");

  //Phi Depdendence
  c3->cd(1);hdD0Phi0->Draw("colz");
  hdD0Phi0->FitSlicesY(0,0,-1,20);
  TH1D* m_hdD0Phi0 = (TH1D*)gDirectory->Get("hdD0Phi0_1");
  TH1D* s_hdD0Phi0 = (TH1D*)gDirectory->Get("hdD0Phi0_2");
  if(m_hdD0Phi0){
    m_hdD0Phi0->SetMinimum(-0.15);
    m_hdD0Phi0->SetMaximum(0.15);
    s_hdD0Phi0->SetMinimum(0);
    s_hdD0Phi0->SetMaximum(0.05);
    c3->cd(2);m_hdD0Phi0->Draw();
    c3->cd(3);s_hdD0Phi0->Draw();
  }
  c3->cd(4);hdZ0Phi0->Draw("colz");
  hdZ0Phi0->FitSlicesY(0,0,-1,20);
  TH1D* m_hdZ0Phi0 = (TH1D*)gDirectory->Get("hdZ0Phi0_1");
  TH1D* s_hdZ0Phi0 = (TH1D*)gDirectory->Get("hdZ0Phi0_2");
  if(m_hdZ0Phi0){
    m_hdZ0Phi0->SetMinimum(-1.5);
    m_hdZ0Phi0->SetMaximum(1.5);
    s_hdZ0Phi0->SetMinimum(0);
    s_hdZ0Phi0->SetMaximum(0.5);
    c3->cd(5);m_hdZ0Phi0->Draw();
    c3->cd(6);s_hdZ0Phi0->Draw();
  }
  c3->Print("TwoTrack_real.pdf","Title:diff_Phi0");
  
  /***D0 dependence***/
  c3->cd(1);hdD0D0->Draw("colz");
  hdD0D0->FitSlicesY(0,0,-1,20);
  TH1D* m_hdD0D0 = (TH1D*)gDirectory->Get("hdD0D0_1");
  TH1D* s_hdD0D0 = (TH1D*)gDirectory->Get("hdD0D0_2");
  if(m_hdD0D0){
    m_hdD0D0->SetMinimum(-0.15);
    m_hdD0D0->SetMaximum(0.15);
    s_hdD0D0->SetMinimum(0);
    s_hdD0D0->SetMaximum(0.05);
    c3->cd(2);m_hdD0D0->Draw();
    c3->cd(3);s_hdD0D0->Draw();
  }
  c3->cd(4);hdZ0D0->Draw("colz");
  hdZ0D0->FitSlicesY(0,0,-1,20);
  TH1D* m_hdZ0D0 = (TH1D*)gDirectory->Get("hdZ0D0_1");
  TH1D* s_hdZ0D0 = (TH1D*)gDirectory->Get("hdZ0D0_2");
  if(s_hdZ0D0){
    m_hdZ0D0->SetMinimum(-1.5);
    m_hdZ0D0->SetMaximum(1.5);
    s_hdZ0D0->SetMinimum(0);
    s_hdZ0D0->SetMaximum(0.5);
    c3->cd(5);m_hdZ0D0->Draw();
    c3->cd(6);s_hdZ0D0->Draw();
  }
  c3->Print("TwoTrack_real.pdf","Title:diff2_D0");

  c3->cd(1); hdD0Z0->Draw("colz");
  hdD0Z0->FitSlicesY(0,0,-1,20);
  TH1D* m_hdD0Z0 = (TH1D*)gDirectory->Get("hdD0Z0_1");
  TH1D* s_hdD0Z0 = (TH1D*)gDirectory->Get("hdD0Z0_2");
  if(m_hdD0Z0){
    c3->cd(2); 
    m_hdD0Z0->SetMinimum(-0.15);    m_hdD0Z0->SetMaximum(0.15);
    s_hdD0Z0->SetMinimum(0);    s_hdD0Z0->SetMaximum(0.05);
    m_hdD0Z0->Draw();
    c3->cd(3); s_hdD0Z0->Draw();
  }

  c3->cd(4); hdZ0Z0->Draw("colz");
  hdZ0Z0->FitSlicesY(0,0,-1,20);
  TH1D* m_hdZ0Z0 = (TH1D*)gDirectory->Get("hdZ0Z0_1");
  TH1D* s_hdZ0Z0 = (TH1D*)gDirectory->Get("hdZ0Z0_2");
  if(m_hdZ0Z0){
    c3->cd(5); 
    m_hdZ0Z0->SetMinimum(-1.5);    m_hdZ0Z0->SetMaximum(1.5);
    s_hdZ0Z0->SetMinimum(0);    s_hdZ0Z0->SetMaximum(0.5);
    m_hdZ0Z0->Draw();
    c3->cd(6); s_hdZ0Z0->Draw();
  }
  c3->Print("TwoTrack_real.pdf","Title:diff2_Z0");

  /*********dPt of Pt****************/
  c3 = new TCanvas("c3","",1200,800);
  c3->Divide(3,2);
  c3->cd(1);
  hdPtPt->GetXaxis()->SetNdivisions(8,9);
  hdPtPt->Draw("colz");
  hdPtPt->FitSlicesY(0,0,-1,20);
  TH1D* m_dPtPt = (TH1D*)gDirectory->Get("hdPtPt_1");
  TH1D* s_dPtPt = (TH1D*)gDirectory->Get("hdPtPt_2");
  TH1D* hPtReso = (TH1D*)s_dPtPt->Clone("hPtReso");
  hPtReso->SetTitle("Pt resolution;Pt (GeV);#sigmaPt/#surd2/Pt");
  hPtReso->GetYaxis()->SetTitleOffset(1.6);
  for(int i=0; i<s_dPtPt->GetNbinsX();++i){
    double reso = s_dPtPt->GetBinContent(i)/std::sqrt(2)/s_dPtPt->GetBinCenter(i);
    double dreso = s_dPtPt->GetBinError(i)/std::sqrt(2)/s_dPtPt->GetBinCenter(i);
    hPtReso->SetBinContent(i,reso);
    hPtReso->SetBinError(i,dreso);

  }
  m_dPtPt->SetMinimum(-0.15);
  m_dPtPt->SetMaximum(0.15);
  m_dPtPt->GetYaxis()->SetTitleOffset(1.4);
  s_dPtPt->SetMinimum(0.);
  s_dPtPt->SetMaximum(0.02);

  hPtReso->SetMinimum(0.);
  hPtReso->SetMaximum(0.02);

  if(m_dPtPt && m_dPtPt->GetEntries()>3) 
    {  c3->cd(2);
      TPad* pad2 = (TPad*)c3->GetPrimitive("c3_2");
      pad2->cd();pad2->SetGrid();
      m_dPtPt->SetTitle("Pt;P_{t} (GeV); #DeltaP_{t} (cm)");
      m_dPtPt->Draw();cout<<"HEHEHEHE"<<endl;
      //      m_dPtPt->Fit("pol1");
      //      m_dPtPt->GetFunction("pol1")->SetLineColor(kRed);
      m_dPtPt->GetXaxis()->SetNdivisions(8,9);
      c3->cd(3);
      s_dPtPt->SetYTitle("#sigmaPt;P_{t} (GeV);#sigmaP_{t}");
      s_dPtPt->Draw();
      c3->cd(4);hPtReso->Draw();
    }
  c3->Print("TwoTrack_real.pdf","Title:diff_TanL");



  ///Pt dependence
  hdD0Pt->FitSlicesY(0,0,-1,20);
  TH1D* hdD0Pt_m =  (TH1D*)gDirectory->Get("hdD0Pt_1");
  TH1D* hdD0Pt_s =  (TH1D*)gDirectory->Get("hdD0Pt_2");
  hdZ0Pt->FitSlicesY(0,0,-1,20);
  TH1D* hdZ0Pt_m =  (TH1D*)gDirectory->Get("hdZ0Pt_1");
  TH1D* hdZ0Pt_s =  (TH1D*)gDirectory->Get("hdZ0Pt_2");
  hdPhi0Pt->FitSlicesY(0,0,-1,20);
  TH1D* hdPhi0Pt_m =  (TH1D*)gDirectory->Get("hdPhi0Pt_1");
  TH1D* hdPhi0Pt_s =  (TH1D*)gDirectory->Get("hdPhi0Pt_2");
  hdtanLPt->FitSlicesY(0,0,-1,20);
  TH1D* hdtanLPt_m =  (TH1D*)gDirectory->Get("hdtanLPt_1");
  TH1D* hdtanLPt_s =  (TH1D*)gDirectory->Get("hdtanLPt_2");

  c3 = new TCanvas("c3","",1200,800);
  c3->Divide(3,2);
  c3->cd(1); hdD0Pt->Draw("colz");
  c3->cd(2); hdD0Pt_m->Draw(); 
  c3->cd(3); hdD0Pt_s->Draw();

  c3->cd(4); hdZ0Pt->Draw("colz");
  c3->cd(5); hdZ0Pt_m->Draw(); 
  c3->cd(6); hdZ0Pt_s->Draw();
  c3->Print("TwoTrack_real.pdf","Title:diff_TanL");


  c3 = new TCanvas("c3","",1200,800);
  c3->Divide(3,2);
  c3->cd(1); hdPhi0Pt->Draw("colz");
  c3->cd(2); hdPhi0Pt_m->Draw(); 
  c3->cd(3); hdPhi0Pt_s->Draw();

  c3->cd(4); hdtanLPt->Draw("colz");
  c3->cd(5); hdtanLPt_m->Draw(); 
  c3->cd(6); hdtanLPt_s->Draw();
  c3->Print("TwoTrack_real.pdf)","Title:diff_TanL");

  /************* tan lambda different for different Phi0*/
  /*  delete c3;
  c3 = new TCanvas("c3","",1200,800);
  c3->Divide(nbin/2+1,2);
  vector<double> slope_D0;
  vector<double> slope_Z0;
  vector<double> err_slope_D0;
  vector<double> err_slope_Z0;
  vector<double> phi0_D0;
  vector<double> phi0_Z0;
  vector<double> err_phi0_D0;
  vector<double> err_phi0_Z0;
 
  TH1D* m_hdD0TanPhi0[nbin];
  TH1D* m_hdZ0TanPhi0[nbin];
 
  for(int i=0;i<nbin;++i){
    hdD0TanPhi0[i]->FitSlicesY(0,0,-1,20);
    m_hdD0TanPhi0[i] = (TH1D*)gDirectory->Get(Form("hdD0TanPhi0_%d_1",i));
    c3->cd(i+1);
    if(m_hdD0TanPhi0[i]){
      m_hdD0TanPhi0[i]->SetMinimum(-0.15);      m_hdD0TanPhi0[i]->SetMaximum(0.15);
      m_hdD0TanPhi0[i]->Draw();
      m_hdD0TanPhi0[i]->Fit("pol1","Q","",-0.8,0.8);
      slope_D0.push_back(m_hdD0TanPhi0[i]->GetFunction("pol1")->GetParameter(1));
      err_slope_D0.push_back(m_hdD0TanPhi0[i]->GetFunction("pol1")->GetParError(1));
      phi0_D0.push_back((i+0.5)*binWidth);
      err_phi0_D0.push_back(0.);
    }
  }
  TGraphErrors* gr1 = new TGraphErrors(phi0_D0.size(),&phi0_D0.at(0), &slope_D0.at(0), &err_phi0_D0.at(0),&err_slope_D0.at(0));
  gr1->SetTitle("slope of #Deltad_{0}-tan#lambda vs #varphi_{0};#varphi [deg];");
  // gr1->SetMaximum(0.01);
  //gr1->SetMinimum(-0.01);
  gr1->SetMarkerStyle(20);
  gr1->SetMarkerColor(kRed);
  gr1->SetLineStyle(2);

  c3->cd(nbin+1);gr1->Draw("APL");
  c3->Print("TwoTrack_real.pdf","Title:dD0_tanL_Phi0");
  delete c3;
  c3 = new TCanvas("c3","",1200,800);
  c3->Divide(nbin/2+1,2);

  for(int i=0;i<nbin;++i){
    hdZ0TanPhi0[i]->FitSlicesY(0,0,-1,20);
    m_hdZ0TanPhi0[i] = (TH1D*)gDirectory->Get(Form("hdZ0TanPhi0_%d_1",i));
    c3->cd(i+1);
    if(m_hdD0TanPhi0[i]){
      m_hdZ0TanPhi0[i]->SetMinimum(-1.5);      m_hdZ0TanPhi0[i]->SetMaximum(1.5);
      m_hdZ0TanPhi0[i]->Draw();
      m_hdZ0TanPhi0[i]->Fit("pol1","Q","",-0.8,0.8);
      slope_Z0.push_back(m_hdZ0TanPhi0[i]->GetFunction("pol1")->GetParameter(1));
      err_slope_Z0.push_back(m_hdZ0TanPhi0[i]->GetFunction("pol1")->GetParError(1));
      phi0_Z0.push_back((i+0.5)*binwidth_phi0);
      err_phi0_Z0.push_back(0.);
    }   
  }
  TGraphErrors* gr2 = new TGraphErrors(phi0_Z0.size(),&phi0_Z0.at(0), &slope_Z0.at(0), &err_phi0_Z0.at(0),&err_slope_Z0.at(0));
  gr2->SetTitle("slope of #Deltaz_{0}-tan#lambda vs #varphi_{0}; #varphi [deg];");
  gr2->SetMarkerStyle(20);
  gr2->SetMarkerColor(kRed);
  gr2->SetLineStyle(2);

  //  gr2->SetMaximum(0.1);
  //gr2->SetMinimum(-0.1);
  c3->cd(nbin+1);gr2->Draw("APL");
  c3->Print("TwoTrack_real.pdf)","Title:dZ0_tanL_Phi0");
  */

  TFile *result = new TFile("result.root","recreate");
  result->cd();
  hdD0->Write();
  hdZ0->Write();
  s_dPtPt->Write();
  result->Close();
}
