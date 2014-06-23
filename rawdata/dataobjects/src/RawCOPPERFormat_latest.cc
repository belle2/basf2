//+
// File : RawCOPPERFormat_latest.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawCOPPERFormat_latest.h>


using namespace std;
using namespace Belle2;

//#define DESY
//#define NO_DATA_CHECK
//#define WO_FIRST_EVENUM_CHECK

ClassImp(RawCOPPERFormat_latest);

RawCOPPERFormat_latest::RawCOPPERFormat_latest()
{
}

RawCOPPERFormat_latest::~RawCOPPERFormat_latest()
{
}

int RawCOPPERFormat_latest::GetNumFINESSEBlock(int n)
{
  int cnt = 0;
  for (int i = 0; i < 4; i++) {
    if (GetFINESSENwords(n, i) > 0) {
      cnt++;
    }
  }
  return cnt;
}





int RawCOPPERFormat_latest::GetOffsetFINESSE(int n, int finesse_num)
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
//   sleep(12345678);
//   exit(-1);
//   return 0;

}




int* RawCOPPERFormat_latest::GetFINESSEBuffer(int n, int finesse_num)
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
//   sleep(12345678);
//   exit(-1);
//   return 0;
}


int* RawCOPPERFormat_latest::GetDetectorBuffer(int n, int finesse_num)
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
//   sleep(12345678);
//   exit(-1);
//   return 0;
}



unsigned int  RawCOPPERFormat_latest::CalcXORChecksum(int* buf, int nwords)
{
  unsigned int checksum = 0;
  for (int i = 0; i < nwords; i++) {
    checksum = checksum ^ buf[ i ];
  }
  return checksum;
}


void RawCOPPERFormat_latest::CheckData(int n,
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
    sprintf(err_buf, "CORRUPTED DATA: Event # in RawCOPPERFormat_latest header and FEE header is different : cprhdr 0x%x feehdr 0x%x : Exiting...\n%s %s %d\n",
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
    sprintf(err_buf, "CORRUPTED DATA: RawCOPPERFormat_latest checksum error : block %d : length %d eve 0x%x : Trailer chksum 0x%.8x : calcd. now 0x%.8x\n %s %s %d\n",
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


unsigned int RawCOPPERFormat_latest::GetB2LHeaderWord(int n, int finesse_buffer_pos)
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
//     sleep(12345678);
//     exit(-1);
  }


  if (err_flag == 1) {

    char err_buf[500];
    sprintf(err_buf, "CORRUPTED DATA: Different event number over HSLBs : slot A 0x%x : B 0x%x :C 0x%x : D 0x%x\n %s %s %d\n",
            word[ 0 ], word[ 1 ], word[ 2 ], word[ 3 ],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] [ERROR] %s\n", err_buf);
#ifndef NO_DATA_CHECK
    string err_str = err_buf; throw (err_str);

//     sleep(12345678);
//     exit(-1);
#endif
  }

  return ret_word;

}



#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
void RawCOPPERFormat_latest::CheckB2LFEEHeaderVersion(int n)
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
      sprintf(err_buf, "RawCOPPERFormat_latest contains no FINESSE data. Exiting...\n %s %s %d\n",
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

