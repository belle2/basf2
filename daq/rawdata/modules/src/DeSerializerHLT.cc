/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/rawdata/modules/DeSerializerHLT.h>

// #include <daq/dataobjects/SendHeader.h>
// #include <daq/dataobjects/SendTrailer.h>
// #include <sys/mman.h>

//#define MAXEVTSIZE 400000000
#define CHECKEVT 5000

#define NOT_USE_SOCKETLIB
#define CLONE_ARRAY
//#define DISCARD_DATA
//#define CHECK_SUM
//#define TIME_MONITOR
//#define DEBUG

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DeSerializerHLT)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DeSerializerHLTModule::DeSerializerHLTModule() : DeSerializerPCModule()
{
  B2INFO("DeSerializerHLT: Constructor done.");
}


DeSerializerHLTModule::~DeSerializerHLTModule()
{
}



void DeSerializerHLTModule::initialize()
{
  B2INFO("DeSerializerHLT: initialize() started.");

  // Accept requests for connections
  Connect();

  // allocate buffer
  for (int i = 0 ; i < NUM_PREALLOC_BUF; i++) {
    m_bufary[i] = new int[ BUF_SIZE_WORD ];
  }
  m_buffer = new int[ BUF_SIZE_WORD ];


  // initialize buffer
  for (int i = 0 ; i < NUM_PREALLOC_BUF; i++) {
    memset(m_bufary[i], 0,  BUF_SIZE_WORD * sizeof(int));
  }

  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerInDataStore();

  // Initialize Array of RawCOPPER
  raw_datablkarray.registerInDataStore();
  rawcprarray.registerInDataStore();
  raw_cdcarray.registerInDataStore();
  raw_svdarray.registerInDataStore();
  raw_bpidarray.registerInDataStore();
  raw_epidarray.registerInDataStore();
  raw_eclarray.registerInDataStore();
  raw_klmarray.registerInDataStore();
  raw_ftswarray.registerInDataStore();



  if (m_dump_fname.size() > 0) {
    openOutputFile();
  }


  // Initialize arrays for time monitor
  memset(time_array0, 0, sizeof(time_array0));
  memset(time_array1, 0, sizeof(time_array1));
  memset(time_array2, 0, sizeof(time_array2));

  // initialize buffer number
  clearNumUsedBuf();

  B2INFO("DeSerializerHLT: initialize() done.");

}



void DeSerializerHLTModule::event()
{
  clearNumUsedBuf();

  if (m_start_flag == 0) {
    B2INFO("DeSerializerHLT: event() started.");
    m_start_time = getTimeSec();
    n_basf2evt = 0;
    m_start_flag = 1;
  }


  // DataStore interface
  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP_PC; j++) {
    // Get a record from socket
    int total_buf_nwords = 0 ;
    int delete_flag = 0;
    int num_events_in_sendblock = 0;
    int num_nodes_in_sendblock = 0;

    // Receive data
    int* temp_buf = recvData(&delete_flag, &total_buf_nwords,
                             &num_events_in_sendblock, &num_nodes_in_sendblock);
    RawCOPPER temp_rawcopper;
    temp_rawcopper.SetBuffer(temp_buf, total_buf_nwords, delete_flag, num_events_in_sendblock, num_nodes_in_sendblock);


    // Store data to DataStore
    for (int k = 0; k < temp_rawcopper.GetNumEvents(); k++) {
      for (int l = 0; l < temp_rawcopper.GetNumNodes(); l++) {
        int index = k * temp_rawcopper.GetNumEvents() + l;
        int buf_nwords = temp_rawcopper.GetBlockNwords(index);

        int* temp_buf2 = NULL;
        int delete_flag2 = 0;
        temp_buf2 = getNewBuffer(buf_nwords, &delete_flag2);
        memcpy(temp_buf2, temp_rawcopper.GetBuffer(index), sizeof(int)*buf_nwords);
        const int temp_num_events = 1;
        const int temp_num_nodes = 1;

        //  RawHeader hdr;
        // Fill data to Raw*** class

        switch (temp_rawcopper.GetNodeID(k * num_nodes_in_sendblock + l) & DETECTOR_MASK) {
          case CDC_ID :
            RawCDC* rawcdc;
            rawcdc = raw_cdcarray.appendNew();
            rawcdc->SetBuffer(temp_buf2, buf_nwords, delete_flag2, temp_num_events, temp_num_nodes);
            break;
          case SVD_ID :
            RawSVD* rawsvd;
            rawsvd = raw_svdarray.appendNew();
            rawsvd->SetBuffer(temp_buf2, buf_nwords, delete_flag2, temp_num_events, temp_num_nodes);
            break;
          case BECL_ID :
            RawECL* rawecl;
            rawecl = raw_eclarray.appendNew();
            rawecl->SetBuffer(temp_buf2, buf_nwords, delete_flag2, temp_num_events, temp_num_nodes);
            break;
          case TOP_ID :
            RawTOP* rawbpid;
            rawbpid = raw_bpidarray.appendNew();
            rawbpid->SetBuffer(temp_buf2, buf_nwords, delete_flag2, temp_num_events, temp_num_nodes);
            break;
          case ARICH_ID :
            RawARICH* rawepid;
            rawepid = raw_epidarray.appendNew();
            rawepid->SetBuffer(temp_buf2, buf_nwords, delete_flag2, temp_num_events, temp_num_nodes);
            break;
          case BKLM_ID :
            RawKLM* rawklm;
            rawklm = raw_klmarray.appendNew();
            rawklm->SetBuffer(temp_buf2, buf_nwords, delete_flag2, temp_num_events, temp_num_nodes);
            break;
          default :
            RawCOPPER* rawcopper;
            rawcopper = rawcprarray.appendNew();
            rawcopper->SetBuffer(temp_buf2, buf_nwords, delete_flag2, temp_num_events, temp_num_nodes);
            break;
        }
        // Fill header and trailer
      }
    }

    m_totbytes += total_buf_nwords * sizeof(int);

    if (delete_flag == 1) delete temp_buf;
  }

  //
  // Update EventMetaData
  //
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1);
  m_eventMetaDataPtr->setRun(1);
  m_eventMetaDataPtr->setEvent(n_basf2evt);

  n_basf2evt++;


  if (max_nevt >= 0 || max_seconds >= 0.) {
    if ((n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC >= max_nevt && max_nevt > 0)
        || (getTimeSec() - m_start_time > max_seconds && max_seconds > 0.)) {
      m_eventMetaDataPtr->setEndOfData();
    }
  }


  return;
}
