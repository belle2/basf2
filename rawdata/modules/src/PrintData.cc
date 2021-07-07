/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <rawdata/modules/PrintData.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <framework/core/InputController.h>

#include <iostream>
#include <fstream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>


using namespace std;
using namespace Belle2;

//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PrintData)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PrintDataModule::PrintDataModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  n_basf2evt = -1;
  m_compressionLevel = 0;
  B2INFO("PrintData: Constructor done.");
  m_ncpr = 0;
  m_nftsw = 0;
  m_print_cnt = 0;
  m_start_utime = 0;
  for (int i = 0; i < 10; i++) hist[ i ] = NULL;
  for (int i = 0; i < 1000; i++) prev_tv_eve[ i ] = 0;
  prev_tv_pos = 0;
  for (int i = 0; i < 1000; i++) tv_flag[ i ] = 0;

}



PrintDataModule::~PrintDataModule()
{
}

void PrintDataModule::initialize()
{

  B2INFO("PrintData: initialize() started.");
  prev_tv_pos = 0;

  //  StoreArray<RawCOPPER>::registerTransient(s_auxMCParticlesName.c_str());
  //  StoreObjPtr<RawCOPPER>::registerTransient();
  // Initialize EvtMetaData
  //  m_eventMetaDataPtr.registerAsPersistent();
  // Create Message Handler
  B2INFO("PrintData: initialize() done.");


  char hname[100];


  char title[100];
  int size = 0;
  double min = 0.;
  double max = 100.;


  sprintf(title, "Event Number");
  sprintf(hname, "h_00");
  size = 1000;
  min = 0.;
  max = 1.e5;
  hist[ 0 ] = new TH1F(hname, title, size, min, max);
  sprintf(title, "Size per COPPER");
  sprintf(hname, "h_01");
  size = 1000;
  min = 0.;
  max = 1000.;
  hist[ 1 ] = new TH1F(hname, title, size, min, max);
  sprintf(title, "nodeid");
  sprintf(hname, "h_02");
  size = 50;
  min = 0.;
  max = 50.;
  hist[ 2 ] = new TH1F(hname, title, size, min, max);
  sprintf(hname, "h_03");
  sprintf(title, "event rate");
  size = 5000;
  min = 0.;
  max = 5000.;
  hist[ 3 ] = new TH1F(hname, title, size, min, max);
  sprintf(hname, "h_04");
  sprintf(title, "time difference");
  size = 300;
  min = 0.;
  max = 3.;
  hist[ 4 ] = new TH1F(hname, title, size, min, max);
  sprintf(title, "time difference");
  size = 2000;
  min = -0.01;
  max = 0.01;
  sprintf(hname, "h_05");
  hist[ 5 ] = new TH1F(hname, title, size, min, max);
  sprintf(hname, "h_06");
  hist[ 6 ] = new TH1F(hname, title, size, min, max);
  sprintf(hname, "h_07");
  hist[ 7 ] = new TH1F(hname, title, size, min, max);
  sprintf(hname, "h_08");
  hist[ 8 ] = new TH1F(hname, title, size, min, max);
  sprintf(hname, "h_09");
  hist[ 9 ] = new TH1F(hname, title, size, min, max);

}

void PrintDataModule::endRun()
{

  TFile f("temp.root", "RECREATE");
  //  TFile f(argv[1], "RECREATE");
  //  tree.Write();
  for (int i = 0; i < 10 ; i++) {
    hist[ i ]->Write();
    delete hist[ i ];
  }
  f.Close();

}




//
// User defined functions
//


void PrintDataModule::printBuffer(int* buf, int nwords)
{
  //  printf("%.8x :", 0);
  for (int j = 0; j < nwords; j++) {
    printf(" %.8x", buf[ j ]);
    if ((j + 1) % 10 == 0) {
      //      printf("\n%.8x :", j + 1);
      printf("\n");
    }
    m_print_cnt++;
  }
  printf("\n");

  return;
}


void PrintDataModule::printFTSWEvent(RawDataBlock* raw_datablock, int i)
{
  int* buf  = raw_datablock->GetBuffer(i);
  int nwords =  raw_datablock->GetBlockNwords(i);
  printf("*******FTSW data**********: nwords %d\n", nwords);
  printBuffer(buf, nwords);


  RawFTSW rawftsw;
  int delete_flag = 0; // No need to free the buffer
  int num_event = 1;
  int num_nodes = 1;
  rawftsw.SetBuffer(buf, nwords, delete_flag, num_event, num_nodes);


  timeval tv;
  int n = 0;
  rawftsw.GetTTTimeVal(n , &tv);
  printf("eve %u TLU %d: %d %d %.8x: tv %d %d\n",
         rawftsw.GetEveNo(n),
         rawftsw.Get15bitTLUTag(n),
         rawftsw.GetBlockNwords(n),
         rawftsw.GetNwordsHeader(n),
         rawftsw.GetFTSWNodeID(n),
         (int)(tv.tv_sec), (int)(tv.tv_usec)
        );


  m_nftsw++;
  return;
}


