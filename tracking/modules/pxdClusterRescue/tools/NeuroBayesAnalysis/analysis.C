#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>

#include <TFile.h>
#include <TF1.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TProfile.h>
#include <TMath.h>
#include <TGraph.h>
#include <TImage.h>
#include <TLatex.h>
#include <TPostScript.h>
#include <TLine.h>
#include <TH2F.h>
#include <TBox.h>
#include <TPolyMarker.h>
#include <TColor.h>
#include <TPDF.h>
#include <TCutG.h>
#include <TPaveText.h>
#include <TPad.h>
#include <TGaxis.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <cstdlib>
#include <TLegend.h>

#include "asciiToRoot.C"

using namespace std;

#define ENDOFSTRING -1

class TPaveTextNB 
{
private:
  TPaveText* pave;
public:
  //constructor
  TPaveTextNB(){pave = new TPaveText();};
  TPaveTextNB(double x1,double y1,double x2,double y2,const char* text)
  {
    pave = new TPaveText(x1,y1,x2,y2,"blNDC");
    pave->SetBorderSize(0);
    pave->SetFillStyle(0);
    pave->AddText(text);
  }
  void Draw(const char* o=""){pave->Draw(o);};
};


bool verbose_analysis() {return false;};

// /////////////////////////////////////////////////////////////////////
// //////////////          helper functions         ////////////////////
// /////////////////////////////////////////////////////////////////////

TH1F* loadHisto(TFile* file,int index,const char* title,bool hbookMode)
{
  char histoName[50];
  if (hbookMode) 
    sprintf(histoName,"h%i;1: %s",index,title);
  else
    sprintf(histoName,"%i",index);
  if(! file->IsOpen()) 
    {
      cout<<"the file is not open"<<endl;
      exit(1);
    }

  if(verbose_analysis()) cout<<histoName<<endl;

  TH1F* histo = (TH1F*)file->Get(histoName);
  if(histo==0 && hbookMode==0)
    {
      // retry with another name (newer NB version)
      sprintf(histoName,"h%i",index);
      histo = (TH1F*)file->Get(histoName);
    }
  if(verbose_analysis() && histo==0) cout<<"loadHisto : Histogram "<<histoName
					 <<" does not exist"<<endl;
  return histo;  
}

TH2F* loadHisto2D(TFile* file,int index,const char* title,bool hbookMode)
{
  char histoName[50];
  if (hbookMode) 
    sprintf(histoName,"h%i;1: %s",index,title);
  else
    sprintf(histoName,"h%i",index);

  TH2F* histo = (TH2F*)file->Get(histoName);
  return histo;  
}
void setMarkerAttributes(TAttMarker* h,int color=0,double size=0,int style=0)
{

  if(color>0) h->SetMarkerColor(color);
  if(size>0) h->SetMarkerSize(size);
  if(style>0) h->SetMarkerStyle(style);

  return;  
}

void setAxisAttributes(TH1F* histo,
		       const char* title,
		       const char* xTitle,
		       const char* yTitle,
		       double xTitleSize = 0,
		       double yTitleSize = 0,
		       double xTitleOffset = 0,
		       double yTitleOffset = 0,
		       double xLabelSize = 0,
		       double yLabelSize = 0)
{
  histo->SetTitle(title);  
  histo->GetXaxis()->SetTitle(xTitle);
  histo->GetYaxis()->SetTitle(yTitle);
  histo->GetXaxis()->CenterTitle(1);
  histo->GetYaxis()->CenterTitle(1);

  if(xTitleSize != 0) histo->GetXaxis()->SetTitleSize(xTitleSize);
  if(yTitleSize != 0) histo->GetYaxis()->SetTitleSize(yTitleSize);
  if(xTitleOffset != 0) histo->GetXaxis()->SetTitleOffset(xTitleOffset);
  if(yTitleOffset != 0) histo->GetYaxis()->SetTitleOffset(yTitleOffset);

  if(xLabelSize != 0) histo->GetXaxis()->SetLabelSize(xLabelSize);
  if(yLabelSize != 0) histo->GetYaxis()->SetLabelSize(yLabelSize);

  return;  
}

void setAxisAttributes(TGraph* graph,
		       const char* title,
		       const char* xTitle,
		       const char* yTitle,
		       double xTitleSize = 0,
		       double yTitleSize = 0,
		       double xTitleOffset = 0,
		       double yTitleOffset = 0,
		       double xLabelSize = 0,
		       double yLabelSize = 0)
{
  graph->SetTitle(title);  
  graph->GetXaxis()->SetTitle(xTitle);
  graph->GetYaxis()->SetTitle(yTitle);
  graph->GetYaxis()->CenterTitle(1);
  graph->GetXaxis()->CenterTitle(1);

  if(xTitleSize != 0) graph->GetXaxis()->SetTitleSize(xTitleSize);
  if(yTitleSize != 0) graph->GetYaxis()->SetTitleSize(yTitleSize);
  if(xTitleOffset != 0) graph->GetXaxis()->SetTitleOffset(xTitleOffset);
  if(yTitleOffset != 0) graph->GetYaxis()->SetTitleOffset(yTitleOffset);

  if(xLabelSize != 0) graph->GetXaxis()->SetLabelSize(xLabelSize);
  if(yLabelSize != 0) graph->GetYaxis()->SetLabelSize(yLabelSize);

  return;  
}

void plotErrors(TFile* f, bool HbookMode,bool pdfOutput,TPad* graphPad)
{

  TH1F* h100 = loadHisto(f,100,"ERROR LEARNSAMPLE",HbookMode);
  TH1F* h200 = loadHisto(f,200,"ERROR TESTSAMPLE",HbookMode);
  TH1F* h400 = loadHisto(f,400,"REG*WEIGHT LEARNSAMPLE",HbookMode);
  TH1F* h401 = loadHisto(f,401,"ERR-WEIGHT LEARNSAMPLE",HbookMode);

  graphPad->Divide(2,2);
   
  graphPad->GetPad(1)->cd();
  graphPad->GetPad(1)->SetGridx();
  graphPad->GetPad(1)->SetGridy();
  setAxisAttributes(h100,"","Training iteration","arb. units");
  double max = TMath::Max(0.,h100->GetMaximum());
  h100->SetMaximum(max);
  h100 -> Draw();

  TPaveTextNB *pt1 = new TPaveTextNB(0.3,0.90144,0.7,0.965,"Error");
  pt1->Draw();

  graphPad->GetPad(2)->cd();
  graphPad->GetPad(2)->SetGridx();
  graphPad->GetPad(2)->SetGridy();
  setAxisAttributes(h200,"","Training iteration","arb. units");
  h200 -> Draw();

  TPaveTextNB *pt2 = new TPaveTextNB(0.1,0.90144,0.9,0.965,"Error Testsample");
  pt2->Draw("same");

  graphPad->GetPad(3)->cd();
  graphPad->GetPad(3)->SetGridx();
  graphPad->GetPad(3)->SetGridy();
  setAxisAttributes(h400,"","Training iteration","arb. units");
  h400 -> Draw();

  TPaveTextNB *pt3 = new TPaveTextNB(0.1,0.90144,0.9,0.995,"regularisation param. * weights");
  pt3->Draw("same");


  graphPad->GetPad(4)->cd();
  graphPad->GetPad(4)->SetGridx();
  graphPad->GetPad(4)->SetGridy();
  setAxisAttributes(h401,"","Training iteration","arb. units");
  h401 -> Draw(); 

  TPaveTextNB *pt4 = new TPaveTextNB(0.1,0.90144,0.9,0.965,"Err-Weight Learnsample");
  pt4->Draw("same");

  delete pt1;
  delete pt2;
  delete pt3;
  delete pt4;
  return;
}

void PurityEfficiency(TH1F* h1,  TH1F* h2, TH1F **OutputGraph,
		      bool interpolate,int num=0,bool density=false) 
{
  
  const int NBins = h1->GetNbinsX();

  double NEntries1 = (double) h1 -> GetSumOfWeights();
  double NEntries2  = (double)h2 -> GetSumOfWeights();

  //unpack histograms into arrays
  /* NB: Array[0]:   number of underflows
   *     Array[1]:   first bin
   *     Array[n]:   last bin
   *     Array[n+1]: number of overflows 
   */
  Float_t *Array1  = h1 -> GetArray(); //BG
  Float_t *Array2  = h2 -> GetArray(); // sig
 
  double *Eff11 = new double [NBins+1];
  double *Eff12 = new double [NBins+1];
  double *Eff21 = new double [NBins+1];
  double *Eff22 = new double [NBins+1];

  double *Pur11 = new double [NBins+1];
  double *Pur12 = new double [NBins+1];
  double *Pur21 = new double [NBins+1];
  double *Pur22 = new double [NBins+1];

  double *Pur11Error = new double [NBins+1];
  double *Pur12Error = new double [NBins+1];
  double *Pur21Error = new double [NBins+1];
  double *Pur22Error = new double [NBins+1];

  double *CostnL = new double [NBins+1]; //selection efficiency
  double *CostnR = new double [NBins+1]; //selection efficiency

  double Tot = 0.0;
  double Sum1 = 0.0;
  double Sum2  = 0.0;

  // cutting left --> right
  for (int i = 1; i<= NBins; i++) {
    Sum1 = 0.0;
    Sum2  = 0.0;
    for (int j = i-1; j<= NBins; j++) {
      Sum1  += Array1[j]; //bg
      Sum2  += Array2[j]; //sig
    } //for j
    Tot = Sum1 + Sum2;
    //
    //efficiency
    //
    Eff11[i-1] = Sum1/((double)NEntries1 );
    if (Eff11[i-1]>0.9999)  Eff11[i-1] = 0.9999;

    Eff21[i-1] = Sum2/((double)NEntries2);
    if (Eff21[i-1]>0.9999)  Eff21[i-1] = 0.9999;

    double temp = (Tot)/double(NEntries1+NEntries2); //efficiency
    if(!density)
      CostnL[i-1] = TMath::Min(temp,1.);
    else
      CostnL[i-1] = TMath::Min(Sum1/double(NEntries1),1.);
	
    //
    //purity
    //
    // CL: same formulas as in calc.f
    Pur11[i-1] = (Sum1+1)/(Tot+2);
    Pur11Error[i-1] = sqrt((Tot-Sum1+1)/((Tot+3)*(Tot+2))*Pur11[i-1]);
    Pur21[i-1] = (Sum2+1)/(Tot+2);
    Pur21Error[i-1] = sqrt((Tot-Sum2+1)/((Tot+3)*(Tot+2))*Pur21[i-1]);
  } //for i

  // proper initialization of the last values, 
  // after everything has been cut away
  if(interpolate) {
    CostnL[NBins] = 0.;
    Eff21[NBins] = 0.;
    Pur21[NBins] = 1.;
  }


  //cutting right --> left
  for (int i = NBins; i>=1; i--) {
    Sum1 = 0.0;
    Sum2 = 0.0;
    for (int j = i; j>=1; j--) {
      Sum1 += Array1[j];
      Sum2 += Array2[j];
    } //for j
    Tot = Sum1 + Sum2;
      
    //
    // Efficiency
    //
    Eff12[NBins-i] = Sum1/((double)NEntries1);
    if (Eff12[NBins-i] > 0.9999) Eff12[NBins-i] = 0.9999999;

    Eff22[NBins-i] = Sum2/((double)NEntries2);
    if (Eff22[NBins-i] > 0.9999) Eff22[NBins-i] = 0.9999999;

    double temp = (Tot)/double(NEntries1+NEntries2);
    CostnR[NBins-i] = TMath::Min(temp,1.);
    //
    // purity
    // 
    Pur12[NBins-i] = (Sum1+1)/(Tot+2);
    Pur12Error[NBins-i] = sqrt((Tot-Sum1+1)/((Tot+3)*(Tot+2))*Pur12[NBins-i]);
    if(fabs(Pur12Error[NBins-i])>0.9999) Pur12Error[NBins-i] = 0;
    Pur22[NBins-i] = (Sum2+1)/(Tot+2);
    Pur22Error[NBins-i] = sqrt((Tot-Sum2+1)/((Tot+3)*(Tot+2))*Pur22[NBins-i]);
    if(fabs(Pur22Error[NBins-i])>0.9999) Pur22Error[NBins-i] = 0.;
  } // for i

  if(interpolate){
    CostnR[NBins] = 0.;
    Eff22[NBins] = 1.;
    Pur22[NBins] = 0.;
  }

  double *yval = new double [NBins];   //For signal eff. vs. eff.->Y-Achse
  double *yerr = new double [NBins];   //For signal eff. vs. eff.->Y-Achse

  // declare the histograms
  char histoName[20];
  for (int jj=0;jj<5;jj++) {
    sprintf(histoName,"graph%i_%i",jj,num);
    OutputGraph[jj] = new TH1F(histoName,"",NBins,0.-0.5/(double)NBins,1+0.5/(double)NBins);
  }

  // store the Eff and Pur arrays in arrays of pointers
  // in order to shorten the loop over the bins to set
  // the histogram bin contents and to interpolate
  double* eff[5] = {&(Eff11[0]),&(Eff12[0]),&(Eff21[0]),&(Eff22[0]),&(CostnL[0])};

  double* pur[5] = {&(Pur11[0]),&(Pur12[0]), &(Pur21[0]),&(Pur22[0]), &(Eff21[0])};

  double* purError[5] = {&(Pur11Error[0]),&(Pur12Error[0]), &(Pur21Error[0]),&(Pur22Error[0]),
			 &(CostnL[0])}; //the last element is not used

  for (int mm=0;mm<5;mm++){
    for (int i = 0; i<NBins; i++) {
      Float_t x = OutputGraph[0]->GetBinCenter(i+1);;

      yval[i] = 0.;
      yerr[i] = 0.;
      if(mm<4){
 	yval[i] = *(pur[mm]);
 	yerr[i] = *(purError[mm]);
      }

      for (int j=0; j<NBins; j++){
	if((*(eff[mm]+j) >= x) && (*(eff[mm]+j+1) < x)) {
	  yval[i] = *(pur[mm]+j);
	  // initialize error
	  if(mm<4) yerr[i] = *(purError[mm]+j);
	  if(interpolate){
	    // interpolate
	    yval[i] += (x - *(eff[mm]+j) )* (*(pur[mm]+j+1)- *(pur[mm]+j))
	      /(*(eff[mm]+j+1)- *(eff[mm]+j)); 
		      
	    // compute error
	    if(mm<4) yerr[i] += (x - *(eff[mm]+j)) * (*(purError[mm]+j+1)- *(purError[mm]+j)) 
	      / (*(eff[mm]+j+1)- *(eff[mm]+j));
	  }
	}
      }
      if(fabs(yerr[i])>0.95) yerr[i] = 0.;
      OutputGraph[mm]->SetBinContent(i+1,yval[i]);
      if(mm<4) OutputGraph[mm]->SetBinError(i+1,yerr[i]);
    }
  }

  delete [] Eff11;
  delete [] Eff12;
  delete [] Eff21;
  delete [] Eff22;
  delete [] Pur11;
  delete [] Pur12;
  delete [] Pur21;
  delete [] Pur22;
  delete [] Pur11Error;
  delete [] Pur12Error;
  delete [] Pur21Error;
  delete [] Pur22Error;
  delete [] CostnL; 
  delete [] CostnR; 
  delete [] yval;   
  delete [] yerr;   

  return;

} //PurityEfficiency

