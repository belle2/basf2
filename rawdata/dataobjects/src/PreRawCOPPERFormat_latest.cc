//+
// File : PreRawCOPPERFormat_latest.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/PreRawCOPPERFormat_latest.h>


using namespace std;
using namespace Belle2;

//#define DESY
//#define NO_DATA_CHECK
//#define WO_FIRST_EVENUM_CHECK

ClassImp(PreRawCOPPERFormat_latest);

PreRawCOPPERFormat_latest::PreRawCOPPERFormat_latest()
{
}

PreRawCOPPERFormat_latest::~PreRawCOPPERFormat_latest()
{
}




int PreRawCOPPERFormat_latest::GetBufferPos(int n)
{
  if (m_buffer == NULL || m_nwords <= 0) {
    char err_buf[500];
    sprintf(err_buf, "[DEBUG] [ERROR] RawPacket buffer(%p) is not available or length(%d) is not set.\n %s %s %d\n",
            m_buffer, m_nwords, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf;     throw (err_str);
  }

  if (n >= (m_num_events * m_num_nodes)) {
    char err_buf[500];
    sprintf(err_buf, "[DEBUG] Invalid COPPER block No. (%d : max %d ) is specified. Exiting... \n %s %s %d\n",
            n, (m_num_events * m_num_nodes), __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf;     throw (err_str);

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
      //      exit(1);
    }
  }
  return pos_nwords;
}



unsigned int PreRawCOPPERFormat_latest::CalcDriverChkSum(int n)
{
  int min = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS;
  int max = GetBufferPos(n) + GetBlockNwords(n)
            - tmp_trailer.RAWTRAILER_NWORDS - SIZE_COPPER_DRIVER_TRAILER;
  unsigned int chksum = 0;
  for (int i = min; i < max; i++) {
    chksum ^= m_buffer[ i ];
  }
  return chksum;
}



int PreRawCOPPERFormat_latest::GetDetectorNwords(int n, int finesse_num)
{

  int nwords = 0;
  if (GetFINESSENwords(n, finesse_num) > 0) {
    nwords = GetFINESSENwords(n, finesse_num)
             - (SIZE_B2LHSLB_HEADER + SIZE_B2LHSLB_TRAILER +  SIZE_B2LFEE_HEADER + SIZE_B2LFEE_TRAILER);
  }
  return nwords;

}


int PreRawCOPPERFormat_latest::GetFINESSENwords(int n, int finesse_num)
{
  if (!CheckCOPPERMagic(n)) {
    char err_buf[500];
    sprintf(err_buf, "COPPER's magic word is invalid. Maybe it is due to data corruption or different version of the data format.\n %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
    perror(err_buf);
    exit(1);
    string err_str = err_buf;    throw (err_str);
    exit(1);
  }
  int pos_nwords;
  switch (finesse_num) {
    case 0 :
      pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + POS_CH_A_DATA_LENGTH;
      break;
    case 1 :
      pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + POS_CH_B_DATA_LENGTH;
      break;
    case 2 :
      pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + POS_CH_C_DATA_LENGTH;
      break;
    case 3 :
      pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + POS_CH_D_DATA_LENGTH;
      break;
    default :
      char err_buf[500];
      sprintf(err_buf, "Specifined FINESSE number( = %d ) is invalid. Exiting...\n %s %s %d\n",
              finesse_num, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      string err_str = err_buf; throw (err_str);
      sleep(12345678);
      exit(-1);
      //   return 0;
      //      break;
  }
  return m_buffer[ pos_nwords ];

}


unsigned int PreRawCOPPERFormat_latest::GetB2LFEE32bitEventNumber(int n)
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
//     sleep(12345678);
//     exit(-1);
  }

  if (err_flag == 1) {

    char err_buf[500];
    sprintf(err_buf, "CORRUPTED DATA: Different event number over HSLBs : slot A 0x%x : B 0x%x :C 0x%x : D 0x%x\n%s %s %d\n",
            eve[ 0 ], eve[ 1 ], eve[ 2 ], eve[ 3 ],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] [ERROR] %s\n", err_buf);
#ifndef NO_DATA_CHECK
    string err_str = err_buf; throw (err_str);
//     sleep(12345678);
//     exit(-1);

#endif //NO_DATA_CHECK
  }
  return eve_num;

#else // READ_OLD_B2LFEE_FORMAT_FILE

  char err_buf[500];
  sprintf(err_buf, "You need comment out READ_OLD_B2LFEE_FORMAT_FILE if you are handling a new data format\n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf; throw (err_str);
//   sleep(12345678);
//   exit(1);
//   return 0;

#endif // READ_OLD_B2LFEE_FORMAT_FILE

}


void PreRawCOPPERFormat_latest::CheckData(int n,
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
    sprintf(err_buf, "CORRUPTED DATA: Event # in PreRawCOPPERFormat_latest header and FEE header is different : cprhdr 0x%x feehdr 0x%x : Exiting...\n%s %s %d\n",
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
  tmp_trailer.SetBuffer(GetRawTrlBufPtr(n));
  unsigned int xor_chksum = CalcXORChecksum(GetBuffer(n), GetBlockNwords(n) - tmp_trailer.GetTrlNwords());
  if (tmp_trailer.GetChksum() != xor_chksum) {
    sprintf(err_buf, "CORRUPTED DATA: PreRawCOPPERFormat_latest checksum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x\n %s %s %d\n",
            n, GetBlockNwords(n), *cur_evenum_rawcprhdr, tmp_trailer.GetChksum(), xor_chksum,
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
//     sleep(1234567);
//     exit(-1);
  }

  return;

}

bool PreRawCOPPERFormat_latest::CheckCOPPERMagic(int n)
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

void PreRawCOPPERFormat_latest::CheckUtimeCtimeTRGType(int n)
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
//     sleep(1234567);
//     exit(1);
  }
  return;
}



double PreRawCOPPERFormat_latest::GetEventUnixTime(int n)
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




unsigned int PreRawCOPPERFormat_latest::FillTopBlockRawHeader(unsigned int m_node_id, unsigned int m_data_type,
    unsigned int m_trunc_mask, unsigned int prev_eve32,
    int prev_runsubrun_no, int* cur_runsubrun_no)
{
  const int datablock_id = 0;
  //  m_temp_value = 12345678;
  //
  // This function only fills RawHeader contents for the first datablock.
  // # of block should be 1
  if (m_num_nodes * m_num_events != 1) {
    char err_buf[500];
    sprintf(err_buf,
            "This function should be used for PreRawCOPPERFormat_latest containing only one datablock, while. this object has num_nodes of %d and num_events of %d\n %s %s %d\n",
            m_num_nodes, m_num_events,  __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf;    throw (err_str);
  }

  //////////////////////////////////////////////////
  //
  // Fill info in RawHeader
  //
  //////////////////////////////////////////////////

  //
  // Initialize a RawHeader part
  //
  memset(m_buffer, 0, sizeof(int) * tmp_header.RAWHEADER_NWORDS);
  m_buffer[ tmp_header.POS_VERSION_HDRNWORDS ] = tmp_header.RAWHEADER_NWORDS & tmp_header.HDR_NWORDS_MASK;
  m_buffer[ tmp_header.POS_VERSION_HDRNWORDS ] |= (tmp_header.DATA_FORMAT_VERSION << tmp_header.FORMAT_VERSION_SHIFT);
  m_buffer[ tmp_header.POS_VERSION_HDRNWORDS ] |= (0x80 << tmp_header.FORMAT_VERSION_SHIFT);   // PreFormat
  m_buffer[ tmp_header.POS_VERSION_HDRNWORDS ] |= tmp_header.MAGIC_WORD;

  //
  // Check FINESSEs which containes data
  //
  int* copper_buf = &(m_buffer[ tmp_header.RAWHEADER_NWORDS ]);
  if (copper_buf[ POS_CH_A_DATA_LENGTH ] == 0 &&
      copper_buf[ POS_CH_B_DATA_LENGTH ] == 0 &&
      copper_buf[ POS_CH_C_DATA_LENGTH ] == 0 &&
      copper_buf[ POS_CH_D_DATA_LENGTH ] == 0) {
    char err_buf[500];
    sprintf(err_buf,
            "No FINESSE data in a copper data block. Exiting...\n %s %s %d\n",
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf; throw (err_str);
//     sleep(12345678);
//     exit(-1);
  }

  //
  //   Set total words info
  //
  int datablock_nwords =
    tmp_header.RAWHEADER_NWORDS +
    (copper_buf[ POS_DATA_LENGTH ]
     + SIZE_COPPER_DRIVER_HEADER
     + SIZE_COPPER_DRIVER_TRAILER)
    + tmp_trailer.RAWTRAILER_NWORDS;
  m_buffer[ tmp_header.POS_NWORDS ] = datablock_nwords;


//   if( m_buffer[ tmp_header.RAWHEADER_NWORDS + SIZE_COPPER_HEADER
//    + SIZE_B2LHSLB_HEADER + POS_TT_UTIME ] !=
//       m_buffer[ datablock_nwords - tmp_trailer.RAWTRAILER_NWORDS
//    - SIZE_COPPER_DRIVER_TRAILER - SIZE_B2LHSLB_TRAILER
//    - (SIZE_B2LFEE_TRAILER - POS_TT_CTIME_B2LFEE ) ] ){
//     char err_buf[500];
//     sprintf(err_buf, "CORRUPTED DATA: You may be using an old b2tt core in FEE firmware. Please read Nakao-san's e-mail(b2link_ml:0143  Belle2link version 0.01 -SVN update ) and use the latest b2tt core. Exiting...\n %s %s %d\n",  __FILE__, __PRETTY_FUNCTION__, __LINE__);
//     sprintf(err_buf, "%s %s %d\n",  __FILE__, __PRETTY_FUNCTION__, __LINE__);
//     string err_str = err_buf; throw (err_str);
//   }


  //
  // Check the consistency between data length and length in RawHeader
  //
  if (m_buffer[ tmp_header.POS_NWORDS ] != m_nwords) {
    char err_buf[500];
    sprintf(err_buf,
            "CORRUPTED DATA: Data length is inconsistent m_nwords %d : nwords from COPPER data %d\n %s %s %d\n",
            m_nwords, m_buffer[ tmp_header.POS_NWORDS ],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf; throw (err_str);
//     sleep(12345678);
//     exit(-1);
  }

  //
  // Fill offset values
  //
  int offset_1st_finesse =  tmp_header.RAWHEADER_NWORDS + SIZE_COPPER_HEADER;
  int offset_2nd_finesse = offset_1st_finesse + copper_buf[ POS_CH_A_DATA_LENGTH ];
  int offset_3rd_finesse = offset_2nd_finesse + copper_buf[ POS_CH_B_DATA_LENGTH ];
  int offset_4th_finesse = offset_3rd_finesse + copper_buf[ POS_CH_C_DATA_LENGTH ];
  m_buffer[ tmp_header.POS_OFFSET_1ST_FINESSE ] = offset_1st_finesse;
  m_buffer[ tmp_header.POS_OFFSET_2ND_FINESSE ] = offset_2nd_finesse;
  m_buffer[ tmp_header.POS_OFFSET_3RD_FINESSE ] = offset_3rd_finesse;
  m_buffer[ tmp_header.POS_OFFSET_4TH_FINESSE ] = offset_4th_finesse;

  //
  // Fill Exp/Run value
  //
  int* finesse_buf = &(m_buffer[ offset_1st_finesse ]); // In any finesse implementations, the top finesse buffer should be at offset_1st_finesse;
  m_buffer[ tmp_header.POS_EXP_RUN_NO ] = finesse_buf[ SIZE_B2LHSLB_HEADER + POS_EXP_RUN ];


  //
  // Fill event #
  //
  unsigned int cur_ftsw_eve32 =  finesse_buf[ SIZE_B2LHSLB_HEADER + POS_TT_TAG ];
  m_buffer[ tmp_header.POS_EVE_NO ] = cur_ftsw_eve32;

  //
  // Copy FTSW words from B2LFEE header
  //
  m_buffer[ tmp_header.POS_TTCTIME_TRGTYPE ] = finesse_buf[ SIZE_B2LHSLB_HEADER + POS_TT_CTIME_TYPE ];
  m_buffer[ tmp_header.POS_TTUTIME ] = finesse_buf[ SIZE_B2LHSLB_HEADER + POS_TT_UTIME ];

  //
  // Set node ID, trunc_mask, data_type
  //
  m_buffer[ tmp_header.POS_SUBSYS_ID ] = m_node_id;
  m_buffer[ tmp_header.POS_TRUNC_MASK_DATATYPE ] = ((m_trunc_mask << 31) & 0x80000000) | (m_data_type & 0x7FFFFFFF);


  //////////////////////////////////////////////////
  //
  // Fill info in RawTrailer
  //
  //////////////////////////////////////////////////

  //
  // Calculate XOR checksum
  //
  unsigned int chksum_top = 0, chksum_body = 0, chksum_bottom = 0;

  int top_end = tmp_header.RAWHEADER_NWORDS;
  for (int i = 0; i < top_end; i++) {
    chksum_top ^= m_buffer[ i ];
  }
  int body_end = datablock_nwords - SIZE_COPPER_DRIVER_TRAILER - tmp_trailer.RAWTRAILER_NWORDS;
  for (int i = top_end; i < body_end; i++) {
    chksum_body ^= m_buffer[ i ];
  }

  int bottom_end = datablock_nwords - tmp_trailer.RAWTRAILER_NWORDS;
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
//     sleep(12345678);
//     exit(-1);
  }

  //
  // Fill trailer info (checksum, magic word)
  //
  unsigned int chksum = chksum_top ^ chksum_body ^ chksum_bottom;
  int* trl = &(m_buffer[ datablock_nwords - tmp_trailer.RAWTRAILER_NWORDS ]);
  trl[ tmp_trailer.POS_CHKSUM ] = chksum;
  trl[ tmp_trailer.POS_TERM_WORD ] = tmp_trailer.MAGIC_WORD_TERM_TRAILER;


  //////////////////////////////////////////////////
  //
  // Data check ( magic word, event incrementation )
  //
  //////////////////////////////////////////////////


  //
  // check magic words
  //
  int* fpga_trailer_magic = trl - (SIZE_COPPER_TRAILER - POS_MAGIC_COPPER_3);
  int* driver_trailer_magic = trl - (SIZE_COPPER_TRAILER - POS_MAGIC_COPPER_4);
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
            GetMagicDriverHeader(datablock_id),
            GetMagicFPGAHeader(datablock_id),
            GetMagicFPGATrailer(datablock_id),
            GetMagicDriverTrailer(datablock_id),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] [ERROR] %s\n", err_buf);
#ifndef NO_DATA_CHECK
    string err_str = err_buf; throw (err_str);

//     sleep(12345678);
//     exit(-1);
#endif
  }

  //
  // check incrementation of event #
  //

  if ((unsigned int)(prev_eve32 + 1) < 50) {
    printf("#################EVE cur %.8x prev %.8x\n", cur_ftsw_eve32, prev_eve32);
    fflush(stdout);
  }


  *cur_runsubrun_no = GetRunNoSubRunNo(datablock_id);
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
      PrintData(GetBuffer(datablock_id), TotalBufNwords());
      throw (err_str);
      //      exit(-1);
#endif

    }
  }

  return cur_ftsw_eve32;

}



