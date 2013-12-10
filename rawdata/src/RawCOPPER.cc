//+
// File : RawCOPPER.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/RawCOPPER.h>

using namespace std;
using namespace Belle2;

ClassImp(RawCOPPER);

RawCOPPER::RawCOPPER()
{
}

RawCOPPER::~RawCOPPER()
{
}


int* RawCOPPER::GetRawHdrBufPtr(int n)
{
  int pos_nwords = GetBufferPos(n);
  return &(m_buffer[ pos_nwords ]);
}

int* RawCOPPER::GetRawTrlBufPtr(int n)
{
  int pos_nwords;
  RawTrailer trl;

  if (n == (m_num_events * m_num_nodes) - 1) {
    pos_nwords = m_nwords - trl.GetTrlNwords();
  } else {
    pos_nwords = GetBufferPos(n + 1) - trl.GetTrlNwords();
  }
  return &(m_buffer[ pos_nwords ]);
}


int RawCOPPER::GetBufferPos(int n)
{
  if (m_buffer == NULL || m_nwords <= 0) {
    printf("[ERROR] RawPacket buffer(%p) is not available or length(%d) is not set.\n", m_buffer, m_nwords);
    exit(1);
  }

  if (n >= (m_num_events * m_num_nodes)) {
    printf("Invalid COPPER block No. (%d : max %d ) is specified. Exiting... ", n, (m_num_events * m_num_nodes));
    exit(1);
  }
  int pos_nwords = 0;
  for (int i = 1; i <= n ; i++) {

//     for( int j = 0; j < 127; j++){
//       printf("%.8x ", m_buffer[j]);
//       if(j % 10 == 9) printf("\n");
//     }
//     printf("\n\n");

    int size = tmp_header.RAWHEADER_NWORDS
               + m_buffer[ pos_nwords + tmp_header.RAWHEADER_NWORDS + POS_DATA_LENGTH ]
               + SIZE_COPPER_FRONT_HEADER
               + SIZE_COPPER_TRAILER - 1
               + tmp_trailer.RAWTRAILER_NWORDS;
    // COPPER's data length include one word from COPPER trailer. so -1 is needed.

    pos_nwords +=  size;
    if (pos_nwords >= m_nwords) {
      printf("value of pos_nwords(%d) is larger than m_nwords(%d). Exiting...", pos_nwords, m_nwords);
      exit(1);
    }
  }
  return pos_nwords;

}


int RawCOPPER::GetCOPPERNodeId(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.POS_SUBSYS_ID;
  return m_buffer[ pos_nwords ];
}


int RawCOPPER::GetExpNo(int n)
{
  RawHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetExpNo();
}

int RawCOPPER::GetRunNo(int n)
{
  RawHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetRunNo();
}

unsigned int RawCOPPER::GetEveNo(int n)
{
  RawHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetEveNo();
}


int RawCOPPER::GetSubsysId(int n)
{
  RawHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetSubsysId();
}


int RawCOPPER::GetDataType(int n)
{
  RawHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetDataType();
}

int RawCOPPER::GetTruncMask(int n)
{
  RawHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetTruncMask();
}


unsigned int RawCOPPER::GetCOPPERCounter(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + POS_EVE_NUM_COPPER + hdr.RAWHEADER_NWORDS;
  return (unsigned int)(m_buffer[ pos_nwords ]);
}