void writeTitle(double xMin, double xMax,
		double yMin, double yMax,
		const char* text)
{
  // write the title on the side
  TText* t = new TText();
  t->SetTextAngle(-90.01); 
  t->SetTextAlign(12);
  t->SetTextSize(0.16);
  t->SetTextColor(38);
  double xPos = xMax +(xMax-xMin)*0.05;
  double yPos = yMax;
  char modifiedText[30];
  sprintf(modifiedText,"%12s",text);
  t->DrawText(xPos, yPos,modifiedText);
  delete t;
  return;
}

void writeTitle(TH1F* h1,const char* text)
{
  double xMin = h1->GetXaxis()->GetXmin();
  double xMax = h1->GetXaxis()->GetXmax();
  double yMin = h1->GetMinimum();
  double yMax = h1->GetMaximum();

  writeTitle(xMin,xMax,yMin,yMax,text);
  return;
}

void writeTitle(TH2F* h1,const char* text)
{
  double xMin = h1->GetXaxis()->GetXmin();
  double xMax = h1->GetXaxis()->GetXmax();
  double yMin = h1->GetYaxis()->GetXmin();
  double yMax = h1->GetYaxis()->GetXmax();

  writeTitle(xMin,xMax,yMin,yMax,text);
  return;
}

void writeTabValues(TFile *f,int i,int HbookMode)
{
  TH1F* tab = loadHisto(f,1000000+i+1,"",HbookMode);
	  
  if(tab) 
    {
      double xmin = 1.01/50./2.;
      double xmax = 1.01;
      double yPos = 0.0;
      TText* t = new TText();
      t->SetTextAngle(-90.01);
      t->SetTextAlign(13);
      t->SetTextSize(0.055);
      char binText[20];
      const int nBins = 50;
      int count = 0;
      double val=0;
      double xPos=0;
      for(int jj=0;jj<=nBins;jj++)
	{
	  ++count;
	  xPos = xmin + double(jj)/nBins * (xmax-xmin);
	  val = tab->GetBinContent(2*jj+1);
	  sprintf(binText," %.7g", val);
	  TString s(binText);
	  s.Remove(0,1);
	  if (s.Length()>9)
	      sprintf(binText," %.3g", val);
 	  t->DrawText(xPos, yPos,binText);
	}
      delete t;
    }
  return;
}

void writeMapValues(TFile *f,int i,int HbookMode,
		    double yMin, double yMax)
{
  TH1F* tab = loadHisto(f,1500000+i+1,"",HbookMode);
  if(tab) 
    {
      const int nBins = tab->GetXaxis()->GetNbins();
      double xmin = tab->GetXaxis()->GetXmin();
      double xmax = tab->GetXaxis()->GetXmax();
      double yPos = yMin - 0.02*(yMax-yMin);
      TText* t = new TText();
      t->SetTextAngle(-90.01);
      t->SetTextAlign(13);
      t->SetTextSize(0.055);
      char binText[20];
      int count = 0;
      for(int jj=1;jj<=nBins;jj++)
	{
	  ++count;
	  double xPos = xmin + (double(jj)-0.5)/nBins * (xmax-xmin);
	  sprintf(binText," %.5g", tab->GetBinContent(jj));
 	  t->DrawText(xPos, yPos,binText);
	}
      delete t;
    }
  return;
}

void SplinePrepro(TFile *f,int i,int HbookMode,
		  bool decorations=true,const char* text = "")
{
  char hTitle[80];
  int ID = 1100000 + i+1;
  int IDdelta = 1300000 + i+1;
  double deltaspace = 0.05;
  int MonoFlag=0;
  sprintf(hTitle,"var %3i before reg. spline fit",i+1);
  TH1F* h1 = loadHisto(f,ID,hTitle,HbookMode);

  //Get Delta-Histo:
  sprintf(hTitle,"var %i before map prepro", IDdelta);
  TH1F* hDelta = loadHisto(f,IDdelta,hTitle,HbookMode);

  int deltafun = 0;

  if (h1!=0){
    //search for max(binContent+binError) and min(binContent-binError)
    double minH1 = 1000;
    double maxH1 = -1000;
    for(int jj = 0;jj <= h1->GetXaxis()->GetNbins();++jj)
      {
	if(h1->GetBinError(jj) > 0){
	  if(h1->GetBinContent(jj)+h1->GetBinError(jj)>maxH1)
	    maxH1 = h1->GetBinContent(jj)+h1->GetBinError(jj);
	  if(h1->GetBinContent(jj)-h1->GetBinError(jj)<minH1)
	    minH1 = h1->GetBinContent(jj)-h1->GetBinError(jj);}
      }
    double HistoMin = 0.8*minH1;
    if(minH1<0) HistoMin = 1.2*minH1;
    double HistoMax =  1.2*maxH1;

    h1->SetMaximum(HistoMax);
    h1->SetMinimum(HistoMin);
    h1->SetLineColor(1);
    sprintf(hTitle,"node %i spline fit", i+1);
    h1 -> SetTitle("");

    TPaveTextNB *pt1 = new TPaveTextNB(0.1,0.90144,0.9,0.995,hTitle);

    h1 -> SetXTitle("bin n");
    h1 -> SetYTitle("purity");
    h1 -> GetYaxis()->CenterTitle(1);
    if (hDelta!=0) 
      deltafun = 1;
    else 
      deltafun = 0;
    double xmin = h1->GetXaxis()->GetXmin();
    double xmax = h1->GetXaxis()->GetXmax();
    double ymin = h1->GetMinimum();
    double ymax = h1->GetMaximum();
    double ydelta1 = 0;
    double ydelta2 = 0;
    double edelta = 0;
    if (deltafun)
      {
	ydelta1 = hDelta->GetBinContent(1);
        edelta = hDelta->GetBinError(1);

	IDdelta += 100000;
	TH1F* hDelta = loadHisto(f,IDdelta,hTitle,HbookMode);
	
	ydelta2 =  hDelta->GetBinContent(1);
	xmin = xmin - deltaspace*(xmax-xmin);
	if (ydelta1>ymax) ymax = ydelta1;
	if (ydelta1<ymin) ymin=ydelta1;
      }//deltafun

    ymin = ymin - 0.1*(ymax-ymin);
    ymax = ymax + 0.1*(ymax-ymin);

    //"null" in paw
    char histoname[20];
    sprintf(histoname,"frame_%i",i);
    TH2F* frame = new TH2F(histoname,"",100,xmin,xmax,100,ymin,ymax);
    frame->SetTitle(h1->GetTitle());
    frame -> SetXTitle("bin #");
    frame -> SetYTitle("purity");
    frame->GetXaxis()->SetTitleSize(0.08);
    frame->GetXaxis()->SetLabelSize(0.07);
    frame->GetYaxis()->SetTitleSize(0.08);
    frame->GetYaxis()->SetLabelSize(0.07);
    frame->GetYaxis()->CenterTitle(1);
    frame->GetXaxis()->SetTitleOffset(0.8);
    frame->GetYaxis()->SetTitleOffset(0.6);
    frame->DrawCopy();

    //end "null"
    if (deltafun)
      {
	double xdmin = xmin;
	double xdmax = xmin + (xmax-xmin)/(1+deltaspace)*deltaspace;
	
	TPave* box = new TPave(xdmin,ymin,xdmax,ymax,1);
	int ci = TColor::GetColor("#f7d208");
	box->SetFillColor(ci);
	box->Draw("same");
	//end "dbox"
	double xdmid = 0.5*(xdmin+xdmax);
	
	TLine* line = new TLine(xdmin+0.25*(xdmax-xdmin), 
				ydelta1,xdmax-0.25*(xdmax-xdmin),ydelta1);
	TLine* line2 = new TLine(xdmid,ydelta1-edelta,xdmid,ydelta1+edelta);
	line->Draw("same");
	line2->Draw("same");

	double* ini = 0;
	TPolyMarker* marker = new TPolyMarker(1,ini,ini,"");
	setMarkerAttributes(marker,2,0.7,4);
	marker->SetPoint(0,xdmid,ydelta2);
	marker->Draw("same");
      }//deltafun
    
    h1->Draw("same");
    if(decorations) writeTitle(h1,"spline fit");
    if(!decorations && strcmp(text,"")) writeTitle(h1,text);

    //determine number of spline fit iterations

    int niter=0;
    int ID2 = 1201000 + i+1;
    
    sprintf(hTitle,"var %3i after reg. spline fit",i+1);

    TH1F* h2 = loadHisto(f,ID2,hTitle,HbookMode);

    while (h2!=0) 
      {
	niter++;
	ID2 +=1000;
	sprintf(hTitle,"var %2i after reg. spline fit",i+1);

	h2 = loadHisto(f,ID2,hTitle,HbookMode);
      }//while (h2!=0)

    //calculate error band:
    //Montonous spline fit has more than  one iteration => Set MonoFlag==1
    if(niter!=1) {niter=0;MonoFlag=1;} 

    int ID3 = 1200000 + i+1 + 1000*niter;
    sprintf(hTitle,"var %3i after reg. spline fit",i+1);

    TH1F* h3 = loadHisto(f,ID3,hTitle,HbookMode);

    //int IDE1 = 1200000 + i+1 + 1000*(niter+1);

    sprintf(hTitle,"var %3i after reg. spline fit",i+1);
  
    sprintf(histoname,"hE1_%i",i);
    TH1F* hE1 = new TH1F(histoname,hTitle,h3->GetNbinsX(),h3->GetXaxis()->GetXmin(),
			 h3->GetXaxis()->GetXmax());

    //int IDE2 = 1200000 + i+1 + 1000*(niter+2);
    sprintf(hTitle,"var %3i after reg. spline fit",i+1);

    sprintf(histoname,"hE2_%i",i);
    TH1F* hE2 = new TH1F(histoname,hTitle,h3->GetNbinsX(),h3->GetXaxis()->GetXmin(),
			 h3->GetXaxis()->GetXmax());
    TH1F* h3copy = new TH1F("h3dummy",hTitle,h3->GetNbinsX(),h3->GetXaxis()->GetXmin(),
			 h3->GetXaxis()->GetXmax());

    float hval1=0;
    float hval2=0;

    for (int ibin=1;ibin<=h3->GetNbinsX();ibin++)
      {
	hval1 = h3->GetBinContent(ibin)+h3->GetBinError(ibin);
	hval2 = h3->GetBinContent(ibin)-h3->GetBinError(ibin);
	hE1->SetBinContent(ibin,hval1);
	hE2->SetBinContent(ibin,hval2);
	h3copy->SetBinContent(ibin,h3->GetBinContent(ibin));
      }
    h3copy->SetLineColor(2);
    h3copy->DrawCopy("same,l");
    // draw error band only for monotonous spline fit
    if(MonoFlag==0)
    {
    hE1->SetLineStyle(2);
    hE1->SetLineColor(2);
    hE1->DrawCopy("same,l");
    hE2->SetLineStyle(2);
    hE2->SetLineColor(2);
    hE2->DrawCopy("same,l");
    }
    delete hE1;
    delete hE2;
    delete h3copy;
    delete frame;
    delete pt1;
  }

} //SplinePrepro


