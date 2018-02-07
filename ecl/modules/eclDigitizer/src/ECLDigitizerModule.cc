/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *               Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Alex Bobrov                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclDigitizer/ECLDigitizerModule.h>
#include <ecl/digitization/algorithms.h>
#include <ecl/digitization/shaperdsp.h>
#include <ecl/digitization/ECLCompress.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>
#include <framework/database/DBObjPtr.h>

// ROOT
#include <TRandom.h>
#include <TFile.h>
#include <TTree.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDigitizerModule::ECLDigitizerModule() : Module()
{
  //Set module properties
  setDescription("Creates ECLDigiHits from ECLHits.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("Background", m_background, "Flag to use the Digitizer configuration with backgrounds; Default is no background", false);
  addParam("Calibration", m_calibration, "Flag to use the Digitizer for Waveform fit Covariance Matrix calibration; Default is false",
           false);
  addParam("DiodeDeposition", m_inter,
           "Flag to take into account energy deposition in photodiodes; Default diode is not sensitive detector", false);
  addParam("WaveformMaker", m_waveformMaker, "Flag to produce background waveform digits", false);
  addParam("CompressionAlgorithm", m_compAlgo, "Waveform compression algorithm", 0u);
  addParam("eclWaveformsName", m_eclWaveformsName, "Name of the output/input collection (digitized waveforms)", string(""));
  addParam("HadronPulseShapes", m_HadronPulseShape, "Flag to include hadron component in pulse shape construction.", false);
}

ECLDigitizerModule::~ECLDigitizerModule()
{
}

void ECLDigitizerModule::initialize()
{
  m_eclDsps.registerInDataStore();
  m_eclDigits.registerInDataStore();
  m_eclTrigs.registerInDataStore();

  m_eclDiodeHits.registerInDataStore("ECLDiodeHits");

  m_eclDigits.registerRelationTo(m_eclHits);
  if (m_waveformMaker)
    m_eclWaveforms.registerInDataStore(m_eclWaveformsName);

  readDSPDB();

  m_adc.resize(EclConfiguration::m_nch);

  EclConfiguration::get().setBackground(m_background);
}

void ECLDigitizerModule::beginRun()
{
  const EclConfiguration& ec = EclConfiguration::get();
  DBObjPtr<ECLCrystalCalib>
  Ael("ECLCrystalElectronics"),
      Aen("ECLCrystalEnergy"),
      Tel("ECLCrystalElectronicsTime"),
      Ten("ECLCrystalTimeOffset"),
      Tmc("ECLMCTimeOffset");
  double ns_per_tick = 1.0 / (4.0 * ec.m_rf) * 1e3;// ~0.49126819903043308239 ns/tick

  calibration_t def = {1, 0};
  m_calib.assign(8736, def);

  if (Ael) for (int i = 0; i < 8736; i++) m_calib[i].ascale /= Ael->getCalibVector()[i];
  if (Aen) for (int i = 0; i < 8736; i++) m_calib[i].ascale /= Aen->getCalibVector()[i] * 20000.0;

  if (Tel) for (int i = 0; i < 8736; i++) m_calib[i].tshift += Tel->getCalibVector()[i] * ns_per_tick;
  if (Ten) for (int i = 0; i < 8736; i++) m_calib[i].tshift += Ten->getCalibVector()[i] * ns_per_tick;
  if (Tmc) for (int i = 0; i < 8736; i++) m_calib[i].tshift += Tmc->getCalibVector()[i] * ns_per_tick;
}

// interface to C shape fitting function function
void ECLDigitizerModule::shapeFitterWrapper(const int j, const int* FitA, const int ttrig,
                                            int& m_lar, int& m_ltr, int& m_lq, int& m_chi) const
{
  const int n16 = 16; // number of points before signal n16 = 16
  const crystallinks_t& t = m_tbl[j]; //lookup table [0,8735]
  const fitparams_t& r = m_fitparams[t.ifunc];
  shapeFitter((short int*)m_idn[t.idn].id, (int*)r.f, (int*)r.f1, (int*)r.fg41, (int*)r.fg43,
              (int*)r.fg31, (int*)r.fg32, (int*)r.fg33,
              (int*)FitA, (int*)&ttrig, (int*)&n16,
              &m_lar, &m_ltr, &m_lq , &m_chi);
}

int ECLDigitizerModule::shapeSignals()
{
  const EclConfiguration& ec = EclConfiguration::get();
  ECLGeometryPar* eclp = ECLGeometryPar::Instance();

  const double trgtick = ec.s_clock / ec.m_rf / ec.m_ntrg;   // trigger tick
  const int  DeltaT = gRandom->Uniform(0, double(ec.m_ntrg) / 2.); // trigger decision can come in any time ???
  const double timeInt = 2.* (double) DeltaT * trgtick;
  const int      ttrig = 2 * DeltaT;
  const double timeOffset = timeInt - ec.s_clock / (2 * ec.m_rf);

  const auto eclTrig = m_eclTrigs.appendNew();
  eclTrig->setTimeTrig(timeInt); //t0 (us)= (1520 - m_ltr)*24.*

  // clear the storage for the event
  memset(m_adc.data(), 0, m_adc.size()*sizeof(adccounts_t));

  const double E2GeV = 1 / Unit::GeV; // convert Geant energy units to GeV
  const double T2us = 1 / Unit::us; // convert Geant time units to microseconds

  // emulate response for ECL hits after ADC measurements
  for (const auto& hit : m_eclSimHits) {
    int j = hit.getCellId() - 1; //0~8735
    double hitE       = hit.getEnergyDep() * m_calib[j].ascale * E2GeV;
    double hitTimeAve = (hit.getFlightTime() + m_calib[j].tshift + eclp->time2sensor(j, hit.getPosition())) * T2us;
    if (m_HadronPulseShape == true) {
      double hitHadronE       = hit.getHadronEnergyDep() * m_calib[j].ascale * E2GeV;
      m_adc[j].AddHit(hitE - hitHadronE, hitTimeAve + timeOffset, m_ss[2]);//Gamma Component
      m_adc[j].AddHit(hitHadronE, hitTimeAve + timeOffset, m_ss[3]); //Hadron Component
      m_adc[j].totalHadron += hit.getHadronEnergyDep();
    } else {
      m_adc[j].AddHit(hitE, hitTimeAve + timeOffset, m_ss[m_tbl[j].iss]);
    }
  }

  // add only background hits
  for (const auto& hit : m_eclHits) {
    if (hit.getBackgroundTag() == ECLHit::bg_none) continue;
    int j = hit.getCellId() - 1; //0~8735
    double hitE       = hit.getEnergyDep() * m_calib[j].ascale * E2GeV;
    double hitTimeAve = (hit.getTimeAve() + m_calib[j].tshift) * T2us;
    m_adc[j].AddHit(hitE, hitTimeAve + timeOffset, m_ss[m_tbl[j].iss]);
  }

  // internuclear counter effect -- charged particle crosses diode and produces signal
  if (m_inter) {
    // ionisation energy in Si is I = 3.6x10^-6 MeV for electron-hole pair
    // 5000 pairs in the diode per 1 MeV deposited in the crystal attached to the diode
    // conversion factor to get equvalent energy deposition in the crystal to sum up it with deposition in crystal
    const double diodeEdep2crystalEdep = E2GeV * (1 / (5000 * 3.6e-6));
    for (const auto& hit : m_eclDiodeHits) {
      int j = hit.getCellId() - 1; //0~8735
      double hitE       = hit.getEnergyDep() * m_calib[j].ascale * diodeEdep2crystalEdep;
      double hitTimeAve = (hit.getTimeAve() + m_calib[j].tshift) * T2us;

      adccounts_t& a = m_adc[j];
      // cout << "internuclearcountereffect " << j << " " << hit.getEnergyDep() << " " << hit.getTimeAve() << " " << a.total << endl;
      // for (int  i = 0; i < ec.m_nsmp; i++) cout << i << " " << a.c[i] << endl;
      a.AddHit(hitE, hitTimeAve + timeOffset, m_ss[1]); // m_ss[1] is the sampled diode response
      //    for (int  i = 0; i < ec.m_nsmp; i++) cout << i << " " << a.c[i] << endl;
    }
  }

  if (m_calibration) {
    // This has been added by Alex Bobrov for calibration
    // of covariance matrix artificially generate 100 MeV in time for each crystal
    double hitE = 0.1, hitTimeAve = 0.0;
    for (int j = 0; j < ec.m_nch; j++)
      m_adc[j].AddHit(hitE, hitTimeAve + timeOffset, m_ss[m_tbl[j].iss]);
  }

  return ttrig;
}

void ECLDigitizerModule::makeElectronicNoiseAndPedestal(int J, int* FitA)
{
  const EclConfiguration& ec = EclConfiguration::get();
  float z[ec.m_nsmp], AdcNoise[ec.m_nsmp]; // buffers with electronic noise
  // Noise generation
  for (int i = 0; i < ec.m_nsmp; i++) z[i] = gRandom->Gaus(0, 1);
  m_noise[m_tbl[J].inoise].generateCorrelatedNoise(z, AdcNoise);
  for (int i = 0; i < ec.m_nsmp; i++) FitA[i] = 20 * AdcNoise[i] + 3000;
}

void ECLDigitizerModule::makeWaveforms()
{
  const EclConfiguration& ec = EclConfiguration::get();
  BitStream out(ec.m_nch * ec.m_nsmp * 18 / 32);
  out.putNBits(m_compAlgo & 0xff, 8);
  ECLCompress* comp = selectAlgo(m_compAlgo & 0xff);
  if (comp == NULL)
    B2FATAL("Unknown compression algorithm: " << m_compAlgo);

  int FitA[ec.m_nsmp]; // buffer for the waveform fitter
  // loop over entire calorimeter
  for (int j = 0; j < ec.m_nch; j++) {
    adccounts_t& a = m_adc[j];
    makeElectronicNoiseAndPedestal(j, FitA);
    for (int  i = 0; i < ec.m_nsmp; i++) {
      int A = 20000 * a.c[i] + FitA[i];
      FitA[i] = max(0, min(A, (1 << 18) - 1));
    }
    comp->compress(out, FitA);
  }
  out.resize();

  ECLWaveforms* wf = new ECLWaveforms;
  m_eclWaveforms.assign(wf);

  std::swap(out.getStore(), wf->getStore());
  if (comp) delete comp;
}

void ECLDigitizerModule::event()
{
  const EclConfiguration& ec = EclConfiguration::get();
  const int ttrig = shapeSignals();

  // We want to produce background waveforms in simulation first than
  // dump to a disk, read from the disk to test before real data
  if (m_waveformMaker) { makeWaveforms(); return; }

  // make relation between cellid and eclhits
  struct ch_t {int cell, id;};
  vector<ch_t> hitmap;
  for (const auto& hit : m_eclHits) {
    int j = hit.getCellId() - 1; //0~8735
    if (hit.getBackgroundTag() == ECLHit::bg_none) hitmap.push_back({j, hit.getArrayIndex()});
    //    cout<<"C:"<<hit.getBackgroundTag()<<" "<<hit.getCellId()<<" "<<hit.getEnergyDep()<<" "<<hit.getTimeAve()<<endl;
  }

  StoreObjPtr<ECLWaveforms> wf(m_eclWaveformsName);
  bool isBGOverlay = wf.isValid();
  BitStream out;
  ECLCompress* comp = NULL;

  // check background overlay
  if (isBGOverlay) {
    std::swap(out.getStore(), wf->getStore());
    out.setPos(0);
    unsigned int compAlgo = out.getNBits(8);
    comp = selectAlgo(compAlgo);
    if (comp == NULL)
      B2FATAL("Unknown compression algorithm: " << compAlgo);
  }

  int FitA[ec.m_nsmp]; // buffer for the waveform fitter

  // loop over entire calorimeter
  for (int j = 0; j < ec.m_nch; j++) {
    adccounts_t& a = m_adc[j];

    // if background waveform is here there is no need to generate
    // electronic noise since it is already in the waveform
    if (isBGOverlay) {
      comp->uncompress(out, FitA);
    } else {
      // Signal amplitude should be above 100 keV
      if (a.total < 0.0001) continue;
      makeElectronicNoiseAndPedestal(j, FitA);
    }

    for (int i = 0; i < ec.m_nsmp; i++) {
      int A = 20000 * a.c[i] + FitA[i];
      FitA[i] = max(0, min(A, (1 << 18) - 1));
    }

    int  energyFit = 0; // fit output : Amplitude 18 bits
    int       tFit = 0; // fit output : T_ave     12 bits
    int qualityFit = 0; // fit output : quality    2 bits
    int        chi = 0; // fit output : chi square   it is not available in the experiment

    shapeFitterWrapper(j, FitA, ttrig, energyFit, tFit, qualityFit, chi);

    if (energyFit > 0) {
      int CellId = j + 1;
      const auto eclDsp = m_eclDsps.appendNew();
      eclDsp->setCellId(CellId);
      eclDsp->setDspA(FitA);

      const auto eclDigit = m_eclDigits.appendNew();
      eclDigit->setCellId(CellId); // cellId in range from 1 to 8736
      eclDigit->setAmp(energyFit); // E (GeV) = energyFit/20000;
      eclDigit->setTimeFit(tFit);  // t0 (us)= (1520 - m_ltr)*24.*12/508/(3072/2) ;
      eclDigit->setQuality(qualityFit);
      eclDigit->setChi(chi);
      for (const auto& hit : hitmap)
        if (hit.cell == j) eclDigit->addRelationTo(m_eclHits[hit.id]);
    }
  } //store each crystal hit
  if (comp) delete comp;
}

void ECLDigitizerModule::endRun()
{
}

void ECLDigitizerModule::terminate()
{
}

void ECLDigitizerModule::readDSPDB()
{
  const EclConfiguration& ec = EclConfiguration::get();

  string dataFileName;
  if (m_background) {
    dataFileName = FileSystem::findFile("/data/ecl/ECL-WF-BG.root");
    B2DEBUG(150, "ECLDigitizer: Reading configuration data with background from: " << dataFileName);
  } else {
    dataFileName = FileSystem::findFile("/data/ecl/ECL-WF.root");
    B2DEBUG(150, "ECLDigitizer: Reading configuration data without background from: " << dataFileName);
  }
  assert(! dataFileName.empty());

  TFile rootfile(dataFileName.c_str());
  TTree* tree  = (TTree*) rootfile.Get("EclWF");
  TTree* tree2 = (TTree*) rootfile.Get("EclAlgo");
  TTree* tree3 = (TTree*) rootfile.Get("EclNoise");

  if (tree == 0 || tree2 == 0 || tree3 == 0) B2FATAL("Data not found");

  m_tbl.resize(ec.m_nch);

  const int maxncellid = 512;
  int ncellId;
  vector<int> cellId(maxncellid);//[ncellId] buffer for crystal identification number

  tree->SetBranchAddress("ncellId", &ncellId);
  tree->SetBranchAddress("cellId", cellId.data());

  vector<int> eclWaveformDataTable(ec.m_nch);
  for (Long64_t j = 0, jmax = tree->GetEntries(); j < jmax; j++) {
    tree->GetEntry(j);
    assert(ncellId <= maxncellid);
    for (int i = 0; i < ncellId; ++i)
      eclWaveformDataTable[cellId[i] - 1] = j;
  }
  B2DEBUG(150, "ECLDigitizer: " << tree->GetEntries() << " sets of wave form covariance matricies will be used.");

  ECLWFAlgoParams* algo = new ECLWFAlgoParams;
  tree2->SetBranchAddress("Algopars", &algo);
  tree2->SetBranchAddress("ncellId", &ncellId);
  tree2->SetBranchAddress("cellId", cellId.data());
  Long64_t jmax2 = tree2->GetEntries();
  vector<ECLWFAlgoParams> eclWFAlgoParams;
  eclWFAlgoParams.reserve(jmax2);
  for (Long64_t j = 0; j < jmax2; j++) {
    tree2->GetEntry(j);
    assert(ncellId <= maxncellid);
    eclWFAlgoParams.push_back(*algo);
    for (int i = 0; i < ncellId; ++i)
      m_tbl[cellId[i] - 1].idn = j;
  }
  if (algo) delete algo;
  B2DEBUG(150, "ECLDigitizer: " << eclWFAlgoParams.size() << " parameter sets of fitting algorithm were read.");

  ECLNoiseData* noise = new ECLNoiseData;
  tree3->SetBranchAddress("NoiseM", &noise);
  tree3->SetBranchAddress("ncellId", &ncellId);
  tree3->SetBranchAddress("cellId", cellId.data());

  Long64_t jmax3 = tree3->GetEntries();
  m_noise.reserve(jmax3);
  for (Long64_t j = 0; j < jmax3; j++) {
    tree3->GetEntry(j);
    assert(ncellId <= maxncellid);
    m_noise.push_back(*noise);
    if (ncellId == 0) {
      for (int i = 0; i < ec.m_nch; i++)
        m_tbl[i].inoise = 0;
      break;
    } else {
      for (int i = 0; i < ncellId; ++i)
        m_tbl[cellId[i] - 1].inoise = j;
    }
  }
  if (noise) delete noise;
  B2DEBUG(150, "ECLDigitizer: " << eclWFAlgoParams.size() << " noise matricies were loaded.");

  // repack fitting algorithm parameters
  m_idn.resize(eclWFAlgoParams.size());
  for (int i = 0, imax = eclWFAlgoParams.size(); i < imax; i++)
    repack(eclWFAlgoParams[i], m_idn[i]);

  vector<uint_pair_t> pairIdx;
  for (int i = 0; i < ec.m_nch; i++) {
    unsigned int   wfIdx = eclWaveformDataTable[i];
    unsigned int algoIdx = m_tbl[i].idn;
    uint_pair_t p(wfIdx, algoIdx);
    vector<uint_pair_t>::iterator ip = find(pairIdx.begin(), pairIdx.end(), p);
    if (ip != pairIdx.end()) { // crystal i already have the same parameters as before
      m_tbl[i].ifunc = ip - pairIdx.begin();
    } else {                  // new combination of parameters
      m_tbl[i].ifunc = pairIdx.size();
      pairIdx.push_back(p);
    }
  }

  // now we know how many distinct elements of the (Waveform x AlgoParams) matrix should be
  m_fitparams.resize(pairIdx.size());

  ECLWaveformData* eclWFData = new ECLWaveformData;
  tree->SetBranchAddress("CovarianceM", &eclWFData);
  tree->SetBranchStatus("ncellId", 0); // do not read it at the moment
  tree->SetBranchStatus("cellId", 0); // do not read it at the moment

// fill parameters for each (Waveform x AlgoParams) parameter set
  for (unsigned int ip = 0; ip < pairIdx.size(); ip++) {
    const uint_pair_t& p = pairIdx[ip];
    tree->GetEntry(p.first); // retrieve data to eclWFData pointer
    getfitparams(*eclWFData, eclWFAlgoParams[p.second], m_fitparams[ip]);
  }
  B2DEBUG(150, "ECLDigitizer: " << m_fitparams.size() << " fitting crystals groups were created.");

  // at the moment there is only one sampled signal shape in the pool
  // since all shaper parameters are the same for all crystals
  m_ss.resize(4);
  float MP[10]; eclWFData->getWaveformParArray(MP);
  m_ss[0].InitSample(MP, 27.7221);
  // parameters vector from ps.dat file, time offset 0.5 usec added to
  // have peak position with parameters from ps.dat roughly in the
  // same place as in current MC
  // double crystal_params[10] = {0.5, 0.301298, 0.631401, 0.470911, 0.903988, -0.11734200/19.5216, 2.26567, 0.675393, 0.683995, 0.0498786};
  // m_ss[0].InitSample(crystal_params, 0.9999272*19.5216);
  for (int i = 0; i < ec.m_nch; i++) m_tbl[i].iss = 0;
  // one sampled diode response in the pool, parameters vector from
  // pg.dat file, time offset 0.5 usec added to have peak position with
  // parameters from ps.dat roughly in the same place as in current MC
  double diode_params[] = {0 + 0.5, 0.100002, 0.756483, 0.456153, 0.0729031, 0.3906 / 9.98822, 2.85128, 0.842469, 0.854184, 0.110284};
  m_ss[1].InitSample(diode_params, 0.9569100 * 9.98822);
  double gamma_params_forPSD[] = {0.5, 0.648324, 0.401711, 0.374167, 0.849417, 0.00144548, 4.70722, 0.815639, 0.555605, 0.2752};
  m_ss[2].InitSample(gamma_params_forPSD, 27.7221);
  double psd_params_forPSD[] = {0.654324, 0.110699, 0.606028, 1.2688, 0.553606, 0.304011, 1.2551, 0.771018, 0.454058, 1.25524};
  m_ss[3].InitSample(psd_params_forPSD, 27.7221);

  B2DEBUG(150, "ECLDigitizer: " << m_ss.size() << " sampled signal templates were created.");

  if (eclWFData) delete eclWFData;

  rootfile.Close();
}

void ECLDigitizerModule::repack(const ECLWFAlgoParams& eclWFAlgo, algoparams_t& t)
{
  // filling short int array
  t.id[ 0] = eclWFAlgo.getlAT() + 128;
  t.id[ 1] = eclWFAlgo.getsT()  + 128;
  t.id[ 2] = eclWFAlgo.gethT();
  t.id[ 3] = 17952;
  t.id[ 4] = 19529;
  t.id[ 5] = 69;
  t.id[ 6] = 0;
  t.id[ 7] = 0;
  t.id[ 8] = 257;
  t.id[ 9] = -1;
  t.id[10] = 0;
  t.id[11] = 0;

  // filling unsigned char array
  t.ic[12 * 2 + 0] = eclWFAlgo.getk1();
  t.ic[12 * 2 + 1] = eclWFAlgo.getk2();
  t.ic[13 * 2 + 0] = eclWFAlgo.getka();
  t.ic[13 * 2 + 1] = eclWFAlgo.getkb();
  t.ic[14 * 2 + 0] = eclWFAlgo.getkc();
  t.ic[14 * 2 + 1] = eclWFAlgo.gety0s() - 16;

  // again, filling short int array
  t.id[15] = eclWFAlgo.getcT();
}

void ECLDigitizerModule::getfitparams(const ECLWaveformData& eclWFData, const ECLWFAlgoParams& eclWFAlgo, fitparams_t& p)
{
  const EclConfiguration& ec = EclConfiguration::get();

  double ssd[16][16];
  eclWFData.getMatrix(ssd);
  vector<double> MP(10);
  eclWFData.getWaveformParArray(MP.data());

  // shape function parameters
  int iff = 1 << 14;   //Alex Bobrov for correct chi square

  int ia = 1 << eclWFAlgo.getka();
  int ib = 1 << eclWFAlgo.getkb();
  int ic = 1 << eclWFAlgo.getkc();

  int_array_192x16_t& ref_f    = p.f;
  int_array_192x16_t& ref_f1   = p.f1;
  int_array_192x16_t& ref_fg31 = p.fg31;
  int_array_192x16_t& ref_fg32 = p.fg32;
  int_array_192x16_t& ref_fg33 = p.fg33;
  int_array_24x16_t&  ref_fg41 = p.fg41;
  int_array_24x16_t&  ref_fg43 = p.fg43;

  ShaperDSP_t dsp(MP);
  dsp.settimestride(ec.m_step);
  dsp.setseedoffset(ec.m_step / ec.m_ndt);
  dsp.settimeseed(-(ec.m_step - (ec.m_step / ec.m_ndt)));
  vector<dd_t> f(16);
  for (int k = 0; k < 2 * ec.m_ndt; k++, dsp.nextseed()) { // calculate fit parameters around 0 +- 1 ADC tick
    dsp.fillvector(f);

    double g0g0 = 0, g0g1 = 0, g1g1 = 0, g0g2 = 0, g1g2 = 0, g2g2 = 0;
    double sg0[16], sg1[16], sg2[16];
    for (int j = 0; j < 16; j++) {
      double g0 = 0, g1 = 0, g2 = 0;
      for (int i = 0; i < 16; i++) {
        g0 += ssd[j][i] * f[i].first;
        g1 += ssd[j][i] * f[i].second;
        g2 += ssd[j][i];
      }
      g0g0 += g0 * f[j].first;
      g0g1 += g1 * f[j].first;
      g1g1 += g1 * f[j].second;
      g0g2 += g0;
      g1g2 += g1;
      g2g2 += g2;
      sg0[j] = g0;
      sg1[j] = g1;
      sg2[j] = g2;
    }

    double a00 = g1g1 * g2g2 - g1g2 * g1g2;
    double a11 = g0g0 * g2g2 - g0g2 * g0g2;
    double a22 = g0g0 * g1g1 - g0g1 * g0g1;
    double a01 = g1g2 * g0g2 - g0g1 * g2g2;
    double a02 = g0g1 * g1g2 - g1g1 * g0g2;
    double a12 = g0g1 * g0g2 - g0g0 * g1g2;

    double igg2 = 1 / (a11 * g1g1 + g0g1 * a01 + g1g2 * a12);

    // to fixed point representation
    const double isd = 3. / 4., sd = 1 / isd ; // conversion factor (???)
    for (int i = 0; i < 16; i++) {
      double w = i ? 1.0 : 1. / 16.;

      ref_f   [k][i] = lrint(f[i].first  * iff * w);
      ref_f1  [k][i] = lrint(f[i].second * iff * w * sd);

      double fg31 = (a00 * sg0[i] + a01 * sg1[i] + a02 * sg2[i]) * igg2;
      double fg32 = (a01 * sg0[i] + a11 * sg1[i] + a12 * sg2[i]) * igg2;
      double fg33 = (a02 * sg0[i] + a12 * sg1[i] + a22 * sg2[i]) * igg2;

      ref_fg31[k][i] = lrint(fg31 * ia * w);
      ref_fg32[k][i] = lrint(fg32 * ib * w * isd);
      ref_fg33[k][i] = lrint(fg33 * ic * w);
    }

    //first approximation without time correction
    int jk = 23 + ((48 - k) >> 2);
    if (jk >= 0 && jk < 24 && (48 - k) % 4 == 0) {

      double igg1 = 1 / a11;
      // to fixed point
      for (int i = 0; i < 16; i++) {
        double w = i ? 1.0 : 1. / 16.;

        double fg41 = (g2g2 * sg0[i] - g0g2 * sg2[i]) * igg1;
        double fg43 = (g0g0 * sg2[i] - g0g2 * sg0[i]) * igg1;
        ref_fg41[jk][i] = lrint(fg41 * ia * w);
        ref_fg43[jk][i] = lrint(fg43 * ic * w);
      }
    }
  }
}
