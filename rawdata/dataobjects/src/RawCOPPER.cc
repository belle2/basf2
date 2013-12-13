//+
// File : RawCOPPER.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawCOPPER.h>

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


unsigned int RawCOPPER::FillTopBlockRawHeader(unsigned int m_node_id, unsigned int m_data_type, unsigned int m_trunc_mask)
{

  ErrorMessage print_err;
  //
  // This function only fills RawHeader contents for the first datablock.
  //
  if (m_num_nodes * m_num_events != 1) {
    char err_buf[500];
    sprintf(err_buf,
            "This function should be used for RawCOPPER containing only one datablock, while. this object has num_nodes of %d and num_events of %d\n",
            m_num_nodes, m_num_events);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(12345678);
    exit(-1);
  }
  memset(m_buffer, 0, sizeof(int) * RawHeader::RAWHEADER_NWORDS);
  m_buffer[ RawHeader::POS_HDR_NWORDS ] = RawHeader::RAWHEADER_NWORDS;

  //
  // Check FINESSEs which containes data
  //
  int* copper_buf = &(m_buffer[ RawHeader::RAWHEADER_NWORDS ]);

  if (copper_buf[ POS_CH_A_DATA_LENGTH ] == 0 &&
      copper_buf[ POS_CH_B_DATA_LENGTH ] == 0 &&
      copper_buf[ POS_CH_C_DATA_LENGTH ] == 0 &&
      copper_buf[ POS_CH_D_DATA_LENGTH ] == 0) {
    char err_buf[500];
    sprintf(err_buf,
            "No FINESSE data in a copper data block. Exiting...\n");
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(12345678);
    exit(-1);
  }


  // initialize header(header nwords, magic word) and trailer(magic word)
  //   RawHeader rawhdr;
  //   rawhdr.SetBuffer(raw_copper->GetRawHdrBufPtr(cprblock));
  //   rawhdr.Initialize(); // Fill 2nd( hdr size) and 20th header word( magic word )

  // 1, Set total words info
  //   int nwords = raw_copper->GetBlockNwords(cprblock);
  //   rawhdr.SetNwords(nwords);

  int datablock_nwords = RawHeader::RAWHEADER_NWORDS +
                         (copper_buf[ POS_DATA_LENGTH ] + COPPER_HEADER_TRAILER_NWORDS) +
                         RawTrailer::RAWTRAILER_NWORDS;

  m_buffer[ RawHeader::POS_NWORDS ] = datablock_nwords;


  if (m_buffer[ RawHeader::POS_NWORDS ] != m_nwords) {
    char err_buf[500];
    sprintf(err_buf,
            "Data length is inconsistent m_nwords %d : nwords from COPPER data %d\n",
            m_nwords, m_buffer[ RawHeader::POS_NWORDS ]);
    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    sleep(12345678);
    exit(-1);
  }

  //   // 2, Set run and exp #
  //   rawhdr.SetExpRunNumber(raw_copper->GetExpRunBuf(cprblock));       // Fill 3rd header word
  int offset_1st_finesse =  RawHeader::RAWHEADER_NWORDS + SIZE_COPPER_HEADER;
  int offset_2nd_finesse = offset_1st_finesse + copper_buf[ POS_CH_A_DATA_LENGTH ];
  int offset_3rd_finesse = offset_2nd_finesse + copper_buf[ POS_CH_B_DATA_LENGTH ];
  int offset_4th_finesse = offset_3rd_finesse + copper_buf[ POS_CH_C_DATA_LENGTH ];
  int* finesse_buf = &(m_buffer[ offset_1st_finesse ]); // In any finesse implementations, the top finesse buffer should be at offset_1st_finesse;
  m_buffer[ RawHeader::POS_EXP_RUN_NO ] = finesse_buf[ SIZE_B2LHSLB_HEADER + POS_EXP_RUN ];

  // 3, Make 32bit event number from B2link FEE header
  //  unsigned int cur_ftsw_eve32 =  raw_copper->GetB2LFEE32bitEventNumber(cprblock);
  //  rawhdr.SetEveNo(cur_ftsw_eve32);       // Temporarily use COPPER counter   //raw_copper->GetCOPPERCounter()
  unsigned int cur_ftsw_eve32 =  finesse_buf[ SIZE_B2LHSLB_HEADER + POS_TT_TAG ];
  m_buffer[ RawHeader::POS_EVE_NO ] = cur_ftsw_eve32;

  // Set FTSW word
  //  rawhdr.SetFTSW2Words(raw_copper->GetB2LFEETtCtime(cprblock), raw_copper->GetB2LFEETtUtime(cprblock));
  m_buffer[ RawHeader::POS_HSLB_1 ] = finesse_buf[ SIZE_B2LHSLB_HEADER + POS_TT_CTIME_TYPE ];
  m_buffer[ RawHeader::POS_HSLB_2 ] = finesse_buf[ SIZE_B2LHSLB_HEADER + POS_TT_UTIME ];

  // Obtain info from SlowController via AddParam or COPPER data
//   rawhdr.SetSubsysId(m_nodeid);   // Fill 7th header word
//   rawhdr.SetDataType(m_data_type);   // Fill 8th header word
//   rawhdr.SetTruncMask(m_trunc_mask);   // Fill 8th header word
  m_buffer[ RawHeader::POS_SUBSYS_ID ] = m_node_id;
  m_buffer[ RawHeader::POS_TRUNC_MASK_DATATYPE ] = ((m_trunc_mask << 32) & 0x80000000) | (m_data_type & 0x7FFFFFFF);

  // Offset
//   rawhdr.SetOffset1stFINESSE(raw_copper->GetOffset1stFINESSE(cprblock) - raw_copper->GetBufferPos(cprblock));          // Fill 9th header word
//   rawhdr.SetOffset2ndFINESSE(raw_copper->GetOffset2ndFINESSE(cprblock) - raw_copper->GetBufferPos(cprblock));         // Fill 10th header word
//   rawhdr.SetOffset3rdFINESSE(raw_copper->GetOffset3rdFINESSE(cprblock) - raw_copper->GetBufferPos(cprblock));         // Fill 11th header word
//   rawhdr.SetOffset4thFINESSE(raw_copper->GetOffset4thFINESSE(cprblock) - raw_copper->GetBufferPos(cprblock));         // Fill 12th header word
  m_buffer[ RawHeader::POS_OFFSET_1ST_FINESSE ] = offset_1st_finesse;
  m_buffer[ RawHeader::POS_OFFSET_2ND_FINESSE ] = offset_2nd_finesse;
  m_buffer[ RawHeader::POS_OFFSET_3RD_FINESSE ] = offset_3rd_finesse;
  m_buffer[ RawHeader::POS_OFFSET_4TH_FINESSE ] = offset_4th_finesse;


  // Set magic word
  //  rawhdr.SetMagicWordEntireHeader();
  m_buffer[ RawHeader::POS_TERM_HEADER ] = RawHeader::MAGIC_WORD_TERM_HEADER;


  // Add node-info
  //  rawhdr.AddNodeInfo(m_nodeid);   // Fill 13th header word
  if (m_buffer[ RawHeader::POS_NUM_NODES ] < RawHeader::NUM_MAX_NODES) {
    m_buffer[ RawHeader::POS_NODES_1 + m_buffer[ RawHeader::POS_NUM_NODES ] ] = m_node_id;
  }
  m_buffer[ RawHeader::POS_NUM_NODES ]++;


  //
  // Fill info in Trailer
  //
  //   RawTrailer rawtrl;
  //   rawtrl.SetBuffer(raw_copper->GetRawTrlBufPtr(cprblock));
  //   rawtrl.Initialize(); // Fill 2nd word : magic word
  //   rawtrl.SetChksum(CalcSimpleChecksum(raw_copper->GetBuffer(cprblock),
  //   raw_copper->GetBlockNwords(cprblock) - rawtrl.GetTrlNwords()));
  int* trl = &(m_buffer[ datablock_nwords - RawTrailer::RAWTRAILER_NWORDS ]);
  trl[ RawTrailer::POS_TERM_WORD ] = RawTrailer::MAGIC_WORD_TERM_TRAILER;
  unsigned int chksum = 0;
  for (int i = 0; i < datablock_nwords - RawTrailer::RAWTRAILER_NWORDS; i++) {
    chksum += m_buffer[ i ];
  }
  trl[ RawTrailer::POS_CHKSUM ] = chksum;



//   //magic word check
// #ifndef NO_DATA_CHECK
// // 3, magic word check
//   if (!(raw_copper->CheckCOPPERMagic(cprblock))) {
//     char err_buf[500];
//     sprintf(err_buf, "Invalid Magic word 0x7FFFF0008=%u 0xFFFFFAFA=%u 0xFFFFF5F5=%u 0x7FFF0009=%u\n",
//             raw_copper->GetMagicDriverHeader(cprblock),
//             raw_copper->GetMagicFPGAHeader(cprblock),
//             raw_copper->GetMagicFPGATrailer(cprblock),
//             raw_copper->GetMagicDriverTrailer(cprblock));
//     print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
//     sleep(12345678);
//     exit(-1);
//   }


//   // 3, event # increment check
// #ifdef WO_FIRST_EVENUM_CHECK
//   if ((m_prev_ftsweve32 + 1 != cur_ftsw_eve32) && (m_prev_ftsweve32 != 0xFFFFFFFF)) {
// #else
//   if (m_prev_ftsweve32 + 1 != cur_ftsw_eve32) {
// #endif
//     char err_buf[500];
//     sprintf(err_buf, "Invalid event_number. Exiting...: cur 32bit eve %u preveve %u\n",  cur_ftsw_eve32, m_prev_ftsweve32);
//     print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);

//     printf("i= %d : num entries %d : Tot words %d\n", 0 , raw_copper->GetNumEntries(), raw_copper->TotalBufNwords());
//     for (int j = 0; j < raw_copper->TotalBufNwords(); j++) {
//       printf("0x%.8x ", (raw_copper->GetBuffer(0))[ j ]);
//       if ((j % 10) == 9)printf("\n");
//       fflush(stdout);
//     }

//     exit(-1);
//   }
// #endif
  return cur_ftsw_eve32;

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
#ifdef TEMP
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
#endif
      } else {
        // this word for 32bit unixtime
        flag = 2; // new one (ver.2)
        break;
      }
    }

    if (i == 3) {
#ifdef TEMP
      char err_buf[500];
      sprintf(err_buf, "RawCOPPER contains no FINESSE data. Exiting...\n");
      print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      sleep(12345678);
      exit(-1);
#endif
    }
  }
  return;
}
#endif
