#include <stdlib.h>
#include "TH1F.h"
#include "TH2F.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TVirtualPad.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TF1.h"
#include "TPaveStats.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TArc.h"

struct info { float ncount,ar, mirr, thc, fic,x,y,tx,ty,sx,sy;};
info var;

int ARICHBtest(int agel=0, int mir=0){

char mtxt[128];
            
TCanvas *c= new TCanvas("c","c",210*3.5,297*3.5);
TPaveText *pt = new TPaveText(0,0.95,1,1);
pt->AddText( "ARICH Beamtest");
pt->Draw();
c->cd();
TVirtualPad *main  = new TPad("man","Main", 0,0,1,0.95,10,1);

main->Draw();
main->Divide(2,3);

gStyle->SetOptStat(0);
gStyle->SetOptFit(0);
main->cd(2);

sprintf(mtxt,"agel==%d&&mir==%d " , agel, mir);

TNtuple *hits = (TNtuple *) gDirectory->Get("hits");
hits->Draw("thc*sin(fic):thc*cos(fic)>>h2d(100,-0.4,0.4,100,-0.4,0.4)",mtxt,"");

TH2F *h2d = (TH2F *) gDirectory->Get("h2d");
if (!h2d) {
  printf("h2d does not exist");
  return 0;
}
h2d->SetTitle("Cherenkov space;tx[rad];ty[rad]");
h2d->Draw("colz");
//h2d->Draw();

//TArc *krog= new TArc(0,0,0.31); krog->SetFillStyle(4000);krog->Draw();

main->cd(6);
hits->Draw("y:x","mir==0","box");


gStyle->SetOptStat(1111);
gStyle->SetOptFit(1);
TVirtualPad *c1=main->cd(1);

hits->Draw("thc>>hx(200,0,0.5)",mtxt,""); 

TH1F *hx = (TH1F *) gDirectory->Get("hx");
hx->SetTitle("thetac;Cherenkov angle[rad];N");
         

TF1 *mygaus = new TF1("mygaus","gaus", 0.2, 0.4);
hx->Fit(mygaus,"QR");
Double_t par[5];
mygaus->GetParameters(par);
par[3]=0;
par[4]=0;


TF1 *myfit = new TF1("myfit","[3]+ [4]*x+gaus(0)", 0.2, 0.4);
myfit->SetParameters(par);
myfit->SetRange(0.2,0.4);
hx->Fit(myfit,"QR");
myfit->GetParameters(par);
 

TNtuple *tracks = (TNtuple *) gDirectory->Get("tracks");
double nentr = tracks->GetEntriesFast();
double dx = hx->GetBinWidth(1);
double tmin= par[1]-TMath::Abs(par[2])*3;
double tmax= par[1]+TMath::Abs(par[2])*3;
double  nfot=2.507*par[2]*par[0]/dx/nentr;
double  nbkg=(par[4]*(tmax*tmax-tmin*tmin)/2.+ par[3]*(tmax-tmin))/dx/nentr;

sprintf(mtxt,"%s\t nfot=%2.2f     nbgr=%2.2f agel=%d mir=%d ", gSystem->BaseName(gDirectory->GetFile()->GetName()) , nfot,nbkg, agel, mir);
pt->AddText(mtxt);

c->Modified();
c->Update();
  
hx->GetListOfFunctions()->ls();
TPaveStats *st = (TPaveStats*)(hx->GetListOfFunctions()->FindObject("stats"));
if (st){
  st->SetName("test");
  st->SetX1NDC(0.1); //new x start position
  st->SetX2NDC(0.5); //new x end position
} else {
  printf("stats not found\n");
} 
hx->Draw("sames");

hits->SetBranchAddress("n", &var.ncount);
hits->SetBranchAddress("thc", &var.thc);
hits->SetBranchAddress("fic", &var.fic);
hits->SetBranchAddress("mir", &var.mirr);
hits->SetBranchAddress("agel", &var.ar);

//
// calculate some statistics
//
int evenum=-1;
int neve =  hits->GetEntries();
TH1F *havgthc= new TH1F("havgthc","Average theta Cherenkov;thc[rad];N", 300, 0.1,0.4);
TH1F *hnhits = new TH1F("hnhits","Number of all hits;hits;N", 50, -0.5,49.5);
TH1F *hchhits = new TH1F("hchhits","Number of hits in +-3 sigma;hits;N", 30, -0.5,29.5);
TH1F *hsimchhits = new TH1F("hsimchhits","Number of hits in +-3 sigma;hits;N", 30, -0.5,29.5);

int nhits=0;
int nchhits=0;
int allhits=0;
float thetac=0;
for( Int_t idx=0; idx<neve; ++idx ) {
  hits->GetEntry( idx );
  if (var.ar>0.5 ) continue;
  if (var.mirr>0.5) continue;
  if (idx){
    if (evenum != int(var.ncount)){
       if (nchhits) havgthc->Fill(thetac/nchhits);
       hnhits->Fill(nhits);
       hchhits->Fill(nchhits);
       nhits=0;
       thetac=0;
       nchhits=0;
    }
  } 
  
  nhits++;
  
  if (var.thc<tmax && var.thc >tmin){
    thetac+= var.thc;
    nchhits++;
    allhits++;
  }
  evenum= int(var.ncount);
  
}

//---------------------------------------------------------------------------
gStyle->SetOptFit(1111);
main->cd(3);
havgthc->Draw();
c->Modified();
c->Update();
TPaveStats *st1 = (TPaveStats*)(havgthc->GetListOfFunctions()->FindObject("stats"));
if (st1){
  st1->SetName("test");
  st1->SetX1NDC(0.1); //new x start position
  st1->SetX2NDC(0.3); //new x end position
  havgthc->Draw("sames");
} 


TVirtualPad *pad = main->cd(4);
pad->Divide(1,2);
pad->cd(2);
hnhits->Draw();
pad->cd(1);
tracks->Draw("acc");

main->cd(5);
hchhits->Draw();
double mean= hchhits->GetMean();
double entries= hchhits->GetEntries();
TF1 *f1 = new TF1("f1","[1]*TMath::Poisson(x,[0])",0,30);
f1->SetParameter(0,mean);
f1->SetParameter(1,entries);
hsimchhits->FillRandom("f1",entries);
hsimchhits->SetMarkerStyle( kFullDotMedium);
hsimchhits->Draw("samep");
//f1->Draw("same");

c->Modified();
c->Update();
const char *cpdf = gSystem->BaseName(gDirectory->GetFile()->GetName());
TString *pdf =  new TString(cpdf);
pdf->ReplaceAll(".root",".pdf");
c->Print( pdf->Data()  );
pdf->Prepend("acroread ");
//gSystem->Exec(pdf->Data());
//ARICHBtesttracking( );
return 0;
}


