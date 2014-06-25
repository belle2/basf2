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
//#define NO_DATA_CHECK
//#define WO_FIRST_EVENUM_CHECK

ClassImp(PostRawCOPPERFormat_latest);

PostRawCOPPERFormat_latest::PostRawCOPPERFormat_latest()
{
}

PostRawCOPPERFormat_latest::~PostRawCOPPERFormat_latest()
{
}




int PostRawCOPPERFormat_latest::GetBufferPos(int n)
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
    tmp_header.SetBuffer(&m_buffer[ pos_nwords ]);
    pos_nwords += tmp_header.GetNwords();

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
  sprintf(err_buf, "This function is not supported. Exiting...\n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
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
      sprintf(err_buf, "Invalid finesse # : %s %s %d\n",
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      string err_str = err_buf; throw (err_str);
      exit(1);
      break;
  }

  if (nwords < 0 || nwords > 1e6) {
    char err_buf[500];
    sprintf(err_buf, "# of words is strange. %d : %s %s %d\n", nwords,
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    string err_str = err_buf; throw (err_str);
  }

  return nwords;

}




unsigned int PostRawCOPPERFormat_latest::GetB2LFEE32bitEventNumber(int n)
{
  char err_buf[500];
  sprintf(err_buf, "No event # in B2LFEE header. Exiting...\n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf; throw (err_str);
  return 0;
}



void PostRawCOPPERFormat_latest::CheckData(int n,
                                           unsigned int prev_evenum, unsigned int* cur_evenum_rawcprhdr,
                                           unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                                           int prev_runsubrun_no, int* cur_runsubrun_no)
{
  char err_buf[500];
  int err_flag = 0;

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


  //
  // Check checksum calculated by DeSerializerCOPPER()
  //
  tmp_trailer.SetBuffer(GetRawTrlBufPtr(n));
  unsigned int xor_chksum = CalcXORChecksum(GetBuffer(n), GetBlockNwords(n) - tmp_trailer.GetTrlNwords());
  if (tmp_trailer.GetChksum() != xor_chksum) {
    sprintf(err_buf, "CORRUPTED DATA: PostRawCOPPERFormat_latest checksum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x\n %s %s %d\n",
            n, GetBlockNwords(n), *cur_evenum_rawcprhdr, tmp_trailer.GetChksum(), xor_chksum,
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }


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

bool PostRawCOPPERFormat_latest::CheckCOPPERMagic(int n)
{
  char err_buf[500];
  sprintf(err_buf, "No magic word # in COPPER header. Exiting...\n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf; throw (err_str);
  return false;
}

void PostRawCOPPERFormat_latest::CheckUtimeCtimeTRGType(int n)
{
  char err_buf[500];
  sprintf(err_buf, "This function is not supported. Exiting...\n%s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf; throw (err_str);
  return;
}



double PostRawCOPPERFormat_latest::GetEventUnixTime(int n)
{
  tmp_header.SetBuffer(GetBuffer(n));
  return (double)(tmp_header.GetTTUtime());

}


unsigned int PostRawCOPPERFormat_latest::FillTopBlockRawHeader(unsigned int m_node_id, unsigned int m_data_type,
    unsigned int m_trunc_mask, unsigned int prev_eve32,
    int prev_runsubrun_no, int* cur_runsubrun_no)
{
  char err_buf[500];
  sprintf(err_buf, "This function should be called by PrePostRawCOPPERFormat_latest. Exiting...\n %s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf; throw (err_str);
  return -1;

}


int PostRawCOPPERFormat_latest::CheckB2LHSLBMagicWords(int* finesse_buf, int finesse_nwords)
{
  char err_buf[500];
  sprintf(err_buf, "This function should be called by PrePostRawCOPPERFormat_latest. Exiting...\n %s %s %d\n",
          __FILE__, __PRETTY_FUNCTION__, __LINE__);
  string err_str = err_buf; throw (err_str);
  return -1;

}

int PostRawCOPPERFormat_latest::CheckCRC16(int n, int finesse_num)
{
  unsigned short temp_crc16 = CalcCRC16LittleEndian(0xffff, &(m_buffer[ tmp_header.POS_TTCTIME_TRGTYPE ]), 1);
  temp_crc16 = CalcCRC16LittleEndian(temp_crc16, &(m_buffer[ tmp_header.POS_EVE_NO ]), 1);
  temp_crc16 = CalcCRC16LittleEndian(temp_crc16, &(m_buffer[ tmp_header.POS_TTUTIME ]), 1);
  temp_crc16 = CalcCRC16LittleEndian(temp_crc16, &(m_buffer[ tmp_header.POS_EXP_RUN_NO ]), 1);
  int* buf = GetFINESSEBuffer(n, finesse_num) +  SIZE_B2LHSLB_HEADER + POS_B2L_CTIME;
  int pos_nwords = GetFINESSENwords(n, finesse_num) - (SIZE_B2LHSLB_HEADER + POS_B2L_CTIME + SIZE_B2LFEE_TRAILER + SIZE_B2LHSLB_TRAILER);
  temp_crc16 = CalcCRC16LittleEndian(temp_crc16, buf, pos_nwords);

  buf = GetFINESSEBuffer(n, finesse_num) +  GetFINESSENwords(n, finesse_num) - ((SIZE_B2LFEE_TRAILER - POS_B2LFEE_CRC16) + SIZE_B2LHSLB_TRAILER) ;

  //
  // CRC16 CCIT MSB error
  //
  if ((unsigned short)(*buf & 0xFFFF) != temp_crc16) {
    printf("CRC16 error %x %x %d\n", *buf , temp_crc16, GetFINESSENwords(n, finesse_num));
    printf("\n");
    buf = GetFINESSEBuffer(n, finesse_num);
    printf("POINTER %p\n", buf);
    printf("%.8x ", 0);
    for (int k = 0; k <  GetFINESSENwords(n, finesse_num); k++) {
      printf("%.8x ", buf[ k ]);
      if ((k + 1) % 10 == 0) printf("\n%.8x : ", k);
    }
    printf("\n");
    exit(1);
  } else {
    return 1;
  }

  return -1;

}