void MapPrepro(TFile *f,int i,int HbookMode,
	       bool decorations=true,const char* text="")
{
  char hTitle[80];
  sprintf(hTitle," var %i before map prepro",i+1);
  TH1F* h1 = loadHisto(f,1300000+i+1,hTitle,HbookMode);

  if (h1!=0 && h1->GetXaxis()->GetNbins()>1)
    {
      h1->SetLineColor(1);
      sprintf(hTitle,"node %i map", i+1);
      setAxisAttributes(h1,"","class n","purity",0.08,0.08,0,0.6,0.07,0.07);
      
      TPaveTextNB *pt1 = new TPaveTextNB(0.1,0.90144,0.9,0.995,hTitle);
      
      sprintf(hTitle," var %i after map prepro",i+1);

      //search for max(binContent+binError) and min(binContent-binError)
      double minH1 = 1000;
      double maxH1 = -1000;
      for(int jj = 0;jj <= h1->GetXaxis()->GetNbins();jj++)
	{
	  if(h1->GetBinContent(jj)+h1->GetBinError(jj)>maxH1)
	    maxH1 = h1->GetBinContent(jj)+h1->GetBinError(jj);
	  if(h1->GetBinContent(jj)-h1->GetBinError(jj)<minH1)
	    minH1 = h1->GetBinContent(jj)-h1->GetBinError(jj);
	}

      TH1F* h2 = loadHisto(f,1400000+i+1,hTitle,HbookMode);

      for(int jj = 0;jj <= h2->GetXaxis()->GetNbins();jj++)
	{
	  if(h2->GetBinContent(jj)>maxH1)
	    maxH1 = h2->GetBinContent(jj);
	  if(h2->GetBinContent(jj)<minH1)
	    minH1 = h2->GetBinContent(jj);
	}

      double HistoMin = minH1>0 ? 0.8*minH1 : 1.2*minH1;
      //      if(minH1<0) HistoMin = 1.2*minH1,-2.;
      double HistoMax =  1.2*maxH1;
      
      setMarkerAttributes(h2,2,1.0,4);
      h2->SetLineColor(2);
      h1->GetXaxis()->SetLabelColor(10);
      h1->SetMinimum(HistoMin);
      h1->SetMaximum(HistoMax);
      h1->Draw();
      h2->Draw("same,p");
      writeMapValues(f,i,HbookMode,HistoMin,HistoMax);

      if(decorations)writeTitle(h1,"map");
      if(!decorations && strcmp(text,"")) writeTitle(h1,text);

      delete pt1;
    }
  return;
}
// implementation for preprocessing 92
void GaussPrepro(TH1F* h1, TH1F* hPurity,int i,
		 bool decorations=true,const char* text = "")
{
  // plot purity=signal/background + delta,
  // where the delta function is contained in h1 already
  double deltaspace = 0.05;

  //in analyse3.kumac it is done like this:
  double hmin = hPurity->GetMinimum();
  double HistoMin = 0.0;
  if (hmin>=0) HistoMin = 0.9*hmin;
  else HistoMin = 1.1*hmin;

  h1->SetMinimum(HistoMin);
  h1 -> SetTitle("");

  double xmin = hPurity->GetXaxis()->GetXmin();
  double xmax = hPurity->GetXaxis()->GetXmax();
  double ymin = hPurity->GetMinimum();
  double ymax = hPurity->GetMaximum();

  double ydelta = h1->GetBinContent(1);
  double edelta = h1->GetBinError(1);
      
  xmin = xmin - deltaspace*(xmax-xmin);
  if (ydelta>ymax) ymax = ydelta;
  if (ydelta<ymin) ymin = ydelta;

  ymin = ymin - 0.1*(ymax-ymin);
  ymax = ymax + 0.1*(ymax-ymin);

  hPurity->SetMinimum(ymin);
  hPurity->SetMaximum(ymax);

  //"null" in paw
  TH2F* frame = new TH2F("frame","",100,xmin,xmax,100,ymin,ymax);
  frame->SetTitle(h1->GetTitle());
  frame -> SetXTitle("bin n");
  frame -> SetYTitle("purity");
  frame->GetXaxis()->SetTitleSize(0.08);
  frame->GetXaxis()->SetLabelSize(0.07);
  frame->GetYaxis()->SetTitleSize(0.08);
  frame->GetYaxis()->SetLabelSize(0.07);
  frame->GetXaxis()->SetTitleOffset(0.8);
  frame->GetYaxis()->SetTitleOffset(0.6);
  frame->GetYaxis()->CenterTitle(1);
  frame->DrawCopy();
  if(decorations)writeTitle(frame,"purity");
  if(!decorations && strcmp(text,"")) writeTitle(frame,text);
  //end "null"
      
  hPurity->Draw("same");

  double xdmin = xmin;
  double xdmax = xmin + (xmax-xmin)/(1+deltaspace)*deltaspace;
      
  TPave* box = new TPave(xdmin,ymin,xdmax,ymax,1);
  int ci = TColor::GetColor("#f7d208");
  box->SetFillColor(ci);
  box->Draw("same");
  //end "dbox"
  double xdmid = 0.5*(xdmin+xdmax);
      
  TLine* line = new TLine(xdmin+0.25*(xdmax-xdmin), ydelta,xdmax-0.25*(xdmax-xdmin),ydelta);
  TLine* line2 = new TLine(xdmid,ydelta-edelta,xdmid,ydelta+edelta);
  line->Draw("same");
  line2->Draw("same");

  delete frame;
  return;
} //GaussPrepro

void distribsingle5(TH1F* hSig, TH1F* hBG,int i,bool decorations=true)
{
  Float_t yMax=0.0;
  Float_t yMin=0.0;
  char hTitle[80];
  if (hSig->GetMaximum() > hBG->GetMaximum())
    yMax = hSig->GetMaximum();
  else
    yMax = hBG->GetMaximum();
  yMax = 1.1*yMax;
  Float_t xMin = hSig->GetXaxis()->GetXmin();
  Float_t xMax = hSig->GetXaxis()->GetXmax();
  int deltafun = 0;
  if (hBG->GetBinContent(0)>0){
    deltafun = 1;
  }

  Float_t deltaspace = -1;
  if (deltafun){
    deltafun = 1;
    deltaspace = 0.05;
    xMin = xMin - deltaspace * (xMax-xMin);
  }

  //"null" in paw"
  char histoname[20];
  sprintf(histoname,"frame_%i",i);
  TH2F* frame = new TH2F(histoname,"",100,xMin,xMax,100,yMin,yMax);

  //   sprintf(hTitle,"equalised input node %i", i+1);
  ///sprintf(hTitle,"input node %i", i+1);
  ///TPaveTextNB *pt1 = new TPaveTextNB(0.1,0.85144,0.9,0.995,hTitle);

  frame -> SetXTitle("");
  frame -> SetYTitle("events");
  frame->GetXaxis()->SetLabelSize(0.07);
  frame->GetYaxis()->SetLabelSize(0.07);
  frame->GetYaxis()->CenterTitle(1);
  frame->GetYaxis()->SetTitleSize(0.08);
  frame->GetYaxis()->SetTitleOffset(0.6);
  frame->GetXaxis()->SetTitleSize(0.08);
  frame->GetXaxis()->SetLabelSize(0.07);
  frame->GetXaxis()->SetLabelColor(10);
  frame->DrawCopy();
  //end "null" 

  hBG->SetMaximum(yMax);
  hBG->SetTitle("");
  hBG->SetLineWidth(1);
  hBG->SetLineColor(1);    
  hBG->DrawCopy("histsame"); 
  hSig -> SetMaximum(yMax);
  hSig->SetTitle("");
  hSig -> SetLineWidth(1);
  hSig -> SetLineColor(2);
  hSig -> DrawCopy("histsame");
///  if(decorations) pt1->Draw();

  if (deltafun){
    double xdmin = xMin;
    double xdmax = xMin + (xMax-xMin)/(1+deltaspace)*deltaspace;
    TPave* box = new TPave(xdmin,yMin,xdmax,yMax,1); 
    int ci = TColor::GetColor("#f7d208");
    box->SetFillColor(ci);
    box->Draw();
    int numbers = int(hSig->GetBinContent(0) + hBG->GetBinContent(0));
    sprintf(hTitle,"+ #delta %i", numbers);
    double xtext = (xdmin-xdmax)*0.35;
    double ytext = (yMax-yMin)*0.15;
    TLatex* tex = new TLatex(xtext,ytext,hTitle);
    tex->SetTextSize(0.12);
    tex->SetTextColor(9);
    tex->SetTextFont(62);
    tex->SetTextAngle(90);
    tex->Draw();
  }	  

  if(decorations)writeTitle(frame,"flat");

  delete frame;
//  delete pt1;
}

