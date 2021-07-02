/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RawCOPPERFormat_v1.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/PostRawCOPPERFormat_v1.h>


using namespace std;
using namespace Belle2;

//#define DESY
//#define NO_DATA_CHECK
//#define WO_FIRST_EVENUM_CHECK

//ClassImp(PostRawCOPPERFormat_v1);

PostRawCOPPERFormat_v1::PostRawCOPPERFormat_v1()
{
}

PostRawCOPPERFormat_v1::~PostRawCOPPERFormat_v1()
{
}

// int PostRawCOPPERFormat_v1::GetBufferPos(int n)
// {
//   if (m_buffer == NULL || m_nwords <= 0) {
//     char err_buf[500];
//     sprintf(err_buf, "[DEBUG] [ERROR] RawPacket buffer(%p) is not available or length(%d) is not set.\n %s %s %d\n",
//             m_buffer, m_nwords, __FILE__, __PRETTY_FUNCTION__, __LINE__);
//     string err_str = err_buf;     throw (err_str);
//   }

//   if (n >= (m_num_events * m_num_nodes)) {
//     char err_buf[500];
//     sprintf(err_buf, "[DEBUG] Invalid COPPER block No. (%d : max %d ) is specified. Exiting... \n %s %s %d\n",
//             n, (m_num_events * m_num_nodes), __FILE__, __PRETTY_FUNCTION__, __LINE__);
//     string err_str = err_buf;     throw (err_str);
//   }
//   int pos_nwords = 0;


//   for (int i = 1; i <= n ; i++) {
//     tmp_header.SetBuffer(&m_buffer[ pos_nwords ]);
//     pos_nwords += tmp_header.GetNwords();

//     if (pos_nwords >= m_nwords) {
//       char err_buf[500];
//       sprintf(err_buf, "CORRUPTED DATA: value of pos_nwords(%d) is larger than m_nwords(%d). Exiting...\n %s %s %d\n",
//               pos_nwords, m_nwords, __FILE__, __PRETTY_FUNCTION__, __LINE__);
//       string err_str = err_buf;     throw (err_str);
//       //      exit(1);
//     }
//   }
//   return pos_nwords;
// }


int PostRawCOPPERFormat_v1::GetDetectorNwords(int n, int finesse_num)
{

  int nwords = 0;
  if (GetFINESSENwords(n, finesse_num) > 0) {
    nwords = GetFINESSENwords(n, finesse_num)
             - (SIZE_B2LHSLB_HEADER + SIZE_B2LHSLB_TRAILER +  SIZE_B2LFEE_HEADER + SIZE_B2LFEE_TRAILER);
  }
  return nwords;

}


