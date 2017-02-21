/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/microtpc/modules/TpcDigitizerModule.h>
#include <beast/microtpc/modules/TrackFitter_v2Module.h>
#include <beast/microtpc/dataobjects/MicrotpcSimHit.h>
#include <beast/microtpc/dataobjects/MicrotpcHit.h>
#include <beast/microtpc/dataobjects/MicrotpcDataHit.h>
#include <beast/microtpc/dataobjects/MicrotpcMetaHit.h>
#include <beast/microtpc/dataobjects/MicrotpcMetaEDataHit.h>
#include <beast/microtpc/dataobjects/MicrotpcRecoTrack.h>

#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/RandomNumbers.h>

//c++
#include <cmath>
#include <boost/foreach.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

// ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>


using namespace std;
using namespace Belle2;
using namespace microtpc;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackFitter_v2)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackFitter_v2Module::TrackFitter_v2Module() : Module()
{
  // Set module properties
  setDescription("Track fitter module");

  //Default values are set here. New values can be in MICROTPC.xml.
  addParam("GEMGain1", m_GEMGain1, "GEM1 gain", 10.0);
  addParam("GEMGain2", m_GEMGain2, "GEM1 gain", 20.0);
  addParam("GainRMS1", m_GEMGainRMS1, "GEM1 rms", 0.2);
  addParam("GainRMS2", m_GEMGainRMS2, "GEM1 rms", 0.2);
  addParam("ScaleGain1", m_ScaleGain1, "scale gain 1 by a factor", 2.0);
  addParam("ScaleGain2", m_ScaleGain2, "scale gain 2 by a factor", 4.0);
  addParam("GEMpitch", m_GEMpitch, "GEM pitch", 0.014);
  addParam("PixelThreshold", m_PixelThreshold, "Pixel threshold in [e]", 3000);
  addParam("PixelThresholdRMS", m_PixelThresholdRMS, "Pixel threshold rms in [e]", 150);
  addParam("ChipRowNb", m_ChipRowNb, "Chip number of row", 226);
  addParam("ChipColumnNb", m_ChipColumnNb, "Chip number of column", 80);
  addParam("ChipColumnX", m_ChipColumnX, "Chip x dimension in cm / 2", 1.0);
  addParam("ChipRowY", m_ChipRowY, "Chip y dimension in cm / 2", 0.86);
  addParam("PixelTimeBinNb", m_PixelTimeBinNb, "Pixel number of time bin", 256);
  addParam("PixelTimeBin", m_PixelTimeBin, "Pixel time bin in ns", 25.0);
  addParam("TOTA1", m_TOTA1, "TOT factor A 1", 24.4678);
  addParam("TOTB1", m_TOTB1, "TOT factor B 1", -34.7008);
  addParam("TOTC1", m_TOTC1, "TOT factor C 1", 264.282);
  addParam("TOTQ1", m_TOTQ1, "TOT factor Q 1", 57.);
  addParam("TOTA2", m_TOTA2, "TOT factor A 2", 24.4678);
  addParam("TOTB2", m_TOTB2, "TOT factor B 2", -34.7008);
  addParam("TOTC2", m_TOTC2, "TOT factor C 2", 264.282);
  addParam("TOTQ2", m_TOTQ2, "TOT factor Q 2", 57.);
  addParam("z_DG", m_z_DG, "Drift gap distance [cm]", 12.0);
  addParam("z_TG", m_z_TG, "Transfer gap distance [cm]", 0.28);
  addParam("z_CG", m_z_CG, "Collection gap distance [cm]", 0.34);
  addParam("Dt_DG", m_Dt_DG, "Transverse diffusion in drift gap [cm^-1]", 0.0129286);
  addParam("Dt_TG", m_Dt_TG, "Transverse diffusion in transfer gap [cm^-1]", 0.0153198);
  addParam("Dt_CG", m_Dt_CG, "Transverse diffusion in collection gap [cm^1]", 0.014713);
  addParam("Dl_DG", m_Dl_DG, "Longitudinal diffusion in drift gap [cm^-1]", 0.0124361);
  addParam("Dl_TG", m_Dl_TG, "Longitudinal diffusion in transfer gap distance [cm^-1]", 0.0131141);
  addParam("Dl_CG", m_Dl_CG, "Longitudinal diffusion in collection gap [cm^-1]", 0.0134958);
  addParam("v_DG", m_v_DG, "Drift velocity in gap distance [cm/ns]", 0.00100675);
  addParam("v_TG", m_v_TG, "Drift velocity in transfer gap [cm/ns]", 0.0004079);
  addParam("v_CG", m_v_CG, "Drift velocity in collection gap [cm/ns]", 0.00038828);
  //addParam("P_vessel", m_P_vessel,"Pressure in vessel [atm]",1.0);
  addParam("Workfct", m_Workfct, "Work function", 35.075);
  addParam("Fanofac", m_Fanofac, "Fano factor", 0.19);
  addParam("GasAbs", m_GasAbs, "Gas absorption", 0.05);

}

TrackFitter_v2Module::~TrackFitter_v2Module()
{
}

