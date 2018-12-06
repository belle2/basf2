//+
// File : DesSerCOPPER.cc
// Description : Module to receive data from eb0 and send it to eb1tx
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <daq/rawdata/modules/copper.h>
#include <daq/rawdata/DesSerCOPPER.h>

using namespace std;
using namespace Belle2;



//----------------------------------------------------------------
//                 Implementation
//----------------------------------------------------------------

DesSerCOPPER::DesSerCOPPER(string host_recv, int port_recv, string host_send, int port_send, int shmflag,
                           const std::string& nodename, int nodeid, int finesse_bitflag)
{
  m_finesse_bit_flag = finesse_bitflag;

  for (int i = 0 ; i < m_num_connections; i++) {
    //  m_hostname_from.push_back( "localhost");
    m_hostname_from.push_back(host_recv);
    //  m_port_from.push_back(30000);
    m_port_from.push_back(port_recv) ;
    m_socket_recv.push_back(-1);
  }

  //  m_port_to = 31001;
  m_port_to = port_send;
  //  m_hostname_local = "localhost";
  m_hostname_local = host_send;
  m_nodename = nodename;

  m_shmflag = shmflag;

  m_nodeid = nodeid;

  //  B2INFO("DeSerializerPrePC: Constructor done.");
  printf("[DEBUG] DeSerializerPrePC: Constructor done.\n"); fflush(stdout);
}



DesSerCOPPER::~DesSerCOPPER()
{
}

