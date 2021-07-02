/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <rawdata/modules/CheckErrorEvent.h>

using namespace std;
using namespace Belle2;

//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CheckErrorEvent)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CheckErrorEventModule::CheckErrorEventModule() : PrintDataTemplateModule()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  B2INFO("CheckErrorEvent: Constructor done.");

}



CheckErrorEventModule::~CheckErrorEventModule()
{
}

void CheckErrorEventModule::initialize()
{
  B2INFO("CheckErrorEvent: initialize() started.");
  m_packetCRCerr_cpr = 0;
  m_eventCRCerr_cpr = 0;
  m_packetCRCerr_evt = 0;
  m_eventCRCerr_evt = 0;
  m_eventCRCerr_evtmetadata = 0;
  B2INFO("CheckErrorEvent: initialize() done.");
}

void CheckErrorEventModule::checkCRCError(RawCOPPER* rawcpr, int i)
{
  for (int j = 0; j < rawcpr->GetNumEntries(); j++) {
    int temp_packet = rawcpr->GetPacketCRCError(j);
    int temp_event = rawcpr->GetEventCRCError(j);
    m_packetCRCerr_cpr = + temp_packet;
    m_eventCRCerr_cpr = + temp_event;
    if (temp_packet) {
      printf("Packet CRC error : block %d ent %d eve %.12u node %.8x\n", i, j,
             rawcpr->GetEveNo(j), rawcpr->GetNodeID(j));
    }
    if (temp_event) {
      printf("Packet EVE error : block %d ent %d eve %.12u node %.8x\n", i, j,
             rawcpr->GetEveNo(j), rawcpr->GetNodeID(j));
    }
  }
}

void CheckErrorEventModule::terminate()
{
  // printf("Packet CRC error %d CPRs %d Events\n",
  //        m_packetCRCerr_cpr,
  //        m_eventCRCerr_cpr);
  printf("Event  CRC error %d CPRs %d Events (obtained by checking each Raw*** header)\n",
         m_packetCRCerr_evt,
         m_eventCRCerr_evt
        );
  printf("Event  CRC error %d Events (obtained by checking EventMetaData. it should be equal to the above value. )\n",
         m_eventCRCerr_evtmetadata);
}

void CheckErrorEventModule::event()
{

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
        // No operation
      } else if (raw_datablkarray[ i ]->CheckTLUID(j)) {
        // No operation
      } else {
        // COPPER data block
        RawCOPPER temp_raw_copper;
        temp_raw_copper.SetBuffer(temp_buf, nwords, delete_flag, num_nodes, num_events);
        checkCRCError(&temp_raw_copper, i);
      }
    }
  }


  StoreArray<RawCOPPER> raw_cprarray;
  for (int i = 0; i < raw_cprarray.getEntries(); i++) {
    checkCRCError(raw_cprarray[ i ], i);
  }

  StoreArray<RawSVD> raw_svdarray;
  for (int i = 0; i < raw_svdarray.getEntries(); i++) {
    checkCRCError(raw_svdarray[ i ], i);
  }

  StoreArray<RawCDC> raw_cdcarray;
  for (int i = 0; i < raw_cdcarray.getEntries(); i++) {
    checkCRCError(raw_cdcarray[ i ], i);
  }

  StoreArray<RawTOP> raw_toparray;
  for (int i = 0; i < raw_toparray.getEntries(); i++) {
    checkCRCError(raw_toparray[ i ], i);
  }

  StoreArray<RawARICH> raw_aricharray;
  for (int i = 0; i < raw_aricharray.getEntries(); i++) {
    checkCRCError(raw_aricharray[ i ], i);
  }

  StoreArray<RawKLM> raw_klmarray;
  for (int i = 0; i < raw_klmarray.getEntries(); i++) {
    checkCRCError(raw_klmarray[ i ], i);
  }

  StoreArray<RawECL> raw_eclarray;
  for (int i = 0; i < raw_eclarray.getEntries(); i++) {
    checkCRCError(raw_eclarray[ i ], i);
  }

  StoreArray<RawTRG> raw_trgarray;
  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    checkCRCError(raw_trgarray[ i ], i);
  }



  StoreObjPtr<EventMetaData> evtmetadata;
  //  evtmetadata.create();
  if (evtmetadata->getErrorFlag()) {
    printf("EventMeta Error %.8x : exp %d run %d sub %d eve %d\n",
           evtmetadata->getErrorFlag(),
           evtmetadata->getExperiment(), evtmetadata->getRun(),
           evtmetadata->getSubrun(), evtmetadata->getEvent());
    m_eventCRCerr_evtmetadata++;

  }

  //  printf("loop %d\n", n_basf2evt);
  n_basf2evt++;

}
