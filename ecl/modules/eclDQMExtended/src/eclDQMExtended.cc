/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 *  ECL Data Quality Monitor (Second Module)                              *
 *                                                                        *
 * This module provides histograms to check out ECL electronics logic     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitry Matvienko (d.v.matvienko@inp.nsk.su)              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//THIS MODULE
#include <ecl/modules/eclDQMExtended/eclDQMExtended.h>

//FRAMEWORK
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationsObject.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/utility/ECLDspUtilities.h>
#include <ecl/utility/ECLDspEmulator.h>

//STL
#include <boost/filesystem.hpp>
#include <regex>
#include <map>
#include <vector>
#include <string>
#include <iostream>

//ROOT
#include <TH1F.h>
#include <TH2F.h>
#include <TDirectory.h>



//NAMESPACE(S)
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDQMEXTENDED)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDQMEXTENDEDModule::ECLDQMEXTENDEDModule()
  : HistoModule(),
    m_ECLDspDataArray0("ECLDSPPars0"),
    m_ECLDspDataArray1("ECLDSPPars1"),
    m_ECLDspDataArray2("ECLDSPPars2"),
    m_calibrationThrA0("ECL_FPGA_LowAmp"),
    m_calibrationThrAhard("ECL_FPGA_HitThresh")

{

  //Set module properties
  setDescription("ECL Data Quality Monitor. Logic Test");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "histogram directory in ROOT file", std::string("ECL"));
  addParam("InitKey", m_InitKey,
           "How to initialize DSP coeffs: ''DB'' or ''File'' are acceptable ", std::string("DB"));
  addParam("DSPDirectoryName", m_DSPDirectoryName,
           "directory for DSP coeffs", std::string("/hsm/belle2/bdata/users/dmitry/dsp/"));
  addParam("RunName", m_RunName,
           "Name of run with DSP files", std::string("run0000/"));

}

