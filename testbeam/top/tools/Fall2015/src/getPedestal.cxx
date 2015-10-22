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
  if(argc<3) {
    printf("Usage: %s <input root pedestal file> <output root pedestal file>\n",argv[0]);
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

  // number of waveforms collected for each window
  unsigned int num_ped[4][4][4][8][NUM_WINDOWS];

  Int_t runNum, nCh, nWindow;
  Short_t eModule[512], asicRow[512], asicCol[512], asicCh[512],
          firstWindow[512];
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

  for(int module=0; module<4; module++) {
    for(int carrier=0; carrier<4; carrier++) {
      for(int irsx=0; irsx<4; irsx++) {
        for(int channel=0; channel<8; channel++) {
          for (int window=0; window<NUM_WINDOWS; window++) {
            num_ped[module][carrier][irsx][channel][window]=0;
            for(int sample=0; sample<NUM_SAMPLES; sample++) {
              wave_ped[module][carrier][irsx][channel][window][sample]=0;
  } } } } } }

  int nentry = t_in->GetEntries();
  for (int i=0; i<nentry; i++) {
    t_in->GetEntry(i);
    for (int c=0; c<nCh; c++) {
      for (int w=0; w<nWindow; w++) {
        int window = (firstWindow[c]+w)%NUM_WINDOWS;
        num_ped[eModule[c]][asicRow[c]][asicCol[c]][asicCh[c]][window]++;
        for (int s=0; s<NUM_SAMPLES; s++) {
          wave_ped[eModule[c]][asicRow[c]][asicCol[c]][asicCh[c]][window][s]
          += adc[c][w*NUM_SAMPLES+s];
        }
      }
    }
  }

  char *filename = argv[1];
  strtok(filename,"/");
  while (1) {
    sprintf(filename,"%s",strtok(NULL,"/"));
    if (strstr(filename,".root")) break;
  }

  TFile *fout = new TFile(argv[2],"recreate");
  Short_t ped[512][NUM_WINDOWS*NUM_SAMPLES];
  char leaflist[256];
  TTree *t_ped = new TTree("pedestal",filename);
  t_ped->Branch("runNum",&runNum,"runNum/I");
  t_ped->Branch("nCh",&nCh,"nCh/I");
  t_ped->Branch("eModule",eModule,"eModule[nCh]/S");
  t_ped->Branch("asicRow",asicRow,"asicRow[nCh]/S");
  t_ped->Branch("asicCol",asicCol,"asicCol[nCh]/S");
  t_ped->Branch("asicCh",asicCh,"asicCh[nCh]/S");
  sprintf(leaflist,"samples[nCh][%d]/S",NUM_WINDOWS*NUM_SAMPLES);
  t_ped->Branch("samples",ped,leaflist);

  for (int c=0; c<512; c++) {
    for (int s=0; s<NUM_WINDOWS*NUM_SAMPLES; s++) {
      ped[c][s] = 0;
    }
  }

  nCh = 0;
  for(int module=0; module<4; module++) {
    for(int carrier=0; carrier<4; carrier++) {
      for(int irsx=0; irsx<4; irsx++) {
        for(int channel=0; channel<8; channel++) {
          for (int window=0; window<NUM_WINDOWS; window++) {
            if (num_ped[module][carrier][irsx][channel][window]) {
              eModule[nCh] = module;
              asicRow[nCh] = carrier;
              asicCol[nCh] = irsx;
              asicCh[nCh] = channel;
              for(int sample=0; sample<NUM_SAMPLES; sample++) {
                ped[nCh][window*NUM_SAMPLES+sample]
                = short(wave_ped[module][carrier][irsx][channel][window][sample]
                        /num_ped[module][carrier][irsx][channel][window]+0.5);
              }
            }
          }
          if (ped[nCh][0]) nCh++;
  } } } }
  t_ped->Fill();

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

  t_ped->Write();
  fout->Close();
  fin->Close();
}