int RawCOPPER::GetNumFINESSEBlock(int n)
{

  RawHeader hdr;
  int cnt = 0;
  int pos_nwords = GetBufferPos(n) + POS_CH_A_DATA_LENGTH + hdr.RAWHEADER_NWORDS;
  if (m_buffer[ pos_nwords ] > 0) cnt++;
  pos_nwords = GetBufferPos(n) + POS_CH_B_DATA_LENGTH + hdr.RAWHEADER_NWORDS;
  if (m_buffer[ POS_CH_B_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  pos_nwords = GetBufferPos(n) + POS_CH_C_DATA_LENGTH + hdr.RAWHEADER_NWORDS;
  if (m_buffer[ POS_CH_C_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  pos_nwords = GetBufferPos(n) + POS_CH_D_DATA_LENGTH + hdr.RAWHEADER_NWORDS;
  if (m_buffer[ POS_CH_D_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  return cnt;
}

int RawCOPPER::GetDetectorNwords(int n, int finesse_num)
{
  switch (finesse_num) {
    case 0 :
      return Get1stDetectorNwords(n);
      break;
    case 1 :
      return Get2ndDetectorNwords(n);
      break;
    case 2 :
      return Get3rdDetectorNwords(n);
      break;
    case 3 :
      return Get4thDetectorNwords(n);
      break;
    default :
      break;
  }
  ErrorMessage print_err;
  char err_buf[500];
  sprintf(err_buf, "Specifined FINESSE number( = %d ) is invalid. Exiting...\n", finesse_num);
  print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  sleep(12345678);
  exit(-1);
  return 0;
}

int RawCOPPER::Get1stDetectorNwords(int n)
{
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  int nwords = 0;
  if (Get1stFINESSENwords(n) > 0) {
    nwords = Get1stFINESSENwords(n) -  SIZE_B2LHSLB_HEADER - SIZE_B2LFEE_HEADER
             - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER;
  }
  return nwords;
}

int RawCOPPER::Get2ndDetectorNwords(int n)
{
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  int nwords = 0;
  if (Get2ndFINESSENwords(n) > 0) {
    nwords = Get2ndFINESSENwords(n) -  SIZE_B2LHSLB_HEADER -
             SIZE_B2LFEE_HEADER - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER;
  }
  return nwords;
}

int RawCOPPER::Get3rdDetectorNwords(int n)
{
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  int nwords = 0;
  if (Get3rdFINESSENwords(n) > 0) {
    nwords = Get3rdFINESSENwords(n) -  SIZE_B2LHSLB_HEADER -
             SIZE_B2LFEE_HEADER - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER;
  }
  return nwords;
}

int RawCOPPER::Get4thDetectorNwords(int n)
{
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  int nwords = 0;
  if (Get4thFINESSENwords(n) > 0) {
    nwords = Get4thFINESSENwords(n) -  SIZE_B2LHSLB_HEADER -
             SIZE_B2LFEE_HEADER - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER;
  }
  return nwords;
}


int RawCOPPER::GetFINESSENwords(int n, int finesse_num)
{
  switch (finesse_num) {
    case 0 :
      return Get1stFINESSENwords(n);
      break;
    case 1 :
      return Get2ndFINESSENwords(n);
      break;
    case 2 :
      return Get3rdFINESSENwords(n);
      break;
    case 3 :
      return Get4thFINESSENwords(n);
      break;
    default :
      break;
  }
  ErrorMessage print_err;
  char err_buf[500];
  sprintf(err_buf, "Specifined FINESSE number( = %d ) is invalid. Exiting...\n", finesse_num);
  print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  sleep(12345678);
  exit(-1);
  return 0;

}


int RawCOPPER::Get1stFINESSENwords(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_A_DATA_LENGTH;
  return m_buffer[ pos_nwords ];
}

int RawCOPPER::Get2ndFINESSENwords(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_B_DATA_LENGTH;
  return m_buffer[ pos_nwords ];
}

int RawCOPPER::Get3rdFINESSENwords(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_C_DATA_LENGTH;
  return m_buffer[ pos_nwords ];
}

int RawCOPPER::Get4thFINESSENwords(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_D_DATA_LENGTH;
  return m_buffer[ pos_nwords ];
}



int RawCOPPER::GetOffsetFINESSE(int n, int finesse_num)
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
  ErrorMessage print_err;
  char err_buf[500];
  sprintf(err_buf, "Specifined FINESSE number( = %d ) is invalid. Exiting...\n", finesse_num);
  print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  sleep(12345678);
  exit(-1);
  return 0;

}



int RawCOPPER::GetOffset1stFINESSE(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + SIZE_COPPER_HEADER;
  return pos_nwords;
}

int RawCOPPER::GetOffset2ndFINESSE(int n)
{
  return GetOffset1stFINESSE(n) + Get1stFINESSENwords(n);
}

int RawCOPPER::GetOffset3rdFINESSE(int n)
{
  return GetOffset2ndFINESSE(n) + Get2ndFINESSENwords(n);
}

int RawCOPPER::GetOffset4thFINESSE(int n)
{
  return GetOffset3rdFINESSE(n) + Get3rdFINESSENwords(n);
}



int* RawCOPPER::GetFINESSEBuffer(int n, int finesse_num)
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

  ErrorMessage print_err;
  char err_buf[500];
  sprintf(err_buf, "Specifined FINESSE number( = %d ) is invalid. Exiting...\n", finesse_num);
  print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  sleep(12345678);
  exit(-1);
  return 0;
}

int* RawCOPPER::Get1stFINESSEBuffer(int n)
{
  int pos_nwords = GetOffset1stFINESSE(n);
  if (pos_nwords >= m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ pos_nwords]);
}

int* RawCOPPER::Get2ndFINESSEBuffer(int n)
{
  int pos_nwords = GetOffset2ndFINESSE(n);
  if (pos_nwords >= m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ pos_nwords]);
}

int* RawCOPPER::Get3rdFINESSEBuffer(int n)
{
  int pos_nwords = GetOffset3rdFINESSE(n);
  if (pos_nwords >= m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ pos_nwords]);
}

int* RawCOPPER::Get4thFINESSEBuffer(int n)
{
  int pos_nwords = GetOffset4thFINESSE(n);
  if (pos_nwords >= m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ pos_nwords]);
}