ECLDQMEXTENDEDModule::~ECLDQMEXTENDEDModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void ECLDQMEXTENDEDModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;

  // Create a separate histogram directory and cd into it.

  TDirectory* dirDAQ = dynamic_cast<TDirectory*>(oldDir->Get(m_histogramDirectoryName.c_str()));
  if (!dirDAQ) dirDAQ = oldDir->mkdir(m_histogramDirectoryName.c_str());
  dirDAQ->cd();

  //1D histograms creation.

  h_amp_timefail_q0  = new TH1F("amp_timefail_q0", "Amp for time mismatches w/ Quality=0", 1200, 0, 262144);
  h_amp_timefail_q0->GetXaxis()->SetTitle("Amplitude");
  h_amp_timefail_q0->SetOption("LIVE");

  h_amp_timefail_q3  = new TH1F("amp_timefail_q3", "Amp for time mismatches w/ Quality=3", 1200, 0, 262144);
  h_amp_timefail_q3->GetXaxis()->SetTitle("Amplitude");
  h_amp_timefail_q3->SetOption("LIVE");

  h_amp_qf0_qd3  = new TH1F("amp_qf0_qd3", "Amp for QualityFit=0 and QualityData=3", 240, 0, 262144);
  h_amp_qf0_qd3->GetXaxis()->SetTitle("Amplitude");
  h_amp_qf0_qd3->SetOption("LIVE");

  h_amp_qf3_qd0  = new TH1F("amp_qf3_qd0", "Amp for QualityFit=3 and QualityData=0", 240, 0, 262144);
  h_amp_qf3_qd0->GetXaxis()->SetTitle("Amplitude");
  h_amp_qf3_qd0->SetOption("LIVE");

  h_time_qf0_qd3  = new TH1F("time_qf0_qd3", "Time for QualityFit=0 and QualityData=3", 240, -2050, 2050);
  h_time_qf0_qd3->GetXaxis()->SetTitle("Time");
  h_time_qf0_qd3->SetOption("LIVE");

  h_time_qf3_qd0  = new TH1F("time_qf3_qd0", "Time for QualityFit=3 and QualityData=0", 240, -2050, 2050);
  h_time_qf3_qd0->GetXaxis()->SetTitle("Time");
  h_time_qf3_qd0->SetOption("LIVE");

  h_ampfail_quality  = new TH1F("ampfail_quality", "Amp. control flags in bins of QualityData", 5, -1, 4);
  h_ampfail_quality->GetXaxis()->SetTitle("QualityData");
  h_ampfail_quality->SetOption("LIVE");

  h_timefail_quality  = new TH1F("timefail_quality", "Time control flags in bins of QualityData", 5, -1, 4);
  h_timefail_quality->GetXaxis()->SetTitle("QualityData");
  h_timefail_quality->SetOption("LIVE");

  h_ampfail_cellid = new TH1F("ampfail_cellid", "CellIDs w/ failed amps", 8736, 1, 8737);
  h_ampfail_cellid->GetXaxis()->SetTitle("Cell ID");
  h_ampfail_cellid->SetOption("LIVE");

  h_timefail_cellid = new TH1F("timefail_cellid", "CellIDs w/ failed time", 8736, 1, 8737);
  h_timefail_cellid->GetXaxis()->SetTitle("Cell ID");
  h_timefail_cellid->SetOption("LIVE");

  h_qualityfail_cellid = new TH1F("qualityfail_cellid", "CellIDs w/ failed qualities", 8736, 1, 8737);
  h_qualityfail_cellid->GetXaxis()->SetTitle("Cell ID");
  h_qualityfail_cellid->SetOption("LIVE");

  //2D histograms creation.

  h_ampdiff_cellid = new TH2F("ampdiff_cellid", "Amp. diff. (Emulator-Data) for amp. fails", 8736, 1, 8737, 239, -262143, 262143);
  h_ampdiff_cellid->GetXaxis()->SetTitle("Cell ID");
  h_ampdiff_cellid->GetYaxis()->SetTitle("Amplitude difference");
  h_ampdiff_cellid->SetOption("LIVE");

  h_timediff_cellid = new TH2F("timediff_cellid", "Time diff.(Emulator-Data) for time fails", 8736, 1, 8737, 239, -4095, 4095);
  h_timediff_cellid->GetXaxis()->SetTitle("Cell ID");
  h_timediff_cellid->GetYaxis()->SetTitle("Time difference");
  h_timediff_cellid->SetOption("LIVE");

  h_ampdiff_quality = new TH2F("ampdiff_quality", "Amp. diff. (Emulator-Data) for amp. fails in bins of QualityData", 4, 0, 4, 239,
                               -262143, 262143);
  h_ampdiff_quality->GetXaxis()->SetTitle("QualityData");
  h_ampdiff_quality->GetYaxis()->SetTitle("Amplitude difference");
  h_ampdiff_quality->SetOption("LIVE");

  h_timediff_quality = new TH2F("timediff_quality", "Time diff. (Emulator-Data) for time fails in bins of QualityData", 4, 0, 4, 239,
                                -4095, 4095);
  h_timediff_quality->GetXaxis()->SetTitle("QualityData");
  h_timediff_quality->GetYaxis()->SetTitle("Time difference");
  h_timediff_quality->SetOption("LIVE");

  h_ampdiff_shaper = new TH2F("ampdiff_shaper", "Amp. diff. (Emulator-Data) for amp. fails in bins of Shaper Id", 624, 1, 625, 239,
                              -262143, 262143);
  h_ampdiff_shaper->GetXaxis()->SetTitle("Shaper Id");
  h_ampdiff_shaper->GetYaxis()->SetTitle("Amplitude difference");
  h_ampdiff_shaper->SetOption("LIVE");

  h_timediff_shaper = new TH2F("timediff_shaper", "Time diff. (Emulator-Data) for time fails in bins of Shaper Id", 624, 1, 625, 239,
                               -4095, 4095);
  h_timediff_shaper->GetXaxis()->SetTitle("Shaper Id");
  h_timediff_shaper->GetYaxis()->SetTitle("Time difference");
  h_timediff_shaper->SetOption("LIVE");

  h_quality_fit_data = new TH2F("quality_fit_data", "QualityFit vs QualityData for quality fails", 4, 0, 4, 4, 0, 4);
  h_quality_fit_data->GetXaxis()->SetTitle("QualityFit");
  h_quality_fit_data->GetYaxis()->SetTitle("QualityData");
  h_quality_fit_data->SetOption("LIVE");

  h_ampflag_qualityfail = new TH2F("ampflag_qualityfail", "Amp flag (0/1) w/ failed qualities in bins of QualityData", 4, 0, 4, 4, -1,
                                   3);
  h_ampflag_qualityfail->GetXaxis()->SetTitle("QualityData");
  h_ampflag_qualityfail->GetYaxis()->SetTitle("Amp flag");
  h_ampflag_qualityfail->SetOption("LIVE");

  h_timeflag_qualityfail = new TH2F("timeflag_qualityfail", "Time flag (0/1) w/ failed qualities in bins of Quality Data", 4, 0, 4, 4,
                                    -1, 3);
  h_timeflag_qualityfail->GetXaxis()->SetTitle("QualityData");
  h_timeflag_qualityfail->GetYaxis()->SetTitle("Time flag");
  h_timeflag_qualityfail->SetOption("LIVE");

  oldDir->cd();
}


