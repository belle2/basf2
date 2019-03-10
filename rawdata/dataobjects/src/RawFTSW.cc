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
    m_access = nullptr;
  }

  //
  // Assign unpacker
  //
  // Special treatment is required because Nakao-san did not set verision number in ver.0,1, and 2 header.
  // I'm using the size of header which is stored the 1st word (0,1,..)
  //
  int temp_version = m_version;
  if (m_buffer[ POS_HEADER_SIZE ] == VER_2_HEADER_SIZE) {
    m_access = new RawFTSWFormat_latest;
    m_version = 2; // as of 2019.3.2, the latest version is 2.
  } else if (m_buffer[ POS_HEADER_SIZE ] == VER_1_HEADER_SIZE) {
    m_access = new RawFTSWFormat_v1;
    m_version = 1;
  } else if (m_buffer[ POS_HEADER_SIZE ] == VER_0_HEADER_SIZE) {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] Ver.0 of RawFTSW( so-called early DESYtest format ) is detected but not supported. (header size = 0x%.8x ) Exiting...\n %s %s %d\n",
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

  if (temp_version >= 0 && temp_version != m_version) {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] Already assigned RawFTSW format version (= %.8x) is different from the one (= %.8x) from the current event. Exiting...\n %s %s %d\n",
            temp_version, m_version, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    string err_str = err_buf; throw (err_str);
  }

  m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
}


// I copied this from RawCOPPER.cc, since SetVersion() is necessary for RawFTSW after ver.2 is included.
void RawFTSW::SetBuffer(int* bufin, int nwords, int delete_flag, int num_events, int num_nodes)
{

  if (bufin == NULL) {
    printf("[FATAL] bufin is NULL. Exting...\n");
    exit(1);
  }
  if (!m_use_prealloc_buf && m_buffer != NULL) delete[] m_buffer;

  if (delete_flag == 0) {
    m_use_prealloc_buf = true;
  } else {
    m_use_prealloc_buf = false;
  }

  m_nwords = nwords;
  m_buffer = bufin;

  m_num_nodes = num_nodes;
  m_num_events = num_events;

  SetVersion();

}
