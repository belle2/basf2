//+
// File : PrintData.cc
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <daq/rawdata/modules/PrintData.h>
#include <framework/core/InputController.h>


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
}



PrintDataModule::~PrintDataModule()
{
}

void PrintDataModule::initialize()
{
  B2INFO("PrintData: initialize() started.");

  //  StoreArray<RawCOPPER>::registerTransient(s_auxMCParticlesName.c_str());
  //  StoreObjPtr<RawCOPPER>::registerTransient();
  // Initialize EvtMetaData
  //  m_eventMetaDataPtr.registerAsPersistent();
  // Create Message Handler
  m_msghandler = new MsgHandler(m_compressionLevel);
  B2INFO("PrintData: initialize() done.");

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
  int malloc_flag = 0; // No need to free the buffer
  int num_event = 1;
  int num_nodes = 1;
  rawftsw.SetBuffer(buf, nwords, malloc_flag, num_event, num_nodes);


  timeval tv;
  int n = 0;
  rawftsw.GetTTTimeVal(n , &tv);
  printf("eve %d TLU %d: %d %d %.8x: tv %d %d\n",
         rawftsw.GetEveNo(n),
         rawftsw.Get15bitTLUTag(n),
         rawftsw.GetNwords(n),
         rawftsw.GetNwordsHeader(n),
         rawftsw.GetFTSWNodeID(n),
         tv.tv_sec, tv.tv_usec
        );


#ifdef DEBUG
#endif
  m_nftsw++;
  return;
}

#ifndef REDUCED_RAWCOPPER
#else
// void PrintDataModule::printReducedCOPPEREvent(ReducedRawCOPPER* reduced_raw_copper, int i)
// {

//   printf(": Event # %d : node ID 0x%.8x : block size %d bytes\n",
//          reduced_raw_copper->GetEveNo(i), reduced_raw_copper->GetCOPPERNodeId(i),
//          reduced_raw_copper->GetBlockNwords(i) * sizeof(int));

//   //#ifdef DEBUG
//   printf("******* Reduced RawCOPPER data block(including Detector Buffer)**********: %d words\n",
//          reduced_raw_copper->GetBlockNwords(i));
//   printBuffer(reduced_raw_copper->GetBuffer(i), reduced_raw_copper->GetBlockNwords(i));

// //   if (!(reduced_raw_copper->CheckCOPPERMagic( i ))) {
// //     ErrorMessage print_err;
// //     char err_buf[500];
// //     sprintf(err_buf, "Invalid Magic word 0x7FFFF0008=%x 0xFFFFFAFA=%x 0xFFFFF5F5=%x 0x7FFF0009=%x\n",
// //             reduced_raw_copper->GetMagicDriverHeader( i ),
// //             reduced_raw_copper->GetMagicFPGAHeader( i ),
// //             reduced_raw_copper->GetMagicFPGATrailer( i ),
// //             reduced_raw_copper->GetMagicDriverTrailer( i ));
// //     print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
// //     sleep(12345678);
// //     exit(-1);
// //   }
//   //#endif

//   //
//   // Print data from each FINESSE
//   //
//   if (reduced_raw_copper->Get1stDetectorNwords(i) > 0) {
//     printf("== Detector Buffer(FINESSE A): nwords %d\n", reduced_raw_copper->Get1stDetectorNwords(i));
//     printBuffer(reduced_raw_copper->Get1stDetectorBuffer(i), reduced_raw_copper->Get1stDetectorNwords(i));
//   }

//   if (reduced_raw_copper->Get2ndDetectorNwords(i) > 0) {
//     printf("== Detector Buffer(FINESSE B)\n");
//     printBuffer(reduced_raw_copper->Get2ndDetectorBuffer(i), reduced_raw_copper->Get2ndDetectorNwords(i));
//   }