void ECLDQMEXTENDEDModule::initialize()
{
  REG_HISTOGRAM;   // required to register histograms to HistoManager

  StoreArray<ECLDigit> ECLDigits;
  ECLDigits.isRequired();

  StoreArray<ECLTrig> ECLTrigs;
  ECLTrigs.isOptional();

  StoreArray<ECLDsp> ECLDsps;
  ECLDsps.isOptional();

  if (!mapper.initFromDB()) B2FATAL("ECL Display:: Can't initialize eclChannelMapper");

  if (m_InitKey == "DB")  initDspfromDB();
  else if (m_InitKey == "File") initDspfromFile();
  else B2FATAL(" No way to initialize DSP coeffs!!! Please choose InitKey = DB or InitKey = File");

}

void ECLDQMEXTENDEDModule::callbackCalibration(DBObjPtr<ECLCrystalCalib>& cal, std::vector<short int>& constants)
{
  const std::vector<float> intermediate = cal->getCalibVector();
  constants.resize(intermediate.size());
  for (size_t i = 0; i < constants.size(); i++) constants[i] = (short int)intermediate[i];
}


void ECLDQMEXTENDEDModule::callbackCalibration(ECLDspData* dspdata, std::map<std::string, std::vector<short int>>& map1,
                                               std::map<std::string, short int>& map2)
{

  dspdata->getF(map1["F"]);
  dspdata->getF1(map1["F1"]);
  dspdata->getF31(map1["F31"]);
  dspdata->getF32(map1["F32"]);
  dspdata->getF33(map1["F33"]);
  dspdata->getF41(map1["F41"]);
  dspdata->getF43(map1["F43"]);

  map2["k_a"]  = (short int)dspdata->getka();
  map2["k_b"]  = (short int)dspdata->getkb();
  map2["k_c"]  = (short int)dspdata->getkc();
  map2["k_16"] = (short int)dspdata->gety0Startr();
  map2["k_1"]  = (short int)dspdata->getk1();
  map2["k_2"]  = (short int)dspdata->getk2();
  map2["chi_thres"] = dspdata->getchiThresh();
}


int ECLDQMEXTENDEDModule::conversion(int cellID)
{
  int iCrate = mapper.getCrateID(cellID);
  int iShaperPosition = mapper.getShaperPosition(cellID);
  return (iCrate - 1) * 12 + iShaperPosition;

}
short int* ECLDQMEXTENDEDModule::vectorsplit(std::vector<short int>& vectorFrom, int iChannel)
{
  size_t size = vectorFrom.size();
  if (size % 16) B2ERROR("Split is impossible!" << LogVar("Vector size", size));
  return (vectorFrom.data() + (size / 16) * (iChannel - 1));
}