//plot correlation matrix
void correlations(TFile* f,int HbookMode,int histo_id=2000){


  gStyle->SetFrameLineWidth(2);
  double split = 0.05;


  TPad* PadCorr=new TPad("PadFit","The first subpad",0.,split,1.,1.);


  PadCorr->Draw();
  PadCorr->cd();
  char hTitle[80];
  TH2F* h1  = loadHisto2D(f,histo_id,"correlation matrix",HbookMode);

  TPaveTextNB *pt1 = new TPaveTextNB(0.1,0.88144,0.9,0.985,"correlation matrix of input variables");

  //define color palette
  const int colNum = 40-7;
  int palette[colNum];


  if (h1!=0){
    float col = -999.;
    //white -> cyan
    int n2 = 24;
    int n1 = 20;
    for (int icol=n2-1;icol>n1;icol--) {
      col = 0 + (icol-n1)/Float_t(n2-n1);
      if(!gROOT->GetColor(300+icol)) {
	TColor *color = new TColor(300+icol,col,1,1,"");
	color->GetNumber(); //dummy request to avoid warning about unused variable
      }
      
      palette[icol-8] = 300+icol;
    }
    //cyan -> blue
    n2 = 20;
    n1 = 12;
    for (int icol=n2;icol>n1;icol--) {
      col = 0 + 0.9*(icol-n1)/Float_t(n2-n1);
      if(!gROOT->GetColor(300+icol)) {
	TColor *color = new TColor(300+icol,0,col,1,"");
	color->GetNumber(); //dummy request to avoid warning about unused variable
      }
      palette[icol-8] = 300+icol;
    }
    //blue -> dark blue
    n2 = 12;
    n1 = 8;
    for (int icol=n2;icol>=n1;icol--) {
      col = 0.5 + 0.4*(icol-n1)/Float_t(n2-n1);
      if(!gROOT->GetColor(300+icol)) {
	TColor *color = new TColor(300+icol,0,0,col,"");
	color->GetNumber(); //dummy request to avoid warning about unused variable
      }
      
      palette[icol-8] = 300+icol;
    }
    //white -> yellow
    n1 = 24;
    n2 = 28;
    for (int icol=n1;icol<n2;icol++) {
      col = 1 - (icol-n1)/Float_t(n2-n1);
      if(!gROOT->GetColor(300+icol)) {
	TColor *color = new TColor(300+icol,1,1,col,"");
	color->GetNumber(); //dummy request to avoid warning about unused variable
      }
      
      palette[icol-8] = 300+icol;
    }
    //yellow -> red
    n1 = 28;
    n2 = 36;
    for (int icol=n1;icol<n2;icol++) {
      col = 0.9 - 0.9*(icol-n1)/Float_t(n2-n1);
      if(!gROOT->GetColor(300+icol)) {
	TColor *color = new TColor(300+icol,1,col,0,"");
	color->GetNumber(); //dummy request to avoid warning about unused variable
      }
      
      palette[icol-8] = 300+icol;
    }
    //red -> dark red
    n1 = 36;
    n2 = 40;
    for (int icol=n1;icol<=n2;icol++) {
      col = 0.9 - 0.4*(icol-n1)/Float_t(n2-n1);
      if(!gROOT->GetColor(300+icol)) {
	TColor *color = new TColor(300+icol,col,0,0,"");
	color->GetNumber(); //dummy request to avoid warning about unused variable
      }
      
      palette[icol-8] = 300+icol;
    }

    gStyle->SetPalette(colNum,palette);

    h1->SetTitle("");
  
    //print axis values
    TAxis* x = h1->GetXaxis();
    TAxis* y = h1->GetYaxis();
    TAxis* z = h1->GetZaxis();
    z->SetRangeUser(-1.,1.);
    z->SetLabelSize(0.028);
    x->SetTickLength(0);
    y->SetTickLength(0);
    z->SetTickLength(0);
    int nxbins = x->GetNbins();
    int nybins = y->GetNbins();
    y->SetLabelFont(102);
    x->SetLabelFont(102);
    if(nxbins>50)
    {
	x->SetLabelSize(0.02);
	y->SetLabelSize(0.02);
    }

    for (int ibin=1;ibin<=nxbins;ibin++){
      int xlabel = ibin;
      sprintf(hTitle,"%i", xlabel);
      x->SetBinLabel(ibin,hTitle);
      x->LabelsOption("v");
    }

    for (int ibin=1;ibin<=nybins;ibin++){
      int ylabel = nybins+1-ibin;
      sprintf(hTitle,"%i", ylabel);
      y->SetBinLabel(ibin,hTitle);
    }
    gStyle->SetOptStat(0);
    gStyle->SetNumberContours(colNum);

    float xmax = x->GetXmax();
    float xmin = x->GetXmin();
    h1->Draw("col");
    
    //black boxes for diagonal elements
    for (int i=1;i<=nxbins;i++){
      float x1 = xmin + (i-1)/static_cast<float>(nxbins)*(xmax-xmin);
      float x2 = xmin + (i)/static_cast<float>(nxbins)*(xmax-xmin);
      float y1 = xmax - (i)/static_cast<float>(nxbins)*(xmax-xmin);
      float y2 = xmax - (i-1)/static_cast<float>(nxbins)*(xmax-xmin);
      TBox* box = new TBox(x1,y1,x2,y2);
      box->SetFillColor(1);
      box->Draw("same");
    }
  }
  else {
    TLatex* tex = new TLatex(0.1,0.8,"This Plot is not implemented in the"); 
    TLatex* tex2 = new TLatex(0.1,0.75,"NeuroBayes version you are using!"); 
    TLatex* tex3 = new TLatex(0.15,0.4,"Switch to the newest version!");
    tex3->SetTextColor(2);
    tex->Draw();
    tex2->Draw();
    tex3->Draw();
  }
  pt1->Draw("same");


  delete pt1;



  // Draw Color Pallette by "hand"
  TPad* PadRes=new TPad("PadRes","The second subpad",0.,0.,1.,split-0.025);
  PadRes->SetBorderSize(0);
  PadRes->Draw();
  PadRes->cd();
  
  float xmin = 0.1;
  float ymin = 0.7;
  float xmax = 0.9;
  TGaxis *axis1 = new TGaxis(xmin,ymin,xmax,ymin,-1,1,510,"W");
  axis1->SetName("axis1");
  axis1->SetLabelOffset(0.005);
  axis1->SetLabelSize(0.8);
  axis1->SetLabelFont(102);
     for (int i=1;i<=colNum;i++){
       float x1 = xmin + (i-1)/static_cast<float>(colNum)*(xmax-xmin);
       float x2 = xmin + (i)/static_cast<float>(colNum)*(xmax-xmin);
       float y1 = ymin;
       float y2 = ymin+50*(x2-x1);
       TBox* box = new TBox(x1,y1,x2,y2);
       box->SetFillColor(palette[i-1]);
       box->Draw("same");
       }
     axis1->Draw();
     //     canvas->Update();
     gStyle->SetFrameLineWidth(1);
  return;
}

TPad* printGlobalTitle(TCanvas* canvas,bool isPdf,
    const char* correlSigniFile,const char heading[] = NULL)
{
  //create pad for title
  double shift = 0;
  ifstream significanceList(correlSigniFile);
  if(!significanceList)
    { 
      shift = 0.3;
      significanceList.close();
    }

  TPad *title =0;
  if(isPdf)
//    title = new TPad("title", "title", 0.07+shift,0.82,0.41+shift,0.92);
      title = new TPad("title", "title", 0.,0.8,0.99,0.99);
  else
      title = new TPad("title", "title", 0.,0.8,0.99,0.99);
//    title = new TPad("title", "title", 0.07+shift,0.89,0.41+shift,0.99);

  title->Draw();
  title->cd();
  title->SetBorderSize(2);
  title->SetFillColor(10);
  title->SetFrameFillColor(0);

  //draw phi-T-logo "by hand"
  TLatex* logo = new TLatex(0.855,0.89,"  Phi-T  ");
  logo->SetTextFont();
  logo->SetTextSize(0.15);
  logo->SetLineWidth(2);
  logo->Draw();
  TLine *line = new TLine(0.89,0.878,0.96,0.878);
  line->SetLineColor(2);
  line->SetLineWidth(3);
  //  line->Draw();
  
  TLatex *   tex = new TLatex(0.86,0.81,"NeuroBayes^{#oright} Teacher");
  tex->SetTextFont();
  tex->SetTextSize(0.05);
  tex->SetLineWidth(2);
  tex->Draw();
  if(heading!=NULL) {
    TPaveText* title_tpt= new TPaveText(0.2,0.89,0.8,1.0);
    title_tpt->SetBorderSize(1);
    title_tpt->SetFillColor(10);
    title_tpt->AddText(heading);
    title_tpt->Draw();
  }
  canvas->cd();   
  TPad* graphPad = 0;
  if(isPdf)
    graphPad = new TPad("Graphs","Graphs",0.,0.,1.,0.95);
  else
    graphPad = new TPad("Graphs","Graphs",0.,0.,1.,0.95);  
  graphPad->Draw();
  graphPad->cd();
  graphPad->SetFillColor(10);

  return graphPad;
}//void printOldGlobalTitle

//Plot of the network architecture
void netplot(TFile* f, int HbookMode)
{ 
  char nbText[80];
  int nodes[3] = {0,0,0};

  //retrieve number of nodes in the first layer: nodes[0]
  int ID = 1011001;
  TH1F* h1 = loadHisto(f,ID,"Check: Input variables",HbookMode);

  while (h1!=0 && h1->GetEntries()>0)
    {
      nodes[0]++;
      h1 = loadHisto(f,++ID,"Check: Input variables",HbookMode);
    }

  //retrieve number of nodes in the 3rd layer: nodes[2]
  ID = 101;
  h1 = loadHisto(f,ID,"F(NETOUT) FOR BACKGROUND",HbookMode);

  while (h1!=0)
    {
      nodes[2]++;
      h1 = loadHisto(f,++ID,"F(NETOUT) FOR BACKGROUND",HbookMode);
    }

  //retrieve number of nodes in 2nd layer: nodes[1]
  h1 = loadHisto(f,3001,"Hesse ordered",HbookMode);

  int nn = h1->GetNbinsX();
  nodes[1] = nn/(nodes[0]+nodes[2]);

  //print number of nodes:
  cout << endl << "***************Training results*****************" << endl;
  cout << "number of nodes in the 1st layer: " << nodes[0] << endl;
  cout << "number of nodes in the 2nd layer: " << nodes[1] << endl;
  cout << "number of nodes in the 3rd layer: " << nodes[2] << endl;

  //retrieve number of iterations:
  h1 = loadHisto(f,100,"ERROR LEARNSAMPLE",HbookMode);

  int niter = h1->GetNbinsX();
  cout << "number of iterations: " << niter << endl;

  //get contents from last significance histogram:
  nn=niter;
  h1=loadHisto(f,6000+nn,"sign. as is",HbookMode);
  if(h1==NULL) {
    nn=1;
    cerr<<"Warning, histogram of network weights for iteration "<<niter<<" not found."<<endl;
    cerr<<"Trying to load weights of first iteration"<<endl;
    h1=loadHisto(f,6000+nn,"sign. as is",HbookMode);
  }

  if(h1!=NULL) {
    const int nbins = h1->GetNbinsX();

    auto hsigni = std::vector<double>(nbins);
    for (int i=0;i<nbins;i++)
      {
	hsigni[i] = h1->GetBinContent(i+1); //0 is overflow, nbins+1 is overflow
      }

    //int jlayer=-1;
    double signimaxl[3] = {0,0,0};
    double signilji[3][500][500];
    for (int ilayer=1;ilayer<=2;ilayer++)
      {
	int nkni = nodes[ilayer-1];
	int nknj = nodes[ilayer];
	if (nkni>500 || nknj>500) 
	  {
	    cout << "Array too small! Abort ..." << endl;
	    return;
	  }
	int offset = 0;
	if (ilayer==1) 
	  offset = 0;
	else if (ilayer==2) 
	  offset = nodes[0]*nodes[1];

	for (int inode=1;inode<=nkni;inode++)
	  {
	    for (int jnode=1;jnode<=nknj;jnode++)
	      {
		int index = offset + (jnode-1)*nkni+inode;
		double signi = hsigni[index-1];
		signilji[ilayer-1][jnode-1][inode-1] = signi;
		if (signi>signimaxl[ilayer-1]) signimaxl[ilayer-1] = signi;
	      }//(for (int jnode...)
	  }//(for (int inode...)
      }//for (int ilayer...)

    //draw network:
    //....frame:
    TH2F* frame = new TH2F("frame","",100,0.,1.,100,0.,1.);
    frame->GetXaxis()->SetNdivisions(0);
    frame->GetYaxis()->SetNdivisions(0);
    frame->Draw();
    //....connections:
    double xpos[3];
    xpos[0] = 0.1;
    xpos[1]= 0.5;
    xpos[2] = 0.9;
    for (int ilayer=1;ilayer<=2;ilayer++)
      {
	int nkni = nodes[ilayer-1]; 
	int nknj = nodes[ilayer];
	double xposi = xpos[ilayer-1];
	double xposj = xpos[ilayer];
	for (int inode=1;inode<=nkni;inode++)
	  {
	    for (int jnode=1;jnode<=nknj;jnode++)
	      {
		double yposi = 1 - (2*inode-1)/(2.0*nkni);
		double yposj =  1 - (2*jnode-1)/(2.0*nknj);

		if(signimaxl[ilayer-1]<=0 && signilji[ilayer-1][jnode-1][inode-1]>0) 
		  signimaxl[ilayer-1] = signilji[ilayer-1][jnode-1][inode-1];
		else if(signilji[ilayer-1][jnode-1][inode-1]<=0)
		  signimaxl[ilayer-1] = 1;

		double signirat = signilji[ilayer-1][jnode-1][inode-1]/signimaxl[ilayer-1];
		double col = 1-signirat;
		if (col!=1)
		  {
		    //get the first available integer for the color!
		    int colNum=((TObjArray*)gROOT->GetListOfColors())->GetLast()+1;
		    TColor* color = new TColor(colNum,col,col,col,"");
		    color->GetNumber(); //dummy request to avoid warning about unused variable
		    TLine* weight = new TLine(xposi,yposi,xposj,yposj);
		    weight->SetLineColor(colNum);
		    weight->Draw("same");
		  }
	      }
	  }
      }
  
    //....nodes
    for (int ilayer=1;ilayer<=3;ilayer++)
      { 
	double xposilayer = xpos[ilayer-1];
	int nkn = nodes[ilayer-1];
	for (int inode=1;inode<=nkn;inode++)
	  {
	    double ypos = 1 - (2*inode-1)/(2.0*nkn);
	    double* dum = 0;
	    TPolyMarker* marker = new TPolyMarker(1,dum,dum,"");
	    setMarkerAttributes(marker,9,0,20);
	    marker->SetPoint(0,xposilayer,ypos);
	    marker->Draw("same");
	    if (ilayer==1||ilayer==3)
	      {
		double tx = 0;
		if (ilayer==1) tx = xposilayer - 0.05;
		if (ilayer==3) tx = xposilayer + 0.02;
		sprintf(nbText,"%i",inode);
		TText* text = new TText(tx,ypos-0.01,nbText);
		text->SetTextSize(0.02);
		text->Draw("same");
	      }
	  }
	TText* layer = NULL;
	if (ilayer==1) 
	  layer = new TText(xposilayer-0.07,-0.05,"input layer");
	else if (ilayer==2)
	  layer = new TText(xposilayer-0.07,-0.05,"hidden layer");
	else if (ilayer==3)
	  layer = new TText(xposilayer-0.07,-0.05,"output layer");

	layer->SetTextSize(0.03);
	layer->Draw("same");
      }
     char text_heading[50];
     sprintf(text_heading,"Network at iteration %i",niter);
     TText* heading = new TText((frame->GetXaxis()->GetXmax()-frame->GetXaxis()->GetXmin())/2.-0.25,frame->GetYaxis()->GetXmax()+0.05,text_heading);
     heading->SetTextSize(0.04);
     heading->Draw("same");
  }
  return;
}