//   if (reduced_raw_copper->Get3rdDetectorNwords(i) > 0) {
//     printf("== Detector Buffer(FINESSE C)\n");
//     printBuffer(reduced_raw_copper->Get3rdDetectorBuffer(i), reduced_raw_copper->Get3rdDetectorNwords(i));
//   }

//   if (reduced_raw_copper->Get4thDetectorNwords(i) > 0) {
//     printf("== Detector Buffer(FINESSE D)\n");
//     printBuffer(reduced_raw_copper->Get4thDetectorBuffer(i), reduced_raw_copper->Get4thDetectorNwords(i));
//   }

//   m_ncpr++;

// }
#endif


void PrintDataModule::printCOPPEREvent(RawCOPPER* raw_copper, int i)
{

  printf(": Event # %d : node ID 0x%.8x : block size %d bytes\n",
         raw_copper->GetEveNo(i), raw_copper->GetCOPPERNodeId(i),
         raw_copper->GetBlockNwords(i) * sizeof(int));

  //#ifdef DEBUG
  printf("===== Raw COPPER data block(including 4 FINESSE buffers )\n");
  printBuffer(raw_copper->GetBuffer(i), raw_copper->GetBlockNwords(i));

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
  if (raw_copper->Get1stFINESSENwords(i) > 0) {
    printf("===== FINESSE  Buffer(FINESSE A) 0x%x words \n", raw_copper->Get1stDetectorNwords(i));
    printBuffer(raw_copper->Get1stFINESSEBuffer(i), raw_copper->Get1stFINESSENwords(i));
  }

  if (raw_copper->Get1stDetectorNwords(i) > 0) {
    printf("===== Detector Buffer(FINESSE A) 0x%x words \n", raw_copper->Get1stDetectorNwords(i));
    printBuffer(raw_copper->Get1stDetectorBuffer(i), raw_copper->Get1stDetectorNwords(i));
  }
  printf("\n");
  if (raw_copper->Get2ndFINESSENwords(i) > 0) {
    printf("===== FINESSE  Buffer(FINESSE B) 0x%x words \n", raw_copper->Get2ndDetectorNwords(i));
    printBuffer(raw_copper->Get2ndFINESSEBuffer(i), raw_copper->Get2ndFINESSENwords(i));
  }

  if (raw_copper->Get2ndDetectorNwords(i) > 0) {
    printf("====== Detector Buffer(FINESSE B)\n");
    printBuffer(raw_copper->Get2ndDetectorBuffer(i), raw_copper->Get2ndDetectorNwords(i));
  }
  printf("\n");
  if (raw_copper->Get3rdFINESSENwords(i) > 0) {
    printf("===== FINESSE  Buffer(FINESSE C) 0x%x words \n", raw_copper->Get3rdDetectorNwords(i));
    printBuffer(raw_copper->Get3rdFINESSEBuffer(i), raw_copper->Get3rdFINESSENwords(i));
  }

  if (raw_copper->Get3rdDetectorNwords(i) > 0) {
    printf("===== Detector Buffer(FINESSE C)\n");
    printBuffer(raw_copper->Get3rdDetectorBuffer(i), raw_copper->Get3rdDetectorNwords(i));
  }
  printf("\n");
  if (raw_copper->Get4thFINESSENwords(i) > 0) {
    printf("===== FINESSE  Buffer(FINESSE D) 0x%x words \n", raw_copper->Get4thDetectorNwords(i));
    printBuffer(raw_copper->Get4thFINESSEBuffer(i), raw_copper->Get4thFINESSENwords(i));
  }

  if (raw_copper->Get4thDetectorNwords(i) > 0) {
    printf("===== Detector Buffer(FINESSE D)\n");
    printBuffer(raw_copper->Get4thDetectorBuffer(i), raw_copper->Get4thDetectorNwords(i));
  }

  m_ncpr++;

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
  //
  // FTSW + COPPER can be combined in the array
  //
  StoreArray<RawDataBlock> raw_datablkarray;


  for (int i = 0; i < raw_datablkarray.getEntries(); i++) {
    for (int j = 0; j < raw_datablkarray[ i ]->GetNumEntries(); j++) {
      int* temp_buf = raw_datablkarray[ i ]->GetBuffer(j);
      int nwords = raw_datablkarray[ i ]->GetBlockNwords(j);
      int malloc_flag = 0;
      int num_nodes = 1;
      int num_events = 1;
      if (raw_datablkarray[ i ]->CheckFTSWID(j)) {
        // FTSW data block
        printf("\n===== DataBlock( RawDataBlock(FTSW) ) : Block # %d ", i);
        RawFTSW temp_raw_ftsw;
        temp_raw_ftsw.SetBuffer(temp_buf, nwords, malloc_flag, num_nodes, num_events);
        printFTSWEvent(&temp_raw_ftsw, 0);
      } else if (raw_datablkarray[ i ]->CheckTLUID(j)) {
        // No operation
      } else {

        // COPPER data block
        printf("\n===== DataBlock( RawDataBlock(COPPER) ) : Block # %d ", i);
        RawCOPPER temp_raw_copper;
        temp_raw_copper.SetBuffer(temp_buf, nwords, malloc_flag, num_nodes, num_events);
        printCOPPEREvent(&temp_raw_copper, 0);

      }
    }
  }

  //
  // FTSW data
  //
  StoreArray<RawFTSW> raw_ftswarray;
  for (int i = 0; i < raw_ftswarray.getEntries(); i++) {
    for (int j = 0; j < raw_ftswarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawFTSW): Block # %d ", i);
      printFTSWEvent(raw_ftswarray[ i ], j);
    }
  }



  //
  // Data from COPPER ( data from any detectors(e.g. CDC, SVD, ... ))
  //
  StoreArray<RawCOPPER> rawcprarray;
  for (int i = 0; i < rawcprarray.getEntries(); i++) {
    for (int j = 0; j < rawcprarray[ i ]->GetNumEntries(); j++) {
      printf("\n\n===== DataBlock(RawCOPPER): Block # %d ", i);
      printCOPPEREvent(rawcprarray[ i ], j);
    }
  }


