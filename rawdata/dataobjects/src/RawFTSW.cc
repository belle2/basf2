//+
// File : RawFTSW.cc
// Description : Module to handle raw data from FTSW
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawFTSW.h>
using namespace std;
using namespace Belle2;

RawFTSW::RawFTSW()
{
  m_access = NULL;
  m_version = -1;
}

RawFTSW::~RawFTSW()
{
  if (m_access != NULL) delete m_access;
  m_access = NULL;
}

void RawFTSW::SetVersion()
{
  if (m_buffer == NULL) {
    perror("m_buffer is NULL. Exiting...");
    exit(1);
  }

  if (m_access != NULL) {
    delete m_access;
  }


  //
  // Special treatment is required because Nakao-san did not set verision number in ver.0,1, and 2 header.
  //
  if (m_buffer[ POS_HEADER_SIZE ] == VER_2_HEADER_SIZE) {
    m_access = new RawFTSWFormat_latest;
    m_version = 1;
  } else if (m_buffer[ POS_HEADER_SIZE ] == VER_1_HEADER_SIZE) {
    m_access = new RawFTSWFormat_v1;
    m_version = 0;
  } else if (m_buffer[ POS_HEADER_SIZE ] == VER_0_HEADER_SIZE) {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] Ver.0 of RawFTSW( so-called early DESYtest format is detected but not supported. (header size = 0x%.8x ) Exiting...\n %s %s %d\n",
            m_buffer[ POS_HEADER_SIZE ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    string err_str = err_buf; throw (err_str);
  } else {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] ERROR_EVENT : Invalid header size of FTSW data format(= 0x%.8x words). Exiting...\n %s %s %d\n",
            m_buffer[ POS_HEADER_SIZE ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    string err_str = err_buf; throw (err_str);
  }

  //
  // Assign unpacker
  //
  switch (m_version) {
    case LATEST_FTSW_FORMAT_VER :
      m_access = new RawFTSWFormat_latest;
      break;
    case 1 : // it is 0x20 as of 2019/02/18
      m_access = new RawFTSWFormat_v1;
      break;
    default : {
      char err_buf[500];
      sprintf(err_buf, "[FATAL] ERROR_EVENT : Invalid version of FTSW data format (data says it is 0x%.8x now). Exiting...\n %s %s %d\n",
              m_version, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("%s", err_buf); fflush(stdout);
      string err_str = err_buf; throw (err_str);
    }
    exit(1);
  }
  m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
}
