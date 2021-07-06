/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/ecl/modules/trgeclUnpacker/trgeclUnpackerModule.h>

#include "trg/ecl/TrgEclDataBase.h"
#include "trg/ecl/TrgEclMapping.h"

using namespace std;
using namespace Belle2;

//! Register Module
REG_MODULE(TRGECLUnpacker);

string TRGECLUnpackerModule::version() const
{
  return string("4.02");
}

TRGECLUnpackerModule::TRGECLUnpackerModule()
  : Module::Module(), n_basf2evt(0)
{

  string desc = "TRGECLUnpackerModule(" + version() + ")";
  setDescription(desc);
  setPropertyFlags(c_ParallelProcessingCertified);

  B2DEBUG(20, "trgeclunpacker: Constructor done.");
}

TRGECLUnpackerModule::~TRGECLUnpackerModule() {}

void TRGECLUnpackerModule::terminate()
{
}

void TRGECLUnpackerModule::initialize()
{
  m_TRGECLSumArray.registerInDataStore();
  m_TRGECLTCArray.registerInDataStore();
  m_TRGECLEvtArray.registerInDataStore();
  m_TRGECLClusterArray.registerInDataStore();
}

void TRGECLUnpackerModule::beginRun() {}
void TRGECLUnpackerModule::endRun() {}
void TRGECLUnpackerModule::event()
{

  StoreArray<RawTRG> raw_trgarray;

  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    iFiness = i;
    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      nodeid     = ((raw_trgarray[i]->GetNodeID(j)) >> 24) & 0x1F;
      nwords     = raw_trgarray[i]->GetDetectorNwords(j, 0);
      trgtype    = raw_trgarray[i]->GetTRGType(j);
      n_basf2evt = raw_trgarray[i]->GetEveNo(j);
      if (nodeid == 0x13) {
        if (nwords < 9) {
          B2ERROR("Consistecy error in unpacker.");
          B2ERROR("data length " << nwords << " nWord " << nwords);
          B2ERROR("Node ID " << nodeid << ", Finness ID " << iFiness);
          continue;
        }
        readCOPPEREvent(raw_trgarray[i], j, nwords);
      }
    }
  }
}

void TRGECLUnpackerModule::readCOPPEREvent(RawTRG* raw_copper, int i, int nnn)
{
  /* cppcheck-suppress variableScope */
  int* rdat;
  if (raw_copper->GetDetectorNwords(i, 0) > 0) {
    rdat = raw_copper->GetDetectorBuffer(i, 0);
    etm_version = ((rdat[0] >> 16) & 0xffff);
    if (etm_version > 136) {
      checkBuffer(rdat, nnn);
    } else  {
      checkBuffer_v136(rdat, nnn);
    }
  }
}