void plotNetOutput(TH1F* hSig,TH1F* hBG,int index,int Nodes,string draw_option="",int color_sig=2,int color_back=1,bool draw_line=true)
{

  //now plot network output (signal+bg) and purity vs. netoutput
  float delta = 2.0/((float) Nodes); //step width (2 = net output range from -1 to +1)
  double xpos = 1.0 -(index-0.5)*delta;
      
  float Max = 0;
  if (hSig->GetMaximum() > hBG->GetMaximum())
    Max = hSig->GetMaximum();
  else
    Max = hBG->GetMaximum();
  Max = 1.1*Max;
      
  hSig -> Sumw2();    // have correct error treatment:
  hBG->Sumw2();    //
  // if histogram is already filled, 
  // the sum of squares of weights
  // is filled with the existing bin contents
  // The error per bin will be computed as 
  // sqrt(sum of squares of weight) for each bin.
  // if switched on here, the netoutput for signal and
  // background will be plotted as points with error bars
  hSig->SetMaximum(Max);
  hBG ->SetMaximum(Max);

  setAxisAttributes(hSig,"","Network output","events",0,0,0,1.2);
  hSig->SetLineWidth(1);
  hSig->SetLineColor(color_sig);
  hBG->SetLineWidth(1);
  hBG->SetLineColor(color_back);    
  hSig->Draw(("hist"+draw_option).c_str());
  //  gPad->SetLogy();   
  hBG->Draw("samehist");
  //  gPad->SetLogy(); 
  // labels and lines
  char title[1000];
  sprintf(title,"Output Node %i (n = %g  s = %g  b = %g  p_{sig} = %1.4f)", index,hSig->GetSum()+hBG->GetSum(),hSig->GetSum(),hBG->GetSum(),hSig->GetSum()/(hSig->GetSum()+hBG->GetSum()));
  TPaveTextNB *pt = new TPaveTextNB(0.2,0.90144,0.8,0.965,title);
  pt->Draw("same");
  
  TLine* line1 = new TLine(xpos,0.0,xpos,Max);
  line1->SetLineColor(4);
  if(draw_line) {
    line1->Draw(); 
  }
  return;
}

void plotPurity(TH1F* hpur,TH1F* hsum,int index,int Nodes,string draw_option = "",int color=1)
{

  hpur->Divide(hpur,hsum,1,1,"B");
  setAxisAttributes(hpur,"","Network output","purity");
  hpur->SetMaximum(1.02);
  hpur->SetMinimum(-0.02); 
  hpur->SetMaximum(1.);
  hpur->SetMinimum(0.0); 
  hpur->SetLineColor(color);
  hpur->SetMarkerColor(color);
  //Calculate error assuming binomial statistics and prior with expectation value equal to bin purity
  double k=0;
  double n=0;
  double varianz=0;
  double p_bin=0;
  for(int i = 1;i<=hpur->GetNbinsX();i++) {
    n=hsum->GetBinContent(i);
    if(n!=0)
      {
    k=n*hpur->GetBinContent(i);
    p_bin=(hpur->GetBinCenter(i)+1)/2;
    varianz=((p_bin+k)*(1-p_bin+n-k))/((n+2)*(n+1)*(n+1));
    //    varianz=((n-k+1)*(k+1))/((n+3)*(n+2)*(n+2));
    //    cout<<"Bincenter: "<<(hpur->GetBinCenter(i)+1)/2<<" i: "<<i<<endl;
    //cout<<"k: "<<k<<" n: "<<n<<" var: "<<varianz<<endl;
    hpur->SetBinError(i,sqrt(varianz));
    hpur->SetBinContent(i,(p_bin+k)/(1+n));
      }
    else
      //Do not show bins with 0 events
      {
	hpur->SetBinError(i,0);
	hpur->SetBinContent(i,0);
      }

  }

  hpur->Draw(("pe0"+draw_option).c_str());
  
  float delta = 2.0/((float) Nodes); //step width (2 = net output range from -1 to +1)
  double xpos = 1.0 -(index-0.5)*delta;

  TLine* lineZero = new TLine(-1.0,0.0,1.0,0.0);  // red line at y=0
  //  lineZero->SetLineStyle(3);
//  lineZero->SetLineColor(14);
  lineZero->SetLineColor(2);


  TLine* lineOne = new TLine(-1.0,1.0,1.0,1.0);  // red line at y=1
  //  lineOne->SetLineStyle(3);
//  lineOne->SetLineColor(14);
  lineOne->SetLineColor(2);


  TLine* line2 = new TLine(-1.0,0.0,1.0,1.0);  // this is the ideal output
  line2->SetLineColor(4);
  line2->Draw(); 
  
  // now plot line for current position
  TLine* line3 = new TLine(xpos,0.0,xpos,1.0);
  line3->SetLineColor(4);
  line3->Draw(); 
  
  double ypos = (xpos+1.0)/2.0;
  TLine* line4 = new TLine(-1.0,ypos,1.0,ypos);
  line4->SetLineColor(4);
  line4->Draw();    
  lineZero->Draw(); 
  lineOne->Draw(); 
  hpur->Draw("pe0same");
  return;
}

void plotIndividualPurEff(TFile* f,int index,bool HbookMode)
{
  // prepare purity/efficiency plots
  TH1F* hSigPass = loadHisto(f,1024000+index+1,"Check: Input variables",HbookMode); 
  TH1F* hBGPass = loadHisto(f,1014000+index+1,"Check: Input variables",HbookMode);
  
  TH1F *PurEff[5];
  if (hBGPass->GetEntries() == 0 || hSigPass->GetEntries() == 0) {
    cout << "WARNING in analysis.C!" << endl;
    cout << " Function ";
    cout << "\'plotIndividualPurEff(TFile* f,int index,bool HbookMode)\'";
    cout << " can not compute purity efficiency for input node " << index+1;
    cout << " since at least one of the histograms ";
    cout << hSigPass->GetName() << " and " <<  hBGPass->GetName();
    cout << " is empty!" << endl;
    return;
  }
  PurityEfficiency(hBGPass, hSigPass, PurEff,0,index+1000);
  
  PurEff[2]->SetLineColor(1);
  setMarkerAttributes(PurEff[2],1,0.5,1);
  PurEff[2]->DrawCopy("psame");
  
  PurEff[3]->SetLineColor(1);
  setMarkerAttributes(PurEff[3],1,0.5,1);
  PurEff[3]->DrawCopy("psame");   

  return;
}  

void plotIndividualPurEff(TFile* f,int index,TH1F* hSignal,TH1F* hBackground,
			  bool HbookMode)
{
  TH1F* hSigPass = 0;
  TH1F* hBGPass = 0;
  if( hSignal!=0 && hBackground!=0)
    {
      hSigPass = (TH1F*)hSignal->Clone("hSigPass");	
      hBGPass = (TH1F*)hBackground->Clone("hBGPass");
    }

  TH1F *PurEff[5];
  PurityEfficiency(hBGPass, hSigPass, PurEff,0,index+1000);
  
  PurEff[2]->SetLineColor(1);
  setMarkerAttributes(PurEff[2],1,0.5,1);
  PurEff[2]->DrawCopy("psame");
  
  PurEff[3]->SetLineColor(1);
  setMarkerAttributes(PurEff[3],1,0.5,1);
  PurEff[3]->DrawCopy("psame");   
  
  return;
}  

void plotTargetDistribution(TFile* f, TPad* graphPad,
			    bool HbookMode)
{
  TH2F* cumulativeTarget = loadHisto2D(f,493,"",HbookMode);
  TH1F* splineFitHisto = loadHisto(f,497,"",HbookMode);
  TH1F* splineDerivativeHisto = loadHisto(f,496,"",HbookMode);

  // these plots are available only with NB teacher>20051010
  if( cumulativeTarget!=0 && splineFitHisto!=0) {
    graphPad->Divide(1,2);
    graphPad->GetPad(1)->cd();
    graphPad->GetPad(1)->SetGridx();
    graphPad->GetPad(1)->SetGridy();
  } else   {
    graphPad->cd();
    graphPad->SetGridx();
    graphPad->SetGridy();
  }

  TH1F* targetHisto = loadHisto(f,1000,"",HbookMode);
  double ymin = targetHisto->GetMinimum();
  double ymax = targetHisto->GetMaximum();

  ymax = 1.3*(ymax-ymin); 
  targetHisto->SetMaximum(ymax);
  if(cumulativeTarget==0) setAxisAttributes(targetHisto,"","","",0,0,0,0,0.02,0.02);
  targetHisto->SetFillColor(8);
  targetHisto->SetTitle("");
  targetHisto->Draw();
  double norm = targetHisto->Integral()/splineDerivativeHisto->Integral();
  splineDerivativeHisto->Scale(norm);
  splineDerivativeHisto->SetLineWidth(2);
  splineDerivativeHisto->Draw("samel");

  TPaveTextNB *pt1 = new TPaveTextNB(0.33,0.89144,0.67,0.995,"distribution of target");

  double xmin = targetHisto->GetXaxis()->GetXmin();
  double xmax = targetHisto->GetXaxis()->GetXmax();
  double xPos = xmin+0.05*(xmax-xmin);
  double yPos = ymin+0.93*(ymax-ymin);

  double textSize = 0.055;
  if(cumulativeTarget==0) textSize = 0.04;

  // get the min and max from histogram 1000001
  TH1F* extremeHisto = loadHisto(f,1000001,"",HbookMode);
  if(extremeHisto)   {
    xmin = extremeHisto->GetBinContent(1);
    xmax = extremeHisto->GetBinContent(101);
  } else {
    xmin = -999.;
    xmax = -999.;
  }
  char titleText[50];
  sprintf(titleText,"lowest target value = %.3f",xmin);
  TLatex* tex1 = new TLatex(xPos,yPos,titleText);
  tex1->SetTextSize(textSize);
  tex1->SetTextFont(42);
  tex1->Draw();
  sprintf(titleText,"highest target value = %.3f",xmax);
  TLatex* tex2 = new TLatex(xPos,0.9*yPos,titleText);
  tex2->SetTextSize(textSize);
  tex2->SetTextFont(42);
  tex2->Draw();
  pt1->Draw();

  if(cumulativeTarget!=0 && splineFitHisto!=0)  {
    // draw spline fit to cumulative target distribution
    // this plot is available only with NB teacher>20051010
    const int nColors = 100;
    int palette[nColors];
    for (int nc = 0;nc<nColors;++nc) palette[nc] = 8; // set all to green for this plot
    gStyle->SetPalette(nColors,palette);

    graphPad->GetPad(2)->cd();
    graphPad->GetPad(2)->SetGridx();
    graphPad->GetPad(2)->SetGridy();
    setMarkerAttributes(cumulativeTarget,8,1.0,7);
    cumulativeTarget->SetTitle("");
    cumulativeTarget->SetMaximum(2);
    cumulativeTarget->SetMinimum(0.);
    cumulativeTarget->Draw("col");
    splineFitHisto->Draw("samel");
    TPaveTextNB *pt = new TPaveTextNB(0.15,0.89144,0.85,0.995,
				      "spline fit to cumulative target distribution");
    pt->Draw();
  }
  return;
}

double computeIntegral(TH1F* h){

  double integral = 0;
  const int nBins = h->GetNbinsX();
  for(int nn = 1;nn<=nBins;++nn){
    integral += h->GetBinWidth(nn) * h->GetBinContent(nn);
  }
  return integral;
}

//Svenja: struct to store information from correl_signi.txt
struct myCorrelSigni{
  bool is_signi;
  int index;
  float position;
  float addSigni;
  float aloneSigni;
  float loss;
  float correlation;
  char varName[50];
  char prepro[100];
};

