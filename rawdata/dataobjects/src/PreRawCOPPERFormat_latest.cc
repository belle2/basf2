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

//#define NO_ERROR_STOP
//#define WO_FIRST_EVENUM_CHECK

//ClassImp(PreRawCOPPERFormat_latest);

PreRawCOPPERFormat_latest::PreRawCOPPERFormat_latest()
{
}

PreRawCOPPERFormat_latest::~PreRawCOPPERFormat_latest()
{
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
    PrintData(m_buffer, m_nwords);
    sprintf(err_buf,
            "[FATAL] ERROR_EVENT : COPPER's magic word is invalid. Exiting... Maybe it is due to data corruption or different version of the data format. : slot%c eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
            65 + finesse_num, GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);

    for (int i = 0; i < 4; i++) {
      printf("[DEBUG] ========== CRC check : block # %d finesse %d ==========\n", n, i);
      if (GetFINESSENwords(n, i) > 0) {
        CheckCRC16(n, i);
      }
    }
    printf("[DEBUG] ========== CRC check ended. : block %d =========\n", n);
    string err_str = err_buf;
    throw (err_str);
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
      sprintf(err_buf,
              "[FATAL] ERROR_EVENT : Specifined FINESSE number( = %d ) is invalid. Exiting... : slot%c eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
              finesse_num,
              65 + finesse_num, GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("%s", err_buf); fflush(stdout);
      string err_str = err_buf;
      throw (err_str);

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
    eve[ i ] = 0xbaadf00d;
    if (GetFINESSENwords(n, i) > 0) {
      int pos_nwords = GetOffsetFINESSE(n, i) + SIZE_B2LHSLB_HEADER + POS_TT_TAG;
      eve[ i ] = m_buffer[ pos_nwords ];
      if (flag != 1) eve_num = eve[ i ];
      if (eve_num != eve[ i ]) err_flag = 1;
      flag = 1;
    }
  }

  if (flag == 0) {
    PrintData(m_buffer, m_nwords);
    char err_buf[500];
    sprintf(err_buf, "[FATAL] ERROR_EVENT : No HSLB data in COPPER data. Exiting... : eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    string err_str = err_buf; throw (err_str);
  }

  if (err_flag == 1) {
    PrintData(m_buffer, m_nwords);
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] ERROR_EVENT : CORRUPTED DATA: Different event number over HSLBs : slot A 0x%.8x : B 0x%.8x :C 0x%.8x : D 0x%.8x : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
            eve[ 0 ], eve[ 1 ], eve[ 2 ], eve[ 3 ],
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] %s\n", err_buf);

    for (int i = 0; i < 4; i++) {
      printf("[DEBUG] ========== CRC check : block # %d finesse %d ==========\n", n, i);
      if (GetFINESSENwords(n, i) > 0) {
        CheckCRC16(n, i);
      }
    }
    printf("[DEBUG] ========== CRC error is done. : block %d =========\n", n);
#ifndef NO_ERROR_STOP
    string err_str = err_buf; throw (err_str);
#endif //NO_ERROR_STOP
  }
  return eve_num;

#else // READ_OLD_B2LFEE_FORMAT_FILE

  char err_buf[500];
  sprintf(err_buf, "[FATAL] You need comment out READ_OLD_B2LFEE_FORMAT_FILE if you are handling a new data format\n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s", err_buf); fflush(stdout);
  string err_str = err_buf; throw (err_str);

  //   sleep(12345678);
  //   exit(1);
  //   return 0;

#endif // READ_OLD_B2LFEE_FORMAT_FILE

}


