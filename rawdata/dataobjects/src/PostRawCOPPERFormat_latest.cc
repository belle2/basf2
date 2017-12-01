//+
// File : RawCOPPERFormat_latest.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

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


int PostRawCOPPERFormat_latest::GetDetectorNwords(int n, int finesse_num)
{
  int nwords = 0;
  if (GetFINESSENwords(n, finesse_num) > 0) {
    nwords = GetFINESSENwords(n, finesse_num)
             - (SIZE_B2LHSLB_HEADER + SIZE_B2LHSLB_TRAILER +  SIZE_B2LFEE_HEADER + SIZE_B2LFEE_TRAILER);
  }
  return nwords;
}


unsigned int PostRawCOPPERFormat_latest::CalcDriverChkSum(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf; throw (err_str);
  return 0;
}


int PostRawCOPPERFormat_latest::GetFINESSENwords(int n, int finesse_num)
{
  int pos_nwords_0, pos_nwords_1;
  int nwords;
  switch (finesse_num) {
    case 0 :
      pos_nwords_0 = GetBufferPos(n) + tmp_header.POS_OFFSET_1ST_FINESSE;
      pos_nwords_1 = GetBufferPos(n) + tmp_header.POS_OFFSET_2ND_FINESSE;
      nwords = m_buffer[ pos_nwords_1 ] - m_buffer[ pos_nwords_0 ];
      break;
    case 1 :
      pos_nwords_0 = GetBufferPos(n) + tmp_header.POS_OFFSET_2ND_FINESSE;
      pos_nwords_1 = GetBufferPos(n) + tmp_header.POS_OFFSET_3RD_FINESSE;
      nwords = m_buffer[ pos_nwords_1 ] - m_buffer[ pos_nwords_0 ];
      break;
    case 2 :
      pos_nwords_0 = GetBufferPos(n) + tmp_header.POS_OFFSET_3RD_FINESSE;
      pos_nwords_1 = GetBufferPos(n) + tmp_header.POS_OFFSET_4TH_FINESSE;
      nwords = m_buffer[ pos_nwords_1 ] - m_buffer[ pos_nwords_0 ];
      break;
    case 3 :
      pos_nwords_0 = GetBufferPos(n) + tmp_header.POS_OFFSET_4TH_FINESSE;
      {
        int nwords_1 = GetBlockNwords(n)
                       - SIZE_COPPER_DRIVER_TRAILER
                       - tmp_trailer.GetTrlNwords();
        nwords = nwords_1 - m_buffer[ pos_nwords_0 ];
      }
      break;
    default :
      char err_buf[500];
      sprintf(err_buf, "[FATAL] Invalid finesse # : %s %s %d\n",
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      string err_str = err_buf; throw (err_str);
  }

  if (nwords < 0 || nwords > 1e6) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] ERROR_EVENT : # of words is strange. %d : eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
            nwords,
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf; throw (err_str);
  }

  return nwords;

}




unsigned int PostRawCOPPERFormat_latest::GetB2LFEE32bitEventNumber(int n)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] ERROR_EVENT : No event # in B2LFEE header. (block %d) Exiting... : eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
          n,
          GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf; throw (err_str);
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
    sprintf(err_buf,
            "[FATAL] ERROR_EVENT : checksum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x : eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
            n, GetBlockNwords(n), *cur_evenum_rawcprhdr, tmp_trailer.GetChksum(), xor_chksum,
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }


  if (err_flag == 1) {
    printf("[DEBUG] ========== dump a data blcok : block # %d==========\n", n);
    PrintData(GetBuffer(n), GetBlockNwords(n));
    printf("Print out variables to reduce unused-variables-warnings : %u %u\n", prev_copper_ctr, *cur_copper_ctr);
    string err_str = err_buf;
    throw (err_str);

    //     sleep(1234567);
    //     exit(-1);
  }

  return;

}

bool PostRawCOPPERFormat_latest::CheckCOPPERMagic(int n)
{
  char err_buf[500];
  sprintf(err_buf,
          "[FATAL] ERROR_EVENT : No magic word # in COPPER header (block %d). Exiting...: eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
          n,
          GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf; throw (err_str);
  return false;
}

void PostRawCOPPERFormat_latest::CheckUtimeCtimeTRGType(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported (block %d). Exiting...\n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf;
  throw (err_str);
}

unsigned int PostRawCOPPERFormat_latest::FillTopBlockRawHeader(unsigned int m_node_id, unsigned int prev_eve32,
    unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)

{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function should be called by PrePostRawCOPPERFormat_***. Exiting...\n %s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("Print out variables to reduce unused-variables-warnings : %u %u %u %u\n",
         m_node_id,  prev_eve32, prev_exprunsubrun_no, *cur_exprunsubrun_no);
  string err_str = err_buf;
  throw (err_str);

}


int PostRawCOPPERFormat_latest::CheckB2LHSLBMagicWords(int* finesse_buf, int finesse_nwords)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function should be called by PrePostRawCOPPERFormat_***. Exiting...\n %s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("Print out variables to reduce unused-variables-warnings : %p %d\n", finesse_buf, finesse_nwords);
  string err_str = err_buf;
  throw (err_str);

}

