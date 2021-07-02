/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RawHeader_v2.cc
// Description : Module to handle RawHeader_v2 attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawHeader_v2.h>


using namespace Belle2;

//ClassImp(RawHeader_v2);

RawHeader_v2::RawHeader_v2()
{
  m_buffer = NULL;
  //  initialize();
  //  cout << "RawHeader_v2 NULL constructor" << endl;
}

RawHeader_v2::RawHeader_v2(int* buffer)
{
  m_buffer = buffer;
}

RawHeader_v2::~RawHeader_v2()
{
}


// int RawHeader_v2::AddNodeInfo(int node_id)
// {
//   CheckSetBuffer();
//   // When the number of total nodes exceeds NUM_MAX_NODES
//   if (GetNumNodes() >= NUM_MAX_NODES) {
//     m_buffer[ POS_NUM_NODES ]++;
//     return -1;
//   }

//   m_buffer[ POS_NODES_1 + m_buffer[ POS_NUM_NODES ] ] = node_id;
//   m_buffer[ POS_NUM_NODES ]++;
//   return 0;
// }

// int RawHeader_v2::GetNodeInfo(int node_no, int* node_id)
// {
//   CheckGetBuffer();
//   if (node_no >= GetNumNodes()) {
//     return -1;
//   }
//   *node_id = m_buffer[ POS_NODES_1 + node_no ];

//   return 0;
// }




void RawHeader_v2::CheckHeader(int* buf)
{

  // # of words
  if (buf[ POS_NWORDS ] > 2.5e7 || buf[ POS_NWORDS ] < RAWHEADER_NWORDS) {
    char err_buf[500];
    sprintf(err_buf, "data size error %d words. Exiting... : %s %s %d\n",
            buf[ POS_NWORDS ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  // # of words in this block
  if ((buf[ POS_VERSION_HDRNWORDS ] & HDR_NWORDS_MASK) != RAWHEADER_NWORDS) {
    char err_buf[500];
    sprintf(err_buf, "Invalid header size %d words should be %d. Exiting... : %s %s %d\n",
            buf[ buf[ POS_VERSION_HDRNWORDS ] & HDR_NWORDS_MASK ],
            RAWHEADER_NWORDS, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  //  exp. run #

  //  utime( 2010 1/1 - 2050 1/1 )
  if ((unsigned int)(buf[ POS_TTUTIME ]) < (unsigned int)0x4b3cbc70 ||
      (unsigned int)0x9679f770 < (unsigned int)(buf[ POS_TTUTIME ])) {
    char err_buf[500];
    sprintf(err_buf, "Invalid utime %d . Exiting...: %s %s %d\n",
            buf[ POS_TTUTIME ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  // truncation mask

  // offset
  if (buf[ POS_OFFSET_1ST_FINESSE ] < RAWHEADER_NWORDS) {
    char err_buf[500];
    sprintf(err_buf, "Invalid data offset for 1st finesse buffer(%d). Exiting...: %s %s %d\n",
            buf[ POS_OFFSET_1ST_FINESSE ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  if (buf[ POS_OFFSET_2ND_FINESSE ] < buf[ POS_OFFSET_1ST_FINESSE ]) {
    char err_buf[500];
    sprintf(err_buf, "Invalid data offset for 2nd finesse buffer(%d). Exiting...: %s %s %d\n",
            buf[ POS_OFFSET_2ND_FINESSE ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  if (buf[ POS_OFFSET_3RD_FINESSE ] < buf[ POS_OFFSET_2ND_FINESSE ]) {
    char err_buf[500];
    sprintf(err_buf, "Invalid data offset for 3rd finesse buffer(%d). Exiting...: %s %s %d\n",
            buf[ POS_OFFSET_3RD_FINESSE ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  if (buf[ POS_OFFSET_4TH_FINESSE ] < buf[ POS_OFFSET_3RD_FINESSE ] ||
      buf[ POS_OFFSET_4TH_FINESSE ] > buf[ POS_NWORDS ]) {
    char err_buf[500];
    sprintf(err_buf, "Invalid data offset for 4th finesse buffer(%d). Exiting...: %s %s %d\n",
            buf[ POS_OFFSET_4TH_FINESSE ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  return;


}
