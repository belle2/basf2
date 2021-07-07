/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/rawdata/modules/DummyDataSource.h>
#include <daq/rawdata/modules/DeSerializer.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DummyDataSource)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DummyDataSourceModule::DummyDataSourceModule() : Module()
{
  ///Set module properties
  setDescription("an Example to pack data to a RawCOPPER object");

  ///  maximum # of events to produce( -1 : inifinite)
  addParam("MaxEventNum", max_nevt, "Maximum event number to make", -1);

  ///  maximum # of events to produce( -1 : inifinite)
  addParam("NodeID", m_nodeid, "Node ID", 0);

  addParam("NodeName", m_nodename, "Node(subsystem) name", std::string(""));

  addParam("UseShmFlag", m_shmflag, "Use shared memory to communicate with Runcontroller", 0);

  B2INFO("DummyDataSource: Constructor done.");
}



DummyDataSourceModule::~DummyDataSourceModule()
{

}


void DummyDataSourceModule::initialize()
{
  B2INFO("DummyDataSource: initialize() started.");

  /// Initialize EvtMetaData
  m_eventMetaDataPtr.registerInDataStore();

#ifdef USE_RAWDATABLOCK
  raw_datablkarray.registerInDataStore();
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

  B2INFO("DummyDataSource: initialize() done.");
}




void DummyDataSourceModule::event()
{

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

  int* buf1, *buf2, *buf3, *buf4;
  int nwords_1st = 0, nwords_2nd = 0, nwords_3rd = 0, nwords_4th = 0;

  //  nwords_1st = n_basf2evt % 10;
  //  nwords_1st = n_basf2evt % 100 + 1;
  nwords_1st = 1;
  if (nwords_1st > 10000 || nwords_1st <= 0) {
    printf("ERRORORRRR nword1st %d\n", nwords_1st);
    fflush(stdout);
  }
  buf1 = new int[ nwords_1st];

  for (int i = 0; i < nwords_1st; i++) {
    if (i == 0) {
      buf1[ i ] = 0x12345678;
    } else {
      buf1[ i ] = (i + 1) * buf1[ i - 1 ];
    }
  }

  //  nwords_2nd = (n_basf2evt + 1) % 10;
  //  nwords_2nd = (n_basf2evt + 1) % 300 + 200;
  nwords_2nd = 1;
  if (nwords_2nd > 10000 || nwords_2nd <= 0) {
    printf("ERRORORRRR nword2nd %d\n", nwords_2nd);
    fflush(stdout);
  }

  buf2 = new int[ nwords_2nd];
  for (int i = 0; i < nwords_2nd; i++) {
    if (i == 0) {
      buf2[ i ] = 0x34567890;
    } else {
      buf2[ i ] = (i + 1) * buf2[ i - 1 ];
    }
  }

  //  nwords_3rd = 3 * (n_basf2evt + 2) % 10;
  //  nwords_3rd = 3 * (n_basf2evt + 2) % 200 + 100;
  nwords_3rd = 1;
  if (nwords_3rd > 10000 || nwords_3rd <= 0) {
    printf("ERRORORRRR nword3rd %d\n", nwords_3rd);
    fflush(stdout);
  }

  buf3 = new int[ nwords_3rd];
  for (int i = 0; i < nwords_3rd; i++) {
    if (i == 0) {
      buf3[ i ] = 0x56789012;
    } else {
      buf3[ i ] = (i + 1) * buf3[ i - 1 ];
    }
  }

  //  nwords_4th = 4 * (n_basf2evt + 3)  % 10;
  //  nwords_4th = 4 * (n_basf2evt + 3)  % 20 + 300;
  nwords_4th = 1;
  if (nwords_4th > 10000 || nwords_4th <= 0) {
    printf("ERRORORRRR nwor_4th %d\n", nwords_4th);
    fflush(stdout);
  }

  buf4 = new int[ nwords_4th];
  for (int i = 0; i < nwords_4th; i++) {
    if (i == 0) {
      buf4[ i ] = 0x78901234;
    } else {
      buf4[ i ] = (i + 1) * buf4[ i - 1 ];
    }
  }

#ifdef USE_RAWDATABLOCK
  raw_copper->PackDetectorBuf4DummyData(buf1, nwords_1st,
                                        buf2, nwords_2nd,
                                        buf3, nwords_3rd,
                                        buf4, nwords_4th,
                                        rawcprpacker_info);
#else
  raw_copper->PackDetectorBuf(buf1, nwords_1st,
                              buf2, nwords_2nd,
                              buf3, nwords_3rd,
                              buf4, nwords_4th,
                              rawcprpacker_info);

#endif

#ifdef USE_RAWDATABLOCK
  RawDataBlock* raw_datablk = raw_datablkarray.appendNew();
  raw_datablk->SetBuffer(raw_copper->GetWholeBuffer(), raw_copper->TotalBufNwords(),
                         false, 1, 1);
#endif

  delete [] buf1;
  delete [] buf2;
  delete [] buf3;
  delete [] buf4;

  //
  // Update EventMetaData : Not affect on the output
  //
  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setExperiment(rawcprpacker_info.exp_num);
  m_eventMetaDataPtr->setRun(rawcprpacker_info.run_subrun_num);
  m_eventMetaDataPtr->setEvent(n_basf2evt);


  //
  // Monitor
  //
  if (max_nevt >= 0) {
    if ((int)n_basf2evt >= max_nevt && max_nevt > 0) {
      printf("[DEBUG] RunStop was detected. ( Setting:  Max event # %d ) Processed Event %d \n",
             max_nevt , n_basf2evt);
      fflush(stdout);
      m_eventMetaDataPtr->setEndOfData();
    }
  }

  if (n_basf2evt % 10000 == 0) {
    double cur_time = getTimeSec();
    double interval = cur_time - m_prev_time;
    double total_time = cur_time - m_start_time;
    if (n_basf2evt == 0) {
      interval = 1;
    }
    printf("[INFO] Event %12d Rate %6.2lf[kHz] Recvd Flow %6.2lf[MB/s] RunTime %8.2lf[s] interval %8.4lf[s]\n",
           n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC,
           (n_basf2evt  - m_prev_nevt)*NUM_EVT_PER_BASF2LOOP_PC / interval / 1.e3,
           (m_totbytes - m_prev_totbytes) / interval / 1.e6,
           total_time,
           interval);
    fflush(stdout);
    m_prev_time = cur_time;
    m_prev_totbytes = m_totbytes;
    m_prev_nevt = n_basf2evt;


  }

  n_basf2evt++;
  RunInfoBuffer& status(DeSerializerModule::getStatus());
  if (status.isAvailable()) {
    status.addInputNBytes(1);
    status.setInputCount(n_basf2evt);
  }
  return;
}
