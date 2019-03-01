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
// 3.00 : 2018/07/31 : ETM version dependence included
// 3.01 : 2019/02/25 : Trigger bit modify
//---------------------------------------------------------------

#include <trg/ecl/modules/trgeclUnpacker/trgeclUnpackerModule.h>

using namespace std;
using namespace Belle2;

//! Register Module
REG_MODULE(TRGECLUnpacker);

string TRGECLUnpackerModule::version() const
{
  return string("3.01");
}

TRGECLUnpackerModule::TRGECLUnpackerModule()
  : Module::Module(), n_basf2evt(0)
{

  string desc = "TRGECLUnpackerModule(" + version() + ")";
  setDescription(desc);
  setPropertyFlags(c_ParallelProcessingCertified);
  B2INFO("trgeclunpacker: Constructor done.");
}

TRGECLUnpackerModule::~TRGECLUnpackerModule() {}

void TRGECLUnpackerModule::terminate()
{

  // Debug
  // cout << "total TRG ECL events : " << n_basf2evt << endl;
}

void TRGECLUnpackerModule::initialize()
{
  m_TRGECLSumArray.registerInDataStore();
  m_TRGECLTCArray.registerInDataStore();
  m_TRGECLEvtArray.registerInDataStore();
}

void TRGECLUnpackerModule::beginRun() {}
void TRGECLUnpackerModule::endRun() {}
void TRGECLUnpackerModule::event()
{

  StoreArray<RawTRG> raw_trgarray;

  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    iFiness = i;
    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      nodeid = ((raw_trgarray[i]->GetNodeID(j)) >> 24) & 0x1F;
      nwords = raw_trgarray[i]->GetDetectorNwords(j, 0);
      if (nodeid == 0x13) {
        if (nwords < 9) {
          B2ERROR("Consistecy error in unpacker.");
          B2ERROR("data length " << nwords << " nWord " << nwords);
          B2ERROR("Node ID " << nodeid << ", Finness ID " << iFiness);
          continue;
        }
        readCOPPEREvent(raw_trgarray[i], j, nwords);
        n_basf2evt++;
      }
    }
  }
}

void TRGECLUnpackerModule::readCOPPEREvent(RawTRG* raw_copper, int i, int nnn)
{

  if (raw_copper->GetDetectorNwords(i, 0) > 0) {
    checkBuffer(raw_copper->GetDetectorBuffer(i, 0), nnn);
  }
}

void TRGECLUnpackerModule::checkBuffer(int* rdat, int nnn)
{

  int version_check = (rdat[0] >> 12) & 0xf;
  etm_version       = (rdat[0] >> 16) & 0xffff;
  if (version_check == 15) {
    if (etm_version >= 115) {
      checkBuffer_115(rdat, nnn);
    }
  } else {
    checkBuffer_114(rdat, nnn);
  }

}

