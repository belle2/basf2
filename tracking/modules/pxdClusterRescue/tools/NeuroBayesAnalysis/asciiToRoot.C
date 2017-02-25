// This macro transforms the teacher ASCII output into root histograms
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <assert.h>
#include <TLeaf.h>
#include <TBranch.h>
#include <TString.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

using namespace std;

void asciiToRoot(const char* filenameIn="ahist.txt",const char* filenameOut = "teacherHistos.root", int verbose = 0)
{

  float nbins,nbinsx,xmin,xmax,nbinsy,ymin,ymax;
  float underflow,overflow;
  float histId,histType;

  std::cout<<"asciiToRoot: converting "<<filenameIn<<" to Root format ";
  FILE*teacherHistos = fopen(filenameIn,"r");

  if(teacherHistos) {

    // open the output file
    TFile* root_file = new TFile(filenameOut,"RECREATE");

    unsigned counter = 0;
    int i,j;

    while(fscanf(teacherHistos,"%f %f", &histId,&histType) != EOF){
      counter++;

      if (verbose>1)
        std::cerr<<"histId \t"<<histId<<"\t type \t"<<histType<<std::endl;

      root_file->cd();

      if ( verbose > 0)
	std::cerr << "Extracting hist " << (int)histId << " of type " << (int)histType;

      if((int)histType == 1 || (int)histType == 11)    {// 1-dim histogram

	fscanf(teacherHistos,"%f %f %f", &nbins,&xmin,&xmax);

	if ( verbose > 0)
	  std::cerr << " with " << (int)nbins << " bins " << std::endl;

	char hName[80];
	sprintf(hName,"h%i",(int)histId);
	TH1F* histo = new TH1F(hName,"",(int)nbins,xmin,xmax);
        i=1;
	bool allNulls = true;
	while(i<nbins+1)	{// read the bin content
	  float tmp;
	  fscanf(teacherHistos,"%f",&tmp);
	  histo->SetBinContent(i,tmp);
	  if ( verbose > 1)
	    std::cerr << "value " << i << " is " << tmp << std::endl;
	  if(allNulls && tmp!=0) allNulls = false;
	  i++;
	}

	fscanf(teacherHistos,"%f %f",&underflow,&overflow);
	histo->SetBinContent(0,underflow);
	if(allNulls && underflow!=0) allNulls = false;
	histo->SetBinContent((int)nbins+1,overflow);
	if(allNulls && overflow!=0) allNulls = false;

	if((int)histType> 10) { // read out the errors as well
	  i = 1;
	  while(i<=nbins)  {// read the bin content
	    float tmp;
	    fscanf(teacherHistos,"%f",&tmp);
	    histo->SetBinError(i,tmp);
	    if ( verbose > 2)
	      std::cerr << "error " << i << " is " << tmp << std::endl;
	    i++;
	  }
	}
	if(allNulls) histo->SetEntries(0);
	histo->Write();
	delete histo;
      }
      else if((int)histType == 2 || (int)histType == 12)  {// 2-dim histogram
	fscanf(teacherHistos,"%f %f %f", &nbinsx,&xmin,&xmax);
	fscanf(teacherHistos,"%f %f %f", &nbinsy,&ymin,&ymax);
	if ( verbose > 0)
	  std::cerr << " with " << (int)nbinsx << " bins in x and " << (int)nbinsy << " bins in y"<< std::endl;
	char hName[80];
	sprintf(hName,"h%i",(int)histId);
	TH2F* histo2 = new TH2F(hName,"",(int)nbinsx,xmin,xmax,
				(int)nbinsy,ymin,ymax);
	j=1;
	while(j<nbinsy+1) {
	  i=1;
	  while(i<nbinsx+1) {// read the bin content
	    float tmp;
	    fscanf(teacherHistos,"%f",&tmp);
	    histo2->SetBinContent(i,j,tmp);
	    if ( verbose > 1)
	      std::cerr << "value " << i << "," << j << " is " << tmp << std::endl;
	    i++;
	  }
	  j++;
	}

	fscanf(teacherHistos,"%f ",&overflow);
	histo2->SetBinContent(int(nbinsx)+1,(int)nbinsy+1,overflow);

	if((int)histType> 10){ // read out the errors as well
	  j=1;
	  while(j<=nbinsy)  {
	    int i=1;
	    while(i<=nbinsx){// read the bin content
	      float tmp;
	      fscanf(teacherHistos,"%f",&tmp);
	      histo2->SetBinError(i,j,tmp);
	      if ( verbose > 2)
		std::cerr << "error " << i << "," << j << " is " << tmp << std::endl;
	      i++;
	    }
	    j++;
	  }
	}
	histo2->Write();
	delete histo2;
      }
      else if((int)histType == 3)  {// variable bin size histogram

	float nbinstmp;

	bool allNulls = true;
	fscanf(teacherHistos,"%f", &nbinstmp);
	char hName[80];
	int nbinsHist = (int)nbinstmp;
        int nbinsconv = (int)nbinstmp;
	const int nbinsvar = ++nbinsconv;
	if ( verbose > 0)
	  std::cerr << " with " << nbinsvar-1 << " bins " << std::endl;
	auto binedge = std::vector<float>(nbinsvar);

	sprintf(hName,"h%i",(int)histId);
	j=0;
	while(j<nbinsvar) {
	  fscanf(teacherHistos,"%f",&binedge[j]);
	  if ( verbose > 2)
	    std::cerr << " binedge is " << binedge[j] << std::endl;
	  j++;
	}
	TH1F* histo1xbin = new TH1F(hName,"",nbinsHist,binedge.data());
	i=1;
	while(i<nbinsvar) {// read the bin content
	  float tmp;
	  fscanf(teacherHistos,"%f",&tmp);
	  histo1xbin->SetBinContent(i,tmp);
	  if ( verbose > 1)
	    std::cerr << "value " << i << " is " << tmp << std::endl;
	  if(allNulls && tmp!=0) allNulls = false;
	  i++;
	}

	fscanf(teacherHistos,"%f %f",&underflow,&overflow);
	histo1xbin->SetBinContent(0,underflow);
	if(allNulls && underflow!=0) allNulls = false;
	histo1xbin->SetBinContent((int)nbins+1,overflow);
	if(allNulls && overflow!=0) allNulls = false;

	if(allNulls) histo1xbin->SetEntries(0);
	histo1xbin->Write();

	delete histo1xbin;

      }
    }
    // close the output file
    root_file->Close();
  } else  {
    std::cout<<"ERROR: histogram ascii file "<<filenameIn<<" not found"<<std::endl;
    return;
  }
  fclose(teacherHistos);
  std::cout<<"... finished"<<endl;
  return;
}
