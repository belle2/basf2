//+
// File : PreRawCOPPER.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <daq/dataobjects/PreRawCOPPER.h>


using namespace std;
using namespace Belle2;

//#define DESY
#define NO_DATA_CHECK
#define WO_FIRST_EVENUM_CHECK

ClassImp(PreRawCOPPER);

PreRawCOPPER::PreRawCOPPER()
{
}

PreRawCOPPER::~PreRawCOPPER()
{
}




int PreRawCOPPER::GetBufferPos(int n)
{
  if (m_buffer == NULL || m_nwords <= 0) {
    printf("[DEBUG] [ERROR] RawPacket buffer(%p) is not available or length(%d) is not set.\n", m_buffer, m_nwords);
    exit(1);
  }

  if (n >= (m_num_events * m_num_nodes)) {
    printf("[DEBUG] Invalid COPPER block No. (%d : max %d ) is specified. Exiting... ", n, (m_num_events * m_num_nodes));
    exit(1);
  }
  int pos_nwords = 0;
  for (int i = 1; i <= n ; i++) {
    int size = tmp_header.RAWHEADER_NWORDS
               + m_buffer[ pos_nwords + tmp_header.RAWHEADER_NWORDS + POS_DATA_LENGTH ]
               + SIZE_COPPER_DRIVER_HEADER
               + SIZE_COPPER_DRIVER_TRAILER
               + tmp_trailer.RAWTRAILER_NWORDS;
    // COPPER's data length include one word from COPPER trailer. so -1 is needed.
    pos_nwords +=  size;
    if (pos_nwords >= m_nwords) {
      char err_buf[500];
      sprintf(err_buf, "CORRUPTED DATA: value of pos_nwords(%d) is larger than m_nwords(%d). Exiting...\n %s %s %d\n",
              pos_nwords, m_nwords, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      string err_str = err_buf;     throw (err_str);
      exit(1);
    }
  }
  return pos_nwords;
}



unsigned int PreRawCOPPER::CalcDriverChkSum(int n)
{
  int min = GetBufferPos(n) + ReducedRawHeader::RAWHEADER_NWORDS;
  int max = GetBufferPos(n) + GetBlockNwords(n)
            - ReducedRawTrailer::RAWTRAILER_NWORDS - SIZE_COPPER_DRIVER_TRAILER;
  unsigned int chksum = 0;
  for (int i = min; i < max; i++) {
    chksum ^= m_buffer[ i ];
  }
  GetDriverChkSum(n);
  return chksum;
}


int PreRawCOPPER::GetNumFINESSEBlock(int n)
{

  ReducedRawHeader hdr;
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

int PreRawCOPPER::GetDetectorNwords(int n, int finesse_num)
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

  char err_buf[500];
  sprintf(err_buf, "Specifined FINESSE number( = %d ) is invalid. Exiting...\n %s %s %d\n", finesse_num,
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf; throw (err_str);
  sleep(12345678);
  exit(-1);
  return 0;
}


int PreRawCOPPER::GetFINESSENwords(int n, int finesse_num)
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

  char err_buf[500];
  sprintf(err_buf, "Specifined FINESSE number( = %d ) is invalid. Exiting...\n %s %s %d\n",
          finesse_num, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf; throw (err_str);
  sleep(12345678);
  exit(-1);
  return 0;

}



int PreRawCOPPER::GetOffsetFINESSE(int n, int finesse_num)
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
  sleep(12345678);
  exit(-1);
  return 0;

}




int* PreRawCOPPER::GetFINESSEBuffer(int n, int finesse_num)
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
  sleep(12345678);
  exit(-1);
  return 0;
}



int* PreRawCOPPER::GetDetectorBuffer(int n, int finesse_num)
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
  sleep(12345678);
  exit(-1);
  return 0;
}


