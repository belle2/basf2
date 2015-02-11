#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TBranch.h>
#include "imp_chNames.h"

void EvtGenValidate_BrTable()
{
  Char_t input[500];
  cout << "Input file: " << endl;
  cin >> input;
  cout << endl;

  const Int_t len = 6;

  Char_t* trees[len] = {"d0","dp","dsp","dst0","dstp","dsstp"};
  Char_t* gtrees[len] = {"d0gen","dpgen","dspgen","dst0gen","dstpgen","dsstpgen"};
  Char_t* branches[len] = {"D0__extraInfodecayModeID","D__extraInfodecayModeID","D_s__extraInfodecayModeID","DST0__extraInfodecayModeID","DST__extraInfodecayModeID","D_sST__extraInfodecayModeID"};
  Char_t* particle[len] = {"D0","D+","D_s+","D*0","D*+","D_s*+"};
  Int_t decModes[len] = {16,11,10,2,3,2};
 
  Int_t k, toggle;
  cout << "Produce LaTeX table? (0/1/2 = no/yes/exit):" << endl;
  cin >> toggle;
  cout << endl;

  for(k = 0; k < len; k++) {
    
    Int_t *chan = new Int_t[decModes[k]];
    Int_t *chID = new Int_t[decModes[k]];
    Double_t *perc = new Double_t[decModes[k]];
    Double_t *cperc = new Double_t[decModes[k]];

    Float_t ID;
    Int_t i = 0, j = 0;
    Double_t sum = 0;

    TFile* f = new TFile(input);
    TTree* t = f->Get(trees[k]);
    TTree* g = f->Get(gtrees[k]);

    t->SetBranchAddress(branches[k], &ID);

    Int_t nEntries = t->GetEntries();
    Int_t nGenEntries = g->GetEntries();
    
    if (nGenEntries == 0 || nEntries == 0)
      continue;

    cout << "************************************************************************"<<endl;;
    cout << "*   " << particle[k]<<endl;
    cout << "************************************************************************";
    cout << "\nSum of branching ratios: " << nEntries*100.0/(nGenEntries) <<" %" << endl << "Table of info for various decays (all Br are in [%]):" << endl << endl;

    for (i=0; i<decModes[k]; i++) chan[i]=0;

    for (i=0; i<nEntries; i++)
      {
	t->GetEntry(i);
	chan[(int) ID-1] = chan[(int) ID-1]+1;
      }
  
    for (i=0; i<decModes[k]; i++) perc[i]=(Double_t) chan[i]*100.0/(nEntries);
    
    TMath::Sort(decModes[k],perc,chID);
    
    for (i=0; i<decModes[k]; i++)
      {
	for (j=0; j<=i; j++)
	  {
	    sum += perc[chID[j]];
	  }
	cperc[i] = sum;
	sum = 0;
      }

    if(toggle == 0) {
      cout << "*Sort no.*\t*Channel no.*\t*Br ratio (rel)*\t*Br ratio (abs)*\t*Cumulative Br ratio (rel)*\t*Decay mode*" << endl;
      cout << "***************************************************************************************************************************************************************" << endl;

      for (i=0; i<decModes[k]; i++)
	{
	  std::cout << std::setprecision(3);
	  cout << i+1<<"\t\tch:"<<chID[i]+1<<"\t\t"<<perc[chID[i]]<<"\t\t"<<perc[chID[i]]*nEntries/nGenEntries<<"\t\t"<<cperc[i]<<"\t\t\t"<<namesD[k][chID[i]]<<"\\\\"<<endl;
	}
    } else if(toggle == 1) {
      cout << "Srt&Ch&$Br(\\mathrm{rel})$&$Br(\\mathrm{abs})$&$\\Sigma Br(\\mathrm{rel})$&Decay\\\\" << endl;
      cout << "\\hline" << endl;

      for (i=0; i<decModes[k]; i++)
	{
	  std::cout << std::setprecision(3);
	  cout<< i+1<<"&ch:"<<chID[i]+1<<"&"<<perc[chID[i]]<<"&"<<perc[chID[i]]*nEntries/nGenEntries<<"&"<<cperc[i]<<"&"<<namesD[k][chID[i]]<<"\\\\"<<endl;
	}
    } else if(toggle == 2)
      {
      }

    delete [] chan;
    delete [] chID;
    delete [] perc;
    delete [] cperc;
    
  }
  exit(0);
}

  
