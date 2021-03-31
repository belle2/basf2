//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGECLTimingCalibration.cc
// Section  : TRG ECL
// Owner    : YoungJun Kim
// Email    : rladudwns118@korea.ac.kr
//---------------------------------------------------------------
// Description : TRGECL TC time offset calibration Module for TRG ECL
//
//---------------------------------------------------------------
// 1.00 : 2019/06/26 : First version
//---------------------------------------------------------------

#include "trg/ecl/modules/trgeclTimingCal/TrgEclTimingCalibration.h"

#include "TDecompLU.h"
#include "TFile.h"
#include "TTree.h"

using namespace Belle2;
using namespace std;

REG_MODULE(TRGECLTimingCal); // Register module

TRGECLTimingCalModule::TRGECLTimingCalModule() : Module()
{

  setDescription("example module for TRGECL timing calibration");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TRGECLCalSim", fSimulation,
           "0 : Raw data  1 : Simulation data", 0);

  addParam("TRGECLCalType", fCalType,
           "0 : Beam data  1 : Cosmic ray data", 0);

  addParam("TRGECLCal3DBhabhaVeto", f3Dbhabha_veto,
           "0 : Not using 3D bhabha veto bit  1 : Using 3D bhabha veto bit ", 1);

  addParam("TRGECLCalTCRef", TC_ref,
           "Reference TCID. Default setting : 184", 184);

  addParam("TCEnergyCalibrationConstant", TCEnergyCalibrationConstant,
           "A TC energy calibration factor (GeV/ADC), default 0.00525", 0.00525);

  addParam("TRGECLCalnTC", cut_ntc,
           "Number of TC cut : Events will be used < ntc", 999);

  addParam("TRGECLCalHighEnergyCut", cut_high_energy,
           "High energy cut", 9999.0);

  addParam("TRGECLCalLowEnergyCut", cut_low_energy,
           "Low energy cut", 0.0);

  addParam("TRGECLCalFWD", fInclude_FWD,
           "Flag including FWD end cap", 1);

  addParam("TRGECLCalBR", fInclude_BR,
           "Flag including Barrel", 1);

  addParam("TRGECLCalBWD", fInclude_BWD,
           "Flag including BWD end cap", 1);

  addParam("TRGECLCalofname", str_ofilename,
           "Output file name", str_default_name);

  addParam("TRGECLCalIteration", fIteration,
           "iteration", 0);

  addParam("TRGECLCalChisqCut", cut_chisq,
           "iteration chisq cut", 1000.0);

  addParam("TRGECLCalOffsetFname", str_timeoffset_fname,
           "Input offset file name", str_timeoffset_fname);

}

TRGECLTimingCalModule::~TRGECLTimingCalModule()
{
}

void TRGECLTimingCalModule::initialize()
{
  StoreArray<TRGECLHit> TCHit;
  StoreArray<TRGECLUnpackerStore> TCHitUnpacker;
  StoreArray<TRGECLUnpackerEvtStore> TRGECLEvtStore;
  StoreArray<TRGECLTrg> TRGECLTrg;
  //StoreArray<TRGECLCluster> TCCluster;

  if (fSimulation == 1) {
    TCHit.isRequired();
    if (f3Dbhabha_veto == 1) {
      TRGECLTrg.isRequired();
    }
    //StoreArray<TRGECLHit>::registerPersistent();
  } else if (fSimulation == 0) {
    TCHitUnpacker.isRequired();
    if (f3Dbhabha_veto == 1) {
      TRGECLEvtStore.isRequired();
    }
    //StoreArray<TRGECLUnpackerStore>::registerPersistent();
  }

  InitParams();
}

void TRGECLTimingCalModule::beginRun()
{
}

void TRGECLTimingCalModule::endRun()
{
}

void TRGECLTimingCalModule::terminate()
{

  FlagMatrixSolved = Solve_Matrix();
  // -1 : No event for reference TC
  //  0 : Singular matrix
  //  1 : Solvable matrix
  Calculate_Chisq();
  Save_Result();

}

void TRGECLTimingCalModule::event()
{
  nevt_read++;

  if (f3Dbhabha_veto == 1) {
    if (fSimulation == 0) {
      StoreArray<TRGECLUnpackerEvtStore> TRGECLEvtStore;
      int nEvtStore = TRGECLEvtStore.getEntries();
      if (nEvtStore <= 0) return;
      if (TRGECLEvtStore[0] -> getCheckSum() != 0) return;
      if (TRGECLEvtStore[0] -> get3DBhabhaV() != 1)  return;
    }

    if (fSimulation == 1) {
      StoreArray<TRGECLTrg> TRGECLTrg;
      int nEvtStore = TRGECLTrg.getEntries();
      if (nEvtStore <= 0) return;
      if (TRGECLTrg[0] -> get3DBhabha() != 1) return;
    }
  }
  TRGECLTimingCalClear();

  if (ReadData() == 0) return;
  if (FillMatrix() == 1)
    nevt_selected++;

  return;
}

