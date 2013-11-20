//+
// File : PrintData.cc
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/rawdata/modules/PrintDataTemplate.h>
#include <framework/core/InputController.h>

using namespace std;
using namespace Belle2;

//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PrintDataTemplate)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PrintDataTemplateModule::PrintDataTemplateModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  n_basf2evt = -1;
  m_compressionLevel = 0;
  B2INFO("PrintDataTemplate: Constructor done.");
  m_ncpr = 0;
  m_nftsw = 0;
}



PrintDataTemplateModule::~PrintDataTemplateModule()
{
}

void PrintDataTemplateModule::initialize()
{
  B2INFO("PrintDataTemplate: initialize() started.");

  //  StoreArray<RawCOPPER>::registerTransient(s_auxMCParticlesName.c_str());
  //  StoreObjPtr<RawCOPPER>::registerTransient();
  // Initialize EvtMetaData
  //  m_eventMetaDataPtr.registerAsPersistent();
  // Create Message Handler
  m_msghandler = new MsgHandler(m_compressionLevel);
  B2INFO("PrintDataTemplate: initialize() done.");

}



//
// User defined functions
//


void PrintDataTemplateModule::PrintData(int* buf, int nwords)
{
  //  printf("\n%.8d : ", 0);
  //  printf("%.8d : ", 0);
  for (int j = 0; j < nwords; j++) {
    printf("0x%.8x ", buf[ j ]);
    if ((j + 1) % 10 == 0) {
      //        printf("\n%.8d : ", j + 1);
      printf("\n");
      //      break;
    }
  }
  printf("\n");
  //  printf("\n");
  return;
}


void PrintDataTemplateModule::PrintFTSWEvent(RawDataBlock* raw_datablock, int i)
{
  int* buf  = raw_datablock->GetBuffer(i);
  int nwords =  raw_datablock->GetBlockNwords(i);
  printf("*******FTSW data**********: nwords %d\n", nwords);
  PrintData(buf, nwords);

  RawFTSW rawftsw;
  int malloc_flag = 0; // No need to free the buffer
  int num_event = 1;
  int num_nodes = 1;
  rawftsw.SetBuffer(buf, nwords, malloc_flag, num_event, num_nodes);

  int n = 0;
  printf("%d %d %.8x %.8x %lf\n",
         rawftsw.GetNwords(n),
         rawftsw.GetNwordsHeader(n),
         rawftsw.GetFTSWNodeID(n),
         rawftsw.GetTrailerMagic(n),
         rawftsw.GetEventUnixTime(n)
        );


#ifdef DEBUG
#endif
  m_nftsw++;
  return;
}


void PrintDataTemplateModule::PrintCOPPEREvent(RawCOPPER* raw_copper, int i)
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(raw_copper->GetRawHdrBufPtr(i));
  printf(": Event # %d : node ID 0x%.8x : block size %d bytes\n",
         raw_copper->GetEveNo(i), raw_copper->GetCOPPERNodeId(i),
         raw_copper->GetBlockNwords(i) * sizeof(int));

#ifdef DEBUG
  printf("******* Raw COPPER data block(including Detector Buffer)**********\n");
  PrintData(raw_copper->GetBuffer(i), raw_copper->GetBlockNwords(i));
#endif

  //
  // Print data from each FINESSE
  //
  if (raw_copper->Get1stDetectorNwords(i) > 0) {
    printf("== Detector Buffer(FINESSE A)\n");
    PrintData(raw_copper->Get1stDetectorBuffer(i), raw_copper->Get1stDetectorNwords(i));
  }

  if (raw_copper->Get2ndDetectorNwords(i) > 0) {
    printf("== Detector Buffer(FINESSE B)\n");
    PrintData(raw_copper->Get2ndDetectorBuffer(i), raw_copper->Get2ndDetectorNwords(i));
  }

  if (raw_copper->Get3rdDetectorNwords(i) > 0) {
    printf("== Detector Buffer(FINESSE C)\n");
    PrintData(raw_copper->Get3rdDetectorBuffer(i), raw_copper->Get3rdDetectorNwords(i));
  }

  if (raw_copper->Get4thDetectorNwords(i) > 0) {
    printf("== Detector Buffer(FINESSE D)\n");
    PrintData(raw_copper->Get4thDetectorBuffer(i), raw_copper->Get4thDetectorNwords(i));
  }

  m_ncpr++;

}


void PrintDataTemplateModule::event()
{
  B2INFO("PrintDataTemplate: event() started.");
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
        PrintFTSWEvent(&temp_raw_ftsw, 0);
      } else if (raw_datablkarray[ i ]->CheckTLUID(j)) {
        // No operation
      } else {
        // COPPER data block
        printf("\n===== DataBlock( RawDataBlock(COPPER) ) : Block # %d ", i);
        RawCOPPER temp_raw_copper;
        temp_raw_copper.SetBuffer(temp_buf, nwords, malloc_flag, num_nodes, num_events);
        PrintCOPPEREvent(&temp_raw_copper, 0);
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
      PrintFTSWEvent(raw_ftswarray[ i ], j);
    }
  }

  //
  // Data from COPPER ( data from any detectors(e.g. CDC, SVD, ... ))
  //
  StoreArray<RawCOPPER> rawcprarray;
  for (int i = 0; i < rawcprarray.getEntries(); i++) {
    for (int j = 0; j < rawcprarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawCOPPER): Block # %d ", i);
      PrintCOPPEREvent(rawcprarray[ i ], j);
    }
  }

  //
  // Data from COPPER named as RawSVD by software
  //
  StoreArray<RawSVD> raw_svdarray;
  for (int i = 0; i < raw_svdarray.getEntries(); i++) {
    for (int j = 0; j < raw_svdarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawSVD) : Block # %d ", i);
      PrintCOPPEREvent(raw_svdarray[ i ], j);
    }
  }

  //
  // Data from COPPER named as RawCDC by software
  //
  StoreArray<RawCDC> raw_cdcarray;
  for (int i = 0; i < raw_cdcarray.getEntries(); i++) {
    for (int j = 0; j < raw_cdcarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawCDC) : Block # %d ", i);
      PrintCOPPEREvent(raw_cdcarray[ i ], j);
    }
  }

  StoreArray<RawBPID> raw_bpidarray;
  StoreArray<RawEPID> raw_epidarray;
  StoreArray<RawKLM> raw_klmarray;
  StoreArray<RawECL> raw_eclarray;

  n_basf2evt++;

}
