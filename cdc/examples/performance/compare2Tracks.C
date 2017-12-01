#include "TROOT.h"
#include "TGraphErrors.h"
//#include "SliceFit.h"
void fit(TH1D* h1){
  gStyle->SetOptFit(1111);
  double p1 = h1->GetMaximum();
  double p3 = h1->GetRMS();
  Double_t par[6];
  TF1* f1 = new TF1("f1","gaus(0)+gaus(3)",2,2);
  f1->SetLineColor(kRed);
  f1->SetParameters(p1*0.8, 0., p3*0.6, p1*0.2, 0, p3*2.);
  TF1* g1 =new TF1("g1","gaus",-2,2);g1->SetLineColor(kBlue);
  TF1* g2 =new TF1("g2","gaus",-2,2);g2->SetLineColor(kGreen);
  h1->Fit("f1","MQ","",-3*p3,3*p3);
  f1->GetParameters(par);
  g1->SetParameters(&par[0]);
  g2->SetParameters(&par[3]);
  g1->Draw("same");
  g2->Draw("same");

}
void compare2Tracks(bool draw_sim =false, TString sCharge="all"){
  gStyle->SetTitleOffset(1.4,"y");
  gStyle->SetOptStat(0000);
  gStyle->SetPalette(1);
  gStyle->SetEndErrorSize(0);
  gROOT->SetBatch(1);
  TGaxis::SetMaxDigits(4);
  double binWidth=45;
  int ndfmin=25;
  int nbin = floor(180/binWidth);
  string filename = "output/output_*.root";
  //Root file contain histograms for compare with this results
  //TString result4Compare = "result_sim.root";
  TString result4Compare = "result_July.root";
  int color4Compare = 4; //color of input histo
  std::string label4Compare = "July"; //label of input histo

  //Belle CDC params
  //Belle CDC only: = 0.28Pt oplus 0.35/beta %
  double Param_belle_CDC[2] = {0.28, 0.35};

  TChain *tree = new TChain("tree");
  const double pi=3.141592653589793;
  tree->Add(filename.c_str());
  //  tree->Add(filename2.c_str());
  if(!tree->GetBranch("Pval1")) return;
  cout<<"Open TChain: "<<filename.c_str()<<endl;
  cout<<" Number of entry:"<<tree->GetEntries()<<endl;

  double ndf1, Pval1, Phi01, tanLambda1, D01, Z01;
  double ndf2, Pval2, Phi02, tanLambda2, D02, Z02, evtT0;
  TVector3* posSeed1=0;
  TVector3* posSeed2=0;
  TVector3* Mom1=0;
  TVector3* Mom2=0;
  double Omega1, Omega2;
  short charge;

  tree->SetBranchAddress("evtT0",&evtT0);
  tree->SetBranchAddress("charge",&charge);
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
  TH1D* htanLambda1 = new TH1D("htanLambda1","tan#lambda;tan#lambda;#tracks",100,-2,2);
  TH1D* htanLambda2 = new TH1D("htanLambda2","tan#lambda;tan#lambda;#tracks",100,-2,2);
  TH1D* hD01 = new TH1D("hD01","d_{0}",200,-30,30);
  TH1D* hD02 = new TH1D("hD02","d_{0}",200,-30,30);
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
  TH2D*hdD0TanL = new TH2D("hdD0TanL","#Deltad_{0}/#surd2 vs tan#lambda; tan#lambda;#Deltad_{0} [cm]",100,-1.5,1.5,100,-0.15,0.15);
  TH2D*hdZ0TanL = new TH2D("hdZ0TanL","#Deltaz_{0}/#surd2 vs tan#lambda; tan#lambda;#Deltaz_{0} [cm]",100,-1.5,1.5,100,-1.5,1.5);
  for(int i =0;i<nbin;++i){
    hdD0TanPhi0[i] = new TH2D(Form("hdD0TanPhi0_%d",i),"#Deltad_{0}/#surd2 vs tan#lambda; tan#lambda;#Deltad_{0} [cm]",100,-1,1,100,-0.15,0.15);
    hdZ0TanPhi0[i] = new TH2D(Form("hdZ0TanPhi0_%d",i),"#Deltaz_{0}/#surd2 vs tan#lambda; tan#lambda;#Deltaz_{0} [cm]",100,-1,1,100,-1.5,1.5);
  }
  TH2D*hdtanLTanL = new TH2D("hdtanLTanL","#Deltatan#lambda/#surd2 vs Tan#lambda1; tan#lambda1;#Deltatan#lambda",100,-1.5,1.5,100,-0.05,0.05);
  TH2D*hdPhi0TanL = new TH2D("hdPhi0TanL","#Delta#phi_{0}/#surd2 vs tan#lambda1; tan#lambda1;#Delta #phi_{0}",100,-1.5,1.5,100,-1,1);
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
  TH2D* hdPPt = new TH2D("hdPPt"," ;P_{t} (Gev/c);#surd2(P^{up}-P^{down})/(P^{up}+P^{down})",20,0,10,100,-0.1,0.1);
  TH2D* hdPtPt = new TH2D("hdPtPt"," ;P_{t} (Gev/c);#surd2(P_{t}^{up}-P_{t}^{down})/(P_{t}^{up}+P_{t}^{down})",20,0,10,100,-0.05,0.05);
  TH2D* hdPtPt_old = new TH2D("hdPtPt_old"," ;P_{t} (Gev/c);P_{t}^{up}-P_{t}^{down} (Gev/c)",20,0,10,100,-0.1,0.1);
  TH2D* hdD0Pt = new TH2D("hdD0Pt","#Deltad_{0}/#surd2 vs P_{t};P_{t};#Deltad_{0}",20,0,10,100,-0.15,0.15);
  TH2D* hdZ0Pt = new TH2D("hdZ0Pt","#Deltaz_{0}/#surd2 vs P_{t};P_{t};#Deltaz_{0}",20,0,10,100,-1.5,1.5);
  TH2D* hdPhi0Pt = new TH2D("hdPhi0Pt","#Delta#phi_{0}/#surd2 vs P_{t};P_{t};#Delta#phi0_{0} (deg)",20,0,10,100,-0.7,0.7);
  TH2D* hdtanLPt = new TH2D("hdtanLPt","#Deltatan#lambda/#surd2  vs P_{t};P_{t};#Deltatan#lambda",20,0,10,100,-0.03,0.03);
  TH2D* hdPtdPhi0 = new TH2D("hdPtdPhi0",";P_{t};#Delta#phi_{0}",100,-0.1,0.1,100,-1,1);
  TH2D* hdPtD0 = new TH2D("hdPtD0",";D0;#DeltaPt/Pt",100,-15,15,100,-0.1,0.1);
  TH2D* hdPtPhi0 = new TH2D("hdPtPhi0",";Phi0;#DeltaPt/Pt",180,-180,0,100,-0.1,0.1);
  TH2D* hevtT0Pt = new TH2D("hevtT0Pt","",20,0,10,100,-5,5);
  TH2D* hZ0Pt = new TH2D("hZ0Pt","z_{0} vs P_{t};P_{t};z_{0}",200,0,10,200,-80,150);
  TH3D* hdPtD0Phi0 = new TH3D("hdPtD0Phi0","",64,-16,16,36,-180,0,100,-0.05,0.05);
  TH2D* hsPtPhi0[30];
  for (int i=0;i<30;++i){
    hsPtPhi0[i] = new TH2D(Form("hsPtPhi0_%d",i),Form("D0 = %3.1f ;Phi0;dPt",i-15+0.5),90,-180,0,100,-0.1,0.1);
  }
  double ndf, Phi0, tanLambda, D0,Z0, Pval, Omega;

  hNDF1->SetLineColor(kRed);
  hPval1->SetLineColor(kRed);
  hPhi01->SetLineColor(kRed);
  htanLambda1->SetLineColor(kRed);
  hD01->SetLineColor(kRed);
  hZ01->SetLineColor(kRed);
  hOmega1->SetLineColor(kRed);
  hPt1->SetLineColor(kRed);
  hOmega1->GetXaxis()->SetNdivisions(4,5);
  hdOmega->GetXaxis()->SetNdivisions(8,9);
  hdtanL->GetXaxis()->SetNdivisions(8,9);
  hdPhi0->GetXaxis()->SetNdivisions(8,9);

  hdD0TanL->GetYaxis()->SetTitleOffset(1.4);
  hdZ0TanL->GetYaxis()->SetTitleOffset(1.4);
  hdD0TanL->GetYaxis()->SetTitleSize(0.05);
  hdZ0TanL->GetYaxis()->SetTitleSize(0.05);
  hdD0TanL->GetYaxis()->SetLabelSize(0.05);
  hdZ0TanL->GetYaxis()->SetLabelSize(0.05);
  TH1D* hdPtPt_sim;
  TH1D* hdD0Pt_sim;
  TH1D* hdZ0Pt_sim;
  TH1D* hdPhi0Pt_sim;
  TH1D* hdtanLPt_sim;
  TH1D*  hdPPt_sim;
  if(draw_sim){
    TFile ff(result4Compare);
    hdPtPt_sim = (TH1D*)ff.Get("hdPtPt_2;1");
    hdPPt_sim = (TH1D*)ff.Get("hdPPt_2;1");
    hdD0Pt_sim = (TH1D*)ff.Get("hdD0Pt_2;1"); 
    hdZ0Pt_sim = (TH1D*)ff.Get("hdZ0Pt_2;1");
    hdPhi0Pt_sim = (TH1D*)ff.Get("hdPhi0Pt_2;1"); 
    hdtanLPt_sim = (TH1D*)ff.Get("hdtanLPt_2;1"); 
    hdPtPt_sim->SetDirectory(0);
    hdPtPt_sim->SetLineColor(color4Compare);
    hdPtPt_sim->SetMarkerColor(color4Compare);
    hdPtPt_sim->GetFunction("f1")->SetLineColor(color4Compare);
    hdPtPt_sim->GetFunction("f1")->SetLineWidth(1);
    hdPPt_sim->SetDirectory(0);
    hdPPt_sim->SetLineColor(color4Compare);
    hdPPt_sim->SetMarkerColor(color4Compare);
    hdPPt_sim->GetFunction("f1")->SetLineColor(color4Compare);
    hdPPt_sim->GetFunction("f1")->SetLineWidth(1);

    hdD0Pt_sim->SetDirectory(0);
    hdD0Pt_sim->SetLineColor(color4Compare);
    hdZ0Pt_sim->SetDirectory(0);
    hdZ0Pt_sim->SetLineColor(color4Compare);
    hdPhi0Pt_sim->SetDirectory(0);
    hdPhi0Pt_sim->SetLineColor(color4Compare);
    hdtanLPt_sim->SetDirectory(0);
    hdtanLPt_sim->SetLineColor(color4Compare);
    ff.Close();
  }


  Long64_t nbytes=0;
  int n=0;
  for(int i=0; i< tree->GetEntries();++i){
    nbytes += tree->GetEntry(i);    
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
    Phi01 *=180/pi;
    Phi02 *=180/pi;
    n +=1;
    double dD0=(D01-D02)/sqrt(2);
    double dZ0 = (Z01-Z02)/sqrt(2);
    double dPhi0=(Phi01-Phi02)/sqrt(2);
    double dtanLambda = (tanLambda1-tanLambda2)/sqrt(2);
    double dOmega = (Omega1 - Omega2)/sqrt(2);
    double dPt = (Mom1->Perp() - Mom2->Perp())/sqrt(2);
    double dP = (Mom1->Mag() - Mom2->Mag())/sqrt(2);
    
    double D0m = (D01+D02)/2;
    double Z0m = (Z01+Z02)/2;
    double Phi0m = (Phi01+Phi02)/2;
    double tanLm = (tanLambda1 + tanLambda2)/2;
    double Ptm = (Mom1->Perp() + Mom2->Perp())/2;
    double Pm = (Mom1->Mag() + Mom2->Mag())/2;

    double sigmaPt = dPt/Ptm;
    double sigmaP = dP/Pm;
    int index = floor(fabs(Phi01)/binWidth);

    //    if(n>300000) continue;
    if(sCharge=="pos"){
      if(charge <0) continue;
    }else if (sCharge =="neg"){
      if(charge >0) continue;
    }else{}

    hNDF1->Fill(ndf1);   hPval1->Fill(Pval1); hPhi01->Fill(Phi01);  hD01->Fill(D01); hZ01->Fill(Z01); htanLambda1->Fill(tanLambda1); hOmega1->Fill(Omega1); hPt1->Fill(Mom1->Perp());
    hNDF2->Fill(ndf2);    hPval2->Fill(Pval2); hPhi02->Fill(Phi02);  hD02->Fill(D02); hZ02->Fill(Z02); htanLambda2->Fill(tanLambda2); hOmega2->Fill(Omega2); hPt2->Fill(Mom2->Perp());
    hZ0Pt->Fill(Ptm,Z0m);

    
    // NDF cut
    if(ndf1<ndfmin ||ndf2<ndfmin) continue;

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
    if(Z0m>-30 && Z0m < 50 ){
      hdD0D0->Fill(D0m, dD0);
      hdZ0D0->Fill(D0m, dZ0);
    }
    if(fabs(Z0)<25 && Ptm>0.5 && Ptm<1.5 && fabs(Phi0m+105)<5){
      hdPtD0->Fill(D0m, sigmaPt);
      hdPtPhi0->Fill(Phi0m, sigmaPt);}
    // cut at both d0 and z0 dependence for other dependence
    //    if(Z0m>-10 && Z0m < 30 &&  fabs(D0m) > 7 & fabs(D0m) < 15 && fabs(tanLm) <0.45){
    if(Z0m>-5 && Z0m < 10 &&  fabs(D0m) < 3){
      hdtanLTanL->Fill(tanLm,dtanLambda);
      hdPhi0TanL->Fill(tanLm,dPhi0);
      hdD0TanL->Fill(tanLm, dD0);
      hdZ0TanL->Fill(tanLm, dZ0);}
    if(Z0m>-5 && Z0m < 10 &&  fabs(D0m) <3 && fabs(tanLm) < 0.5){
      hdD0Phi0->Fill(Phi0m,dD0);
      hdZ0Phi0->Fill(Phi0m,dZ0);
    }
    //good cut    if(Z0m>-5 && Z0m < 10 &&  fabs(D0m) <3 && fabs(tanLm) < 0.45 && fabs(Phi0m+110)<10){
    if(Ptm<2.5){
      hdPtD0Phi0->Fill(D0m,Phi0m,sigmaPt);}
    if(Z0m>-5 && Z0m < 15 &&  fabs(D0m) <3 && fabs(tanLm) < 0.45){

      hdD0->Fill(dD0);
      hdZ0->Fill(dZ0);
      hdPhi0->Fill(dPhi0);
      hdtanL->Fill(dtanLambda);
      hdPt->Fill(dPt);
      hdOmega->Fill(dOmega);

      hdPtPt->Fill(Ptm,sigmaPt);
      hdPPt->Fill(Ptm,sigmaP);
      hdPtPt_old->Fill(Ptm,dPt);
      hdD0Pt->Fill(Ptm,dD0);
      hdZ0Pt->Fill(Ptm,dZ0);
      hdPhi0Pt->Fill(Ptm,dPhi0);
      hdtanLPt->Fill(Ptm,dtanLambda);
      hevtT0Pt->Fill(Ptm,evtT0);
      hdPtdPhi0->Fill(sigmaPt, dPhi0);

    }

    if(fabs(D0m)>15) continue;
    int ibin = floor(D0m+15);
    hsPtPhi0[ibin]->Fill(Phi0m,sigmaPt);
  }


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
  //Belle results
  TF1* fbelle =new TF1("fbelle","sqrt([0]*[0]*x*x+[1]*[1])",0,10);
  //  fbelle->SetParameters(0.201,0.29);// CDC+SVD
  fbelle->SetParameters(Param_belle_CDC);//CDC only
  fbelle->SetLineColor(kMagenta-7);
  fbelle->SetLineStyle(2);
  fbelle->SetLineWidth(3);

  ///
  TF1* f1 =new TF1("f1","sqrt([0]*[0]*x*x+[1]*[1])",0,10);
  f1->SetParameters(0.2,0.2);
  f1->SetLineColor(kBlack);
  f1->SetLineWidth(1);
  c3 = new TCanvas("c3","",1200,800);
  c3->Divide(3,2);
  c3->cd(1);
  hdPtPt->GetXaxis()->SetNdivisions(8,9);
  hdPtPt->Draw("colz");
  hdPtPt->FitSlicesY(0,0,-1,0);
  TH1D* hdPtPt_m = (TH1D*)gDirectory->Get("hdPtPt_1");
  TH1D* hdPtPt_s = (TH1D*)gDirectory->Get("hdPtPt_2");
  //  TH1D* hdPtPt_s = (TH1D*)SliceFit::doSliceFitY(hdPtPt,10)->Clone("hdPtPt_2");
  //  hdPtPt_s->SetDirectory(0);
  if(hdPtPt_m && hdPtPt_m->GetEntries()>3 && hdPtPt_s) 
    {  c3->cd(2);
      TPad* pad2 = (TPad*)c3->GetPrimitive("c3_2");
      pad2->cd();pad2->SetGrid();
      hdPtPt_m->SetTitle("#DeltaPt/Pt;P_{t} (GeV/c); #DeltaP_{t}/P_{t}");
      hdPtPt_m->SetMinimum(-0.01);
      hdPtPt_m->SetMaximum(0.01);
      hdPtPt_m->Draw();
      hdPtPt_m->GetXaxis()->SetNdivisions(8,9);
      c3->cd(3);
      //      s_dPtPt->SetTitle(";#sigma[]sP_{t};#sigmaP_{t}");
      hdPtPt_s->SetTitle("Pt Resolution; Pt (Gev/c);Pt resolution (%)");
      hdPtPt_s->Scale(100);
      hdPtPt_s->SetMinimum(0);
      hdPtPt_s->SetMaximum(2);
      hdPtPt_s->SetMarkerStyle(8);
      hdPtPt_s->SetMarkerSize(0.6);
      hdPtPt_s->Draw();
      hdPtPt_s->Fit("f1","MQ","",1.5,10);
      gPad->Update();
      cout<<"helo"<<endl;
      TPaveStats *s = (TPaveStats*)hdPtPt_s->GetListOfFunctions()->FindObject("stats");
      s->SetBorderSize(0);  s->SetFillColor(0);s->SetShadowColor(0);  
      s->SetX1(0.5); s->SetY1(1.5);s->SetX2(6); s->SetY2(1.95);
      gPad->Modified();
      fbelle->DrawF1(0.6,10,"same");
      TLatex lt;
      lt.SetTextSize(0.05);
      lt.SetTextColor(kMagenta-7);
      lt.DrawLatex(7,1.7,"#splitline{Belle}{CDC only}");
      if(draw_sim){
	cout<<"draw Sim Pt resolution"<<endl;
	//	hdPtPt_sim->Scale(100);
	hdPtPt_sim->SetStats(0);
	hdPtPt_sim ->Draw("same");
	TLatex lt2;
	lt2.SetTextSize(0.05);
	lt2.SetTextColor(color4Compare);
	lt2.DrawLatex(8,0.5,label4Compare.c_str());
      }
    }

  //For Momentum reso vs Transver mom
  c3->cd(4);
  hdPPt->GetXaxis()->SetNdivisions(8,9);
  hdPPt->Draw("colz");
  hdPPt->FitSlicesY(0,0,-1,20);
  TH1D* hdPPt_m = (TH1D*)gDirectory->Get("hdPPt_1");
  TH1D* hdPPt_s = (TH1D*)gDirectory->Get("hdPPt_2");
  hdPPt_s->SetTitle("P resolution;Pt (Gev/c);P resolution");
  if(hdPPt_m && hdPPt_m->GetEntries()>3) 
    {  c3->cd(5);
      TPad* pad5 = (TPad*)c3->GetPrimitive("c3_5");
      pad5->cd();pad5->SetGrid();
      hdPPt_m->SetTitle("P;P_{t} (GeV); #DeltaP (Gev/c)");
      hdPPt_m->Draw();
      hdPPt_m->GetXaxis()->SetNdivisions(8,9);
      c3->cd(6);
      //      s_dPtPt->SetTitle(";#sigma[]sP_{t};#sigmaP_{t}");
      hdPPt_s->SetMinimum(0);
      hdPPt_s->SetMaximum(0.02);
      hdPPt_s->Draw();
      hdPPt_s->Fit("f1","MQ","",1.5,10);
      if(draw_sim){
      	hdPPt_sim ->Draw("same");
      }
    }

  ///Old Formular//////////////////////////////////////////////////
  /*
  c3->cd(4);
  hdPtPt_old->GetXaxis()->SetNdivisions(8,9);
  hdPtPt_old->Draw("colz");
  hdPtPt_old->FitSlicesY(0,0,-1,20);
  TH1D* m_dPtPt_old = (TH1D*)gDirectory->Get("hdPtPt_old_1");
  TH1D* s_dPtPt_old = (TH1D*)gDirectory->Get("hdPtPt_old_2");
  m_dPtPt_old->SetMinimum(-0.15);
  m_dPtPt_old->SetMaximum(0.15);
  m_dPtPt_old->GetYaxis()->SetTitleOffset(1.4);
  s_dPtPt_old->SetMinimum(0.);
  s_dPtPt_old->SetMaximum(0.02);
  TH1D* hPtReso = (TH1D*)s_dPtPt_old->Clone("hPtReso");
  hPtReso->SetTitle("Pt resolution;Pt (GeV);#sigmaPt/#surd2/Pt");
  hPtReso->GetYaxis()->SetTitleOffset(1.6);

  //  s_dPtPt_old->SetTitle(";#sigma[]sP_{t};#sigmaP_{t}");
  if(m_dPtPt_old && m_dPtPt_old->GetEntries()>3) {  
    c3->cd(5);
    TPad* pad5 = (TPad*)c3->GetPrimitive("c3_5");
    pad5->cd();pad5->SetGrid();
    m_dPtPt_old->SetTitle("#DeltaP_{t};P_{t} (GeV); #DeltaP_{t} (cm)");
    m_dPtPt_old->Draw();
    m_dPtPt_old->GetXaxis()->SetNdivisions(8,9);
    c3->cd(6);
    for(int i=0; i<s_dPtPt_old->GetNbinsX();++i){
     double reso = s_dPtPt_old->GetBinContent(i)/std::sqrt(2)/s_dPtPt_old->GetBinCenter(i);
     double dreso = s_dPtPt_old->GetBinError(i)/std::sqrt(2)/s_dPtPt_old->GetBinCenter(i);
     hPtReso->SetBinContent(i,reso);
     hPtReso->SetBinError(i,dreso);
    }
    hPtReso->SetMinimum(0.);
    hPtReso->SetMaximum(0.02);
    hPtReso->Draw();
    hPtReso->Fit("f1","MQ");
  */
  c3->Print("TwoTrack_real.pdf","Title:Pt_reso");



  ///Pt dependence
  hdD0Pt->FitSlicesY(0,0,-1,5);
  TH1D* hdD0Pt_m =  (TH1D*)gDirectory->Get("hdD0Pt_1");
  TH1D* hdD0Pt_s =  (TH1D*)gDirectory->Get("hdD0Pt_2");
  hdD0Pt_s->SetTitle("d_{0} resolution;Pt (Gev/c);#sigma (cm)");
  hdD0Pt_m->SetMinimum(-0.15);
  hdD0Pt_m->SetMaximum(0.15);
  hdD0Pt_s->SetMinimum(0);
  hdD0Pt_s->SetMaximum(0.15);


  hdZ0Pt->FitSlicesY(0,0,-1,5);
  TH1D* hdZ0Pt_m =  (TH1D*)gDirectory->Get("hdZ0Pt_1");
  TH1D* hdZ0Pt_s =  (TH1D*)gDirectory->Get("hdZ0Pt_2");
  hdZ0Pt_s->SetTitle("z_{0} resolution;Pt (Gev/c);#sigma (cm)");
  hdZ0Pt_m->SetMinimum(-1.5);
  hdZ0Pt_m->SetMaximum(1.5);
  hdZ0Pt_s->SetMinimum(0);
  hdZ0Pt_s->SetMaximum(0.5);

  hdPhi0Pt->FitSlicesY(0,0,-1,5);
  TH1D* hdPhi0Pt_m =  (TH1D*)gDirectory->Get("hdPhi0Pt_1");
  TH1D* hdPhi0Pt_s =  (TH1D*)gDirectory->Get("hdPhi0Pt_2");
  hdPhi0Pt_s->SetTitle("Phi_{0} resolution;Pt (Gev/c);#sigma (deg)");
  hdPhi0Pt_m->SetMinimum(-1);
  hdPhi0Pt_m->SetMaximum(1);
  hdPhi0Pt_s->SetMinimum(0);
  hdPhi0Pt_s->SetMaximum(1);

  hdtanLPt->FitSlicesY(0,0,-1,5);
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
  if(draw_sim){
    hdD0Pt_sim ->Draw("same");
  }

  c3->cd(4); hdZ0Pt->Draw("colz");
  c3->cd(5); hdZ0Pt_m->Draw(); 
  c3->cd(6); hdZ0Pt_s->Draw();
  if(draw_sim){
    hdZ0Pt_sim ->Draw("same");
  }

  c3->Print("TwoTrack_real.pdf","Title:d0z0_vs_Pt");


  c3 = new TCanvas("c3","",1200,800);
  c3->Divide(3,2);
  c3->cd(1); hdPhi0Pt->Draw("colz");
  c3->cd(2); hdPhi0Pt_m->Draw(); 
  c3->cd(3); hdPhi0Pt_s->Draw();
  if(draw_sim){
    hdPhi0Pt_sim ->Draw("same");
  }

  c3->cd(4); hdtanLPt->Draw("colz");
  c3->cd(5); hdtanLPt_m->Draw(); 
  c3->cd(6); hdtanLPt_s->Draw();
  if(draw_sim){
    hdtanLPt_sim ->Draw("same");
  }

  c3->Print("TwoTrack_real.pdf)","Title:angle_vs_Pt");
  cout<<"Finish"<<endl;
  TFile *result = new TFile("result.root","recreate");
  result->cd();
  hD01->Write();
  hZ01->Write();
  htanLambda1->Write();
  hPhi01->Write();
  hPt1->Write();

  hdD0->Write();
  hdZ0->Write();
  hdPtPt->Write();
  hdPPt_s->Write();
  hdPtPt_s->Write();
  hdZ0Pt_s->Write();
  hdD0Pt_s->Write();
  hdPhi0Pt_s->Write();
  hdtanLPt_s->Write();
  //hPtReso->Write();
  hdD0D0->Write();
  hdZ0TanL->Write();
  hdD0TanL->Write();
  hevtT0Pt->Write();
  hdPtdPhi0->Write();
  hdPtD0->Write();
  hdPtPhi0->Write();
  hZ0Pt->Write();
  hdPtD0Phi0->Write();
  //D0 phi0 dependence
  TDirectory* newDir = gDirectory->mkdir("Phi0Depend");
  newDir->cd();
  for(int i=0;i<30;++i){
    hsPtPhi0[i]->Write();
  }
  result->Close();
}
