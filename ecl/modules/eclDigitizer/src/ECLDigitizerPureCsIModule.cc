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
#include <ecl/dataobjects/ECLPureCsIInfo.h>
#include <ecl/geometry/ECLGeometryPar.h>

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
  addParam("FirstRing", m_thetaIdMin, "First ring (0-12)", 0);
  addParam("LastRing", m_thetaIdMax, "Last ring (0-12)", 12);
  addParam("Background", m_background, "Flag to use the DigitizerPureCsI configuration with backgrounds; Default is no background",
           false);
  addParam("Calibration", m_calibration,
           "Flag to use the DigitizerPureCsI for Waveform fit Covariance Matrix calibration; Default is false",
           false);
  addParam("adcTickFactor", m_tickFactor, "multiplication factor to get adc tick from trigger tick", 8);
  addParam("sigmaTrigger", m_sigmaTrigger, "Trigger resolution used", 0.);
  addParam("elecNoise", m_elecNoise, "Electronics noise energy equivalent in MeV", 0.5);
  /* photo statistics resolution measurement at LNF sigma = 55 % at 1 MeV
     Csi(Tl) is 12%
  */
  addParam("photostatresolution", m_photostatresolution, "sigma for 1 MeV energy deposit", 0.22);
  addParam("Debug", m_debug, "debug mode on (default off)", false);
  addParam("debugtrgtime", m_testtrg, "set fixed trigger time for testing purposes", 0);
  addParam("debugsigtimeshift", m_testsig, "shift signal arrival time for testing purposes (in microsec)", 0.);
  addParam("debugenergydeposit", m_testenedep, "energy deposit in all crystals for testing purposes", 0.);
  addParam("NoCovMatrix", m_NoCovMatrix, "Use a diagonal (neutral) Covariance matrix", true);

}

ECLDigitizerPureCsIModule::~ECLDigitizerPureCsIModule()
{
}

void ECLDigitizerPureCsIModule::initialize()
{
  m_nEvent  = 0 ;
  EclConfigurationPure::m_tickPure = m_tickFactor * EclConfiguration::m_tick / EclConfiguration::m_ntrg;

  StoreArray<ECLDsp>    ecldsp(eclDspArrayName());
  ecldsp.registerInDataStore();
  StoreArray<ECLDigit> ecldigi(eclDigitArrayName());
  ecldigi.registerInDataStore();

  ecldigi.registerRelationTo(ecldsp);

  StoreArray<ECLPureCsIInfo> eclpurecsiinfo(eclPureCsIInfoArrayName());
  eclpurecsiinfo.registerInDataStore();
  ecldigi.registerRelationTo(eclpurecsiinfo);

  StoreArray<ECLHit> hitList;
  ecldigi.registerRelationTo(hitList);
  readDSPDB();

  m_adc.resize(EclConfigurationPure::m_nch);

  mapGeometry();
}

void ECLDigitizerPureCsIModule::beginRun()
{
}

