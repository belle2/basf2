/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

unsigned int PreRawCOPPERFormat_latest::GetB2LFEE32bitEventNumber(int n)
{

#ifndef READ_OLD_B2LFEE_FORMAT_FILE

  int err_flag = 0;
  unsigned int eve_num = 0;
  int flag = 0;
  unsigned int eve[MAX_PCIE40_CH];
  for (int i = 0; i < MAX_PCIE40_CH; i++) {
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
    char hostname[128];
    GetNodeName(n, hostname, sizeof(hostname));
    sprintf(err_buf,
            "[FATAL] %s ch=%d : ERROR_EVENT : No HSLB data in COPPER data. Exiting... : eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
            hostname, -1,
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    //    string err_str = err_buf; throw (err_str);
    B2FATAL(err_buf); // to reduce multiple error messages
  }

  if (err_flag == 1) {
    vector<vector<unsigned int>> summary_table;
    CompareHeaderValue(n, eve, summary_table);

    char err_buf[2500];
    char err_buf_temp[2500];
    char hostname[128];
    GetNodeName(n, hostname, sizeof(hostname));
    sprintf(err_buf, "[FATAL] %s ch=%d : ERROR_EVENT : CORRUPTED DATA: Different event number over HSLBs :", hostname, -1);
    for (unsigned i = 0; i < summary_table.size(); ++i) {
      memcpy(err_buf_temp, err_buf, sizeof(err_buf_temp));
      sprintf(err_buf, "%s [ch= %u ,val= %u (# of chs= %u )] ",
              err_buf_temp,
              summary_table.at(i).at(0), summary_table.at(i).at(2), summary_table.at(i).at(1));
    }
    memcpy(err_buf_temp, err_buf, sizeof(err_buf_temp));
    sprintf(err_buf, "%s : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
            err_buf_temp,
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);

    printf("[DEBUG] %s\n", err_buf);
    PrintData(m_buffer, m_nwords);
    for (int i = 0; i < MAX_PCIE40_CH; ++i) {
      printf("[DEBUG] ========== CRC check : block # %d finesse %d ==========\n", n, i);
      if (GetFINESSENwords(n, i) > 0) {
        CheckCRC16(n, i);
      }
    }
    printf("[DEBUG] ========== No CRC error. : block %d =========\n", n);
#ifndef NO_ERROR_STOP
    //    string err_str = err_buf; throw (err_str);
    B2FATAL(err_buf); // to reduce multiple error messages
#endif //NO_ERROR_STOP
  }
  return eve_num;

#else // READ_OLD_B2LFEE_FORMAT_FILE

  char err_buf[500];
  sprintf(err_buf, "[FATAL] You need comment out READ_OLD_B2LFEE_FORMAT_FILE if you are handling a new data format\n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s", err_buf); fflush(stdout);
  B2FATAL(err_buf); // to reduce multiple error messages
#endif // READ_OLD_B2LFEE_FORMAT_FILE

}


void PreRawCOPPERFormat_latest::CheckData(int n,
                                          unsigned int prev_evenum, unsigned int* cur_evenum_rawcprhdr,
                                          unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                                          unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)
{

  char err_buf[2500];
  int err_flag = 0;

  //
  // Event # check
  //
  *cur_evenum_rawcprhdr = GetEveNo(n);
  unsigned int evenum_feehdr = GetB2LFEE32bitEventNumber(n);
  if (*cur_evenum_rawcprhdr != evenum_feehdr) {
    char hostname[128];
    GetNodeName(n, hostname, sizeof(hostname));
    sprintf(err_buf,
            "[FATAL] %s ch=%d : ERROR_EVENT : Event # in PreRawCOPPERFormat_latest header and FEE header is different : cprhdr 0x%x feehdr 0x%x : Exiting... : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
            hostname, -1,
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
      char hostname[128];
      GetNodeName(n, hostname, sizeof(hostname));
      sprintf(err_buf,
              "[FATAL] %s ch=%d : ERROR_EVENT : Event # jump : i %d prev 0x%x cur 0x%x : prevrun %.8x currun %.8x: Exiting... : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
              hostname, -1,
              n, prev_evenum, *cur_evenum_rawcprhdr, prev_exprunsubrun_no, *cur_exprunsubrun_no,
              GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      err_flag = 1;
    }
    if ((unsigned int)(prev_copper_ctr + 1) != *cur_copper_ctr) {
      char hostname[128];
      GetNodeName(n, hostname, sizeof(hostname));
      sprintf(err_buf,
              "[FATAL] %s ch=%d : ERROR_EVENT : COPPER counter jump : i %d prev 0x%x cur 0x%x : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
              hostname, -1,
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

        unsigned int eve[MAX_PCIE40_CH];
        for (int i = 0; i < MAX_PCIE40_CH ; ++i) {
          eve[ i ] = 0xbaadf00d;
          if (GetFINESSENwords(n, i) > 0) {
            int pos_nwords = GetOffsetFINESSE(n, i) + SIZE_B2LHSLB_HEADER + POS_TT_TAG;
            eve[ i ] = m_buffer[ pos_nwords ];
          }
        }

        vector<vector<unsigned int>> summary_table;
        CompareHeaderValue(n, eve, summary_table);
        char hostname[128];
        GetNodeName(n, hostname, sizeof(hostname));
        sprintf(err_buf,
                "[FATAL] %s ch=%d : ERROR_EVENT : Invalid Event # at the beginning of the run (It should be zero.): preveve 0x%x cureve 0x%x : prev(exp %u run %u sub %u ) cur(exp %u run %u sub %u ) (",
                hostname, -1,
                prev_evenum, *cur_evenum_rawcprhdr,
                prev_exprunsubrun_no >> 22 , (prev_exprunsubrun_no >> 8) & 0x3FFF, prev_exprunsubrun_no & 0xFF,
                *cur_exprunsubrun_no >> 22 , (*cur_exprunsubrun_no >> 8) & 0x3FFF, *cur_exprunsubrun_no & 0xFF);

        char err_buf_temp[2500];
        for (unsigned i = 0; i < summary_table.size(); ++i) {
          memcpy(err_buf_temp, err_buf, sizeof(err_buf_temp));
          sprintf(err_buf, "%s [ch= %u ,val= %u (# of chs= %u )] ",
                  err_buf_temp,
                  summary_table.at(i).at(0), summary_table.at(i).at(2), summary_table.at(i).at(1));
        }
        memcpy(err_buf_temp, err_buf, sizeof(err_buf_temp));
        sprintf(err_buf, "%s Exiting... : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
                err_buf_temp,
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
    char hostname[128];
    GetNodeName(n, hostname, sizeof(hostname));
    sprintf(err_buf,
            "[FATAL] %s ch=%d : ERROR_EVENT : COPPER driver checkSum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
            hostname, -1,
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
    char hostname[128];
    GetNodeName(n, hostname, sizeof(hostname));
    sprintf(err_buf,
            "[FATAL] %s ch=%d : ERROR_EVENT : PreRawCOPPERFormat_latest checksum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x : eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
            hostname, -1,
            n, GetBlockNwords(n), *cur_evenum_rawcprhdr, tmp_trailer.GetChksum(), xor_chksum,
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }

  if (err_flag == 1) {
    printf("%s", err_buf); fflush(stdout);
    printf("[DEBUG] ========== dump a data blcok : block # %d==========\n", n);
    PrintData(GetBuffer(n), GetBlockNwords(n));
    for (int i = 0; i < MAX_PCIE40_CH; i++) {
      printf("[DEBUG] ========== CRC check : block # %d finesse %d ==========\n", n, i);
      if (GetFINESSENwords(n, i) > 0) {
        CheckCRC16(n, i);
      }
    }
    printf("[DEBUG] ========== No CRC error : block %d =========\n", n);
    //    string err_str = err_buf;    throw (err_str);
    B2FATAL(err_buf); // to reduce multiple error messages
  }

  return;

}

bool PreRawCOPPERFormat_latest::CheckCOPPERMagic(int /*n*/)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return false;
}

void PreRawCOPPERFormat_latest::CheckUtimeCtimeTRGType(int n)
{

#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  CheckB2LFEEHeaderVersion(n);
#endif
  int err_flag = 0;
  int flag = 0;
  unsigned int temp_utime = 0, temp_ctime_trgtype = 0, temp_eve = 0, temp_exprun = 0;
  unsigned int temp_ctime_trgtype_footer, temp_eve_footer;
  unsigned int utime[MAX_PCIE40_CH], ctime_trgtype[MAX_PCIE40_CH], eve[MAX_PCIE40_CH], exprun[MAX_PCIE40_CH];
  char err_buf[2500];

  memset(utime, 0, sizeof(utime));
  memset(ctime_trgtype, 0, sizeof(ctime_trgtype));
  memset(eve, 0, sizeof(eve));
  memset(exprun, 0, sizeof(exprun));

  int first_ch = 0;
  for (int i = 0; i < MAX_PCIE40_CH; i++) {
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
        first_ch = i;
      } else {
        if (temp_ctime_trgtype != ctime_trgtype[ i ] || temp_utime != utime[ i ] ||
            temp_eve != eve[ i ] || temp_exprun != exprun[ i ]) {
          char hostname[128];
          GetNodeName(n, hostname, sizeof(hostname));
          if (err_flag == 0) {
            for (int j = 0; j < MAX_PCIE40_CH; j++) {
              if (GetFINESSENwords(n, j) > 0) {
                printf("[DEBUG] %s ch=%d : FINESSE #=%d buffsize %d ctimeTRGtype 0x%.8x utime 0x%.8x eve 0x%.8x exprun 0x%.8x\n",
                       hostname, -1,
                       j, GetFINESSENwords(n, j), ctime_trgtype[ j ], utime[ j ], eve[ j ], exprun[ j ]);
              }
            }
          }

          char err_buf_1[2500], err_buf_2[2500], err_buf_3[2500];
          sprintf(err_buf_1,
                  "[FATAL] %s ch=%d : ERROR_EVENT : mismatch header value over FINESSEs. Exiting...",
                  hostname, -1);
          sprintf(err_buf_2,
                  "%s FINESSE #=%d buffsize %d ctimeTRGtype 0x%.8x utime 0x%.8x eve 0x%.8x exprun 0x%.8x",
                  err_buf_1,
                  first_ch, GetFINESSENwords(n, first_ch), ctime_trgtype[ first_ch ], utime[ first_ch ], eve[ first_ch ], exprun[ first_ch ]);
          sprintf(err_buf_3,
                  "%s FINESSE #=%d buffsize %d ctimeTRGtype 0x%.8x utime 0x%.8x eve 0x%.8x exprun 0x%.8x",
                  err_buf_2,
                  i, GetFINESSENwords(n, i), ctime_trgtype[ i ], utime[ i ], eve[ i ], exprun[ i ]);
          sprintf(err_buf, "%s\n %s %s %d\n",
                  err_buf_3,
                  __FILE__, __PRETTY_FUNCTION__, __LINE__);
          printf("%s", err_buf); fflush(stdout);

          err_flag = 1;
          break;
        } else if (temp_ctime_trgtype != temp_ctime_trgtype_footer ||
                   (temp_eve & 0xffff) != ((temp_eve_footer >> 16) & 0xffff)) {
          char hostname[128];
          GetNodeName(n, hostname, sizeof(hostname));
          sprintf(err_buf,
                  "[FATAL] %s ch=%d : ERROR_EVENT : mismatch(finesse %d) between header(ctime %.8x eve %.8x) and footer(ctime %.8x eve_crc16 %.8x). Exiting...\n %s %s %d\n",
                  hostname, i,
                  i,  temp_ctime_trgtype,  temp_eve, temp_ctime_trgtype_footer, temp_eve_footer,
                  __FILE__, __PRETTY_FUNCTION__, __LINE__);
          printf("%s", err_buf); fflush(stdout);
          err_flag = 1;
        }
      }
    }
  }

  if (err_flag != 0) {
    for (int i = 0; i < MAX_PCIE40_CH; i++) {
      if (GetFINESSENwords(n, i) > 0) {
        printf("[DEBUG] ========== CRC check : block # %d finesse %d ==========\n", n, i);
        CheckCRC16(n, i);
        printf("[DEBUG] ========== CRC check is done. : block %d =========\n", n);
      }
    }
#ifndef NO_ERROR_STOP
    //    string err_str = err_buf; throw (err_str);
    B2FATAL(err_buf); // to reduce multiple error messages
#endif
  }
  return;
}

unsigned int PreRawCOPPERFormat_latest::FillTopBlockRawHeader(unsigned int /*m_node_id*/, unsigned int /*prev_eve32*/,
    unsigned int /*prev_exprunsubrun_no*/, unsigned int* /*cur_exprunsubrun_no*/)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}



#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
void PreRawCOPPERFormat_latest::CheckB2LFEEHeaderVersion(int n)
{
  int* temp_buf;
  for (int i = 0; i < MAX_PCIE40_CH; i++) {
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
        char hostname[128];
        GetNodeName(n, hostname, sizeof(hostname));
        sprintf(err_buf, "[FATAL] %s ch=%d : ERROR_EVENT : FTSW and b2tt firmwares are old. Exiting...\n %s %s %d\n",
                hostname, i,
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        B2FATAL(err_buf);
#endif
      } else {
        // this word for 32bit unixtime
        // new one (ver.2)
        break;
      }
    }

    if (i == MAX_PCIE40_CH - 1) {
#ifdef TEMP
      char err_buf[500];
      char hostname[128];
      GetNodeName(n, hostname, sizeof(hostname));
      sprintf(err_buf, "[FATAL] %s ch=%d : ERROR_EVENT : PreRawCOPPERFormat_latest contains no FINESSE data. Exiting...\n %s %s %d\n",
              hostname, i,
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("%s", err_buf); fflush(stdout);
      //      string err_str = err_buf; throw (err_str);
      B2FATAL(err_buf); // to reduce multiple error messages
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


int PreRawCOPPERFormat_latest::CalcReducedNwords(int /*n*/)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return -1;
}


int PreRawCOPPERFormat_latest::CopyReducedBuffer(int /*n*/, int* /*buf_to*/)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return -1;
}


int PreRawCOPPERFormat_latest::CheckB2LHSLBMagicWords(int* finesse_buf, int finesse_nwords)
{

  if ((finesse_buf[ POS_MAGIC_B2LHSLB ] & 0xFFFF0000) == B2LHSLB_HEADER_MAGIC &&
      ((finesse_buf[ finesse_nwords - SIZE_B2LHSLB_TRAILER + POS_CHKSUM_B2LHSLB ] & 0xFFFF0000)
       == B2LHSLB_TRAILER_MAGIC)) {
    return 1;
  } else {
    PrintData(m_buffer, m_nwords);
    char err_buf[500];
    sprintf(err_buf,
            "Invalid B2LHSLB magic words : header 0x%x (= should be ffaa**** ) or trailer 0x%x (= should be ff55**** ). Exiting... :%s %s %d\n",
            finesse_buf[ POS_MAGIC_B2LHSLB ],
            finesse_buf[ finesse_nwords - SIZE_B2LHSLB_TRAILER + POS_CHKSUM_B2LHSLB ],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
#ifndef NO_ERROR_STOP
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
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
    char err_buf[500];
    char hostname[128];
    GetNodeName(n, hostname, sizeof(hostname));
    PrintData(GetBuffer(n), *(GetBuffer(n) + tmp_header.POS_NWORDS));
    printf("[FATAL] %s ch=%d : ERROR_EVENT : PRE CRC16 error : slot %c : B2LCRC16 %x Calculated CRC16 %x : Nwords of FINESSE buf %d\n",
           hostname, finesse_num,
           65 + finesse_num, *buf , temp_crc16, GetFINESSENwords(n, finesse_num));
    int* temp_buf = GetFINESSEBuffer(n, finesse_num);
    for (int k = 0; k <  GetFINESSENwords(n, finesse_num); k++) {
      printf("%.8x ", temp_buf[ k ]);
      if ((k + 1) % 10 == 0) {
        printf("\n");
      }
    }
    printf("\n");
    fflush(stdout);
    sprintf(err_buf,
            "[FATAL] %s ch=%d : ERROR_EVENT : slot %c : B2LCRC16 (%.4x) differs from one ( %.4x) calculated by PreRawCOPPERfromat class. Exiting...\n %s %s %d\n",
            hostname, finesse_num,
            65 + finesse_num, (unsigned short)(*buf & 0xFFFF), temp_crc16,
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf); // to reduce multiple error messages
  }
  return 1;

}

int* PreRawCOPPERFormat_latest::PackDetectorBuf(int* /*packed_buf_nwords*/,
                                                int* /*detector_buf_1st*/, int /*nwords_1st*/,
                                                int* /*detector_buf_2nd*/, int /*nwords_2nd*/,
                                                int* /*detector_buf_3rd*/, int /*nwords_3rd*/,
                                                int* /*detector_buf_4th*/, int /*nwords_4th*/,
                                                RawCOPPERPackerInfo rawcpr_info)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported. (%u) Exiting...: \n%s %s %d\n",
          rawcpr_info.eve_num,
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return NULL;
}

int* PreRawCOPPERFormat_latest::PackDetectorBuf(int* packed_buf_nwords,
                                                int* const(&detector_buf_ch)[MAX_PCIE40_CH],
                                                int const(&nwords_ch)[MAX_PCIE40_CH],
                                                RawCOPPERPackerInfo rawcpr_info)
{
  int* packed_buf = NULL;
  int poswords_to = 0;

  // calculate the event length
  int length_nwords = tmp_header.GetHdrNwords() + SIZE_COPPER_HEADER + SIZE_COPPER_TRAILER + tmp_trailer.GetTrlNwords();

  for (int i = 0; i < MAX_PCIE40_CH; i++) {
    if (detector_buf_ch[ i ] == NULL || nwords_ch[ i ] <= 0) continue;    // for an empty FINESSE slot
    length_nwords += nwords_ch[ i ];
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
  int ch = 0;
  packed_buf[ tmp_header.POS_CH_POS_TABLE + ch ] = tmp_header.RAWHEADER_NWORDS;
  for (int i = 1; i < MAX_PCIE40_CH; i++) {
    ch = i;
    if (nwords_ch[ ch - 1 ] == 0) {
      packed_buf[ tmp_header.POS_CH_POS_TABLE + ch ] = packed_buf[ tmp_header.POS_CH_POS_TABLE + (ch - 1) ];
    } else {
      packed_buf[ tmp_header.POS_CH_POS_TABLE + ch ] = packed_buf[ tmp_header.POS_CH_POS_TABLE + (ch - 1) ] +
                                                       nwords_ch[ ch - 1 ] + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER  + SIZE_B2LFEE_TRAILER + SIZE_B2LHSLB_TRAILER;
    }
  }
  poswords_to += tmp_header.GetHdrNwords();

  // Fill FINESSE buffer
  for (int i = 0; i < MAX_PCIE40_CH; i++) {
    if (detector_buf_ch[ i ] == NULL || nwords_ch[ i ] <= 0) continue;     // for an empty FINESSE slot

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
    memcpy(packed_buf + poswords_to, detector_buf_ch[ i ], nwords_ch[ i ]*sizeof(int));
    poswords_to += nwords_ch[ i ];

    // Fill b2link b2tt-tag trailer
    packed_buf[ poswords_to + POS_TT_CTIME_B2LFEE ] = temp_ctime_type;

    // Fill b2link FEE trailer
    unsigned short crc16 = CalcCRC16LittleEndian(0xffff, crc16_start, nwords_ch[ i ] + SIZE_B2LFEE_HEADER);
    packed_buf[ poswords_to + POS_CHKSUM_B2LFEE ] = ((0xffff & rawcpr_info.eve_num) << 16) | (crc16 & 0xffff);
    poswords_to += SIZE_B2LFEE_TRAILER;

    // Fill b2link HSLB trailer
    packed_buf[ poswords_to + POS_CHKSUM_B2LHSLB ] = 0xff550000;
    poswords_to += SIZE_B2LHSLB_TRAILER;
  }

  // Calculate RawCOPPER checksum and fill RawTrailer
  unsigned int chksum = 0;
  for (int i = 0; i < poswords_to; i++) {
    chksum ^= packed_buf[ i ];
  }
  packed_buf[ poswords_to + tmp_trailer.POS_CHKSUM ] = chksum;

  packed_buf[ poswords_to + tmp_trailer.POS_TERM_WORD ] = tmp_trailer.MAGIC_WORD_TERM_TRAILER;
  poswords_to += tmp_trailer.GetTrlNwords();

  *packed_buf_nwords = poswords_to;
  return packed_buf;
}