int* RawCOPPER::GetDetectorBuffer(int n, int finesse_num)
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

  ErrorMessage print_err;
  char err_buf[500];
  sprintf(err_buf, "Specifined FINESSE number( = %d ) is invalid. Exiting...\n", finesse_num);
  print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  sleep(12345678);
  exit(-1);
  return 0;
}


int* RawCOPPER::Get1stDetectorBuffer(int n)
{
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  if (Get1stFINESSENwords(n) > 0) {
    int pos_nwords = GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER;
    return &(m_buffer[ pos_nwords ]);
  }
  return NULL;
}

int* RawCOPPER::Get2ndDetectorBuffer(int n)
{
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  if (Get2ndFINESSENwords(n) > 0) {
    int pos_nwords = GetOffset2ndFINESSE(n) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER;
    return &(m_buffer[ pos_nwords ]);
  }
  return NULL;
}

int* RawCOPPER::Get3rdDetectorBuffer(int n)
{
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  if (Get3rdFINESSENwords(n) > 0) {
    int pos_nwords = GetOffset3rdFINESSE(n) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER;
    return &(m_buffer[ pos_nwords ]);
  }
  return NULL;
}

int* RawCOPPER::Get4thDetectorBuffer(int n)
{
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  if (Get4thFINESSENwords(n) > 0) {
    int pos_nwords = GetOffset4thFINESSE(n) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER;
    return &(m_buffer[ pos_nwords ]);
  }
  return NULL;
}

int* RawCOPPER::GetExpRunBuf(int n)
{
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  int pos_nwords = GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + POS_EXP_RUN;
  return &(m_buffer[ pos_nwords ]);
}


unsigned int RawCOPPER::GetB2LFEE32bitEventNumber(int n)
{
  ErrorMessage print_err;

#ifndef READ_OLD_B2LFEE_FORMAT_FILE

  int err_flag = 0;
  unsigned int eve_num = 0;
  int flag = 0;
  unsigned int eve[4];
  for (int i = 0; i < 4 ; i++) {
    eve[ i ] = 12345678;
    if (GetFINESSENwords(n, i) > 0) {
      int pos_nwords = GetOffsetFINESSE(n, i) + SIZE_B2LHSLB_HEADER + POS_TT_TAG;
      eve[ i ] = m_buffer[ pos_nwords ];
      if (flag != 1) eve_num = eve[ i ];
      if (eve_num != eve[ i ]) err_flag = 1;
      flag = 1;
    }
  }

  if (flag == 0) {
    char err_buf[500];
    sprintf(err_buf, "No HSLB data in COPPER data. Exiting...\n");
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(12345678);
    exit(-1);
  }

  if (err_flag == 1) {
#ifndef NO_DATA_CHECK
    char err_buf[500];
    sprintf(err_buf, "Different event number over HSLBs : slot A 0x%x : B 0x%x :C 0x%x : D 0x%x\n",
            eve[ 0 ], eve[ 1 ], eve[ 2 ], eve[ 3 ]);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(12345678);
    exit(-1);
#endif //NO_DATA_CHECK
  }
  return eve_num;

#else // READ_OLD_B2LFEE_FORMAT_FILE

  char err_buf[500];
  sprintf(err_buf, "You need comment out READ_OLD_B2LFEE_FORMAT_FILE if you are handling a new data format\n");
  print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  sleep(12345678);
  exit(1);
  return 0;

#endif // READ_OLD_B2LFEE_FORMAT_FILE

}



unsigned int RawCOPPER::GetMagicDriverHeader(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_MAGIC_COPPER_1;
  return (unsigned int)(m_buffer[ pos_nwords ]);
}

unsigned int RawCOPPER::GetMagicFPGAHeader(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_MAGIC_COPPER_2;
  return (unsigned int)(m_buffer[ pos_nwords ]);
}

unsigned int RawCOPPER::GetMagicFPGATrailer(int n)
{
  RawTrailer trl;
  int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - trl.GetTrlNwords() - 3;
  return (unsigned int)(m_buffer[ pos_nwords ]);
}

unsigned int RawCOPPER::GetMagicDriverTrailer(int n)
{
  RawTrailer trl;
  int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - trl.GetTrlNwords() - 1;
  return (unsigned int)(m_buffer[ pos_nwords ]);
}