void ECLDQMEXTENDEDModule::initDspfromDB()
{
  size_t iShaper = 0;

  callbackCalibration(m_calibrationThrA0, v_totalthrA0);
  callbackCalibration(m_calibrationThrAhard, v_totalthrAhard);

  for (const auto& dspdata : m_ECLDspDataArray0) { //iCrate = 1, ..., 18
    iShaper++;
    map_vec.clear();
    map_coef.clear();
    ECLDspData* dspointer = &(const_cast<ECLDspData&>(dspdata));
    callbackCalibration(dspointer, map_vec, map_coef);
    map_container_vec[iShaper] = map_vec;
    map_container_coef[iShaper] = map_coef;
  }

  for (const auto& dspdata : m_ECLDspDataArray1) { //iCrate = 19, ..., 36
    iShaper++;
    map_vec.clear();
    map_coef.clear();
    ECLDspData* dspointer = &(const_cast<ECLDspData&>(dspdata));
    callbackCalibration(dspointer, map_vec, map_coef);
    map_container_vec[iShaper] = map_vec;
    map_container_coef[iShaper] = map_coef;
  }

  for (const auto& dspdata : m_ECLDspDataArray2) { //iCrate = 37, ..., 52
    iShaper++;
    if (iShaper < 529) {
      if (iShaper - (iShaper - 1) / 12 * 12 > 10) continue;
    } else {
      if (iShaper - (iShaper - 1) / 12 * 12 > 8) continue;
    }
    map_vec.clear();
    map_coef.clear();
    ECLDspData* dspointer = &(const_cast<ECLDspData&>(dspdata));
    callbackCalibration(dspointer, map_vec, map_coef);
    map_container_vec[iShaper] = map_vec;
    map_container_coef[iShaper] = map_coef;
  }
}

void ECLDQMEXTENDEDModule::initDspfromFile()
{
  const boost::filesystem::path MainDir(m_DSPDirectoryName);
  const boost::filesystem::path RunSubDir(m_RunName);
  const std::regex Filter(".*(crate)([0-9]{2})/.*(dsp)([0-9]{2})(.dat)");
  if (!exists(MainDir / RunSubDir)) B2ERROR("Directory w/ DSP files don't exist" << LogVar("Directory", MainDir / RunSubDir));
  for (boost::filesystem::directory_entry& x : boost::filesystem::recursive_directory_iterator(MainDir / RunSubDir)) {
    if (!std::regex_match(x.path().string(), Filter) || !boost::filesystem::is_regular_file(x.path())) continue;
    int iCrate = atoi(std::regex_replace(x.path().string(), Filter, "$2").c_str());
    int iShaperPosition = atoi(std::regex_replace(x.path().string(), Filter, "$4").c_str());
    int iShaper = (iCrate - 1) * 12 + iShaperPosition;
    if (iCrate > 36 && iCrate < 45) {
      if (iShaperPosition > 10) continue;
    } else if (iCrate > 44) {
      if (iShaperPosition > 8) continue;
    }
    ECLDspData* dspdata = readEclDsp(x.path().string().c_str(), iShaperPosition - 1);
    callbackCalibration(dspdata, map_vec, map_coef);
    map_container_vec[iShaper] = map_vec;
    map_container_coef[iShaper] = map_coef;
    callbackCalibration(m_calibrationThrA0, v_totalthrA0);
    callbackCalibration(m_calibrationThrAhard, v_totalthrAhard);

  }
}

void ECLDQMEXTENDEDModule::emulator(int cellID, int trigger_time, std::vector<int> adc_data)
{

  int iShaper = conversion(cellID);
  int iChannelPosition = mapper.getShaperChannel(cellID);
  short int* f, *f1, *fg41, *fg43, *fg31, *fg32, *fg33;
  int k_a, k_b, k_c, k_1, k_2, k_16, chi_thres;
  int A0, Ahard;

  map_vec = map_container_vec[iShaper];
  f    = vectorsplit(map_vec["F"], iChannelPosition);
  f1   = vectorsplit(map_vec["F1"], iChannelPosition);
  fg31 = vectorsplit(map_vec["F31"], iChannelPosition);
  fg32 = vectorsplit(map_vec["F32"], iChannelPosition);
  fg33 = vectorsplit(map_vec["F33"], iChannelPosition);
  fg41 = vectorsplit(map_vec["F41"], iChannelPosition);
  fg43 = vectorsplit(map_vec["F43"], iChannelPosition);

  map_coef = map_container_coef[iShaper];
  k_a = map_coef["k_a"];
  k_b = map_coef["k_b"];
  k_c = map_coef["k_c"];
  k_1 = map_coef["k_1"];
  k_2 = map_coef["k_2"];
  k_16 = map_coef["k_16"];
  chi_thres = map_coef["chi_thres"];

  A0 = (int)v_totalthrA0[cellID - 1];
  Ahard = (int)v_totalthrAhard[cellID - 1];

  int* y = adc_data.data();
  int ttrig2 = trigger_time - 2 * (trigger_time / 8);


  lftda_(f, f1, fg41, fg43, fg31, fg32, fg33, y, ttrig2, A0, Ahard, k_a, k_b, k_c, k_16, k_1, k_2, chi_thres, m_AmpFit, m_TimeFit,
         m_QualityFit);

  if (m_QualityFit == 2) m_TimeFit = 0;
}