void bubblesort(std::vector<myCorrelSigni> & v, int n, string sortVal) {

  for (int i = n - 1; i >= 0; i--) {
    // innere Schleife abhängig von i (Korrelation)
    for (int j = 0; j < i; j++) {
      // ggf. Korrektur der Ordnung zweier,
      // aufeinanderfolgender Werte
      float val1 = 0;
      float val2 = 0;
      if (sortVal == "index"){
	val1 = v[j].index;
	val2 = v[j+1].index;
      }
      else if (sortVal == "position"){
	val1= v[j].position;
	val2 = v[j+1].position;
      }
      else if (sortVal == "aloneSigni"){
	val2= v[j].aloneSigni;
	val1 = v[j+1].aloneSigni;
      }
      else {
	cout << "Sorting criterium not implemented! Please check!" << endl;
	return;
      }
      if (val1 > val2) {
        // ein kleiner Bubble-Schritt (Vertauschung)
        myCorrelSigni swp = v[j];
        v[j] = v[j + 1];
        v[j + 1] = swp;
      }
    }
  }
} 

void plot_diag_fits(int ihisto,int outputNodes,TPad* graphPad,
    TCanvas* canvas,bool pdfOutput,TFile* f,bool HbookMode,
    const char* correlSigniFile,TVirtualPS* PSFile,bool interactive) {
  int pages;
  if(pdfOutput) canvas->Clear();
  if (outputNodes>=1)
    {
      TText pt;
      int nodesperpage=outputNodes;
      if(outputNodes>1)nodesperpage=20;
      //      cout<<"Number of Outputnodes: "<<outputNodes<<endl;
      pages = int(ceil(float(outputNodes)/nodesperpage));
      for (Int_t j=0; j< pages; ++j) {
	//      cout<<"page "<<j<<endl;

	if(ihisto == 2110000) graphPad=printGlobalTitle(canvas,pdfOutput,correlSigniFile,"Final Diag Fit");
	else graphPad=printGlobalTitle(canvas,pdfOutput,correlSigniFile,"First Round Diag Fit");

	if(outputNodes==1)
	  graphPad->Divide(1,1,0,0);
	else
	  graphPad->Divide(4,5,0,0);
	
	if (outputNodes==1)
	  j++;

	for (Int_t i=1+j*nodesperpage; i<= (j+1)*nodesperpage; ++i) {

	  graphPad->GetPad(i-j*nodesperpage)->cd();
	  graphPad->GetPad(i-j*nodesperpage)->SetGridx(1);
	  graphPad->GetPad(i-j*nodesperpage)->SetGridy(1);

	  int IDh1 = ihisto+i;
	  int IDh2 = ihisto+100000+i;

	  char tempbuffer[250];
	  sprintf(tempbuffer,"Output Node %d",i);
	  
	  TH1F* hout1 = loadHisto(f,IDh1,"",HbookMode);
	  TH1F* hout2 = loadHisto(f,IDh2,"",HbookMode);
	  if(hout1!=NULL && hout2!=NULL) {
	    hout2->GetYaxis()->SetRangeUser(-0.1,1.1);
	    hout2->GetYaxis()->SetLabelColor(0);
	    hout2->GetYaxis()->SetNdivisions(406,kFALSE);
	    hout2->GetXaxis()->SetLabelColor(0);
	    hout2->GetXaxis()->SetNdivisions(404,kFALSE);
	  
	    hout2->SetLineWidth(2);
	    hout2->SetLineColor(2);

	    TBox *box=new TBox(0.75,-0.095,100,0.0);
	    box->SetFillColor(590);
	    box->SetFillStyle(1001);
	    TBox *box2=new TBox(0.75,1.0,100,1.095);
	    box2->SetFillColor(590);
	    box2->SetFillStyle(1001);
	    hout2->Draw("L");	
	    box->Draw();
	    box2->Draw();
	    hout2->Draw("Lsame");	
	    hout1->SetLineColor(4);
	    hout1->SetMarkerColor(4);
	    hout1->Draw("same");
	    pt.DrawText(40,1.03,tempbuffer);
	  }
	}
	canvas->Update();
	if(j!=pages-1 && outputNodes!=1) canvas->Clear(); //not last page -> Get new page
      	if(!pdfOutput) PSFile->NewPage(); 
        if (interactive) getchar();
      }


    }


}

// /////////////////////////////////////////////////////////////////////////
//                       analysis function
// /////////////////////////////////////////////////////////////////////////

