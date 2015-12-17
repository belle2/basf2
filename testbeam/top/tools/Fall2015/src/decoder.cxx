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
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <list>

#include "TFile.h"
#include "TTree.h"

#define NUM_CHANNELS   512
#define NUM_SAMPLES    64
//#define NUM_WINDOWS    512
#define NUM_WINDOWS    16
//#define NUM_WINDOWS    4
#define MAX_NWORD      0x1FFF


#define FOOTER  0x6c617374  // "last" in ASCII

using namespace std;


////////////////////////////////////////////////////////////

int getModuleCNumber() {
  // Helper function for next two functions to return correct scrod mappings for each module.
  // Sadly a "magic number" at present - Change to the (Construction) Number of the Module in question.
  return 3;
}


short map_Scrod_to_position(unsigned int Scrod){

  //LM: magic numbers! 
  
  // ModuleC01  
  if (14 == Scrod) {return 0;}
  if (15 == Scrod) {return 1;}
  if (16 == Scrod) {return 2;}
  if (13 == Scrod) {return 3;}

  // ModuleC03  
  if (42 == Scrod) {return 0;}
  if (26 == Scrod) {return 1;}
  if (30 == Scrod) {return 2;}
  if (25 == Scrod) {return 3;}

  // ModuleC04  
  if (45 == Scrod) {return 0;}
  if (50 == Scrod) {return 1;}
  if (48 == Scrod) {return 2;}
  if (46 == Scrod) {return 3;}

  std::cout << "Error! No Scrod with ID " << Scrod << " in module " << getModuleCNumber() << "!" << std::endl; 
  return -1;
  
}


short map_position_to_Scrod(unsigned int pos){

  //LM: magic numbers!
  if (1 == getModuleCNumber()) {
    if (0 == pos) {return 14;}
    if (1 == pos) {return 15;}
    if (2 == pos) {return 16;}
    if (3 == pos) {return 13;}
  }

  if (3 == getModuleCNumber()) {
    if (0 == pos) {return 42;}
    if (1 == pos) {return 26;}
    if (2 == pos) {return 30;}
    if (3 == pos) {return 25;}
  }  
  
  if (4 == getModuleCNumber()) {
    if (0 == pos) {return 45;}
    if (1 == pos) {return 50;}
    if (2 == pos) {return 48;}
    if (3 == pos) {return 46;}
  }

  std::cout << "Error in map_position_to_Scrod: boardstack position must be an integer betweeen 0 and 3 inclusive." << std::endl; 
  exit(1);

}


void map_channel_to_asic(int channel, short *eModule, short *asicRow, short *asicCol, short *asicCh) {
  *eModule = short(channel/128);
  *asicRow = short((channel%128)/32);
  *asicCol = short(((channel%128)%32)/8);
  *asicCh = short(channel%8);
}