void DesSerCOPPER::DataAcquisition()
{
  // For data check
  unsigned int eve_copper_0 = 0;
  //  B2INFO("initializing...");
  printf("[DEBUG] initializing...\n"); fflush(stdout);
  initialize(false);


  //  B2INFO("Done.");
  printf("[DEBUG] Done.\n"); fflush(stdout);

  unsigned int m_prev_ftsweve32 = 0xffffffff;

  if (m_start_flag == 0) {
    //
    // Connect to eb0: This should be here because we want Serializer first to accept connection from eb1tx
    //
    initializeCOPPER();
    //    Connect();

    if (m_status.isAvailable()) {
      //      B2INFO("DeSerializerPrePC: Waiting for Start...\n");
      printf("[DEBUG] DeSerializerPrePC: Waiting for Start...\n"); fflush(stdout);
      m_status.reportRunning();
    }
    m_start_time = getTimeSec();
    n_basf2evt = 0;
  }

  //
  // Main loop
  //
  while (1) {
    //
    // Stand-by loop
    //
#ifdef NONSTOP
    if (m_run_pause > 0 || m_run_error > 0) {
      waitResume();
    }
#endif

    clearNumUsedBuf();
    //    RawDataBlock raw_datablk[ NUM_EVT_PER_BASF2LOOP_PC ];
    RawDataBlockFormat raw_datablk[ NUM_EVT_PER_BASF2LOOP_PC ];


    //
    // Recv loop
    //
    for (int j = 0; j < NUM_EVT_PER_BASF2LOOP_PC; j++) {
      //
      // Receive data from COPPER
      //
      eve_copper_0 = 0;
      int delete_flag_from =
        0; // Delete flag for temp_rawdatablk.It can be set to 1 by setRecvdBuffer if the buffer size is larger than that of pre-allocated buffer.
      int delete_flag_to =
        0; // Delete flag for raw_datablk[i]. It can be set to 1 by getNewBuffer if the buffer size is larger than that of pre-allocated buffer.
      RawDataBlockFormat temp_rawdatablk;
      try {

        int delete_flag = 0, m_size_word = 0;

        if (m_start_flag == 0) {
          printf("[DEBUG] DeSerializerCOPPER: Reading the 1st event from COPPER FIFO... run %.8x eve %.8x\n", m_prev_exprunsubrun_no,
                 m_prev_ftsweve32); fflush(stdout);
        }
        int* temp_buf;

        temp_buf = readOneEventFromCOPPERFIFO(j, &delete_flag, &m_size_word);
        m_status.copyEventHeader(temp_buf);
        if (m_start_flag == 0) {
          printf("[DEBUG] DeSerializerCOPPER: Done. the size of the 1st event is %d words\n", m_size_word); fflush(stdout);
        }
        const int num_nodes = 1;
        const int num_events = 1;
        PreRawCOPPERFormat_latest temp_rawcopper;
        temp_rawcopper.SetBuffer(temp_buf, m_size_word, 0, num_events, num_nodes);
        // Fill header and trailer
        try {
          m_prev_ftsweve32 = temp_rawcopper.FillTopBlockRawHeader(m_nodeid, m_prev_ftsweve32, m_prev_exprunsubrun_no, &m_exprunsubrun_no);
          m_prev_exprunsubrun_no = m_exprunsubrun_no;
          //    fillNewRawCOPPERHeader( &temp_rawcopper );
        } catch (string err_str) {
          print_err.PrintError(m_shmflag, &m_status, err_str);
          exit(1);
        }

//  if (m_dump_fname.size() > 0) {
//    dumpData((char*)temp_buf, m_size_word * sizeof(int));
//  }
        m_recvd_totbytes += m_size_word * sizeof(int);

        //  temp_rawdblk =  raw_dblkarray.appendNew();
        //  temp_rawdatablk->SetBuffer(temp_buf, m_size_word, delete_flag, num_events, num_nodes);
        raw_datablk[ j ].SetBuffer(temp_buf, m_size_word, delete_flag, num_events, num_nodes);

      } catch (string err_str) {
        printf("Error was detected\n"); fflush(stdout);
        break;
      }
    }

#ifdef NONSTOP
    // Goto Stand-by loop when run is paused or stopped by error
    if (m_run_pause != 0 || m_run_error != 0) continue;
#endif


    ///////////////////////////////////////////////////////////////
    // From Serializer.cc
    //////////////////////////////////////////////////////////////
    if (m_start_flag == 0) {
      m_start_time = getTimeSec();
      n_basf2evt = 0;
    }

    //
    // Send data
    //
    for (int j = 0; j < NUM_EVT_PER_BASF2LOOP_PC; j++) {
      if (m_start_flag == 0) {
        //        B2INFO("SerializerPC: Sending the 1st packet...");
        printf("[DEBUG] SerializerPC: Sending the 1st packet...\n"); fflush(stdout);
      }
      try {
        m_sent_totbytes += sendByWriteV(&(raw_datablk[ j ]));
      } catch (string err_str) {
#ifdef NONSTOP
        break;
#endif
        print_err.PrintError((char*)err_str.c_str(), __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(1);
      }
      if (m_start_flag == 0) {
        //        B2INFO("Done. ");
        printf("[DEBUG] Done.\n"); fflush(stdout);
        m_start_flag = 1;
      }
    }

#ifdef NONSTOP
    // Goto Stand-by loop when run is paused or stopped by error
    if (m_run_pause != 0 || m_run_error != 0) continue;
#endif

    //
    // Monitor
    //
    if (max_nevt >= 0 || max_seconds >= 0.) {
#ifdef AIUEO
      if (n_basf2evt % 10000 == 0) {
//     if ((n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC >= max_nevt && max_nevt > 0)
//         || (getTimeSec() - m_start_time > max_seconds && max_seconds > 0.)) {
        printf("[DEBUG] RunStop was detected. ( Setting:  Max event # %d MaxTime %lf ) Processed Event %d Elapsed Time %lf[s]\n",
               max_nevt , max_seconds, n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC, getTimeSec() - m_start_time);
      }
#endif
    }

    if ((n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC) % 100000 == 0) {
      double interval = cur_time - m_prev_time;
      double total_time = cur_time - m_start_time;
      printf("[DEBUG] Event %12d Rate %6.2lf[kHz] Recvd %6.2lf[MB/s] sent %6.2lf[MB/s] RunTime %8.2lf[s] interval %8.4lf[s]\n",
             n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC,
             (n_basf2evt  - m_prev_nevt)*NUM_EVT_PER_BASF2LOOP_PC / interval / 1.e3,
             (m_recvd_totbytes - m_recvd_prev_totbytes) / interval / 1.e6,
             (m_sent_totbytes - m_sent_prev_totbytes) / interval / 1.e6,
             total_time,
             interval);
      fflush(stdout);

      m_prev_time = cur_time;
      m_recvd_prev_totbytes = m_recvd_totbytes;
      m_sent_prev_totbytes = m_sent_totbytes;
      m_prev_nevt = n_basf2evt;
      cur_time = getTimeSec();
    }

    n_basf2evt++;

    if (m_status.isAvailable()) {
      m_status.setOutputNBytes(m_sent_totbytes);
      m_status.setOutputCount(n_basf2evt * NUM_EVT_PER_BASF2LOOP_PC);
    }

  }

  return;
}

/////////////////////////////////////////////////////
//   From Serializer.cc
/////////////////////////////////////////////////////

#ifdef NONSTOP
void DesSerCOPPER::waitResume()
{
  if (m_run_pause == 0) {
    while (true) {
      if (checkRunPause()) break;
#ifdef NONSTOP_DEBUG
      printf("\033[31m");
      printf("###########(DesSerCOPPER) Waiting for Runstop()  ###############\n");
      fflush(stdout);
      printf("\033[0m");
#endif
      sleep(1);
    }
  }

  // close COPPER FIFO
  if (m_cpr_fd != -1) close(m_cpr_fd);
  m_cpr_fd = -1;

  while (true) {
#ifdef NONSTOP_DEBUG
    printf("\033[31m");
    printf("###########(Ser) Waiting for Resume ###############\n");
    fflush(stdout);
    printf("\033[0m");
#endif
    if (checkRunRecovery()) {
      m_run_pause = 0;
      m_run_error = 0;

      break;
    }
    sleep(1);
  }

  printf("Done!\n"); fflush(stdout);

  if (CheckConnection(m_socket_send) < 0) {
    printf("Trying Accept1\n"); fflush(stdout);
    Accept(false);
    printf("Trying Accept2\n"); fflush(stdout);
  }

  resumeRun();
  return;
}



void DesSerCOPPER::resumeRun()
{
  initializeCOPPER();
#ifdef NONSTOP_DEBUG
  printf("\033[34m");
  printf("###########(Ser) the 1st event sicne the resume  ###############\n");
  fflush(stdout);
  printf("\033[0m");
#endif
  m_run_error = 0;
  m_run_pause = 0;
  return;
}



#endif



int* DesSerCOPPER::readOneEventFromCOPPERFIFO(const int entry, int* delete_flag, int* m_size_word)
{

  // prepare buffer
  *m_size_word = 0;
  int* temp_buf = m_bufary[ entry ];
  temp_buf[0] =  BUF_SIZE_WORD ;
  *delete_flag = 0;

  //
  // Read data from HSLB
  //
#ifndef DUMMY
  int recvd_byte = (m_pre_rawcpr.tmp_header.RAWHEADER_NWORDS) * sizeof(int);
  // Firstly, read data with an allocated buffer.
  while (1) {
    int read_size = 0;
    if ((read_size = read(m_cpr_fd, (char*)m_bufary[entry] + recvd_byte, sizeof(int) *  BUF_SIZE_WORD  - recvd_byte)) < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {

        if (recvd_byte > (int)((m_pre_rawcpr.tmp_header.RAWHEADER_NWORDS) * sizeof(int))) {
          char err_buf[500];
          sprintf(err_buf, "[FATAL] EAGAIN return in the middle of an event( COPPER driver should't do this.). Exting...");
          print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
          exit(-1);
        }

#ifdef NONSTOP
        // Check run-pause request from SLC
        string err_str;
        callCheckRunPause(err_str);
#endif
        continue;

      } else {
        char err_buf[500];
        sprintf(err_buf, "[FATAL] Failed to read data from COPPER. Exiting...");
        print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(-1);
      }
    } else {
      recvd_byte += read_size;
      if (recvd_byte - (m_pre_rawcpr.tmp_header.RAWHEADER_NWORDS) * sizeof(int) > (int)(sizeof(int) *
          (m_pre_rawcpr.POS_DATA_LENGTH + 1)))break;
    }
  }

  //
  // Calcurate data size
  //
  *m_size_word = m_bufary[ entry ][ m_pre_rawcpr.POS_DATA_LENGTH + (m_pre_rawcpr.tmp_header.RAWHEADER_NWORDS) ]
                 + m_pre_rawcpr.SIZE_COPPER_DRIVER_HEADER + m_pre_rawcpr.SIZE_COPPER_DRIVER_TRAILER
                 + m_pre_rawcpr.tmp_header.RAWHEADER_NWORDS +
                 m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS; // 9 words are COPPER haeder and trailer size.

  //
  // If there are data remaining to be read, continue reading
  //
  if ((int)((*m_size_word - m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS) * sizeof(int)) > recvd_byte) {

    // If event size is larger than BUF_SIZE_WORD, allocate a new buffer
    if (*m_size_word >  BUF_SIZE_WORD) {
      *delete_flag = 1;
      temp_buf = new int[ *m_size_word ];
      memcpy(temp_buf, m_bufary[ entry ], recvd_byte);
      recvd_byte += readFD(m_cpr_fd, (char*)temp_buf + recvd_byte,
                           (*m_size_word - m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS) * sizeof(int) - recvd_byte, *delete_flag);
    } else {

      recvd_byte += readFD(m_cpr_fd, (char*)(m_bufary[ entry ]) + recvd_byte,
                           (*m_size_word - m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS) * sizeof(int) - recvd_byte, *delete_flag);
    }

    if ((int)((*m_size_word - m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS) * sizeof(int)) != recvd_byte) {
      char    err_buf[500];

      sprintf(err_buf, "[FATAL] CORRUPTED DATA: Read less bytes(%d) than expected(%d:%d). Exiting...\n",
              recvd_byte,
              *m_size_word * sizeof(int) - m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS * sizeof(int),
              m_bufary[ entry ][ m_pre_rawcpr.POS_DATA_LENGTH ]);
      print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      exit(-1);
    }
  } else if ((int)((*m_size_word - m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS) * sizeof(int)) < recvd_byte) {
    char    err_buf[500];
    sprintf(err_buf, "[FATAL] CORRUPTED DATA: Read more than data size. Exiting...: %d %d %d %d %d\n",
            recvd_byte, *m_size_word * sizeof(int) , m_pre_rawcpr.tmp_trailer.RAWTRAILER_NWORDS * sizeof(int),
            m_bufary[ entry ][ m_pre_rawcpr.POS_DATA_LENGTH ],  m_pre_rawcpr.POS_DATA_LENGTH);
    print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }
#else
  //
  // Make dummy data
  //
  *m_size_word = 256 + entry;
  m_bufary[entry][0] = *m_size_word;
#endif

  //
  // Fill Data length
  //
  temp_buf[ 0 ] = *m_size_word;


#ifdef TIME_MONITOR
  if (n_basf2evt >= 50000 && n_basf2evt < 50500) {
    cur_time = getTimeSec();
    time_array2[ n_basf2evt - 50000 ] = cur_time - m_start_time;
  }
#endif

#ifdef CHECK_SUM
  unsigned int checksum = 0;
  for (int i = 0; i < m_bufary[entry][0]; i++) {
    if (i != 2) checksum += m_bufary[entry][i];
  }
  m_bufary[entry][2] = checksum;
#endif
  return temp_buf;
}



void DesSerCOPPER::initializeCOPPER()
{

#ifndef DUMMY
  m_use_slot = 0; /* bit mask */
  int slot_shift;

  if ((m_finesse_bit_flag & 0x1) == 1) {
    slot_shift = 0; // a:0, b:1, c:2, d:3
    m_use_slot |= 1 << slot_shift;  //
  }

  if (((m_finesse_bit_flag >> 1) & 0x1) == 1) {
    slot_shift = 1; // a:0, b:1, c:2, d:3
    m_use_slot |= 1 << slot_shift;  //
  }

  if (((m_finesse_bit_flag >> 2) & 0x1) == 1) {
    slot_shift = 2; // a:0, b:1, c:2, d:3
    m_use_slot |= 1 << slot_shift;  //
  }

  if (((m_finesse_bit_flag >> 3) & 0x1) == 1) {
    slot_shift = 3; // a:0, b:1, c:2, d:3
    m_use_slot |= 1 << slot_shift;  //
  }
  //
  // Present slots to use
  //
  if (! m_use_slot) {
    char err_buf[100] = "[FATAL] Slot is not specified. Exiting...";
    print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(1);
  } else {
    int slot;
    printf("[DEBUG] ");
    for (slot = 0; slot < 4; slot++) {
      if (m_use_slot & (1 << slot)) printf(" %c", 'A' + slot);
    }
    printf("\n");
  }


#endif

#ifndef YAMADA_DUMMY
  //  B2INFO("Opening COPPER...");  fflush(stderr);
  printf("[DEBUG] Opening COPPER...\n"); fflush(stdout);
  openCOPPER();
  //  B2INFO("Done.\n"); fflush(stderr);
  printf("[DEBUG] Done.\n"); fflush(stdout);
#endif
}


void DesSerCOPPER::openCOPPER()
{

  if (m_cpr_fd != -1) {
    close(m_cpr_fd);
    m_cpr_fd = -1;
  }
  //
  // Open a finesse device
  //
  if ((m_cpr_fd = open("/dev/copper/copper", O_RDONLY)) == -1) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] Failed to open /dev/copper/copper. Exiting... ");
    print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(1);
  }

  int set_regval = 15; // How many events to be stored in COPPER FIFO before request for DMA
  //    int set_regval=1;
  ioctl(m_cpr_fd, CPRIOSET_LEF_WA_FF, &set_regval);
  ioctl(m_cpr_fd, CPRIOSET_LEF_WB_FF, &set_regval);
  ioctl(m_cpr_fd, CPRIOSET_LEF_WC_FF, &set_regval);
  ioctl(m_cpr_fd, CPRIOSET_LEF_WD_FF, &set_regval);
  ioctl(m_cpr_fd, CPRIOSET_FINESSE_STA, &m_use_slot, sizeof(m_use_slot));

  int v = 511 - 32;

  ioctl(m_cpr_fd, CPRIOSET_LEF_WA_AF, &v, sizeof(v));
  ioctl(m_cpr_fd, CPRIOSET_LEF_WB_AF, &v, sizeof(v));
  ioctl(m_cpr_fd, CPRIOSET_LEF_WC_AF, &v, sizeof(v));
  ioctl(m_cpr_fd, CPRIOSET_LEF_WD_AF, &v, sizeof(v));


  //  B2INFO("DeSerializerCOPPER: openCOPPER() done.");
  printf("[DEBUG] DeSerializerCOPPER: openCOPPER() done.\n"); fflush(stdout);

}



