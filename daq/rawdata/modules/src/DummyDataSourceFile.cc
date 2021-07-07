/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/rawdata/modules/DummyDataSourceFile.h>
#include <daq/rawdata/modules/DeSerializer.h>
#include <rawdata/dataobjects/RawCDC.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DummyDataSourceFile)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DummyDataSourceFileModule::DummyDataSourceFileModule() : DummyDataSourceModule()
{
}



DummyDataSourceFileModule::~DummyDataSourceFileModule()
{

}


void DummyDataSourceFileModule::initialize()
{
  B2INFO("DummyDataSourceFile: initialize() started.");

  /// Initialize EvtMetaData
  //  m_eventMetaDataPtr.registerInDataStore();

#ifdef USE_RAWDATABLOCK
  //  raw_datablkarray.registerInDataStore();
#endif

  rawcprarray.registerInDataStore();

  //  n_basf2evt = 0;
  n_basf2evt = 0;
  m_start_flag = 0;

  if (m_shmflag > 0) {
    if (m_nodename.size() == 0 || m_nodeid < 0) {
      m_shmflag = 0;
    } else {
      printf("nodename = %s\n", m_nodename.c_str());
      RunInfoBuffer& status(DeSerializerModule::getStatus());
      status.open(m_nodename, m_nodeid);
    }
  }

  B2INFO("DummyDataSourceFile: initialize() done.");
}




void DummyDataSourceFileModule::event()
{

  StoreArray<RawCDC> raw_cdcarray;
  //    Make RawCOPPER array
  if (m_start_flag == 0) {
    m_start_flag = 1;
    RunInfoBuffer& status(DeSerializerModule::getStatus());
    if (status.isAvailable()) {
      B2INFO("DeSerializerCOPPER: Waiting for Start...\n");
      status.reportRunning();
    }
  }

  //
  // Fill event info (These values will be stored in RawHeader )
  //
  RawCOPPERPackerInfo rawcprpacker_info;
  rawcprpacker_info.exp_num = 1;
  rawcprpacker_info.run_subrun_num = 2; // run number : 14bits, subrun # : 8bits
  rawcprpacker_info.eve_num = n_basf2evt;
  rawcprpacker_info.node_id = m_nodeid;
  rawcprpacker_info.tt_ctime = 0x7123456;
  rawcprpacker_info.tt_utime = 0xF1234567;
  rawcprpacker_info.b2l_ctime = 0x7654321;


  //
  // Prepare buffer to fill dummy data
  //
  RawCOPPER* raw_copper = rawcprarray.appendNew();

  int* buf[4] = {0, 0, 0, 0};
  int nwords[4] = {0, 0, 0, 0};

  for (int i = 0; i < raw_cdcarray.getEntries(); i++) {
    for (int j = 0; j < raw_cdcarray[ i ]->GetNumEntries(); j++) {

      for (int k = 0; k < 4; k ++) {
        //  nwords_1st = n_basf2evt % 10;
        nwords[ k ] = raw_cdcarray[ i ]->GetDetectorNwords(j, k);
        buf[ k ] = raw_cdcarray[ i ]->GetDetectorBuffer(j, k);
        //    memset( buf[k], 0, nwords[k]*sizeof(int));
      }

#ifdef USE_RAWDATABLOCK
      raw_copper->PackDetectorBuf4DummyData(buf[0], nwords[0],
                                            buf[1], nwords[1],
                                            buf[2], nwords[2],
                                            buf[3], nwords[3],
                                            rawcprpacker_info);
#else
      raw_copper->PackDetectorBuf(buf[0], nwords[0],
                                  buf[1], nwords[1],
                                  buf[2], nwords[2],
                                  buf[3], nwords[3],
                                  rawcprpacker_info);

#endif


#ifdef USE_RAWDATABLOCK
      //  raw_datablkarray.create();
      RawDataBlock* raw_datablk = raw_datablkarray.appendNew();
      raw_datablk->SetBuffer(raw_copper->GetWholeBuffer(), raw_copper->TotalBufNwords(),
                             false, 1, 1);
#endif

    }
  }
  //
  // Monitor
  //
  if (max_nevt >= 0) {
    if (n_basf2evt >= max_nevt && max_nevt > 0) {
      printf("[DEBUG] RunStop was detected. ( Setting:  Max event # %d ) Processed Event %d \n",
             max_nevt , n_basf2evt);
      fflush(stdout);
      m_eventMetaDataPtr->setEndOfData();
    }
  }

  if (n_basf2evt % 1000 == 0) {
    printf("Dummy data : Event # %.8d\n", n_basf2evt);
    fflush(stdout);
  }

  n_basf2evt++;
  RunInfoBuffer& status(DeSerializerModule::getStatus());
  if (status.isAvailable()) {
    status.addInputNBytes(1);
    status.setInputCount(n_basf2evt);
  }
  return;
}