bool RawCOPPER::CheckCOPPERMagic(int n)
{
  if (GetMagicDriverHeader(n) != COPPER_MAGIC_DRIVER_HEADER) {
    return false;
  } else if (GetMagicFPGAHeader(n) != COPPER_MAGIC_FPGA_HEADER) {
    return false;
  } else if (GetMagicFPGATrailer(n) != COPPER_MAGIC_FPGA_TRAILER) {
    return false;
  } else if (GetMagicDriverTrailer(n) != COPPER_MAGIC_DRIVER_TRAILER) {
    return false;
  }
  return true;
}



double RawCOPPER::GetEventUnixTime(int n)
{
#ifndef READ_OLD_B2LFEE_FORMAT_FILE
  int pos, run;
  double retval;
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  pos = GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + POS_EXP_RUN;
  run = m_buffer[ pos ] & 0x3FFFFF;
  retval = (double)(m_buffer[ GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + POS_TT_UTIME ]) +
           (double)((m_buffer[ GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + POS_TT_CTIME_TYPE ] >> 4) & 0x7FFFFFF) / 1.27e8;
  return retval;
#else
  exit(1);
  return 0.;
#endif

}



unsigned int RawCOPPER::GetB2LFEETtCtime(int n)
{
#ifndef READ_OLD_B2LFEE_FORMAT_FILE
  return GetB2LHeaderWord(n, SIZE_B2LHSLB_HEADER + POS_TT_CTIME_TYPE);
#else
  exit(1);
  return 0;
#endif
}



unsigned int RawCOPPER::GetB2LFEETtUtime(int n)
{
#ifndef READ_OLD_B2LFEE_FORMAT_FILE
  return GetB2LHeaderWord(n, SIZE_B2LHSLB_HEADER + POS_TT_UTIME);
#else
  exit(1);
  return 0;
#endif
}


unsigned int RawCOPPER::GetB2LHeaderWord(int n, int finesse_buffer_pos)
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

  ErrorMessage print_err;
  if (flag == 0) {
    char err_buf[500];
    sprintf(err_buf, "No HSLB data in COPPER data. Exiting...\n");
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(12345678);
    exit(-1);
  }


  if (err_flag == 1) {
#ifndef NO_DATA_CHECK
    char err_buf[500];
    sprintf(err_buf, "Different event number over HSLBs : slot A 0x%x : B 0x%x :C 0x%x : D 0x%x\n",
            word[ 0 ], word[ 1 ], word[ 2 ], word[ 3 ]);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(12345678);
    exit(-1);
#endif
  }

  return ret_word;

}


#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
void RawCOPPER::CheckB2LFEEHeaderVersion(int n)
{
  int flag = 0;
  int* temp_buf;
  ErrorMessage print_err;

  for (int i = 0; i < 4; i++) {
    if (GetFINESSENwords(n, i) > 0) {
      temp_buf = GetFINESSEBuffer(n, i);
      if ((temp_buf[ 3 ] & 0x40000000) == 0) {
        // this word for exp/run
        flag = 1; // old one (ver.1) used for SPring8 test in 2013
        printf("\033[31m");
        printf("===Firmware ver. ERROR===\n ");
        printf("FTSW and b2tt firmwares was updated on Nov.22, 2013 and the header format attached by B2link was changed in the new firmwares.\n");
        printf("If you are going to take data now, Please update the firmware.\n");
        printf("For details, please see Nakao-san's e-mail [b2link_ml:0111] Re: [daq2ml:0159] beta version of trigger timing receiver firmware (b2tt) on bdaq SVN\n");
        printf("Or if you are going to read data taken before the update, please use basf2 software before rev. 7419 in  https://belle2.cc.kek.jp/svn/trunk/software/daq/\n");
        printf("About the format please see Nakao-san's B2GM slides(p. 13 and 15) http://kds.kek.jp/getFile.py/access?contribId=143&sessionId=38&resId=0&materialId=slides&confId=13911.\n");
        printf("Sorry for inconvenience.\n");
        printf("\033[0m");
        fflush(stdout);
        char err_buf[500];
        sprintf(err_buf, "FTSW and b2tt firmwares are old. Exiting...\n");
        print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
        sleep(12345678);
        exit(-1);
      } else {
        // this word for 32bit unixtime
        flag = 2; // new one (ver.2)
        break;
      }
    }

    if (i == 3) {

      char err_buf[500];
      sprintf(err_buf, "RawCOPPER contains no FINESSE data. Exiting...\n");
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(12345678);
      exit(-1);
    }
  }
  return;
}
#endif
