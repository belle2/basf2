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
  m_nwords = 0;
  m_num_nodes = 0;
  m_num_events = 0;
  m_buffer = NULL;

}


//
// Functions for RawDataBlock
//


void RawCOPPERFormat::PrintData(int* buf, int nwords)
{
  printf("[DEBUG] ");
  for (int i = 0; i < nwords; i++) {
    printf("%.8x ", buf[ i ]);
    if (i % 10 == 9) printf("\n[DEBUG] ");
  }
  printf("\n[DEBUG] ");
  printf("\n");
  return;
}


int RawCOPPERFormat::TotalBufNwords()
{
  return m_nwords;
}


int RawCOPPERFormat::GetBlockNwords(int n)
{
  int size;
  if (n == (m_num_events * m_num_nodes) - 1) {
    size =  m_nwords - GetBufferPos(n);
  } else {
    size = GetBufferPos(n + 1) - GetBufferPos(n);
  }
  return size;
}


int* RawCOPPERFormat::GetWholeBuffer()
{
  return m_buffer;
}

int* RawCOPPERFormat::GetBuffer(int n)
{
  int pos_nwords = GetBufferPos(n);
  return &(m_buffer[ pos_nwords ]);
}




void RawCOPPERFormat::SetBuffer(int* bufin, int nwords, int delete_flag, int num_events, int num_nodes)
{
  if (delete_flag  == 1) {
    printf("RawCOPPER format class does not delete m_buffer. Please specify 0 for delete_flag. Exiting...\n");
    exit(1);
  }

  if (bufin == NULL) {
    printf("[DEBUG] bufin is NULL. Exting...\n");
    exit(1);
  }

//   if (!m_use_prealloc_buf && m_buffer != NULL) delete[] m_buffer;
//   if (delete_flag == 0) {
//     m_use_prealloc_buf = true;
//   } else {
//     m_use_prealloc_buf = false;
//   }

  m_nwords = nwords;
  m_buffer = bufin;

  m_num_nodes = num_nodes;
  m_num_events = num_events;

  // Set length at the first word of the buffer

  //
  // Assign header and trailer
  //

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
  sprintf(err_buf, "Specifined FINESSE number( = %d ) is invalid. Exiting...\n%s %s %d\n", finesse_num,
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
  sprintf(err_buf, "Specifined FINESSE number( = %d ) is invalid. Exiting...\n%s %s %d\n", finesse_num,
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
  sprintf(err_buf, "Specifined FINESSE number( = %d ) is invalid. Exiting...\n%s %s %d\n", finesse_num,
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
    sprintf(err_buf, "No HSLB data in COPPER data. Exiting...\n %s %s %d\n",
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf; throw (err_str);
//     sleep(12345678);
//     exit(-1);
  }


  if (err_flag == 1) {

    char err_buf[500];
    sprintf(err_buf, "CORRUPTED DATA: Different event number over HSLBs : slot A 0x%x : B 0x%x :C 0x%x : D 0x%x\n %s %s %d\n",
            word[ 0 ], word[ 1 ], word[ 2 ], word[ 3 ],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] [ERROR] %s\n", err_buf);
#ifndef NO_DATA_CHECK
    string err_str = err_buf; throw (err_str);

//     sleep(12345678);
//     exit(-1);
#endif
  }

  return ret_word;

}



