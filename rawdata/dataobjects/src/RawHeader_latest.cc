/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RawHeader_latest.cc
// Description : Module to handle RawHeader_latest attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawHeader_latest.h>


using namespace Belle2;

//ClassImp(RawHeader_latest);

RawHeader_latest::RawHeader_latest()
{
  m_buffer = NULL;
  //  initialize();
  //  cout << "RawHeader_latest NULL constructor" << endl;
}

RawHeader_latest::RawHeader_latest(int* buffer)
{
  m_buffer = buffer;
}

RawHeader_latest::~RawHeader_latest()
{
}


// int RawHeader_latest::AddNodeInfo(int node_id)
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

// int RawHeader_latest::GetNodeInfo(int node_no, int* node_id)
// {
//   CheckGetBuffer();
//   if (node_no >= GetNumNodes()) {
//     return -1;
//   }
//   *node_id = m_buffer[ POS_NODES_1 + node_no ];

//   return 0;
// }

void RawHeader_latest::CheckHeader(int* buf)
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

  int ch = 0;
  // offset
  if (buf[ POS_CH_POS_TABLE + ch ] < RAWHEADER_NWORDS) {
    char err_buf[500];
    sprintf(err_buf, "Invalid data offset for ch(=%d) finesse buffer(%d). Exiting...: %s %s %d\n",
            ch, buf[ POS_CH_POS_TABLE + ch ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  for (int i = 1; i < (MAX_PCIE40_CH - 1); i++) {
    ch = i;
    if (buf[ POS_CH_POS_TABLE + ch ] < buf[ POS_CH_POS_TABLE + (ch - 1) ]) {
      char err_buf[500];
      sprintf(err_buf, "Invalid data offset for ch(=%d) finesse buffer(%d). Exiting...: %s %s %d\n",
              ch, buf[ POS_CH_POS_TABLE + ch ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("%s", err_buf); fflush(stdout);
      B2FATAL(err_buf);
    }
  }
  ch = MAX_PCIE40_CH - 1;

  if (buf[ POS_CH_POS_TABLE + ch ] < buf[ POS_CH_POS_TABLE + (ch - 1) ] ||
      buf[ POS_CH_POS_TABLE + ch ] > buf[ POS_NWORDS ]) {
    char err_buf[500];
    sprintf(err_buf, "Invalid data offset for ch(=%d) finesse buffer(%d). Exiting...: %s %s %d\n",
            ch, buf[ POS_CH_POS_TABLE + ch ], __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }
  return;
}

void RawHeader_latest::SetOffset1stFINESSE(int /*offset_1st_FINESSE*/)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
}

void RawHeader_latest::SetOffset2ndFINESSE(int /*offset_2nd_FINESSE*/)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
}

void RawHeader_latest::SetOffset3rdFINESSE(int /*offset_3rd_FINESSE*/)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
}

void RawHeader_latest::SetOffset4thFINESSE(int /*offset_4th_FINESSE*/)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
}

int RawHeader_latest::GetOffset1stFINESSE()
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}

int RawHeader_latest::GetOffset2ndFINESSE()
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}

int RawHeader_latest::GetOffset3rdFINESSE()
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}

int RawHeader_latest::GetOffset4thFINESSE()
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}
