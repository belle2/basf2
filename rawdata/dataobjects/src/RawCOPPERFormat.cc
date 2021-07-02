//+
// File : RawCOPPERFormat.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 3 - July - 2014
//-

#include <rawdata/dataobjects/RawCOPPERFormat.h>


using namespace Belle2;
//ClassImp(RawCOPPERFormat);


RawCOPPERFormat::RawCOPPERFormat()
{
//   m_nwords = 0;
//   m_num_nodes = 0;
//   m_num_events = 0;
//   m_buffer = NULL;
}

unsigned int  RawCOPPERFormat::CalcXORChecksum(int* buf, int nwords)
{
  unsigned int checksum = 0;
  for (int i = 0; i < nwords; i++) {
    checksum = checksum ^ buf[ i ];
  }
  return checksum;
}




int RawCOPPERFormat::GetOffsetFINESSE(int n, int finesse_num)
{
  switch (finesse_num) {
    case 0 :
      return GetOffset1stFINESSE(n);
      break;
    case 1 :
      return GetOffset2ndFINESSE(n);
      break;
    case 2 :
      return GetOffset3rdFINESSE(n);
      break;
    case 3 :
      return GetOffset4thFINESSE(n);
      break;
    default :
      break;
  }

  char err_buf[500];
  sprintf(err_buf, "[FATAL] Specifined FINESSE number( = %d ) is invalid. Exiting...\n%s %s %d\n", finesse_num,
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s", err_buf); fflush(stdout);
  B2FATAL(err_buf);
}




int* RawCOPPERFormat::GetFINESSEBuffer(int n, int finesse_num)
{
  switch (finesse_num) {
    case 0 :
      return Get1stFINESSEBuffer(n);
      break;
    case 1 :
      return Get2ndFINESSEBuffer(n);
      break;
    case 2 :
      return Get3rdFINESSEBuffer(n);
      break;
    case 3 :
      return Get4thFINESSEBuffer(n);
      break;
    default :
      break;
  }

  char err_buf[500];
  sprintf(err_buf, "[FATAL] Specifined FINESSE number( = %d ) is invalid. Exiting...\n%s %s %d\n", finesse_num,
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s", err_buf); fflush(stdout);
  B2FATAL(err_buf);
}




int* RawCOPPERFormat::GetDetectorBuffer(int n, int finesse_num)
{
  switch (finesse_num) {
    case 0 :
      return Get1stDetectorBuffer(n);
      break;
    case 1 :
      return Get2ndDetectorBuffer(n);
      break;
    case 2 :
      return Get3rdDetectorBuffer(n);
      break;
    case 3 :
      return Get4thDetectorBuffer(n);
      break;
    default :
      break;
  }

  char err_buf[500];
  sprintf(err_buf, "[FATAL] Specifined FINESSE number( = %d ) is invalid. Exiting...\n%s %s %d\n", finesse_num,
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s", err_buf); fflush(stdout);
  B2FATAL(err_buf);
}


// This function is available after RawCOPPER/Header ver.2
int RawCOPPERFormat::GetPacketCRCError(int n)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of  RawCOPPER format that you're using. n=%d : %s %s %d: Exiting...\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return -1;
}

// This function is available after RawCOPPER/Header ver.2
int RawCOPPERFormat::GetEventCRCError(int n)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of  RawCOPPER format that you're using. n=%d : %s %s %d: Exiting...\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return -1;
}

int RawCOPPERFormat::GetEventCRC16Value(int n, int finesse_num)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of  RawCOPPER format that you're using. n=%d fin=%d : %s %s %d: Exiting...\n",
          n, finesse_num, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return -1;
}

int* RawCOPPERFormat::PackDetectorBuf(int* /*packed_buf_nwords*/,
                                      int* const(& /*detector_buf_ch*/)[MAX_PCIE40_CH],
                                      int const(& /*nwords_ch*/)[MAX_PCIE40_CH],
                                      RawCOPPERPackerInfo /*rawcpr_info*/)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of  RawCOPPER format that you're using. : %s %s %d: Exiting...\n", __FILE__,
          __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return NULL;
}

void RawCOPPERFormat::CompareHeaderValue(int /*n*/, const unsigned int (& /*input_val*/)[MAX_PCIE40_CH],
                                         std::vector<std::vector<unsigned int> >& /*result*/)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of  RawCOPPER format that you're using. : %s %s %d: Exiting...\n", __FILE__,
          __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return;
}

void RawCOPPERFormat::GetNodeName(int n, char* node_name, int bufsize)
{
  unsigned int node_id = GetNodeID(n);
  if (node_name == NULL || node_id == 0 || bufsize < 20) {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] Null pointer or strange node_id(%.8x)in an argument of this function. Exiting... : \n%s %s %d\n",
            node_id,
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf); // to reduce multiple error messages
  } else {
    sprintf(node_name, "cpr%u",
            (10 * (node_id >> 28) + (node_id >> 24)) * 1000 +
            (node_id & COPPERID_MASK));
  }
}

void RawCOPPERFormat::GetNodeName(char* node_name, unsigned int node_id, int bufsize)
{
  if (node_name == NULL || node_id == 0 || bufsize < 20) {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] Null pointer or strange node_id(%.8x)in an argument of this function. Exiting... : \n%s %s %d\n",
            node_id,
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf); // to reduce multiple error messages
  } else {
    sprintf(node_name, "cpr%u",
            (10 * (node_id >> 28) + (node_id >> 24)) * 1000 +
            (node_id & COPPERID_MASK));
  }
}