void TRGECLUnpackerModule::checkBuffer_115(int* rdat, int nnn)
{

  // Checksum variable
  unsigned char check_sum = (rdat[nnn - 1] >> 24) & 0xFF;
  unsigned char data_sum  = 0;
  unsigned char kdat[4]   = {0};
  for (int j = nnn - 2; j > -1; j--) {
    kdat[0] =  rdat[j]        & 0xff;
    kdat[1] = (rdat[j] >>  8) & 0xff;
    kdat[2] = (rdat[j] >> 16) & 0xff;
    kdat[3] = (rdat[j] >> 24) & 0xff;
    for (int k = 0; k < 4; k++) {
      data_sum = data_sum + kdat[k];
    }
  }

  int flag_checksum = 0;

  if (check_sum == data_sum) {
    flag_checksum = 0;
  } else {
    flag_checksum = 1;
  }

  // Information
  int l1_revo     = rdat[0] & 0x7ff;
  int i           = 0;
  int window_num  = 0;

  // Summary
  int summary_data      = 0;
  int summary_revo      = 0;
  bool summary_trg      = false;

  int data_win          = 0;
  int evt_revo          = -9999;

  // TC
  int ntc_win           = 0;
  bool tc_trg           = false;
  // TC info
  int tc_id             = 0;
  int tc_t              = 0;
  int tc_e              = 0;
  int conv_tc_t         = 0;

  vector<int> sum_data;
  vector<vector<int>> sum_info;

  vector<int> tc_data;
  vector<vector<int>> tc_info;

  bool btrg    = false;
  int trg_win  = -1;
  int trg_revo = -1;

  // Unpacking ---->
  while (i < nnn - 2) {
    summary_data = rdat[i + 1];
    summary_trg  = (summary_data >> 23) & 0x1;
    summary_revo = (summary_data >> 16) & 0x7f;
    ntc_win      = summary_data & 0x3ff;
    if (ntc_win == 0) {
      tc_trg = false;
    } else {
      tc_trg = true;
    }
    data_win    = window_num;

    if (summary_trg == true) { // Summary on
      sum_data.push_back(data_win);
      sum_data.push_back(summary_revo);
      for (int j = 0; j < 9; j++) {
        sum_data.push_back(rdat[i + j + 2]);
      }
      sum_info.push_back(sum_data);
      sum_data.clear();
      i = i + 11;

      if (btrg == false) {
        trg_win  = data_win;
        trg_revo = summary_revo;
        btrg = true;
      }

      if (tc_trg == true) { // summary on & TC on
        for (int j = 0; j < ntc_win; j++) {
          tc_id     = (rdat[i + j] >> 20) & 0x3FF;
          tc_t      = (rdat[i + j] >> 12) & 0x7F;
          tc_e      = rdat[i + j] & 0xFFF;
          conv_tc_t = (data_win - 3) * 128 + tc_t;

          // TC vector
          tc_data.push_back(tc_id);
          tc_data.push_back(conv_tc_t);
          tc_data.push_back(tc_e);
          tc_data.push_back(data_win);
          tc_info.push_back(tc_data);
          tc_data.clear();
        }
        i = i + ntc_win - 1;
      }

    } else { // Summary off
      if (tc_trg == true) { // summary off & TC on
        for (int j = 0; j < ntc_win; j++) {
          tc_id     = (rdat[i + j + 2] >> 20) & 0x3FF;
          tc_t      = (rdat[i + j + 2] >> 12) & 0x7F;
          conv_tc_t = (data_win - 3) * 128 + tc_t;
          tc_e      = rdat[i + j + 2] & 0xFFF;

          // TC vector
          tc_data.push_back(tc_id);
          tc_data.push_back(conv_tc_t);
          tc_data.push_back(tc_e);
          tc_data.push_back(data_win);
          tc_info.push_back(tc_data);
          tc_data.clear();
        }
        i = i + ntc_win + 1;
      } else { // Summary off & TC off
        i = i + 1;
      }
    }
    window_num++;
  }

  vector<int> vwin_revo;
  vwin_revo.clear();
  int win_revo = -1;
  for (i = 0; i < 8; i++) {
    if (btrg == true) {
      win_revo = trg_revo + i - trg_win;
      if (win_revo < 0) win_revo += 80;
      vwin_revo.push_back(win_revo);
    } else {
      vwin_revo.push_back(-1);
    }
    if (i == 3) evt_revo = win_revo;
  }

  // <---- Unpacking

  // Summary
  int sum_num       = 0;
  int sum_revo      = 0;
  int cl_theta[6]   = {0};
  int cl_phi[6]     = {0};
  int cl_time[6]    = {0};
  int cl_energy[6]  = {0};
  int ncl           = 0;
  int low_multi     = 0;
  int b2bhabha_v    = 0;
  int b2bhabha_s    = 0;
  int mumu          = 0;
  int prescale      = 0;
  int icn_over      = 0;
  int bg_veto       = 0;
  int icn           = 0;
  int etot_type     = 0;
  int etot          = 0;
  int b1_type       = 0;
  int b1bhabha      = 0;
  int physics       = 0;
  int time_type     = 0;
  int time          = 0;

  int m_sumNum      = 0;
  int evt_b1bha     = 0;
  int evt_b2bha_v   = 0;
  int evt_b2bha_s   = 0;
  int evt_mumu      = 0;
  int evt_prescale  = 0;
  int evt_phy       = 0;

  // Store Summary
  int sum_size = sum_info.size();
  if (sum_size != 0) {
    for (int j = 0; j < sum_size; j++) {
      sum_num  = sum_info[j][0];
      sum_revo = sum_info[j][1];
      if (etm_version > 119) {
        cl_theta[5]     = (sum_info[j][2] >> 19) & 0x7f;
        cl_phi[5]       = (sum_info[j][2] >> 11) & 0xff;
        cl_time[5]      = (sum_info[j][2] >>  3) & 0xff;
        cl_energy[5]    = ((sum_info[j][2] & 0x7) << 9) + ((sum_info[j][3] >> 23) & 0x1ff);

        cl_theta[4]      = (sum_info[j][3] >> 16) & 0x7f;
        cl_phi[4]        = (sum_info[j][3] >>  8) & 0xff;
        cl_time[4]       = (sum_info[j][3]) & 0xff;
        cl_energy[4]     = (sum_info[j][4] >> 20) & 0xfff;

        cl_theta[3]      = (sum_info[j][4] >> 13) & 0x7f;
        cl_phi[3]        = (sum_info[j][4] >>  5) & 0xff;
        cl_time[3]       = ((sum_info[j][4] & 0x1f) << 3) + ((sum_info[j][5] >> 29) & 0x7);
        cl_energy[3]     = (sum_info[j][4] >>  17) & 0xfff;

        cl_theta[2]      = (sum_info[j][5] >> 10) & 0x7f;
        cl_phi[2]        = (sum_info[j][5] >>  2) & 0xff;
        cl_time[2]       = ((sum_info[j][5] & 0x3)  << 6) + ((sum_info[j][6] >> 26) & 0x3f);
        cl_energy[2]     = (sum_info[j][6] >> 14) & 0xfff;

        cl_theta[1]      = (sum_info[j][6] >>  7) & 0x7f;
        cl_phi[1]        = ((sum_info[j][6] & 0x7f) << 1) + ((sum_info[j][7] >> 31) & 0x1);
        cl_time[1]       = (sum_info[j][7] >> 23) & 0xff;
        cl_energy[1]     = (sum_info[j][7] >> 11) & 0xfff;

        cl_theta[0]      = (sum_info[j][7] >>  4) & 0x7f;
        cl_phi[0]        = ((sum_info[j][7] & 0xf)  << 4) + ((sum_info[j][8] >> 28) & 0xf);
        cl_time[0]       = (sum_info[j][8] >> 20) & 0xff;
        cl_energy[0]     = (sum_info[j][8] >>  8) & 0xfff;

        ncl              = (sum_info[j][8] >>  5) & 0x7;

        prescale         = (sum_info[j][8] >>  4) & 0x1;
        mumu             = (sum_info[j][8] >>  3) & 0x1;
        b2bhabha_s       = (sum_info[j][8] >>  2) & 0x1;
        low_multi        = ((sum_info[j][8] & 0x3) << 10) + ((sum_info[j][9] >> 22) & 0x3ff);
        b2bhabha_v       = (sum_info[j][9] >> 21) & 0x1;
        icn_over         = (sum_info[j][9] >> 20) & 0x1;
        bg_veto          = (sum_info[j][9] >> 17) & 0x7;
        icn              = (sum_info[j][9] >> 10) & 0x7f;
        etot_type        = (sum_info[j][9] >>  7) & 0x7;
        etot             = ((sum_info[j][9] & 0x7f) << 6) + ((sum_info[j][10] >> 26) & 0x3f);
        b1_type          = (sum_info[j][10] >> 12) & 0x3fff;
        b1bhabha         = (sum_info[j][10] >> 11) & 0x1;
        physics          = (sum_info[j][10] >> 10) & 0x1;
        time_type        = (sum_info[j][10] >>  7) & 0x7;
        time             = (sum_info[j][10]) & 0x7f;
      } else {
        cl_theta[5]      = (sum_info[j][2] >> 24) & 0x7f;
        cl_phi[5]        = (sum_info[j][2] >> 16) & 0xff;
        cl_time[5]       = (sum_info[j][2] >>  8) & 0xff;
        cl_energy[5]     = ((sum_info[j][2] & 0xff) << 4) + ((sum_info[j][3] >> 28) & 0xf);

        cl_theta[4]      = (sum_info[j][3] >> 21) & 0x7f;
        cl_phi[4]        = (sum_info[j][3] >> 13) & 0xff;
        cl_time[4]       = (sum_info[j][3] >>  5) & 0xff;
        cl_energy[4]     = ((sum_info[j][3] & 0x1f) << 7) + ((sum_info[j][4] >> 25) & 0x7f);

        cl_theta[3]      = (sum_info[j][4] >> 18) & 0x7f;
        cl_phi[3]        = (sum_info[j][4] >> 10) & 0xff;
        cl_time[3]       = (sum_info[j][4] >>  2) & 0xff;
        cl_energy[3]     = ((sum_info[j][4] & 0x3) << 10) + ((sum_info[j][5] >> 22) & 0x3ff);

        cl_theta[2]      = (sum_info[j][5] >> 15) & 0x7f;
        cl_phi[2]        = (sum_info[j][5] >>  7) & 0xff;
        cl_time[2]       = ((sum_info[j][5] & 0x7f)  << 1) + ((sum_info[j][6] >> 31) & 0x1);
        cl_energy[2]     = (sum_info[j][6] >> 19) & 0xfff;

        cl_theta[1]      = (sum_info[j][6] >> 12) & 0x7f;
        cl_phi[1]        = (sum_info[j][6] >>  4) & 0xff;
        cl_time[1]       = ((sum_info[j][6] & 0xf) << 4) + ((sum_info[j][7] >> 28) & 0xf);
        cl_energy[1]     = (sum_info[j][7] >> 16) & 0xfff;

        cl_theta[0]      = (sum_info[j][7] >>  9) & 0x7f;
        cl_phi[0]        = (sum_info[j][7] >>  1) & 0xff;
        cl_time[0]       = ((sum_info[j][7] & 0x1)  << 7) + ((sum_info[j][8] >> 25) & 0x7f);
        cl_energy[0]     = (sum_info[j][8] >> 13) & 0xfff;

        ncl              = (sum_info[j][8] >> 10) & 0x7;

        low_multi        = ((sum_info[j][8] & 0x3ff) << 2) + ((sum_info[j][9] >> 30) & 0x3);
        b2bhabha_v       = (sum_info[j][9] >> 29) & 0x1;
        icn_over         = (sum_info[j][9] >> 28) & 0x1;
        bg_veto          = (sum_info[j][9] >> 25) & 0x7;
        icn              = (sum_info[j][9] >> 18) & 0x7f;
        etot_type        = (sum_info[j][9] >> 15) & 0x7;
        etot             = (sum_info[j][9] >>  2) & 0x1fff;

        b1_type          = ((sum_info[j][9] & 0x3) << 12) + ((sum_info[j][10] >> 20) & 0xfff);
        b1bhabha         = (sum_info[j][10] >> 19) & 0x1;
        physics          = (sum_info[j][10] >> 18) & 0x1;
        time_type        = (sum_info[j][10] >> 15) & 0x7;
        time             = (sum_info[j][10] >>  8) & 0x7f;

        b2bhabha_s       = 0;
        mumu             = 0;
        prescale         = 0;
      }
      m_TRGECLSumArray.appendNew();
      m_sumNum = m_TRGECLSumArray.getEntries() - 1;
      m_TRGECLSumArray[m_sumNum]->setEventId(n_basf2evt);
      m_TRGECLSumArray[m_sumNum]->setSumNum(sum_num);
      m_TRGECLSumArray[m_sumNum]->setSumRevo(sum_revo);
      m_TRGECLSumArray[m_sumNum]->setCLTheta(cl_theta);
      m_TRGECLSumArray[m_sumNum]->setCLPhi(cl_phi);
      m_TRGECLSumArray[m_sumNum]->setCLTime(cl_time);
      m_TRGECLSumArray[m_sumNum]->setCLEnergy(cl_energy);
      m_TRGECLSumArray[m_sumNum]->setNCL(ncl);
      m_TRGECLSumArray[m_sumNum]->setICN(icn);
      m_TRGECLSumArray[m_sumNum]->setICNOver(icn_over);
      m_TRGECLSumArray[m_sumNum]->setLowMulti(low_multi);
      m_TRGECLSumArray[m_sumNum]->set3DBhabhaV(b2bhabha_v);
      m_TRGECLSumArray[m_sumNum]->set3DBhabhaS(b2bhabha_s);
      m_TRGECLSumArray[m_sumNum]->setMumu(mumu);
      m_TRGECLSumArray[m_sumNum]->setPrescale(prescale);
      m_TRGECLSumArray[m_sumNum]->set2DBhabha(b1bhabha);
      m_TRGECLSumArray[m_sumNum]->setBhabhaType(b1_type);
      m_TRGECLSumArray[m_sumNum]->setPhysics(physics);
      m_TRGECLSumArray[m_sumNum]->setBG(bg_veto);
      m_TRGECLSumArray[m_sumNum]->setEtot(etot);
      m_TRGECLSumArray[m_sumNum]->setEtotType(etot_type);
      m_TRGECLSumArray[m_sumNum]->setTime(time);
      m_TRGECLSumArray[m_sumNum]->setTimeType(time_type);

      evt_b1bha    = evt_b1bha    || b1bhabha;
      evt_b2bha_v  = evt_b2bha_v  || b2bhabha_v;
      evt_b2bha_s  = evt_b2bha_s  || b2bhabha_s;
      evt_mumu     = evt_mumu     || mumu;
      evt_prescale = evt_prescale || prescale;
      evt_phy      = evt_phy      || physics;
    }
  } else {
    memset(cl_theta,    0, sizeof(cl_theta));
    memset(cl_phi,      0, sizeof(cl_phi));
    memset(cl_time, -9999, sizeof(cl_time));
    memset(cl_energy,   0, sizeof(cl_energy));
    ncl        = 0;
    low_multi  = 0;
    b2bhabha_v = 0;
    b2bhabha_s = 0;
    mumu       = 0;
    prescale   = 0;
    icn_over   = 0;
    bg_veto    = 0;
    icn        = 0;
    etot_type  = 0;
    etot       = 0;
    b1_type    = 0;
    b1bhabha   = 0;
    physics    = 0;
    time_type  = 0;
    time       = -9999;

    m_TRGECLSumArray.appendNew();
    m_sumNum = m_TRGECLSumArray.getEntries() - 1;
    m_TRGECLSumArray[m_sumNum]->setEventId(n_basf2evt);
    m_TRGECLSumArray[m_sumNum]->setCLTheta(cl_theta);
    m_TRGECLSumArray[m_sumNum]->setCLPhi(cl_phi);
    m_TRGECLSumArray[m_sumNum]->setCLTime(cl_time);
    m_TRGECLSumArray[m_sumNum]->setCLEnergy(cl_energy);
    m_TRGECLSumArray[m_sumNum]->setNCL(ncl);
    m_TRGECLSumArray[m_sumNum]->setICN(icn);
    m_TRGECLSumArray[m_sumNum]->setICNOver(icn_over);
    m_TRGECLSumArray[m_sumNum]->setLowMulti(low_multi);
    m_TRGECLSumArray[m_sumNum]->set3DBhabhaV(b2bhabha_v);
    m_TRGECLSumArray[m_sumNum]->set3DBhabhaS(b2bhabha_s);
    m_TRGECLSumArray[m_sumNum]->setMumu(mumu);
    m_TRGECLSumArray[m_sumNum]->setPrescale(prescale);
    m_TRGECLSumArray[m_sumNum]->set2DBhabha(b1bhabha);
    m_TRGECLSumArray[m_sumNum]->setBhabhaType(b1_type);
    m_TRGECLSumArray[m_sumNum]->setPhysics(physics);
    m_TRGECLSumArray[m_sumNum]->setBG(bg_veto);
    m_TRGECLSumArray[m_sumNum]->setEtot(etot);
    m_TRGECLSumArray[m_sumNum]->setEtotType(etot_type);
    m_TRGECLSumArray[m_sumNum]->setTime(time);
    m_TRGECLSumArray[m_sumNum]->setTimeType(time_type);
  }

  // TC & TRG
  int m_evtNum   = 0;
  int evt_ntc    = tc_info.size();
  int evt_timing = -9999;
  int evt_win    = 0;
  int evt_etot   = 0;
  int evt_etot_p = 0;

  int m_tcNum    = 0;
  int m_tcid     = 0;
  int m_time     = -9999;
  int m_energy   = 0;
  int m_win      = 0;
  int m_revo     = 0;
  int m_caltime  = -9999;

  if (evt_ntc != 0 && flag_checksum == 0 && nnn > 7) {
    // Find most energetic TC timing
    sort(tc_info.begin(), tc_info.end(),
    [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[2] > aa2[2];});
    evt_timing = tc_info[0][1];
    evt_win    = tc_info[0][3];
    // Sort by TC number
    sort(tc_info.begin(), tc_info.end(),
    [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[0] < aa2[0];});

    for (int ii = 0; ii < evt_ntc; ii++) {
      m_tcid    = tc_info[ii][0];
      m_time    = tc_info[ii][1];
      m_energy  = tc_info[ii][2];
      m_win     = tc_info[ii][3];
      m_revo    = vwin_revo[3];
      m_caltime = evt_timing - m_time;

      evt_etot += m_energy;
      if ((m_tcid < 81 && m_tcid % 5 != 1) || (m_tcid > 80 && m_tcid < 513)) {
        evt_etot_p += m_energy;
      }
      m_TRGECLTCArray.appendNew();
      m_tcNum = m_TRGECLTCArray.getEntries() - 1;
      m_TRGECLTCArray[m_tcNum]->setEventId(n_basf2evt);
      m_TRGECLTCArray[m_tcNum]->setTCId(m_tcid);
      m_TRGECLTCArray[m_tcNum]->setTCTime(m_time);
      m_TRGECLTCArray[m_tcNum]->setTCCALTime(m_caltime);
      m_TRGECLTCArray[m_tcNum]->setHitWin(m_win);
      m_TRGECLTCArray[m_tcNum]->setRevoFAM(m_revo);
      m_TRGECLTCArray[m_tcNum]->setTCEnergy(m_energy);
      m_TRGECLTCArray[m_tcNum]->setChecksum(flag_checksum);
    }

    m_TRGECLEvtArray.appendNew();
    m_evtNum = m_TRGECLEvtArray.getEntries() - 1;
    m_TRGECLEvtArray[m_evtNum]->setEventId(n_basf2evt);
    m_TRGECLEvtArray[m_evtNum]->setETM(etm_version);
    m_TRGECLEvtArray[m_evtNum]->setL1Revo(l1_revo);
    m_TRGECLEvtArray[m_evtNum]->setEvtTime(evt_timing);
    m_TRGECLEvtArray[m_evtNum]->setEvtRevo(evt_revo);
    m_TRGECLEvtArray[m_evtNum]->setEvtWin(evt_win);
    m_TRGECLEvtArray[m_evtNum]->setNTC(evt_ntc);
    m_TRGECLEvtArray[m_evtNum]->setEtot(evt_etot);
    m_TRGECLEvtArray[m_evtNum]->setEtotP(evt_etot_p);
    m_TRGECLEvtArray[m_evtNum]->set3DBhabhaV(evt_b2bha_v);
    m_TRGECLEvtArray[m_evtNum]->set3DBhabhaS(evt_b2bha_s);
    m_TRGECLEvtArray[m_evtNum]->setMumu(evt_mumu);
    m_TRGECLEvtArray[m_evtNum]->setPrescale(evt_prescale);
    m_TRGECLEvtArray[m_evtNum]->set2DBhabha(evt_b1bha);
    m_TRGECLEvtArray[m_evtNum]->setPhysics(evt_phy);
    m_TRGECLEvtArray[m_evtNum]->setCheckSum(flag_checksum);
    m_TRGECLEvtArray[m_evtNum]->setEvtExist(1);
  } else {
    m_TRGECLTCArray.appendNew();
    m_tcNum = m_TRGECLTCArray.getEntries() - 1;
    m_TRGECLTCArray[m_tcNum]->setEventId(n_basf2evt);
    m_TRGECLTCArray[m_tcNum]->setTCId(0);
    m_TRGECLTCArray[m_tcNum]->setTCTime(-9999);
    m_TRGECLTCArray[m_tcNum]->setTCCALTime(-9999);
    m_TRGECLTCArray[m_tcNum]->setHitWin(-9999);
    m_TRGECLTCArray[m_tcNum]->setRevoFAM(-9999);
    m_TRGECLTCArray[m_tcNum]->setTCEnergy(0);
    m_TRGECLTCArray[m_tcNum]->setChecksum(flag_checksum);

    m_TRGECLEvtArray.appendNew();
    m_evtNum = m_TRGECLEvtArray.getEntries() - 1;
    m_TRGECLEvtArray[m_evtNum]->setEventId(n_basf2evt);
    m_TRGECLEvtArray[m_evtNum]->setETM(etm_version);
    m_TRGECLEvtArray[m_evtNum]->setL1Revo(-9999);
    m_TRGECLEvtArray[m_evtNum]->setEvtTime(-9999);
    m_TRGECLEvtArray[m_evtNum]->setEvtRevo(-9999);
    m_TRGECLEvtArray[m_evtNum]->setEvtWin(-9999);
    m_TRGECLEvtArray[m_evtNum]->setNTC(0);
    m_TRGECLEvtArray[m_evtNum]->setEtot(0);
    m_TRGECLEvtArray[m_evtNum]->setEtotP(0);
    m_TRGECLEvtArray[m_evtNum]->set3DBhabhaV(0);
    m_TRGECLEvtArray[m_evtNum]->set3DBhabhaS(0);
    m_TRGECLEvtArray[m_evtNum]->setMumu(0);
    m_TRGECLEvtArray[m_evtNum]->setPrescale(0);
    m_TRGECLEvtArray[m_evtNum]->set2DBhabha(0);
    m_TRGECLEvtArray[m_evtNum]->setPhysics(0);
    m_TRGECLEvtArray[m_evtNum]->setCheckSum(flag_checksum);
    m_TRGECLEvtArray[m_evtNum]->setEvtExist(0);
  }

  return;
}