void analysis( string FileName = "teacherHistos.root",
    const char* PSFileName = "analysis.pdf", 
    int sort=0,
    const char* correlSigniFile = "correl_signi.txt") 
{

  string FileName2 = "teacherHistosTemp.root";

  if(FileName.substr(FileName.size()-4,FileName.size())==".txt"){
    FileName2 = FileName.substr(0,FileName.size()-4);
    FileName2 +=".root";
    asciiToRoot(FileName.c_str(),FileName2.c_str(),0);
    FileName=FileName2;
  }

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

  //"old" parameters of analysis.C:
  int netPlot = 1;
  int interactive = 0;

  // check the file extension
  string outFilename(PSFileName);
  bool pdfOutput = false;
  if(outFilename.substr(outFilename.size()-3,outFilename.size())=="pdf") pdfOutput=true;

  char hTitle[80];
  TFile *f = new TFile(FileName.c_str());
  if (f->IsZombie() || !(f->IsOpen())) {
    cerr << "Could not open file, abort!"<< endl;
    exit(1);
  }  
  
  //
  // determine if we look at a converted hbook
  // file or native root
  //
  int HbookMode=0;
  TH1F* histoTest = (TH1F*)f->Get("h100;1: ERROR LEARNSAMPLE");
  if (histoTest)  HbookMode = 1;

  bool zeroIterations = false;
  TH1F* h100 = loadHisto(f,100,"ERROR LEARNSAMPLE",HbookMode);
  if(h100 == 0) zeroIterations = true;

  //if zero iterations: check if plots after prepro are available:
  bool afterPrepro = false;
  //  if (zeroIterations){
    TH1F* h9101 = loadHisto(f,9101,"F(NETOUT) FOR BACKGROUND",HbookMode);
    TH1F* h9102 = loadHisto(f,9102,"F(NETOUT) FOR BACKGROUND",HbookMode);
    if (h9101 != NULL || h9102 != NULL) afterPrepro = true;
    //    }
    //    cout<<afterPrepro<<zeroIterations<<endl;     
    const char* working_dir = gSystem->WorkingDirectory();
    const char* base_name = gSystem->BaseName(PSFileName);
    const char* dir_name = gSystem->DirName(PSFileName);
    gSystem->cd(dir_name);
  //
  // open Postscript file 
  //
  TVirtualPS* PSFile = 0;
  if(pdfOutput)
    PSFile = new TPDF(base_name,111);
  else
    PSFile = new TPostScript(base_name,111);
  gSystem->cd(working_dir);



  TCanvas* canvas = new TCanvas("canvas","NeuroBayes",500,704);
  canvas->SetFillColor(10);

  TPad* graphPad = 0;

  // determine the number of output nodes from the input file
  int outputNodes = 0;
  int id = 101;
  TH1F* htemp = loadHisto(f,id,"F(NETOUT) FOR BACKGROUND",HbookMode);
  while (htemp != 0) {
    ++outputNodes;
    htemp = loadHisto(f,++id,"F(NETOUT) FOR BACKGROUND",HbookMode);
  }
  delete htemp;

  // first page : error plots
  if (!zeroIterations)
    {
      if(!pdfOutput) canvas->Clear();
      graphPad=printGlobalTitle(canvas,pdfOutput,correlSigniFile); 
      plotErrors(f,HbookMode,pdfOutput,graphPad);
      canvas->Update();
      if(!pdfOutput) PSFile->NewPage();

      if (interactive) getchar();
    }

  // second (group of) page(s) : Net output, eff/purity plots
  
  const Float_t MarkerSize = 0.5;
  int MarkerColour = 1;
      
  Float_t x[4];
  Float_t y[4];
  x[0]=0.; x[1]=1.0; x[2]=1.0; x[3]=0.;
  y[0]=0.; y[1]=0.0; y[2]=1.0; y[3]=1.0;
  
  TGraph *NullGraph = new TGraph(4,x,y);
  setAxisAttributes(NullGraph,"","signal efficiency","signal purity",
      0.,0.,0,0,0.03,0.03);
  NullGraph->SetMaximum(1.0);
  NullGraph->SetMinimum(0.0); 
  NullGraph->SetLineColor(MarkerColour);
  NullGraph->GetXaxis()->SetLimits(0.0,1.0);
  setMarkerAttributes(NullGraph,MarkerColour,0.001,20);

  //now plot network output (signal+bg) and purity vs. netoutput

  TH1F** PurEffGraph = new TH1F*[5*outputNodes];
  TH1F** PurEffGraph_preboost = new TH1F*[5*outputNodes];
  TH1F* hSig = 0;
  TH1F* hBG  = 0;
  TH1F* hSig_preboost = 0;
  TH1F* hBG_preboost  = 0;
  bool draw_boost;
  TF1* lineMin = 0;
  TCutG* lineMax = 0;
  if(!zeroIterations || afterPrepro)  {
    for (int i=1; i<= outputNodes; i++) 
      {
	if(!(zeroIterations && pdfOutput)) canvas->Clear();
	graphPad=printGlobalTitle(canvas,pdfOutput,correlSigniFile);
	  
	graphPad->Divide(1,2);
	// upper part of plot: Network output
	graphPad->GetPad(1)->cd();
	graphPad->GetPad(1)->SetBottomMargin(0.2);
	  
	// get histograms from file
	int special = 0;
	if (afterPrepro) special = 1;
	int IDSig = 200 + i + 9000*special;
	int IDBG  = 100 + i + 9000*special;
	  
	hSig = loadHisto(f,IDSig,"F(NETOUT) FOR SIGNAL",HbookMode);
	hBG  = loadHisto(f,IDBG,"F(NETOUT) FOR BACKGROUND",HbookMode);

	if (hSig == NULL && hBG == NULL && outputNodes==1)
	  {
	    // this is probably Diag + Classification, 
	    // the histos we want are in 9202 and 9102
	    ++IDSig;
	    ++IDBG;
	    hSig = loadHisto(f,IDSig,"F(NETOUT) FOR SIGNAL",HbookMode);
	    hBG  = loadHisto(f,IDBG,"F(NETOUT) FOR BACKGROUND",HbookMode);

	  }
	//boost histos are in 8102 etc.
	//	cout<<IDSig-1000<<endl;
	hSig_preboost = loadHisto(f,IDSig-1000,"F(NETOUT) FOR SIGNAL",HbookMode);
	hBG_preboost  = loadHisto(f,IDBG-1000,"F(NETOUT) FOR BACKGROUND",HbookMode);

	if(hSig_preboost!= NULL)
	  draw_boost=true;
	else
	  draw_boost=false;
	//	cout<<IDSig-1000<<draw_boost<<endl;
	if (hSig != NULL && hBG != NULL){
	  if(draw_boost) {
	    plotNetOutput(hSig_preboost,hBG_preboost,i,outputNodes,"",46,17);
	    plotNetOutput(hSig,hBG,i,outputNodes,"same",2,1,false);
	  }
	  else
	    plotNetOutput(hSig,hBG,i,outputNodes);

	  canvas->Update();
	  
	  // lower part of plot: purity vs. Netout
	  graphPad->GetPad(2)->cd();
	  graphPad->GetPad(1)->SetBottomMargin(0.2);
	  
	  TH1F* hsum = (TH1F*) hSig->Clone("hsum");
	  hsum->Add(hBG);
	  TH1F* hpur =(TH1F*) hSig->Clone("hpur");

	  if(draw_boost) {
	    TH1F* hsum_preboost = (TH1F*) hSig_preboost->Clone("hsum_preboost");
	    hsum_preboost->Add(hBG_preboost);
	    TH1F* hpur_preboost =(TH1F*) hSig_preboost->Clone("hpur_preboost");
	    plotPurity(hpur_preboost,hsum_preboost,i,outputNodes,"",17);
	    plotPurity(hpur,hsum,i,outputNodes,"same");
	  }
	  else
	    plotPurity(hpur,hsum,i,outputNodes);

	  canvas->Update();
	  canvas->cd();
	  if (i!=outputNodes&&zeroIterations) PSFile->NewPage();
	  
	  // prepare Purity/Efficiency Plots
	  PurityEfficiency(hBG, hSig, &(PurEffGraph[5*(i-1)]),1,i);
	  if(draw_boost){PurityEfficiency(hBG_preboost, hSig_preboost, &(PurEffGraph_preboost[5*(i-1)]),1,i+5*outputNodes);}

	} else {
	  cerr<<"the signal and background histograms "<<IDSig<<" and "<<IDBG
	      <<" were not found"<<endl;
	  return;
	}
	if (interactive) getchar();
      } // for i,nodes
    
    //
    // now plot purity/efficiency
    //
    canvas->Clear();
    graphPad=printGlobalTitle(canvas,pdfOutput,correlSigniFile);
      
    if (outputNodes==1)
      {              //for plot signal efficiency vs. efficiency
	graphPad->Divide(1,2);
	graphPad->GetPad(1)->cd();
	graphPad->GetPad(1)->SetGridx();
	graphPad->GetPad(1)->SetGridy();
      }
    else
      {
	graphPad->SetGridx();
	graphPad->SetGridy();
      }
    gStyle->SetPalette(1);
    gROOT->ForceStyle();
      
    NullGraph->GetXaxis()->SetTitle("signal efficiency"); 
    NullGraph->SetLineColor(10); 
    NullGraph->SetLineWidth(0); 
    NullGraph->SetMarkerColor(10);
    NullGraph->SetMarkerSize(0.001);
      
    NullGraph->Draw("AP");
      
    int colours[10]={4,50,8,9,45,6,38,46,42,34};
    int colIndex = 0;
    if(draw_boost && outputNodes==1) {
      PurEffGraph_preboost[2]->SetLineColor(2);
      setMarkerAttributes( PurEffGraph_preboost[2],2,MarkerSize,1);
      PurEffGraph_preboost[2]->Draw("psame");
      PurEffGraph_preboost[3]->SetLineColor(2);
      setMarkerAttributes( PurEffGraph_preboost[3],2,MarkerSize,1);
      PurEffGraph_preboost[3]->Draw("psame");
    }
    for (int i=1; i<= outputNodes; ++i) {

      MarkerColour = colours[colIndex];
      PurEffGraph[2 + 5*(i-1)]->SetLineColor(MarkerColour);
      setMarkerAttributes( PurEffGraph[2 + 5*(i-1)],MarkerColour,MarkerSize,1);
      PurEffGraph[2 + 5*(i-1)]->Draw("psame");
	
      PurEffGraph[3 + 5*(i-1)]->SetLineColor(MarkerColour);
      setMarkerAttributes( PurEffGraph[3 + 5*(i-1)],MarkerColour,MarkerSize,1);
      PurEffGraph[3 + 5*(i-1)]->Draw("psame");   
	
      colIndex++;
      if(colIndex>9) colIndex = 0;
    } //for i,nodes
      
    if (outputNodes==1){
      //plot signal efficiency vs. efficiency
      graphPad->GetPad(2)->cd();
      graphPad->GetPad(2)->SetGridx();
      graphPad->GetPad(2)->SetGridy();
      graphPad->GetPad(2)->SetBottomMargin(0.2);
      TGraph *NullGraph2 = (TGraph*) NullGraph->Clone();
      setAxisAttributes(NullGraph2,"","efficiency","signal efficiency");
      NullGraph2->Draw("AP");

      // compute the Gini index, the maximum possible 
      // and write the values on the plot
      // Gini coefficient = area between lift chart and diagonal, 
      //           divided by the area below the diagonal
      // Ginin index = Gini coefficient * 100;
      // (http://en.wikipedia.org/wiki/Gini_coefficient)
      int nBins = PurEffGraph[2]->GetNbinsX();
      double maxGiniIndex = 100.* (1 - PurEffGraph[2]->GetBinContent(nBins) );
      double giniIndex = (2*computeIntegral(PurEffGraph[4])-1) * 100;
   
      char title[500];
      if(!draw_boost) {
	sprintf(title,"Gini index = %5.1f%%, max possible = %5.1f%%",giniIndex,maxGiniIndex);
      } 
      else { 
	double giniIndex_preboost = (2*computeIntegral(PurEffGraph_preboost[4])-1) * 100;
	sprintf(title,"Gini index final/preboost = %5.1f%%/%5.1f%% max possible = %5.1f%%",giniIndex,giniIndex_preboost,maxGiniIndex);
      }
      TPaveTextNB *ptGini = new TPaveTextNB(0.1,0.90144,0.9,0.965,title);
      ptGini->Draw("same");
  
	  
      // shaded area under the line which represent
      // the performance of a random selection
      //      float x[3] = {0.,1.,1.};
      //      float y[3] = {0.,1.,0.};
      lineMin = new TF1("line","x",0,1);
      lineMin->SetFillColor(14);
      lineMin->SetFillStyle(3003);
      lineMin->SetLineWidth(1);
      lineMin->Draw("same");
	  
      // shaded area above the line which represent
      // the performance of the best possible selection
      float x2[4] = {0.,float(PurEffGraph[2]->GetBinContent(nBins)),0,0};
      float y2[4] = {0.,1.,1.,0};
      lineMax = new TCutG("lineMax",4,x2,y2);
      lineMax->SetFillColor(14);
      lineMax->SetFillStyle(3003);
      lineMax->SetLineWidth(1);
      lineMax->SetLineColor(kBlack);
      lineMax->Draw("samef");
      lineMax->Draw("samel");
	  
      PurEffGraph[4]->SetLineColor(4);
      PurEffGraph[4]->Draw("lsame");
      if(draw_boost) {
	PurEffGraph_preboost[4]->SetLineColor(2);
	PurEffGraph_preboost[4]->Draw("lsame");
	TLegend* leg = new TLegend(0.8,0.2,0.9,0.3);
	//   leg->SetHeader("The Legend Title");
	leg->AddEntry(PurEffGraph_preboost[4],"first round","l");
	leg->AddEntry(PurEffGraph[4],"final","l");
	leg->Draw();
      }
    }

    canvas->Update();
    if(!pdfOutput) PSFile->NewPage();
    if (interactive) getchar();
  }
  //Plot spline fit of the outputnodes for DIAG level i only for ps
  int n_diag_fits=3;
  int ihisto;
  TH1F* hdummy;
  for(int i = 0;i<n_diag_fits;i++) {
    int ihisto=2110000 + i*1000000;
    hdummy = loadHisto(f,ihisto+2,"ERROR LEARNSAMPLE",HbookMode);
    if(hdummy != NULL) 
      plot_diag_fits(ihisto,outputNodes,graphPad,canvas,pdfOutput,f
	  ,HbookMode,correlSigniFile,PSFile,interactive);
    else 
      delete hdummy;
  }
  //
  //plot the distribution of the target for density trainings
  //
  if(outputNodes>1) {
    canvas->Clear();

    graphPad=printGlobalTitle(canvas,pdfOutput,correlSigniFile); 

    plotTargetDistribution(f,graphPad,HbookMode);

    canvas->Update();
    gStyle->SetPalette(1);
    gROOT->ForceStyle();
    if(!pdfOutput) PSFile->NewPage();
    if (interactive) getchar();
  }

  // this is the first page in case of 0 iterations
  if((!zeroIterations || afterPrepro || !pdfOutput)) 
    canvas->Clear();
    graphPad = printGlobalTitle(canvas,pdfOutput,correlSigniFile); 

  canvas->Update();

  correlations(f,HbookMode); //plot correlation matrix

  canvas->Update();
  if(!pdfOutput) PSFile->NewPage();

  if (interactive) getchar();

  if(loadHisto2D(f,2001,"correlation matrix",HbookMode)!=0) {
    canvas->Clear();
    graphPad = printGlobalTitle(canvas,pdfOutput,correlSigniFile); 
    correlations(f,HbookMode,2001); //plot correlation matrix internal boost
    canvas->Update();
    if(!pdfOutput) PSFile->NewPage();
    if (interactive) getchar();
  }

  //////////////////////////////////////////////////
  // now plot Purity/Efficiency for each variable //
  //////////////////////////////////////////////////

  // find minimum and maximum of the eff/purity plot
  // and use these values to zoom in the graph
  double maxGraph = 0.;
  double minGraph = 1.;
  int middleIndex = 5*int(outputNodes/2);
  if(!zeroIterations || afterPrepro) {
    for(int j=1;j<= PurEffGraph[2 + middleIndex]->GetNbinsX();j++) {
      if(PurEffGraph[2 + middleIndex]->GetBinContent(j) > maxGraph)
	maxGraph = PurEffGraph[2 + middleIndex]->GetBinContent(j);
    }
    for(int j=1;j<= PurEffGraph[3 + middleIndex]->GetNbinsX();j++) {
      if(PurEffGraph[3 + middleIndex]->GetBinContent(j) < minGraph)
	minGraph = PurEffGraph[3 + middleIndex]->GetBinContent(j);
    }
      
    PurEffGraph[2 + middleIndex]->SetMaximum(1.0);                  
    PurEffGraph[2 + middleIndex]->SetMinimum(0.0);
    PurEffGraph[2 + middleIndex]->SetLineColor(2);
    setMarkerAttributes(PurEffGraph[2+middleIndex],2,MarkerSize,1);
      
    PurEffGraph[3 + middleIndex]->SetLineColor(2);
    setMarkerAttributes(PurEffGraph[3+middleIndex],2,MarkerSize,1);
  } else  {
    maxGraph = 1.;
    minGraph = 0.;
  }

  //retrieve number of input variables:
  int  numInputVars = 0;
  id = 1015002;
  htemp = loadHisto(f,id,"Check: Input variables",HbookMode);
  while (htemp != NULL) {
    ++numInputVars;
    htemp = loadHisto(f,++id,"Check: Input variables",HbookMode);
  }
  delete htemp;
  cout << "Found " << numInputVars << " input variables." << endl;

  bool signi=false;
  bool noName = false;
  string line;
  string line2;

  auto correlSigni = std::vector<myCorrelSigni>(numInputVars);
  bool found_num_signi_vars = false;
  int num_signi_vars;

  for (int i=1; i<=numInputVars; i++) 
    {
      correlSigni[i-1].index = i;
      correlSigni[i-1].position = -999;
      correlSigni[i-1].addSigni = -999.;
      correlSigni[i-1].aloneSigni = -999.;
      correlSigni[i-1].loss = -999.;
      correlSigni[i-1].correlation = -999.;
      sprintf(correlSigni[i-1].prepro," ");
      ifstream significanceList(correlSigniFile);
      if(significanceList)
	{
	  signi =true;
	  int varIndex = -1;
	  // search for the line containing the significance of this variable
	  int counter = 0;
	  // have to scan the file all over for each variable
	  // to search for this variable
	  while (varIndex!=i+1 && ! significanceList.eof())	{
	    ++counter;  // line counter
	    getline(significanceList,line);
	    if(counter == 1){
	      if(((int)line.find("names of variables are not available"))!= (ENDOFSTRING)) noName = true;
	    }
	    if(line.size()>40) {
	      //read the content of the line
	      string::size_type endPosition = (line.substr(0,25)).find(":");
	      endPosition = (line.substr(0,25)).find(":",endPosition+1);
	      string varTag = line.substr(0,endPosition);
	      sscanf(varTag.c_str(),"%*f: variable %i:",&varIndex);
	    }
	    if(found_num_signi_vars==false && varIndex==i+1) {
	      while(!significanceList.eof()) { // loop until end of file 
		getline(significanceList,line2);
		if((int)line2.find("Keep only")!=-1) {
		  // READ : Keep only XXX most significant input variables
		  sscanf(line2.c_str(),"%*s %*s %d",&num_signi_vars);
		  found_num_signi_vars=true;
		}
	      }
	    }
	  }
	  significanceList.close();

	  if(noName){
	    sscanf(line.c_str(),"%f: %*s %*i: %f %f %f %f",
		&correlSigni[i-1].position,&correlSigni[i-1].addSigni,&correlSigni[i-1].aloneSigni,&correlSigni[i-1].loss,&correlSigni[i-1].correlation);
	  } else {
	    sscanf(line.c_str(),"%f: %*s %*i: %f %f %f %f %*c %49s",
		&correlSigni[i-1].position,&correlSigni[i-1].addSigni,&correlSigni[i-1].aloneSigni,&correlSigni[i-1].loss,&correlSigni[i-1].correlation,correlSigni[i-1].varName);
	    string::size_type found=line.find(correlSigni[i-1].varName);
	    string help =line.substr(int(found)+string(correlSigni[i-1].varName).size(),line.size());
	    sprintf(correlSigni[i-1].prepro,"%s",help.c_str());
	  }
	  if(verbose_analysis()){
	    if(noName){
	      printf("%f: %f %f %f %f\n",
		  correlSigni[i-1].position,correlSigni[i-1].addSigni,correlSigni[i-1].aloneSigni,correlSigni[i-1].loss,correlSigni[i-1].correlation);
	    } else {
	      printf("%f: %f %f %f %f %s %s\n",
		  correlSigni[i-1].position,correlSigni[i-1].addSigni,correlSigni[i-1].aloneSigni,correlSigni[i-1].loss,correlSigni[i-1].correlation,correlSigni[i-1].varName,correlSigni[i-1].prepro);
	    }
	  }
	}
    }
  for (int i=1; i<=numInputVars; i++) {
    if(correlSigni[i-1].position <= num_signi_vars)
      correlSigni[i-1].is_signi = true;
    else
      correlSigni[i-1].is_signi = false;
  }

  if (signi){
    string sortVal = "";
    if (sort == 0) {
      //default: sort by input array index
      sortVal = "index";
    }
    else if (sort == 1) {
      sortVal = "position";
    }
    else if (sort == 2)  {
      sortVal = "aloneSigni";
    }
    else {
      cout << "Sorting criterium not implemented! Please check!" << endl;
      return;
    }
    bubblesort(correlSigni, numInputVars, sortVal);
    cout << "Sorting input variables by " << sortVal << endl;
  }

  int ID1 = 0;
  int ID2 = 0;
  TPaveTextNB *pt1 = 0;
  for (int it=1; it<=numInputVars; it++) 
    {
      int i = correlSigni[it-1].index; //Index of Variable
      //       cout << "i = " << i << "; position = " << correlSigni[it-1].position;
      //       cout << "; aloneSigni = " << correlSigni[it-1].aloneSigni;
      //       cout << "; addSigni = " << correlSigni[it-1].addSigni << endl;
   
      if(verbose_analysis()) cout<<"node "<<i+1<<endl;

      canvas->Clear();
      graphPad=printGlobalTitle(canvas,pdfOutput,correlSigniFile);

      if(signi)
	{
          //
          // significance of this variable
          //
	  //	  int orange = TColor::GetColor("#ef521c");


	  //create pad for title and states
	  canvas->cd();

	  TPad *stats = new TPad("stats", "stats", 0.1,0.88,0.84,0.99);
	  stats->Draw();
	  stats->cd();
	  stats->SetFillColor(10);
	  stats->SetBorderSize(2);
	  stats->SetFrameFillColor(0);

	  // Box
	  TPave* statbox= new TPave(0.01,0.02,0.99,1);
	  statbox->SetBorderSize(1);
	  statbox->SetFillColor(10);
	  statbox->Draw();

	  // Left stats
	  TPaveText* p_left=new TPaveText(0.05,0.05,0.45,0.78);
	  p_left->SetBorderSize(0);
	  p_left->SetFillColor(10);
	  p_left->SetTextAlign(11);

	  //Right stats
	  TPaveText* p_right= new TPaveText(0.45,0.05,0.9,0.78);
	  p_right->SetBorderSize(0);
	  p_right->SetFillColor(10);
	  p_right->SetTextAlign(11);

	  // Varname
	  TPaveText* p_varname= new TPaveText(0.015,0.8,0.94,0.9);
	  p_varname->SetBorderSize(0);
	  p_varname->SetFillColor(10);
	  p_varname->SetTextAlign(11);

	  // is_signi
	  TBox* b_signi= new TBox(0.94,0.8,0.98,0.9);

	  char Str1[300];

	  // Draw Varname
	  if(!noName){
	    sprintf(Str1," Input node %i : %s ",i+1,correlSigni[it-1].varName);
	    p_varname->SetTextColor(2);
	    p_varname->AddText(Str1);
	  }
	  if(correlSigni[it-1].is_signi) {
	    b_signi->SetFillColor(3);
	  }
	  else {
	    b_signi->SetFillColor(2);
	  }

	  char conversion[10];
	  sprintf(conversion,"%.0f",correlSigni[it-1].position);
	  string ordinal = conversion;
	  if(ordinal.size()>1 &&
	      ordinal.substr(ordinal.size()-2,1)== '1') {
	    sprintf(conversion,"th");
	  } else {
	    if(ordinal.substr(ordinal.size()-1,1)== '1')
	      sprintf(conversion,"st");
	    else if(ordinal.substr(ordinal.size()-1,1)== '2' )
	      sprintf(conversion,"nd");
	    else if(ordinal.substr(ordinal.size()-1,1)== '3' )
	      sprintf(conversion,"rd");
	    else sprintf(conversion,"th");
	  }

	  sprintf(Str1,"%.0f%s most important",correlSigni[it-1].position,conversion);
	  p_left->AddText(Str1);

	  sprintf(Str1,"PrePro: %s",correlSigni[it-1].prepro);
	  p_right->AddText(Str1);
	  
	  sprintf(Str1,"added signi. %.2f",correlSigni[it-1].addSigni);
	  p_left->AddText(Str1);

	  sprintf(Str1,"only this %.2f",correlSigni[it-1].aloneSigni);
	  p_right->AddText(Str1);

	  sprintf(Str1,"signi. loss %.2f",correlSigni[it-1].loss);
	  p_left->AddText(Str1);

	  sprintf(Str1,"corr. to others %.2f%%",correlSigni[it-1].correlation);
	  p_right->AddText(Str1);
	  
	  // We want the same textsize in all PaveText objects
	  float textsize = 0.15;
	  p_right->SetTextSize(textsize);
	  p_left->SetTextSize(textsize);
	  p_varname->SetTextSize(textsize);

	  p_varname->Draw();
	  b_signi->Draw();
	  p_right->Draw();
	  p_left->Draw();
	}

      canvas->Update();
      // make graphPad smaller due to the stats box 
      graphPad->SetPad(0.,0.,1.,0.88);

      graphPad->Divide(1,4);
      canvas->Update();

      //
      // upper plot: signal + background vs. network output
      //
      TH1F* hSig = loadHisto(f,1025000+i+1,"Check: Input variables",HbookMode); 
      TH1F* hBG = loadHisto(f,1015000+i+1,"Check: Input variables",HbookMode); 
      graphPad->GetPad(1)->cd();
      graphPad->GetPad(1)->SetTopMargin(0.1);
      //      graphPad->GetPad(1)->SetTopMargin(0.15);
      //      graphPad->GetPad(1)->SetBottomMargin(0.15);
      graphPad->GetPad(1)->SetBottomMargin(0.25);
      graphPad->GetPad(1)->SetGridx();
      graphPad->GetPad(1)->SetGridy();
      if(verbose_analysis()) cout<<"distribsingle"<<endl;
      distribsingle5(hSig,hBG,i);
      // write the original values of this variable	  
      writeTabValues(f,i,HbookMode);


      //
      // second plot: purity vs. netout
      //
      graphPad->GetPad(2)->cd();
      //      graphPad->GetPad(2)->SetTopMargin(0.05);
      graphPad->GetPad(2)->SetTopMargin(0.05);
      graphPad->GetPad(2)->SetBottomMargin(0.15);
      graphPad->GetPad(2)->SetGridx();
      graphPad->GetPad(2)->SetGridy();
      if(verbose_analysis()) cout<<"spline or map"<<endl;
      hSig->Sumw2();    // have correct error treatment:
      hBG->Sumw2();

      int IDSpline = 1100000 + i+1;
      sprintf(hTitle,"var %3i before reg. spline fit",i+1);
      TH1F* hSpline = loadHisto(f,IDSpline,hTitle,HbookMode);
	  
      int IDMap = 1300000 + i+1;
      sprintf(hTitle,"var %i before map prepro",i+1);
      TH1F* hMap = loadHisto(f,IDMap,hTitle,HbookMode); 
      //SR
      if (hSpline!=0)  
	SplinePrepro(f,i,HbookMode);
      else if (hMap!=0 && hMap->GetXaxis()->GetNbins()>1)
	MapPrepro(f,i,HbookMode);
      else 
	{//End SR

	  // plot delta function if the hMap plot exists
	  TH1F* hSum = (TH1F*) hSig->Clone("hSumVar");
	  hSum->Add(hBG);
	  TH1F* hPur = (TH1F*) hSig->Clone("hSumVar");
	      
	  hPur -> Divide(hPur,hSum,1,1,"B");
	  hPur -> SetLineColor(1);
	  hPur -> SetMaximum(hPur->GetMaximum()*1.1);
	  hPur -> SetMinimum(hPur->GetMinimum()*0.9); 
	  setAxisAttributes(hPur,"","bin #","purity",0.08,0.08,0.8,0.6,0.07,0.07);
	  if(hMap!=0)
	    {
	      GaussPrepro(hMap,hPur,i);
	    }
	  else //draw it
	    {
	      hPur -> Draw("pe");
	      writeTitle(hPur,"purity");
	    }
	}
      //
      // third plot
      //
      graphPad->GetPad(3)->cd();
      graphPad->GetPad(3)->SetTopMargin(0.1);
      graphPad->GetPad(3)->SetGridx();
      graphPad->GetPad(3)->SetGridy();
      gStyle->SetOptStat("oun");
      ID1 = 1016000 + i+1;
      TH1F* h1 = loadHisto(f,ID1,"Check: Input variables",HbookMode); 
	  
      ID2 = 1026000 + i+1;
      TH1F* h2 = loadHisto(f,ID2,"Check: Input variables",HbookMode); 

      if(verbose_analysis()) cout<<"final"<<endl;
      //set maximum
      double Max = 0;
      if (h1->GetMaximum() > h2->GetMaximum())
	Max = h1->GetMaximum();
      else
	Max = h2->GetMaximum();
      Max = 1.1*Max;
      h1->SetMaximum(Max);


      h1->SetLineColor(1);

      //      setAxisAttributes(NullGraph,"","signal efficiency","signal purity",
      //			0.08,0.07,0,0.6,0.07,0.07);
      //      setAxisAttributes(NullGraph,"","signal efficiency","signal purity",
      //	    0.,0.,0,0,0.03,0.03);
      setAxisAttributes(h1,"","final netinput","events",
	  0.08,0.07,0.5,0.6,0.06,0.06);

      gStyle->SetStatX(0.9);
      gStyle->SetStatY(0.95);
      gStyle->SetStatFontSize(0.05);
      gStyle->SetStatTextColor(1);
      h1->SetName("background");
      //      h1->UseCurrentStyle();
      h1->Draw();
      canvas->Update();

      

      h2->SetName("signal");
      gStyle->SetStatY(0.8);
      gStyle->SetStatTextColor(2);

      //     h2->UseCurrentStyle();
      h2->SetLineColor(2);

      h2->Draw("sames");

      writeTitle(h1,"final");

      sprintf(hTitle,"Input variable %i final",i+1);
      pt1 = new TPaveTextNB(0.1,0.90144,0.9,0.995,hTitle);

      //
      // lower plot: purity vs efficiency
      //
      graphPad->GetPad(4)->cd();
      graphPad->GetPad(4)->Clear(); 
      //      graphPad->GetPad(4)->SetTopMargin(0.05);
      //      graphPad->GetPad(4)->SetBottomMargin(0.15);
      graphPad->GetPad(4)->SetTopMargin(0.05);
      graphPad->GetPad(4)->SetBottomMargin(0.2);
      graphPad->GetPad(4)->SetGridx();
      graphPad->GetPad(4)->SetGridy();

      NullGraph->SetMaximum(maxGraph+0.05);
      NullGraph->SetMinimum(minGraph-0.05);

      setAxisAttributes(NullGraph,"","signal efficiency","signal purity",
	  0.08,0.07,0,0.6,0.07,0.07);
      NullGraph-> Draw("AP");

      if(verbose_analysis()) cout<<"eff/pur"<<endl;
      if(hSpline!=0 || (hMap!=0 && hMap->GetXaxis()->GetNbins()>1))
	plotIndividualPurEff(f,i,HbookMode);
      else
	plotIndividualPurEff(f,i,hSig,hBG,HbookMode);
      writeTitle(0,1,minGraph,maxGraph+0.05,"separation");
      if(!zeroIterations || afterPrepro )  {
	//nn output purity/efficiency                    
	PurEffGraph[2 + middleIndex]->Draw("psame");
	PurEffGraph[3 + middleIndex]->Draw("psame");

      }
      //
      // plotting done, update and wait for user
      //
      canvas -> Update();
      gStyle->SetOptStat(0);	  
      if(!pdfOutput && it<numInputVars) PSFile->NewPage();

      if (hSig) delete hSig;
      if (hBG) delete hBG;
    } // for nodes

  if (interactive) getchar();

  if (netPlot && !zeroIterations)
    { 
      //if requested plot net architecture
      if(!pdfOutput) PSFile->NewPage();
      canvas->Clear();
      graphPad=printGlobalTitle(canvas,pdfOutput,correlSigniFile);

      canvas->Update();
      netplot(f,HbookMode); 
      
      canvas->Update();
      if (interactive) getchar();
    }
  PSFile->Close();

  delete lineMin;
  delete lineMax;

  delete [] PurEffGraph;
  delete f;
  delete PSFile;
  delete canvas;
  delete NullGraph;
  delete pt1;
  return;

} //void analysis