unsigned int PostRawCOPPERFormat_v1::CalcDriverChkSum(int n)
{
  char err_buf[500];
  sprintf(err_buf, "This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return 0;
}


int PostRawCOPPERFormat_v1::GetFINESSENwords(int n, int finesse_num)
{
  int pos_nwords_0, pos_nwords_1;
  int nwords = 0;
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
      sprintf(err_buf, "Invalid finesse # : %s %s %d\n",
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("%s", err_buf); fflush(stdout);
      B2FATAL(err_buf);
  }

  if (nwords < 0 || nwords > 1e6) {
    char err_buf[500];
    sprintf(err_buf, "# of words is strange. %d : %s %s %d\n", nwords,
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }

  return nwords;

}




unsigned int PostRawCOPPERFormat_v1::GetB2LFEE32bitEventNumber(int n)
{
  char err_buf[500];
  sprintf(err_buf, "No event # in B2LFEE header. (block %d) Exiting...\n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return 0;
}



void PostRawCOPPERFormat_v1::CheckData(int n,
                                       unsigned int prev_evenum, unsigned int* cur_evenum_rawcprhdr,
                                       unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                                       unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)
{

  char err_buf[500];
  sprintf(err_buf,
          "This function for format ver.1 is not supported. (n %d preveve %u eve %u  prectr %u ctr %u prevrun %u run %u) Exiting...\n %s %s %d\n",
          n, prev_evenum, *cur_evenum_rawcprhdr, prev_copper_ctr, *cur_copper_ctr,
          prev_exprunsubrun_no, *cur_exprunsubrun_no,
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s", err_buf); fflush(stdout);
  B2FATAL(err_buf);

//   char err_buf[500];
//   int err_flag = 0;
//   //
//   // Check incrementation of event #
//   //
//   *cur_evenum_rawcprhdr = GetEveNo(n);
//   *cur_exprunsubrun_no = GetExpRunSubrun(n);
//   if (
//       prev_exprunsubrun_no == *cur_exprunsubrun_no
// #ifdef WO_FIRST_EVENUM_CHECK
//      && prev_evenum != 0xFFFFFFFF && *cur_evenum_rawcprhdr != 0
// #endif
//   ) {
//     if ((unsigned int)(prev_evenum + 1) != *cur_evenum_rawcprhdr) {
//       sprintf(err_buf, "CORRUPTED DATA: Event # jump : i %d prev 0x%x cur 0x%x : Exiting...\n%s %s %d\n",
//               n, prev_evenum, *cur_evenum_rawcprhdr,
//               __FILE__, __PRETTY_FUNCTION__, __LINE__);
//       err_flag = 1;
//     }
//   }


//   //
//   // Check checksum calculated by DeSerializerCOPPER()
//   //
//   tmp_trailer.SetBuffer(GetRawTrlBufPtr(n));
//   unsigned int xor_chksum = CalcXORChecksum(GetBuffer(n), GetBlockNwords(n) - tmp_trailer.GetTrlNwords());
//   if (tmp_trailer.GetChksum() != xor_chksum) {
//     sprintf(err_buf,
//             "CORRUPTED DATA: checksum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x\n %s %s %d\n",
//             n, GetBlockNwords(n), *cur_evenum_rawcprhdr, tmp_trailer.GetChksum(), xor_chksum,
//             __FILE__, __PRETTY_FUNCTION__, __LINE__);
//     err_flag = 1;
//   }


//   if (err_flag == 1) {
//     printf("[DEBUG] ========== dump a data blcok : block # %d==========\n", n);
//     PrintData(GetBuffer(n), GetBlockNwords(n));
//     printf("Print out variables to reduce unused-variables-warnings : %u %u\n", prev_copper_ctr, *cur_copper_ctr);
//     string err_str = err_buf;
//     throw (err_str);

//     //     sleep(1234567);
//     //     exit(-1);
//   }

  return;

}

bool PostRawCOPPERFormat_v1::CheckCOPPERMagic(int n)
{
  char err_buf[500];
  sprintf(err_buf, "No magic word # in COPPER header (block %d). Exiting...\n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s", err_buf); fflush(stdout);
  B2FATAL(err_buf);
  return false;
}

void PostRawCOPPERFormat_v1::CheckUtimeCtimeTRGType(int n)
{
  char err_buf[500];
  sprintf(err_buf, "This function is not supported (block %d). Exiting...\n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("%s", err_buf); fflush(stdout);
  B2FATAL(err_buf);
}


unsigned int PostRawCOPPERFormat_v1::FillTopBlockRawHeader(unsigned int m_node_id, unsigned int prev_eve32,
                                                           unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)
{
  char err_buf[500];
  sprintf(err_buf, "This function should be called by PrePostRawCOPPERFormat_***. Exiting...\n %s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("Print out variables to reduce unused-variables-warnings : %u %u %u %u\n",
         m_node_id,  prev_eve32, prev_exprunsubrun_no, *cur_exprunsubrun_no);
  printf("%s", err_buf); fflush(stdout);
  B2FATAL(err_buf);
}


int PostRawCOPPERFormat_v1::CheckB2LHSLBMagicWords(int* finesse_buf, int finesse_nwords)
{
  char err_buf[500];
  sprintf(err_buf, "This function should be called by PrePostRawCOPPERFormat_***. Exiting...\n %s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("Print out variables to reduce unused-variables-warnings : %p %d\n", finesse_buf, finesse_nwords);
  printf("%s", err_buf); fflush(stdout);
  B2FATAL(err_buf);
}

int PostRawCOPPERFormat_v1::CheckCRC16(int n, int finesse_num)
{

  //
  // Calculate CRC16
  //
  int finesse_nwords = GetFINESSENwords(n, finesse_num);
  if (finesse_nwords <= 0) {
    char err_buf[500];
    sprintf(err_buf, "The specified finesse(%d) seems to be empty(nwords = %d). Cannot calculate CRC16. Exiting...\n %s %s %d\n",
            finesse_num, finesse_nwords, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    B2FATAL(err_buf);
  }
  unsigned short temp_crc16 = CalcCRC16LittleEndian(0xffff, &(m_buffer[ tmp_header.POS_TTCTIME_TRGTYPE ]), 1);
  temp_crc16 = CalcCRC16LittleEndian(temp_crc16, &(m_buffer[ tmp_header.POS_EVE_NO ]), 1);
  temp_crc16 = CalcCRC16LittleEndian(temp_crc16, &(m_buffer[ tmp_header.POS_TTUTIME ]), 1);
  temp_crc16 = CalcCRC16LittleEndian(temp_crc16, &(m_buffer[ tmp_header.POS_EXP_RUN_NO ]), 1);
  int* buf = GetFINESSEBuffer(n, finesse_num) +  SIZE_B2LHSLB_HEADER + POS_B2L_CTIME;
  int pos_nwords = finesse_nwords - (SIZE_B2LHSLB_HEADER + POS_B2L_CTIME + SIZE_B2LFEE_TRAILER + SIZE_B2LHSLB_TRAILER);
  temp_crc16 = CalcCRC16LittleEndian(temp_crc16, buf, pos_nwords);

  //
  // Compare CRC16 with B2LCRC16
  //
  buf = GetFINESSEBuffer(n, finesse_num) +  GetFINESSENwords(n,
                                                             finesse_num) - ((SIZE_B2LFEE_TRAILER - POS_B2LFEE_CRC16) + SIZE_B2LHSLB_TRAILER) ;

  if (GetEveNo(n) % 10000 == 0) {
    printf("#### PostRawCOPPER : Eve %.8x block %d finesse %d B2LCRC16 %.8x calculated CRC16 %.8x\n", GetEveNo(n), n, finesse_num,
           *buf, temp_crc16);
  }
  if ((unsigned short)(*buf & 0xFFFF) != temp_crc16) {
    //  if ( false ) {
    PrintData(m_buffer, m_nwords);
    printf("POST CRC16 error %x %x %d\n", *buf , temp_crc16, GetFINESSENwords(n, finesse_num));
    printf("\n");
    int* temp_buf = GetFINESSEBuffer(n, finesse_num);
    printf("%.8x ", 0);
    for (int k = 0; k <  GetFINESSENwords(n, finesse_num); k++) {
      printf("%.8x ", temp_buf[ k ]);
      if ((k + 1) % 10 == 0) printf("\n%.8x : ", k);
    }
    printf("\n");
    fflush(stdout);
    char err_buf[500];
    sprintf(err_buf,
            "[DEBUG] [ERROR] B2LCRC16 (%.4x) differs from one ( %.4x) calculated by PostRawCOPPERfromat class. Exiting...\n %s %s %d\n",
            (unsigned short)(*buf & 0xFFFF), temp_crc16, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    B2FATAL(err_buf);
  }

  return 1;


}


int* PostRawCOPPERFormat_v1::PackDetectorBuf(int* packed_buf_nwords,
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
    packed_buf[ poswords_to + POS_B2LFEE_CRC16 ] = ((0xffff & rawcpr_info.eve_num) << 16) | (crc16 & 0xffff);
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

