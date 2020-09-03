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

//ClassImp(RawCOPPERFormat_latest);

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

int* RawCOPPERFormat_latest::GetFINESSEBuffer(int n, int finesse_num)
{
  if (finesse_num >= MAX_PCIE40_CH || finesse_num < 0) {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] Invalid finesse # : %s %s %d\n",
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] %s\n", err_buf);
    B2FATAL(err_buf);
    return NULL;
  }
  int cur_pos = GetBufferPos(n) + (tmp_header.POS_CH_POS_TABLE + finesse_num);
  return (m_buffer + cur_pos);
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
        printf("[DEBUG] Or if you are going to read data taken before the update, please use basf2 software before svn rev. 7419\n");
        printf("[DEBUG] About the format please see Nakao-san's B2GM slides(p. 13 and 15) http://kds.kek.jp/getFile.py/access?contribId=143&sessionId=38&resId=0&materialId=slides&confId=13911.\n");
        printf("[DEBUG] Sorry for inconvenience.\n");
        printf("[DEBUG] \033[0m");
        fflush(stderr);
        char err_buf[500];
        sprintf(err_buf, "FTSW and b2tt firmwares are old. Exiting...\n %s %s %d\n",
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        printf("%s\n", err_buf); fflush(stdout);
        B2FATAL(err_buf);
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
      printf("%s\n", err_buf); fflush(stdout);
      B2FATAL(err_buf);
#endif
    }
  }
  return;
}
#endif