#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
void PreRawCOPPERFormat_latest::CheckB2LFEEHeaderVersion(int n)
{
  int* temp_buf;
  for (int i = 0; i < 4; i++) {
    if (GetFINESSENwords(n, i) > 0) {
      temp_buf = GetFINESSEBuffer(n, i);
      if ((temp_buf[ 3 ] & 0x40000000) == 0) {
#ifdef TEMP
        // this word for exp/run
        // old one (ver.1) used for SPring8 test in 2013
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
//         sleep(12345678);
//         exit(-1);
#endif
      } else {
        // this word for 32bit unixtime
        // new one (ver.2)
        break;
      }
    }

    if (i == 3) {
#ifdef TEMP
      char err_buf[500];
      sprintf(err_buf, "PreRawCOPPERFormat_latest contains no FINESSE data. Exiting...\n %s %s %d\n",
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      string err_str = err_buf; throw (err_str);
//       sleep(12345678);
//      exit(-1);
#endif
    }
  }
  return;
}
#endif


int PreRawCOPPERFormat_latest::CalcReducedDataSize(RawDataBlock* raw_datablk)
{
  //
  // Calculate reduced length for a total RawDataBlock (containing multiple data blocks)
  //


  int reduced_nwords = 0;
  for (int k = 0; k < raw_datablk->GetNumEvents(); k++) {
    int num_nodes_in_sendblock = raw_datablk->GetNumNodes();
    for (int l = 0; l < num_nodes_in_sendblock; l++) {
      int entry_id = l + k * num_nodes_in_sendblock;
      if (raw_datablk->CheckFTSWID(entry_id) || raw_datablk->CheckTLUID(entry_id)) {
        // No size reduction for non-COPPER data ( FTSW and TLU data blocks )
        reduced_nwords += raw_datablk->GetBlockNwords(entry_id);
      } else {
        PreRawCOPPERFormat_latest temp_prerawcpr;
        int temp_malloc_flag = 0, temp_num_eve = 1, temp_num_nodes = 1;

        // Call CalcReducedNwords
        temp_prerawcpr.SetBuffer(raw_datablk->GetBuffer(entry_id),
                                 raw_datablk->GetBlockNwords(entry_id),
                                 temp_malloc_flag, temp_num_eve,
                                 temp_num_nodes);
        reduced_nwords += temp_prerawcpr.CalcReducedNwords(0);
      }
    }
  }
  return reduced_nwords;

}

void PreRawCOPPERFormat_latest::CopyReducedData(RawDataBlock* raw_datablk, int* buf_to,
                                                int malloc_flag_from)
{
  //
  // Make a reduced buffer a total RawDataBlock (containing multiple data blocks)
  //
  int pos_nwords_to = 0;
  for (int k = 0; k < raw_datablk->GetNumEvents(); k++) {
    int num_nodes_in_sendblock = raw_datablk->GetNumNodes();
    for (int l = 0; l < num_nodes_in_sendblock; l++) {
      int entry_id = l + k * num_nodes_in_sendblock;
      if (raw_datablk->CheckFTSWID(entry_id) ||
          raw_datablk->CheckTLUID(entry_id)) {
        raw_datablk->CopyBlock(entry_id, buf_to + pos_nwords_to);
        pos_nwords_to += raw_datablk->GetBlockNwords(entry_id);

      } else {
        SetBuffer(raw_datablk->GetBuffer(entry_id),
                  raw_datablk->GetBlockNwords(entry_id), 0, 1, 1);
        pos_nwords_to += CopyReducedBuffer(0, buf_to + pos_nwords_to);

      }
    }
  }

  int* buf_from = raw_datablk->GetWholeBuffer();
  raw_datablk->SetBuffer(buf_to, pos_nwords_to, 0,
                         raw_datablk->GetNumEvents(), raw_datablk->GetNumNodes());

  if (malloc_flag_from == 1) { delete buf_from;}
  return ;
}


int PreRawCOPPERFormat_latest::CalcReducedNwords(int n)
{
  //
  // Calculate reduced length for one data block which is a part of a RawDataBlock
  //

  int nwords_to = 0;

  //RawCOPPER header
  nwords_to += tmp_header.RAWHEADER_NWORDS;

  for (int j = 0; j < 4; j++) {

    int finesse_nwords = GetFINESSENwords(n, j);
    if (finesse_nwords > 0) {
      //
      // B2L(HSLB/FEE) header and trailers are resized
      // m_reduced_rawcpr should be PostRawCOPPERFormat_latest
      //
      nwords_to +=
        finesse_nwords
        - (SIZE_B2LHSLB_HEADER -  m_reduced_rawcpr.SIZE_B2LHSLB_HEADER)
        - (SIZE_B2LFEE_HEADER -   m_reduced_rawcpr.SIZE_B2LFEE_HEADER)
        - (SIZE_B2LFEE_TRAILER -  m_reduced_rawcpr.SIZE_B2LFEE_TRAILER)
        - (SIZE_B2LHSLB_TRAILER - m_reduced_rawcpr.SIZE_B2LHSLB_TRAILER);
    }

  }

  //RawCOPPER Trailer
  nwords_to += tmp_trailer.GetTrlNwords();

  return nwords_to;
}






int PreRawCOPPERFormat_latest::CopyReducedBuffer(int n, int* buf_to)
{
  //
  // Make a reduced buffer for one data block which is a part of a RawDataBlock
  //

  int* buf_from = NULL;
  int nwords_buf_to = CalcReducedNwords(n);
  int pos_nwords_to = 0;
  int copy_nwords = 0;

  // copyt to ReducedRawCOPPER
  //  ReducedRawCOPPER m_reduced_rawcpr;
  //Header copy
  copy_nwords = tmp_header.RAWHEADER_NWORDS;
  buf_from = GetBuffer(n);
  copyData(buf_to, &pos_nwords_to, buf_from, copy_nwords, nwords_buf_to);

  // Unset the PreFormat bit ( 15th bit )
  buf_to[ tmp_header.POS_VERSION_HDRNWORDS ] &= 0xFFFF7FFF;

  //Check Header
  //  m_reduced_rawcpr.tmp_header.CheckHeader(buf_to + pos_nwords_to - copy_nwords);


  // copy FINESSE buffer
  int pos_nwords_finesse[ 4 ];
  for (int j = 0; j < 4; j++) {
    pos_nwords_finesse[ j ] = pos_nwords_to;
    if (GetFINESSENwords(n, j) > 0) {
      int* finesse_buf = GetFINESSEBuffer(n, j);
      int finesse_nwords = GetFINESSENwords(n, j);
      CheckB2LHSLBMagicWords(finesse_buf, finesse_nwords);

      // copy the whole B2LHSLB header (1word)
      buf_to[ pos_nwords_to ] = finesse_buf[ POS_MAGIC_B2LHSLB ];
      pos_nwords_to++;

      // copy the last word of B2LFEE and body( DetectorBuffer )
      buf_from =
        finesse_buf
        + SIZE_B2LHSLB_HEADER
        + POS_B2L_CTIME; //the last word of B2LFEE
      copy_nwords =
        finesse_nwords
        - SIZE_B2LHSLB_HEADER // already copied
        - POS_B2L_CTIME // only one word copied ( SIZE_B2LFEE_HEADER - 1 = POS_B2L_CTIME )
        - SIZE_B2LFEE_TRAILER // will be copied later
        - SIZE_B2LHSLB_TRAILER; // Not copied

      //      printf("pos %d nwords %d  nwords to %d\n", pos_nwords_to, copy_nwords, nwords_buf_to );
      copyData(buf_to, &pos_nwords_to, buf_from, copy_nwords, nwords_buf_to);

      //copy B2LFEE trailer(CRC info)
      buf_to[ pos_nwords_to ] =
        finesse_buf[ finesse_nwords - SIZE_B2LHSLB_TRAILER
                     - (SIZE_B2LFEE_TRAILER - POS_CHKSUM_B2LFEE) ];
      pos_nwords_to++;

      // check CRC data
      //      CheckCRC16( n, j );
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
  *(buf_to + m_reduced_rawcpr.tmp_header.POS_NWORDS) = nwords_buf_to;
  *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_1ST_FINESSE) = pos_nwords_finesse[ 0 ];
  *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_2ND_FINESSE) = pos_nwords_finesse[ 1 ];
  *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_3RD_FINESSE) = pos_nwords_finesse[ 2 ];
  *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_4TH_FINESSE) = pos_nwords_finesse[ 3 ];


  //
  // CRC16 check
  //
  m_reduced_rawcpr.SetBuffer(buf_to, nwords_buf_to, 0, GetNumEvents(), GetNumNodes());
  if (m_reduced_rawcpr.GetNumEvents() * m_reduced_rawcpr.GetNumNodes() <= 0) {
    printf("Invalid data block numbers.(# of events %d, # of nodes %d) Exiting...\n",
           m_reduced_rawcpr.GetNumEvents(), m_reduced_rawcpr.GetNumNodes());
    fflush(stdout);
    exit(1);
  }

  for (int i = 0; i < m_reduced_rawcpr.GetNumEvents() * m_reduced_rawcpr.GetNumNodes(); i++) {
    int nonzero_finesse_buf = 0;
    for (int j = 0; j < 4; j++) {
      pos_nwords_finesse[ j ] = pos_nwords_to;
      if (GetFINESSENwords(n, j) > 0) {
        m_reduced_rawcpr.CheckCRC16(i, j);
        nonzero_finesse_buf++;
      }
    }
    if (nonzero_finesse_buf == 0) {
      printf("No non-zero FINESSE buffer. Exiting...\n");
      fflush(stdout);
      exit(1);
    }
  }

  //    post_rawcopper_latest.CheckCRC16(0, 0);
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