//
  // Data from COPPER named as RawSVD by software
  //
  StoreArray<RawSVD> raw_svdarray;
  for (int i = 0; i < raw_svdarray.getEntries(); i++) {

    for (int j = 0; j < raw_svdarray[ i ]->GetNumEntries(); j++) {
//       printf("\n===== DataBlock(RawSVD) : Block # %d : tempval %d", i,
//       raw_svdarray[ i ]->m_temp_value );
      printf("\n===== DataBlock(RawSVD) : Block # %d\n", i);
      //      raw_svdarray[ i ]->ShowBuffer();
      printCOPPEREvent(raw_svdarray[ i ], j);
    }
    //    raw_svdarray[ i ]->m_temp_value = 12345678 + i ;
  }




  //
  // Data from COPPER named as RawCDC by software
  //
  StoreArray<RawCDC> raw_cdcarray;
  for (int i = 0; i < raw_cdcarray.getEntries(); i++) {
    for (int j = 0; j < raw_cdcarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawCDC) : Block # %d ", i);
      printCOPPEREvent(raw_cdcarray[ i ], j);
    }
  }

  //
  // Data from COPPER named as RawPXD by software
  //
  StoreArray<RawPXD> raw_pxdarray;
  for (int i = 0; i < raw_pxdarray.getEntries(); i++) {
    printf("\n===== DataBlock(RawPXD) : Block # %d ", i);
    printPXDEvent(raw_pxdarray[ i ]);
  }
  StoreArray<RawBPID> raw_bpidarray;
  StoreArray<RawEPID> raw_epidarray;
  StoreArray<RawKLM> raw_klmarray;
  StoreArray<RawECL> raw_eclarray;

  //  printf("loop %d\n", n_basf2evt);
  n_basf2evt++;

}