int PostRawCOPPERFormat_latest::CheckCRC16(int n, int finesse_num)
{

  //
  // Calculate CRC16
  //
  int finesse_nwords = GetFINESSENwords(n, finesse_num);
  if (finesse_nwords <= 0) {
    char err_buf[500];
    sprintf(err_buf,
            "[FATAL] ERROR_EVENT : The specified finesse(%c) seems to be empty(nwords = %d). Cannot calculate CRC16. Exiting...: eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
            65 + finesse_num, finesse_nwords,
            GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    string err_str = err_buf;
    throw (err_str);
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
    char err_buf[500];

    // dump an event
    int copper_nwords = copper_buf[ tmp_header.POS_NWORDS ];
    PrintData(copper_buf, copper_nwords);
    // Check whether packet-CRC error has occcured or not.
    if (copper_buf[ tmp_header.POS_TRUNC_MASK_DATATYPE ] & tmp_header.B2LINK_PACKET_CRC_ERROR) {
      //
      // Do not stop data
      //
      printf("[ERROR] POST B2link event CRC16 error with B2link Packet CRC error. data(%x) calc(%x) fns nwords %d type 0x%.8x : slot%c eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
             *buf , temp_crc16, GetFINESSENwords(n, finesse_num), copper_buf[ tmp_header.POS_TRUNC_MASK_DATATYPE ],
             65 + finesse_num, GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
             __FILE__, __PRETTY_FUNCTION__, __LINE__);
    } else {
      //
      // Stop taking data
      //
      printf("[FATAL] ERROR_EVENT : POST B2link event CRC16 error without B2link Packet CRC error. data(%x) calc(%x) fns nwords %d type 0x%.8x: slot%c eve 0x%x exp %d run %d sub %d\n%s %s %d\n",
             *buf , temp_crc16, GetFINESSENwords(n, finesse_num), copper_buf[ tmp_header.POS_TRUNC_MASK_DATATYPE ],
             65 + finesse_num, GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
             __FILE__, __PRETTY_FUNCTION__, __LINE__);
      PrintData(GetFINESSEBuffer(n, finesse_num), GetFINESSENwords(n, finesse_num));
#ifndef NO_ERROR_STOP
      string err_str = err_buf;     throw (err_str);
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


int* PostRawCOPPERFormat_latest::PackDetectorBuf(int* packed_buf_nwords,
                                                 int* detector_buf_1st,  int nwords_1st,
                                                 int* detector_buf_2nd,  int nwords_2nd,
                                                 int* detector_buf_3rd,  int nwords_3rd,
                                                 int* detector_buf_4th,  int nwords_4th,
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
  poswords_to += SIZE_COPPER_HEADER;

  // Fill FINESSE buffer
  for (int i = 0; i < 4; i++) {

    if (detector_buf[ i ] == NULL || nwords[ i ] <= 0) continue;     // for an empty FINESSE slot

    // Fill b2link HSLB header
    packed_buf[ poswords_to + POS_B2LHSLB_MAGIC ] = 0xffaa0000 | (0xffff & rawcpr_info.eve_num);
    poswords_to += SIZE_B2LHSLB_HEADER;

    // Fill b2link FEE header
    packed_buf[ poswords_to + POS_B2L_CTIME ] = (rawcpr_info.b2l_ctime & 0x7FFFFFF) << 4;
    poswords_to += SIZE_B2LFEE_HEADER;

    // copy the 1st Detector Buffer
    memcpy(packed_buf + poswords_to, detector_buf[ i ], nwords[ i ]*sizeof(int));
    poswords_to += nwords[ i ];

    // Fill b2link FEE trailer
    unsigned int crc16 = 0;
    packed_buf[ poswords_to + POS_B2LFEE_ERRCNT_CRC16 ] =  crc16 &
                                                           0xffff; // Error count is stored in this buffer for ver.2 format but it is set to zero here.
    poswords_to += SIZE_B2LFEE_TRAILER;

    // Fill b2link HSLB trailer
    poswords_to += SIZE_B2LHSLB_TRAILER;

  }

  // Fill COPPER trailer
  poswords_to += SIZE_COPPER_TRAILER;

  // Fill RawTrailer
  packed_buf[ poswords_to + tmp_trailer.POS_TERM_WORD ] = tmp_trailer.MAGIC_WORD_TERM_TRAILER;
  poswords_to += tmp_trailer.GetTrlNwords();

  *packed_buf_nwords = poswords_to;

  return packed_buf;
}