int PreRawCOPPERFormat_latest::CheckB2LHSLBMagicWords(int* finesse_buf, int finesse_nwords)
{
  if ((finesse_buf[ POS_MAGIC_B2LHSLB ] & 0xFFFF0000) == B2LHSLB_HEADER_MAGIC &&
      ((finesse_buf[ finesse_nwords - SIZE_B2LHSLB_TRAILER + POS_CHKSUM_B2LHSLB ] & 0xFFFF0000)
       == B2LHSLB_TRAILER_MAGIC)) {
    return 1;
  } else {
    printf("Invalid B2LHSLB magic words 0x%x 0x%x. Exiting... :%s %s %d\n",
           finesse_buf[ POS_MAGIC_B2LHSLB ],
           finesse_buf[ finesse_nwords - SIZE_B2LHSLB_TRAILER + POS_CHKSUM_B2LHSLB ],
           __FILE__, __PRETTY_FUNCTION__, __LINE__);
    fflush(stdout);
    exit(-1);
    return -1;
  }
}



int PreRawCOPPERFormat_latest::CheckCRC16(int n, int finesse_num)
{
  //
  // Calculate CRC16
  //
  int* buf = GetFINESSEBuffer(n, finesse_num) +  SIZE_B2LHSLB_HEADER;
  int nwords = GetFINESSENwords(n, finesse_num) - (SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_TRAILER + SIZE_B2LHSLB_TRAILER);
  unsigned short temp_crc16 = CalcCRC16LittleEndian(0xffff, buf, nwords);

  //
  // Compare CRC16 with B2LCRC16
  //
  buf = GetFINESSEBuffer(n, finesse_num) +  GetFINESSENwords(n, finesse_num)
        - ((SIZE_B2LFEE_TRAILER - POS_CHKSUM_B2LFEE) + SIZE_B2LHSLB_TRAILER) ;

  //  printf("PreRawCOPPER  : Eve %.8x B2LCRC16 %.8x calculated CRC16 %.8x\n", GetEveNo(n), *buf, temp_crc16 );

  if ((unsigned short)(*buf & 0xFFFF) != temp_crc16) {
    printf("PRE CRC16 error : B2LCRC16 %x Calculated CRC16 %x : Nwords of FINESSE buf %d\n",
           *buf , temp_crc16, GetFINESSENwords(n, finesse_num));
    int* temp_buf = GetFINESSEBuffer(n, finesse_num);
    for (int k = 0; k <  GetFINESSENwords(n, finesse_num); k++) {
      printf("%.8x ", temp_buf[ k ]);
      if ((k + 1) % 10 == 0) {
        printf("\n");
      }
    }
    printf("\n");
    fflush(stdout);

    char err_buf[500];
    sprintf(err_buf, "[DEBUG] [ERROR] B2LCRC16 (%.4x) differs from one ( %.4x) calculated by PreRawCOPPERfromat class. Exiting...\n %s %s %d\n",
            (unsigned short)(*buf & 0xFFFF), temp_crc16,
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf;     throw (err_str);

  }
  return 1;

}