void ECLDQMEXTENDEDModule::beginRun()
{
  h_amp_timefail_q0->Reset();
  h_amp_timefail_q3->Reset();
  h_amp_qf0_qd3->Reset();
  h_amp_qf3_qd0->Reset();
  h_time_qf0_qd3->Reset();
  h_time_qf3_qd0->Reset();
  h_ampfail_quality->Reset();
  h_timefail_quality->Reset();
  h_ampfail_cellid->Reset();
  h_timefail_cellid->Reset();
  h_qualityfail_cellid->Reset();
  h_ampdiff_cellid->Reset();
  h_timediff_cellid->Reset();
  h_ampdiff_quality->Reset();
  h_timediff_quality->Reset();
  h_ampdiff_shaper->Reset();
  h_timediff_shaper->Reset();
  h_quality_fit_data->Reset();
  h_ampflag_qualityfail->Reset();
  h_timeflag_qualityfail->Reset();
}

void ECLDQMEXTENDEDModule::event()
{
  StoreArray<ECLDigit> ECLDigits;

  int iAmpflag_qualityfail = 0;
  int iTimeflag_qualityfail = 0;

  for (auto& aECLDigit : ECLDigits) {
    m_AmpData = aECLDigit.getAmp();
    m_TimeData = aECLDigit.getTimeFit();
    m_QualityData = aECLDigit.getQuality();
    m_CellId = aECLDigit.getCellId();
    m_TrigTime = aECLDigit.getRelationsTo<ECLTrig>()[0]->getTimeTrig();
    ECLDsp* aECLDsp = aECLDigit.getRelated<ECLDsp>();

    if (aECLDsp) {

      std::vector<int> DspArray = aECLDsp->getDspA();
      emulator(m_CellId, m_TrigTime, DspArray);

      if (m_AmpFit != m_AmpData) {
        h_ampfail_quality->Fill(m_QualityData);
        h_ampfail_cellid->Fill(m_CellId);
        h_ampdiff_cellid->Fill(m_CellId, m_AmpFit - m_AmpData);
        h_ampdiff_quality->Fill(m_QualityData, m_AmpFit - m_AmpData);
        h_ampdiff_shaper->Fill(conversion(m_CellId), m_AmpFit - m_AmpData);
      }
      if (m_TimeFit != m_TimeData) {
        if (m_QualityData == 0) h_amp_timefail_q0->Fill(m_AmpData);
        if (m_QualityData == 3) h_amp_timefail_q3->Fill(m_AmpData);
        h_timefail_quality->Fill(m_QualityData);
        h_timefail_cellid->Fill(m_CellId);
        h_timediff_cellid->Fill(m_CellId, m_TimeFit - m_TimeData);
        h_timediff_quality->Fill(m_QualityData, m_TimeFit - m_TimeData);
        h_timediff_shaper->Fill(conversion(m_CellId), m_TimeFit - m_TimeData);
      }
      if (m_QualityFit != m_QualityData) {
        if (m_AmpFit != m_AmpData) iAmpflag_qualityfail = 1;
        if (m_TimeFit != m_TimeData) iTimeflag_qualityfail = 1;
        h_qualityfail_cellid->Fill(m_CellId);
        h_ampflag_qualityfail->Fill(m_QualityData, iAmpflag_qualityfail);
        h_timeflag_qualityfail->Fill(m_QualityData, iTimeflag_qualityfail);
        h_quality_fit_data->Fill(m_QualityFit, m_QualityData);
      }
      h_ampfail_quality->Fill(-1);
      h_timefail_quality->Fill(-1);
    } //aECLDsp
  }  //aECLDigit
} //event


void ECLDQMEXTENDEDModule::endRun()
{
}



void ECLDQMEXTENDEDModule::terminate()
{
}
