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
//---------------------------------------------------------------

#include <trg/ecl/modules/trgeclUnpacker/trgeclUnpackerModule.h>

using namespace std;
using namespace Belle2;

//! Register Module
REG_MODULE(TRGECLUnpacker);

string TRGECLUnpackerModule::version() const
{
  return string("1.00");
}

TRGECLUnpackerModule::TRGECLUnpackerModule()
  : Module::Module(),
    n_basf2evt(0)
{

  string desc = "TRGECLUnpackerModule(" + version() + ")";
  setDescription(desc);
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

  StoreArray<TRGECLUnpackerStore>::registerPersistent();
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

  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      nodeid = ((raw_trgarray[i]->GetNodeID(j)) >> 24) & 0x1F;
      if (nodeid == 0x13) {

        readCOPPEREvent(raw_trgarray[i], j);
        n_basf2evt++;
      }
    }
  }
}

void TRGECLUnpackerModule::readCOPPEREvent(RawTRG* raw_copper, int i)
{

  if (raw_copper->GetDetectorNwords(i, 0) > 0) {
    checkBuffer(raw_copper->GetDetectorBuffer(i, 0));
  }
}

void TRGECLUnpackerModule::checkBuffer(int* rdat)
{

  //  1600 = 12800 /   8
  //   400 = 12800 /  32
  // 12800 = 12672(TC data) + 32(revo + ntc + etot) + 96 (null)
  //    22 = 12672 / 576
  //  1584 = 12682 /   8
  //   144 =  1584 /  11
  unsigned char kdat[1600];
  for (int i = 0; i < 400; i++) {
    kdat[4 * i + 3] =  rdat[i]        & 0xff;
    kdat[4 * i + 2] = (rdat[i] >>  8) & 0xff;
    kdat[4 * i + 1] = (rdat[i] >> 16) & 0xff;
    kdat[4 * i + 0] = (rdat[i] >> 24) & 0xff;
  }
  // kdat
  // to be updated......
  // -------------------
  // 1599 - 1588 : null
  // 1587 - 1584 : summary
  // -------------------
  // 1583 -    0 : TC data

  int data[4];
  int tmp1;
  int tmp2;
  int tmp3;
  int tmp4;

  int ntc        = 0;
  int m_hitNum   = 0;
  int tc_num     = 0;


  int p_flag     = 0;
  int p_time     = 0;
  int p_peak     = 0;

  vector<int> tc_data;
  vector<vector<int>> evt_data;

  evt_data.clear();
  tc_data.clear();

  for (int j = 0; j < 144; j++) {
    tmp1 = kdat[11 * j + 10] & 0xFF;
    tmp2 = kdat[11 * j +  9] & 0xFF;
    tmp3 = kdat[11 * j +  8] & 0xFC;
    data[3] = (tmp1 << 14) + (tmp2 << 6) + (tmp3 >> 2);
    tmp1 = kdat[11 * j +  8] & 0x03;
    tmp2 = kdat[11 * j +  7] & 0xFF;
    tmp3 = kdat[11 * j +  6] & 0xFF;
    tmp4 = kdat[11 * j +  5] & 0xF0;
    data[2] = (tmp1 << 20) + (tmp2 << 12) + (tmp3 << 4) + (tmp4 >> 4);
    tmp1 = kdat[11 * j +  5] & 0x0F;
    tmp2 = kdat[11 * j +  4] & 0xFF;
    tmp3 = kdat[11 * j +  3] & 0xFF;
    tmp4 = kdat[11 * j +  2] & 0xC0;
    data[1] = (tmp1 << 18) + (tmp2 << 10) + (tmp3 << 2) + (tmp4 >> 6);
    tmp1 = kdat[11 * j +  2] & 0x3F;
    tmp2 = kdat[11 * j +  1] & 0xFF;
    tmp3 = kdat[11 * j +  0] & 0xFF;
    data[0] = (tmp1 << 16) + (tmp2 << 8) + tmp3;

    for (int i = 0; i < 4; i++) {
      tc_num = 4 * j + i + 1;
      p_flag = (data[i] >> 21) & 0x1;
      if (p_flag == 1) {
        p_peak  =  data[i] & 0xFFF;
        p_time  = (data[i] >> 12) & 0x1FF; // revo+time
        tc_data.push_back(tc_num);
        tc_data.push_back(p_peak);
        tc_data.push_back(p_time);
        evt_data.push_back(tc_data);
        tc_data.clear();
      }
    }

  }
  ntc = evt_data.size();
  if (ntc != 0) {

    sort(evt_data.begin(), evt_data.end(),
    [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[1] > aa2[1];});

    int tmp_timepeak = 0;
    int tmp_peak = 0;
    int w_time     = -999;

    if (ntc == 1) {
      tmp_peak     = evt_data[0][1];
      tmp_timepeak = evt_data[0][1] * evt_data[0][2];
    } else if (ntc == 2) {
      tmp_peak     = evt_data[0][1] + evt_data[1][1];
      tmp_timepeak = (evt_data[0][1] * evt_data[0][2]) + (evt_data[1][1] * evt_data[1][2]);
    } else {
      tmp_peak     = evt_data[0][1] + evt_data[1][1] + evt_data[2][1];
      tmp_timepeak = (evt_data[0][1] * evt_data[0][2]) + (evt_data[1][1] * evt_data[1][2]) + (evt_data[2][1] * evt_data[2][2]);
    }

    if (tmp_peak != 0) {
      w_time     = (int)tmp_timepeak / tmp_peak;
    }

    sort(evt_data.begin(), evt_data.end(),
    [](const vector<int>& aa1, const vector<int>& aa2) {return aa1[0] < aa2[0];});

    int t_tc      = 0;
    int t_energy  = 0;
    int t_time    = 0;
    int t_caltime = -999;

    for (int i = 0; i < ntc; i++) {
      t_tc       = evt_data[i][0];
      t_energy   = evt_data[i][1];
      t_time     = evt_data[i][2];
      t_caltime  = w_time - t_time;

      StoreArray<TRGECLUnpackerStore> TRGECLUnpackerArray;
      TRGECLUnpackerArray.appendNew();
      m_hitNum = TRGECLUnpackerArray.getEntries() - 1;
      TRGECLUnpackerArray[m_hitNum]->setEventId(n_basf2evt);

      TRGECLUnpackerArray[m_hitNum]->setTCId(t_tc);
      TRGECLUnpackerArray[m_hitNum]->setNTC(ntc);
      TRGECLUnpackerArray[m_hitNum]->setTCEnergy(t_energy);
      TRGECLUnpackerArray[m_hitNum]->setTCTime(t_time);
      TRGECLUnpackerArray[m_hitNum]->setTCCALTime(t_caltime);
    }
  }
  return;
}

