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
  //  double D0min = 5;
  int nbin = floor(180/binWidth);
  string filename = "output/output_*";
  TChain *tree = new TChain("tree");
  const double pi=3.141592653589793;
  tree->Add(filename.c_str());
  if(!tree->GetBranch("Pval1")) return;
  cout<<"Open TChain: "<<filename.c_str()<<endl;
  cout<<" Number of entry"<<tree->GetEntries()<<endl;
  double ndf1, Pval1, Phi01, tanLambda1, D01, Z01;
  double ndf2, Pval2, Phi02, tanLambda2, D02, Z02,evtT0;
  TVector3* posSeed1=0;
  TVector3* posSeed2=0;
  TVector3* Mom1=0;
  TVector3* Mom2=0;
  double Omega1, Omega2;


  //  double binwidth_phi0 = 45;
  tree->SetBranchAddress("evtT0",&evtT0);
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
  TH1D* hPhi01 = new TH1D("hPhi01","#phi_{0};#phi_{0}; #Tracks",360,-180,180);
  TH1D* hPhi02 = new TH1D("hPhi02","#phi_{0};#phi_{0}; #Tracks",360,-180,180);
  TH1D* htanLambda1 = new TH1D("htanLambda1","tan#lambda;tan#lambda;#tracks",100,-5,5);
  TH1D* htanLambda2 = new TH1D("htanLambda2","tan#lambda;tan#lambda;#tracks",100,-5,5);
  TH1D* hD01 = new TH1D("hD01","d_{0}",200,-110,110);
  TH1D* hD02 = new TH1D("hD02","d_{0}",200,-110,110);
  TH1D* hZ01 = new TH1D("hZ01","z_{0}",200,-80,150);
  TH1D* hZ02 = new TH1D("hZ02","z_{0}",200,-80,150);

  TH1D* hOmega1 = new TH1D("hOmega1","Omega;Omega;#Tracks",200,-0.02,0.02);
  TH1D* hOmega2 = new TH1D("hOmega2","Omega;Omega;#Tracks",200,-0.02,0.02);

  TH1D* hPt1 = new TH1D("hPt1","P_{t};P_{t};#Tracks",200,0,15);
  TH1D* hPt2 = new TH1D("hPt2","P_{t};P_{t};#Tracks",200,0,15);

  /**Different of track param Histogram*/
  TH1D* hdPt = new TH1D("hdPt","#DeltaP_{t}/#surd2;DeltaP_{t};#Tracks",200,-0.15,0.15);
  TH1D* hdOmega = new TH1D("hdOmega","#DeltaOmega/#surd2;#DeltaOmega;#Tracks",200,-0.0001,0.0001);


  TH1D* hdD0 = new TH1D("hdD0","#Deltad_{0}/#surd2;[cm];#Tracks",75,-0.15,0.15);
  TH1D* hdZ0 = new TH1D("hdZ0","#Deltaz_{0}/#surd2; [cm];#Tracks",75,-1.5,1.5);
  TH1D* hdPhi0 = new TH1D("hdPhi0","#Delta#phi_{0}/#surd2;#phi_{0}[Degree]",100,-0.5,0.5);
  TH1D* hdtanL = new TH1D("hdtanL","#Deltatan#lambda/#surd2",100,-0.05,0.05);
  //Tanlambda dependence
  TH2D* hdD0TanPhi0[nbin];
  TH2D* hdZ0TanPhi0[nbin];
  TH2D*hdD0TanL = new TH2D("hdD0TanL","#Deltad_{0}/#surd2 vs tan#lambda; tan#lambda;#Deltad_{0} [cm]",100,-1,1,100,-0.15,0.15);
  TH2D*hdZ0TanL = new TH2D("hdZ0TanL","#Deltaz_{0}/#surd2 vs tan#lambda; tan#lambda;#Deltaz_{0} [cm]",100,-1,1,100,-1.5,1.5);
  for(int i =0;i<nbin;++i){
    hdD0TanPhi0[i] = new TH2D(Form("hdD0TanPhi0_%d",i),"#Deltad_{0}/#surd2 vs tan#lambda; tan#lambda;#Deltad_{0} [cm]",100,-1,1,100,-0.15,0.15);
    hdZ0TanPhi0[i] = new TH2D(Form("hdZ0TanPhi0_%d",i),"#Deltaz_{0}/#surd2 vs tan#lambda; tan#lambda;#Deltaz_{0} [cm]",100,-1,1,100,-1.5,1.5);
  }
  TH2D*hdtanLTanL = new TH2D("hdtanLTanL","#Deltatan#lambda/#surd2 vs Tan#lambda1; tan#lambda1;#Deltatan#lambda",100,-1,1,100,-0.05,0.05);
  TH2D*hdPhi0TanL = new TH2D("hdPhi0TanL","#Delta#phi_{0}/#surd2 vs tan#lambda1; tan#lambda1;#Delta #phi_{0}",100,-1,1,100,-1,1);
  //Z0 dependence
  TH2D*hdD0Z0 = new TH2D("hdD0Z0","#Deltad_{0}/#surd2 vs z_{0}; z_{0};#Deltad_{0};",100,-80,150,100,-0.15,0.15);
  TH2D*hdZ0Z0 = new TH2D("hdZ0Z0","#Deltaz_{0}/#surd2 vs z_{0}; z_{0};#Deltaz_{0};",100,-80,150,100,-1.5,1.5);
  // D0 dependence
  TH2D*hdD0D0 = new TH2D("hdD0D0","#Deltad_{0}/#surd2 vs d_{0}; d_{0};#Deltad_{0};",50,-50,50,100,-0.15,0.15);
  TH2D*hdZ0D0 = new TH2D("hdZ0D0","#Deltaz_{0}/#surd2 vs d_{0}; d_{0};#Deltaz_{0};",50,-50,50,100,-1.5,1.5);
  // Phi0 dependence
 
  TH2D*hdD0Phi0 = new TH2D("hdD0Phi0","#Deltad_{0}/#surd2 vs #varphi_{0}; #varphi_{0};#Deltad_{0};",90,-180,0,100,-0.15,0.15);
  TH2D*hdZ0Phi0 = new TH2D("hdZ0Phi0","#Deltaz_{0}/#surd2 vs #varphi_{0}; #varphi_{0};#Deltaz_{0};",90,-180,0,100,-1.5,1.5);
  // Pt dependence
  TH2D* hdPtPt = new TH2D("hdPtPt"," ;P_{t} (Gev/c);#surd2(P_{t}^{up}-P_{t}^{down})/(P_{t}^{up}+P_{t}^{down})",10,0,5,100,-0.1,0.1);
  TH2D* hdPtPt_old = new TH2D("hdPtPt_old"," ;P_{t} (Gev/c);P_{t}^{up}-P_{t}^{down} (Gev/c)",10,0,5,100,-0.1,0.1);
  TH2D* hdD0Pt = new TH2D("hdD0Pt","#Deltad_{0}/#surd2 vs P_{t};P_{t};#Deltad_{0}",10,0,5,100,-0.15,0.15);
  TH2D* hdZ0Pt = new TH2D("hdZ0Pt","#Deltaz_{0}/#surd2 vs P_{t};P_{t};#Deltaz_{0}",10,0,5,100,-1.5,1.5);
  TH2D* hdPhi0Pt = new TH2D("hdPhi0Pt","#Delta#phi_{0}/#surd2 vs P_{t};P_{t};#Delta#phi0_{0} (rad)",10,0,5,100,-1,1);
  TH2D* hdtanLPt = new TH2D("hdtanLPt","#Deltatan#lambda/#surd2  vs P_{t};P_{t};#Deltatan#lambda",10,0,5,100,-0.05,0.05);
  TH2D* hevtT0Pt = new TH2D("hevtT0Pt","",10,0,5,100,-10,10);
  double ndf, Phi0, tanLambda, D0,Z0, Pval, Omega;

  hNDF1->SetLineColor(kRed);
  hPval1->SetLineColor(kRed);
  hPhi01->SetLineColor(kRed);
  htanLambda1->SetLineColor(kRed);
  hD01->SetLineColor(kRed);
  hZ01->SetLineColor(kRed);
  hOmega1->SetLineColor(kRed);
  hPt1->SetLineColor(kRed);
  hdOmega->GetXaxis()->SetNdivisions(8,9);
  hdtanL->GetXaxis()->SetNdivisions(8,9);
  hdPhi0->GetXaxis()->SetNdivisions(8,9);

  hdD0TanL->GetYaxis()->SetTitleOffset(1.4);
  hdZ0TanL->GetYaxis()->SetTitleOffset(1.4);
  hdD0TanL->GetYaxis()->SetTitleSize(0.05);
  hdZ0TanL->GetYaxis()->SetTitleSize(0.05);
  hdD0TanL->GetYaxis()->SetLabelSize(0.05);
  hdZ0TanL->GetYaxis()->SetLabelSize(0.05);
  Long64_t nbytes=0;
  double tanLm, Phi0m, D0m, Z0m;
  for(int i=0; i< tree->GetEntries();++i){
    //for(int i=0; i< 200;++i){
    nbytes += tree->GetEntry(i);    

    //    if(posSeed1->Y() * posSeed2->Y() > 0) continue;
    //    if(D01*D02 <0) continue;    
    //    if(cos(Mom1->Angle(*Mom2))<0) continue;    
    if(Mom1->Dot(*Mom2)<0) continue;    
    if(posSeed1->Y() <  posSeed2->Y() ){
      Phi0 = Phi01; Phi01 =Phi02; Phi02 =Phi0;
      Omega = Omega1; Omega1 = Omega2; Omega2 = Omega;
      tanLambda = tanLambda1; tanLambda1 = tanLambda2; tanLambda2 =tanLambda;
      Z0 = Z01; Z01 =Z02; Z02 =Z0;
      D0 = D01; D01 =D02; D02 =D0;
      ndf = ndf1; ndf1 =ndf2; ndf2 =ndf;
      Pval = Pval1; Pval1 =Pval2; Pval2 =Pval;
      TVector3*Mom = new TVector3(*Mom1); Mom1 = Mom2;Mom2 = Mom;
    }
    //    if(fabs(D02)>D0min||fabs(D01)>D0min) continue;

    Phi01 *=180/pi;
    Phi02 *=180/pi;

    hNDF1->Fill(ndf1);   hPval1->Fill(Pval1); hPhi01->Fill(Phi01);  hD01->Fill(D01); hZ01->Fill(Z01); htanLambda1->Fill(tanLambda1); hOmega1->Fill(Omega1); hPt1->Fill(Mom1->Perp());
    hNDF2->Fill(ndf2);   hPval2->Fill(Pval2); hPhi02->Fill(Phi02);  hD02->Fill(D02); hZ02->Fill(Z02); htanLambda2->Fill(tanLambda2); hOmega2->Fill(Omega2); hPt2->Fill(Mom2->Perp());

    // cut
    //    if(ndf1<25 ||ndf2<25) continue;
    if(ndf1<ndfmin||ndf2<ndfmin) continue;
    //    if(fabs(D01)>5||fabs(D01)>5) continue;

    //    if(Mom1->Perp()<1 || Mom2->Perp()<1) continue;
    double dD0=(D01-D02)/sqrt(2);
    double dZ0 = (Z01-Z02)/sqrt(2);
    double dPhi0=(Phi01-Phi02)/sqrt(2);
    double dtanLambda = (tanLambda1-tanLambda2)/sqrt(2);
    double dOmega = (Omega1 - Omega2)/sqrt(2);
    double dPt = (Mom1->Perp() - Mom2->Perp())/sqrt(2);
    
    D0m = (D01+D02)/2;
    Z0m = (Z01+Z02)/2;
    Phi0m = (Phi01+Phi02)/2;
    tanLm = (tanLambda1 + tanLambda2)/2;
        
    double Ptm = (Mom1->Perp() + Mom2->Perp())/2;
    double sigmaPt = dPt/Ptm;

    int index = floor(fabs(Phi01)/binWidth);
    if(index>=0 && index<nbin){
      hdD0TanPhi0[index]->Fill(tanLm, dD0);
      hdZ0TanPhi0[index]->Fill(tanLm, dZ0);
    }
    // z0 dependence, cut d0
    if(fabs(D0m) < 5){
      if(fabs(tanLambda1)<0.35 && fabs(tanLambda2)<0.35){
	hdD0Z0->Fill(Z0m, dD0);
	hdZ0Z0->Fill(Z0m, dZ0);    
      }
    }
    //D0 dependence, cut z0
    if(Z0m>-30 && Z0m < 50){
      hdD0D0->Fill(D0m, dD0);
      hdZ0D0->Fill(D0m, dZ0);
    }

    // cut at both d0 and z0 dependence for other dependence
    if(Z0m>-5 && Z0m < 15 &&  fabs(D0m) < 3 && fabs(tanLm)<0.45){
      //if(fabs(Z0m)<3 && fabs(tanLm) < 0.5 &&  fabs(D0m) < 1 && fabs(Phi0m+90)<40){
      hdD0->Fill(dD0);
      hdZ0->Fill(dZ0);
      hdPhi0->Fill(dPhi0);
      hdtanL->Fill(dtanLambda);
      hdPt->Fill(dPt);
      hdOmega->Fill(dOmega);
      
      hdPtPt->Fill(Ptm,sigmaPt);
      hdPtPt_old->Fill(Ptm,dPt);
      hdD0Pt->Fill(Ptm,dD0);
      hdZ0Pt->Fill(Ptm,dZ0);
      hdPhi0Pt->Fill(Ptm,dPhi0);
      hdtanLPt->Fill(Ptm,dtanLambda);
      hevtT0Pt->Fill(Ptm,evtT0);
      //tanL dependece
      hdtanLTanL->Fill(tanLm,dtanLambda);
      hdPhi0TanL->Fill(tanLm,dPhi0);
      
      hdD0TanL->Fill(tanLm, dD0);
      hdZ0TanL->Fill(tanLm, dZ0);
      
      hdD0Phi0->Fill(Phi0m,dD0);
      hdZ0Phi0->Fill(Phi0m,dZ0);
    }

    //    hTanLTanL->Fill(tanLambda1,tanLambda2);

  }

  //Draw.
  // First is track fit quality
  TCanvas* c1 = new TCanvas("c1","",1200,800);
  c1->Divide(2,1);

  c1->cd(1); hNDF1->Draw(); hNDF2->Draw("same");
  c1->cd(2); hPval1->Draw(); hPval2->Draw("same");
  c1->Print("TwoTrack_real.pdf(","Title:Fit_QA");
  //Second is track parameters

  c1 = new TCanvas("c1","",1200,800);
  c1->Divide(3,2);
  c1->cd(1); hPhi01->Draw(); hPhi02->Draw("same");
  c1->cd(2); hOmega1->Draw();hOmega2->Draw("same");
  c1->cd(3); hPt1->Draw();hPt2->Draw("same");

  c1->cd(4); htanLambda1->Draw(); htanLambda2->Draw("same");
  c1->cd(5); hD01->Draw(); hD02->Draw("same");
  c1->cd(6); hZ01->Draw(); hZ02->Draw("same");
  c1->Print("TwoTrack_real.pdf","Title:Pars_distribution");

  // Draw difference of track params
  TCanvas* c2 = new TCanvas("c2","",1200,800);
  c2->Divide(3,2);
  c2->cd(1);hdD0->Draw();fit(hdD0);
  c2->cd(2);hdZ0->Draw();fit(hdZ0);
  c2->cd(3);hdPhi0->Draw();fit(hdPhi0);
  c2->cd(4);hdtanL->Draw();fit(hdtanL);
  c2->cd(5);hdOmega->Draw();fit(hdOmega);
  c2->cd(6);hdPt->Draw();fit(hdPt);
  c2->Print("TwoTrack_real.pdf","Title:Pars_diff");

  //  c3->cd(3);hdD0TanL2->Draw("colz");
  // c3->cd(4);hdZ0TanL2->Draw("colz");

  //TanL dependece of D0, Z0
  TCanvas* c3 = new TCanvas("c3","",1200,800);
  c3->Divide(3,2);
  c3->cd(1);
  hdD0TanL->GetXaxis()->SetNdivisions(8,9);
  hdD0TanL->Draw("colz");
  hdD0TanL->FitSlicesY(0,0,-1,20);
  TH1D* m_D0TanL = (TH1D*)gDirectory->Get("hdD0TanL_1");
  TH1D* s_D0TanL = (TH1D*)gDirectory->Get("hdD0TanL_2");
  m_D0TanL->SetMinimum(-0.15);
  m_D0TanL->SetMaximum(0.15);
  m_D0TanL->GetYaxis()->SetTitleOffset(1.4);
  s_D0TanL->SetMinimum(0.);
  s_D0TanL->SetMaximum(0.05);

  if(m_D0TanL && m_D0TanL->GetEntries()>10)
    {  c3->cd(2);
      TPad* pad2 = (TPad*)c3->GetPrimitive("c3_2");
      pad2->cd();pad2->SetGrid();
      m_D0TanL->SetYTitle("#Deltad_{0} [cm]");
      m_D0TanL->Draw();
      //      m_D0TanL1->Fit("pol1");
      //      m_D0TanL1->GetFunction("pol1")->SetLineColor(kRed);
      m_D0TanL->GetXaxis()->SetNdivisions(8,9);
      c3->cd(3);
      s_D0TanL->SetYTitle("sigma of dD0 [cm]");
      s_D0TanL->Draw();
    }

  c3->cd(4);hdZ0TanL->Draw("colz");
  hdZ0TanL->FitSlicesY(0,0,-1,20);
  TH1D* m_Z0TanL = (TH1D*)gDirectory->Get("hdZ0TanL_1");
  TH1D* s_Z0TanL = (TH1D*)gDirectory->Get("hdZ0TanL_2");
  m_Z0TanL->SetMinimum(-1.5);
  m_Z0TanL->SetMaximum(1.5);
  m_Z0TanL->GetYaxis()->SetTitleOffset(1.4);
  s_Z0TanL->SetMinimum(0.);
  s_Z0TanL->SetMaximum(0.5);
  hdZ0TanL->GetXaxis()->SetNdivisions(8,9);
  if(m_Z0TanL  && m_Z0TanL->GetEntries()>10){
    c3->cd(5);
    TPad* pad5 = (TPad*)c3->GetPrimitive("c3_5");
    pad5->cd();pad5->SetGrid();

    m_Z0TanL->SetYTitle("#Deltaz_{0} [cm]");
    m_Z0TanL->Draw();
    //    m_Z0TanL1->Fit("pol1");
    //    m_Z0TanL1->GetFunction("pol1")->SetLineColor(kRed);
    m_Z0TanL->GetXaxis()->SetNdivisions(8,9);
    c3->cd(6);
    s_Z0TanL->SetYTitle("Sigma of dZ0 [cm]");
    s_Z0TanL->Draw();
  }
  c3->Print("TwoTrack_real.pdf","Title:tanL_depend");

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
  c3->Print("TwoTrack_real.pdf","Title:d0_depend");

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
  c3->Print("TwoTrack_real.pdf","Title:z0_depend");

  /*********dPt of Pt****************/
  TF1* f1 =new TF1("f1","sqrt([0]+[1]*x*x)",0,5);
  f1->SetParameters(0.2,0.002);
  f1->SetLineColor(kRed);
  c3 = new TCanvas("c3","",1200,800);
  c3->Divide(3,2);
  c3->cd(1);
  hdPtPt->GetXaxis()->SetNdivisions(8,9);
  hdPtPt->Draw("colz");
  hdPtPt->FitSlicesY(0,0,-1,20);
  TH1D* m_dPtPt = (TH1D*)gDirectory->Get("hdPtPt_1");
  TH1D* s_dPtPt = (TH1D*)gDirectory->Get("hdPtPt_2");
  s_dPtPt->SetTitle("Pt resolution;Pt (Gev/c);Pt resolution");
  if(m_dPtPt && m_dPtPt->GetEntries()>3) 
    {  c3->cd(2);
      TPad* pad2 = (TPad*)c3->GetPrimitive("c3_2");
      pad2->cd();pad2->SetGrid();
      m_dPtPt->SetTitle("Pt;P_{t} (GeV); #DeltaP_{t} (Gev/c)");
      m_dPtPt->Draw();
      m_dPtPt->GetXaxis()->SetNdivisions(8,9);
      c3->cd(3);
      //      s_dPtPt->SetTitle(";#sigma[]sP_{t};#sigmaP_{t}");
      s_dPtPt->SetMinimum(0);
      s_dPtPt->SetMaximum(0.02);
      s_dPtPt->Draw();
      s_dPtPt->Fit("f1","MQ","",1.5,5);
    }
  c3->Print("TwoTrack_real.pdf","Title:Pt_reso");



  ///Pt dependence
  hdD0Pt->FitSlicesY(0,0,-1,20);
  TH1D* hdD0Pt_m =  (TH1D*)gDirectory->Get("hdD0Pt_1");
  TH1D* hdD0Pt_s =  (TH1D*)gDirectory->Get("hdD0Pt_2");
  hdD0Pt_s->SetTitle("d_{0} resolution;Pt (Gev/c);#sigma (cm)");
  hdD0Pt_m->SetMinimum(-0.15);
  hdD0Pt_m->SetMaximum(0.15);
  hdD0Pt_s->SetMinimum(0);
  hdD0Pt_s->SetMaximum(0.15);


  hdZ0Pt->FitSlicesY(0,0,-1,20);
  TH1D* hdZ0Pt_m =  (TH1D*)gDirectory->Get("hdZ0Pt_1");
  TH1D* hdZ0Pt_s =  (TH1D*)gDirectory->Get("hdZ0Pt_2");
  hdZ0Pt_s->SetTitle("z_{0} resolution;Pt (Gev/c);#sigma (cm)");
  hdZ0Pt_m->SetMinimum(-1.5);
  hdZ0Pt_m->SetMaximum(1.5);
  hdZ0Pt_s->SetMinimum(0);
  hdZ0Pt_s->SetMaximum(0.5);

  hdPhi0Pt->FitSlicesY(0,0,-1,20);
  TH1D* hdPhi0Pt_m =  (TH1D*)gDirectory->Get("hdPhi0Pt_1");
  TH1D* hdPhi0Pt_s =  (TH1D*)gDirectory->Get("hdPhi0Pt_2");
  hdPhi0Pt_s->SetTitle("Phi_{0} resolution;Pt (Gev/c);#sigma (deg)");
  hdPhi0Pt_m->SetMinimum(-1);
  hdPhi0Pt_m->SetMaximum(1);
  hdPhi0Pt_s->SetMinimum(0);
  hdPhi0Pt_s->SetMaximum(1);

  hdtanLPt->FitSlicesY(0,0,-1,20);
  TH1D* hdtanLPt_m =  (TH1D*)gDirectory->Get("hdtanLPt_1");
  TH1D* hdtanLPt_s =  (TH1D*)gDirectory->Get("hdtanLPt_2");
  hdtanLPt_s->SetTitle("tan#lambda resolution;Pt (Gev/c);#sigma");
  hdtanLPt_m->SetMinimum(-0.05);
  hdtanLPt_m->SetMaximum(0.05);
  hdtanLPt_s->SetMinimum(0);
  hdtanLPt_s->SetMaximum(0.05);


  c3 = new TCanvas("c3","",1200,800);
  c3->Divide(3,2);
  c3->cd(1); hdD0Pt->Draw("colz");
  c3->cd(2); hdD0Pt_m->Draw(); 
  c3->cd(3); hdD0Pt_s->Draw();

  c3->cd(4); hdZ0Pt->Draw("colz");
  c3->cd(5); hdZ0Pt_m->Draw(); 
  c3->cd(6); hdZ0Pt_s->Draw();
  c3->Print("TwoTrack_real.pdf","Title:d0z0_vs_Pt");


  c3 = new TCanvas("c3","",1200,800);
  c3->Divide(3,2);
  c3->cd(1); hdPhi0Pt->Draw("colz");
  c3->cd(2); hdPhi0Pt_m->Draw(); 
  c3->cd(3); hdPhi0Pt_s->Draw();

  c3->cd(4); hdtanLPt->Draw("colz");
  c3->cd(5); hdtanLPt_m->Draw(); 
  c3->cd(6); hdtanLPt_s->Draw();
  c3->Print("TwoTrack_real.pdf)","Title:angle_vs_Pt");

  TFile *result = new TFile("result.root","recreate");
  result->cd();
  hdD0->Write();
  hdZ0->Write();
  s_dPtPt->Write();
  hdZ0Pt_s->Write();
  hdD0Pt_s->Write();
  hdPhi0Pt_s->Write();
  hdtanLPt_s->Write();
  hdD0D0->Write();
  hdZ0TanL->Write();
  hdD0TanL->Write();
  hevtT0Pt->Write();
  result->Close();
}
