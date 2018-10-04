//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : trgeclUnpackerModule.cc
// Section  : TRG ECL
// Owner    : SungHyun Kim
// Email    : sungnhyun.kim@belle2.org
//---------------------------------------------------------------
// Description : TRG ECL Unpacker Module
//---------------------------------------------------------------
// 1.00 : 2017/05/06 : First version
// 1.01 : 2017/07/17 : Add FTSW clock from FAM, fine timing (ETM Ver. old_89)
// 2.00 : 2018/02/17 : 8 window data (ETM Ver. 100)
//---------------------------------------------------------------

#include <trg/ecl/modules/trgeclUnpacker/trgeclUnpackerModule.h>

using namespace std;
using namespace Belle2;

//! Register Module
REG_MODULE(TRGECLUnpacker);

string TRGECLUnpackerModule::version() const
{
  return string("2.00");
}

TRGECLUnpackerModule::TRGECLUnpackerModule()
  : Module::Module(),
    n_basf2evt(0)
{

  string desc = "TRGECLUnpackerModule(" + version() + ")";
  setDescription(desc);
  setPropertyFlags(c_ParallelProcessingCertified);
  B2INFO("trgeclunpacker: Constructor done.");
}

TRGECLUnpackerModule::~TRGECLUnpackerModule()
{
}

void TRGECLUnpackerModule::terminate()
{

  cout << "total TRG ECL events : " << n_basf2evt << endl;
}

void TRGECLUnpackerModule::initialize()
{

  m_TRGECLUnpackerArray.registerInDataStore();
}

void TRGECLUnpackerModule::beginRun()
{
}

void TRGECLUnpackerModule::endRun()
{
}

void TRGECLUnpackerModule::event()
{

  StoreArray<RawTRG> raw_trgarray;
  unsigned int nodeid;
  int nwords;

  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      nodeid = ((raw_trgarray[i]->GetNodeID(j)) >> 24) & 0x1F;
      nwords = raw_trgarray[i]->GetDetectorNwords(j, 0);
      if (nodeid == 0x13) {
        readCOPPEREvent(raw_trgarray[i], j, nwords);
        n_basf2evt++;
        if (n_basf2evt % 1000 == 0) {
          printf("%.5dK", (int)n_basf2evt / 1000);
          fflush(stdout);
          printf("\r");
        }
      }
    }
  }
}

void TRGECLUnpackerModule::readCOPPEREvent(RawTRG* raw_copper, int i, int nwords)
{

  if (raw_copper->GetDetectorNwords(i, 0) > 0) {
    checkBuffer(raw_copper->GetDetectorBuffer(i, 0), nwords);
  }
}

