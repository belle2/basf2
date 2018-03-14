//+
// File : RawCOPPERFormat.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 3 - July - 2014
//-

#include <rawdata/dataobjects/RawCOPPERFormat.h>

using namespace std;
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
  printf("%s", err_buf);
  string err_str = err_buf; throw (err_str);
  //   sleep(12345678);
  //   exit(-1);
  //   return 0;

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
  printf("%s", err_buf);
  string err_str = err_buf; throw (err_str);
//   sleep(12345678);
//   exit(-1);
//   return 0;
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
  printf("%s", err_buf);
  string err_str = err_buf; throw (err_str);
//   sleep(12345678);
//   exit(-1);
//   return 0;
}



unsigned int RawCOPPERFormat::GetB2LHeaderWord(int n, int finesse_buffer_pos)
{
  unsigned int word[4];
  unsigned int ret_word = 0;
  int flag = 0, err_flag = 0;

  for (int i = 0; i < 4; i++) {
    if (GetFINESSENwords(n, i) > 0) {
      word[ i ] = m_buffer[ GetOffsetFINESSE(n, i) + finesse_buffer_pos ];
      if (flag != 0 && (ret_word != word[ i ])) {
        err_flag = 1;
      }
      ret_word = word[ i ];
      flag = 1;
    }
  }

  if (flag == 0) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] ERROR_EVENT : No HSLB data in COPPER data. Exiting...\n %s %s %d\n",
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf; throw (err_str);
  }

  if (err_flag == 1) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] ERROR_EVENT : Different event number over HSLBs : slot A 0x%x : B 0x%x :C 0x%x : D 0x%x\n %s %s %d\n",
            word[ 0 ], word[ 1 ], word[ 2 ], word[ 3 ],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] %s\n", err_buf);
#ifndef NO_DATA_CHECK
    string err_str = err_buf; throw (err_str);
#endif
  }
  return ret_word;
}


// This function is available after RawCOPPER/Header ver.2
int RawCOPPERFormat::GetPacketCRCError(int n)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of  RawCOPPER format that you're using. n=%d : %s %s %d: Exiting...\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  string err_str = err_buf;
  throw (err_str);
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
  string err_str = err_buf;
  throw (err_str);
  return -1;
}

int RawCOPPERFormat::GetEventCRC16Value(int n, int finesse_num)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] This function is not supported in the version of  RawCOPPER format that you're using. n=%d fin=%d : %s %s %d: Exiting...\n",
          n, finesse_num, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s\n", err_buf); fflush(stdout);
  string err_str = err_buf;
  throw (err_str);
  return -1;
}

