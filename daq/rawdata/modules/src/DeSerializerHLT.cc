//+
// File : DeSerializerHLT.cc
// Description : Module to receive data from outside and store it to DataStore on HLT nodes
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 21 - Sep - 2013
//-

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
}


DeSerializerHLTModule::~DeSerializerHLTModule()
{
}



// void DeSerializerHLTModule::FillNewRawCOPPERHeader(RawCOPPER* raw_copper, int num_events, int num_nodes )
// {
//   const int num_cprblock = num_events * num_nodes; // On COPPER, 1 COPPER block will be stored in a RawCOPPER.

//   for( int i = 0; i < num_cprblock; i++){
//     RawHeader rawhdr;
//     rawhdr.SetBuffer( raw_copper->GetRawHdrBufPtr( i ) );

//     //
//     // initialize header(header nwords, magic word) and trailer(magic word)
//     //
//     rawhdr.Initialize(); // Fill 2nd( hdr size) and 20th header word( magic word )

//     // Set total words info
//     int nwords = raw_copper->GetCprBlockNwords( i );
//     rawhdr.SetNwords(nwords);

//     //
//     // Obtain info from SlowController via AddParam
//     //
//     rawhdr.SetExpNo(m_exp_no);   // Fill 3rd header word
//     rawhdr.SetRunNo(m_run_no);   // Fill 3rd header word

//     // Obtain eve.# from COPPER header
//     rawhdr.SetEveNo( raw_copper->GetCOPPEREveNo( i ) );     // Fill 4th header word
//     //  rawhdr.SetB2LFEEHdrPart(raw_copper->GetB2LFEEHdr1(), raw_copper->GetB2LFEEHdr2());   // Fill 5th and 6th words

//     // Obtain info from SlowController via AddParam or COPPER data
//     rawhdr.SetSubsysId( raw_copper->GetCOPPEREveNo( i ) );   // Fill 7th header word


//     rawhdr.SetDataType(m_data_type);   // Fill 8th header word
//     rawhdr.SetTruncMask(m_trunc_mask);   // Fill 8th header word

//     // Offset
//     rawhdr.SetOffset1stFINNESSE(raw_copper->GetOffset1stFINNESSE(num_cprblock) - raw_copper->GetBufferPos(num_cprblock));          // Fill 9th header word
//     rawhdr.SetOffset2ndFINNESSE(raw_copper->GetOffset2ndFINNESSE(num_cprblock) - raw_copper->GetBufferPos(num_cprblock));         // Fill 10th header word
//     rawhdr.SetOffset3rdFINNESSE(raw_copper->GetOffset3rdFINNESSE(num_cprblock) - raw_copper->GetBufferPos(num_cprblock));         // Fill 11th header word
//     rawhdr.SetOffset4thFINNESSE(raw_copper->GetOffset4thFINNESSE(num_cprblock) - raw_copper->GetBufferPos(num_cprblock));         // Fill 12th header word

//     // Add node-info
//     rawhdr.SetMagicWordEntireHeader();

//     // Add node-info
//     rawhdr.AddNodeInfo(m_nodeid);   // Fill 13th header word

//     //
//     // Fill info in Trailer
//     //
//     RawTrailer rawtrl;
//     rawtrl.SetBuffer(raw_copper->GetRawTrlBufPtr(num_cprblock));
//     rawtrl.Initialize(); // Fill 2nd word : magic word
//     rawtrl.SetChksum(CalcSimpleChecksum(raw_copper->GetBuffer(num_cprblock),
//          raw_copper->GetCprBlockNwords(num_cprblock) - rawtrl.GetTrlNwords()));

//     // Check magic words are set at proper positions

//   }

//   return;
// }


void DeSerializerHLTModule::event()
{
  if (n_basf2evt < 0) {
    m_start_time = GetTimeSec();
    n_basf2evt = 0;
  }


  raw_cdcarray.create();
//   raw_svdarray.create();
//   raw_eclarray.create();
//   raw_bpidarray.create();
//   raw_epidarray.create();
//   raw_klmarray.create();



  // DataStore interface


  for (int j = 0; j < NUM_EVT_PER_BASF2LOOP; j++) {
    // Get a record from socket
    int total_buf_nwords = 0 ;
    int malloc_flag = 0;
    int num_events_in_sendblock = 0;
    int num_nodes_in_sendblock = 0;
    int* temp_buf = RecvData(&malloc_flag, &total_buf_nwords,
                             &num_events_in_sendblock, &num_nodes_in_sendblock);


    //    RawCOPPER* temp_rawcopper;
    RawCOPPER* temp_rawcopper;
    temp_rawcopper->SetBuffer(temp_buf, total_buf_nwords, malloc_flag, num_events_in_sendblock, num_nodes_in_sendblock);

    for (int k = 0; k < num_events_in_sendblock; k++) {
      for (int l = 0; l < num_nodes_in_sendblock; l++) {

        switch (temp_rawcopper->GetSubsysId(k * num_nodes_in_sendblock + l) & DETECTOR_MASK) {
          case CDC_ID :
            RawCDC* temp_rawcdc;
            temp_rawcdc = raw_cdcarray.appendNew();
            //    temp_rawcdc->SetBuffer();
            break;
//  case SVD_ID :
//    temp_rawsvd = raw_svdarray.appendNew();
//    temp_rawsvd->SetBuffer();
//    break;
//  case ECL_ID :
//    temp_rawecl = raw_eclarray.appendNew();
//    temp_rawecl->SetBuffer();
//    break;
//  case BPID_ID :
//    temp_rawbpid = raw_bpidarray.appendNew();
//    temp_rawbpid->SetBuffer();
//    break;
//  case EPID_ID :
//    temp_rawepid = raw_epidarray.appendNew();
//    temp_rawepid->SetBuffer();
//    break;
//  case KLM_ID :
//    temp_rawklm = raw_klmarray.appendNew();
//    temp_rawklm->SetBuffer();
//    break;
          default :
            break;
        }


        // Fill header and trailer




      }
    }


    m_totbytes += total_buf_nwords * sizeof(int);



    if (malloc_flag == 1) delete temp_buf;

  }

  //
  // Update EventMetaData
  //
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(1);
  m_eventMetaDataPtr->setRun(1);
  m_eventMetaDataPtr->setEvent(n_basf2evt);

  n_basf2evt++;

//   if (max_nevt >= 0 || max_seconds >= 0.) {
//     if (n_basf2evt * NUM_EVT_PER_BASF2LOOP >= max_nevt && max_nevt > 0
//         ||  GetTimeSec() - m_start_time > max_seconds) {
//       m_eventMetaDataPtr->setEndOfData();
//     }
//   }

  return;
}