void TRGECLUnpackerModule::checkBuffer(int* rdat, int nwords)
{

  unsigned char check_sum = (rdat[nwords - 1] >> 8) & 0xFF;

  unsigned char sum_data  = 0; // checksum
  unsigned char summary_data[4] = {0};
  vector<int> tc_info;
  vector<vector<int>> evt_info;

  int i      = 0;
  int window = 0;
  int w_ntc  = 0;
  int w_revo = 0;
  int tc_id  = 0;
  int tc_t   = 0;
  int tc_e   = 0;
  int conv_tc_t = 0;
  int flag_checksum = 0;
  int ntc     = 0;
  int gdl_cnt = 0;
  int hit_win = 0;
  // TRG timing = window 3 (TEMP)

  // cout << nwords << endl;
  // cout << "==========" << endl;

  tc_info.clear();
  evt_info.clear();

  gdl_cnt = (rdat[0] >> 16) & 0xffff;
  sum_data = ((gdl_cnt >> 8) & 0xFF) + (gdl_cnt & 0xff);

  while (i < nwords - 1) {
    summary_data[0] = (rdat[i] >> 8) & 0xff;
    summary_data[1] =  rdat[i] & 0xff;
    summary_data[2] = (rdat[i + 1] >> 24) & 0xff;
    summary_data[3] = (rdat[i + 1] >> 16) & 0xff;

    for (int j = 0; j < 4; j++) {
      sum_data = sum_data + summary_data[j];
      // if(n_basf2evt == 3775){
      //  cout << "= " << i << " =" << endl;
      //  printf("%x\n", sum_data);
      // }

    }
    w_ntc  = (summary_data[2] << 8) + summary_data[3];
    ntc    = ntc + w_ntc;
    if (window == 3) {
      w_revo = summary_data[1];
    }
    // cout << "window = " << window << endl;
    // cout << "ntc = " << w_ntc  << endl;
    if (w_ntc == 0) {
      i++;
      window++;
    } else {
      for (int j = 0; j < w_ntc; j++) {
        tc_id     = (rdat[i + 1 + j] >> 3) & 0x3FF;
        tc_t      = ((rdat[i + 1 + j] & 0x7) << 4) + ((rdat[i + 2 + j] >> 28) & 0xF);
        conv_tc_t = (window - 3) * 128 + tc_t;
        tc_e      = (rdat[i + 2 + j] >> 16) & 0xFFF;
        hit_win   = window;
        sum_data = sum_data +
                   ((rdat[i + 1 + j] >>  8) & 0xff) +
                   ((rdat[i + 1 + j]) & 0xff) +
                   ((rdat[i + 2 + j] >> 24) & 0xff) +
                   ((rdat[i + 2 + j] >> 16) & 0xff);
        // if(n_basf2evt == 3775){
        //  cout << "= " << i << " =" << endl;
        //  printf("%x\n", sum_data);
        // }

        tc_info.push_back(tc_id);
        tc_info.push_back(conv_tc_t);
        tc_info.push_back(tc_e);
        tc_info.push_back(hit_win);
        evt_info.push_back(tc_info);

        tc_info.clear();
        //    cout << tc_id << " " << tc_t << " " << tc_e << endl;
      }
      i = i + w_ntc + 1;
      window++;
    }
    //  cout << "i = " << i << endl;
    // if(i == nwords-1){ // last window
    //     break;
    // }

  }


  if (check_sum == sum_data) {
    flag_checksum = 0;
  } else {
    flag_checksum = 1;
  }

  // if(n_basf2evt == 3775){
  //  printf("%x %x %d\n", check_sum, sum_data, flag_checksum);
  // }

  int evt_size   = evt_info.size();
  int evt_timing = -9999;
  if (evt_size != 0 && flag_checksum == 0 && nwords > 7) {
    // Find most energetic TC timing
    sort(evt_info.begin(), evt_info.end(),
    [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[2] > aa2[2];});

    evt_timing = evt_info[0][1];

    // Sort by TC number
    sort(evt_info.begin(), evt_info.end(),
    [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[0] < aa2[0];});

    // printf("%.4x\n",check_sum);
    // printf("%.4x\n",sum_data);

    m_gdl   = gdl_cnt;
    m_revo  = w_revo;
    m_ntc   = ntc;
    m_evttime = evt_timing;
    m_checksum = flag_checksum;
    for (int ii = 0; ii < evt_size; ii++) {
      m_tcid   = evt_info[ii][0];
      m_time   = evt_info[ii][1];
      m_energy = evt_info[ii][2];
      m_hitwin = evt_info[ii][3];
      m_caltime = evt_timing - m_time;

      StoreArray<TRGECLUnpackerStore> TRGECLUnpackerArray;
      TRGECLUnpackerArray.appendNew();
      m_hitNum = TRGECLUnpackerArray.getEntries() - 1;
      TRGECLUnpackerArray[m_hitNum]->setEventId(n_basf2evt);
      TRGECLUnpackerArray[m_hitNum]->setTCId(m_tcid);
      TRGECLUnpackerArray[m_hitNum]->setNTC(m_ntc);
      TRGECLUnpackerArray[m_hitNum]->setTCEnergy(m_energy);
      TRGECLUnpackerArray[m_hitNum]->setTCTime(m_time);
      TRGECLUnpackerArray[m_hitNum]->setTCCALTime(m_caltime);
      TRGECLUnpackerArray[m_hitNum]->setEVTTime(m_evttime);
      TRGECLUnpackerArray[m_hitNum]->setRevoFAM(m_revo);
      TRGECLUnpackerArray[m_hitNum]->setRevoGDL(m_gdl);
      TRGECLUnpackerArray[m_hitNum]->setHitWin(m_hitwin);
      TRGECLUnpackerArray[m_hitNum]->setChecksum(m_checksum);
    }
  } else {
    m_tcid     = 0;
    if (ntc == 0) {
      m_ntc = 0;
    } else {
      m_ntc = -1;
    }

    m_energy   = 0;
    m_hitwin   = -1;
    m_time     = -9999;
    m_caltime  = -9999;
    m_evttime  = -9999;
    m_revo     = w_revo;
    m_gdl      = gdl_cnt;
    if (flag_checksum == 1) {
      m_checksum = 1;
    } else {
      m_checksum = 2;
    }

    StoreArray<TRGECLUnpackerStore> TRGECLUnpackerArray;
    TRGECLUnpackerArray.appendNew();
    m_hitNum = TRGECLUnpackerArray.getEntries() - 1;
    TRGECLUnpackerArray[m_hitNum]->setEventId(n_basf2evt);
    TRGECLUnpackerArray[m_hitNum]->setTCId(m_tcid);
    TRGECLUnpackerArray[m_hitNum]->setNTC(m_ntc);
    TRGECLUnpackerArray[m_hitNum]->setTCEnergy(m_energy);
    TRGECLUnpackerArray[m_hitNum]->setTCTime(m_time);
    TRGECLUnpackerArray[m_hitNum]->setTCCALTime(m_caltime);
    TRGECLUnpackerArray[m_hitNum]->setEVTTime(m_evttime);
    TRGECLUnpackerArray[m_hitNum]->setRevoFAM(m_revo);
    TRGECLUnpackerArray[m_hitNum]->setRevoGDL(m_gdl);
    TRGECLUnpackerArray[m_hitNum]->setHitWin(m_hitwin);
    TRGECLUnpackerArray[m_hitNum]->setChecksum(m_checksum);

  }

  return;
}
