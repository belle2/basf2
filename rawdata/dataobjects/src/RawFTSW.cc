/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
    char err_buf[500];
    sprintf(err_buf, "m_buffer is NULL. Exiting...");
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
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
  if (m_buffer[ POS_HEADER_SIZE ] == VER_0_HEADER_SIZE) {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] Ver.0 of RawFTSW( so-called early DESYtest format ) is detected but not supported. (header size = 0x%.8x ) Exiting...\n %s %s %d\n",
            m_buffer[ POS_HEADER_SIZE ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  } else if (m_buffer[ POS_NODE_FORMAT_ID ] == FORMAT_ID_VER_0TO2) {
    if (m_buffer[ POS_HEADER_SIZE ] == VER_2_HEADER_SIZE) {
      m_access = new RawFTSWFormat_v2;
      m_version = 2; // as of 2019.3.2, the latest version is 2.
    } else if (m_buffer[ POS_HEADER_SIZE ] == VER_1_HEADER_SIZE) {
      m_access = new RawFTSWFormat_v1;
      m_version = 1;
    } else {
      char err_buf[500];
      sprintf(err_buf, "[FATAL] ERROR_EVENT : Invalid RawFTSW header size of FTSW data format(= 0x%.8x words). Exiting...\n %s %s %d\n",
              m_buffer[ POS_HEADER_SIZE ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("%s", err_buf); fflush(stdout);
      B2FATAL(err_buf);
    }
  } else if (m_buffer[ POS_NODE_FORMAT_ID ] == FORMAT_ID_VER_0TO3 ||
             m_buffer[ POS_NODE_FORMAT_ID ] == 0x54544432 ||
             m_buffer[ POS_NODE_FORMAT_ID ] == 0x54544433 ||
             m_buffer[ POS_NODE_FORMAT_ID ] == 0x54544434 ||
             m_buffer[ POS_NODE_FORMAT_ID ] == 0x54544435 ||
             m_buffer[ POS_NODE_FORMAT_ID ] == 0x54544436 ||
             m_buffer[ POS_NODE_FORMAT_ID ] == 0x54544437 ||
             m_buffer[ POS_NODE_FORMAT_ID ] == 0x54544438 ||
             m_buffer[ POS_NODE_FORMAT_ID ] == 0x54544439) {
    // Request from Nakao-san in Oct. 15, 2019
    // - FORMAT_ID_VER_0TO3 should be changed to x"54544431" from x"5454421"
    // - RawFTSWFormat_latest should accept newer format versions(= larger version number), so that this unpacker
    //    will not stop due to version error, when Nakao-san updated version number in RawFTSW data.
    // -- Since the latest format is basically the addition of some variables in RawFTSW data, probably it won't cause the problem.
    m_access = new RawFTSWFormat_latest;
    m_version = 3; // as of 2019.3.20 the latest version is 3.
  } else {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] ERROR_EVENT : Invalid RawFTSW header size(= 0x%.8x words) or version number.(=0x%.8x) Exiting...\n %s %s %d\n",
            m_buffer[ POS_HEADER_SIZE ], m_buffer[ POS_NODE_FORMAT_ID ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }


  if (temp_version >= 0 && temp_version != m_version) {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] Already assigned RawFTSW format version (= %.8x) is different from the one (= 0x%.8x) from the current event. Exiting...\n %s %s %d\n",
            temp_version, m_version, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
}


// I copied this from RawCOPPER.cc, since SetVersion() is necessary for RawFTSW after ver.2 is included.
void RawFTSW::SetBuffer(int* bufin, int nwords, int delete_flag, int num_events, int num_nodes)
{

  if (bufin == NULL) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] bufin is NULL. Exting...\n");
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
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