void TRGECLUnpackerModule::checkBuffer_114(int* rdat, int nnn)
{

  unsigned char check_sum = (rdat[nnn - 1] >> 8) & 0xFF;

  unsigned char data_sum  = 0; // checksum
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
  int l1_revo = 0;
  int hit_win = 0;

  tc_info.clear();
  evt_info.clear();

  l1_revo = (rdat[0] >> 16) & 0xffff;
  data_sum = ((l1_revo >> 8) & 0xFF) + (l1_revo & 0xff);

  while (i < nnn - 1) {
    summary_data[0] = (rdat[i] >> 8) & 0xff;
    summary_data[1] =  rdat[i] & 0xff;
    summary_data[2] = (rdat[i + 1] >> 24) & 0xff;
    summary_data[3] = (rdat[i + 1] >> 16) & 0xff;

    for (int j = 0; j < 4; j++) {
      data_sum = data_sum + summary_data[j];
    }
    w_ntc  = (summary_data[2] << 8) + summary_data[3];
    ntc    = ntc + w_ntc;
    if (window == 3) {
      w_revo = summary_data[1];
    }

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
        data_sum  = data_sum +
                    ((rdat[i + 1 + j] >>  8) & 0xff) +
                    ((rdat[i + 1 + j]) & 0xff) +
                    ((rdat[i + 2 + j] >> 24) & 0xff) +
                    ((rdat[i + 2 + j] >> 16) & 0xff);

        tc_info.push_back(tc_id);
        tc_info.push_back(conv_tc_t);
        tc_info.push_back(tc_e);
        tc_info.push_back(hit_win);
        evt_info.push_back(tc_info);

        tc_info.clear();
      }
      i = i + w_ntc + 1;
      window++;
    }
  }


  if (check_sum == data_sum) {
    flag_checksum = 0;
  } else {
    flag_checksum = 1;
  }

  int evt_size   = evt_info.size();
  int evt_timing = -9999;

  int m_revo;
  int m_gdl;
  int m_ntc;
  int m_tcid;
  int m_time;
  int m_caltime;
  int m_evttime;
  int m_energy;
  int m_checksum;
  int m_hitwin;
  int m_hitNum;
  int m_evtNum;

  if (evt_size != 0 && flag_checksum == 0 && nnn > 7) {
    // Find most energetic TC timing
    sort(evt_info.begin(), evt_info.end(),
    [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[2] > aa2[2];});
    evt_timing = evt_info[0][1];

    // Sort by TC number
    sort(evt_info.begin(), evt_info.end(),
    [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[0] < aa2[0];});

    m_gdl      = l1_revo;
    m_revo     = w_revo;
    m_ntc      = ntc;
    m_evttime  = evt_timing;
    m_checksum = flag_checksum;
    for (int ii = 0; ii < evt_size; ii++) {
      m_tcid    = evt_info[ii][0];
      m_time    = evt_info[ii][1];
      m_energy  = evt_info[ii][2];
      m_hitwin  = evt_info[ii][3];
      m_caltime = evt_timing - m_time;

      m_TRGECLTCArray.appendNew();
      m_hitNum = m_TRGECLTCArray.getEntries() - 1;
      m_TRGECLTCArray[m_hitNum]->setEventId(n_basf2evt);
      m_TRGECLTCArray[m_hitNum]->setTCId(m_tcid);
      m_TRGECLTCArray[m_hitNum]->setTCTime(m_time);
      m_TRGECLTCArray[m_hitNum]->setTCCALTime(m_caltime);
      m_TRGECLTCArray[m_hitNum]->setHitWin(m_hitwin);
      m_TRGECLTCArray[m_hitNum]->setRevoFAM(m_revo);
      m_TRGECLTCArray[m_hitNum]->setTCEnergy(m_energy);
      m_TRGECLTCArray[m_hitNum]->setChecksum(m_checksum);
    }
    m_TRGECLEvtArray.appendNew();
    m_evtNum = m_TRGECLEvtArray.getEntries() - 1;
    m_TRGECLEvtArray[m_evtNum]->setEventId(n_basf2evt);
    m_TRGECLEvtArray[m_evtNum]->setNTC(m_ntc);
    m_TRGECLEvtArray[m_evtNum]->setEvtTime(m_evttime);
    m_TRGECLEvtArray[m_evtNum]->setL1Revo(m_gdl);
    m_TRGECLEvtArray[m_evtNum]->setCheckSum(m_checksum);


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
    m_gdl      = l1_revo;
    if (flag_checksum == 1) {
      m_checksum = 1;
    } else {
      m_checksum = 2;
    }

    m_TRGECLTCArray.appendNew();
    m_hitNum = m_TRGECLTCArray.getEntries() - 1;
    m_TRGECLTCArray[m_hitNum]->setEventId(n_basf2evt);
    m_TRGECLTCArray[m_hitNum]->setTCId(m_tcid);
    m_TRGECLTCArray[m_hitNum]->setTCTime(m_time);
    m_TRGECLTCArray[m_hitNum]->setTCCALTime(m_caltime);
    m_TRGECLTCArray[m_hitNum]->setHitWin(m_hitwin);
    m_TRGECLTCArray[m_hitNum]->setRevoFAM(m_revo);
    m_TRGECLTCArray[m_hitNum]->setTCEnergy(m_energy);
    m_TRGECLTCArray[m_hitNum]->setChecksum(m_checksum);

    m_TRGECLEvtArray.appendNew();
    m_evtNum = m_TRGECLEvtArray.getEntries() - 1;
    m_TRGECLEvtArray[m_evtNum]->setEventId(n_basf2evt);
    m_TRGECLEvtArray[m_evtNum]->setNTC(0);
    m_TRGECLEvtArray[m_evtNum]->setEvtTime(-9999);
    m_TRGECLEvtArray[m_evtNum]->setL1Revo(m_gdl);
    m_TRGECLEvtArray[m_evtNum]->setCheckSum(m_checksum);

  }

  return;
}
