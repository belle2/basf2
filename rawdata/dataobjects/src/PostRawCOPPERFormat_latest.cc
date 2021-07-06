/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <rawdata/dataobjects/PostRawCOPPERFormat_latest.h>


using namespace std;
using namespace Belle2;

//#define DESY
//#define NO_ERROR_STOP
//ClassImp(PostRawCOPPERFormat_latest);

PostRawCOPPERFormat_latest::PostRawCOPPERFormat_latest()
{
}

PostRawCOPPERFormat_latest::~PostRawCOPPERFormat_latest()
{
}

unsigned int PostRawCOPPERFormat_latest::CalcDriverChkSum(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}





unsigned int PostRawCOPPERFormat_latest::GetB2LFEE32bitEventNumber(int n)
{
  char err_buf[500];
  char hostname[128];
  GetNodeName(n, hostname, sizeof(hostname));
  sprintf(err_buf,
          "[FATAL] %s ch=%d : ERROR_EVENT : No event # in B2LFEE header. (block %d) Exiting... : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
          hostname, -1,
          n,
          GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}



void PostRawCOPPERFormat_latest::CheckData(int n,
                                           unsigned int prev_evenum, unsigned int* cur_evenum_rawcprhdr,
                                           unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                                           unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)
{
  char err_buf[500];
  int err_flag = 0;

  //
  // Check incrementation of event #
  //
  *cur_evenum_rawcprhdr = GetEveNo(n);
  *cur_exprunsubrun_no = GetExpRunSubrun(n);

  if (prev_exprunsubrun_no == *cur_exprunsubrun_no) {
    if ((unsigned int)(prev_evenum + 1) != *cur_evenum_rawcprhdr) {
      sprintf(err_buf, "CORRUPTED DATA: Event # jump : i %d prev 0x%x cur 0x%x : Exiting...\n%s %s %d\n",
              n, prev_evenum, *cur_evenum_rawcprhdr,
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      err_flag = 1;
    }
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
            "[FATAL] %s ch=%d : ERROR_EVENT : checksum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x : eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
            hostname, -1,
            n, GetBlockNwords(n), *cur_evenum_rawcprhdr, tmp_trailer.GetChksum(), xor_chksum,
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }


  if (err_flag == 1) {
    printf("[DEBUG] %s\n", err_buf);
    printf("[DEBUG] ========== dump a data blcok : block # %d==========\n", n);
    PrintData(GetBuffer(n), GetBlockNwords(n));
    printf("Print out variables to reduce unused-variables-warnings : %u %u\n", prev_copper_ctr, *cur_copper_ctr);
    B2FATAL(err_buf);
  }

  return;

}

bool PostRawCOPPERFormat_latest::CheckCOPPERMagic(int n)
{
  char err_buf[500];
  char hostname[128];
  GetNodeName(n, hostname, sizeof(hostname));
  sprintf(err_buf,
          "[FATAL] %s ch=%d : ERROR_EVENT : No magic word # in COPPER header (block %d). Exiting...: eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
          hostname, -1,
          n,
          GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return false;
}

void PostRawCOPPERFormat_latest::CheckUtimeCtimeTRGType(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported (block %d). Exiting...\n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
}

unsigned int PostRawCOPPERFormat_latest::FillTopBlockRawHeader(unsigned int m_node_id, unsigned int prev_eve32,
    unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)

{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function should be called by PrePostRawCOPPERFormat_***. Exiting...\n %s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("Print out variables to reduce unused-variables-warnings : %u %u %u %u\n",
         m_node_id,  prev_eve32, prev_exprunsubrun_no, *cur_exprunsubrun_no);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
}


int PostRawCOPPERFormat_latest::CheckB2LHSLBMagicWords(int* finesse_buf, int finesse_nwords)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function should be called by PrePostRawCOPPERFormat_***. Exiting...\n %s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("Print out variables to reduce unused-variables-warnings : %p %d\n", finesse_buf, finesse_nwords);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
}

int PostRawCOPPERFormat_latest::CheckCRC16(int n, int finesse_num)
{

  //
  // Calculate CRC16
  //
  int finesse_nwords = GetFINESSENwords(n, finesse_num);
  if (finesse_nwords <= 0) {
    char err_buf[500];
    char hostname[128];
    GetNodeName(n, hostname, sizeof(hostname));
    sprintf(err_buf,
            "[FATAL] %s ch=%d : ERROR_EVENT : The specified finesse(%c) seems to be empty(nwords = %d). Cannot calculate CRC16. Exiting...: eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
            hostname, finesse_num,
            65 + finesse_num, finesse_nwords,
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  int* copper_buf = GetBuffer(n);


  unsigned short temp_crc16 = CalcCRC16LittleEndian(0xffff, &(copper_buf[ tmp_header.POS_TTCTIME_TRGTYPE ]), 1);
  temp_crc16 = CalcCRC16LittleEndian(temp_crc16, &(copper_buf[ tmp_header.POS_EVE_NO ]), 1);
  temp_crc16 = CalcCRC16LittleEndian(temp_crc16, &(copper_buf[ tmp_header.POS_TTUTIME ]), 1);
  temp_crc16 = CalcCRC16LittleEndian(temp_crc16, &(copper_buf[ tmp_header.POS_EXP_RUN_NO ]), 1);
  int* buf = GetFINESSEBuffer(n, finesse_num) +  SIZE_B2LHSLB_HEADER + POS_B2L_CTIME;
  int pos_nwords = finesse_nwords - (SIZE_B2LHSLB_HEADER + POS_B2L_CTIME + SIZE_B2LFEE_TRAILER + SIZE_B2LHSLB_TRAILER);
  temp_crc16 = CalcCRC16LittleEndian(temp_crc16, buf, pos_nwords);

  //
  // Compare CRC16 with B2LCRC16
  //
  buf = GetFINESSEBuffer(n, finesse_num) +  GetFINESSENwords(n,
                                                             finesse_num) - ((SIZE_B2LFEE_TRAILER - POS_B2LFEE_ERRCNT_CRC16) + SIZE_B2LHSLB_TRAILER) ;

  if (GetEveNo(n) % 100000 == 0) {
    printf("#### PostRawCOPPER : Eve %.8x block %d finesse %d B2LCRC16 %.8x calculated CRC16 %.8x\n", GetEveNo(n), n, finesse_num,
           *buf, temp_crc16);
  }

  //  if ( false ) {
  if ((unsigned short)(*buf & 0xFFFF) != temp_crc16) {

    // dump an event
    int copper_nwords = copper_buf[ tmp_header.POS_NWORDS ];
    PrintData(copper_buf, copper_nwords);
    // Check whether packet-CRC error has occcured or not.
    if (copper_buf[ tmp_header.POS_TRUNC_MASK_DATATYPE ] & tmp_header.B2LINK_PACKET_CRC_ERROR) {
      //
      // Do not stop data
      //
      char err_buf[500];
      char hostname[128];
      GetNodeName(n, hostname, sizeof(hostname));
      if ((GetNodeID(n) & DETECTOR_MASK) == ARICH_ID) {
        sprintf(err_buf,
                "[WARNING] %s ch=%d : ARICH : POST B2link event CRC16 error with B2link Packet CRC error. data(%x) calc(%x) fns nwords %d type 0x%.8x : This error is ignored and the error event will be recorded in .sroot file acording to request from ARICH group: slot%c eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
                hostname, finesse_num,
                *buf , temp_crc16, GetFINESSENwords(n, finesse_num), copper_buf[ tmp_header.POS_TRUNC_MASK_DATATYPE ],
                65 + finesse_num, GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        printf("%s", err_buf); fflush(stdout);
        PrintData(GetFINESSEBuffer(n, finesse_num), GetFINESSENwords(n, finesse_num));
      } else {
        sprintf(err_buf,
                "[FATAL] %s ch=%d : ERROR_EVENT : POST B2link event CRC16 error with B2link Packet CRC error. data(%x) calc(%x) fns nwords %d type 0x%.8x : slot%c eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
                hostname, finesse_num,
                *buf , temp_crc16, GetFINESSENwords(n, finesse_num), copper_buf[ tmp_header.POS_TRUNC_MASK_DATATYPE ],
                65 + finesse_num, GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        printf("%s", err_buf); fflush(stdout);
        PrintData(GetFINESSEBuffer(n, finesse_num), GetFINESSENwords(n, finesse_num));
#ifndef NO_ERROR_STOP
        B2FATAL(err_buf);
#endif
      }



    } else {
      //
      // Stop taking data
      //
      char err_buf[500];
      char hostname[128];
      GetNodeName(n, hostname, sizeof(hostname));
      sprintf(err_buf,
              "[FATAL] %s ch=%d : ERROR_EVENT : POST B2link event CRC16 error without B2link Packet CRC error. data(%x) calc(%x) fns nwords %d type 0x%.8x: slot%c eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
              hostname, finesse_num,
              *buf , temp_crc16, GetFINESSENwords(n, finesse_num), copper_buf[ tmp_header.POS_TRUNC_MASK_DATATYPE ],
              65 + finesse_num, GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("%s", err_buf); fflush(stdout);
      PrintData(GetFINESSEBuffer(n, finesse_num), GetFINESSENwords(n, finesse_num));
#ifndef NO_ERROR_STOP
      B2FATAL(err_buf);
#endif
    }
    // Modify XOR checksum due to adding a bit flag
    copper_buf[ copper_nwords - tmp_trailer.RAWTRAILER_NWORDS + tmp_trailer.POS_CHKSUM ]
    ^= copper_buf[ tmp_header.POS_TRUNC_MASK_DATATYPE ];
    copper_buf[ tmp_header.POS_TRUNC_MASK_DATATYPE ] |= tmp_header.B2LINK_EVENT_CRC_ERROR;
    copper_buf[ copper_nwords - tmp_trailer.RAWTRAILER_NWORDS + tmp_trailer.POS_CHKSUM ]
    ^= copper_buf[ tmp_header.POS_TRUNC_MASK_DATATYPE ];
  }

  return 1;


}


int* PostRawCOPPERFormat_latest::PackDetectorBuf(int* /*packed_buf_nwords*/,
                                                 int* /*detector_buf_1st*/,  int /*nwords_1st*/,
                                                 int* /*detector_buf_2nd*/,  int /*nwords_2nd*/,
                                                 int* /*detector_buf_3rd*/,  int /*nwords_3rd*/,
                                                 int* /*detector_buf_4th*/,  int /*nwords_4th*/,
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


int* PostRawCOPPERFormat_latest::PackDetectorBuf(int* packed_buf_nwords,
                                                 int* const(&detector_buf_ch)[MAX_PCIE40_CH],
                                                 int const(&nwords_ch)[MAX_PCIE40_CH],
                                                 RawCOPPERPackerInfo rawcpr_info)
{
  int* packed_buf = NULL;
  int poswords_to = 0;

  // calculate the event length
  int length_nwords = tmp_header.GetHdrNwords() + tmp_trailer.GetTrlNwords();

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
  packed_buf[ tmp_header.POS_VERSION_HDRNWORDS ] =
    0x7f7f0000
    | ((DATA_FORMAT_VERSION << tmp_header.FORMAT_VERSION_SHIFT) & tmp_header.FORMAT_VERSION__MASK)
    | tmp_header.RAWHEADER_NWORDS; // ver.#, header length
  packed_buf[ tmp_header.POS_EXP_RUN_NO ] = (rawcpr_info.exp_num << 22)
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
    packed_buf[ poswords_to + POS_B2LHSLB_MAGIC ] = 0xffaa0000 | (0xffff & rawcpr_info.eve_num);
    poswords_to += SIZE_B2LHSLB_HEADER;

    // Fill b2link FEE header
    packed_buf[ poswords_to + POS_B2L_CTIME ] = (rawcpr_info.b2l_ctime & 0x7FFFFFF) << 4;
    poswords_to += SIZE_B2LFEE_HEADER;

    // copy the 1st Detector Buffer
    memcpy(packed_buf + poswords_to, detector_buf_ch[ i ], nwords_ch[ i ]*sizeof(int));
    poswords_to += nwords_ch[ i ];

    // Fill b2link FEE trailer
    unsigned int crc16 = 0;
    packed_buf[ poswords_to + POS_B2LFEE_ERRCNT_CRC16 ] =
      ((0xffff & rawcpr_info.eve_num) << 16) | (crc16 &
                                                0xffff);       // Error count is stored in this buffer for ver.2 format but it is set to zero here.
    poswords_to += SIZE_B2LFEE_TRAILER;

    // Fill b2link HSLB trailer
    packed_buf[ poswords_to + POS_B2LHSLB_TRL_MAGIC ] =  0xff550000;
    poswords_to += SIZE_B2LHSLB_TRAILER;

  }

  // Fill RawTrailer
  packed_buf[ poswords_to + tmp_trailer.POS_TERM_WORD ] = tmp_trailer.MAGIC_WORD_TERM_TRAILER;
  poswords_to += tmp_trailer.GetTrlNwords();

  *packed_buf_nwords = poswords_to;

  return packed_buf;
}

























