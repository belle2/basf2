//+
// File : PrintData.cc
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/dataobjects/RawPXD.h>
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
  m_print_cnt = 0;
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
  B2INFO("PrintDataTemplate: initialize() done.");

}



//
// User defined functions
//


void PrintDataTemplateModule::printBuffer(int* buf, int nwords)
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


void PrintDataTemplateModule::printFTSWEvent(RawDataBlock* raw_datablock, int i)
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
  printf("eve %d TLU %d: %d %d %.8x: tv %d %d\n",
         rawftsw.GetEveNo(n),
         rawftsw.Get15bitTLUTag(n),
         rawftsw.GetBlockNwords(n),
         rawftsw.GetNwordsHeader(n),
         rawftsw.GetFTSWNodeID(n),
         tv.tv_sec, (int)(tv.tv_usec)
        );


#ifdef DEBUG
#endif
  m_nftsw++;
  return;
}


void PrintDataTemplateModule::printCOPPEREvent(RawCOPPER* raw_copper, int i)
{

  printf(": Event %8d node 0x%.8x block %d by: sum det %d by : A %d by B %d by C %d by D %d by\n",
         raw_copper->GetEveNo(i), raw_copper->GetNodeID(i),
         raw_copper->GetBlockNwords(i) * sizeof(int),
         sizeof(int) * (raw_copper->GetDetectorNwords(i, 0) + raw_copper->GetDetectorNwords(i, 1) +
                        raw_copper->GetDetectorNwords(i, 2) + raw_copper->GetDetectorNwords(i, 3)),
         sizeof(int) * (raw_copper->GetDetectorNwords(i, 0)),
         sizeof(int) * (raw_copper->GetDetectorNwords(i, 1)),
         sizeof(int) * (raw_copper->GetDetectorNwords(i, 2)),
         sizeof(int) * (raw_copper->GetDetectorNwords(i, 3))
        );
  printf("EventMetaData : exp %d run %d subrun %d eve %.8x\n", m_eventMetaDataPtr->getExperiment(),
         m_eventMetaDataPtr->getRun(), m_eventMetaDataPtr->getSubrun(), m_eventMetaDataPtr->getEvent());

  if (m_eventMetaDataPtr->getErrorFlag()) {
    printf("!!!!!!!!! ERROR event !!!!!!!!!! : eve %d errflag %.8x\n", raw_copper->GetEveNo(i), m_eventMetaDataPtr->getErrorFlag());
  }

  printBuffer(raw_copper->GetWholeBuffer(), raw_copper->TotalBufNwords());
  //
  // Print data from each FINESSE
  //
  if (raw_copper->GetDetectorNwords(i, 0) > 0) {
    printf("===== Detector Buffer(FINESSE A) 0x%x words \n", raw_copper->GetDetectorNwords(i, 0));
    printBuffer(raw_copper->GetDetectorBuffer(i, 0), raw_copper->GetDetectorNwords(i, 0));
  }

  if (raw_copper->GetDetectorNwords(i, 1) > 0) {
    printf("===== Detector Buffer(FINESSE B) 0x%x words \n", raw_copper->GetDetectorNwords(i, 1));
    printBuffer(raw_copper->GetDetectorBuffer(i, 1), raw_copper->GetDetectorNwords(i, 1));
  }

  if (raw_copper->GetDetectorNwords(i, 2) > 0) {
    printf("===== Detector Buffer(FINESSE C) 0x%x words \n", raw_copper->GetDetectorNwords(i, 2));
    printBuffer(raw_copper->GetDetectorBuffer(i, 2), raw_copper->GetDetectorNwords(i, 2));
  }

  if (raw_copper->GetDetectorNwords(i, 3) > 0) {
    printf("===== Detector Buffer(FINESSE D) 0x%x words \n", raw_copper->GetDetectorNwords(i, 3));
    printBuffer(raw_copper->GetDetectorBuffer(i, 3), raw_copper->GetDetectorNwords(i, 3));
  }

  m_ncpr++;

}

