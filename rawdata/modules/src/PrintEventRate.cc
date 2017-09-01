//+
// File : PrintEventRate.cc
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 18 - Jun - 2015
//-

#include <rawdata/modules/PrintEventRate.h>
#include <time.h>

using namespace std;
using namespace Belle2;

#define EVENT_UNIT 75000
#define BYTES_PER_WORD 4

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PrintEventRate)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PrintEventRateModule::PrintEventRateModule()
{
  addParam("PrintInterval", m_print_interval, "Print Interval", 60);

  m_prev_utime = 0;
  m_prev_event = 0;
  m_start_event = 0;
  m_start_utime = 0;
  m_run = 0;
  m_cur_utime = 0;
  m_cur_event = 0;
  m_tot_bytes = 0;
  m_prev_tot_bytes = 0;
  m_n_basf2evt = 0;
  m_first_evt = 0;

  m_erreve = 0;
  m_errcpr = 0;

}



PrintEventRateModule::~PrintEventRateModule()
{

}


void PrintEventRateModule::initialize()
{
  B2INFO("PrintDataTemplate: initialize() started.");

  //  StoreArray<RawCOPPER>::registerTransient(s_auxMCParticlesName.c_str());
  //  StoreObjPtr<RawCOPPER>::registerTransient();
  // Initialize EvtMetaData
  //  m_eventMetaDataPtr.registerPersistent();
  // Create Message Handler
  B2INFO("PrintDataTemplate: initialize() done.");

}



void PrintEventRateModule::printCOPPEREvent(RawCOPPER* raw_copper, int i)
{
  m_cur_utime = raw_copper->GetTTUtime(i);
  m_cur_event = raw_copper->GetEveNo(i);
  m_run = raw_copper->GetRunNo(i);
  m_subrun = raw_copper->GetSubRunNo(i);

  unsigned int error_flag = 0;
  error_flag = (unsigned int)(raw_copper->GetDataType(i));
  if (error_flag) {
    printf("!!!!!!!!! ERROR (RawCOPPER hdr) !!!!!!!!!! : run %d sub %d event %d errflag %.8x nodeID %.8x\n",
           m_run, m_subrun, m_cur_event, error_flag , raw_copper->GetNodeID(i));
    m_errcpr++;
  }

  //  printf("eve %d\n", m_cur_event );
  m_tot_bytes += (double)(BYTES_PER_WORD * raw_copper->GetBlockNwords(i));

  //  printf("TIME %u  start %u eve %d\n", m_cur_utime, m_start_utime, m_n_basf2evt );

  // fflush(stdout);
  if (m_first_evt == 0) {
    m_first_evt = 1;
    m_start_utime = m_cur_utime;
    m_start_event = m_cur_event;
    m_prev_utime = m_cur_utime;
    m_prev_event = m_cur_event;
    m_prev_tot_bytes = 0;
  }


}

void PrintEventRateModule::endRun()
{
  int interval = m_cur_utime - m_prev_utime;
  int total_time = m_cur_utime - m_start_utime;
  time_t timer = (time_t)m_cur_utime;
  struct tm t_st;
  //    time(&timer);
  localtime_r(&timer, &t_st);

  if (interval != 0) {
    printf("END %d/%d/%d/%d:%d:%d run %d sub %d Event %d Rate %lf [kHz] %lf [MB/s] RunTime %d [s] interval %d [s] total %lf [bytes] cpr %d s %d c %d to %d a %d e %d k %d tr %d other %d erreve %d errcpr %d\n",
           t_st.tm_year + 1900, t_st.tm_mon + 1, t_st.tm_mday, t_st.tm_hour, t_st.tm_min, t_st.tm_sec,
           m_run, m_subrun,   m_n_basf2evt, (double)(m_n_basf2evt  - m_prev_event) / interval / 1.e3,
           (double)(m_tot_bytes - m_prev_tot_bytes) / interval / 1.e6,
           total_time, interval, m_tot_bytes / 1.e6,
           m_cpr, m_cpr_svd, m_cpr_cdc, m_cpr_top, m_cpr_arich, m_cpr_ecl, m_cpr_klm, m_cpr_trg, m_cpr_others, m_erreve, m_errcpr);

  } else {
    printf("END %d/%d/%d/%d:%d:%d run %d sub %d Event %d Rate %lf [kHz] %lf [MB/s] RunTime %d [s] interval %d [s] total %lf [bytes] cpr %d s %d c %d to %d a %d e %d k %d tr %d other %d erreve %d errcpr %d\n",
           t_st.tm_year + 1900, t_st.tm_mon + 1, t_st.tm_mday, t_st.tm_hour, t_st.tm_min, t_st.tm_sec,
           m_run, m_subrun,   m_n_basf2evt, 0., 0.,
           total_time, interval, m_tot_bytes / 1.e6,
           m_cpr, m_cpr_svd, m_cpr_cdc, m_cpr_top, m_cpr_arich, m_cpr_ecl, m_cpr_klm, m_cpr_trg, m_cpr_others, m_erreve, m_errcpr);
  }
  fflush(stdout);
}