void TRGECLUnpackerModule::checkBuffer(int* rdat, int nnn)
{

  int version_check = (rdat[0] >> 12) & 0xf;
  if (version_check != 15) return;

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
  /* cppcheck-suppress variableScope */
  int summary_data      = 0;
  int summary_recon     = 0;
  /* cppcheck-suppress variableScope */
  int summary_revo      = 0;
  /* cppcheck-suppress variableScope */
  bool summary_trg      = false;
  /* cppcheck-suppress variableScope */
  int data_win          = 0;

  // TC
  /* cppcheck-suppress variableScope */
  int ntc_win           =     0;
  bool tc_trg           = false;
  // TC info
  int tc_id             =     0;
  int tc_t              =     0;
  int tc_e              =     0;
  int conv_tc_t         =     0;
  int win3_revo         = -9999;

  vector<unsigned> sum_data;
  vector<vector<unsigned>> sum_info;

  vector<int> tc_data;
  vector<vector<int>> tc_info;
  vector<vector<int>> tc_info_FE1;
  vector<vector<int>> tc_info_FE2;
  vector<vector<int>> tc_info_BE1;
  vector<vector<int>> tc_info_BE2;

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
    if (window_num == 3) win3_revo = summary_revo;

    if (summary_trg == true) { // Summary on
      sum_data.push_back(data_win);
      sum_data.push_back(summary_revo);
      for (int j = 0; j < 12; j++) {
        summary_recon =
          (((rdat[i + j + 2] >>  0) & 0xFF) << 24) +
          (((rdat[i + j + 2] >>  8) & 0xFF) << 16) +
          (((rdat[i + j + 2] >> 16) & 0xFF) <<  8) +
          (((rdat[i + j + 2] >> 24) & 0xFF) <<  0);
        sum_data.push_back(summary_recon);
      }
      sum_info.push_back(sum_data);
      sum_data.clear();
      i = i + 14;

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
          if (tc_id < 81) {
            if (tc_id > 75) {
              tc_info_FE1.push_back(tc_data);
            } else {
              tc_info_FE2.push_back(tc_data);
            }
          } else if (tc_id > 512) {
            if (tc_id > 572) {
              tc_info_BE1.push_back(tc_data);
            } else {
              tc_info_BE2.push_back(tc_data);
            }
          } else {
            tc_info.push_back(tc_data);
          }
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
          if (tc_id < 81) {
            if (tc_id > 75) {
              tc_info_FE1.push_back(tc_data);
            } else {
              tc_info_FE2.push_back(tc_data);
            }
          } else if (tc_id > 512) {
            if (tc_id > 572) {
              tc_info_BE1.push_back(tc_data);
            } else {
              tc_info_BE2.push_back(tc_data);
            }
          } else {
            tc_info.push_back(tc_data);
          }
          tc_data.clear();
        }
        i = i + ntc_win + 1;
      } else { // Summary off & TC off
        i = i + 1;
      }
    }
    window_num++;
  }

  // <---- Unpacking

  // Summary
  /* cppcheck-suppress variableScope */
  int sum_num_ord  = 0;
  /* cppcheck-suppress variableScope */
  int sum_num      = 0;
  /* cppcheck-suppress variableScope */
  int sum_revo     = 0;
  int cl_theta[6]  = {0};
  int cl_phi[6]    = {0};
  int cl_time[6]   = { -9999};
  int cl_energy[6] = {0};
  int cl_1gev[6]   = {0};
  int cl_2gev[6]   = {0};
  int cl_bha[6]    = {0};
  int ncl          = 0;
  int low_multi    = 0;
  int b2bhabha_v   = 0;
  int b2bhabha_s   = 0;
  int mumu         = 0;
  int prescale     = 0;
  int icn_over     = 0;
  int bg_veto      = 0;
  int icn          = 0;
  int etot_type    = 0;
  int etot         = 0;
  int b1_type      = 0;
  int b1bhabha     = 0;
  int physics      = 0;
  int time_type    = 0;
  int time         = 0;
  int ecl_bst      = 0;

  int m_sumNum     = 0;

  TrgEclDataBase database;
  TrgEclMapping mapping;

  vector<int> cl_1d;
  vector<vector<int>> cl_2d;

  vector<int> evt_1d_vector;
  vector<vector<int>> evt_2d_vector;

  // Store Summary
  int sum_size = sum_info.size();
  if (sum_size != 0) {
    for (int j = 0; j < sum_size; j++) {
      sum_num  = sum_info[j][0];
      sum_revo = sum_info[j][1];
      // TRG
      time         = (sum_info[j][2]) & 0x7F;
      time_type    = (sum_info[j][2] >>  7) & 0x7;
      physics      = (sum_info[j][2] >> 10) & 0x1;
      b1bhabha     = (sum_info[j][2] >> 11) & 0x1;
      b1_type      = (sum_info[j][2] >> 12) & 0x3FFF;
      etot         = ((sum_info[j][3] & 0x7F) << 6) + ((sum_info[j][2] >> 26) & 0x3F);
      etot_type    = (sum_info[j][3] >>  7) & 0x7;
      icn          = (sum_info[j][3] >> 10) & 0x7F;
      bg_veto      = (sum_info[j][3] >> 17) & 0x7;
      icn_over     = (sum_info[j][3] >> 20) & 0x1;
      b2bhabha_v   = (sum_info[j][3] >> 21) & 0x1;
      low_multi    = (((sum_info[j][4] >> 6) & 0x3) << 12) + ((sum_info[j][4] & 0x3) << 10) + ((sum_info[j][3] >> 22) & 0x3FF);
      b2bhabha_s   = (sum_info[j][4] >>  2) & 0x1;
      mumu         = (sum_info[j][4] >>  3) & 0x1;
      prescale     = (sum_info[j][4] >>  4) & 0x1;
      ecl_bst      = (sum_info[j][4] >>  5) & 0x1;
      // CL
      cl_energy[0] = (sum_info[j][5]) & 0xFFF;
      cl_time[0]   = (sum_info[j][5] >> 12) & 0xFF;
      cl_phi[0]    = (sum_info[j][5] >> 20) & 0xFF;
      cl_theta[0]  = ((sum_info[j][6] & 0x7) << 4) + ((sum_info[j][5] >> 28) & 0xF);

      cl_energy[1] = (sum_info[j][6] >>  3) & 0xFFF;
      cl_time[1]   = (sum_info[j][6] >> 15) & 0xFF;
      cl_phi[1]    = (sum_info[j][6] >> 23) & 0xFF;
      cl_theta[1]  = ((sum_info[j][7] & 0x3F) << 1) + ((sum_info[j][6] >> 31) & 0x1);

      cl_energy[2] = (sum_info[j][7] >>  6) & 0xFFF;
      cl_time[2]   = (sum_info[j][7] >> 18) & 0xFF;
      cl_phi[2]    = ((sum_info[j][8] & 0x3) << 6) + ((sum_info[j][7] >> 26) & 0x3F);
      cl_theta[2]  = (sum_info[j][8] >>  2) & 0x3F;

      cl_energy[3] = (sum_info[j][8] >>  9) & 0xFFF;
      cl_time[3]   = (sum_info[j][8] >> 21) & 0xFF;
      cl_phi[3]    = ((sum_info[j][9] & 0x1F) << 3) + ((sum_info[j][8] >> 29) & 0x7);
      cl_theta[3]  = (sum_info[j][9] >>  5) & 0x3F;

      cl_energy[4] = (sum_info[j][ 9] >> 12) & 0xFFF;
      cl_time[4]   = (sum_info[j][ 9] >> 24) & 0xFF;
      cl_phi[4]    = (sum_info[j][10]) & 0xFF;
      cl_theta[4]  = (sum_info[j][10] >>  8) & 0x3F;

      cl_energy[5] = (sum_info[j][10] >> 15) & 0xFFF;
      cl_time[5]   = ((sum_info[j][11] & 0x7) << 5) + ((sum_info[j][10] >> 27) & 0x1F);
      cl_phi[5]    = (sum_info[j][11] >>  3) & 0xFF;
      cl_theta[5]  = (sum_info[j][11] >> 11) & 0x3F;
      // CL others
      for (int k = 0; k < 6; k++) {
        cl_1gev[k] = (sum_info[j][12] >>  k) & 0x1;
        cl_2gev[k] = (sum_info[j][12] >> (k + 6)) & 0x1;
        cl_bha[k]  = (sum_info[j][12] >> (k + 12)) & 0x1;
      }
      ncl          = (sum_info[j][13]) & 0x7;

      m_TRGECLSumArray.appendNew();
      m_sumNum = m_TRGECLSumArray.getEntries() - 1;
      m_TRGECLSumArray[m_sumNum]->setEventId(n_basf2evt);
      m_TRGECLSumArray[m_sumNum]->setSumNum(sum_num);
      m_TRGECLSumArray[m_sumNum]->setSumRevo(sum_revo);
      m_TRGECLSumArray[m_sumNum]->setCLTheta(cl_theta);
      m_TRGECLSumArray[m_sumNum]->setCLPhi(cl_phi);
      m_TRGECLSumArray[m_sumNum]->setCLTime(cl_time);
      m_TRGECLSumArray[m_sumNum]->setCLEnergy(cl_energy);
      m_TRGECLSumArray[m_sumNum]->setCLF1GeV(cl_1gev);
      m_TRGECLSumArray[m_sumNum]->setCLF2GeV(cl_2gev);
      m_TRGECLSumArray[m_sumNum]->setCLFBha(cl_bha);
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
      m_TRGECLSumArray[m_sumNum]->setECLBST(ecl_bst);
      m_TRGECLSumArray[m_sumNum]->setTime(time);
      m_TRGECLSumArray[m_sumNum]->setTimeType(time_type);

      for (int k = 0; k < 6; k++) {
        cl_1d.push_back(cl_theta[k]);
        cl_1d.push_back(cl_phi[k]);
        cl_1d.push_back(cl_time[k]);
        cl_1d.push_back(cl_energy[k]);
        cl_1d.push_back(cl_1gev[k]);
        cl_1d.push_back(cl_2gev[k]);
        cl_1d.push_back(cl_bha[k]);
        cl_2d.push_back(cl_1d);
        cl_1d.clear();
      }
      sort(cl_2d.begin(), cl_2d.end(),
      [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[3] > aa2[3];});

      if (sum_num == -9999) {
        sum_num_ord = -9999;
      } else if (sum_num <= 3) {
        sum_num_ord = 2 * abs(sum_num - 3);
      } else {
        sum_num_ord = (sum_num * 2) - 7;
      }
      evt_1d_vector.push_back(sum_num_ord);
      evt_1d_vector.push_back(sum_revo);
      evt_1d_vector.push_back(sum_num);
      evt_1d_vector.push_back(time);
      for (int k = 0; k < 6; k++) {
        for (int l = 0; l < 7; l++) {
          evt_1d_vector.push_back(cl_2d[k][l]);
        }
      }
      evt_1d_vector.push_back(ncl);
      evt_1d_vector.push_back(low_multi);
      evt_1d_vector.push_back(b2bhabha_v);
      evt_1d_vector.push_back(b2bhabha_s);
      evt_1d_vector.push_back(mumu);
      evt_1d_vector.push_back(prescale);
      evt_1d_vector.push_back(icn);
      evt_1d_vector.push_back(icn_over);
      evt_1d_vector.push_back(etot_type);
      evt_1d_vector.push_back(etot);
      evt_1d_vector.push_back(ecl_bst);
      evt_1d_vector.push_back(b1_type);
      evt_1d_vector.push_back(b1bhabha);
      evt_1d_vector.push_back(physics);
      evt_1d_vector.push_back(time_type);
      evt_2d_vector.push_back(evt_1d_vector);
      evt_1d_vector.clear();
    }
  } else {
    for (int k = 0; k < 6; k++) {
      cl_theta[k]   = 0;
      cl_phi[k]     = 0;
      cl_time[k]    = -9999;
      cl_energy[k]  = 0;
      cl_1gev[k]    = 0;
      cl_2gev[k]    = 0;
      cl_bha[k]     = 0;
    }
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
    ecl_bst    = 0;
    b1_type    = 0;
    b1bhabha   = 0;
    physics    = 0;
    time_type  = 0;
    time       = -9999;

    m_TRGECLSumArray.appendNew();
    m_sumNum = m_TRGECLSumArray.getEntries() - 1;
    m_TRGECLSumArray[m_sumNum]->setEventId(n_basf2evt);
    m_TRGECLSumArray[m_sumNum]->setSumNum(0);
    m_TRGECLSumArray[m_sumNum]->setCLTheta(cl_theta);
    m_TRGECLSumArray[m_sumNum]->setCLPhi(cl_phi);
    m_TRGECLSumArray[m_sumNum]->setCLTime(cl_time);
    m_TRGECLSumArray[m_sumNum]->setCLEnergy(cl_energy);
    m_TRGECLSumArray[m_sumNum]->setCLF1GeV(cl_1gev);
    m_TRGECLSumArray[m_sumNum]->setCLF2GeV(cl_2gev);
    m_TRGECLSumArray[m_sumNum]->setCLFBha(cl_bha);
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
    m_TRGECLSumArray[m_sumNum]->setECLBST(ecl_bst);
    m_TRGECLSumArray[m_sumNum]->setTime(time);
    m_TRGECLSumArray[m_sumNum]->setTimeType(time_type);
  }

  // TC & TRG
  tc_info.insert(tc_info.end(), tc_info_FE1.begin(), tc_info_FE1.end());
  tc_info.insert(tc_info.end(), tc_info_FE2.begin(), tc_info_FE2.end());
  tc_info.insert(tc_info.end(), tc_info_BE1.begin(), tc_info_BE1.end());
  tc_info.insert(tc_info.end(), tc_info_BE2.begin(), tc_info_BE2.end());

  int m_evtNum   = 0;

  int m_tcNum    = 0;
  /* cppcheck-suppress variableScope */
  int m_tcid     = 0;
  /* cppcheck-suppress variableScope */
  int m_time     = -9999;
  /* cppcheck-suppress variableScope */
  int m_energy   = 0;
  /* cppcheck-suppress variableScope */
  int m_win      = 0;
  /* cppcheck-suppress variableScope */
  int m_revo     = 0;
  /* cppcheck-suppress variableScope */
  int m_caltime  = -9999;

  int tot_ntc          = tc_info.size();
  /* cppcheck-suppress variableScope */
  int evt_ntc          = 0;
  /* cppcheck-suppress variableScope */
  int evt_revo         = -9999;
  /* cppcheck-suppress variableScope */
  int evt_win          = 0;
  /* cppcheck-suppress variableScope */
  int evt_timing       = -9999; // most energetic
  int evt_cl_theta[6]  = {0};
  int evt_cl_phi[6]    = {0};
  int evt_cl_time[6]   = { -9999};
  int evt_cl_energy[6] = {0};
  int evt_cl_1gev[6]   = {0};
  int evt_cl_2gev[6]   = {0};
  int evt_cl_bha[6]    = {0};
  /* cppcheck-suppress variableScope */
  int evt_ncl          = 0;
  /* cppcheck-suppress variableScope */
  int evt_low_multi    = 0;
  /* cppcheck-suppress variableScope */
  int evt_b2bhabha_v   = 0;
  /* cppcheck-suppress variableScope */
  int evt_b2bhabha_s   = 0;
  /* cppcheck-suppress variableScope */
  int evt_mumu         = 0;
  /* cppcheck-suppress variableScope */
  int evt_prescale     = 0;
  /* cppcheck-suppress variableScope */
  int evt_icn          = 0;
  /* cppcheck-suppress variableScope */
  int evt_icn_over     = 0;
  /* cppcheck-suppress variableScope */
  int evt_etot_type    = 0;
  /* cppcheck-suppress variableScope */
  int evt_etot         = 0;
  /* cppcheck-suppress variableScope */
  int evt_ecl_bst      = 0;
  /* cppcheck-suppress variableScope */
  int evt_b1_type      = 0;
  /* cppcheck-suppress variableScope */
  int evt_b1bhabha     = 0;
  /* cppcheck-suppress variableScope */
  int evt_physics      = 0;
  /* cppcheck-suppress variableScope */
  int evt_time_type    = 0;
  /* cppcheck-suppress variableScope */
  int evt_etot_all     = 0;
  /* cppcheck-suppress variableScope */
  int evt_time_min     = 0;
  /* cppcheck-suppress variableScope */
  int evt_time_max     = 0;
  /* cppcheck-suppress variableScope */
  int evt_time_win     = 0;
  /* cppcheck-suppress variableScope */
  int etot_i     = 0;
  /* cppcheck-suppress variableScope */
  int etot_c     = 0;
  /* cppcheck-suppress variableScope */
  int etot_f     = 0;
  /* cppcheck-suppress variableScope */
  int cl_tcid = 0;
  /* cppcheck-suppress variableScope */
  int cl_thetaid = 0;
  /* cppcheck-suppress variableScope */
  int cl_phiid = 0;
  /* cppcheck-suppress variableScope */
  int m_clNum    = 0;


  int evt_v_size = evt_2d_vector.size();
  if (evt_v_size != 0) {
    // Sort window : 3 => 4 => 2 => 5 => 1 => 6 => 7
    sort(evt_2d_vector.begin(), evt_2d_vector.end(),
    [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[0] < aa2[0];});
  }

  if (tot_ntc != 0 && flag_checksum == 0 && nnn > 7) {
    if (evt_v_size == 0) {
      // Find most energetic TC timing
      sort(tc_info.begin(), tc_info.end(),
      [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[2] > aa2[2];});
      evt_revo         = win3_revo;
      evt_win          = tc_info[0][3];
      evt_timing       = tc_info[0][1];
      for (int k = 0; k < 6; k++) {
        evt_cl_theta[k]  = 0;
        evt_cl_phi[k]    = 0;
        evt_cl_time[k]   = 0;
        evt_cl_energy[k] = 0;
        evt_cl_1gev[k]   = 0;
        evt_cl_2gev[k]   = 0;
        evt_cl_bha[k]    = 0;
      }
      evt_ncl          = 0;
      evt_low_multi    = 0;
      evt_b2bhabha_v   = 0;
      evt_b2bhabha_s   = 0;
      evt_mumu         = 0;
      evt_prescale     = 0;
      evt_icn          = 0;
      evt_icn_over     = 0;
      evt_etot_type    = 0;
      evt_etot         = 0;
      evt_ecl_bst      = 0;
      evt_b1_type      = 0;
      evt_b1bhabha     = 0;
      evt_physics      = 0;
      evt_time_type    = 0;
    } else {
      evt_revo   = evt_2d_vector[0][1];
      evt_win    = evt_2d_vector[0][2];
      evt_timing = evt_2d_vector[0][3];
      for (int k = 0; k < 6; k++) {
        evt_cl_theta[k]  = evt_2d_vector[0][ 4 + k * 7];
        evt_cl_phi[k]    = evt_2d_vector[0][ 5 + k * 7];
        evt_cl_time[k]   = evt_2d_vector[0][ 6 + k * 7];
        evt_cl_energy[k] = evt_2d_vector[0][ 7 + k * 7];
        evt_cl_1gev[k]   = evt_2d_vector[0][ 8 + k * 7];
        evt_cl_2gev[k]   = evt_2d_vector[0][ 9 + k * 7];
        evt_cl_bha[k]    = evt_2d_vector[0][10 + k * 7];
      }
      evt_ncl          = evt_2d_vector[0][46];
      evt_low_multi    = evt_2d_vector[0][47];
      evt_b2bhabha_v   = evt_2d_vector[0][48];
      evt_b2bhabha_s   = evt_2d_vector[0][49];
      evt_mumu         = evt_2d_vector[0][50];
      evt_prescale     = evt_2d_vector[0][51];
      evt_icn          = evt_2d_vector[0][52];
      evt_icn_over     = evt_2d_vector[0][53];
      evt_etot_type    = evt_2d_vector[0][54];
      evt_etot         = evt_2d_vector[0][55];
      evt_ecl_bst      = evt_2d_vector[0][56];
      evt_b1_type      = evt_2d_vector[0][57];
      evt_b1bhabha     = evt_2d_vector[0][58];
      evt_physics      = evt_2d_vector[0][59];
      evt_time_type    = evt_2d_vector[0][60];
    }
    // Sort by TC number
    sort(tc_info.begin(), tc_info.end(),
    [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[0] < aa2[0];});

    for (int ii = 0; ii < tot_ntc; ii++) {
      m_tcid    = tc_info[ii][0];
      m_time    = tc_info[ii][1];
      m_energy  = tc_info[ii][2];
      m_win     = tc_info[ii][3];
      m_revo    = win3_revo;
      m_caltime = m_time - ((evt_win - 3) * 128 + evt_timing);
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

      if (m_win == evt_win || m_win == evt_win + 1) evt_ntc++;
      if (m_win == evt_win - 1) {
        etot_i += m_energy;
      }
      if (m_win == evt_win) {
        etot_c += m_energy;
      }
      if (m_win == evt_win + 1) {
        etot_f += m_energy;
      }
    }

    if (etot_i == 0 && etot_f == 0) {
      evt_etot_all = etot_c;
      evt_time_min =     - evt_timing;
      evt_time_max = 256 - evt_timing;
      evt_time_win = 1;
    } else if (etot_i >= etot_f) {
      evt_etot_all = etot_c + etot_i;
      evt_time_min = -128 - evt_timing;
      evt_time_max =  128 - evt_timing;
      evt_time_win = -1;
    } else {
      evt_etot_all = etot_c + etot_f;
      evt_time_min =     - evt_timing;
      evt_time_max = 256 - evt_timing;
      evt_time_win = 1;
    }

    for (int icluster = 0; icluster < 6; icluster++) {
      if (evt_cl_energy[icluster] == 0 || evt_cl_theta[icluster] == 0 || evt_cl_phi[icluster] == 0) {continue;}
      cl_tcid = mapping.getTCIdFromPosition(evt_cl_theta[icluster], evt_cl_phi[icluster]);
      if (cl_tcid == 0) {continue;}
      cl_thetaid = mapping.getTCThetaIdFromTCId(cl_tcid);
      cl_phiid   = mapping.getTCPhiIdFromTCId(cl_tcid);

      m_TRGECLClusterArray.appendNew();
      m_clNum    =  m_TRGECLClusterArray.getEntries() - 1;
      m_TRGECLClusterArray[m_clNum]->setEventId(n_basf2evt);
      m_TRGECLClusterArray[m_clNum]->setClusterId(icluster);
      m_TRGECLClusterArray[m_clNum]->setEventRevo(evt_revo);

      m_TRGECLClusterArray[m_clNum]->setMaxTCId(cl_tcid);  // center of Cluster
      m_TRGECLClusterArray[m_clNum]->setMaxThetaId(cl_thetaid);
      m_TRGECLClusterArray[m_clNum]->setMaxPhiId(cl_phiid);
      m_TRGECLClusterArray[m_clNum]->setClusterId(icluster);
      m_TRGECLClusterArray[m_clNum]->setEnergyDep((double)evt_cl_energy[icluster] * 5.25); // MeV
      m_TRGECLClusterArray[m_clNum]->setTimeAve((double)evt_cl_time[icluster]);
      m_TRGECLClusterArray[m_clNum]->setPositionX(mapping.getTCPosition(cl_tcid).X());
      m_TRGECLClusterArray[m_clNum]->setPositionY(mapping.getTCPosition(cl_tcid).Y());
      m_TRGECLClusterArray[m_clNum]->setPositionZ(mapping.getTCPosition(cl_tcid).Z());
    }
    m_TRGECLEvtArray.appendNew();
    m_evtNum = m_TRGECLEvtArray.getEntries() - 1;
    m_TRGECLEvtArray[m_evtNum]->setEventId(n_basf2evt);
    m_TRGECLEvtArray[m_evtNum]->setETM(etm_version);
    m_TRGECLEvtArray[m_evtNum]->setL1Revo(l1_revo);
    m_TRGECLEvtArray[m_evtNum]->setEvtRevo(evt_revo);
    m_TRGECLEvtArray[m_evtNum]->setEvtWin(evt_win);
    m_TRGECLEvtArray[m_evtNum]->setEvtTime(evt_timing);
    m_TRGECLEvtArray[m_evtNum]->setNTC(evt_ntc);
    m_TRGECLEvtArray[m_evtNum]->setCLTheta(evt_cl_theta);
    m_TRGECLEvtArray[m_evtNum]->setCLPhi(evt_cl_phi);
    m_TRGECLEvtArray[m_evtNum]->setCLTime(evt_cl_time);
    m_TRGECLEvtArray[m_evtNum]->setCLEnergy(evt_cl_energy);
    m_TRGECLEvtArray[m_evtNum]->setCLF1GeV(evt_cl_1gev);
    m_TRGECLEvtArray[m_evtNum]->setCLF2GeV(evt_cl_2gev);
    m_TRGECLEvtArray[m_evtNum]->setCLFBha(evt_cl_bha);
    m_TRGECLEvtArray[m_evtNum]->setNCL(evt_ncl);
    m_TRGECLEvtArray[m_evtNum]->setLowMulti(evt_low_multi);
    m_TRGECLEvtArray[m_evtNum]->set3DBhabhaV(evt_b2bhabha_v);
    m_TRGECLEvtArray[m_evtNum]->set3DBhabhaS(evt_b2bhabha_s);
    m_TRGECLEvtArray[m_evtNum]->setMumu(evt_mumu);
    m_TRGECLEvtArray[m_evtNum]->setPrescale(evt_prescale);
    m_TRGECLEvtArray[m_evtNum]->setICN(evt_icn);
    m_TRGECLEvtArray[m_evtNum]->setICNOver(evt_icn_over);
    m_TRGECLEvtArray[m_evtNum]->setEtotType(evt_etot_type);
    m_TRGECLEvtArray[m_evtNum]->setEtot(evt_etot);
    m_TRGECLEvtArray[m_evtNum]->setECLBST(evt_ecl_bst);
    m_TRGECLEvtArray[m_evtNum]->set2DBhabha(evt_b1bhabha);
    m_TRGECLEvtArray[m_evtNum]->setBhabhaType(evt_b1_type);
    m_TRGECLEvtArray[m_evtNum]->setPhysics(evt_physics);
    m_TRGECLEvtArray[m_evtNum]->setTimeType(evt_time_type);
    m_TRGECLEvtArray[m_evtNum]->setCheckSum(flag_checksum);
    m_TRGECLEvtArray[m_evtNum]->setEvtExist(1);
    m_TRGECLEvtArray[m_evtNum]->setTRGTYPE(trgtype);
    m_TRGECLEvtArray[m_evtNum]->setEtotAll(evt_etot_all);
    m_TRGECLEvtArray[m_evtNum]->setEvtTimeMin(evt_time_min);
    m_TRGECLEvtArray[m_evtNum]->setEvtTimeMax(evt_time_max);
    m_TRGECLEvtArray[m_evtNum]->setEvtTimeWin(evt_time_win);
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
    for (int k = 0; k < 6; k++) {
      evt_cl_theta[k]  = 0;
      evt_cl_phi[k]    = 0;
      evt_cl_time[k]   = -9999;
      evt_cl_energy[k] = 0;
      evt_cl_1gev[k]   = 0;
      evt_cl_2gev[k]   = 0;
      evt_cl_bha[k]    = 0;
    }
    m_TRGECLEvtArray[m_evtNum]->setCLTheta(evt_cl_theta);
    m_TRGECLEvtArray[m_evtNum]->setCLPhi(evt_cl_phi);
    m_TRGECLEvtArray[m_evtNum]->setCLTime(evt_cl_time);
    m_TRGECLEvtArray[m_evtNum]->setCLEnergy(evt_cl_energy);
    m_TRGECLEvtArray[m_evtNum]->setCLF1GeV(evt_cl_1gev);
    m_TRGECLEvtArray[m_evtNum]->setCLF2GeV(evt_cl_2gev);
    m_TRGECLEvtArray[m_evtNum]->setCLFBha(evt_cl_bha);
    m_TRGECLEvtArray[m_evtNum]->setNCL(0);
    m_TRGECLEvtArray[m_evtNum]->setLowMulti(0);
    m_TRGECLEvtArray[m_evtNum]->set3DBhabhaV(0);
    m_TRGECLEvtArray[m_evtNum]->set3DBhabhaS(0);
    m_TRGECLEvtArray[m_evtNum]->setMumu(0);
    m_TRGECLEvtArray[m_evtNum]->setPrescale(0);
    m_TRGECLEvtArray[m_evtNum]->setICN(0);
    m_TRGECLEvtArray[m_evtNum]->setICNOver(0);
    m_TRGECLEvtArray[m_evtNum]->setEtotType(0);
    m_TRGECLEvtArray[m_evtNum]->setEtot(0);
    m_TRGECLEvtArray[m_evtNum]->setECLBST(0);
    m_TRGECLEvtArray[m_evtNum]->set2DBhabha(0);
    m_TRGECLEvtArray[m_evtNum]->setBhabhaType(0);
    m_TRGECLEvtArray[m_evtNum]->setPhysics(0);
    m_TRGECLEvtArray[m_evtNum]->setTimeType(0);
    m_TRGECLEvtArray[m_evtNum]->setCheckSum(flag_checksum);
    m_TRGECLEvtArray[m_evtNum]->setEvtExist(0);
    m_TRGECLEvtArray[m_evtNum]->setTRGTYPE(trgtype);
    m_TRGECLEvtArray[m_evtNum]->setEtotAll(0);
    m_TRGECLEvtArray[m_evtNum]->setEvtTimeMin(-9999);
    m_TRGECLEvtArray[m_evtNum]->setEvtTimeMax(-9999);
    m_TRGECLEvtArray[m_evtNum]->setEvtTimeWin(0);
  }

  return;
}