int align(){
gStyle->SetOptFit(1111);
TNtuple *hits = (TNtuple *) gDirectory->Get("hits");
gStyle->SetOptFit(111);
hits->Draw("TMath::Sqrt((y-ty)*(y-ty)+(x-tx)*(x-tx)):TMath::ATan2(y-ty,x-tx)>>halign(100,-3.1415,3.1415,100,0,10)");
//TF1 *f = new TF1("mgauss","gaus", 5, 8);
TF1 *f = new TF1("mgauss","[3]+ [4]*x+gaus(0)", 5, 8);
TF1 *fshift = new TF1("mshift","[0]-[1]*cos(x)+[2]*sin(x)", -3.1415,3.1415);
f->SetParameter(0,100);
f->SetParameter(1,6.5);
f->SetParameter(2,1);
//f->FixParameter(2,max*0.5);
TH2F *halign = (TH2F *) gDirectory->Get("halign");
  halign->ls();
  printf("Start\n");
 halign->FitSlicesY(f);
 f->ls();
 printf("Plot\n");
 TAxis *ax= halign->GetYaxis();
 double xmin=  ax->GetXmin();
 double xmax=  ax->GetXmax();
 TH1F *h0 = ((TH1F * ) gDirectory->Get("halign_0"));
 TH1F *h1 = ((TH1F * ) gDirectory->Get("halign_1"));
 TH1F *h2 = ((TH1F * ) gDirectory->Get("halign_2"));
 //h0->Draw();
 TCanvas *c= new TCanvas("calign","calign",800,1000);
 c->Divide(2,3);
 
 
 c->cd(2);
 TH2F *ha = halign;
 double maxcontent =   ha->GetBinContent(ha->GetMaximumBin());
 h0->SetMaximum(maxcontent);
 h0->SetMinimum(0);
 h0->Draw();
 c->cd(3);
 
 h1->SetMaximum(xmax);
 h1->SetMinimum(xmin);
 h1->Fit(fshift);
 c->cd(4);
 h2->SetMaximum(2);
 h2->SetMinimum(0);
 h2->Draw();
 c->cd(1);
 
 halign->Draw();
 fshift->Draw("same");
 printf("now thc\n");
 c->cd(5);
TF1 *f1 = new TF1("mthc","[3]+ [4]*x+gaus(0)",0.2, 0.4);
f1->SetParameter(0,100);
f1->SetParameter(1,0.3);
f1->SetParameter(2,0.020);
char txt[255];

 hits->Draw("thc:fic>>h2dm0(100,-3.1415,3.1415,100,0,0.4)","agel==0&&mir==0");
 TH2F *h2dm0 = (TH2F *) gDirectory->Get("h2dm0");
 h2dm0->FitSlicesY(f1);
 TH1F *hx0 = ((TH1F * ) gDirectory->Get("h2dm0_1"));
 hx0->Fit(fshift);
 h2dm0->Draw(); 
 fshift->Draw("same");
 double par[3];
 fshift->GetParameters(par);
 sprintf(txt,"p0=%3.3g p1=%3.3g p2=%3.3g", par[0],par[1],par[2]);
 h2dm0->SetTitle(txt);
 TPaveText *pt0 = new TPaveText(0,0.5,1,7);
pt0->AddText(txt);
pt0->Draw();
 c->cd(6);

 hits->Draw("thc:fic>>h2dm1(100,-3.1415,3.1415,100,0,0.4)","agel==1&&mir==0");
 TH2F *h2dm1 = (TH2F *) gDirectory->Get("h2dm1"); 
 h2dm1->FitSlicesY(f1);
 TH1F *hx1 = ((TH1F * ) gDirectory->Get("h2dm1_1"));
 hx1->Fit(fshift);
 h2dm1->Draw(); 
 fshift->Draw("same");
 fshift->GetParameters(par);
 sprintf(txt,"p0=%3.3g p1=%3.3g p2=%3.3g", par[0],par[1],par[2]);
 h2dm1->SetTitle(txt);
  TPaveText *pt1 = new TPaveText(0,0.5,1,7);
pt1->AddText(txt);
pt1->Draw();

 c->Modified();
 c->Update();
 gStyle->SetOptFit();
 return 0;
} 




