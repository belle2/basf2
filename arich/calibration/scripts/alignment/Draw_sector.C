/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <TH2.h>
#include <fstream>
#include <TLegend.h>

void Draw_sector(const char*file){
  TFile *_file0 = new TFile(Form("simbha/Bhabha_%s.root",file));
  TTree* ch = (TTree*)_file0->Get("arich");

  TH2D* h1 = new TH2D("h1",";arich_of_phi[rad];n*cos(theta_c)", 2*TMath::Pi(), -TMath::Pi(),TMath::Pi(), 40, 0.98, 1.02);
  //ch->Draw("photons.m_nCosThetaCh_e:photons.m_phiHit>>h1","charge<0 && recHit.p>4.0");
  ch->Draw("photons.m_nCosThetaCh_e:atan2(photons.m_y,photons.m_x)>>h1","charge<0.0 && eop>0.9 && e9e21>0.95 && recHit.p>4.0");

  TH1D*h2=new TH1D("h2",";n*cos(theta_c);Entries", 40, 0.98, 1.02);
  //ch->Draw("photons.m_nCosThetaCh_e>>h2","charge<0 && recHit.p>4.0");
  ch->Draw("photons.m_nCosThetaCh_e>>h2","charge<0.0 && eop>0.9 && e9e21>0.95 && recHit.p>4.0");

  TH2D* h3 = new TH2D("h3",";phi[rad];n*cos(theta_c)", 2*TMath::Pi(), -TMath::Pi(), TMath::Pi(), 40, 0.98, 1.02);
  //ch->Draw("photons.m_nCosThetaCh_e:photons.m_phiCh_e>>h3","charge<0 && recHit.p>4.0");
  ch->Draw("photons.m_nCosThetaCh_e:photons.m_phiCh_e>>h3","charge<0.0 && eop>0.9 && e9e21>0.95 && recHit.p>4.0");

  TH1D*pj[13];
  
  for(Int_t i=0;i<13;i++){
    int first=h3->GetXaxis()->FindBin(-3.0+i*0.5-0.1);
    int last=h3->GetXaxis()->FindBin(-3.0+i*0.5+0.1);
    pj[i]=h3->ProjectionY(Form("pj_%d",i),first,last);
  }
  TF1*f1[13];
  for(Int_t j=0;j<13;j++) {
    pj[j]->Draw();
    Float_t Mean=pj[j]->GetMean();
    Float_t RMS=pj[j]->GetRMS();
    Float_t Max=pj[j]->GetMaximum();
    f1[j]=new TF1(Form("f1_%d",j),"gaus(0)+pol1(3)",Mean-RMS*3.0,Mean+RMS*3.0);
    f1[j]->SetParameter(0,Max);
    f1[j]->SetParameter(1,Mean);
    f1[j]->SetParameter(2,RMS);
    f1[j]->SetParameter(3,0);
    f1[j]->SetParameter(4,0);
    //f1[j]->SetParameter(5,0); //pol2
    pj[j]->Fit(Form("f1_%d",j),"R");
  }
  Int_t N=13;
  Float_t x[13],x_err[13];
  Float_t y[13],y_err[13];
  for(Int_t k=0;k<13;k++){
    x[k]=-3+0.5*k;
    x_err[k]=0;
    y[k]=f1[k]->GetParameter(1);
    y_err[k]=f1[k]->GetParError(1);
  }
  TGraphErrors*g=new TGraphErrors(N,x,y,x_err,y_err);
  g->SetTitle("theta_vs_phi");
  g->GetXaxis()->SetTitle("phi[rad]");
  g->GetYaxis()->SetTitle("n*cos(theta_c)");
  g->GetXaxis()->SetTitleSize(0.05);
  g->GetYaxis()->SetTitleSize(0.05);
  g->GetXaxis()->SetLabelSize(0.05);
  g->GetYaxis()->SetLabelSize(0.05);
  g->SetMarkerStyle(7);
  g->SetMaximum(1.02);
  g->SetMinimum(0.98);

  TF1*f2=new TF1("f2","[0]+[1]*cos(x+[2])",-TMath::Pi(),TMath::Pi());
  f2->SetParameter(0,1.01);
  f2->SetParLimits(0, 0.0, 10.0);
  f2->SetParameter(1,0.0);
  f2->SetParLimits(1, 0.0, 1.0);
  f2->SetParameter(2,0);                                                              
  f2->SetParLimits(2,-TMath::Pi(),TMath::Pi());
  g->Fit("f2","R");
  gStyle->SetOptFit(0111);

  TF1*h2_fit=new TF1("h2_fit", "gaus(0)+pol1(3)", 0.98, 1.04);
  h2_fit->SetParameters(500000, 1.01, 0.004, 0, 0);

  // Difine Canvas                                                                 
  TCanvas* c1 = new TCanvas("c1");
  c1->Update();
  c1->Divide(2,2);

  c1->cd(1);
  h1->Draw("colz");
  c1->cd(2);
  h2->Fit("h2_fit","R");
  h2->Draw();
  c1->cd(3);
  h3->Draw("colz");
  c1->cd(4);
  g->Draw("AP");


  //Difine Canvas 6*6   
  TCanvas* c2=new TCanvas("c2");
  c2->Update();
  c2->Divide(6,6);

  //Make histogram ARICH is subdivided into 24 regions.                                                   
  TH2D* h4[36];
  for(int t=0;t<12;t++){ //Divide 12 parts (-pi~pi) sector                                                         
    for(int m=0;m<3;m++){ //Divide 3 parts (0~105)                                                        
      int s=m+3*t; //0~35                                                                                 
      c2->cd(s+1); //change divided canvas                                                                
      h4[s]=new TH2D(Form("h4_%d",s),";phi[rad];n*cos(theta_c)", 32, -3.2, 3.2, 40, 0.98, 1.02); //Difine24 histograms                                   
      //Difine range                                                                    
      int rmin=50+m*18.5; //50~87                                                        
      int rmax=50+(m+1)*18.5; //68.5~105.5                                             
      double phiMin=-TMath::Pi()+t*TMath::Pi()/6.0; //-pi~(2pi/3)   
      double phiMax=-TMath::Pi()+(t+1.0)*TMath::Pi()/6.0; //(-pi/2)~pi                                    
      //Fill histograms                                                                                   
      //ch->Draw(Form("photons.m_nCosThetaCh_e:photons.m_phiCh_e>>h4_%d",s), Form("charge<0 && %d<photons.m_rHit && photons.m_rHit<%d && %f<photons.m_phiHit && photons.m_phiHit<%f && recHit.p>4.0",rmin, rmax, phiMin, phiMax));
      ch->Draw(Form("photons.m_nCosThetaCh_e:photons.m_phiCh_e>>h4_%d",s), Form("charge<0.0 && eop>0.9 && e9e21>0.95 && %d<photons.m_rHit && photons.m_rHit<%d && %f<atan2(photons.m_y,photons.m_x) && atan2(photons.m_y,photons.m_x)<%f && recHit.p>4.0",rmin, rmax, phiMin, phiMax));
      h4[s]->SetTitle(Form("%d<r<%d, %f<phi<%f", rmin, rmax, phiMin, phiMax));
      h4[s]->Draw("colz");
      h4[s]->SetStats(0);
    }
  }
  //Fit 36 regions                                        
  TCanvas*c3=new TCanvas("c3");
  c3->Update();
  c3->Divide(6,6);
  TGraphErrors *g2[36];
  TF1* fit1[36];
  //TCanvas*c4[36];
  TFile*g_file=new TFile(Form("simbha/Graph/g36_Bhabha_%s.root",file),"RECREATE");
  for(int k=0;k<36;k++){
    c3->cd(k+1);
    TH1D* projection[13];
    Int_t first=0;
    Int_t last=0;
    TF1* fit[13];
    Int_t j=0;
    //c4[k]=new TCanvas(Form("c4_%d",k));
    //c4[k]->Update();
    //c4[k]->Divide(5,3);
    for(Int_t i=0;i<N;i++){
      //c4[k]->cd(i+1);
      first=h4[k]->GetXaxis()->FindBin(-3.0+i*0.5-0.1); //Difine first bin 0.92,0.92,...,1.05 
      last=h4[k]->GetXaxis()->FindBin(-3.0+i*0.5+0.1); //Difine last bin 0.92,0.93,...,1.06   
      projection[i]=h4[k]->ProjectionY(Form("projection_%d",i),first,last); //Divide h2 into 13 (ProjectionY)                                                                   
             
      Float_t Max=projection[i]->GetMaximum();
      Float_t Mean=projection[i]->GetMean();
      Float_t RMS=projection[i]->GetRMS();
      Int_t Entries=projection[i]->Integral();
      fit[i]=new TF1(Form("fit_%d",i),"gaus(0)+pol1(3)",Mean-RMS*3.0,Mean+RMS*3.0);
      fit[i]->SetParameter(0,Max);
      fit[i]->SetParameter(1,Mean);
      fit[i]->SetParameter(2,RMS);
      fit[i]->SetParameter(3,0);
      fit[i]->SetParameter(4,0);
      //fit[i]->SetParameter(5,0); //pol2
      projection[i]->Fit(Form("fit_%d",i),"R");
      
      if(Entries>100 && fit[i]->GetParError(1)<0.003 && fit[i]->GetParameter(2)<0.01){
	x[j]=-3.0+i*0.5; //phi                                                       
	x_err[j]=0; //phi err                                                             
	y[j]=fit[i]->GetParameter(1); //Get n*cos(thetac) Mean                                      
	y_err[j]=fit[i]->GetParError(1); //Get Mean Error                                        
	j++;
      }
    }
    //c4[k]->Print(Form("simbha/PNG/%s_%d.png",file,k+1));
    //c3->cd(k+1); //with c4

    g2[k]=new TGraphErrors(j,x,y,x_err,y_err); // phi vs n*cos(thetac) graph      
    g2[k]->SetTitle(Form("n*cos(thetac)_vs_phi_%d",k));
    g2[k]->GetXaxis()->SetTitle("phi[rad]");
    g2[k]->GetYaxis()->SetTitle("n*cos(thetac)");
    g2[k]->GetXaxis()->SetTitleSize(0.05);
    g2[k]->GetYaxis()->SetTitleSize(0.05);
    g2[k]->GetXaxis()->SetLabelSize(0.05);
    g2[k]->GetYaxis()->SetLabelSize(0.05);
    g2[k]->SetMarkerStyle(7);
    g2[k]->SetMaximum(1.02);
    g2[k]->SetMinimum(0.98);
    g2[k]->GetXaxis()->SetLimits(-3.3,3.3);
    g2[k]->Draw("AP");
    g2[k]->Write();
    fit1[k]=new TF1(Form("fit1_%d",k+1),"[0]+[1]*cos(x+[2])",-TMath::Pi(),TMath::Pi());
    fit1[k]->SetParameter(0,1.01);
    fit1[k]->SetParLimits(0, 0.0, 10.0);
    //fit1[k]->SetParameter(1,0.003); //z_tra, z_rot
    fit1[k]->SetParameter(1,0.005);// x_tra,y_tra
    //fit1[k]->SetParLimits(1, 0.0005, 0.1); //x_tra, z_tra, x_rot
    fit1[k]->SetParLimits(1, 0.004, 0.1); // y_tra
    //fit1[k]->SetParLimits(1, 0.002, 0.1); // y_rot
    //fit1[k]->SetParameter(2,0); //x_tra, z_tra, y_rot, z_rot
    fit1[k]->SetParameter(2,-TMath::Pi()/2); //y_tra
    //fit1[k]->SetParameter(2,TMath::Pi()/2); //x_rot
    //fit1[k]->SetParLimits(2,-TMath::Pi(),TMath::Pi()); //x_tra, z_rot, z_tra
    fit1[k]->SetParLimits(2,-TMath::Pi()*2/3,-1/3*TMath::Pi()); //y_tra
    //fit1[k]->SetParLimits(2,TMath::Pi()/3,2/3*TMath::Pi()); //x_rot
    //fit1[k]->SetParLimits(2,-TMath::Pi()/2,TMath::Pi()/2); //y_rot
    gStyle->SetStatH(0.2);    
    gStyle->SetStatW(0.5);    
    g2[k]->Fit(Form("fit1_%d",k+1),"R");
  }
  g_file->Close();
  
    //Save 36 Offsets (p0)                                                                          
  ofstream fout3(Form("simbha/Offset/Offset_Bhabha_%s.txt",file));
  for(int j=0;j<36;j++){
    fout3<<fit1[j]->GetParameter(0)<<","<<fit1[j]->GetParError(0)<<","<<endl;
 }
  
  //Save 36 Amplitudes (p1)                                                                          
  ofstream fout(Form("simbha/Amplitude/Amp_Bhabha_%s.txt",file));
  for(int j=0;j<36;j++){
  fout<<fit1[j]->GetParameter(1)<<","<<fit1[j]->GetParError(1)<<","<<endl;
  }
  
  //Save 36 phase (p2)                                                                     
  ofstream fout2(Form("simbha/Phase/Phase_Bhabha_%s.txt",file));
  for(int j=0;j<36;j++){
    fout2<<fit1[j]->GetParameter(2)<<","<<fit1[j]->GetParError(2)<<","<<endl;
  }
  //*/
}