string TRGECLUnpackerModule::u_int2string(int u_int, int u_num)
{

  string aaa = "0";
  ostringstream bbb;
  bbb << u_int;
  if (u_num == 0 || u_num == 1) {
    aaa = bbb.str();
  } else if (u_num == 2) {
    if (u_int < 10) { aaa = "0" + bbb.str();}
    else          { aaa =       bbb.str(); }
  } else if (u_num == 3) {
    if (u_int < 10) { aaa = "00" + bbb.str(); }
    else if (u_int < 100) { aaa =  "0" + bbb.str(); }
    else           { aaa =        bbb.str(); }
  } else if (u_num == 4) {
    if (u_int <  10) { aaa = "000" + bbb.str(); }
    else if (u_int < 100) { aaa =  "00" + bbb.str(); }
    else if (u_int < 1000) { aaa =   "0" + bbb.str(); }
    else            { aaa =         bbb.str(); }
  } else if (u_num == 5) {
    if (u_int <   10) { aaa = "0000" + bbb.str(); }
    else if (u_int <  100) { aaa =  "000" + bbb.str(); }
    else if (u_int < 1000) { aaa =   "00" + bbb.str(); }
    else if (u_int < 10000) { aaa =    "0" + bbb.str(); }
    else             { aaa =          bbb.str(); }
  } else if (u_num == 6) {
    if (u_int <    10) { aaa = "00000" + bbb.str(); }
    else if (u_int <   100) { aaa =  "0000" + bbb.str(); }
    else if (u_int <  1000) { aaa =   "000" + bbb.str(); }
    else if (u_int < 10000) { aaa =    "00" + bbb.str(); }
    else if (u_int < 100000) { aaa =     "0" + bbb.str(); }
    else                   { aaa =           bbb.str(); }
  }
  return aaa;
}