unsigned int PreRawCOPPER::GetB2LFEE32bitEventNumber(int n)
{

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
    sprintf(err_buf, "No HSLB data in COPPER data. Exiting...\n%s %s %d\n",
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf; throw (err_str);
    sleep(12345678);
    exit(-1);
  }

  if (err_flag == 1) {

    char err_buf[500];
    sprintf(err_buf, "CORRUPTED DATA: Different event number over HSLBs : slot A 0x%x : B 0x%x :C 0x%x : D 0x%x\n%s %s %d\n",
            eve[ 0 ], eve[ 1 ], eve[ 2 ], eve[ 3 ],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] [ERROR] %s\n", err_buf);
#ifndef NO_DATA_CHECK
    string err_str = err_buf; throw (err_str);

    sleep(12345678);
    exit(-1);
#endif //NO_DATA_CHECK
  }
  return eve_num;

#else // READ_OLD_B2LFEE_FORMAT_FILE

  char err_buf[500];
  sprintf(err_buf, "You need comment out READ_OLD_B2LFEE_FORMAT_FILE if you are handling a new data format\n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf; throw (err_str);
  sleep(12345678);
  exit(1);
  return 0;

#endif // READ_OLD_B2LFEE_FORMAT_FILE

}



unsigned int  PreRawCOPPER::CalcXORChecksum(int* buf, int nwords)
{
  unsigned int checksum = 0;
  for (int i = 0; i < nwords; i++) {

    checksum = checksum ^ buf[ i ];
  }
  return checksum;
}


void PreRawCOPPER::CheckData(int n,
                             unsigned int prev_evenum, unsigned int* cur_evenum_rawcprhdr,
                             unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                             int prev_runsubrun_no, int* cur_runsubrun_no)
{

  char err_buf[500];
  int err_flag = 0;
  //
  // check Magic words
  //
  if (!CheckCOPPERMagic(n)) {
    sprintf(err_buf, "CORRUPTED DATA: Invalid Magic word 0x7FFFF0008=%u 0xFFFFFAFA=%u 0xFFFFF5F5=%u 0x7FFF0009=%u\n%s %s %d\n",
            GetMagicDriverHeader(n),
            GetMagicFPGAHeader(n),
            GetMagicFPGATrailer(n),
            GetMagicDriverTrailer(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }

  //
  // Event # check
  //
  *cur_evenum_rawcprhdr = GetEveNo(n);
  unsigned int evenum_feehdr = GetB2LFEE32bitEventNumber(n);
  if (*cur_evenum_rawcprhdr != evenum_feehdr) {
    sprintf(err_buf, "CORRUPTED DATA: Event # in PreRawCOPPER header and FEE header is different : cprhdr 0x%x feehdr 0x%x : Exiting...\n%s %s %d\n",
            *cur_evenum_rawcprhdr, evenum_feehdr,
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }

  //
  // Check incrementation of event #
  //
  *cur_runsubrun_no = GetRunNoSubRunNo(n);
#ifdef WO_FIRST_EVENUM_CHECK
  if (prev_evenum != 0xFFFFFFFF && *cur_evenum_rawcprhdr != 0)  {
#else
  if (prev_runsubrun_no == *cur_runsubrun_no && prev_runsubrun_no >= 0) {
#endif
    if ((unsigned int)(prev_evenum + 1) != *cur_evenum_rawcprhdr) {
      sprintf(err_buf, "CORRUPTED DATA: Event # jump : i %d prev 0x%x cur 0x%x : Exiting...\n%s %s %d\n",
              n, prev_evenum, *cur_evenum_rawcprhdr,
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      err_flag = 1;
    }
  }


  *cur_copper_ctr = GetCOPPERCounter(n);
#ifdef WO_FIRST_EVENUM_CHECK
  if (prev_copper_ctr != 0xFFFFFFFF) {
#else
  if (true) {
#endif
    if ((unsigned int)(prev_copper_ctr + 1) != *cur_copper_ctr) {
      sprintf(err_buf, "COPPER counter jump : i %d prev 0x%x cur 0x%x :\n%s %s %d\n",
              n, prev_copper_ctr, *cur_copper_ctr,
              __FILE__, __PRETTY_FUNCTION__, __LINE__);

#ifdef DESY
      //
      // In DESY test, we ignore this error
      //
      printf("[DEBUG] [INFO] %s", err_buf);
#else
      err_flag = 1;
#endif

    }
  }

  //
  // Check is utime and ctime_trgtype same over different FINESSE data
  //
  CheckUtimeCtimeTRGType(n);


  //
  // Check checksum calculated by COPPER driver
  //
  if (GetDriverChkSum(n) != CalcDriverChkSum(n)) {
    sprintf(err_buf, "CORRUPTED DATA: COPPER driver checkSum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x\n%s %s %d\n",
            n,
            GetBlockNwords(n),
            *cur_evenum_rawcprhdr,
            GetDriverChkSum(n),
            CalcDriverChkSum(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }


  //
  // Check checksum calculated by DeSerializerCOPPER()
  //
  ReducedRawTrailer rawtrl;
  rawtrl.SetBuffer(GetRawTrlBufPtr(n));
  if (rawtrl.GetChksum() !=
      CalcXORChecksum(GetBuffer(n), GetBlockNwords(n) - rawtrl.GetTrlNwords())) {
    sprintf(err_buf, "CORRUPTED DATA: PreRawCOPPER checksum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x\n %s %s %d\n",
            n, GetBlockNwords(n), *cur_evenum_rawcprhdr, rawtrl.GetChksum(),
            CalcXORChecksum(GetBuffer(n), GetBlockNwords(n) - rawtrl.GetTrlNwords()),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }


#ifdef DEBUG
  printf("[DEBUG] eve %d %d %d %d %d\n",
         GetEveNo(n),
         Get1stDetectorNwords(n),
         Get2ndDetectorNwords(n),
         Get3rdDetectorNwords(n),
         Get4thDetectorNwords(n)
        );
  printf("[DEBUG] ===COPPER BLOCK==============\n");
  printData(GetBuffer(n), GetBlockNwords(n));

  printf("[DEBUG] ===FINNESSE A ==============\n");
  printData(Get1stDetectorBuffer(n), Get1stDetectorNwords(n));

  printf("[DEBUG] ===FINNESSE B ==============\n");
  printData(Get2ndDetectorBuffer(n), Get2ndDetectorNwords(n));

  printf("[DEBUG] ===FINNESSE C ==============\n");
  printData(Get3rdDetectorBuffer(n), Get3rdDetectorNwords(n));

  printf("[DEBUG] ===FINNESSE D ==============\n");
  printData(Get4thDetectorBuffer(n), Get4thDetectorNwords(n));
  printf("[DEBUG] === END ==============\n");

#endif

  if (err_flag == 1) {
    printf("[DEBUG] ========== dump a data blcok : block # %d==========\n", n);
    PrintData(GetBuffer(n), GetBlockNwords(n));
    string err_str = err_buf;
    throw (err_str);
    sleep(1234567);
    exit(-1);
  }

  return;

}

bool PreRawCOPPER::CheckCOPPERMagic(int n)
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

void PreRawCOPPER::CheckUtimeCtimeTRGType(int n)
{

#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  int err_flag = 0;
  int flag = 0;
  unsigned int temp_utime = 0, temp_ctime_trgtype = 0;
  unsigned int utime[4], ctime_trgtype[4];
  memset(utime, 0, sizeof(utime));
  memset(ctime_trgtype, 0, sizeof(ctime_trgtype));

  for (int i = 0; i < 4; i++) {
    if (GetFINESSENwords(n, i) > 0) {
      ctime_trgtype[ i ] = m_buffer[ GetOffsetFINESSE(n, i) +
                                     SIZE_B2LHSLB_HEADER + POS_TT_CTIME_TYPE ];
      utime[ i ] = m_buffer[ GetOffsetFINESSE(n, i) +
                             SIZE_B2LHSLB_HEADER + POS_TT_UTIME ];
      if (flag == 0) {
        temp_ctime_trgtype = ctime_trgtype[ i ];
        temp_utime = utime[ i ];
        flag = 1;
      } else {
        if (temp_ctime_trgtype != ctime_trgtype[ i ]
            || temp_utime != utime[ i ]) {
          err_flag = 1;
        }
      }
    }
  }

  if (err_flag != 0) {
    for (int i = 0; i < 4; i++) {
      printf("[DEBUG] FINESSE #=%d buffsize %d ctimeTRGtype 0x%.8x utime 0x%.8x\n",
             i, GetFINESSENwords(n, i), ctime_trgtype[ i ], utime[ i ]);
    }
    char err_buf[500];
    sprintf(err_buf, "CORRUPTED DATA: mismatch over FINESSEs. Exiting...\n %s %s %d\n",
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf; throw (err_str);
    sleep(1234567);
    exit(1);
  }
  return;
}



double PreRawCOPPER::GetEventUnixTime(int n)
{
#ifndef READ_OLD_B2LFEE_FORMAT_FILE
  double retval;
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  retval = (double)(m_buffer[ GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + POS_TT_UTIME ]) +
           (double)((m_buffer[ GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + POS_TT_CTIME_TYPE ] >> 4) & 0x7FFFFFF) / 1.27e8;
  return retval;
#else
  exit(1);
  return 0.;
#endif

}


unsigned int PreRawCOPPER::GetB2LHeaderWord(int n, int finesse_buffer_pos)
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
    sleep(12345678);
    exit(-1);
  }


  if (err_flag == 1) {

    char err_buf[500];
    sprintf(err_buf, "CORRUPTED DATA: Different event number over HSLBs : slot A 0x%x : B 0x%x :C 0x%x : D 0x%x\n %s %s %d\n",
            word[ 0 ], word[ 1 ], word[ 2 ], word[ 3 ],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] [ERROR] %s\n", err_buf);
#ifndef NO_DATA_CHECK
    string err_str = err_buf; throw (err_str);

    sleep(12345678);
    exit(-1);
#endif
  }

  return ret_word;

}


unsigned int PreRawCOPPER::FillTopBlockRawHeader(unsigned int m_node_id, unsigned int m_data_type,
                                                 unsigned int m_trunc_mask, unsigned int prev_eve32,
                                                 int prev_runsubrun_no, int* cur_runsubrun_no)
{
  const int cpr_id = 0;
  //
  // This function only fills ReducedRawHeader contents for the first datablock.
  // # of block should be 1
  if (m_num_nodes * m_num_events != 1) {
    char err_buf[500];
    sprintf(err_buf,
            "This function should be used for PreRawCOPPER containing only one datablock, while. this object has num_nodes of %d and num_events of %d\n %s %s %d\n",
            m_num_nodes, m_num_events,  __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf;    throw (err_str);
  }

  //////////////////////////////////////////////////
  //
  // Fill info in ReducedRawHeader
  //
  //////////////////////////////////////////////////

  //
  // Initialize a ReducedRawHeader part
  //
  memset(m_buffer, 0, sizeof(int) * ReducedRawHeader::RAWHEADER_NWORDS);
  m_buffer[ ReducedRawHeader::POS_HDR_NWORDS ] = ReducedRawHeader::RAWHEADER_NWORDS;

  //
  // Check FINESSEs which containes data
  //
  int* copper_buf = &(m_buffer[ ReducedRawHeader::RAWHEADER_NWORDS ]);
  if (copper_buf[ POS_CH_A_DATA_LENGTH ] == 0 &&
      copper_buf[ POS_CH_B_DATA_LENGTH ] == 0 &&
      copper_buf[ POS_CH_C_DATA_LENGTH ] == 0 &&
      copper_buf[ POS_CH_D_DATA_LENGTH ] == 0) {
    char err_buf[500];
    sprintf(err_buf,
            "No FINESSE data in a copper data block. Exiting...\n %s %s %d\n",
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf; throw (err_str);
    sleep(12345678);
    exit(-1);
  }

  //
  //   Set total words info
  //
  int datablock_nwords =
    ReducedRawHeader::RAWHEADER_NWORDS +
    (copper_buf[ POS_DATA_LENGTH ]
     + SIZE_COPPER_DRIVER_HEADER
     + SIZE_COPPER_DRIVER_TRAILER)
    + ReducedRawTrailer::RAWTRAILER_NWORDS;
  m_buffer[ ReducedRawHeader::POS_NWORDS ] = datablock_nwords;

  //
  // Check the consistency between data length and length in ReducedRawHeader
  //
  if (m_buffer[ ReducedRawHeader::POS_NWORDS ] != m_nwords) {
    char err_buf[500];
    sprintf(err_buf,
            "CORRUPTED DATA: Data length is inconsistent m_nwords %d : nwords from COPPER data %d\n %s %s %d\n",
            m_nwords, m_buffer[ ReducedRawHeader::POS_NWORDS ],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf; throw (err_str);
    sleep(12345678);
    exit(-1);
  }

  //
  // Fill offset values
  //
  int offset_1st_finesse =  ReducedRawHeader::RAWHEADER_NWORDS + SIZE_COPPER_HEADER;
  int offset_2nd_finesse = offset_1st_finesse + copper_buf[ POS_CH_A_DATA_LENGTH ];
  int offset_3rd_finesse = offset_2nd_finesse + copper_buf[ POS_CH_B_DATA_LENGTH ];
  int offset_4th_finesse = offset_3rd_finesse + copper_buf[ POS_CH_C_DATA_LENGTH ];
  m_buffer[ ReducedRawHeader::POS_OFFSET_1ST_FINESSE ] = offset_1st_finesse;
  m_buffer[ ReducedRawHeader::POS_OFFSET_2ND_FINESSE ] = offset_2nd_finesse;
  m_buffer[ ReducedRawHeader::POS_OFFSET_3RD_FINESSE ] = offset_3rd_finesse;
  m_buffer[ ReducedRawHeader::POS_OFFSET_4TH_FINESSE ] = offset_4th_finesse;

  //
  // Fill Exp/Run value
  //
  int* finesse_buf = &(m_buffer[ offset_1st_finesse ]); // In any finesse implementations, the top finesse buffer should be at offset_1st_finesse;
  m_buffer[ ReducedRawHeader::POS_EXP_RUN_NO ] = finesse_buf[ SIZE_B2LHSLB_HEADER + POS_EXP_RUN ];


  //
  // Fill event #
  //
  unsigned int cur_ftsw_eve32 =  finesse_buf[ SIZE_B2LHSLB_HEADER + POS_TT_TAG ];
  m_buffer[ ReducedRawHeader::POS_EVE_NO ] = cur_ftsw_eve32;

  //
  // Copy FTSW words from B2LFEE header
  //
  m_buffer[ ReducedRawHeader::POS_TTCTIME_TRGTYPE ] = finesse_buf[ SIZE_B2LHSLB_HEADER + POS_TT_CTIME_TYPE ];
  m_buffer[ ReducedRawHeader::POS_TTUTIME ] = finesse_buf[ SIZE_B2LHSLB_HEADER + POS_TT_UTIME ];

  //
  // Set node ID, trunc_mask, data_type
  //
  m_buffer[ ReducedRawHeader::POS_SUBSYS_ID ] = m_node_id;
  m_buffer[ ReducedRawHeader::POS_TRUNC_MASK_DATATYPE ] = ((m_trunc_mask << 31) & 0x80000000) | (m_data_type & 0x7FFFFFFF);

  //
  // Set ReducedRawHeader magic word
  //
  //  m_buffer[ ReducedRawHeader::POS_TERM_HEADER ] = ReducedRawHeader::MAGIC_WORD_TERM_HEADER;


  //
  // Add node-info
  //
//   if (m_buffer[ ReducedRawHeader::POS_NUM_NODES ] < ReducedRawHeader::NUM_MAX_NODES) {
//     m_buffer[ ReducedRawHeader::POS_NODES_1 + m_buffer[ ReducedRawHeader::POS_NUM_NODES ] ] = m_node_id;
//   }
//   m_buffer[ ReducedRawHeader::POS_NUM_NODES ]++;



  //////////////////////////////////////////////////
  //
  // Fill info in ReducedRawTrailer
  //
  //////////////////////////////////////////////////

  //
  // Calculate XOR checksum
  //
  unsigned int chksum_top = 0, chksum_body = 0, chksum_bottom = 0;
  int top_end = ReducedRawHeader::RAWHEADER_NWORDS;
  for (int i = 0; i < top_end; i++) {
    chksum_top ^= m_buffer[ i ];
  }
  int body_end = datablock_nwords - SIZE_COPPER_DRIVER_TRAILER - ReducedRawTrailer::RAWTRAILER_NWORDS;
  for (int i = top_end; i < body_end; i++) {
    chksum_body ^= m_buffer[ i ];
  }

  int bottom_end = datablock_nwords - ReducedRawTrailer::RAWTRAILER_NWORDS;
  for (int i = body_end; i < bottom_end; i++) {
    chksum_bottom ^= m_buffer[ i ];
  }

  //
  // check COPPER driver checksum
  //
  if (chksum_body != (unsigned int)(m_buffer[ body_end ])) {
    char err_buf[500];
    sprintf(err_buf, "CORRUPTED DATA: COPPER driver checksum is not consistent.: calcd. %.8x data %.8x\n %s %s %d\n",
            chksum_body, m_buffer[ body_end ],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf; throw (err_str);
    sleep(12345678);
    exit(-1);
  }

  //
  // Fill trailer info (checksum, magic word)
  //
  unsigned int chksum = chksum_top ^ chksum_body ^ chksum_bottom;
  int* trl = &(m_buffer[ datablock_nwords - ReducedRawTrailer::RAWTRAILER_NWORDS ]);
  trl[ ReducedRawTrailer::POS_CHKSUM ] = chksum;
  trl[ ReducedRawTrailer::POS_TERM_WORD ] = ReducedRawTrailer::MAGIC_WORD_TERM_TRAILER;


  //////////////////////////////////////////////////
  //
  // Data check ( magic word, event incrementation )
  //
  //////////////////////////////////////////////////


  //
  // check magic words
  //
  int* fpga_trailer_magic = trl - 3;
  int* driver_trailer_magic = trl - 1;
  int err_flag = 0;
  if ((unsigned int)(copper_buf[ POS_MAGIC_COPPER_1 ]) != COPPER_MAGIC_DRIVER_HEADER) {
    err_flag = 1;
  } else if ((unsigned int)(copper_buf[ POS_MAGIC_COPPER_2 ]) != COPPER_MAGIC_FPGA_HEADER) {
    err_flag = 1;
  } else if ((unsigned int)(*fpga_trailer_magic) != COPPER_MAGIC_FPGA_TRAILER) {
    err_flag = 1;
  } else if ((unsigned int)(*driver_trailer_magic) != COPPER_MAGIC_DRIVER_TRAILER) {
    err_flag = 1;
  }
  if (err_flag == 1) {
    char err_buf[500];
    sprintf(err_buf, "CORRUPTED DATA: Invalid Magic word 0x7FFFF0008=%u 0xFFFFFAFA=%u 0xFFFFF5F5=%u 0x7FFF0009=%u\n %s %s %d\n",
            GetMagicDriverHeader(cpr_id),
            GetMagicFPGAHeader(cpr_id),
            GetMagicFPGATrailer(cpr_id),
            GetMagicDriverTrailer(cpr_id),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] [ERROR] %s\n", err_buf);
#ifndef NO_DATA_CHECK
    string err_str = err_buf; throw (err_str);

    sleep(12345678);
    exit(-1);
#endif
  }

  //
  // check incrementation of event #
  //
  *cur_runsubrun_no = GetRunNoSubRunNo(cpr_id);
  if (prev_runsubrun_no == *cur_runsubrun_no && prev_runsubrun_no >= 0) {
#ifdef WO_FIRST_EVENUM_CHECK
    if ((prev_eve32 + 1 != cur_ftsw_eve32) && (prev_eve32 != 0xFFFFFFFF && cur_ftsw_eve32 != 0)) {
#else
    if (prev_eve32 + 1 != cur_ftsw_eve32) {
#endif

#ifndef NO_DATA_CHECK
      char err_buf[500];
      sprintf(err_buf, "CORRUPTED DATA: Invalid event_number. Exiting...: cur 32bit eve %u preveve %u prun %d crun %d\n %s %s %d\n",  cur_ftsw_eve32, prev_eve32,
              prev_runsubrun_no, *cur_runsubrun_no,
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("[DEBUG] [ERROR] %s\n", err_buf);

      string err_str = err_buf;
      printf("[DEBUG] i= %d : num entries %d : Tot words %d\n", 0 , GetNumEntries(), TotalBufNwords());
      PrintData(GetBuffer(cpr_id), TotalBufNwords());
      throw (err_str);
      exit(-1);
#endif

    }
  }
  return cur_ftsw_eve32;
}


int PreRawCOPPER::CalcReducedNwords(int n)
{
  int nwords_to = 0;

  //RawCOPPER header
  nwords_to += tmp_header.GetHdrNwords();
  for (int j = 0; j < 4; j++) {
    int finesse_nwords = GetFINESSENwords(n, j);
    if (finesse_nwords > 0) {
      //FINESSE buffer
      nwords_to +=
        finesse_nwords
        - SIZE_B2LHSLB_HEADER
        - POS_B2L_CTIME
        - SIZE_B2LFEE_TRAILER
        - SIZE_B2LHSLB_TRAILER;
    }
  }

  //RawCOPPER Trailer
  nwords_to += tmp_trailer.GetTrlNwords();

  return nwords_to;
}


// void PreRawCOPPER::CopyReducedBuffer(int n, int* buf_to ){
//   int* buf_from = GetBuffer(n);
//   int pos_nwords_from = 0, pos_nwords_to = 0;
//   int copy_nwords = 0;


//   //
//   // ReducedRawHeader part
//   //
//   copy_nwords = tmp_header.GetHdrNwords();
//   memcpy( buf_to + pos_nwords_to, buf_from + pos_nwords_to,
//    copy_nwords * sizeof(int) );
//   pos_nwords_to += copy_nwords;
//   pos_nwords_from += copy_nwords;

//   //
//   // COPPER driver/FPGA header part
//   //
//   //  copy_nwords = 0;
//   //   memcpy( buf_to + pos_nwords_to, buf_from + pos_nwords_to,
//   //     copy_nwords * sizeof(int) );
//   //  pos_nwords_to += copy_nwords;
//   pos_nwords_from += tmp_header.SIZE_COPPER_HEADER;

//   //
//   // FINESSE part
//   //

//   int pos_nwords_finesse[ 4 ];
//   for( int i = 0; i < 4; i++){
//     pos_nwords_finesse[ i ] = pos_nwords_to;
//     if( GetFINESSENwords( n, i ) <= 0 )continue;
//     int cur_header_nwrds = SIZE_B2LHSLB_HEADER + POS_B2L_CTIME;
//     int cur_trailer_nwords = SIZE_B2LFEE_TRAILER;
//     int reduced_nwords = cur_header_nwrds + cur_trailer_nwords;
//     pos_nwords_from += cur_header_nwrds;
//     copy_nwords = GetFINESSENwords( n, i ) - reduced_nwords;
//     memcpy( buf_to + pos_nwords_to, buf_from + pos_nwords_to,
//      copy_nwords * sizeof(int) );
//     pos_nwords_to += copy_nwords;
//     pos_nwords_from += copy_nwords + cur_trailer_nwords;
//   }

//   //
//   // COPPER driver/FPGA trailer part
//   //
//   //  copy_nwords = 0;
//   //   memcpy( buf_to + pos_nwords_to, buf_from + pos_nwords_to,
//   //     copy_nwords * sizeof(int) );
//   //pos_nwords_to += copy_nwords;
//   pos_nwords_from += tmp_header.SIZE_COPPER_TRAILER;

//   //
//   // ReducedRawTrailer part
//   //
//   copy_nwords = tmp_header.GetTrlNwords();
//   memcpy( buf_to + pos_nwords_to, buf_from + pos_nwords_to,
//    copy_nwords * sizeof(int) );
//   pos_nwords_to += copy_nwords;
//   pos_nwords_from += copy_nwords;


//   //
//   // Apply changes followed by data size reduction
//   //
//   ReducedRawCOPPER reduced_rawcpr;
//   *(buf_to + reduced_rawcpr.tmp_header.POS_NWORDS) = pos_nwords_to;  //nwords info.
//   *(buf_to + reduced_rawcpr.tmp_header.POS_OFFSET_1ST_FINESSE ) = pos_nwords_finesse[ 0 ];
//   *(buf_to + reduced_rawcpr.tmp_header.POS_OFFSET_2ND_FINESSE ) = pos_nwords_finesse[ 1 ];
//   *(buf_to + reduced_rawcpr.tmp_header.POS_OFFSET_3RD_FINESSE ) = pos_nwords_finesse[ 2 ];
//   *(buf_to + reduced_rawcpr.tmp_header.POS_OFFSET_4TH_FINESSE ) = pos_nwords_finesse[ 3 ];

//   return;
// }



int PreRawCOPPER::CopyReducedBuffer(int n, int* buf_to)
{
  int* buf_from = NULL;
  int nwords_buf_to = CalcReducedNwords(n);
  int pos_nwords_to = 0;
  int copy_nwords = 0;

  //Header copy
  copy_nwords = tmp_header.GetHdrNwords();
  buf_from = GetBuffer(n);
  copyData(buf_to, &pos_nwords_to, buf_from, copy_nwords, nwords_buf_to);

  // copy FINESSE buffer
  int pos_nwords_finesse[ 4 ];
  for (int j = 0; j < 4; j++) {
    pos_nwords_finesse[ j ] = pos_nwords_to;
    if (GetFINESSENwords(n, j) > 0) {
      // copy body
      buf_from =
        GetFINESSEBuffer(n, j)
        + SIZE_B2LHSLB_HEADER
        + POS_B2L_CTIME;
      copy_nwords =
        GetFINESSENwords(n, j)
        - SIZE_B2LHSLB_HEADER
        - POS_B2L_CTIME
        - SIZE_B2LFEE_TRAILER
        - SIZE_B2LHSLB_TRAILER;

      //      printf("pos %d nwords %d  nwords to %d\n", pos_nwords_to, copy_nwords, nwords_buf_to );
      copyData(buf_to, &pos_nwords_to, buf_from, copy_nwords, nwords_buf_to);


    }
  }

  // copy RawCOPPER trailer
  buf_from =
    GetBuffer(n)
    + GetBlockNwords(n)
    - tmp_trailer.GetTrlNwords();
  copy_nwords = tmp_trailer.GetTrlNwords();
  copyData(buf_to, &pos_nwords_to, buf_from, copy_nwords, nwords_buf_to);

  // length check
  if (pos_nwords_to != nwords_buf_to) {
    printf("Buffer overflow. Exiting... %d %d\n", pos_nwords_to, nwords_buf_to);
    fflush(stdout);
    exit(1);
  }

  //
  // Apply changes followed by data size reduction
  //
  ReducedRawCOPPER reduced_rawcpr;
  *(buf_to + reduced_rawcpr.tmp_header.POS_NWORDS) = nwords_buf_to;
  *(buf_to + reduced_rawcpr.tmp_header.POS_OFFSET_1ST_FINESSE) = pos_nwords_finesse[ 0 ];
  *(buf_to + reduced_rawcpr.tmp_header.POS_OFFSET_2ND_FINESSE) = pos_nwords_finesse[ 1 ];
  *(buf_to + reduced_rawcpr.tmp_header.POS_OFFSET_3RD_FINESSE) = pos_nwords_finesse[ 2 ];
  *(buf_to + reduced_rawcpr.tmp_header.POS_OFFSET_4TH_FINESSE) = pos_nwords_finesse[ 3 ];



//       printf("fROM =======================================\n");
//       for (int k = 0; k < nwords_buf_to; k++) {
//  printf(" %.8x", GetBuffer(n)[ k ]);
//  if ( ( k + 1 ) % 10 == 0) {
//    printf("\n");
//  }
//       }
//       printf("\n");


//       printf("tO   =======================================\n");
//       for (int k = 0; k < nwords_buf_to; k++) {
//  printf(" %.8x", buf_to[ k ]);
//  if ( ( k + 1 ) % 10 == 0) {
//    printf("\n");
//  }
//       }
//       printf("\n");
//       printf("=============================================\n");



  return pos_nwords_to;

}



void PreRawCOPPER::copyData(int* buf_to, int* pos_nwords_to, const int* buf_from,
                            const int copy_nwords, const int nwords_buf_to)
{

  if (*pos_nwords_to + copy_nwords > nwords_buf_to) {
    perror("Buffer overflow. Exiting...");
    exit(1);
  }

  memcpy(buf_to + *pos_nwords_to, buf_from, copy_nwords * sizeof(int));
  *pos_nwords_to += copy_nwords;
  return;
}


#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
void PreRawCOPPER::CheckB2LFEEHeaderVersion(int n)
{
  int flag = 0;
  int* temp_buf;

  for (int i = 0; i < 4; i++) {
    if (GetFINESSENwords(n, i) > 0) {
      temp_buf = GetFINESSEBuffer(n, i);
      if ((temp_buf[ 3 ] & 0x40000000) == 0) {
#ifdef TEMP
        // this word for exp/run
        flag = 1; // old one (ver.1) used for SPring8 test in 2013
        printf("[DEBUG] \033[31m");
        printf("[DEBUG] ===Firmware ver. ERROR===\n ");
        printf("[DEBUG] FTSW and b2tt firmwares was updated on Nov.22, 2013 and the header format attached by B2link was changed in the new firmwares.\n");
        printf("[DEBUG] If you are going to take data now, Please update the firmware.\n");
        printf("[DEBUG] For details, please see Nakao-san's e-mail [b2link_ml:0111] Re: [daq2ml:0159] beta version of trigger timing receiver firmware (b2tt) on bdaq SVN\n");
        printf("[DEBUG] Or if you are going to read data taken before the update, please use basf2 software before rev. 7419 in  https://belle2.cc.kek.jp/svn/trunk/software/daq/\n");
        printf("[DEBUG] About the format please see Nakao-san's B2GM slides(p. 13 and 15) http://kds.kek.jp/getFile.py/access?contribId=143&sessionId=38&resId=0&materialId=slides&confId=13911.\n");
        printf("[DEBUG] Sorry for inconvenience.\n");
        printf("[DEBUG] \033[0m");
        fflush(stderr);
        char err_buf[500];
        sprintf(err_buf, "FTSW and b2tt firmwares are old. Exiting...\n %s %s %d\n",
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        string err_str = err_buf; throw (err_str);
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
      sprintf(err_buf, "PreRawCOPPER contains no FINESSE data. Exiting...\n %s %s %d\n",
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      string err_str = err_buf; throw (err_str);
      sleep(12345678);
      exit(-1);
#endif
    }
  }
  return;
}
#endif