void TrackFitter_v2Module::initialize()
{
  //trackCandidates;
  //trackCandidates.isRequired();

  B2INFO("TrackFitter_v2: Initializing");

  StoreArray<MicrotpcRecoTrack>::registerPersistent();

  StoreArray<MicrotpcRecoTrack> RecoTracks;
  RecoTracks.isRequired();

  StoreArray<MicrotpcHit> TpcHits;
  TpcHits.isOptional();

  StoreArray<MicrotpcDataHit> TpcDataHits;
  TpcDataHits.isOptional();

  //StoreArray<MicrotpcMetaEDataHit> TpcDataHits;
  //TpcDataHits.isOptional();

  RecoTracks.registerRelationTo(TpcDataHits);
  RecoTracks.registerRelationTo(TpcHits);

  //get the garfield drift data, gas, and TPC paramters
  getXMLData();

  //fctToT_Calib1 = new TF1("fctToT_Calib1", "[0]*(x/[3]+[1])/(x/[3]+[2])", 0., 100000.);
  //fctToT_Calib1->SetParameters(m_TOTA1, m_TOTB1, m_TOTC1, m_TOTQ1);

  //fctToT_Calib2 = new TF1("fctToT_Calib2", "[0]*(x/[3]+[1])/(x/[3]+[2])", 0., 100000.);
  //fctToT_Calib2->SetParameters(m_TOTA2, m_TOTB2, m_TOTC2, m_TOTQ2);

  fctQ_Calib1 = new TF1("fctQ_Calib1", "[0]*([1]*x-[2])/([3]-x)", 0., 13.);
  fctQ_Calib1->SetParameters(m_TOTQ1, m_TOTC1, m_TOTA1 * m_TOTB1, m_TOTA1);

  fctQ_Calib2 = new TF1("fctQ_Calib2", "[0]*([1]*x-[2])/([3]-x)", 0., 13.);
  fctQ_Calib2->SetParameters(m_TOTQ2, m_TOTC2, m_TOTA2 * m_TOTB2, m_TOTA2);

  m_CutTPC1e4_PO210bot = new TCutG("m_CutTPC1e4_PO210bot", 25);
  m_CutTPC1e4_PO210bot->SetPoint(0, -0.8436139, 1.216809);
  m_CutTPC1e4_PO210bot->SetPoint(1, -0.7202467, 1.468415);
  m_CutTPC1e4_PO210bot->SetPoint(2, -0.5424528, 4.990899);
  m_CutTPC1e4_PO210bot->SetPoint(3, -0.1614659, 12.28747);
  m_CutTPC1e4_PO210bot->SetPoint(4, 0.1106676, 17.5712);
  m_CutTPC1e4_PO210bot->SetPoint(5, 0.4662554, 26.37741);
  m_CutTPC1e4_PO210bot->SetPoint(6, 0.6222787, 32.91916);
  m_CutTPC1e4_PO210bot->SetPoint(7, 0.7093614, 38.20289);
  m_CutTPC1e4_PO210bot->SetPoint(8, 0.7529028, 44.74465);
  m_CutTPC1e4_PO210bot->SetPoint(9, 0.7819303, 48.01552);
  m_CutTPC1e4_PO210bot->SetPoint(10, 0.8363571, 48.26713);
  m_CutTPC1e4_PO210bot->SetPoint(11, 0.8436139, 31.66113);
  m_CutTPC1e4_PO210bot->SetPoint(12, 0.8472424, 21.34529);
  m_CutTPC1e4_PO210bot->SetPoint(13, 0.8436139, 15.55835);
  m_CutTPC1e4_PO210bot->SetPoint(14, 0.7855588, 18.57762);
  m_CutTPC1e4_PO210bot->SetPoint(15, 0.738389, 24.11296);
  m_CutTPC1e4_PO210bot->SetPoint(16, 0.5787373, 20.33887);
  m_CutTPC1e4_PO210bot->SetPoint(17, 0.2703193, 10.52623);
  m_CutTPC1e4_PO210bot->SetPoint(18, -0.1433237, 1.720021);
  m_CutTPC1e4_PO210bot->SetPoint(19, -0.527939, -5.073341);
  m_CutTPC1e4_PO210bot->SetPoint(20, -0.6476778, -6.079765);
  m_CutTPC1e4_PO210bot->SetPoint(21, -0.7746735, -6.582977);
  m_CutTPC1e4_PO210bot->SetPoint(22, -0.8617562, -5.576553);
  m_CutTPC1e4_PO210bot->SetPoint(23, -0.8472424, 1.216809);
  m_CutTPC1e4_PO210bot->SetPoint(24, -0.8436139, 1.216809);

  m_CutTPC1e4_PO210top = new TCutG("m_CutTPC1e4_PO210top", 25);
  m_CutTPC1e4_PO210top->SetPoint(0, -0.8169236, -10.40452);
  m_CutTPC1e4_PO210top->SetPoint(1, -0.760302, -13.52468);
  m_CutTPC1e4_PO210top->SetPoint(2, -0.645486, -14.82474);
  m_CutTPC1e4_PO210top->SetPoint(3, -0.4724756, -13.13466);
  m_CutTPC1e4_PO210top->SetPoint(4, -0.266436, -10.0145);
  m_CutTPC1e4_PO210top->SetPoint(5, -0.04466814, -5.724287);
  m_CutTPC1e4_PO210top->SetPoint(6, 0.2164203, -1.564079);
  m_CutTPC1e4_PO210top->SetPoint(7, 0.4366153, 2.856143);
  m_CutTPC1e4_PO210top->SetPoint(8, 0.5844605, 5.456273);
  m_CutTPC1e4_PO210top->SetPoint(9, 0.694558, 7.79639);
  m_CutTPC1e4_PO210top->SetPoint(10, 0.7857817, 7.926396);
  m_CutTPC1e4_PO210top->SetPoint(11, 0.8785782, 6.366318);
  m_CutTPC1e4_PO210top->SetPoint(12, 0.8644228, -4.294215);
  m_CutTPC1e4_PO210top->SetPoint(13, 0.7842089, -4.294215);
  m_CutTPC1e4_PO210top->SetPoint(14, 0.5860333, -8.454423);
  m_CutTPC1e4_PO210top->SetPoint(15, 0.2132746, -13.26466);
  m_CutTPC1e4_PO210top->SetPoint(16, 0.002516512, -17.29487);
  m_CutTPC1e4_PO210top->SetPoint(17, -0.2680088, -22.10511);
  m_CutTPC1e4_PO210top->SetPoint(18, -0.5479711, -24.96525);
  m_CutTPC1e4_PO210top->SetPoint(19, -0.6816609, -25.74529);
  m_CutTPC1e4_PO210top->SetPoint(20, -0.7964769, -25.48528);
  m_CutTPC1e4_PO210top->SetPoint(21, -0.8342246, -24.70524);
  m_CutTPC1e4_PO210top->SetPoint(22, -0.84838, -14.69474);
  m_CutTPC1e4_PO210top->SetPoint(23, -0.8169236, -10.53453);
  m_CutTPC1e4_PO210top->SetPoint(24, -0.8169236, -10.40452);

  m_CutTPC2e3_PO210bot = new TCutG("m_CutTPC2e3_PO210bot", 31);
  m_CutTPC2e3_PO210bot->SetPoint(0, -0.8122051, 14.55068);
  m_CutTPC2e3_PO210bot->SetPoint(1, -0.78704, 18.53199);
  m_CutTPC2e3_PO210bot->SetPoint(2, -0.7115445, 18.53199);
  m_CutTPC2e3_PO210bot->SetPoint(3, -0.6093111, 17.84948);
  m_CutTPC2e3_PO210bot->SetPoint(4, -0.4709028, 16.37071);
  m_CutTPC2e3_PO210bot->SetPoint(5, -0.3089022, 13.75442);
  m_CutTPC2e3_PO210bot->SetPoint(6, -0.0855615, 9.318106);
  m_CutTPC2e3_PO210bot->SetPoint(7, 0.05756527, 5.791806);
  m_CutTPC2e3_PO210bot->SetPoint(8, 0.2478767, 1.810499);
  m_CutTPC2e3_PO210bot->SetPoint(9, 0.4020132, -1.488298);
  m_CutTPC2e3_PO210bot->SetPoint(10, 0.6096257, -5.469605);
  m_CutTPC2e3_PO210bot->SetPoint(11, 0.6788298, -7.17588);
  m_CutTPC2e3_PO210bot->SetPoint(12, 0.7621894, -7.972141);
  m_CutTPC2e3_PO210bot->SetPoint(13, 0.8109468, -7.403383);
  m_CutTPC2e3_PO210bot->SetPoint(14, 0.853413, -4.900847);
  m_CutTPC2e3_PO210bot->SetPoint(15, 0.86285, -1.147043);
  m_CutTPC2e3_PO210bot->SetPoint(16, 0.853413, 1.696747);
  m_CutTPC2e3_PO210bot->SetPoint(17, 0.7763448, 2.834263);
  m_CutTPC2e3_PO210bot->SetPoint(18, 0.5073923, 8.635596);
  m_CutTPC2e3_PO210bot->SetPoint(19, 0.22743, 14.32318);
  m_CutTPC2e3_PO210bot->SetPoint(20, 0.007234977, 18.41824);
  m_CutTPC2e3_PO210bot->SetPoint(21, -0.2271154, 22.7408);
  m_CutTPC2e3_PO210bot->SetPoint(22, -0.4174269, 26.0396);
  m_CutTPC2e3_PO210bot->SetPoint(23, -0.5275244, 27.74587);
  m_CutTPC2e3_PO210bot->SetPoint(24, -0.7288456, 28.31463);
  m_CutTPC2e3_PO210bot->SetPoint(25, -0.8373703, 26.94961);
  m_CutTPC2e3_PO210bot->SetPoint(26, -0.8609626, 19.55575);
  m_CutTPC2e3_PO210bot->SetPoint(27, -0.8562441, 13.98192);
  m_CutTPC2e3_PO210bot->SetPoint(28, -0.8420887, 13.07191);
  m_CutTPC2e3_PO210bot->SetPoint(29, -0.8137779, 13.98192);
  m_CutTPC2e3_PO210bot->SetPoint(30, -0.8122051, 14.55068);

  m_CutTPC2e3_PO210top = new TCutG("m_CutTPC2e3_PO210top", 21);
  m_CutTPC2e3_PO210top->SetPoint(0, -0.858477, 7.478814);
  m_CutTPC2e3_PO210top->SetPoint(1, -0.7327586, 7.669492);
  m_CutTPC2e3_PO210top->SetPoint(2, -0.5639368, 5.572034);
  m_CutTPC2e3_PO210top->SetPoint(3, -0.2298851, -0.5296611);
  m_CutTPC2e3_PO210top->SetPoint(4, 0.01795977, -4.533898);
  m_CutTPC2e3_PO210top->SetPoint(5, 0.2658046, -8.728814);
  m_CutTPC2e3_PO210top->SetPoint(6, 0.4454023, -10.82627);
  m_CutTPC2e3_PO210top->SetPoint(7, 0.5962644, -10.25424);
  m_CutTPC2e3_PO210top->SetPoint(8, 0.7507184, -7.966102);
  m_CutTPC2e3_PO210top->SetPoint(9, 0.8081897, -6.631356);
  m_CutTPC2e3_PO210top->SetPoint(10, 0.8405173, -8.347458);
  m_CutTPC2e3_PO210top->SetPoint(11, 0.8405173, -23.98305);
  m_CutTPC2e3_PO210top->SetPoint(12, 0.7471264, -23.79237);
  m_CutTPC2e3_PO210top->SetPoint(13, 0.5100575, -24.55509);
  m_CutTPC2e3_PO210top->SetPoint(14, 0.2658046, -21.31356);
  m_CutTPC2e3_PO210top->SetPoint(15, -0.1616379, -13.49576);
  m_CutTPC2e3_PO210top->SetPoint(16, -0.5711207, -6.25);
  m_CutTPC2e3_PO210top->SetPoint(17, -0.8081897, -3.008475);
  m_CutTPC2e3_PO210top->SetPoint(18, -0.8800287, -2.245763);
  m_CutTPC2e3_PO210top->SetPoint(19, -0.8477012, 7.288136);
  m_CutTPC2e3_PO210top->SetPoint(20, -0.858477, 7.478814);

  m_CutExtraPO210 = new TCutG("m_CutExtraPO210", 13);
  m_CutExtraPO210->SetPoint(0, -3.916762, 0.1491452);
  m_CutExtraPO210->SetPoint(1, 72.35678, 0.1491452);
  m_CutExtraPO210->SetPoint(2, 148.6303, 0.2675731);
  m_CutExtraPO210->SetPoint(3, 332.0991, 0.6450622);
  m_CutExtraPO210->SetPoint(4, 472.2775, 1.037355);
  m_CutExtraPO210->SetPoint(5, 604.2101, 1.35563);
  m_CutExtraPO210->SetPoint(6, 744.3885, 1.573981);
  m_CutExtraPO210->SetPoint(7, 866.0139, 1.770128);
  m_CutExtraPO210->SetPoint(8, 989.7007, 1.955171);
  m_CutExtraPO210->SetPoint(9, 1039.175, 2.036591);
  m_CutExtraPO210->SetPoint(10, 2.267579, 2.03289);
  m_CutExtraPO210->SetPoint(11, -1.855315, 0.1454443);
  m_CutExtraPO210->SetPoint(12, -3.916762, 0.1491452);


  m_CutTPC3ExtraPO210 = new TCutG("m_CutTPC3ExtraPO210", 11);
  m_CutTPC3ExtraPO210->SetPoint(0, -1.144943, 0.4526023);
  m_CutTPC3ExtraPO210->SetPoint(1, 122.8923, 0.7479061);
  m_CutTPC3ExtraPO210->SetPoint(2, 294.6363, 1.053053);
  m_CutTPC3ExtraPO210->SetPoint(3, 641.3046, 1.481244);
  m_CutTPC3ExtraPO210->SetPoint(4, 895.7401, 1.934043);
  m_CutTPC3ExtraPO210->SetPoint(5, 1032.499, 2.293329);
  m_CutTPC3ExtraPO210->SetPoint(6, 1089.747, 2.677224);
  m_CutTPC3ExtraPO210->SetPoint(7, 1127.912, 3.002058);
  m_CutTPC3ExtraPO210->SetPoint(8, -4.325386, 3.002058);
  m_CutTPC3ExtraPO210->SetPoint(9, -1.144943, 0.4624457);
  m_CutTPC3ExtraPO210->SetPoint(10, -1.144943, 0.4526023);

  m_CutTPC4ExtraPO210 = new TCutG("m_CutTPC4ExtraPO210", 8);
  m_CutTPC4ExtraPO210->SetPoint(0, -4.325386, 0.4476806);
  m_CutTPC4ExtraPO210->SetPoint(1, 170.599, 1.018601);
  m_CutTPC4ExtraPO210->SetPoint(2, 399.5909, 1.599365);
  m_CutTPC4ExtraPO210->SetPoint(3, 689.0113, 2.342546);
  m_CutTPC4ExtraPO210->SetPoint(4, 892.5596, 2.992215);
  m_CutTPC4ExtraPO210->SetPoint(5, -1.144943, 2.997137);
  m_CutTPC4ExtraPO210->SetPoint(6, -7.50583, 0.4722892);
  m_CutTPC4ExtraPO210->SetPoint(7, -4.325386, 0.4476806);
}

