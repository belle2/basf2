/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclDigitizer/ECLDigitizerPureCsIModule.h>
#include <ecl/digitization/algorithms.h>
#include <ecl/digitization/shaperdsp.h>
#include <ecl/digitization/ECLDspFitterPure.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLTrig.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

// ROOT
#include <TRandom.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <iostream>

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDigitizerPureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDigitizerPureCsIModule::ECLDigitizerPureCsIModule() : Module()
{
  //Set module properties
  setDescription("Creates ECLDigiHits from ECLHits for Pure CsI.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("Background", m_background, "Flag to use the DigitizerPureCsI configuration with backgrounds; Default is no background",
           false);
  addParam("Calibration", m_calibration,
           "Flag to use the DigitizerPureCsI for Waveform fit Covariance Matrix calibration; Default is false",
           false);
  addParam("adcTickFactor", m_tickFactor, "multiplication factor to get adc tick from trigger tick", 8);
  addParam("sigmaTrigger", m_sigmaTrigger, "Trigger resolution used", 0.020);
  addParam("elecNoise", m_elecNoise, "Electronics noise energy equivalent in MeV", 0.5);
  addParam("Debug", m_debug, "debug mode on (default off)", false);
  addParam("debugtrgtime", m_testtrg, "set fixed trigger time for testing purposes", 0);
  addParam("debugsigtimeshift", m_testsig, "shift signal arrival time for testing purposes (in microsec)", 0.);


}

ECLDigitizerPureCsIModule::~ECLDigitizerPureCsIModule()
{
}

void ECLDigitizerPureCsIModule::initialize()
{
  m_nEvent  = 0 ;
  EclConfigurationPure::m_tickPure = m_tickFactor * EclConfiguration::m_tick / EclConfiguration::m_ntrg;

  StoreArray<ECLDsp>    ecldsp(eclDspArrayName()); ecldsp.registerInDataStore();
  StoreArray<ECLDigit> ecldigi(eclDigitArrayName()); ecldigi.registerInDataStore();

  ecldigi.registerRelationTo(ecldsp);

  StoreArray<ECLHit> hitList;
  ecldigi.registerRelationTo(hitList);
  readDSPDB();

  m_adc.resize(EclConfigurationPure::m_nch);
}

void ECLDigitizerPureCsIModule::beginRun()
{
}

