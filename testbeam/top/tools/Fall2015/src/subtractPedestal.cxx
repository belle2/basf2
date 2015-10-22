/************************************************************************** 
 * BASF2 (Belle Analysis Framework 2)                                     * 
 * Copyright(C) 2015  Belle II Collaboration                              * 
 *                                                                        * 
 * Author: The Belle II Collaboration                                     * 
 * Contributors: Kodai Matsuoka                                           * 
 *                                                                        * 
 * This software is provided "as is" without any warranty.                * 
 **************************************************************************/

#include <stdio.h>
#include <iostream>

#include "TFile.h"
#include "TTree.h"

#define NUM_SAMPLES    64
//#define NUM_WINDOWS    512
//#define NUM_WINDOWS    16
#define NUM_WINDOWS    64

using namespace std;


int main(int argc, char **argv)
{
  if(argc<4) {
    printf("Usage: %s <input root file> <input pedestal file> <output root file>\n",argv[0]);
    exit(1);
  }

  // wave array - scrod, carrier, asic, channel, window, sample
//  unsigned int wave_ped[4][4][4][8][NUM_WINDOWS][NUM_SAMPLES];
  unsigned int ******wave_ped;
  wave_ped = (unsigned int ******)calloc(4, sizeof(unsigned int *****));
  for (int module=0; module<4; module++) {
    wave_ped[module] = (unsigned int *****)calloc(4, sizeof(unsigned int ****));
    for (int carrier=0; carrier<4; carrier++) {
      wave_ped[module][carrier] = (unsigned int ****)calloc(4, sizeof(unsigned int ***));
      for(int irsx=0; irsx<4; irsx++) {
        wave_ped[module][carrier][irsx] = (unsigned int ***)calloc(8, sizeof(unsigned int **));
        for(int channel=0; channel<8; channel++) {
          wave_ped[module][carrier][irsx][channel] = (unsigned int **)calloc(NUM_WINDOWS, sizeof(unsigned int *));
          for (int window=0; window<NUM_WINDOWS; window++) {
            wave_ped[module][carrier][irsx][channel][window] = (unsigned int *)calloc(NUM_SAMPLES, sizeof(unsigned int));
  } } } } }

  for(int module=0; module<4; module++) {
    for(int carrier=0; carrier<4; carrier++) {
      for(int irsx=0; irsx<4; irsx++) {
        for(int channel=0; channel<8; channel++) {
          for (int window=0; window<NUM_WINDOWS; window++) {
            for(int sample=0; sample<NUM_SAMPLES; sample++) {
              wave_ped[module][carrier][irsx][channel][window][sample]=0;
  } } } } } }

  Int_t runNum, nCh, nWindow;
  Short_t eModule[512], asicRow[512], asicCol[512], asicCh[512],
          firstWindow[512], ped[512][NUM_WINDOWS*NUM_SAMPLES];
  TFile *fped = TFile::Open(argv[2]);
  TTree *t_ped = (TTree*)fped->Get("pedestal");
  t_ped->SetBranchAddress("runNum",&runNum);
  t_ped->SetBranchAddress("nCh",&nCh);
  t_ped->SetBranchAddress("eModule",eModule);
  t_ped->SetBranchAddress("asicRow",asicRow);
  t_ped->SetBranchAddress("asicCol",asicCol);
  t_ped->SetBranchAddress("asicCh",asicCh);
  t_ped->SetBranchAddress("samples",ped);

  int nentry = t_ped->GetEntries();
  for (int i=0; i<nentry; i++) {
    t_ped->GetEntry(i);
    for (int c=0; c<nCh; c++) {
      for (int w=0; w<NUM_WINDOWS; w++) {
        for (int s=0; s<NUM_SAMPLES; s++) {
          wave_ped[eModule[c]][asicRow[c]][asicCol[c]][asicCh[c]][w][s] = ped[c][w*NUM_SAMPLES+s];
        }
      }
    }
  }

  TFile *fin = TFile::Open(argv[1]);
  TTree *t_in = (TTree*)fin->Get("rawdata");
  t_in->SetBranchAddress("runNum",&runNum);
  t_in->SetBranchAddress("nCh",&nCh);
  t_in->SetBranchAddress("nWindow",&nWindow);
  t_in->SetBranchAddress("eModule",eModule);
  t_in->SetBranchAddress("asicRow",asicRow);
  t_in->SetBranchAddress("asicCol",asicCol);
  t_in->SetBranchAddress("asicCh",asicCh);
  t_in->SetBranchAddress("firstWindow",firstWindow);
  t_in->GetEntry(0);
  const int nsample = nWindow*NUM_SAMPLES;
  Short_t adc[512][nsample];
  t_in->SetBranchAddress("samples",adc);

  TFile *fout = new TFile(argv[3],"recreate");
  Short_t adc_ps[512][nsample];
  char leaflist[256];
  TTree *t_out = (TTree*)t_in->CloneTree(0);
  TTree *t_out_ped = (TTree*)t_ped->CloneTree();
  sprintf(leaflist,"PSsamples[nCh][%d]/S",nsample);
  t_out->Branch("PSsamples",adc_ps,leaflist);

  nentry = t_in->GetEntries();
  for (int i=0; i<nentry; i++) {
    t_in->GetEntry(i);
    for (int c=0; c<nCh; c++) {
      for (int w=0; w<NUM_WINDOWS; w++) {
        if (!wave_ped[eModule[c]][asicRow[c]][asicCol[c]][asicCh[c]][w]) {
          cout << "Warning: no pedestal data for "
               << "mod" << eModule[c] << " row" << asicRow[c]
               << " col" << asicCol[c] << " ch" << asicCh[c] 
               << " win" << w << endl;
        }
        for (int s=0; s<NUM_SAMPLES; s++) {
          int window = (firstWindow[c]+w)%NUM_WINDOWS;
          adc_ps[c][w*NUM_SAMPLES+s]
          = adc[c][w*NUM_SAMPLES+s]
            - wave_ped[eModule[c]][asicRow[c]][asicCol[c]][asicCh[c]][window][s];
        }
      }
    }
    t_out->Fill();
  }

  for (int module=0; module<4; module++) {
    for (int carrier=0; carrier<4; carrier++) {
      for(int irsx=0; irsx<4; irsx++) {
        for(int channel=0; channel<8; channel++) {
          for (int window=0; window<NUM_WINDOWS; window++) {
            free(wave_ped[module][carrier][irsx][channel][window]);
          }
          free(wave_ped[module][carrier][irsx][channel]);
        }
        free(wave_ped[module][carrier][irsx]);
      }
      free(wave_ped[module][carrier]);
    }
    free(wave_ped[module]);
  }
  free(wave_ped);

  t_out->Write();
  t_out_ped->Write();
  fout->Close();
  fin->Close();
  fped->Close();
}