void TrackFitter_v2Module::beginRun()
{
}

void TrackFitter_v2Module::event()
{

  StoreArray<MCParticle> particles;
  StoreArray<MicrotpcSimHit> TpcSimHits;
  StoreArray<MicrotpcHit> TpcHits;
  StoreArray<MicrotpcDataHit> TpcDataHits;
  StoreArray<MicrotpcMetaHit> TpcMetaHits;
  //StoreArray<MicrotpcMetaEDataHit> TpcDataHits;

  /*
  //Skip events with no TpcSimHits, but continue the event counter
  if (TpcHits.getEntries() == 0) {
    Event++;
    return;
  }
  */
  int nentries = TpcHits.getEntries();
  int nEntries = TpcDataHits.getEntries();
  //auto columnArray = new vector<int>[nTPC](); //column
  //auto rowArray = new vector<int>[nTPC](); //row
  //auto bcidArray = new vector<int>[nTPC](); //BCID
  //auto totArray = new vector<int>[nTPC](); //TOT
  int detNB = -1;
  if (nentries > 0 || nEntries > 0) {
    //cout << " nTPC " << nTPC << endl;
    if (nEntries > 0) {
      nTPC = TpcMetaHits.getEntries();
    }
    for (int i = 0; i < nTPC; i++) {
      if (nentries > 0) detNB = i;

      for (int j = 0; j < MAXSIZE; j++) {
        x[j] = 0;
        y[j] = 0;
        z[j] = 0;
        e[j] = 0;
      }

      Track = new TGraph2DErrors();

      for (int j = 0; j < 16; j++) m_side[j] = 0;

      int xnpts[4];
      int ynpts[4];
      for (int j = 0; j < 4; j++) {
        m_impact_x[j] = 0;
        m_impact_y[j] = 0;
        xnpts[j] = 0;
        ynpts[j] = 0;
      }

      int fpxhits = 0;
      int m_time_range = 0;
      int m_totsum = 0;
      float m_esum = 0;

      if (nentries > 0) {

        //Determine T0 for each TPC
        m_dchip_map.clear();
        m_dchip.clear();

        for (int j = 0; j < nentries; j++) {
          MicrotpcHit* aHit = TpcHits[j];
          int detNb = aHit->getdetNb();
          int col = aHit->getcolumn();
          int row = aHit->getrow();
          int tot = aHit->getTOT();
          int bcid = aHit->getBCID();
          if (detNb == i) {
            m_dchip_map[std::tuple<int, int>(col, row)] = 1;
            m_dchip[std::tuple<int, int, int>(col, row, bcid)] = tot;
          }
        }

        if (m_dchip_map.size() > 0) {
          fpxhits = m_dchip_map.size();
          //cout << "fpxhits from size " << fpxhits << "  other " << m_dchip.size() << endl;
          int time[fpxhits];
          int itime[fpxhits];

          fpxhits = 0;
          if (m_dchip_map.size() != m_dchip.size()) {
            for (auto& keyValuePair1 : m_dchip_map) {
              const auto& key1 = keyValuePair1.first;
              //column
              int col = std::get<0>(key1);
              //row
              int row = std::get<1>(key1);
              if (m_dchip_map[std::tuple<int, int>(col, row)] == 1) {
                for (auto& keyValuePair2 : m_dchip) {
                  const auto& key2 = keyValuePair2.first;
                  //column
                  int col2 = std::get<0>(key2);
                  //row
                  int row2 = std::get<1>(key2);
                  //bcid
                  int bcid = std::get<2>(key2);
                  //tot
                  int tot = m_dchip[std::tuple<int, int, int>(col2, row2, bcid)];
                  if (tot >= 0 && col2 == col && row2 == row) {
                    time[fpxhits] = bcid;
                  }
                }
                fpxhits++;
              }
            }
          } else {
            for (auto& keyValuePair2 : m_dchip) {
              const auto& key2 = keyValuePair2.first;
              //column
              int col2 = std::get<0>(key2);
              //row
              int row2 = std::get<1>(key2);
              //bcid
              int bcid = std::get<2>(key2);
              //tot
              int tot = m_dchip[std::tuple<int, int, int>(col2, row2, bcid)];
              if (tot >= 0)time[fpxhits] = bcid;
              fpxhits++;
            }
          }
          //cout << " fpxhits " << fpxhits << endl;
          if (fpxhits == 0 || fpxhits > MAXSIZE)continue;

          TMath::Sort(fpxhits, time, itime, false);
          m_time_range = fabs(time[itime[0]] - time[itime[fpxhits - 1]]);
          //cout << " m_time_range " << m_time_range << endl;
          fpxhits = 0;
          m_totsum = 0;
          m_esum = 0;
          for (auto& keyValuePair1 : m_dchip_map) {
            const auto& key1 = keyValuePair1.first;
            //column
            int col = std::get<0>(key1);
            //row
            int row = std::get<1>(key1);
            x[fpxhits] = col * (2. * m_ChipColumnX / (float)m_ChipColumnNb) - m_ChipColumnX + TPCCenter[i].X();
            y[fpxhits] = row * (2. * m_ChipRowY / (float)m_ChipRowNb) - m_ChipRowY + TPCCenter[i].Y();
            if (m_dchip_map[std::tuple<int, int>(col, row)] == 1) {
              for (auto& keyValuePair2 : m_dchip) {
                const auto& key2 = keyValuePair2.first;
                //column
                int col2 = std::get<0>(key2);
                //row
                int row2 = std::get<1>(key2);
                //bcid
                int bcid = std::get<2>(key2);
                //tot
                int tot = m_dchip[std::tuple<int, int, int>(col2, row2, bcid)];
                if (tot >= 0 && col2 == col && row2 == row) {
                  z[fpxhits] = (m_PixelTimeBin / 2. + m_PixelTimeBin * (bcid - time[itime[0]])) * m_v_DG;
                  m_totsum += tot;
                  if (tot < 3) e[fpxhits] = fctQ_Calib1->Eval(tot) / (m_GEMGain1 * m_GEMGain2) * m_Workfct * 1e-3;
                  else e[fpxhits] = fctQ_Calib2->Eval(tot) / (m_GEMGain1 * m_GEMGain2) * m_Workfct * 1e-3;
                  m_esum += e[fpxhits];
                }
              }

              Track->SetPoint(fpxhits, x[fpxhits], y[fpxhits], z[fpxhits]);
              //Track->SetPointError(fpxhits,0,0,e[fpxhits]);
              Track->SetPointError(fpxhits, 0, 0, 1.);

              for (int k = 0; k < 4; k++) {
                if (1 <= col && col <= k + 1)m_side[4 * k + 0] = k + 1;
                if (80 - (k + 1) <= col && col <= 80)m_side[4 * k + 1] = k + 1;
                if (1 <= row && row <= 5 * (k + 1))m_side[4 * k + 2] = k + 1;
                if (336 - 5 * (k + 1) <= row && row <= 336)m_side[4 * k + 3] = k + 1;
              }

              if (col == 1) {
                m_impact_y[0] += y[fpxhits];
                ynpts[0]++;
              }
              if (col == 2) {
                m_impact_y[1] += y[fpxhits];
                ynpts[1]++;
              }
              if (col == 79) {
                m_impact_y[2] += y[fpxhits];
                ynpts[2]++;
              }
              if (col == 80) {
                m_impact_y[3] += y[fpxhits];
                ynpts[3]++;
              }
              if (row == 1) {
                m_impact_x[0] += x[fpxhits];
                xnpts[0]++;
              }
              if (row == 2) {
                m_impact_x[1] += x[fpxhits];
                xnpts[1]++;
              }
              if (row == 335) {
                m_impact_x[2] += x[fpxhits];
                xnpts[2]++;
              }
              if (row == 336) {
                m_impact_x[3] += x[fpxhits];
                xnpts[3]++;
              }

              /*
                    for (int k = 0; k < 4; k++) {
                      if (1 <= col && col <= k + 1)m_side[4 * k + 0] = k + 1;
                      if (80 - (k + 1) <= col && col <= 80)m_side[4 * k + 1] = k + 1;
                      if (1 <= row && row <= 5 * (k + 1))m_side[4 * k + 2] = k + 1;
                      if (336 - 5 * (k + 1) <= row && row <= 336)m_side[4 * k + 3] = k + 1;
                    }
                    if (col == 1) {
                      m_impact_y[0] += y[fpxhits];
                      ynpts[0]++;
                    }
                    if (col == 2) {
                      m_impact_y[1] += y[fpxhits];
                      ynpts[1]++;
                    }
                    if (col == 79) {
                      m_impact_y[2] += y[fpxhits];
                      ynpts[2]++;
                    }
                    if (col == 80) {
                      m_impact_y[3] += y[fpxhits];
                      ynpts[3]++;
                    }
                    if (row == 1) {
                      m_impact_x[0] += x[fpxhits];
                      xnpts[0]++;
                    }
                    if (row == 2) {
                      m_impact_x[1] += x[fpxhits];
                      xnpts[1]++;
                    }
                    if (row == 335) {
                      m_impact_x[2] += x[fpxhits];
                      xnpts[2]++;
                    }
                    if (row == 336) {
                      m_impact_x[3] += x[fpxhits];
                      xnpts[3]++;
                    }
              */
              fpxhits++;
            }
          }
        }
      }

      if (nEntries > 0) {
        MicrotpcMetaHit* aMetaHit = TpcMetaHits[0];
        detNB = aMetaHit->getdetNb() - 1;

        fpxhits = TpcDataHits.getEntries();
        //MicrotpcMetaEDataHit * aHit = TpcDataHits[0];
        //fpxhits = (aHit->getcolumn()).size();
        //cout <<"fpxhits " << fpxhits << endl;
        int time[fpxhits];
        int itime[fpxhits];
        m_totsum = 0;
        m_time_range = 0;
        m_esum = 0;

        for (int j = 0; j < nEntries; j++) {
          //for (int j = 0; j < fpxhits; j++) {
          MicrotpcDataHit* aHit = TpcDataHits[j];
          int bcid = aHit->getBCID();
          time[j] = bcid;
          //time[j] = aHit->getBCID()[j];
        }

        TMath::Sort(fpxhits, time, itime, false);
        m_time_range = fabs(time[itime[0]] - time[itime[fpxhits - 1]]);

        for (int j = 0; j < nEntries; j++) {
          //for (int j = 0; j < fpxhits; j++) {

          MicrotpcDataHit* aHit = TpcDataHits[j];
          int bcid = aHit->getBCID();
          int col = aHit->getcolumn();
          int row = aHit->getrow();
          int tot = aHit->getTOT();
          //int bcid = aHit->getBCID()[j];
          //int col = aHit->getcolumn()[j];
          //int row = aHit->getrow()[j];
          //int tot = aHit->getTOT()[j];
          //x[j] = col * (2. * m_ChipColumnX / (float)m_ChipColumnNb) - m_ChipColumnX + TPCCenter[2].X();
          //y[j] = row * (2. * m_ChipRowY / (float)m_ChipRowNb) - m_ChipRowY + TPCCenter[2].Y();
          x[j] = col * (2. * m_ChipColumnX / (float)m_ChipColumnNb) - m_ChipColumnX;
          y[j] = row * (2. * m_ChipRowY / (float)m_ChipRowNb) - m_ChipRowY;
          z[j] = (m_PixelTimeBin / 2. + m_PixelTimeBin * (bcid - time[itime[0]])) * m_v_DG;
          if (tot < 3) e[j] = fctQ_Calib1->Eval(tot) / (m_GEMGain1 * m_GEMGain2) * m_Workfct * 1e-3;
          else e[j] = fctQ_Calib2->Eval(tot) / (m_GEMGain1 * m_GEMGain2) * m_Workfct * 1e-3;
          m_esum += e[j];
          m_totsum += tot;

          Track->SetPoint(j, x[j], y[j], z[j]);
          //Track->SetPointError(fpxhits,0,0,e[fpxhits]);
          Track->SetPointError(j, 0, 0, 1.);

          for (int k = 0; k < 4; k++) {
            if (1 <= col && col <= k + 1)m_side[4 * k + 0] = k + 1;
            if (80 - (k + 1) <= col && col <= 80)m_side[4 * k + 1] = k + 1;
            if (1 <= row && row <= 5 * (k + 1))m_side[4 * k + 2] = k + 1;
            if (336 - 5 * (k + 1) <= row && row <= 336)m_side[4 * k + 3] = k + 1;
          }

          if (col == 1) {
            m_impact_y[0] += y[j];
            ynpts[0]++;
          }
          if (col == 2) {
            m_impact_y[1] += y[j];
            ynpts[1]++;
          }
          if (col == 79) {
            m_impact_y[2] += y[j];
            ynpts[2]++;
          }
          if (col == 80) {
            m_impact_y[3] += y[j];
            ynpts[3]++;
          }
          if (row == 1) {
            m_impact_x[0] += x[j];
            xnpts[0]++;
          }
          if (row == 2) {
            m_impact_x[1] += x[j];
            xnpts[1]++;
          }
          if (row == 335) {
            m_impact_x[2] += x[j];
            xnpts[2]++;
          }
          if (row == 336) {
            m_impact_x[3] += x[j];
            xnpts[3]++;
          }
        }

      }
      fpxhits = 0;
      if (nentries > 0)fpxhits = m_dchip_map.size();
      else if (nEntries > 0)fpxhits = TpcDataHits.getEntries();

      if (fpxhits > 0) {

        for (int j = 0; j < 4; j++) {
          if (xnpts[j] > 0)m_impact_x[j] = m_impact_x[j] / ((double)xnpts[j]);
          if (ynpts[j] > 0)m_impact_y[j] = m_impact_y[j] / ((double)ynpts[j]);
        }

        L = new float[fpxhits];
        ix = new int[fpxhits];
        iy = new int[fpxhits];
        iz = new int[fpxhits];
        TMath::Sort(fpxhits, x, ix, false);
        TMath::Sort(fpxhits, y, iy, false);
        TMath::Sort(fpxhits, z, iz, false);

        XYZVector temp_vector3(0, 0, 0);
        float pStart[5] = {0, 0, 0, 0, 0};
        if (fpxhits != 0)temp_vector3  = XYZVector(x[ix[fpxhits - 1]] - x[ix[0]] , y[iy[fpxhits - 1]] - y[iy[0]],
                                                     z[iz[fpxhits - 1]] - z[iz[0]]);
        //float init_theta = TPCCenter[i].Theta();//temp_vector3.Theta();
        //float init_phi = TPCCenter[i].Phi();//temp_vector3.Phi();
        float init_theta = temp_vector3.Theta();
        float init_phi = temp_vector3.Phi();

        pStart[0] = x[ix[0]];
        pStart[1] = y[iy[0]];
        pStart[2] = z[iz[0]];
        pStart[3] = init_theta;
        pStart[4] = init_phi;
        /*
        cout <<"fpxhits " << fpxhits
             << " pStart[0] " << pStart[0]
             << " pStart[1] " << pStart[1]
             << " pStart[2] " << pStart[2]
             << " pStart[3] " << pStart[3]
             << " pStart[4] " << pStart[4] << endl;
        */
        TVirtualFitter::SetDefaultFitter("Minuit");
        TVirtualFitter* min = TVirtualFitter::Fitter(0, 5);
        min->SetDefaultFitter("Minuit");
        min->SetObjectFit(Track);
        min->SetFCN(SumDistance2_angles);

        Double_t arglist[6] = { -1, 0, 0, 0, 0, 0};
        min->ExecuteCommand("SET PRINT", arglist, 1);
        min->ExecuteCommand("SET NOWARNINGS", arglist, 0);

        min->SetParameter(0, "x0",    pStart[0], 0.01, 0, 0);
        min->SetParameter(1, "y0",    pStart[1], 0.01, 0, 0);
        min->SetParameter(2, "z0",    pStart[2], 0.01, 0, 0);
        min->SetParameter(3, "theta", pStart[3], 0.0001, 0, TMath::Pi());
        min->SetParameter(4, "phi",   pStart[4], 0.0001, -TMath::Pi(), TMath::Pi());

        arglist[0] = 10000; // number of fucntion calls
        arglist[1] = 0.001; // tolerance
        min->ExecuteCommand("MIGRAD", arglist, 2);

        int nvpar, nparx;
        double amin, edm, errdef;
        min->GetStats(amin, edm, errdef, nvpar, nparx);
        m_chi2 = amin;

        for (int j = 0; j < 5; j++) {
          m_parFit[j] = 0;
          m_parFit_err[j] = 0;
          m_parFit[j] = min->GetParameter(j);
          m_parFit_err[i] = min->GetParError(j);
          for (int k = 0; k < 5; k++) {
            //m_cov[j][k] = 0;
            //m_cov[j][k] = min->GetCovarianceMatrixElement(j, k);
            m_cov[j * 5 + k] = 0;
            m_cov[j * 5 + k] = min->GetCovarianceMatrixElement(j, k);
          }
        }

        TVector3 TrackDir(1, 0, 0);
        TrackDir.SetTheta(m_parFit[3]);
        TrackDir.SetPhi(m_parFit[4]);

        m_theta = m_parFit[3] * TMath::RadToDeg();
        m_phi   = m_parFit[4] * TMath::RadToDeg();

        for (int j = 0; j < fpxhits; j++) {
          TVector3 Point(x[j], y[j], z[j]);
          L[j] = Point * TrackDir.Unit();
        }

        iL = new int [fpxhits];
        TMath::Sort(fpxhits, L, iL, false);
        m_trl = fabs(L[iL[fpxhits - 1]] - L[iL[0]]);

        for (int j = 0; j < 6; j++)m_partID[j] = 0;
        Bool_t GoodRawXray = false;
        if (fpxhits < 10) {
          GoodRawXray = true;
          m_partID[0] = 1;
        }
        Bool_t EdgeCuts = false;
        if (m_side[0] == 0 && m_side[1] == 0 && m_side[2] == 0 && m_side[3] == 0) EdgeCuts = true;
        //Bool_t GoodXray = false;
        if (EdgeCuts && GoodRawXray) {
          //GoodXray = true;
          m_partID[1] = 1;
        }
        Bool_t StillAlpha = false;
        //StillAlpha = m_CutExtraPO210->IsInside(m_esum, m_trl);
        Bool_t IsTopSource = false;
        Bool_t IsBotSource = false;
        if (detNB == 0 || detNB == 3) {
          IsBotSource = m_CutTPC1e4_PO210bot->IsInside(m_impact_y[0], m_phi);
          IsTopSource = m_CutTPC1e4_PO210top->IsInside(m_impact_y[0], m_phi);
          StillAlpha = m_CutTPC4ExtraPO210->IsInside(m_esum, m_trl);
        } else if (detNB == 1 || detNB == 2) {
          IsBotSource = m_CutTPC2e3_PO210bot->IsInside(m_impact_y[3], m_phi);
          IsTopSource = m_CutTPC2e3_PO210top->IsInside(m_impact_y[3], m_phi);
          StillAlpha = m_CutTPC3ExtraPO210->IsInside(m_esum, m_trl);
        }
        if (StillAlpha) m_partID[2] = 1;
        if (IsBotSource) m_partID[3] = 1;
        if (IsTopSource) m_partID[4] = 1;

        //Bool_t GoodNeutron = false;
        if (EdgeCuts && fpxhits > 10  && m_esum > 10.0 && !StillAlpha) {
          //GoodNeutron = true;
          m_partID[5] = 1;
        }

        StoreArray<MicrotpcRecoTrack> RecoTracks;
        RecoTracks.appendNew(i, fpxhits, m_chi2, m_theta, m_phi, m_esum, m_totsum, m_trl, m_time_range, m_parFit, m_parFit_err, m_cov,
                             m_impact_x, m_impact_y, m_side, m_partID);




        delete [] iL;
        delete min;
        delete [] L;
        delete [] ix;
        delete [] iy;
        delete [] iz;

        Track->Delete();
      }
    }
  }

  //Event++;

}

