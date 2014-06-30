//+
// File : RawHeader_latest.cc
// Description : Module to handle RawHeader_latest attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawHeader_latest.h>


using namespace std;
using namespace Belle2;

ClassImp(RawHeader_latest);

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
  if (buf[ POS_NWORDS ] > 2.5e7 ||
      buf[ POS_NWORDS ] < RAWHEADER_NWORDS) {
    printf("data size error %d words. Exiting... : %s %s %d\n", buf[ POS_NWORDS ], __FILE__, __PRETTY_FUNCTION__, __LINE__); fflush(stdout);
    exit(1);
  }

  // # of words in this block
  if ((buf[ POS_VERSION_HDRNWORDS ] & HDR_NWORDS_MASK) != RAWHEADER_NWORDS) {
    printf("Invalid header size %d words should be %d. Exiting... : %s %s %d\n", buf[ buf[ POS_VERSION_HDRNWORDS ] & HDR_NWORDS_MASK ],
           RAWHEADER_NWORDS, __FILE__, __PRETTY_FUNCTION__, __LINE__); fflush(stdout);
    exit(1);
  }

  //  exp. run #

  //  utime( 2010 1/1 - 2050 1/1 )
  if ((unsigned int)(buf[ POS_TTUTIME ]) < (unsigned int)0x4b3cbc70 ||
      (unsigned int)0x9679f770 < (unsigned int)(buf[ POS_TTUTIME ])) {
    printf("Invalid utime %d . Exiting...: %s %s %d\n", buf[ POS_TTUTIME ]
           , __FILE__, __PRETTY_FUNCTION__, __LINE__); fflush(stdout);
    exit(1);
  }

  // subsystem ID

  // truncation mask

  // offset
  if (buf[ POS_OFFSET_1ST_FINESSE ] < RAWHEADER_NWORDS) {
    printf("Invalid data offset for 1st finesse buffer(%d). Exiting...: %s %s %d\n",
           buf[ POS_OFFSET_1ST_FINESSE ], __FILE__, __PRETTY_FUNCTION__, __LINE__); fflush(stdout);
    exit(1);
  }

  if (buf[ POS_OFFSET_2ND_FINESSE ] < buf[ POS_OFFSET_1ST_FINESSE ]) {
    printf("Invalid data offset for 2nd finesse buffer(%d). Exiting...: %s %s %d\n",
           buf[ POS_OFFSET_2ND_FINESSE ], __FILE__, __PRETTY_FUNCTION__, __LINE__); fflush(stdout);
    exit(1);
  }

  if (buf[ POS_OFFSET_3RD_FINESSE ] < buf[ POS_OFFSET_2ND_FINESSE ]) {
    printf("Invalid data offset for 3rd finesse buffer(%d). Exiting...: %s %s %d\n",
           buf[ POS_OFFSET_3RD_FINESSE ], __FILE__, __PRETTY_FUNCTION__, __LINE__); fflush(stdout);
    exit(1);
  }

  if (buf[ POS_OFFSET_4TH_FINESSE ] < buf[ POS_OFFSET_3RD_FINESSE ] ||
      buf[ POS_OFFSET_4TH_FINESSE ] > buf[ POS_NWORDS ]) {
    printf("Invalid data offset for 4th finesse buffer(%d). Exiting...: %s %s %d\n",
           buf[ POS_OFFSET_4TH_FINESSE ], __FILE__, __PRETTY_FUNCTION__, __LINE__); fflush(stdout);
    exit(1);
  }

  return;


}
