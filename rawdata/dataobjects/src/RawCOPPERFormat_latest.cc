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

int RawCOPPERFormat_latest::Get1stDetectorNwords(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}

int RawCOPPERFormat_latest::Get2ndDetectorNwords(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}

int RawCOPPERFormat_latest::Get3rdDetectorNwords(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}

int RawCOPPERFormat_latest::Get4thDetectorNwords(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}


int RawCOPPERFormat_latest::Get1stFINESSENwords(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}

int RawCOPPERFormat_latest::Get2ndFINESSENwords(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}

int RawCOPPERFormat_latest::Get3rdFINESSENwords(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}

int RawCOPPERFormat_latest::Get4thFINESSENwords(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}


int* RawCOPPERFormat_latest::Get1stDetectorBuffer(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return NULL;
}

int* RawCOPPERFormat_latest::Get2ndDetectorBuffer(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return NULL;
}

int* RawCOPPERFormat_latest::Get3rdDetectorBuffer(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return NULL;
}

int* RawCOPPERFormat_latest::Get4thDetectorBuffer(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return NULL;
}

int RawCOPPERFormat_latest::GetOffset1stFINESSE(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}


int RawCOPPERFormat_latest::GetOffset2ndFINESSE(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}


int RawCOPPERFormat_latest::GetOffset3rdFINESSE(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}

int RawCOPPERFormat_latest::GetOffset4thFINESSE(int n)
{
  char err_buf[500];
  sprintf(err_buf, "[FATAL] This function is not supported.(block %d) Exiting...: \n%s %s %d\n",
          n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
  printf("[DEBUG] %s\n", err_buf);
  B2FATAL(err_buf);
  return 0;
}

void RawCOPPERFormat_latest::CompareHeaderValue(int n, const unsigned int (&input_val)[MAX_PCIE40_CH] ,
                                                std::vector<std::vector<unsigned int>>& summary_table)
{

  //
  // Format of summary_table
  //  <The 1st channel of a value> <# of channels of the value> <the value>
  //
  std::vector<std::vector<unsigned int>> temp;
  for (int i = 0; i < MAX_PCIE40_CH; i++) {
    if (GetFINESSENwords(n, i) > 0) {

      int same_flag = 0;
      for (unsigned j = 0; j < summary_table.size(); ++j) {
        if (input_val[i] == summary_table.at(j).at(2)) {
          summary_table.at(j).at(1)++;
          same_flag = 1;
          break;
        }
      }
      if (same_flag == 0) {
        summary_table.push_back({ static_cast<unsigned int>(i), 1, input_val[i] });
      }
    }
  }
  sort(summary_table.begin(), summary_table.end(), [](const std::vector<unsigned int>& alpha,
  const std::vector<unsigned int>& beta) {return alpha.at(1) < beta.at(1);});
  return;
}
