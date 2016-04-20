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
#include <stdlib.h>
#include <iostream>

#include "TFile.h"
#include "TTree.h"

#define NUM_SAMPLES    64
//#define NUM_WINDOWS    512
//#define NUM_WINDOWS    16
#define NUM_WINDOWS    64

#define THRESHOLD 40 // pulse discrimination threshold (ADC count)
#define NMIN_PULSE 4 // min. number of samples above threshold needed for discrimination
#define NMAX_PULSE 8 // max. number of samples above threshold needed for discrimination
#define CFD 0.5 // fraction of discrimination to the maximum height

using namespace std;


int main(int argc, char **argv)
{
  if(argc<3) {
    printf("Usage: %s <input root file> <output root file>\n",argv[0]);
    exit(1);
  }

  bool getRunNumberFromFileName(false);

  Int_t runNum, eventNum, nCh, nWindow;
  Short_t ftsw;
  Short_t id[512], module[512], carrier[512], irsx[512], channel[512],
          window0[512];
  TFile *fin = TFile::Open(argv[1]);
  TTree *t_in = (TTree*)fin->Get("rawdata");
  if (!getRunNumberFromFileName) {
    t_in->SetBranchAddress("runNum",&runNum);
  }
  t_in->SetBranchAddress("eventNum",&eventNum);
  t_in->SetBranchAddress("ftsw",&ftsw);
  t_in->SetBranchAddress("nCh",&nCh);
  t_in->SetBranchAddress("nWindow",&nWindow);
  t_in->SetBranchAddress("scrodId",id);
  t_in->SetBranchAddress("eModule",module);
  t_in->SetBranchAddress("asicRow",carrier);
  t_in->SetBranchAddress("asicCol",irsx);
  t_in->SetBranchAddress("asicCh",channel);
  t_in->SetBranchAddress("firstWindow",window0);
  t_in->GetEntry(0);
  const int nsample = nWindow*NUM_SAMPLES;
  Short_t wave[512][nsample];
  t_in->SetBranchAddress("PSsamples",wave);

  if (getRunNumberFromFileName) {
    runNum = 0;
    TString fileName(argv[1]);

    TObjArray* tokens;
    TObjString* objstring;   
    TString string;    
    tokens = fileName.Tokenize("/");
    objstring = (TObjString*) tokens->At(tokens->GetLast());
    if (objstring) {string = objstring->GetString();}
    tokens->Delete(); 
    
    tokens = string.Tokenize("run");      
    if (tokens->At(1)) {objstring = (TObjString*) tokens->At(1);}   
    if (objstring) {string = objstring->GetString();}
    tokens->Delete();

    tokens = string.Tokenize(".");    
    if (tokens->At(0)) {objstring = (TObjString*) tokens->At(0);}    
    if (objstring) {string = objstring->GetString();}
    tokens->Delete();
      
    if (string.IsDec()) {
      runNum = string.Atoi();
    } else {
      std::cerr << "Unable to determine run number from file name." << std::endl;
    }
    delete tokens;
  }



  Int_t nhit;
  Short_t scrodId[512], eModule[512], asicRow[512], asicCol[512], asicCh[512],
          firstWindow[512];
  Short_t pmt[512], pmtCh[512], ch[512];
  Float_t tdc[512];
  Int_t adc[512], peak[512], base[512];
  TFile *fout = new TFile(argv[2],"recreate");
  TTree *t_out = new TTree("top","IRSX summary data");
  t_out->Branch("runNum",&runNum,"runNum/I");
  t_out->Branch("eventNum",&eventNum,"eventNum/I");
  t_out->Branch("ftsw",&ftsw,"ftsw/S");
  t_out->Branch("nhit",&nhit,"nhit/I");
  t_out->Branch("scrodId",scrodId,"scrodId[nhit]/S");
  t_out->Branch("eModule",eModule,"eModule[nhit]/S");
  t_out->Branch("carrier",asicRow,"carrier[nhit]/S");
  t_out->Branch("asic",asicCol,"asic[nhit]/S");
  t_out->Branch("asicCh",asicCh,"asicCh[nhit]/S");
  t_out->Branch("firstWindow",firstWindow,"firstWindow[nhit]/S");
  t_out->Branch("pmt",pmt,"pmt[nhit]/S");
  t_out->Branch("pmtCh",pmtCh,"pmtCh[nhit]/S");
  t_out->Branch("ch",ch,"ch[nhit]/S");
  t_out->Branch("tdc",tdc,"tdc[nhit]/F");
  t_out->Branch("adc",adc,"adc[nhit]/I");
  t_out->Branch("peak",peak,"peak[nhit]/I");
  t_out->Branch("base",base,"base[nhit]/I");

  for (int n=0; n<512; n++) {
    adc[n] = 0;
    tdc[n] = 0;
    base[n] = 0;
  }
  int ped[NUM_WINDOWS];

  int nentry = t_in->GetEntries();
  for (int i=0; i<nentry; i++) {
    t_in->GetEntry(i);
    nhit = 0;
    for (int c=0; c<nCh; c++) {
      int flag = 0;
      float max = -1e4;
      for (int w=0; w<nWindow; w++) {
        int mean = 0;
        for (int s=0; s<NUM_SAMPLES; s++) mean += wave[c][w*NUM_SAMPLES+s];
        mean /= NUM_SAMPLES;
        int num = 0;
        ped[w] = 0;
        for (int s=0; s<NUM_SAMPLES; s++) {
          if (abs(wave[c][w*NUM_SAMPLES+s]-mean) < THRESHOLD) {
            ped[w] += wave[c][w*NUM_SAMPLES+s];
            num++;
          }
        }
        ped[w] = (num) ? int(ped[w]/num+0.5) : mean;
        peak[nhit] = ped[w];

        for (int s=0; s<NUM_SAMPLES; s++) {
          if (wave[c][w*NUM_SAMPLES+s]-ped[w] >= THRESHOLD) {
            flag++;
            adc[nhit] += wave[c][w*NUM_SAMPLES+s]-ped[w];
            if (wave[c][w*NUM_SAMPLES+s]-ped[w] > max) max = wave[c][w*NUM_SAMPLES+s]-ped[w];

          } else if (wave[c][w*NUM_SAMPLES+s]-ped[w] < THRESHOLD &&
                     flag >= NMIN_PULSE && flag <= NMAX_PULSE) {
            for (int ss=s-flag-2; ss<s; ss++) {
              int w1 = (ss>=0)  ? w : w-1;
              int w2 = (ss>=-1) ? w : w-1;
              if (wave[c][w*NUM_SAMPLES+ss]-ped[w1] <= max*CFD &&
                  wave[c][w*NUM_SAMPLES+ss+1]-ped[w2] > max*CFD) {
                tdc[nhit] = 
                  w*NUM_SAMPLES+ss
                  + (max*CFD-(wave[c][w*NUM_SAMPLES+ss]-ped[w1]))
                    /(wave[c][w*NUM_SAMPLES+ss+1]-ped[w2]
                      -(wave[c][w*NUM_SAMPLES+ss]-ped[w1]));
                base[nhit] = ped[w1];
                break;
              }
            }
            scrodId[nhit] = id[c];
            eModule[nhit] = module[c];
            asicRow[nhit] = carrier[c];
            asicCol[nhit] = irsx[c];
            asicCh[nhit] = channel[c];
            ch[nhit] = (irsx[c]%2==1)*channel[c]+(irsx[c]%2==0)*(7-channel[c]+64)+(irsx[c]>1)*8+carrier[c]*128+module[c]*16;
            pmt[nhit] = int((ch[nhit]%64)/4)+(ch[nhit]>255)*16;
            pmtCh[nhit] = (3-ch[nhit]%4)+int((ch[nhit]%256)/64)*4;
            ch[nhit]++;
            pmt[nhit]++;
            pmtCh[nhit]++;
            firstWindow[nhit] = window0[c];
            peak[nhit] = max;
            flag = 0;
            max = -1e4;
            nhit++;
            if (nhit>=512) {
              cout << "ERROR: number of hits exceeded the maximum (512)" << endl;
              nhit--;
//              exit(1);
            }

          } else {
            flag = 0;
            max = -1e4;
            adc[nhit] = 0;
          }
        }
      }
    }
    t_out->Fill();

    for (int n=0; n<nhit; n++) {
      adc[n] = 0;
      tdc[n] = 0;
      base[n] = 0;
    }
  }

  t_out->Write();
  fout->Close();
  fin->Close();
}