void PrintDataTemplateModule::printPXDEvent(RawPXD* raw_pxd)
{
  printf(": block size %d bytes : bebafeca = %04x :\n",
         (int)raw_pxd->size(), raw_pxd->data()[0]);
  printf("******* Raw PXD data block (including Detector Buffer) **********\n");
  printBuffer(raw_pxd->data(), raw_pxd->size());

  int* temp_buf = raw_pxd->data();
  vector <int> nframesv;
  int nframes = ((temp_buf[ 1 ] >> 24) & 0xff) | (((temp_buf[ 1 ] >> 16) & 0xff) << 8) |
                (((temp_buf[ 1 ] >> 8) & 0xff) << 16) | (((temp_buf[ 1 ] >> 0) & 0xff) << 24);
  ;
  for (int i = 0; i < nframes; i++) {
    int temp_nframes =
      ((temp_buf[ i + 2 ] >> 24) & 0xff) | (((temp_buf[ i + 2 ] >> 16) & 0xff) << 8) |
      (((temp_buf[ i + 2 ] >> 8) & 0xff) << 16) | (((temp_buf[ i + 2 ] >> 0) & 0xff) << 24);
    nframesv.push_back(temp_nframes);
  }

  //   printf("FRAME %.8x %.8x %.8x\n", nframes, temp_buf[ nframes +  10 ], temp_buf[ nframes +  11 ]);
  unsigned int onsen_trg = temp_buf[ nframes + 2 ];
  onsen_trg = ((onsen_trg >> 24) & 0xff) | (((onsen_trg >> 16) & 0xff) << 8) |
              (((onsen_trg >> 8) & 0xff) << 16) | (((onsen_trg >> 0) & 0xff) << 24);
  unsigned int hlttrg = ((temp_buf[ nframes + 4 ] >> 24) & 0xff) | (((temp_buf[ nframes + 4 ] >> 16) & 0xff) << 8) |
                        (((temp_buf[ nframes + 4 ] >> 8) & 0xff) << 16) | (((temp_buf[ nframes + 4 ] >> 0) & 0xff) << 24);
  //     unsigned int hlttrg = temp_buf[ nframes + 4 ];

  int ctime_type = 0;
  int dhe_time = 0;
  if (nframes != 0) {
    int pos = nframesv[ 0 ] / 4 + nframes + 2 ;
    //     printf("nf0 %d\n", nframesv[ 0 ]);
    for (int i = 1; i < nframesv.size(); i++) {
      if ((nframesv[ i ] % 4) != 0) break;
      int dhh_trg1 = (temp_buf[ pos ] >> 24) & 0xff;
      int dhh_trg2 = (temp_buf[ pos ] >> 16) & 0xff;
      int dhh_trg3 = (temp_buf[ pos + 1 ] >> 8) & 0xff;
      int dhh_trg4 = (temp_buf[ pos + 1 ] >> 0) & 0xff;
      unsigned int dhh_trg = dhh_trg1 | (dhh_trg2 << 8) | (dhh_trg3 << 16) | (dhh_trg4 << 24);

      //      if (i == 1) {
      if (((temp_buf[ pos ] >> 4)  & 0x7) == 5) {
        ctime_type = ((temp_buf[ pos + 2 ] & 0xff) << 24) |
                     (((temp_buf[ pos + 2 ] >> 8) & 0xff) << 16) |
                     (((temp_buf[ pos + 1 ] >> 16) & 0xff) << 8) |
                     (((temp_buf[ pos + 1 ] >> 24) & 0xff));
      }

      if (((temp_buf[ pos ] >> 4)  & 0x7) == 1) {
        dhe_time = ((temp_buf[ pos + 2 ] & 0xff) << 24) |
                   (((temp_buf[ pos + 2 ] >> 8) & 0xff) << 16) |
                   (((temp_buf[ pos + 1 ] >> 16) & 0xff) << 8) |
                   (((temp_buf[ pos + 1 ] >> 24) & 0xff));
      }

//         printf("Event mixing is occured. hlt %.8x dhh %.8x onsen %.8x %.8x %.8x nf %d\n", hlttrg, dhh_trg, onsen_trg,
//          temp_buf[ nframes + 2 + pos ], temp_buf[ nframes + 3 + pos ], nframesv[ i ] );
      printf("Event tag(16bits) in PXD data : frame %.2d hlt %.4x onsen %.4x dhh %.4x\n", i + 1, hlttrg & 0xffff, onsen_trg & 0xffff,
             dhh_trg & 0xffff);
      if (((dhh_trg & 0xffff) != (hlttrg & 0xffff)) || ((dhh_trg & 0xffff) != (onsen_trg & 0xffff))) {
        printf("### ERROR !! Event mixing occured. :  frame %d hlt %.4x onsen %.4x dhh %.4x\n", i + 1, hlttrg & 0xffff, onsen_trg & 0xffff,
               dhh_trg & 0xffff);
      }
      pos += nframesv[ i ] / 4;
    }
  }
  printf("PXD FTSW %u TRG %u DHE %u\n", ctime_type, hlttrg, dhe_time);


}

