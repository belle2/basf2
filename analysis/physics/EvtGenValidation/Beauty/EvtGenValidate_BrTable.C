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
  
  const Int_t len = 2;
  
  Char_t* trees[len] = {"bp", "b0"};
  Char_t* gtrees[len] = {"bpgen", "b0gen"};
  Char_t* branches[len] = {"B____bodecayModeID__bc", "B0____bodecayModeID__bc"};
  Char_t* particle[len] = {"B+", "B0"};
  Int_t decModes[len] = {124, 123};

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

    cout << "************************************************************************" << endl;;
    cout << "*   " << particle[k] << endl;
    cout << "************************************************************************";
    cout << "\nSum of branching ratios: " << nEntries * 100.0 / (nGenEntries) << " %" << endl << "Table of info for various decays (all Br are in [%]):" << endl << endl;
    cout << "1: Decay included in the Full Event Interpretation Report 2014\n2: Frequent decay (cumulatively in top 98 %)\n3: Clean decay mode\n4: Less frequent decay mode (included only with argument \"1\")" << endl << endl;

    for (i = 0; i < decModes[k]; i++)
      chan[i] = 0;

    for (i = 0; i < nEntries; i++) {
      t->GetEntry(i);
      if (ID == 0) ID = 1;
      chan[(int) ID - 1] = chan[(int) ID - 1] + 1;
    }

    for (i = 0; i < decModes[k]; i++) perc[i] = (double) chan[i] * 100.0 / (nEntries);

    TMath::Sort(decModes[k], perc, chID);

    for (i = 0; i < decModes[k]; i++) {
      for (j = 0; j <= i; j++) {
        sum += perc[chID[j]];
      }
      cperc[i] = sum;
      sum = 0;
    }

    if (toggle == 0) {
      cout << "*Sort no.*\t*Channel no.*\t*Decay type*\t*Br ratio (rel)*\t*Br ratio (abs)*\t*Cumulative Br ratio (rel)*\t*Decay mode*" << endl;
      cout << "***************************************************************************************************************************************************************" << endl;

      for (i = 0; i < decModes[k]; i++) {
        std::cout << std::setprecision(3);
        cout << i + 1 << "\t\tch:" << chID[i] + 1 << "\t\t" << typeB[k][chID[i]] << "\t\t" << perc[chID[i]] << "\t\t\t" << perc[chID[i]]*nEntries / nGenEntries << "\t\t" << cperc[i] << "\t\t\t" << namesB[k][chID[i]] << "\\\\" << endl;
      }
    } else if (toggle == 1) {
      cout << "Srt&Ch&Type&$Br(\\mathrm{rel})$&$Br(\\mathrm{abs})$&$\\Sigma Br(\\mathrm{rel})$&Decay\\\\" << endl;
      cout << "\\hline" << endl;

      for (i = 0; i < decModes[k]; i++) {
        std::cout << std::setprecision(3);
        cout << i + 1 << "&ch:" << chID[i] + 1 << "&" << typeB[k][chID[i]] << "&" << perc[chID[i]] << "&" << perc[chID[i]]*nEntries / nGenEntries << "&" << cperc[i] << "&" << namesB[k][chID[i]] << "\\\\" << endl;
      }
    } else if (toggle == len) {
    }
    
    delete [] chan;
    delete [] chID;
    delete [] perc;
    delete [] cperc;

  }
  exit(0);
}