int DesSerCOPPER::readFD(int fd, char* buf, int data_size_byte, int delete_flag)
{

  int n = 0;
  int read_size = 0;
  while (1) {
    if ((read_size = read(fd, (char*)buf + n, data_size_byte - n)) < 0) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        if (n > 0) {
          char err_buf[500];
          sprintf(err_buf, "[FATAL] Return due to EAGAIN in the middle of an event( COPPER driver would't do this.). Exting...");
          print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
          exit(-1);
        }
#ifdef NONSTOP
        // Check run-pause request from SLC
        string err_str;
        try {
          callCheckRunPause(err_str);
        } catch (string err_str) {
          if (delete_flag) {
            // Delete allocated buffer
            //            B2WARNING("Delete buffer before going to Run-pause state");
            printf("[WARNING] Delete buffer before going to Run-pause state\n"); fflush(stdout);
            delete buf;
          }
          throw (err_str);
        }
#endif
        continue;
      } else {
#ifdef NONSTOP
        m_run_error = 1;
        //        B2ERROR(err_buf);
        printf("[ERROR] Failed to read data from COPPER. %s %s %d",
               __FILE__, __PRETTY_FUNCTION__, __LINE__);
        string err_str = "RUN_ERROR";
        throw (err_str);
#endif
        char err_buf[500];
        sprintf(err_buf, "[FATAL] Failed to read data from COPPER. %s %s %d",
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        print_err.PrintError(m_shmflag, &m_status, err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        exit(-1);
      }
    } else {
      n += read_size;
      if (n == data_size_byte)break;
    }
  }
  return n;
}
