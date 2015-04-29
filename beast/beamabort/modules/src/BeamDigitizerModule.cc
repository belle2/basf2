/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/beamabort/modules/BeamDigitizerModule.h>
#include <beast/beamabort/dataobjects/BeamabortSimHit.h>
#include <beast/beamabort/dataobjects/BeamabortHit.h>

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
using namespace beamabort;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BeamDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BeamDigitizerModule::BeamDigitizerModule() : Module()
{
  // Set module properties
  setDescription("Beamabort digitizer module");

  //Default values are set here. New values can be in BEAMABORT.xml.
  addParam("LookAtRec", m_LookAtRec, "Only look at nuclear recoils", 0);
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

BeamDigitizerModule::~BeamDigitizerModule()
{
}

void BeamDigitizerModule::initialize()
{
  B2INFO("BeamDigitizer: Initializing");
  StoreArray<BeamabortHit>::registerPersistent();

  //get the garfield drift data, gas, and BEAM paramters
  getXMLData();

  //converter: electron number to TOT part I
  fctToT_Calib1 = new TF1("fctToT_Calib1", "[0]*(x/[3]+[1])/(x/[3]+[2])", 0., 100000.);
  fctToT_Calib1->SetParameters(m_TOTA1, m_TOTB1, m_TOTC1, m_TOTQ1);
  //converter: electron number to TOT part II
  fctToT_Calib2 = new TF1("fctToT_Calib2", "[0]*(x/[3]+[1])/(x/[3]+[2])", 0., 100000.);
  fctToT_Calib2->SetParameters(m_TOTA2, m_TOTB2, m_TOTC2, m_TOTQ2);

}

void BeamDigitizerModule::beginRun()
{
}

void BeamDigitizerModule::event()
{

  StoreArray<MCParticle> particles;
  StoreArray<BeamabortSimHit> BeamSimHits;

  //Skip events with no BeamSimHits, but continue the event counter
  if (BeamSimHits.getEntries() == 0) {
    Event++;
    return;
  }

  Event++;

}

//read tube centers, impulse response, and garfield drift data filename from BEAMABORT.xml
void BeamDigitizerModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"BEAMABORT\"]/Content/");

  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    BEAMCenter.push_back(TVector3(activeParams.getLength("BEAMpos_x"), activeParams.getLength("BEAMpos_y"),
                                  activeParams.getLength("BEAMpos_z")));
    nBEAM++;
  }
  m_LookAtRec = content.getDouble("LookAtRec");
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

  B2INFO("BeamDigitizer: Aquired beam locations and gas parameters");
  B2INFO("              from BEAMABORT.xml. There are " << nBEAM << " BEAMs implemented");

}

void BeamDigitizerModule::endRun()
{
}

void BeamDigitizerModule::terminate()
{
}