void TRGECLUnpackerModule::checkBuffer_v136(int* rdat, int nnn)
{

  int version_check = (rdat[0] >> 12) & 0xf;
  if (version_check != 15) return;

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
  /* cppcheck-suppress variableScope */
  int summary_data      = 0;
  /* cppcheck-suppress variableScope */
  int summary_revo      = 0;
  /* cppcheck-suppress variableScope */
  bool summary_trg      = false;
  /* cppcheck-suppress variableScope */
  int data_win          = 0;

  // TC
  /* cppcheck-suppress variableScope */
  int ntc_win           =     0;
  bool tc_trg           = false;
  // TC info
  int tc_id             =     0;
  int tc_t              =     0;
  int tc_e              =     0;
  int conv_tc_t         =     0;
  int win3_revo         = -9999;

  vector<unsigned> sum_data;
  vector<vector<unsigned>> sum_info;  //TODO can these be unsigned? (required for bit shifts shifts)

  vector<int> tc_data;
  vector<vector<int>> tc_info;
  vector<vector<int>> tc_info_FE1;
  vector<vector<int>> tc_info_FE2;
  vector<vector<int>> tc_info_BE1;
  vector<vector<int>> tc_info_BE2;

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
    if (window_num == 3) win3_revo = summary_revo;

    if (summary_trg == true) { // Summary on
      sum_data.push_back(data_win);
      sum_data.push_back(summary_revo);
      for (int j = 0; j < 9; j++) {
        sum_data.push_back(rdat[i + j + 2]);
      }
      sum_info.push_back(sum_data);
      sum_data.clear();
      i = i + 11;

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
          if (tc_id < 81) {
            if (tc_id > 75) {
              tc_info_FE1.push_back(tc_data);
            } else {
              tc_info_FE2.push_back(tc_data);
            }
          } else if (tc_id > 512) {
            if (tc_id > 572) {
              tc_info_BE1.push_back(tc_data);
            } else {
              tc_info_BE2.push_back(tc_data);
            }
          } else {
            tc_info.push_back(tc_data);
          }
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
          if (tc_id < 81) {
            if (tc_id > 75) {
              tc_info_FE1.push_back(tc_data);
            } else {
              tc_info_FE2.push_back(tc_data);
            }
          } else if (tc_id > 512) {
            if (tc_id > 572) {
              tc_info_BE1.push_back(tc_data);
            } else {
              tc_info_BE2.push_back(tc_data);
            }
          } else {
            tc_info.push_back(tc_data);
          }
          tc_data.clear();
        }
        i = i + ntc_win + 1;
      } else { // Summary off & TC off
        i = i + 1;
      }
    }
    window_num++;
  }

  // <---- Unpacking

  // Summary
  /* cppcheck-suppress variableScope */
  int sum_num        = 0;
  /* cppcheck-suppress variableScope */
  int sum_revo       = 0;
  int cl_theta[6]    = {0};
  int cl_phi[6]      = {0};
  int cl_time[6]     = { -9999};
  int cl_energy[6]   = {0};
  int ncl            = 0;
  int low_multi      = 0;
  int b2bhabha_v     = 0;
  int b2bhabha_s     = 0;
  int mumu           = 0;
  int prescale       = 0;
  int icn_over       = 0;
  int bg_veto        = 0;
  int icn            = 0;
  int etot_type      = 0;
  int etot           = 0;
  int b1_type        = 0;
  int b1bhabha       = 0;
  int physics        = 0;
  int time_type      = 0;
  int time           = 0;
  int ecl_bst        = 0;

  int m_sumNum       = 0;

  TrgEclDataBase database;
  TrgEclMapping mapping;

  vector<int> cl_1d;
  vector<vector<int>> cl_2d;

  vector<int> evt_1d_vector;
  vector<vector<int>> evt_2d_vector;

  // Store Summary
  int sum_size = sum_info.size();
  if (sum_size != 0) {
    for (int j = 0; j < sum_size; j++) {
      sum_num  = sum_info[j][0];
      sum_revo = sum_info[j][1];
      if (etm_version >= 128) {
        ecl_bst = (sum_info[j][2] >> 26) & 0x1;
      }
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
        cl_energy[3]     = (sum_info[j][5] >>  17) & 0xfff;

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
        if (etm_version >= 135) {
          low_multi      = (((sum_info[j][2] >> 27) & 0x3) << 12) + ((sum_info[j][8] & 0x3) << 10) + ((sum_info[j][9] >> 22) & 0x3ff);
        } else {
          low_multi      = ((sum_info[j][8] & 0x3) << 10) + ((sum_info[j][9] >> 22) & 0x3ff);
        }
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
      m_TRGECLSumArray[m_sumNum]->setECLBST(ecl_bst);
      m_TRGECLSumArray[m_sumNum]->setTime(time);
      m_TRGECLSumArray[m_sumNum]->setTimeType(time_type);

      for (int k = 0; k < 6; k++) {
        cl_1d.push_back(cl_theta[k]);
        cl_1d.push_back(cl_phi[k]);
        cl_1d.push_back(cl_time[k]);
        cl_1d.push_back(cl_energy[k]);
        cl_2d.push_back(cl_1d);
        cl_1d.clear();
      }
      sort(cl_2d.begin(), cl_2d.end(),
      [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[3] > aa2[3];});

      evt_1d_vector.push_back(abs(sum_num - 3));
      evt_1d_vector.push_back(sum_revo);
      evt_1d_vector.push_back(sum_num);
      evt_1d_vector.push_back(time);
      for (int k = 0; k < 6; k++) {
        evt_1d_vector.push_back(cl_2d[k][0]);
        evt_1d_vector.push_back(cl_2d[k][1]);
        evt_1d_vector.push_back(cl_2d[k][2]);
        evt_1d_vector.push_back(cl_2d[k][3]);
      }
      evt_1d_vector.push_back(ncl);
      evt_1d_vector.push_back(low_multi);
      evt_1d_vector.push_back(b2bhabha_v);
      evt_1d_vector.push_back(b2bhabha_s);
      evt_1d_vector.push_back(mumu);
      evt_1d_vector.push_back(prescale);
      evt_1d_vector.push_back(icn);
      evt_1d_vector.push_back(icn_over);
      evt_1d_vector.push_back(etot_type);
      evt_1d_vector.push_back(etot);
      evt_1d_vector.push_back(ecl_bst);
      evt_1d_vector.push_back(b1_type);
      evt_1d_vector.push_back(b1bhabha);
      evt_1d_vector.push_back(physics);
      evt_1d_vector.push_back(time_type);
      evt_2d_vector.push_back(evt_1d_vector);
      evt_1d_vector.clear();
    }
  } else {

    for (int k = 0; k < 6; k++) {
      cl_theta[k]   = 0;
      cl_phi[k]     = 0;
      cl_time[k]    = -9999;
      cl_energy[k]  = 0;
    }
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
    ecl_bst    = 0;
    b1_type    = 0;
    b1bhabha   = 0;
    physics    = 0;
    time_type  = 0;
    time       = -9999;

    m_TRGECLSumArray.appendNew();
    m_sumNum = m_TRGECLSumArray.getEntries() - 1;
    m_TRGECLSumArray[m_sumNum]->setEventId(n_basf2evt);
    m_TRGECLSumArray[m_sumNum]->setSumNum(0);
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
    m_TRGECLSumArray[m_sumNum]->setECLBST(ecl_bst);
    m_TRGECLSumArray[m_sumNum]->setTime(time);
    m_TRGECLSumArray[m_sumNum]->setTimeType(time_type);
  }

  // TC & TRG
  tc_info.insert(tc_info.end(), tc_info_FE1.begin(), tc_info_FE1.end());
  tc_info.insert(tc_info.end(), tc_info_FE2.begin(), tc_info_FE2.end());
  tc_info.insert(tc_info.end(), tc_info_BE1.begin(), tc_info_BE1.end());
  tc_info.insert(tc_info.end(), tc_info_BE2.begin(), tc_info_BE2.end());

  int m_evtNum   = 0;

  int m_tcNum    = 0;
  /* cppcheck-suppress variableScope */
  int m_tcid     = 0;
  /* cppcheck-suppress variableScope */
  int m_time     = -9999;
  /* cppcheck-suppress variableScope */
  int m_energy   = 0;
  /* cppcheck-suppress variableScope */
  int m_win      = 0;
  /* cppcheck-suppress variableScope */
  int m_revo     = 0;
  /* cppcheck-suppress variableScope */
  int m_caltime  = -9999;

  int tot_ntc          = tc_info.size();
  /* cppcheck-suppress variableScope */
  int evt_ntc          = 0;
  /* cppcheck-suppress variableScope */
  int evt_revo         = -9999;
  /* cppcheck-suppress variableScope */
  int evt_win          = 0;
  /* cppcheck-suppress variableScope */
  int evt_timing       = -9999;
  int evt_cl_theta[6]  = {0};
  int evt_cl_phi[6]    = {0};
  int evt_cl_time[6]   = { -9999};
  int evt_cl_energy[6] = {0};
  /* cppcheck-suppress variableScope */
  int evt_ncl          = 0;
  /* cppcheck-suppress variableScope */
  int evt_low_multi    = 0;
  /* cppcheck-suppress variableScope */
  int evt_b2bhabha_v   = 0;
  /* cppcheck-suppress variableScope */
  int evt_b2bhabha_s   = 0;
  /* cppcheck-suppress variableScope */
  int evt_mumu         = 0;
  /* cppcheck-suppress variableScope */
  int evt_prescale     = 0;
  /* cppcheck-suppress variableScope */
  int evt_icn          = 0;
  /* cppcheck-suppress variableScope */
  int evt_icn_over     = 0;
  /* cppcheck-suppress variableScope */
  int evt_etot_type    = 0;
  /* cppcheck-suppress variableScope */
  int evt_etot         = 0;
  /* cppcheck-suppress variableScope */
  int evt_ecl_bst      = 0;
  /* cppcheck-suppress variableScope */
  int evt_b1_type      = 0;
  /* cppcheck-suppress variableScope */
  int evt_b1bhabha     = 0;
  /* cppcheck-suppress variableScope */
  int evt_physics      = 0;
  /* cppcheck-suppress variableScope */
  int evt_time_type    = 0;
  /* cppcheck-suppress variableScope */
  int evt_etot_all     = 0;
  /* cppcheck-suppress variableScope */
  int evt_time_min     = 0;
  /* cppcheck-suppress variableScope */
  int evt_time_max     = 0;
  /* cppcheck-suppress variableScope */
  int evt_time_win     = 0;
  /* cppcheck-suppress variableScope */
  int etot_i     = 0;
  /* cppcheck-suppress variableScope */
  int etot_c     = 0;
  /* cppcheck-suppress variableScope */
  int etot_f     = 0;
  /* cppcheck-suppress variableScope */
  int cl_tcid = 0;
  /* cppcheck-suppress variableScope */
  int cl_thetaid = 0;
  /* cppcheck-suppress variableScope */
  int cl_phiid = 0;
  /* cppcheck-suppress variableScope */
  int m_clNum    = 0;


  int evt_v_size = evt_2d_vector.size();
  if (evt_v_size != 0) {
    // Sort window : 3 => 4 => 2 => 5 => 1 => 6 => 7
    sort(evt_2d_vector.begin(), evt_2d_vector.end(),
    [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[0] < aa2[0];});
  }

  if (tot_ntc != 0 && flag_checksum == 0 && nnn > 7) {
    if (evt_v_size == 0) {
      // Find most energetic TC timing
      sort(tc_info.begin(), tc_info.end(),
      [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[2] > aa2[2];});
      evt_revo         = win3_revo;
      evt_win          = tc_info[0][3];
      evt_timing       = tc_info[0][1];
      for (int k = 0; k < 6; k++) {
        evt_cl_theta[k]  = 0;
        evt_cl_phi[k]    = 0;
        evt_cl_time[k]   = 0;
        evt_cl_energy[k] = 0;
      }
      evt_ncl          = 0;
      evt_low_multi    = 0;
      evt_b2bhabha_v   = 0;
      evt_b2bhabha_s   = 0;
      evt_mumu         = 0;
      evt_prescale     = 0;
      evt_icn          = 0;
      evt_icn_over     = 0;
      evt_etot_type    = 0;
      evt_etot         = 0;
      evt_ecl_bst      = 0;
      evt_b1_type      = 0;
      evt_b1bhabha     = 0;
      evt_physics      = 0;
      evt_time_type    = 0;
    } else {
      evt_revo   = evt_2d_vector[0][1];
      evt_win    = evt_2d_vector[0][2];
      evt_timing = evt_2d_vector[0][3];
      for (int k = 0; k < 6; k++) {
        evt_cl_theta[k]  = evt_2d_vector[0][4 + k * 4];
        evt_cl_phi[k]    = evt_2d_vector[0][5 + k * 4];
        evt_cl_time[k]   = evt_2d_vector[0][6 + k * 4];
        evt_cl_energy[k] = evt_2d_vector[0][7 + k * 4];
      }
      evt_ncl          = evt_2d_vector[0][28];
      evt_low_multi    = evt_2d_vector[0][29];
      evt_b2bhabha_v   = evt_2d_vector[0][30];
      evt_b2bhabha_s   = evt_2d_vector[0][31];
      evt_mumu         = evt_2d_vector[0][32];
      evt_prescale     = evt_2d_vector[0][33];
      evt_icn          = evt_2d_vector[0][34];
      evt_icn_over     = evt_2d_vector[0][35];
      evt_etot_type    = evt_2d_vector[0][36];
      evt_etot         = evt_2d_vector[0][37];
      evt_ecl_bst      = evt_2d_vector[0][38];
      evt_b1_type      = evt_2d_vector[0][39];
      evt_b1bhabha     = evt_2d_vector[0][40];
      evt_physics      = evt_2d_vector[0][41];
      evt_time_type    = evt_2d_vector[0][42];
    }
    // Sort by TC number
    sort(tc_info.begin(), tc_info.end(),
    [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[0] < aa2[0];});

    for (int ii = 0; ii < tot_ntc; ii++) {
      m_tcid    = tc_info[ii][0];
      m_time    = tc_info[ii][1];
      m_energy  = tc_info[ii][2];
      m_win     = tc_info[ii][3];
      m_revo    = win3_revo;
      m_caltime = m_time - ((evt_win - 3) * 128 + evt_timing);
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

      if (m_win == evt_win || m_win == evt_win + 1) evt_ntc++;
      if (m_win == evt_win - 1) {
        etot_i += m_energy;
      }
      if (m_win == evt_win) {
        etot_c += m_energy;
      }
      if (m_win == evt_win + 1) {
        etot_f += m_energy;
      }
    }

    if (etot_i == 0 && etot_f == 0) {
      evt_etot_all = etot_c;
      evt_time_min =     - evt_timing;
      evt_time_max = 256 - evt_timing;
      evt_time_win = 1;
    } else if (etot_i >= etot_f) {
      evt_etot_all = etot_c + etot_i;
      evt_time_min = -128 - evt_timing;
      evt_time_max =  128 - evt_timing;
      evt_time_win = -1;
    } else {
      evt_etot_all = etot_c + etot_f;
      evt_time_min =     - evt_timing;
      evt_time_max = 256 - evt_timing;
      evt_time_win = 1;
    }

    for (int icluster = 0; icluster < 6; icluster++) {
      if (evt_cl_energy[icluster] == 0 || evt_cl_theta[icluster] == 0 || evt_cl_phi[icluster] == 0) {continue;}
      cl_tcid = mapping.getTCIdFromPosition(evt_cl_theta[icluster], evt_cl_phi[icluster]);
      if (cl_tcid == 0) {continue;}
      cl_thetaid = mapping.getTCThetaIdFromTCId(cl_tcid);
      cl_phiid = mapping.getTCPhiIdFromTCId(cl_tcid);

      m_TRGECLClusterArray.appendNew();
      m_clNum    =  m_TRGECLClusterArray.getEntries() - 1;
      m_TRGECLClusterArray[m_clNum]->setEventId(n_basf2evt);
      m_TRGECLClusterArray[m_clNum]->setClusterId(icluster);
      m_TRGECLClusterArray[m_clNum]->setEventRevo(evt_revo);

      m_TRGECLClusterArray[m_clNum]->setMaxTCId(cl_tcid);  // center of Cluster
      m_TRGECLClusterArray[m_clNum]->setMaxThetaId(cl_thetaid);
      m_TRGECLClusterArray[m_clNum]->setMaxPhiId(cl_phiid);
      m_TRGECLClusterArray[m_clNum]->setClusterId(icluster);
      m_TRGECLClusterArray[m_clNum]->setEnergyDep((double)evt_cl_energy[icluster] * 5.25); // MeV
      m_TRGECLClusterArray[m_clNum]->setTimeAve((double)evt_cl_time[icluster]);
      m_TRGECLClusterArray[m_clNum]->setPositionX(mapping.getTCPosition(cl_tcid).X());
      m_TRGECLClusterArray[m_clNum]->setPositionY(mapping.getTCPosition(cl_tcid).Y());
      m_TRGECLClusterArray[m_clNum]->setPositionZ(mapping.getTCPosition(cl_tcid).Z());
    }
    m_TRGECLEvtArray.appendNew();
    m_evtNum = m_TRGECLEvtArray.getEntries() - 1;
    m_TRGECLEvtArray[m_evtNum]->setEventId(n_basf2evt);
    m_TRGECLEvtArray[m_evtNum]->setETM(etm_version);
    m_TRGECLEvtArray[m_evtNum]->setL1Revo(l1_revo);
    m_TRGECLEvtArray[m_evtNum]->setEvtRevo(evt_revo);
    m_TRGECLEvtArray[m_evtNum]->setEvtWin(evt_win);
    m_TRGECLEvtArray[m_evtNum]->setEvtTime(evt_timing);
    m_TRGECLEvtArray[m_evtNum]->setNTC(evt_ntc);
    m_TRGECLEvtArray[m_evtNum]->setCLTheta(evt_cl_theta);
    m_TRGECLEvtArray[m_evtNum]->setCLPhi(evt_cl_phi);
    m_TRGECLEvtArray[m_evtNum]->setCLTime(evt_cl_time);
    m_TRGECLEvtArray[m_evtNum]->setCLEnergy(evt_cl_energy);
    m_TRGECLEvtArray[m_evtNum]->setNCL(evt_ncl);
    m_TRGECLEvtArray[m_evtNum]->setLowMulti(evt_low_multi);
    m_TRGECLEvtArray[m_evtNum]->set3DBhabhaV(evt_b2bhabha_v);
    m_TRGECLEvtArray[m_evtNum]->set3DBhabhaS(evt_b2bhabha_s);
    m_TRGECLEvtArray[m_evtNum]->setMumu(evt_mumu);
    m_TRGECLEvtArray[m_evtNum]->setPrescale(evt_prescale);
    m_TRGECLEvtArray[m_evtNum]->setICN(evt_icn);
    m_TRGECLEvtArray[m_evtNum]->setICNOver(evt_icn_over);
    m_TRGECLEvtArray[m_evtNum]->setEtotType(evt_etot_type);
    m_TRGECLEvtArray[m_evtNum]->setEtot(evt_etot);
    m_TRGECLEvtArray[m_evtNum]->setECLBST(evt_ecl_bst);
    m_TRGECLEvtArray[m_evtNum]->set2DBhabha(evt_b1bhabha);
    m_TRGECLEvtArray[m_evtNum]->setBhabhaType(evt_b1_type);
    m_TRGECLEvtArray[m_evtNum]->setPhysics(evt_physics);
    m_TRGECLEvtArray[m_evtNum]->setTimeType(evt_time_type);
    m_TRGECLEvtArray[m_evtNum]->setCheckSum(flag_checksum);
    m_TRGECLEvtArray[m_evtNum]->setEvtExist(1);
    m_TRGECLEvtArray[m_evtNum]->setTRGTYPE(trgtype);
    m_TRGECLEvtArray[m_evtNum]->setEtotAll(evt_etot_all);
    m_TRGECLEvtArray[m_evtNum]->setEvtTimeMin(evt_time_min);
    m_TRGECLEvtArray[m_evtNum]->setEvtTimeMax(evt_time_max);
    m_TRGECLEvtArray[m_evtNum]->setEvtTimeWin(evt_time_win);
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
    for (int k = 0; k < 6; k++) {
      evt_cl_theta[k]  = 0;
      evt_cl_phi[k]    = 0;
      evt_cl_time[k]   = -9999;
      evt_cl_energy[k] = 0;
    }
    m_TRGECLEvtArray[m_evtNum]->setCLTheta(evt_cl_theta);
    m_TRGECLEvtArray[m_evtNum]->setCLPhi(evt_cl_phi);
    m_TRGECLEvtArray[m_evtNum]->setCLTime(evt_cl_time);
    m_TRGECLEvtArray[m_evtNum]->setCLEnergy(evt_cl_energy);
    m_TRGECLEvtArray[m_evtNum]->setNCL(0);
    m_TRGECLEvtArray[m_evtNum]->setLowMulti(0);
    m_TRGECLEvtArray[m_evtNum]->set3DBhabhaV(0);
    m_TRGECLEvtArray[m_evtNum]->set3DBhabhaS(0);
    m_TRGECLEvtArray[m_evtNum]->setMumu(0);
    m_TRGECLEvtArray[m_evtNum]->setPrescale(0);
    m_TRGECLEvtArray[m_evtNum]->setICN(0);
    m_TRGECLEvtArray[m_evtNum]->setICNOver(0);
    m_TRGECLEvtArray[m_evtNum]->setEtotType(0);
    m_TRGECLEvtArray[m_evtNum]->setEtot(0);
    m_TRGECLEvtArray[m_evtNum]->setECLBST(0);
    m_TRGECLEvtArray[m_evtNum]->set2DBhabha(0);
    m_TRGECLEvtArray[m_evtNum]->setBhabhaType(0);
    m_TRGECLEvtArray[m_evtNum]->setPhysics(0);
    m_TRGECLEvtArray[m_evtNum]->setTimeType(0);
    m_TRGECLEvtArray[m_evtNum]->setCheckSum(flag_checksum);
    m_TRGECLEvtArray[m_evtNum]->setEvtExist(0);
    m_TRGECLEvtArray[m_evtNum]->setTRGTYPE(trgtype);
    m_TRGECLEvtArray[m_evtNum]->setEtotAll(0);
    m_TRGECLEvtArray[m_evtNum]->setEvtTimeMin(-9999);
    m_TRGECLEvtArray[m_evtNum]->setEvtTimeMax(-9999);
    m_TRGECLEvtArray[m_evtNum]->setEvtTimeWin(0);
  }

  return;
}