// 0: Skip this event  1: Use this event
int TRGECLTimingCalModule::ReadData()
{

  int ntc;
  if (fSimulation == 1) { // Simulation data : TRGECLHit table
    StoreArray<TRGECLHit> TCHit;
    ntc = TCHit.getEntries();
    if (ntc < 2 || ntc > cut_ntc) return 0;  // ntc cut
    for (int itc = 0; itc < ntc; itc++) {
      TCId.push_back(TCHit[itc]->getTCId());
      TCEnergy.push_back(TCHit[itc]->getEnergyDep());
      TCTiming.push_back((double) TCHit[itc]->getTimeAve());
    }
  } else {              // Raw data : TRGECLUnpackerStore table
    StoreArray<TRGECLUnpackerStore> TCHit;
    ntc = TCHit.getEntries();
    if (ntc < 2 || ntc > cut_ntc) return 0;   // ntc cut
    for (int itc = 0; itc < ntc; itc++) {
      if (TCHit[itc] -> getChecksum() != 0) return 0;
      if (!(TCHit[itc] -> getHitWin() == 3 || TCHit[itc] -> getHitWin() == 4)) continue;
      TCId.push_back(TCHit[itc]->getTCId());
      TCEnergy.push_back((double) TCHit[itc]->getTCEnergy() * TCEnergyCalibrationConstant);
      TCTiming.push_back((double) TCHit[itc]->getTCTime());
    }
  }
  return 1;
}

// 0: Skip this event  1: Use this event
int TRGECLTimingCalModule::FillMatrix()
{
  int flag_event_used = 0;
  int ntc = TCId.size();

  for (int itc1 = 0; itc1 < ntc; itc1++) {
    if (TCEnergy[itc1] > cut_high_energy || TCEnergy[itc1] < cut_low_energy) continue;

    int m_TCID1 = TCId[itc1] - 1;
    if (b_Exclude_TC[m_TCID1] == true) continue;

    for (int itc2 = itc1 + 1; itc2 < ntc; itc2++) {
      int m_TCID2 = TCId[itc2] - 1;
      if (b_Exclude_TC[m_TCID2] == true) continue;
      // energy cut
      if (TCEnergy[itc2] > cut_high_energy || TCEnergy[itc2] < cut_low_energy) continue;
      //double sigma2 = 1.0/(TCEnergy[itc1]*TCEnergy[itc1]) + 1.0/(TCEnergy[itc2]*TCEnergy[itc2]); // sigma i,j or j,i

      double sigma2 = 5.0 * 5.0; // temporal resolution
      double tmp_chisq =  pow((TCTiming[itc1] - TimeOffset[m_TCID1]) - (TCTiming[itc2] - TimeOffset[m_TCID2]), 2) / sigma2;
      if (fIteration == 1 && tmp_chisq > cut_chisq)  continue;

      // Fill symmetric matrix
      _Matrix[m_TCID1][m_TCID2] += (-1.0 / sigma2);
      _Matrix[m_TCID2][m_TCID1] += (-1.0 / sigma2);
      // Fill diagonal component
      _Matrix[m_TCID1][m_TCID1] += (1.0 / sigma2);
      _Matrix[m_TCID2][m_TCID2] += (1.0 / sigma2);

      // Fill Chisq matrix1
      Chisq1[m_TCID1][m_TCID2] += (1.0 / sigma2);
      Chisq1[m_TCID2][m_TCID1] += (1.0 / sigma2);


      // TOF correction & Fill vector component
      double Vector_component = 0;
      if (fCalType == 0) { //no TOF correction for beam calibration
        Vector_component = (TCTiming[itc1] - TCTiming[itc2]);
      }

      else if (fCalType == 1) { //cosmic ray data
        double TOF_TC2TC = (TCPosition[m_TCID1] - TCPosition[m_TCID2]).Mag() / 29.9792458;
        if (TCPosition[m_TCID1].y() > TCPosition[m_TCID2].y()) {
          Vector_component = (TCTiming[itc1] - (TCTiming[itc2] - TOF_TC2TC));
        } else Vector_component = ((TCTiming[itc1] - TOF_TC2TC) - TCTiming[itc2]);
      }
      Vector_component /= sigma2;
      _Vector[m_TCID1] += Vector_component;
      _Vector[m_TCID2] += -Vector_component;
      // Fill Chisq matrix2,3
      Chisq2[m_TCID1][m_TCID2] += -2.0 * Vector_component;
      Chisq2[m_TCID2][m_TCID1] += 2.0 * Vector_component;

      Chisq3[m_TCID1][m_TCID2] += Vector_component * Vector_component * sigma2;
      Chisq3[m_TCID2][m_TCID1] += Vector_component * Vector_component * sigma2;

      Nevent_TC[m_TCID1]++;
      Nevent_TC[m_TCID2]++;
      flag_event_used = 1;
      if (b_Inc_TC[m_TCID2] == false) {b_Inc_TC[m_TCID2] = true;}
    }// for itc2
    if (b_Inc_TC[m_TCID1] == false) {b_Inc_TC[m_TCID1] = true;}
  }// for itc1
  return flag_event_used;
}

