/***********************************************************************
 * Script plots ARICH memory usage testing                             *
 *                                                                     *
 * > cd ~arich/examples/                                               *
 * > printenv | grep -i belle2                                         *
 * > which basf2                                                       *
 * > which python3                                                     *
 * > b2code-memoryusage -h                                             *
 * > b2code-memoryusage -m record -i 0.01 -p ARICHStandAlone_memory.npz basf2 -n 1000 ARICHStandAlone.py -- -b -m | tee ARICHStandAlone_memory.log
 * > python3 ../utility/scripts/convert_npz_to_ASCII.py --npzfile=ARICHStandAlone_memory.npz --dump
 * > python3 ../utility/scripts/convert_npz_to_ASCII.py --npzfile=ARICHStandAlone_memory.npz --arrayname total_memory
 * > root -l ../utility/scripts/convert_npz_ASCII_to_root.C"(\"total_memory.dat\")"
 *                                                                     *
 *Author: Leonid Burmistrov (Fri Jun 15 22:13:34 JST 2018)             *
***********************************************************************/

//root
#include <TROOT.h>
#include <TStyle.h>
#include <TPad.h>
#include <TCanvas.h>
#include <TString.h>
#include <TFile.h>
#include <TGraph.h>
#include <TAxis.h>
#include <TLine.h>
#include <TLegend.h>
#include <TMultiGraph.h>
#include <TH1D.h>

//C, C++
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <time.h>

using namespace std;

int readDataFile(TString fileName, vector<double> &t_v, vector<double> &v_v, vector<double> &r_v, vector<double> &p_v, vector<double> &s_v);