//double TrackFitterModule::distance2_angles(double px,double py,double pz, double *p)
double distance2_angles_v2(double px, double py, double pz, double* p)
{
  XYZVector xp(px, py, pz);
  XYZVector x0(p[0], p[1], p[2]);
  XYZVector u(TMath::Sin(p[3])*TMath::Cos(p[4]), TMath::Sin(p[3])*TMath::Sin(p[4]), TMath::Cos(p[3]));
  double coeff = u.Dot(xp - x0);
  XYZVector n = xp - x0 - coeff * u;

  double dx = n.x();
  double dy = n.y();
  double dz = n.z();
  double d2_x = TMath::Power(dx / 1., 2);
  double d2_y = TMath::Power(dy / 1., 2);
  double d2_z = TMath::Power(dz / 1., 2);
  double d2 = d2_x + d2_y + d2_z;

  return d2;
}


//MyClass::SumDistance2_angles(int &, double *, double & sum, double * par, int )
void SumDistance2_angles(int&, double*, double& sum, double* par, int)
{
  TGraph2DErrors* gr = dynamic_cast<TGraph2DErrors*>((TVirtualFitter::GetFitter())->GetObjectFit());
  assert(gr != 0);

  double* xp = gr->GetX();
  double* yp = gr->GetY();
  double* zp = gr->GetZ();
  double* ep = gr->GetEZ();
  int npoints = gr->GetN();

  sum = 0;

  for (int i = 0; i < npoints; i++) {
    double w = 1.;
    if (ep[i] > 0.)w = ep[i];
    double d = w * distance2_angles_v2(xp[i], yp[i], zp[i], par);
    sum += d;
  }

}