int ARICHBtesttracking( ){
  
  TCanvas *c= new TCanvas("c","c",1000,1000);
  c->Divide(4,4);
  char name[128];
  for (int i=0;i<4;i++){
    for (int j=0;j<2;j++){
      sprintf(name,"mwpc%d_a%d_sum",i,j);
      TH1* h0= (TH1 *) gDirectory->Get(name);
      printf("%s\n",name);
      c->cd(4*i+2*j+1);
      if (h0) h0->Draw();
   
      sprintf(name,"mwpc%d_a%d_sum_cut",i,j);
      TH1* h0x= (TH1 *) gDirectory->Get(name);
      printf("%s\n",name);
      if (h0x) h0x->Draw("same");

      sprintf(name,"mwpc%d_a%d_diff",i,j);
      TH1* h1= (TH1 *) gDirectory->Get(name);
      printf("%s\n",name);
      c->cd(4*i+2*j+2);
      if (h1) h1->Draw();
    }
  }
  c->Modified();
  c->Update();


  TCanvas *c1= new TCanvas("c1","c1",1000,1000);
  c1->Divide(5,4);

  for (int i=0;i<4;i++){
    for (int j=0;j<5;j++){
      sprintf(name,"mwpc%d_a%d_tdc",i,j);
      TH1* h2= (TH1 *) gDirectory->Get(name);
      printf("%s\n",name);
      c1->cd(5*i+j+1);
      if (h2) h2->Draw();
    }
  }
  c1->Modified();
  c1->Update();

  TCanvas *c2= new TCanvas("c2","c2",1500,1000);
  c2->Divide(6,4);

  for (int i=0;i<4;i++){
    for (int j=0;j<2;j++){
      sprintf(name,"mwpc%d_a%d_resd_z",i,j);
      TH2* h3= (TH2 *) gDirectory->Get(name);
      h3->FitSlicesX();

      printf("%s\n",name);
      c2->cd(6*i+j*3+1);
      if (h3) h3->Draw("colz");
      
      sprintf(name,"mwpc%d_a%d_resd_z_2",i,j);
      c2->cd(6*i+j*3+2)->SetGrid(1,1); 
      TH1* h4= (TH1 *) gDirectory->Get(name);
      if (h4) h4->Draw();
      
      sprintf(name,"mwpc%d_a%d_resd_z_1",i,j);
      c2->cd(6*i+j*3+3)->SetGrid(1,1); 
      TH1* h5= (TH1 *)gDirectory->Get(name);
      if (h5) h5->Draw();    
    }
  }
  c2->Modified();
  c2->Update();
  return 0;
}