void convert_npz_ASCII_to_root( TString fileName = "total_memory.dat"){

  cout<<"fileName "<<fileName<<endl;
    
  vector<double> t_v; //Time
  vector<double> v_v; //Virtual
  vector<double> r_v; //Resident
  vector<double> p_v; //Proportional
  vector<double> s_v; //Swap
  
  int nn = readDataFile(fileName,t_v,v_v,r_v,p_v,s_v);
  
  double *t = new double[nn];
  double *v = new double[nn];
  double *r = new double[nn];
  double *p = new double[nn];
  double *s = new double[nn];
  
  TH1D *h1_p = new TH1D("h1_p","prop",1000,0.0,2000000);
  
  for (unsigned i=0; i<t_v.size(); i++){
    t[i] = t_v.at(i);
    v[i] = v_v.at(i);
    r[i] = r_v.at(i);
    p[i] = p_v.at(i);
    s[i] = s_v.at(i);
    h1_p->Fill(p_v.at(i));
  }
  
  TGraph *gr_v = new TGraph(nn,t,v);
  TGraph *gr_r = new TGraph(nn,t,r);
  TGraph *gr_p = new TGraph(nn,t,p);
  TGraph *gr_s = new TGraph(nn,t,s);
  
  gr_v->SetName("h1_Virtual");
  gr_v->SetTitle("Virtual");
  gr_v->SetMarkerStyle(21);
  gr_v->SetMarkerColor(kBlack);
  gr_v->SetLineColor(kBlack);
  gr_v->SetLineWidth(3);
  gr_v->GetXaxis()->SetTitle("time, s");
  gr_v->GetYaxis()->SetTitle("Memory, kB");
  
  gr_r->SetName("h1_Resident");
  gr_r->SetTitle("Resident (Rss)");
  gr_r->SetMarkerStyle(21);
  gr_r->SetMarkerColor(kRed);
  gr_r->SetLineColor(kRed);
  gr_r->SetLineWidth(3);
  gr_r->GetXaxis()->SetTitle("time, s");
  gr_r->GetYaxis()->SetTitle("Memory, kB");
  
  gr_p->SetName("h1_Proportional");
  gr_p->SetTitle("Proportional (Pss)");
  gr_p->SetMarkerStyle(21);
  gr_p->SetMarkerColor(kBlue);
  gr_p->SetLineColor(kBlue);
  gr_p->SetLineWidth(3);
  gr_p->GetXaxis()->SetTitle("time, s");
  gr_p->GetYaxis()->SetTitle("Memory, kB");
  
  gr_s->SetName("h1_Swap");
  gr_s->SetTitle("Swap");
  gr_s->SetMarkerStyle(21);
  gr_s->SetMarkerColor(kMagenta);
  gr_s->SetLineColor(kMagenta);
  gr_s->SetLineWidth(3);
  gr_s->GetXaxis()->SetTitle("time, s");
  gr_s->GetYaxis()->SetTitle("Memory, kB");
  
  h1_p->SetLineColor(kBlue);
  h1_p->SetLineWidth(3);
  h1_p->GetXaxis()->SetTitle("Memory, kB");

  gStyle->SetPalette(1);
  gStyle->SetFrameBorderMode(0);
  gROOT->ForceStyle();
  gStyle->SetStatColor(kWhite);
  gStyle->SetOptStat(kFALSE);
  
  TCanvas *c1 = new TCanvas("c1","plots",10,10,800,800);
  c1->Clear();
  c1->SetFillColor(kWhite);
  c1->GetPad(0)->SetLeftMargin(0.12);
  c1->GetPad(0)->SetRightMargin(0.02);
  c1->GetPad(0)->SetTopMargin(0.07);
  //c1->GetPad(0)->SetBottomMargin(0.02);
  
  // draw a frame to define the range
  TMultiGraph *mg = new TMultiGraph();
  mg->SetTitle(fileName.Data());
  mg->Add(gr_v);
  mg->Add(gr_r);
  mg->Add(gr_p);
  mg->Add(gr_s);
  mg->Draw("APL");
  
  mg->GetXaxis()->SetTitle("time, s");
  mg->GetYaxis()->SetTitle("Memory, kB");
  
  TLegend *leg = new TLegend(0.6,0.6,0.9,0.9,"","brNDC");
  leg->AddEntry(gr_v, "Virtual","lp");
  leg->AddEntry(gr_r, "Resident","lp");
  leg->AddEntry(gr_p, "Proportional","lp");
  leg->AddEntry(gr_s, "Swap","lp");
  leg->Draw();
  
  TCanvas *c2 = new TCanvas("c2","plots",20,20,800,800);
  c2->Clear();
  c2->SetFillColor(kWhite);
  c2->GetPad(0)->SetLeftMargin(0.12);
  c2->GetPad(0)->SetRightMargin(0.02);
  c2->GetPad(0)->SetTopMargin(0.07);
  h1_p->Draw();
  
  TString rootFileName = fileName;
  rootFileName += ".root";    
  
  TFile f(rootFileName.Data(), "recreate");
  f.cd();
  gr_v->Write();
  gr_r->Write();
  gr_p->Write();
  gr_s->Write();
  h1_p->Write();
  c1->Write();
  
  f.Close();
  
}

int readDataFile(TString fileName, vector<double> &t_v, vector<double> &v_v, vector<double> &r_v, vector<double> &p_v, vector<double> &s_v){
  double t,v,r,p,s;
  ifstream myfile (fileName.Data());
  int nn = 0;
  if(myfile.is_open()){
    while(myfile>>t>>v>>r>>p>>s){
      t_v.push_back(t);
      v_v.push_back(v);
      r_v.push_back(r);
      p_v.push_back(p);
      s_v.push_back(s);
      nn++;
    }
    myfile.close();
  } 
  else{
    cout << "Unable to open file" <<endl; 
  }
  return nn;
}

//#ifdef CONVERT_NPZ_TO_ROOT_MAIN
//# ifndef __CINT__
//int main(int argc, char *argv[]){
//  if(argc == 2){
//    TString fileName = argv[1];
//    convert_npz_ASCII_to_root(fileName);
//    return 0;
//  }
//  convert_npz_ASCII_to_root();
//  return 0;
//}
//# endif //ifndef __CINT__
//#endif //ifdef CONVERT_NPZ_TO_ROOT_MAIN