void PreRawCOPPERFormat_latest::CheckData(int n,
                                          unsigned int prev_evenum, unsigned int* cur_evenum_rawcprhdr,
                                          unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                                          unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)
{

  char err_buf[500];
  int err_flag = 0;
  //
  // check Magic words
  //
  if (!CheckCOPPERMagic(n)) {
    sprintf(err_buf,
            "[FATAL] ERROR_EVENT : Invalid Magic word 0x7FFFF0008=%u 0xFFFFFAFA=%u 0xFFFFF5F5=%u 0x7FFF0009=%u : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
            GetMagicDriverHeader(n), GetMagicFPGAHeader(n), GetMagicFPGATrailer(n), GetMagicDriverTrailer(n),
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }

  //
  // Event # check
  //
  *cur_evenum_rawcprhdr = GetEveNo(n);
  unsigned int evenum_feehdr = GetB2LFEE32bitEventNumber(n);
  if (*cur_evenum_rawcprhdr != evenum_feehdr) {
    sprintf(err_buf,
            "[FATAL] ERROR_EVENT : Event # in PreRawCOPPERFormat_latest header and FEE header is different : cprhdr 0x%x feehdr 0x%x : Exiting... : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
            *cur_evenum_rawcprhdr, evenum_feehdr,
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }

  //
  // Check incrementation of event #
  //
  *cur_exprunsubrun_no = GetExpRunSubrun(n);
  *cur_copper_ctr = GetCOPPERCounter(n);
  if (prev_exprunsubrun_no == *cur_exprunsubrun_no) {
    if ((unsigned int)(prev_evenum + 1) != *cur_evenum_rawcprhdr) {
      sprintf(err_buf,
              "[FATAL] ERROR_EVENT : Event # jump : i %d prev 0x%x cur 0x%x : prevrun %.8x currun %.8x: Exiting... : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
              n, prev_evenum, *cur_evenum_rawcprhdr, prev_exprunsubrun_no, *cur_exprunsubrun_no,
              GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      err_flag = 1;
    }
    if ((unsigned int)(prev_copper_ctr + 1) != *cur_copper_ctr) {
      sprintf(err_buf, "[FATAL] ERROR_EVENT : COPPER counter jump : i %d prev 0x%x cur 0x%x : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
              n, prev_copper_ctr, *cur_copper_ctr,
              GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      err_flag = 1;
    }
  } else {
    printf("[DEBUG] New run started. cur run %.8x prev. run %.8x cur eve %.8x prev eve %8.x : eve 0x%x exp %d run %d sub %d\n",
           *cur_exprunsubrun_no, prev_exprunsubrun_no , *cur_evenum_rawcprhdr, prev_evenum,
           GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n));

    // Check if the first event of a run is zero.
    if ((unsigned int)GetRunNo(n) != (prev_exprunsubrun_no & RawHeader_latest::RUNNO_MASK) >> RawHeader_latest::RUNNO_SHIFT) {
      if (*cur_evenum_rawcprhdr != 0) {
        sprintf(err_buf,
                "[FATAL] ERROR_EVENT : Invalid Event # at the beginning of the run (It should be zero.): preveve 0x%x cureve 0x%x : prev(exp %d run %d sub %d ) cur(exp %d run %d sub %d ) Exiting... : eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
                prev_evenum, *cur_evenum_rawcprhdr,
                prev_exprunsubrun_no >> 22 , (prev_exprunsubrun_no >> 8) & 0x3FFF, prev_exprunsubrun_no & 0xFF,
                *cur_exprunsubrun_no >> 22 , (*cur_exprunsubrun_no >> 8) & 0x3FFF, *cur_exprunsubrun_no & 0xFF,
                GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        err_flag = 1;
      }
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
    sprintf(err_buf,
            "[FATAL] ERROR_EVENT : COPPER driver checkSum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
            n, GetBlockNwords(n), *cur_evenum_rawcprhdr, GetDriverChkSum(n), CalcDriverChkSum(n),
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }

  //
  // Check checksum calculated by DeSerializerCOPPER()
  //
  tmp_trailer.SetBuffer(GetRawTrlBufPtr(n));
  unsigned int xor_chksum = CalcXORChecksum(GetBuffer(n), GetBlockNwords(n) - tmp_trailer.GetTrlNwords());
  if (tmp_trailer.GetChksum() != xor_chksum) {
    sprintf(err_buf,
            "[FATAL] ERROR_EVENT : PreRawCOPPERFormat_latest checksum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x : eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
            n, GetBlockNwords(n), *cur_evenum_rawcprhdr, tmp_trailer.GetChksum(), xor_chksum,
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }

  if (err_flag == 1) {
    printf("%s", err_buf); fflush(stdout);
    printf("[DEBUG] ========== dump a data blcok : block # %d==========\n", n);
    PrintData(GetBuffer(n), GetBlockNwords(n));
    for (int i = 0; i < 4; i++) {
      printf("[DEBUG] ========== CRC check : block # %d finesse %d ==========\n", n, i);
      if (GetFINESSENwords(n, i) > 0) {
        CheckCRC16(n, i);
      }
    }
    printf("[DEBUG] ========== CRC check is done. : block %d =========\n", n);
    string err_str = err_buf;
    throw (err_str);
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
  unsigned int temp_utime = 0, temp_ctime_trgtype = 0, temp_eve = 0, temp_exprun = 0;
  unsigned int temp_ctime_trgtype_footer = 0, temp_eve_footer = 0;
  unsigned int utime[4], ctime_trgtype[4], eve[4], exprun[4];
  char err_buf[500];

  memset(utime, 0, sizeof(utime));
  memset(ctime_trgtype, 0, sizeof(ctime_trgtype));
  memset(eve, 0, sizeof(eve));
  memset(exprun, 0, sizeof(exprun));


  for (int i = 0; i < 4; i++) {
    int finesse_nwords = GetFINESSENwords(n, i);
    if (finesse_nwords > 0) {
      int offset_finesse = GetOffsetFINESSE(n, i);
      ctime_trgtype[ i ] = m_buffer[ offset_finesse + SIZE_B2LHSLB_HEADER + POS_TT_CTIME_TYPE ];
      utime[ i ] = m_buffer[ offset_finesse + SIZE_B2LHSLB_HEADER + POS_TT_UTIME ];
      eve[ i ] = m_buffer[ offset_finesse + SIZE_B2LHSLB_HEADER + POS_TT_TAG ];
      exprun[ i ] = m_buffer[ offset_finesse + SIZE_B2LHSLB_HEADER + POS_EXP_RUN ];
      temp_ctime_trgtype_footer =
        m_buffer[ offset_finesse + finesse_nwords - (SIZE_B2LFEE_TRAILER + SIZE_B2LHSLB_TRAILER) + POS_TT_CTIME_B2LFEE ];
      temp_eve_footer =
        m_buffer[ offset_finesse + finesse_nwords - (SIZE_B2LFEE_TRAILER + SIZE_B2LHSLB_TRAILER) + POS_CHKSUM_B2LFEE ];

      if (flag == 0) {
        temp_ctime_trgtype = ctime_trgtype[ i ];
        temp_utime = utime[ i ];
        temp_eve = eve[ i ];
        temp_exprun = exprun[ i ];
        flag = 1;
      } else {
        if (temp_ctime_trgtype != ctime_trgtype[ i ] || temp_utime != utime[ i ] ||
            temp_eve != eve[ i ] || temp_exprun != exprun[ i ]) {
          if (err_flag == 0) {
            for (int j = 0; j < 4; j++) {
              printf("[DEBUG] FINESSE #=%d buffsize %d ctimeTRGtype 0x%.8x utime 0x%.8x eve 0x%.8x exprun 0x%.8x\n",
                     j, GetFINESSENwords(n, j), ctime_trgtype[ j ], utime[ j ], eve[ j ], exprun[ j ]);
            }
          }
          sprintf(err_buf, "[FATAL] ERROR_EVENT : mismatch header value over FINESSEs. Exiting...\n %s %s %d\n",
                  __FILE__, __PRETTY_FUNCTION__, __LINE__);
          printf("%s", err_buf); fflush(stdout);

          err_flag = 1;
          break;
        } else if (temp_ctime_trgtype != temp_ctime_trgtype_footer ||
                   (temp_eve & 0xffff) != ((temp_eve_footer >> 16) & 0xffff)) {
          sprintf(err_buf,
                  "[FATAL] ERROR_EVENT : mismatch(finesse %d) between header(ctime %.8x eve %.8x) and footer(ctime %.8x eve_crc16 %.8x). Exiting...\n %s %s %d\n",
                  i,  temp_ctime_trgtype,  temp_eve, temp_ctime_trgtype_footer, temp_eve_footer,
                  __FILE__, __PRETTY_FUNCTION__, __LINE__);
          printf("%s", err_buf); fflush(stdout);
          err_flag = 1;
        }
      }
    }
  }

  if (err_flag != 0) {
    for (int i = 0; i < 4; i++) {
      if (GetFINESSENwords(n, i) > 0) {
        printf("[DEBUG] ========== CRC check : block # %d finesse %d ==========\n", n, i);
        CheckCRC16(n, i);
        printf("[DEBUG] ========== CRC check is done. : block %d =========\n", n);
      }
    }
#ifndef NO_ERROR_STOP
    string err_str = err_buf; throw (err_str);
#endif
  }
  return;
}

unsigned int PreRawCOPPERFormat_latest::FillTopBlockRawHeader(unsigned int m_node_id, unsigned int prev_eve32,
    unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)
{
  const int datablock_id = 0;
  //  m_temp_value = 12345678;
  //
  // This function only fills RawHeader contents for the first datablock.
  // # of block should be 1
  if (m_num_nodes * m_num_events != 1) {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] This function should be used for PreRawCOPPERFormat_latest containing only one datablock, while. this object has num_nodes of %d and num_events of %d\n %s %s %d\n",
            m_num_nodes, m_num_events,  __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
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
  m_buffer[ tmp_header.POS_VERSION_HDRNWORDS ] |= (DATA_FORMAT_VERSION << tmp_header.FORMAT_VERSION_SHIFT) &
                                                  tmp_header.FORMAT_VERSION__MASK;
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
            "[FATAL] ERROR_EVENT : No FINESSE data in a copper data block. Exiting...\n %s %s %d\n",
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
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


  //
  // Check the consistency between data length and length in RawHeader
  //
  if (m_buffer[ tmp_header.POS_NWORDS ] != m_nwords) {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] ERROR_EVENT : Data length is inconsistent m_nwords %d : nwords from COPPER data %d\n %s %s %d\n",
            m_nwords, m_buffer[ tmp_header.POS_NWORDS ],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
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
  int* finesse_buf = &
                     (m_buffer[ offset_1st_finesse ]); // In any finesse implementations, the top finesse buffer should be at offset_1st_finesse;
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
  m_buffer[ tmp_header.POS_NODE_ID ] = m_node_id;

  //
  // Check error counts of b2link-packet CRC
  //
  //  m_buffer[ tmp_header.POS_TRUNC_MASK_DATATYPE ] = ((m_trunc_mask << 31) & 0x80000000) | (m_data_type & 0x7FFFFFFF);
  //   printf("%.8x %.8x %.8x %.8x : %.8x\n",
  //   copper_buf[ POS_CH_A_DATA_LENGTH ], copper_buf[ POS_CH_B_DATA_LENGTH ],
  //   copper_buf[ POS_CH_C_DATA_LENGTH ], copper_buf[ POS_CH_D_DATA_LENGTH ], m_buffer[ offset_3rd_finesse + copper_buf[ POS_CH_C_DATA_LENGTH ] - SIZE_B2LHSLB_HEADER ] );

  m_buffer[ tmp_header.POS_TRUNC_MASK_DATATYPE ] = 0;
  if (copper_buf[ POS_CH_A_DATA_LENGTH ] != 0) {

    if ((m_buffer[ offset_1st_finesse + copper_buf[ POS_CH_A_DATA_LENGTH ] - SIZE_B2LHSLB_HEADER ] & 0xFFFF) != 0) {
      m_buffer[ tmp_header.POS_TRUNC_MASK_DATATYPE ] |= tmp_header.B2LINK_PACKET_CRC_ERROR;
      printf("[ERROR] B2link packet CRC error slot A eve %8u foooter %.8x : %s %s %d\n", cur_ftsw_eve32,
             m_buffer[ offset_1st_finesse + copper_buf[ POS_CH_A_DATA_LENGTH ] - SIZE_B2LHSLB_HEADER ],
             __FILE__, __PRETTY_FUNCTION__, __LINE__);
      PrintData(m_buffer, m_nwords);
    }
  }
  if (copper_buf[ POS_CH_B_DATA_LENGTH ] != 0) {
    if ((m_buffer[ offset_2nd_finesse + copper_buf[ POS_CH_B_DATA_LENGTH ] - SIZE_B2LHSLB_HEADER ] & 0xFFFF) != 0) {
      m_buffer[ tmp_header.POS_TRUNC_MASK_DATATYPE ] |= tmp_header.B2LINK_PACKET_CRC_ERROR;
      printf("[ERROR] B2link packet CRC error slot B eve %8u foooter %.8x : %s %s %d\n",  cur_ftsw_eve32,
             m_buffer[ offset_1st_finesse + copper_buf[ POS_CH_B_DATA_LENGTH ] - SIZE_B2LHSLB_HEADER ],
             __FILE__, __PRETTY_FUNCTION__, __LINE__);
      PrintData(m_buffer, m_nwords);
    }
  }
  if (copper_buf[ POS_CH_C_DATA_LENGTH ] != 0) {
    if ((m_buffer[ offset_3rd_finesse + copper_buf[ POS_CH_C_DATA_LENGTH ] - SIZE_B2LHSLB_HEADER ] & 0xFFFF) != 0) {
      m_buffer[ tmp_header.POS_TRUNC_MASK_DATATYPE ] |= tmp_header.B2LINK_PACKET_CRC_ERROR;
      printf("[ERROR] B2link packet CRC error slot C eve %8u foooter %.8x : %s %s %d\n",  cur_ftsw_eve32,
             m_buffer[ offset_1st_finesse + copper_buf[ POS_CH_C_DATA_LENGTH ] - SIZE_B2LHSLB_HEADER ],
             __FILE__, __PRETTY_FUNCTION__, __LINE__);
      PrintData(m_buffer, m_nwords);
    }
  }
  if (copper_buf[ POS_CH_D_DATA_LENGTH ] != 0) {
    if ((m_buffer[ offset_4th_finesse + copper_buf[ POS_CH_D_DATA_LENGTH ] - SIZE_B2LHSLB_HEADER ] & 0xFFFF) != 0) {
      m_buffer[ tmp_header.POS_TRUNC_MASK_DATATYPE ] |= tmp_header.B2LINK_PACKET_CRC_ERROR;
      printf("[ERROR] B2link packet CRC error slot D eve %8u foooter %.8x : %s %s %d\n",  cur_ftsw_eve32,
             m_buffer[ offset_1st_finesse + copper_buf[ POS_CH_D_DATA_LENGTH ] - SIZE_B2LHSLB_HEADER ],
             __FILE__, __PRETTY_FUNCTION__, __LINE__);
      PrintData(m_buffer, m_nwords);
    }
  }


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
    sprintf(err_buf, "[FATAL] ERROR_EVENT : COPPER driver checksum is not consistent.: calcd. %.8x data %.8x\n %s %s %d\n",
            chksum_body, m_buffer[ body_end ],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);

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
    sprintf(err_buf, "[FATAL] ERROR_EVENT : Invalid Magic word 0x7FFFF0008=%u 0xFFFFFAFA=%u 0xFFFFF5F5=%u 0x7FFF0009=%u\n %s %s %d\n",
            GetMagicDriverHeader(datablock_id),
            GetMagicFPGAHeader(datablock_id),
            GetMagicFPGATrailer(datablock_id),
            GetMagicDriverTrailer(datablock_id),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] %s\n", err_buf);

#ifndef NO_ERROR_STOP
    string err_str = err_buf; throw (err_str);
#endif

  }

  //
  // check incrementation of event #
  //


  *cur_exprunsubrun_no = GetExpRunSubrun(datablock_id);

//   if ((unsigned int)cur_ftsw_eve32 < 50) {
//     printf("EVENT OUTPUT ########## %u\n", cur_ftsw_eve32);
//     PrintData(GetBuffer(datablock_id), TotalBufNwords());
//   }
  if (prev_exprunsubrun_no == *cur_exprunsubrun_no) {
    if (prev_eve32 + 1 != cur_ftsw_eve32) {

#ifndef NO_ERROR_STOP
      char err_buf[500];
      sprintf(err_buf,
              "[FATAL] ERROR_EVENT : Invalid event_number. Exiting...: cur 32bit eve %u preveve %u prun %u crun %u\n %s %s %d\n",
              cur_ftsw_eve32, prev_eve32,
              prev_exprunsubrun_no, *cur_exprunsubrun_no,
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("[DEBUG] %s\n", err_buf);

      string err_str = err_buf;
      printf("[DEBUG] i= %d : num entries %d : Tot words %d\n", 0 , GetNumEntries(), TotalBufNwords());
      PrintData(GetBuffer(datablock_id), TotalBufNwords());

      for (int i = 0; i < 4; i++) {
        printf("[DEBUG] ========== CRC check : block # %d finesse %d ==========\n", datablock_id, i);
        if (GetFINESSENwords(datablock_id, i) > 0) {
          CheckCRC16(datablock_id, i);
        }
      }
      printf("[DEBUG] ========== CRC check is O.K. : block %d =========\n", datablock_id);
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
        printf("[DEBUG] Or if you are going to read data taken before the update, please use basf2 software before svn revision 7419\n");
        printf("[DEBUG] About the format please see Nakao-san's B2GM slides(p. 13 and 15) http://kds.kek.jp/getFile.py/access?contribId=143&sessionId=38&resId=0&materialId=slides&confId=13911.\n");
        printf("[DEBUG] Sorry for inconvenience.\n");
        printf("[DEBUG] \033[0m");
        fflush(stderr);
        char err_buf[500];
        sprintf(err_buf, "[FATAL] ERROR_EVENT : FTSW and b2tt firmwares are old. Exiting...\n %s %s %d\n",
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
      sprintf(err_buf, "[FATAL] ERROR_EVENT : PreRawCOPPERFormat_latest contains no FINESSE data. Exiting...\n %s %s %d\n",
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("%s", err_buf); fflush(stdout);
      string err_str = err_buf; throw (err_str);
      //       sleep(12345678);
      //      exit(-1);
#endif
    }
  }
  return;
}
#endif

//int PreRawCOPPERFormat_latest::CalcReducedDataSize(RawDataBlock* raw_datablk)
int PreRawCOPPERFormat_latest::CalcReducedDataSize(int* bufin, int nwords, int num_events, int num_nodes)
{
  //
  // Calculate reduced length for a total RawDataBlock (containing multiple data blocks)
  //
  RawDataBlockFormat radblk_fmt;
  int delete_flag = 0;
  radblk_fmt.SetBuffer(bufin, nwords, delete_flag, num_events, num_nodes);

  int reduced_nwords = 0;
  for (int k = 0; k < radblk_fmt.GetNumEvents(); k++) {
    int num_nodes_in_sendblock = radblk_fmt.GetNumNodes();
    for (int l = 0; l < num_nodes_in_sendblock; l++) {
      int entry_id = l + k * num_nodes_in_sendblock;

      if (radblk_fmt.CheckFTSWID(entry_id) || radblk_fmt.CheckTLUID(entry_id)) {
        reduced_nwords += radblk_fmt.GetBlockNwords(entry_id);
      } else {
        PreRawCOPPERFormat_latest temp_prerawcpr;
        int temp_delete_flag = 0, temp_num_eve = 1, temp_num_nodes = 1;

        // Call CalcReducedNwords
        temp_prerawcpr.SetBuffer(radblk_fmt.GetBuffer(entry_id),
                                 radblk_fmt.GetBlockNwords(entry_id),
                                 temp_delete_flag, temp_num_eve,
                                 temp_num_nodes);
        reduced_nwords += temp_prerawcpr.CalcReducedNwords(0);
      }
    }
  }
  return reduced_nwords;

}

//void PreRawCOPPERFormat_latest::CopyReducedData(RawDataBlock* raw_datablk, int* buf_to, int delete_flag_from)
void PreRawCOPPERFormat_latest::CopyReducedData(int* bufin, int nwords, int num_events, int num_nodes, int* buf_to, int* nwords_to)
{
  //
  // Make a reduced buffer a total RawDataBlock (containing multiple data blocks)
  //
  RawDataBlockFormat radblk_fmt;
  int delete_flag = 0;
  radblk_fmt.SetBuffer(bufin, nwords, delete_flag, num_events, num_nodes);

  int pos_nwords_to = 0;
  for (int k = 0; k < radblk_fmt.GetNumEvents(); k++) {
    int num_nodes_in_sendblock = radblk_fmt.GetNumNodes();
    for (int l = 0; l < num_nodes_in_sendblock; l++) {
      int entry_id = l + k * num_nodes_in_sendblock;
      if (radblk_fmt.CheckFTSWID(entry_id) ||
          radblk_fmt.CheckTLUID(entry_id)) {
        radblk_fmt.CopyBlock(entry_id, buf_to + pos_nwords_to);
        pos_nwords_to += radblk_fmt.GetBlockNwords(entry_id);

      } else {
        SetBuffer(radblk_fmt.GetBuffer(entry_id),
                  radblk_fmt.GetBlockNwords(entry_id), 0, 1, 1);

        pos_nwords_to += CopyReducedBuffer(0, buf_to + pos_nwords_to);


      }
    }
  }

  *nwords_to = pos_nwords_to;

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

  unsigned int removed_xor_chksum = 0;

  // copyt to ReducedRawCOPPER
  //  ReducedRawCOPPER m_reduced_rawcpr;
  //Header copy
  copy_nwords = tmp_header.RAWHEADER_NWORDS;
  buf_from = GetBuffer(n);
  copyData(buf_to, &pos_nwords_to, buf_from, copy_nwords, nwords_buf_to);


  //calcurate XOR checksum diff.( depends on data-format )
  removed_xor_chksum ^= buf_from[ tmp_header.POS_VERSION_HDRNWORDS ];

  // Unset the PreFormat bit ( 15th bit )
  buf_to[ tmp_header.POS_VERSION_HDRNWORDS ] &= 0xFFFF7FFF;

  //calcurate XOR checksum diff.( depends on data-format )
  removed_xor_chksum ^=  buf_to[ tmp_header.POS_VERSION_HDRNWORDS ];
  for (int i = 0; i < SIZE_COPPER_HEADER; i++) {
    removed_xor_chksum ^=  buf_from[ tmp_header.RAWHEADER_NWORDS + i ];
  }

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


      // check finesse buffer size : ( When dumhslb was used, this type of error
      // occured and RecvStream0.py died by Segmentation Fault. ) 2014.12.01.
      if (finesse_nwords - SIZE_B2LHSLB_HEADER - SIZE_B2LFEE_HEADER
          - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER < 0) {
        char err_buf[500];
        sprintf(err_buf,
                "[FATAL] ERROR_EVENT : Finesse buffer size is too small( %d words < %d words). May be the data are corrupted. Exiting...\n %s %s %d\n",
                finesse_nwords, SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER + SIZE_B2LFEE_TRAILER + SIZE_B2LHSLB_TRAILER,
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        printf("%s", err_buf); fflush(stdout);
        string err_str = err_buf;     throw (err_str);
      }

      //calcurate XOR checksum diff.( depends on data-format )
      for (int k = 1; k <= 4 ; k++) {
        removed_xor_chksum ^=  finesse_buf[ k ];
      }

      copy_nwords =
        finesse_nwords
        - SIZE_B2LHSLB_HEADER // already copied
        - POS_B2L_CTIME // only one word copied ( SIZE_B2LFEE_HEADER - 1 = POS_B2L_CTIME )
        - SIZE_B2LFEE_TRAILER // will be copied later
        - SIZE_B2LHSLB_TRAILER; // Not copied
      copyData(buf_to, &pos_nwords_to, buf_from, copy_nwords, nwords_buf_to);
      //      printf("pos %d nwords %d  nwords to %d\n", pos_nwords_to, copy_nwords, nwords_buf_to );

      //copy B2LFEE trailer(CRC info)
      buf_to[ pos_nwords_to ] = 0xffff & finesse_buf[ finesse_nwords - SIZE_B2LHSLB_TRAILER - (SIZE_B2LFEE_TRAILER - POS_CHKSUM_B2LFEE) ];
      //copy B2LHSLB trailer(packet CRC error count)
      buf_to[ pos_nwords_to ] |= (finesse_buf[ finesse_nwords - (SIZE_B2LHSLB_TRAILER - POS_MAGIC_B2LHSLB) ] << 16) & 0xFFFF0000;

      //calcurate XOR checksum diff. ( depends on data-format )
      for (int k = 0; k <= 2 ; k++) {
        removed_xor_chksum ^=  finesse_buf[ finesse_nwords - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER + k ];
      }
      removed_xor_chksum ^=  buf_to[ pos_nwords_to ];

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

  //calcurate XOR checksum diff.( depends on data-format )
  unsigned int old_rawcopper_chksum = buf_from[ tmp_trailer.POS_CHKSUM ];
  for (int i = 0; i < SIZE_COPPER_TRAILER; i++) {
    removed_xor_chksum ^= (unsigned int) * (buf_from - SIZE_COPPER_TRAILER + i);
  }


  // length check
  if (pos_nwords_to != nwords_buf_to) {
    printf("Buffer overflow. Exiting... %d %d\n", pos_nwords_to, nwords_buf_to);
    fflush(stdout);
    exit(1);
  }


  // Recalculate XOR checksum in a RawCOPPER trailer
  removed_xor_chksum ^= *(buf_to + m_reduced_rawcpr.tmp_header.POS_NWORDS);
  removed_xor_chksum ^= *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_1ST_FINESSE);
  removed_xor_chksum ^= *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_2ND_FINESSE);
  removed_xor_chksum ^= *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_3RD_FINESSE);
  removed_xor_chksum ^= *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_4TH_FINESSE);

  //
  // Apply changes followed by data size reduction
  //
  *(buf_to + m_reduced_rawcpr.tmp_header.POS_NWORDS) = nwords_buf_to;
  *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_1ST_FINESSE) = pos_nwords_finesse[ 0 ];
  *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_2ND_FINESSE) = pos_nwords_finesse[ 1 ];
  *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_3RD_FINESSE) = pos_nwords_finesse[ 2 ];
  *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_4TH_FINESSE) = pos_nwords_finesse[ 3 ];

  // Recalculate XOR checksum in a RawCOPPER trailer
  removed_xor_chksum ^= *(buf_to + m_reduced_rawcpr.tmp_header.POS_NWORDS);
  removed_xor_chksum ^= *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_1ST_FINESSE);
  removed_xor_chksum ^= *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_2ND_FINESSE);
  removed_xor_chksum ^= *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_3RD_FINESSE);
  removed_xor_chksum ^= *(buf_to + m_reduced_rawcpr.tmp_header.POS_OFFSET_4TH_FINESSE);

  // Recalculate XOR checksum in a RawCOPPER trailer
  unsigned int new_rawcopper_chksum = CalcXORChecksum(buf_to, pos_nwords_to - tmp_trailer.GetTrlNwords());


  if ((old_rawcopper_chksum ^ removed_xor_chksum) != new_rawcopper_chksum) {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] ERROR_EVENT : RawCOPPER XOR checksum is inconsistent between before/after data reduction.(%.8x != %.8x ^ %.8x = %.8x ) Exiting...\n %s %s %d\n",
            new_rawcopper_chksum,  old_rawcopper_chksum, removed_xor_chksum, old_rawcopper_chksum ^ removed_xor_chksum,
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    string err_str = err_buf;     throw (err_str);
  }

  *(buf_to + pos_nwords_to - tmp_trailer.GetTrlNwords() + tmp_trailer.POS_CHKSUM) = new_rawcopper_chksum;



  //
  // data block # check
  //
  m_reduced_rawcpr.SetBuffer(buf_to, nwords_buf_to, 0, GetNumEvents(), GetNumNodes());
  if (m_reduced_rawcpr.GetNumEvents() * m_reduced_rawcpr.GetNumNodes() <= 0) {
    printf("Invalid data block numbers.(# of events %d, # of nodes %d) Exiting...\n",
           m_reduced_rawcpr.GetNumEvents(), m_reduced_rawcpr.GetNumNodes());
    fflush(stdout);
    exit(1);
  }

  //
  // check no-data buffer
  //
  for (int i = 0; i < m_reduced_rawcpr.GetNumEvents() * m_reduced_rawcpr.GetNumNodes(); i++) {
    int nonzero_finesse_buf = 0;
    for (int j = 0; j < 4; j++) {

      if (GetFINESSENwords(n, j) > 0) {
        //        m_reduced_rawcpr.CheckCRC16(i, j);
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
    PrintData(m_buffer, m_nwords);

    printf("Invalid B2LHSLB magic words : header 0x%x (= should be ffaa**** ) or trailer 0x%x (= should be ff55**** ). Exiting... :%s %s %d\n",
           finesse_buf[ POS_MAGIC_B2LHSLB ],
           finesse_buf[ finesse_nwords - SIZE_B2LHSLB_TRAILER + POS_CHKSUM_B2LHSLB ],
           __FILE__, __PRETTY_FUNCTION__, __LINE__);
#ifndef NO_ERROR_STOP
    fflush(stdout);
    exit(-1);
    return -1;
#endif
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

  if ((unsigned short)(*buf & 0xFFFF) != temp_crc16) {
    PrintData(GetBuffer(n), *(GetBuffer(n) + tmp_header.POS_NWORDS));
    printf("[FATAL] ERROR_EVENT : PRE CRC16 error : B2LCRC16 %x Calculated CRC16 %x : Nwords of FINESSE buf %d\n",
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
    sprintf(err_buf,
            "[FATAL] ERROR_EVENT : B2LCRC16 (%.4x) differs from one ( %.4x) calculated by PreRawCOPPERfromat class. Exiting...\n %s %s %d\n",
            (unsigned short)(*buf & 0xFFFF), temp_crc16,
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    string err_str = err_buf;     throw (err_str);

  }
  return 1;

}

int* PreRawCOPPERFormat_latest::PackDetectorBuf(int* packed_buf_nwords,
                                                int* detector_buf_1st, int nwords_1st,
                                                int* detector_buf_2nd, int nwords_2nd,
                                                int* detector_buf_3rd, int nwords_3rd,
                                                int* detector_buf_4th, int nwords_4th,
                                                RawCOPPERPackerInfo rawcpr_info)
{
  int* packed_buf = NULL;

  int poswords_to = 0;
  int* detector_buf[ 4 ] = { detector_buf_1st, detector_buf_2nd, detector_buf_3rd, detector_buf_4th };
  int nwords[ 4 ] = { nwords_1st, nwords_2nd, nwords_3rd, nwords_4th };

  // calculate the event length
  int length_nwords = tmp_header.GetHdrNwords() + SIZE_COPPER_HEADER + SIZE_COPPER_TRAILER + tmp_trailer.GetTrlNwords();

  for (int i = 0; i < 4; i++) {
    if (detector_buf[ i ] == NULL || nwords[ i ] <= 0) continue;    // for an empty FINESSE slot
    length_nwords += nwords[ i ];
    length_nwords += SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER
                     + SIZE_B2LFEE_TRAILER + SIZE_B2LHSLB_TRAILER;
  }

  // allocate buffer
  packed_buf = new int[ length_nwords ];
  memset(packed_buf, 0, sizeof(int) * length_nwords);

  //
  // Fill RawHeader
  //
  tmp_header.SetBuffer(packed_buf);

  packed_buf[ tmp_header.POS_NWORDS ] = length_nwords; // total length
  packed_buf[ tmp_header.POS_VERSION_HDRNWORDS ] = 0x7f7f8000
                                                   | ((DATA_FORMAT_VERSION << tmp_header.FORMAT_VERSION_SHIFT) & tmp_header.FORMAT_VERSION__MASK)
                                                   | tmp_header.RAWHEADER_NWORDS; // ver.#, header length
  packed_buf[ tmp_header.POS_EXP_RUN_NO ] = (rawcpr_info.exp_num << tmp_header.EXP_SHIFT)
                                            | (rawcpr_info.run_subrun_num & 0x003FFFFF);   // exp. and run #
  packed_buf[ tmp_header.POS_EVE_NO ] = rawcpr_info.eve_num; // eve #
  packed_buf[ tmp_header.POS_TTCTIME_TRGTYPE ] = (rawcpr_info.tt_ctime & 0x7FFFFFF) << 4;   // tt_ctime
  packed_buf[ tmp_header.POS_TTUTIME ] = rawcpr_info.tt_utime; // tt_utime
  packed_buf[ tmp_header.POS_NODE_ID ] = rawcpr_info.node_id; // node ID




  // fill the positions of finesse buffers
  packed_buf[ tmp_header.POS_OFFSET_1ST_FINESSE ] = tmp_header.RAWHEADER_NWORDS + SIZE_COPPER_HEADER;

  packed_buf[ tmp_header.POS_OFFSET_2ND_FINESSE ] = packed_buf[ tmp_header.POS_OFFSET_1ST_FINESSE ];
  if (nwords[ 0 ] > 0) {
    packed_buf[ tmp_header.POS_OFFSET_2ND_FINESSE ] +=
      nwords[ 0 ] + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER  + SIZE_B2LFEE_TRAILER + SIZE_B2LHSLB_TRAILER;
  }

  packed_buf[ tmp_header.POS_OFFSET_3RD_FINESSE ] = packed_buf[ tmp_header.POS_OFFSET_2ND_FINESSE ];
  if (nwords[ 1 ] > 0) {
    packed_buf[ tmp_header.POS_OFFSET_3RD_FINESSE ] +=
      nwords[ 1 ] + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER  + SIZE_B2LFEE_TRAILER + SIZE_B2LHSLB_TRAILER;
  }

  packed_buf[ tmp_header.POS_OFFSET_4TH_FINESSE ] = packed_buf[ tmp_header.POS_OFFSET_3RD_FINESSE ];
  if (nwords[ 2 ] > 0) {
    packed_buf[ tmp_header.POS_OFFSET_4TH_FINESSE ] += nwords[ 2 ] + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER  + SIZE_B2LFEE_TRAILER +
                                                       SIZE_B2LHSLB_TRAILER;
  }
  poswords_to += tmp_header.GetHdrNwords();

  // Fill COPPER header
  packed_buf[ poswords_to + POS_MAGIC_COPPER_1 ] = COPPER_MAGIC_DRIVER_HEADER;
  packed_buf[ poswords_to + POS_MAGIC_COPPER_2 ] = COPPER_MAGIC_FPGA_HEADER;
  packed_buf[ poswords_to + POS_EVE_NUM_COPPER ] = rawcpr_info.eve_num;

  int size_b2l_hdrtrl = SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER + SIZE_B2LFEE_TRAILER + SIZE_B2LHSLB_TRAILER;
  if (nwords[ 0 ] != 0)  packed_buf[ poswords_to + POS_CH_A_DATA_LENGTH ] = nwords[ 0 ] + size_b2l_hdrtrl;
  if (nwords[ 1 ] != 0)  packed_buf[ poswords_to + POS_CH_B_DATA_LENGTH ] = nwords[ 1 ] + size_b2l_hdrtrl;
  if (nwords[ 2 ] != 0)  packed_buf[ poswords_to + POS_CH_C_DATA_LENGTH ] = nwords[ 2 ] + size_b2l_hdrtrl;
  if (nwords[ 3 ] != 0)  packed_buf[ poswords_to + POS_CH_D_DATA_LENGTH ] = nwords[ 3 ] + size_b2l_hdrtrl;

  packed_buf[ poswords_to + POS_DATA_LENGTH ] =
    packed_buf[ poswords_to + POS_CH_A_DATA_LENGTH ] +
    packed_buf[ poswords_to + POS_CH_B_DATA_LENGTH ] +
    packed_buf[ poswords_to + POS_CH_C_DATA_LENGTH ] +
    packed_buf[ poswords_to + POS_CH_D_DATA_LENGTH ] +
    (SIZE_COPPER_HEADER - SIZE_COPPER_DRIVER_HEADER) +
    (SIZE_COPPER_TRAILER - SIZE_COPPER_DRIVER_TRAILER);

  poswords_to += SIZE_COPPER_HEADER;

  // Fill FINESSE buffer
  for (int i = 0; i < 4; i++) {

    if (detector_buf[ i ] == NULL || nwords[ i ] <= 0) continue;     // for an empty FINESSE slot

    // Fill b2link HSLB header
    packed_buf[ poswords_to + POS_MAGIC_B2LHSLB ] = 0xffaa0000 | (0xffff & rawcpr_info.eve_num);
    poswords_to += SIZE_B2LHSLB_HEADER;
    int* crc16_start = &(packed_buf[ poswords_to ]);

    // Fill b2link FEE header

    packed_buf[ poswords_to + POS_TT_CTIME_TYPE ] = (rawcpr_info.tt_ctime & 0x7FFFFFF) << 4;
    unsigned int temp_ctime_type = packed_buf[ poswords_to + POS_TT_CTIME_TYPE ];
    packed_buf[ poswords_to + POS_TT_TAG ] = rawcpr_info.eve_num;
    packed_buf[ poswords_to + POS_TT_UTIME ] = rawcpr_info.tt_utime;
    packed_buf[ poswords_to + POS_EXP_RUN ] = (rawcpr_info.exp_num << tmp_header.EXP_SHIFT) | (rawcpr_info.run_subrun_num &
                                              0x003FFFFF);   // exp. and run #
    packed_buf[ poswords_to + POS_B2L_CTIME ] = (rawcpr_info.b2l_ctime & 0x7FFFFFF) << 4;
    poswords_to += SIZE_B2LFEE_HEADER;

    // copy the 1st Detector Buffer
    memcpy(packed_buf + poswords_to, detector_buf[ i ], nwords[ i ]*sizeof(int));
    poswords_to += nwords[ i ];

    // Fill b2link b2tt-tag trailer
    packed_buf[ poswords_to + POS_TT_CTIME_B2LFEE ] = temp_ctime_type;

    // Fill b2link FEE trailer
    unsigned short crc16 = CalcCRC16LittleEndian(0xffff, crc16_start, nwords[ i ] + SIZE_B2LFEE_HEADER);
    packed_buf[ poswords_to + POS_CHKSUM_B2LFEE ] = ((0xffff & rawcpr_info.eve_num) << 16) | (crc16 & 0xffff);
    poswords_to += SIZE_B2LFEE_TRAILER;

    // Fill b2link HSLB trailer
    packed_buf[ poswords_to + POS_CHKSUM_B2LHSLB ] = 0xff550000;
    poswords_to += SIZE_B2LHSLB_TRAILER;

  }

  // Fill COPPER trailer
  packed_buf[ poswords_to + POS_MAGIC_COPPER_3 ] = COPPER_MAGIC_FPGA_TRAILER;
  packed_buf[ poswords_to + POS_MAGIC_COPPER_4 ] = COPPER_MAGIC_DRIVER_TRAILER;
  unsigned int chksum = 0;
  for (int i = tmp_header.GetHdrNwords(); i < poswords_to + (SIZE_COPPER_TRAILER - SIZE_COPPER_DRIVER_TRAILER); i++) {
    chksum ^= packed_buf[ i ];
  }
  packed_buf[ poswords_to + POS_CHKSUM_COPPER ] = chksum;
  poswords_to += SIZE_COPPER_TRAILER;

  // Calculate RawCOPPER checksum and fill RawTrailer
  chksum = 0;
  for (int i = 0; i < poswords_to; i++) {
    chksum ^= packed_buf[ i ];
  }
  packed_buf[ poswords_to + tmp_trailer.POS_CHKSUM ] = chksum;

  packed_buf[ poswords_to + tmp_trailer.POS_TERM_WORD ] = tmp_trailer.MAGIC_WORD_TERM_TRAILER;
  poswords_to += tmp_trailer.GetTrlNwords();

  *packed_buf_nwords = poswords_to;
  return packed_buf;
}