void PrintDataTemplateModule::event()
{


  B2INFO("aPrintDataTemplate: event() started.");
  //
  // FTSW + COPPER can be combined in the array
  //
  StoreArray<RawDataBlock> raw_datablkarray;


  for (int i = 0; i < raw_datablkarray.getEntries(); i++) {
    for (int j = 0; j < raw_datablkarray[ i ]->GetNumEntries(); j++) {
      int* temp_buf = raw_datablkarray[ i ]->GetBuffer(j);
      int nwords = raw_datablkarray[ i ]->GetBlockNwords(j);
      int delete_flag = 0;
      int num_nodes = 1;
      int num_events = 1;
      if (raw_datablkarray[ i ]->CheckFTSWID(j)) {
        // FTSW data block
        printf("\n===== DataBlock( RawDataBlock(FTSW) ) : Block # %d ", i);
        RawFTSW temp_raw_ftsw;
        temp_raw_ftsw.SetBuffer(temp_buf, nwords, delete_flag, num_nodes, num_events);
        printFTSWEvent(&temp_raw_ftsw, 0);
      } else if (raw_datablkarray[ i ]->CheckTLUID(j)) {
        // No operation
      } else {

        // COPPER data block
        printf("\n===== DataBlock( RawDataBlock(COPPER) ) : Block # %d ", i);
        RawCOPPER temp_raw_copper;
        temp_raw_copper.SetBuffer(temp_buf, nwords, delete_flag, num_nodes, num_events);
        printCOPPEREvent(&temp_raw_copper, 0);

      }
    }
  }

  //
  // TLU data
  //
  StoreArray<RawTLU> raw_tluarray;
  for (int i = 0; i < raw_tluarray.getEntries(); i++) {
    for (int j = 0; j < raw_tluarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawTLU) : Block # %d ", i);
      printFTSWEvent(raw_tluarray[ i ], j);
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
  //  Data from COPPER ( data from any detectors(e.g. CDC, SVD, ... ))
  //
  StoreArray<RawCOPPER> rawcprarray;
  for (int i = 0; i < rawcprarray.getEntries(); i++) {
    for (int j = 0; j < rawcprarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawCOPPER): Block # %d ", i);
      printCOPPEREvent(rawcprarray[ i ], j);
    }
  }

//
  // Data from COPPER named as RawSVD by software
  //
  StoreArray<RawSVD> raw_svdarray;
  for (int i = 0; i < raw_svdarray.getEntries(); i++) {
    for (int j = 0; j < raw_svdarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawSVD) : Block # %d ", i);
      printCOPPEREvent(raw_svdarray[ i ], j);
    }
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

  StoreArray<RawTOP> raw_bpidarray;
  for (int i = 0; i < raw_bpidarray.getEntries(); i++) {
    for (int j = 0; j < raw_bpidarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawTOP) : Block # %d ", i);
      printCOPPEREvent(raw_bpidarray[ i ], j);
    }
  }

  StoreArray<RawARICH> raw_epidarray;
  for (int i = 0; i < raw_epidarray.getEntries(); i++) {
    for (int j = 0; j < raw_epidarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawARICH) : Block # %d ", i);
      printCOPPEREvent(raw_epidarray[ i ], j);
    }
  }

  StoreArray<RawKLM> raw_klmarray;
  for (int i = 0; i < raw_klmarray.getEntries(); i++) {
    for (int j = 0; j < raw_klmarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawKLM) : Block # %d ", i);
      printCOPPEREvent(raw_klmarray[ i ], j);
    }
  }

  StoreArray<RawECL> raw_eclarray;
  for (int i = 0; i < raw_eclarray.getEntries(); i++) {
    for (int j = 0; j < raw_eclarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawECL) : Block # %d ", i);
      printCOPPEREvent(raw_eclarray[ i ], j);
    }
  }


  StoreArray<RawTRG> raw_trgarray;
  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    for (int j = 0; j < raw_trgarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawARICH) : Block # %d ", i);
      printCOPPEREvent(raw_trgarray[ i ], j);
    }
  }


  //  printf("loop %d\n", n_basf2evt);
  n_basf2evt++;

}