int main(int argc, char **argv)
{
  // wave array - scrod, carrier, asic, window, channel, sample
  unsigned short wave[4][4][4][NUM_WINDOWS][8][NUM_SAMPLES];
  
  int event_hdr[2], window_hdr;
  unsigned int data[MAX_NWORD];
  int nword, num_asic;
  bool flag_err=0, next_event;

  unsigned int event_nword;
  unsigned int trig_pat, num_win;
  int event_counter[4][4]; // scrod, carrier
  unsigned int lastWrAddr, convertAddr;
  short scrod, module, carrier, irsx, channel, sample;
  unsigned short window[NUM_CHANNELS][NUM_WINDOWS];
  int buf, event_cam;
  list<int> ch_list;
  list<int>::iterator it;

  if(argc<3) {
    printf("Usage: %s <input raw file> <input camac file> <output root file>\n",argv[0]);
    printf("       %s <input raw file> <output root file>\n",argv[0]);
    exit(1);
  }
  bool withCamac = (argc==4) ? 1 : 0;

  FILE *fin_raw = fopen(argv[1],"r");
  ifstream fin_cam;
  if (withCamac) fin_cam.open(argv[2]);
  TFile *fout = (withCamac) ? new TFile(argv[3],"recreate"):
                              new TFile(argv[2],"recreate");

  Int_t runNum, eventNum, nCh, nWindow;
  Short_t scrodId[NUM_CHANNELS], eModule[NUM_CHANNELS],
          asicRow[NUM_CHANNELS], asicCol[NUM_CHANNELS], asicCh[NUM_CHANNELS],
          firstWindow[NUM_CHANNELS];
  Short_t ftsw;
  TTree *tree = new TTree("rawdata","IRSX waveform data");
  tree->Branch("runNum",&runNum,"runNum/I");
  tree->Branch("eventNum",&eventNum,"eventNum/I");
  if (withCamac) tree->Branch("ftsw",&ftsw,"ftsw/S");
  tree->Branch("nCh",&nCh,"nCh/I");
  tree->Branch("nWindow",&nWindow,"nWindow/I");
  tree->Branch("scrodId",scrodId,"scrodId[nCh]/S");
  tree->Branch("eModule",eModule,"eModule[nCh]/S");
  tree->Branch("asicRow",asicRow,"asicRow[nCh]/S");
  tree->Branch("asicCol",asicCol,"asicCol[nCh]/S");
  tree->Branch("asicCh",asicCh,"asicCh[nCh]/S");
  tree->Branch("firstWindow",firstWindow,"firstWindow[nCh]/S");

#if 1
  char *filename = argv[1];
  runNum = 0;
  if (strstr(filename,"run")) {
    strtok(filename,"/");
    while (1) {
      sprintf(filename,"%s",strtok(NULL,"/"));
      if (strstr(filename,".bin")) break;
    }
    strtok(filename,"_");
    strtok(filename,".");
    char runChar[5];
    strncpy(runChar,filename+3,5);
    runNum = atoi(runChar);
  }
#endif

  for(module=0; module<4; module++) {
    for(carrier=0; carrier<4; carrier++) {
      event_counter[module][carrier] = -1;
      for(irsx=0; irsx<4; irsx++) {
        for(convertAddr=0; convertAddr<NUM_WINDOWS; convertAddr++) {
          for(channel=0;channel<8;channel++) {
            for(sample=0;sample<NUM_SAMPLES;sample++)
              wave[module][carrier][irsx][convertAddr][channel][sample]=0;
  } } } } }

  eventNum = 0;
  event_cam = 0;
  while (!feof(fin_raw)) { // event loop
    ch_list.clear();

    eventNum++;
    if (withCamac) {
      fin_cam >> buf >> ftsw;
      event_cam++;
    }
  
    next_event = false;
    num_asic = 0;
    while (!feof(fin_raw)) { // asic loop
      if ((eventNum==1&&!num_asic) || num_asic) {
        fread(&buf, sizeof(int), 1, fin_raw); event_hdr[0] = buf;
        fread(&buf, sizeof(int), 1, fin_raw); event_hdr[1] = buf;
      }
      if (feof(fin_raw)) break;
      event_nword   =   event_hdr[0]+1;

      if ((eventNum==1&&!num_asic) || num_asic) {
        fread(data, sizeof(int), event_nword-2, fin_raw);
      }
      nword = 0;
      window_hdr = data[nword];
      carrier =    (window_hdr & 0xC0000000)>>30;
      scrod =      (window_hdr & 0x0000FE00)>>9;
      module = map_Scrod_to_position(scrod);
      if (module == -1) {
        flag_err = true; break;
//        exit(1);
      }

      if (event_counter[module][carrier] != (event_hdr[1] & 0x0007FFFF) &&
          event_counter[module][carrier] != -1) {
        for (int m=0; m<4; m++) for (int c=0; c<4; c++) event_counter[m][c] = -1;
        break;
      }

      event_counter[module][carrier] = event_hdr[1] & 0x0007FFFF;
      //event_counter[module][carrier] =  event_hdr[1] & 0x0FFFFFFF;
      trig_pat      =  (event_hdr[1] & 0xF0000000)>>28;
      num_win       = ((event_hdr[1] & 0x0FF80000)>>19)+1;

    // should add #windows to the header?
    //  num_win = 0;  // flag that we haven't read the number of windows yet!
 
      if (event_nword>MAX_NWORD) {
          cout << "event_nword(" << event_nword << ") is larger than " << MAX_NWORD
               << " (event: " << eventNum << ")" << endl;
        flag_err = true; break;
//        exit(1);
      }
      if (num_win>NUM_WINDOWS) {
          cout << "num_win(" << num_win << ") is larger than " << NUM_WINDOWS
               << " (event: " << eventNum << ")" << endl;
        flag_err = true; break;
//        exit(1);
      }

#if 0 // DEBUG : print header
      printf("\n");
      printf("Event %d: %08x  size=%d\n",
             eventNum, event_hdr[0], event_nword);
      printf("%08x  event_counter %08x: num_win=%d  trigpat=%x\n",
             event_hdr[1], event_counter[module][carrier], num_win, trig_pat);
#endif

      for (nWindow=0; nWindow<(int)num_win; nWindow++) { // window loop
        window_hdr = data[nword++];
        carrier =    (window_hdr & 0xC0000000)>>30;
        irsx =       (window_hdr & 0x30000000)>>28;
        lastWrAddr = (window_hdr & 0x00FF0000)>>16;
        scrod =      (window_hdr & 0x0000FE00)>>9;
        convertAddr=  window_hdr & 0x00001FF;
        module = map_Scrod_to_position(scrod);
        if (module == -1) {
          flag_err = true; break;
//          exit(1);
        }
  
#if 0 // DEBUG : print window parsed
        printf("%08x    Window:  Scrod %02d, carrier %d, irsx %d    lastWrAddr=%x   convertAddr=%x\n",
               window_hdr, scrod,
               carrier, irsx, lastWrAddr, convertAddr);
#endif

        for (channel=0; channel<8; channel++) { // channel loop
          int c = module*128+carrier*32+irsx*8+channel;
          ch_list.push_back(c);
          window[c][nWindow] = convertAddr;
#if 0
          short mod, row, col, ch;
          map_channel_to_asic(module*128+carrier*32+irsx*8+channel,&mod,&row,&col,&ch);
          assert(mod==module && row==carrier && col==irsx && ch==channel);
#endif
          for (sample=0; sample<NUM_SAMPLES/2; sample++) { // sample loop
            if ((data[nword]&0xf000) || (data[nword]&0xf0000000)) {
              cout << "Error: Event " << dec << eventNum << " data (0x" << hex << data[nword] << ") corruption" << endl;
              flag_err = true; break;
//              exit(1);
            }
            wave[module][carrier][irsx][nWindow][channel][2*sample] = (data[nword]&0xffff);
            wave[module][carrier][irsx][nWindow][channel][2*sample+1] = (data[nword]>>16);
//if (carrier==3 && irsx==0 && channel==7)
//            printf("evt%08d mod%d row%d col%d cha%02d(%03d) win%d sam%02d val%d\n", event_counter[module][carrier], module, carrier, irsx, channel, module*128+carrier*32+irsx*8+channel, convertAddr, 2*sample, wave[module][carrier][irsx][nWindow][channel][2*sample]);
            nword++;
          } // end of sample loop

          if (flag_err) break;
        } // end of channel loop

        ch_list.sort();
        ch_list.unique();
  
        if (flag_err) break;
      } // end of window loop

      if (data[nword]!=FOOTER) {
        cout << "ERROR: " << hex << data[nword] << " is not FOOTER (" << FOOTER << ")" << endl;
        flag_err = true; break;
//        eixt(1);
      }

      if ((int)event_nword-2!=nword+1 && !feof(fin_raw)) {
        cout << "Data size read out (" << nword << ") is inconsistent with " << event_nword << endl;
        flag_err = true; break;
//        exit(1);
      }

      num_asic++;
      if (flag_err) break;
    } // end of asic loop

    if (flag_err) break;

    if (withCamac && eventNum!=event_cam) {
      cout << "Event number is inconsistent between IRSX and CAMAC" << endl;
      flag_err = true; break;
//      exit(1);
    }

    Short_t adc[NUM_CHANNELS][nWindow*NUM_SAMPLES];
    nCh = 0;
    it = ch_list.begin();
    while (it != ch_list.end()) {
      map_channel_to_asic(*it,&eModule[nCh],&asicRow[nCh],&asicCol[nCh],&asicCh[nCh]);
      scrodId[nCh] = map_position_to_Scrod(eModule[nCh]);
      firstWindow[nCh] = window[*it][0];
      for (int n=0; n<nWindow; n++) {
        for (int s=0; s<NUM_SAMPLES; s++) {
          adc[nCh][n*NUM_SAMPLES+s] = wave[eModule[nCh]][asicRow[nCh]][asicCol[nCh]][n][asicCh[nCh]][s];
        }
      }
      nCh++;
      it++;
    }

    if (eventNum==1) {
      char leaflist[128];
      sprintf(leaflist,"samples[nCh][%d]/S",nWindow*NUM_SAMPLES);
      tree->Branch("samples",adc,leaflist);
    }

    tree->Fill();
  }

  tree->Write();
  fout->Close();
  if (withCamac) fin_cam.close();
  fclose(fin_raw);

  return 0;
}