void PrintEventRateModule::event()
{
  StoreArray<RawCOPPER> raw_cprarray;
  StoreArray<RawFTSW> raw_ftswarray;
  StoreArray<RawDataBlock> raw_datablkarray;
  StoreArray<RawTLU> raw_tluarray;
  StoreArray<RawPXD> raw_pxdarray;
  StoreArray<RawSVD> raw_svdarray;
  StoreArray<RawCDC> raw_cdcarray;
  StoreArray<RawTOP> raw_toparray;
  StoreArray<RawARICH> raw_aricharray;
  StoreArray<RawECL> raw_eclarray;
  StoreArray<RawKLM> raw_klmarray;
  StoreArray<RawTRG> raw_trgarray;


  int datablk_blknum = raw_datablkarray.getEntries();
  int cpr_blknum = raw_cprarray.getEntries();
  int tlu_blknum = raw_tluarray.getEntries();
  int ftsw_blknum = raw_ftswarray.getEntries();
  int pxd_blknum = raw_pxdarray.getEntries();
  int svd_blknum = raw_svdarray.getEntries();
  int cdc_blknum = raw_cdcarray.getEntries();
  int top_blknum = raw_toparray.getEntries();
  int arich_blknum = raw_aricharray.getEntries();
  int ecl_blknum = raw_eclarray.getEntries();
  int klm_blknum = raw_klmarray.getEntries();
  int trg_blknum = raw_trgarray.getEntries();



  if (m_n_basf2evt == 0) {
    m_cpr =
      datablk_blknum +
      cpr_blknum +
      tlu_blknum +
      ftsw_blknum +
      pxd_blknum +
      svd_blknum +
      cdc_blknum +
      top_blknum +
      arich_blknum +
      ecl_blknum +
      klm_blknum +
      trg_blknum;

    m_cpr_svd = svd_blknum;
    m_cpr_cdc = cdc_blknum;
    m_cpr_top = top_blknum;
    m_cpr_arich = arich_blknum;
    m_cpr_ecl = ecl_blknum;
    m_cpr_klm = klm_blknum;
    m_cpr_trg = trg_blknum;
    m_cpr_others = m_cpr
                   - (m_cpr_svd
                      + m_cpr_cdc
                      + m_cpr_top
                      + m_cpr_arich
                      + m_cpr_ecl
                      + m_cpr_klm
                      + m_cpr_trg);


    // m_datablk_blksize = new int[datablk_blknum];
    // m_cpr_blksize = new int[cpr_blknum];
    // m_tlu_blksize = new int[tlu_blknum];
    // m_ftsw_blksize = new int[ftsw_blknum];
    // m_pxd_blksize = new int[pxd_blknum];
    // m_svd_blksize = new int[svd_blknum];
    // m_cdc_blksize = new int[cdc_blknum];
    // m_top_blksize = new int[top_blknum];
    // m_arich_blksize = new int[arich_blknum];
    // m_ecl_blksize = new int[ecl_blknum];
    // m_klm_blksize = new int[klm_blknum];
    // m_trg_blksize = new int[trg_blknum];

    // m_datablk_evecnt = new int[datablk_blknum];
    // m_cpr_evecnt = new int[cpr_blknum];
    // m_tlu_evecnt = new int[tlu_blknum];
    // m_ftsw_evecnt = new int[ftsw_blknum];
    // m_pxd_evecnt = new int[pxd_blknum];
    // m_svd_evecnt = new int[svd_blknum];
    // m_cdc_evecnt = new int[cdc_blknum];
    // m_top_evecnt = new int[top_blknum];
    // m_arich_evecnt = new int[arich_blknum];
    // m_ecl_evecnt = new int[ecl_blknum];
    // m_klm_evecnt = new int[klm_blknum];
    // m_trg_evecnt = new int[trg_blknum];
  }


  //
  // FTSW + COPPER can be combined in the array
  //
  for (int i = 0; i < datablk_blknum; i++) {
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
        //        printf("\n===== DataBlock( RawDataBlock(COPPER) ) : Block # %d ", i);
        RawCOPPER temp_raw_copper;
        temp_raw_copper.SetBuffer(temp_buf, nwords, delete_flag, num_nodes, num_events);
        printCOPPEREvent(&temp_raw_copper, 0);

      }
    }
  }

  for (int i = 0; i < cpr_blknum; i++) {
    for (int j = 0; j < raw_cprarray[ i ]->GetNumEntries(); j++) {
      printCOPPEREvent(raw_cprarray[ i ], j);
    }
  }

  for (int i = 0; i < svd_blknum; i++) {
    for (int j = 0; j < raw_svdarray[ i ]->GetNumEntries(); j++) {
      printCOPPEREvent(raw_svdarray[ i ], j);
    }
  }

  for (int i = 0; i < cdc_blknum; i++) {
    for (int j = 0; j < raw_cdcarray[ i ]->GetNumEntries(); j++) {
      printCOPPEREvent(raw_cdcarray[ i ], j);
    }
  }

  for (int i = 0; i < top_blknum; i++) {
    for (int j = 0; j < raw_toparray[ i ]->GetNumEntries(); j++) {
      printCOPPEREvent(raw_toparray[ i ], j);
    }
  }

  for (int i = 0; i < arich_blknum; i++) {
    for (int j = 0; j < raw_aricharray[ i ]->GetNumEntries(); j++) {
      printCOPPEREvent(raw_aricharray[ i ], j);
    }
  }

  for (int i = 0; i < ecl_blknum; i++) {
    for (int j = 0; j < raw_eclarray[ i ]->GetNumEntries(); j++) {
      printCOPPEREvent(raw_eclarray[ i ], j);
    }
  }

  for (int i = 0; i < klm_blknum; i++) {
    for (int j = 0; j < raw_klmarray[ i ]->GetNumEntries(); j++) {
      printCOPPEREvent(raw_klmarray[ i ], j);
    }
  }

  for (int i = 0; i < trg_blknum; i++) {
    for (int j = 0; j < raw_trgarray[ i ]->GetNumEntries(); j++) {
      printCOPPEREvent(raw_trgarray[ i ], j);
    }
  }

  if (m_eventMetaDataPtr->getErrorFlag()) {
    printf("!!!!!!!!! ERROR (EventMetaData) !!!!!!!!!! : run %d sub %d event %d errflag %.8x\n",
           m_eventMetaDataPtr->getRun(), m_eventMetaDataPtr->getSubrun(),
           m_eventMetaDataPtr->getEvent(), m_eventMetaDataPtr->getErrorFlag());
    m_erreve++;
  }

  int interval = (m_cur_utime - m_prev_utime);
  if (interval >= m_print_interval) {
    int total_time = m_cur_utime - m_start_utime;

    time_t timer = (time_t)m_cur_utime;
    struct tm t_st;
    //    time(&timer);
    localtime_r(&timer, &t_st);

    printf("MID %d/%d/%d/%d:%d:%d run %d sub %d Event %d Rate %lf [kHz] %lf [MB/s] RunTime %d [s] interval %d [s] total %lf [bytes]\n",
           t_st.tm_year + 1900, t_st.tm_mon + 1, t_st.tm_mday, t_st.tm_hour, t_st.tm_min, t_st.tm_sec,
           m_run, m_subrun,   m_n_basf2evt, (double)(m_n_basf2evt  - m_prev_event) / interval / 1.e3,
           (double)(m_tot_bytes - m_prev_tot_bytes) / interval / 1.e6,
           total_time, interval, m_tot_bytes / 1.e6);

    fflush(stdout);
    m_prev_utime = m_cur_utime;
    m_prev_tot_bytes = m_tot_bytes;
    m_prev_event = m_n_basf2evt;



    m_prev_tot_bytes = m_tot_bytes;
    m_prev_event = m_n_basf2evt;
    m_prev_utime = m_cur_utime;
  }

  //  printf("loop %d\n", n_basf2evt);
  m_n_basf2evt++;

}