void ECLDigitizerPureCsIModule::event()
{
  //Input Array
  StoreArray<ECLHit> eclHits;
  if (!eclHits) {
    B2DEBUG(100, "ECLHit array is empty in event " << m_nEvent);
  }

  // Output Arrays
  StoreArray<ECLDigit> eclDigits(eclDigitArrayName());
  StoreArray<ECLDsp> eclDsps(eclDspArrayName());

  /* add trigger resolution defined in a module paramer
     shifting the waveform starting time by a random deltaT,
     assuming that t0=0 adc channel is determined by the trigger */

  double deltaT = m_sigmaTrigger == 0 ? 0 : gRandom->Gaus(0, m_sigmaTrigger);


  /*
    Not using trigger time for now
  StoreArray<ECLTrig> eclTrigs;

  double timeInt = 0;
  const double trgtick = EclConfigurationPure::m_tickPure / EclConfigurationPure::m_ntrg;   // trigger tick
  if (eclTrigs.getEntries() == 1) {
    timeInt = eclTrigs[0]->getTimeTrig();
  }
  const double DeltaT = timeInt / trgtick;
  const int      ttrig = int(DeltaT);


  const double trgtick = m_tick / m_ntrg;   // trigger tick
  const double  DeltaT = gRandom->Uniform(0, double(m_ntrg)); // trigger decision can come in any time ???
  const double timeInt = DeltaT * trgtick;
  const int      ttrig = int(DeltaT);

  const auto eclTrig = eclTrigs.appendNew();
  eclTrig->setTimeTrig(timeInt); //t0 (us)= (1520 - m_ltr)*24.*
  */

  // clear the storage for the event
  memset(m_adc.data(), 0, m_adc.size()*sizeof(adccounts_type));

  // emulate response for ECL hits after ADC measurements
  vector< vector< const ECLHit*> > hitmap(EclConfigurationPure::m_nch);
  for (const auto& eclHit : eclHits) {
    int j = eclHit.getCellId() - 1; //0~8735
    if (isPureCsI(j + 1)) {
      double hitE       = eclHit.getEnergyDep() / Unit::GeV;
      double hitTime    = eclHit.getTimeAve() / Unit::us;
      if (m_debug)
        m_adc[j].AddHit(hitE, hitTime + m_testsig, m_ss[m_tbl[j].iss]);
      else
        m_adc[j].AddHit(hitE, hitTime + deltaT, m_ss[m_tbl[j].iss]);
      hitmap[j].push_back(&eclHit);
    }
  }

  // loop over entire calorimeter
  for (int j = 0; j < EclConfigurationPure::m_nch; j++) {
    if (! isPureCsI(j + 1)) continue;
    adccounts_type& a = m_adc[j];

    if (m_calibration) {
      // This has been added by Alex Bobrov for calibration
      // of covariance matrix artificially generate 100 MeV in time for each crystal
      double hitE = 0.1, hitTimeAve = 0.0;
      a.AddHit(hitE, hitTimeAve + m_testsig, m_ss[m_tbl[j].iss]);
    } else if (a.total < 0.0001)
      continue;

    //Skip Noise generation for now

    float z[EclConfigurationPure::m_nsmp];
    for (int i = 0; i < EclConfigurationPure::m_nsmp; i++)
      z[i] = gRandom->Gaus(0, 1);

    float adcNoise[EclConfigurationPure::m_nsmp];
    m_noise[0].generateCorrelatedNoise(z, adcNoise);

    //    if (a.total < .0001) continue;
    if (a.total < .0001) continue;
    int FitA[EclConfigurationPure::m_nsmp];
    for (int  i = 0; i < EclConfigurationPure::m_nsmp; i++) {
      // skip noise for now
      // FitA[i] = 20 * (1000 * a.c[i] + AdcNoise[i]) + 3000;
      FitA[i] = 20 * (1000 * a.c[i] + adcNoise[i]) + 3000;

      // cout << FitA[i] << endl;
    }

    int  energyFit = 0; // fit output : Amplitude 18 bits
    double    tFit = 0; // fit output : T_ave     12 bits
    int qualityFit = 0; // fit output : quality    2 bits
    double fitChi2 = 0;

    if (m_debug)
      DSPFitterPure(m_fitparams[0], FitA, m_testtrg, energyFit, tFit, fitChi2, qualityFit);
    else
      DSPFitterPure(m_fitparams[0], FitA, 0, energyFit, tFit, fitChi2, qualityFit);

    //}

    if (energyFit > 0) {
      int CellId = j + 1;
      const auto eclDsp = eclDsps.appendNew();
      eclDsp->setCellId(CellId);
      eclDsp->setDspA(FitA);

      const auto eclDigit = eclDigits.appendNew();
      eclDigit->setCellId(CellId); // cellId in range from 1 to 8736
      eclDigit->setAmp(energyFit); // E (GeV) = energyFit/20000;
      eclDigit->setTimeFit(int(tFit * 10)); // t0 (us)= (1520 - m_ltr)*24.*12/508/(3072/2) ;
      eclDigit->setQuality(qualityFit);

      eclDigit->addRelationTo(eclDsp);
      for (const auto& hit : hitmap[j])
        eclDigit->addRelationTo(hit);


    }
  } //store each crystal hit

  m_nEvent++;
}

void ECLDigitizerPureCsIModule::endRun()
{
}

void ECLDigitizerPureCsIModule::terminate()
{
}

void ECLDigitizerPureCsIModule::readDSPDB()
{
  string dataFileName;
  if (m_background) {
    dataFileName = FileSystem::findFile("/data/ecl/ECL-WF-Pure.root");
    B2INFO("ECLDigitizerPureCsI: Reading configuration data with background from: " << dataFileName);
  } else {
    // for the moment it is the same file
    dataFileName = FileSystem::findFile("/data/ecl/ECL-WF-Pure.root");
    B2INFO("ECLDigitizerPureCsI: Reading configuration data without background from: " << dataFileName);
  }
  assert(! dataFileName.empty());

  TFile rootfile(dataFileName.c_str());
  const TH1F* sampledWF = dynamic_cast<TH1F*>(rootfile.Get("sampleddsp"));
  assert(sampledWF != nullptr);
  const TH1F* sampledWF1 = dynamic_cast<TH1F*>(rootfile.Get("sampleddsp1"));
  assert(sampledWF1 != nullptr);

  m_tbl.resize(EclConfigurationPure::m_nch);

  // at the moment there is only one sampled signal shape in the pool
  // since all shaper parameters are the same for all crystals
  m_ss.resize(1);
  m_ss[0].InitSample(sampledWF, sampledWF1);

  for (int i = 0; i < EclConfigurationPure::m_nch; i++) m_tbl[i].iss = 0;
  B2INFO("ECLDigitizerPureCsI: " << m_ss.size() << " sampled signal templates were created.");

  rootfile.Close();

  // at the moment there is one set of fitparams
  m_fitparams.resize(1);

  initParams(m_fitparams[0], m_ss[0]);

  // at the moment same noise for all crystals
  m_noise.resize(1);
  int index = 0;
  for (int i = 0; i < EclConfigurationPure::m_nsmp; i++)
    for (int j = 0; j < EclConfigurationPure::m_nsmp; j++)
      if (i <= j) {
        if (i == j) m_noise[0].setMatrixElement(index++, m_elecNoise);     // units are MeV energy noise eq from electronics
        else m_noise[0].setMatrixElement(index++, 0.);  //uncorrelated
      }

}