void ECLDigitizerPureCsIModule::event()
{
  StoreArray<ECLHit> eclHits;

  StoreArray<ECLDigit> eclDigits(eclDigitArrayName());
  StoreArray<ECLDsp> eclDsps(eclDspArrayName());

  StoreArray<ECLPureCsIInfo> eclPureCsIInfos(eclPureCsIInfoArrayName());

  /* add trigger resolution defined in a module paramer
     shifting the waveform starting time by a random deltaT,
     assuming that t0=0 adc channel is determined by the trigger */
  double deltaT = m_sigmaTrigger == 0 ? 0 : gRandom->Gaus(0, m_sigmaTrigger);

  // clear the storage for the event
  memset(m_adc.data(), 0, m_adc.size()*sizeof(adccounts_type));

  // emulate response for ECL hits after ADC measurements
  vector< vector< const ECLHit*> > hitmap(EclConfigurationPure::m_nch);

  for (const auto& eclHit : eclHits) {
    int j = eclHit.getCellId() - 1; //0~8735
    if (isPureCsI(j + 1)) {
      assert(j < EclConfigurationPure::m_nch);
      double hitE       = eclHit.getEnergyDep() / Unit::GeV;
      double hitTime    = eclHit.getTimeAve() / Unit::us;
      if (m_photostatresolution > 0) {
        double nphotavg1MeV = 1 / (m_photostatresolution * m_photostatresolution);
        int nphotavg = round((hitE / 0.001) * nphotavg1MeV);
        int nphot = gRandom->Poisson(nphotavg);
        hitE = (nphot / nphotavg1MeV) / 1000;

        // hitE = gRandom->Gaus(hitE, 0.001 * m_photostatresolution * sqrt(hitE * 1000));
      }
      m_adc[j].AddHit(hitE , hitTime + deltaT, m_ss[m_tbl[j].iss]);
      if (eclHit.getBackgroundTag() == ECLHit::bg_none) hitmap[j].push_back(&eclHit);
    }
  }

  // loop over entire calorimeter

  for (int j = 0; j < EclConfigurationPure::m_nch; j++) {
    if (! isPureCsI(j + 1)) continue;

    if (m_debug) {
      m_adc[j].AddHit(m_testenedep, m_testsig, m_ss[m_tbl[j].iss]);
      //cout << "Adding enedep = " << m_testenedep << " time: " << m_testsig << endl;
    }
    adccounts_type& a = m_adc[j];
    if (! m_calibration && a.total < 0.0001) continue;

    //Noise generation
    float adcNoise[EclConfigurationPure::m_nsmp];
    memset(adcNoise, 0, sizeof(adcNoise));
    if (m_elecNoise > 0) {
      float z[EclConfigurationPure::m_nsmp];
      for (int i = 0; i < EclConfigurationPure::m_nsmp; i++)
        z[i] = gRandom->Gaus(0, 1);
      m_noise[0].generateCorrelatedNoise(z, adcNoise);
    }

    int FitA[EclConfigurationPure::m_nsmp];
    for (int  i = 0; i < EclConfigurationPure::m_nsmp; i++) {
      FitA[i] = 20 * (1000 * a.c[i] + adcNoise[i]) + 3000;
    }

    int  energyFit = 0; // fit output : Amplitude 18 bits
    double    tFit = 0; // fit output : T_ave     12 bits
    int qualityFit = 0; // fit output : quality    2 bits
    double fitChi2 = 0;

    if (! m_calibration) {
      if (m_debug) {
        DSPFitterPure(m_fitparams[m_tbl[j].idn], FitA, m_testtrg, energyFit, tFit, fitChi2, qualityFit);
        /*
        cout << "energy: " << energyFit
              << " tFit: " << tFit
              << " qualityfit: " << qualityFit
              << endl;
        */
      } else {
        DSPFitterPure(m_fitparams[m_tbl[j].idn], FitA, 0, energyFit, tFit, fitChi2, qualityFit);
        /*
        cout << "energy: " << energyFit
        << " tFit: " << tFit
        << " qualityfit: " << qualityFit
        << endl;
        */
      }
    }

    if (m_calibration || energyFit > 0) {
      int CellId = j + 1;
      auto eclDsp = eclDsps.appendNew();
      eclDsp->setCellId(CellId);
      eclDsp->setDspA(FitA);

      auto eclDigit = eclDigits.appendNew();
      eclDigit->setCellId(CellId); // cellId in range from 1 to 8736
      eclDigit->setAmp(energyFit); // E (GeV) = energyFit/20000;
      eclDigit->setTimeFit(int(tFit * 10)); // time is in 0.1 ns units
      eclDigit->setQuality(qualityFit);

      auto AeclPureCsIInfo = eclPureCsIInfos.appendNew();
      eclDigit->addRelationTo(AeclPureCsIInfo);
      AeclPureCsIInfo->setPureCsI(1);
      AeclPureCsIInfo->setCellId(CellId);

      eclDigit->addRelationTo(eclDsp);
      for (const auto& hit : hitmap[j])
        eclDigit->addRelationTo(hit);
    }
  } //store each crystal hit

  // temporary solution to run Pure CsI reconstruction
  // and baseline independently and simultaneously
  // cloning barrel and bwd digits

  StoreArray<ECLDigit> baselineDigits("ECLDigits");
  for (const auto& eclDigit : baselineDigits) {
    int cellid = eclDigit.getCellId();
    if (! isPureCsI(cellid)) {
      auto eclDigitClone = eclDigits.appendNew();
      eclDigitClone->setCellId(cellid);
      eclDigitClone->setAmp(eclDigit.getAmp());
      eclDigitClone->setTimeFit(eclDigit.getTimeFit());
      eclDigitClone->setQuality(eclDigit.getQuality());
      //eclDigitClone->setPureCsI(0);
      auto AeclPureCsIInfo = eclPureCsIInfos.appendNew();
      eclDigitClone->addRelationTo(AeclPureCsIInfo);
      AeclPureCsIInfo->setPureCsI(0);
      AeclPureCsIInfo->setCellId(cellid);
    }
  }


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
  string dataFileName, dataFileName2;
  if (m_background) {
    dataFileName = FileSystem::findFile("/data/ecl/ECL-WF-Pure.root");
    if (! m_calibration)
      dataFileName2 = FileSystem::findFile("/data/ecl/ECL-WF-cov-Pure-BG.root");
    B2INFO("ECLDigitizerPureCsI: Reading configuration data with background from: " << dataFileName);
    B2INFO("ECLDigitizerPureCsI: Reading configuration data with background from: " << dataFileName2);

  } else {
    dataFileName = FileSystem::findFile("/data/ecl/ECL-WF-Pure.root");
    if (! m_calibration)
      dataFileName2 = FileSystem::findFile("/data/ecl/ECL-WF-cov-Pure.root");
    B2INFO("ECLDigitizerPureCsI: Reading configuration data without background from: " << dataFileName);
    B2INFO("ECLDigitizerPureCsI: Reading configuration data without background from: " << dataFileName2);
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

  if (!(m_calibration || m_NoCovMatrix)) {
    TFile rootfile2(dataFileName2.c_str());
    TTree* tree = (TTree*) rootfile2.Get("EclWF");
    ECLWaveformData* eclWFData = new ECLWaveformData;
    const int maxncellid = 512;
    int ncellId;
    vector<int> cellId(maxncellid);//[ncellId] buffer for crystal identification number

    tree->SetBranchAddress("ncellId", &ncellId);
    tree->SetBranchAddress("cellId", cellId.data());
    tree->SetBranchAddress("CovarianceM", &eclWFData);
    for (Long64_t j = 0, jmax = tree->GetEntries(); j < jmax; j++) {
      tree->GetEntry(j);
      assert(ncellId <= maxncellid);
      for (int i = 0; i < ncellId; ++i)
        m_tbl[cellId[i]].idn = m_fitparams.size();
      fitparams_type params;
      eclWFData->getMatrix(params.invC);
      m_fitparams.push_back(params);
    }
  }
  B2INFO("ECLDigitizerPureCsI: parameters vector size : " << m_fitparams.size());
  // at the moment there is one set of fitparams
  if (m_NoCovMatrix) {
    m_fitparams.resize(1);
    for (int i = 0; i < EclConfigurationPure::m_nch; i++)
      m_tbl[i].idn = 0;
    for (int i = 0; i < 16; i++)
      for (int j = 0; j < 16; j++)
        if (i != j) m_fitparams[0].invC[i][j] = 0;
        else m_fitparams[0].invC[i][j] = 1.0;
    initParams(m_fitparams[0], m_ss[0]);
  } else {
    for (auto& param : m_fitparams) {
      initParams(param, m_ss[0]);
    }
  }

  // at the moment same noise for all crystals
  m_noise.resize(1);
  int index = 0;
  for (int i = 0; i < EclConfigurationPure::m_nsmp; i++)
    for (int j = 0; j <= i; j++)
      if (i == j) m_noise[0].setMatrixElement(index++, m_elecNoise);     // units are MeV energy noise eq from electronics
      else m_noise[0].setMatrixElement(index++, 0.);  //uncorrelated

  float testM[31][31];
  m_noise[0].getMatrix(testM);
}

void ECLDigitizerPureCsIModule::mapGeometry()
{
  ECLGeometryPar* eclgeo = ECLGeometryPar::Instance();
  for (int cellId0 = 0; cellId0 < EclConfigurationPure::m_nch; cellId0++) {
    eclgeo->Mapping(cellId0);
    m_thetaID[cellId0] = eclgeo->GetThetaID();
  }
}