void PrintDataModule::printCOPPEREvent(RawCOPPER* raw_copper, int n, int array_index)
{

//   printf("%%%%%%%%%%%%%%%%%%%%%%%%%\n");
//   printBuffer(raw_copper->GetWholeBuffer(),raw_copper->TotalBufNwords());

  unsigned int eve = raw_copper->GetEveNo(n);

  if (array_index == 0 && n == 0) {
    timeval tv;
    raw_copper->GetTTTimeVal(n, &tv);
    //    printf("%.8x %.8x %x\n", raw_copper->GetTTUtime(n), raw_copper->GetTTCtimeTRGType(n), raw_copper->GetTTCtime(n));
    if (hist[3]->GetEntries() == 0) {
      m_start_utime =  raw_copper->GetTTUtime(n);
    }


    prev_tv[ prev_tv_pos ] = tv;
    prev_tv_eve[ prev_tv_pos ] = eve;
    tv_flag[ prev_tv_pos ] = 0;
    prev_tv_pos++;

    // 1
    for (int i = 0; i < prev_tv_pos ; i++) {
      if ((int)(eve - prev_tv_eve[ i ]) == 1) {
        float diff = (float)(tv.tv_sec - prev_tv[ i ].tv_sec) + (float)(tv.tv_usec - prev_tv[ i ].tv_usec) * 1.e-6 ;
        hist[ 4 ]->Fill(diff);
        hist[ 5 ]->Fill(diff);
        tv_flag[ i ] |= 0x2;
        tv_flag[ prev_tv_pos - 1 ] |= 0x1;
      }
      if ((int)(eve - prev_tv_eve[ i ]) == -1) {
        float diff = (float)(tv.tv_sec - prev_tv[ i ].tv_sec) + (float)(tv.tv_usec - prev_tv[ i ].tv_usec) * 1.e-6 ;
        hist[ 4 ]->Fill(-diff);
        hist[ 5 ]->Fill(-diff);
        tv_flag[ i ] |= 0x1;
        tv_flag[ prev_tv_pos - 1 ] |= 0x2;
      }
    }

    for (int i = 0; i < prev_tv_pos ; i++) {
      if (tv_flag[ i ] == 3) {
        for (int j = i; j < prev_tv_pos - 1 ; j++) {
          prev_tv[ j ] =  prev_tv[ j + 1 ];
          prev_tv_eve[ j ] =  prev_tv_eve[ j + 1 ];
          tv_flag[ j ] = tv_flag[ j + 1 ];
        }
        prev_tv_pos--;
      }
    }

    if (eve % 10000 == 0)   printf("1 %d %d\n", (int)(tv.tv_sec) - 1422134556, (int)eve);
    if (prev_tv_pos > 100) {
      for (int i = 0; i < prev_tv_pos ; i++) {
        printf(" a %d %u %d\n", i, prev_tv_eve[i ], tv_flag[ i ]);
      }
      exit(1);
    }




    hist[ 3 ]->Fill((float)(raw_copper->GetTTUtime(n) - m_start_utime));
  }

  hist[ 0 ]->Fill((float)(eve));
  hist[ 1 ]->Fill((float)(raw_copper->GetBlockNwords(n)));
  hist[ 2 ]->Fill((float)(raw_copper->GetNodeID(n) & 0xFFF));


  for (int i = 0; i < 4; i++) {
    if (raw_copper->Get1stFINESSENwords(n) > 0) {

    }
  }

  return;

  /* The following code is commented out since it's placed after a return
   * and it causes a cppcheck warning unreachableCode */

  /*
  printf(": Event # %u : node ID 0x%.8x : block size %d bytes\n",
         raw_copper->GetEveNo(n), raw_copper->GetNodeID(n),
         (int)(raw_copper->GetBlockNwords(n) * sizeof(int)));


  //#ifdef DEBUG
  printf("===== Raw COPPER data block(including 4 FINESSE buffers )\n");
  printBuffer(raw_copper->GetBuffer(n), raw_copper->GetBlockNwords(n));

  //   if (!(raw_copper->CheckCOPPERMagic( i ))) {
  //     ErrorMessage print_err;
  //     char err_buf[500];
  //     sprintf(err_buf, "Invalid Magic word 0x7FFFF0008=%x 0xFFFFFAFA=%x 0xFFFFF5F5=%x 0x7FFF0009=%x\n",
  //             raw_copper->GetMagicDriverHeader( i ),
  //             raw_copper->GetMagicFPGAHeader( i ),
  //             raw_copper->GetMagicFPGATrailer( i ),
  //             raw_copper->GetMagicDriverTrailer( i ));
  //     print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  //     sleep(12345678);
  //     exit(-1);
  //   }
  //#endif

  //
  // Print data from each FINESSE
  //

  m_ncpr++;
  */
}

void PrintDataModule::printPXDEvent(RawPXD* raw_pxd)
{
  printf(": block size %d bytes : bebafeca = %04x :\n",
         (int)raw_pxd->size(), raw_pxd->data()[0]);
  printf("******* Raw PXD data block (including Detector Buffer) **********\n");
  printBuffer(raw_pxd->data(), raw_pxd->size());
}

void PrintDataModule::event()
{

  B2INFO("aPrintData: event() started.");

  StoreArray<RawECL> raw_eclarray;
  for (int i = 0; i < raw_eclarray.getEntries(); i++) {
    for (int j = 0; j < raw_eclarray[ i ]->GetNumEntries(); j++) {
      //      printf("\n===== DataBlock(RawECL) : Block # %d ", i);
      printCOPPEREvent(raw_eclarray[ i ], j, i);
    }
  }



  //  printf("loop %d\n", n_basf2evt);
  n_basf2evt++;

}