int TRGECLTimingCalModule::Solve_Matrix()
{
  tcal_result.clear();
  tcal_result_err.clear();
  tcal_result.resize(576);
  tcal_result_err.resize(576);
  const int nTC_tot = 576;
  TMatrixDSym mat_A(575);
  mat_A.ResizeTo(575, 575, 0.0);
  TVectorD vec_b(575);
  vec_b.ResizeTo(575);

  // copy matrix excluding reference TC
  for (int i = 0; i < 576; i++) {
    if (i == TC_ref - 1) continue;
    int indexi = i;
    if (i > TC_ref - 1) indexi = i - 1;
    vec_b[indexi] = _Vector[i];
    for (int j = 0; j < 576; j++) {
      if (j == TC_ref - 1) continue;
      int indexj = j;
      if (j > TC_ref - 1) indexj = j - 1;
      mat_A[indexi][indexj] = _Matrix[i][j];
    }
  }

  //     error calculation
  //**** Not supported yet ***********//

  // (A^T * A)^-1_ii : error^2 of t_i
  //TMatrixDSym mat_offset_err(mat_A);
  //TMatrixDSym mat_offset_errT = mat_offset_err.Transpose();
  //mat_offset_err.ResizeTo(576,576,0.0);
  //mat_offset_errT.ResizeTo(576,576,0.0);

  //mat_offset_errT.TMult(mat_offset_err);
  //mat_offset_errT.Invert();
  //double *element_mat_offset_err = mat_offset_err.GetMatrixArray();
  for (int i = 0; i < 576; i++) {
    //tcal_result_err[i] = mat_offset_errT[i][i];
    tcal_result_err[i] = 0.0;
  }

  TDecompLU lu(mat_A);
  bool b_solve; // Is this matrix solvable?   0 : no, singular    1 : yes, solvable

  // Solve Ax = b
  TVectorD x = lu.Solve(vec_b, b_solve);
  /* cppcheck-suppress variableScope */
  int ix = 0;
  if (b_solve) {
    for (int i = 0; i < nTC_tot; i++) {
      if (i == TC_ref - 1) {
        tcal_result[i] = 0.0;
      } else {
        tcal_result[i] = x[ix];
        ++ix;
      }
    }
    return 1;
  } else {
    for (int i = 0; i < nTC_tot; i++) {
      tcal_result[i] = -999.0;
      tcal_result_err[i] = -999.0;
    }
    return 0;
  }
}

void TRGECLTimingCalModule::Set_TCposition()
{
  for (int iTCID = 0; iTCID < 576; iTCID++) {
    TrgEclMapping* trgeclMap = new TrgEclMapping();
    TCPosition.push_back(trgeclMap->getTCPosition(iTCID + 1));
    delete trgeclMap;
  }
}