//read tube centers, impulse response, and garfield drift data filename from MICROTPC.xml
void TrackFitter_v2Module::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"MICROTPC\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    TPCCenter.push_back(TVector3(activeParams.getLength("TPCpos_x"), activeParams.getLength("TPCpos_y"),
                                 activeParams.getLength("TPCpos_z")));
    nTPC++;
  }

  m_GEMGain1 = content.getDouble("GEMGain1");
  m_GEMGain2 = content.getDouble("GEMGain2");
  m_GEMGainRMS1 = content.getDouble("GEMGainRMS1");
  m_GEMGainRMS2 = content.getDouble("GEMGainRMS2");
  m_ScaleGain1 = content.getDouble("ScaleGain1");
  m_ScaleGain2 = content.getDouble("ScaleGain2");
  m_GEMpitch = content.getDouble("GEMpitch");
  m_PixelThreshold = content.getInt("PixelThreshold");
  m_PixelThresholdRMS = content.getInt("PixelThresholdRMS");
  m_PixelTimeBinNb = content.getInt("PixelTimeBinNb");
  m_PixelTimeBin = content.getDouble("PixelTimeBin");
  m_ChipColumnNb = content.getInt("ChipColumnNb");
  m_ChipRowNb = content.getInt("ChipRowNb");
  m_ChipColumnX = content.getDouble("ChipColumnX");
  m_ChipRowY = content.getDouble("ChipRowY");
  m_TOTA1 = content.getDouble("TOTA1");
  m_TOTB1 = content.getDouble("TOTB1");
  m_TOTC1 = content.getDouble("TOTC1");
  m_TOTQ1 = content.getDouble("TOTQ1");
  m_TOTA2 = content.getDouble("TOTA2");
  m_TOTB2 = content.getDouble("TOTB2");
  m_TOTC2 = content.getDouble("TOTC2");
  m_TOTQ2 = content.getDouble("TOTQ2");
  m_z_DG = content.getDouble("z_DG");
  m_z_TG = content.getDouble("z_TG");
  m_z_CG = content.getDouble("z_CG");
  m_Dl_DG = content.getDouble("Dl_DG");
  m_Dl_TG = content.getDouble("Dl_TG");
  m_Dl_CG = content.getDouble("Dl_CG");
  m_Dt_DG = content.getDouble("Dt_DG");
  m_Dt_TG = content.getDouble("Dt_TG");
  m_Dt_CG = content.getDouble("Dt_CG");
  m_v_DG = content.getDouble("v_DG");
  m_v_TG = content.getDouble("v_TG");
  m_v_CG = content.getDouble("v_CG");
  m_Workfct = content.getDouble("Workfct");
  m_Fanofac = content.getDouble("Fanofac");
  m_GasAbs = content.getDouble("GasAbs");

  B2INFO("TrackFitter_v2: Aquired tpc locations and gas parameters");
  B2INFO("              from MICROTPC.xml. There are " << nTPC << " TPCs implemented");

}

void TrackFitter_v2Module::endRun()
{
}

void TrackFitter_v2Module::terminate()
{
}