void TRGECLTimingCalModule::InitParams()
{
  nevt_selected = 0;
  nevt_read = 0;

  if (fCalType == 1) {
    Set_TCposition();
  }

  // initialize matrix and vector
  for (int i = 0; i < 576; i++) {
    b_Inc_TC.push_back(false);
  }
  TimeOffset.assign(576, 0.0);
  _Matrix = TMatrixDSym(576);
  _Vector = TVectorD(576);
  _Matrix.ResizeTo(576, 576, 0.0);
  _Vector.ResizeTo(576);
  for (int i = 0; i < 576; i++) {
    _Vector[i] = 0.0;
    for (int j = 0; j < 576; j++) {
      _Matrix[i][j] = 0.0;
    }
  }
  // initialize chisq matrix
  Chisq1.resize(576);
  Chisq2.resize(576);
  Chisq3.resize(576);
  Nevent_TC.assign(576, 0);
  for (int i = 0; i < 576; i++) {
    Chisq1[i].assign(576, 0.0);
    Chisq2[i].assign(576, 0.0);
    Chisq3[i].assign(576, 0.0);
  }

  // Exclude TC setup
  b_Exclude_TC.resize(576);
  for (int i = 0; i < 576; i++) {
    b_Exclude_TC[i] = false;
  }

  if (fInclude_FWD == 0) {
    for (int i = 0; i < 80; i++) {
      b_Exclude_TC[i] = true;
    }
  }
  if (fInclude_BR == 0) {
    for (int i = 80; i < 512; i++) {
      b_Exclude_TC[i] = true;
    }
  }
  if (fInclude_BWD == 0) {
    for (int i = 512; i < 576; i++) {
      b_Exclude_TC[i] = true;
    }
  }

  GetTimeOffset();

}

void TRGECLTimingCalModule::Save_Result()
{
  TString fname = str_ofilename;
  TFile* tf = new TFile(fname, "RECREATE");
  TTree* tr = new TTree("tree", "TC time calibration result");
  vector<double> time_offset;
  vector<double> time_offset_err;
  vector<int> TCID;
  vector<double> chisq;
  vector<int> Nevent;
  time_offset.resize(576);
  time_offset_err.resize(576);
  TCID.resize(576);
  chisq.resize(576);
  Nevent.resize(576);
  double timeoffsetave = 0.0;
  tr -> Branch("TCID", &TCID);
  tr -> Branch("TimeOffset", &time_offset);
  tr -> Branch("TimeOffAve", &timeoffsetave);
  tr -> Branch("TimeOffsetErr", &time_offset_err);
  tr -> Branch("Chisq", &chisq);
  tr -> Branch("Nevent", &Nevent);
  tr -> Branch("FlagSolved", &FlagMatrixSolved);
  tr -> Branch("Matrix", &_Matrix);
  tr -> Branch("Vector", &_Vector);
  tr -> Branch("ChisqComponent1", &Chisq1);
  tr -> Branch("ChisqComponent2", &Chisq2);
  tr -> Branch("ChisqComponent3", &Chisq3);
  tr -> Branch("NeventRead", &nevt_read);
  tr -> Branch("NeventUsed", &nevt_selected);
  for (int i = 0; i < 576; i++) {
    TCID[i] = i + 1;
    time_offset[i] = tcal_result[i];
    timeoffsetave += tcal_result[i];
    time_offset_err[i] = tcal_result_err[i];
    chisq[i] = chisq_result[i];
    Nevent[i] = Nevent_TC[i];
  }
  timeoffsetave /= 576.0;

  tr -> Fill();
  tf -> cd();
  tr -> Write();
  tf -> Close();

}

void TRGECLTimingCalModule::TRGECLTimingCalClear()
{
  TCId.clear();
  TCEnergy.clear();
  TCTiming.clear();

}

void TRGECLTimingCalModule::Calculate_Chisq()
{
  chisq_result.assign(576, 0.0);
  if (FlagMatrixSolved == 1) {
    for (int i = 0; i < 576; i++) {
      if (b_Inc_TC[i] == false && i != TC_ref - 1) continue;
      std::vector<double> offsetdiff;
      offsetdiff.assign(576, 0.0);
      for (int j = 0; j < 576; j++) {
        if (i == j || (b_Inc_TC[j] == false && j != TC_ref - 1)) continue;
        offsetdiff[j] = tcal_result[i] - tcal_result[j];
        chisq_result[i] += Chisq1[i][j] * offsetdiff[j] * offsetdiff[j] + Chisq2[i][j] * offsetdiff[j] + Chisq3[i][j];
      }
    }
  } else {
    for (int i = 0; i < 576; i++) {
      chisq_result[i] = -1.0;
    }
  }
}

void TRGECLTimingCalModule::GetTimeOffset()
{
  if (fIteration != 0) {
    TString str_fname_tmp = str_timeoffset_fname;
    TFile* tf = new TFile(str_fname_tmp, "READ");
    auto tr = (TTree*) tf -> Get("tree");
    vector<double>* TimeOffset_tmp = 0;
    tr -> SetBranchAddress("TimeOffset", &TimeOffset_tmp);
    TimeOffset = *TimeOffset_tmp;
    tr -> GetEntry(0);
    tf -> Close();
  }
}
