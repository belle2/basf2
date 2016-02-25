/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen, Guglielmo De Nardo                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclDigitizer/ECLDigitizerModule.h>
#include <ecl/digitization/algorithms.h>
#include <ecl/digitization/shaperdsp.h>
#include <ecl/digitization/EclConfiguration.h>
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

void ECLDigitizerModule::signalsample_t::InitSample(const double* MPd)
{
  const int N = m_ns * m_nl;
  vector<double> MP(MPd, MPd + 10);
  ShaperDSP_t dsp(MP);
  dsp.settimestride(m_tick / m_ns);
  dsp.fillarray(0.0, N, m_ft);

  double sum = 0;
  for (int i = 0; i < N; i++) sum += m_ft[i];
  m_sumscale = m_ns / sum;
}

void ECLDigitizerModule::signalsample_t::InitSample(const float* MP)
{
  double MPd[10];
  for (int i = 0; i < 10; i++) MPd[i] = MP[i];
  InitSample(MPd);
}

void ECLDigitizerModule::adccounts_t::AddHit(const double a, const double t0, const ECLDigitizerModule::signalsample_t& s)
{
  total += s.Accumulate(a, t0, c);
}

double ECLDigitizerModule::signalsample_t::Accumulate(const double a, const double t0, double* s) const
{
  // input parameters
  // a -- signal amplitude
  // t -- signal offset
  // output parameter
  // s -- output array with added signal
  const double itick = 1 / m_tick;          // reciprocal to avoid division in usec^-1 (has to be evaluated at compile time)
  const double  tlen = m_nl - 1.0 / m_ns;   // length of the sampled signal in ADC clocks units
  const double  tmax = m_tmin + m_nsmp - 1; // upper range of the fit region

  double t = t0 * itick; // rescale time in usec to ADC clocks
  double x0 = t, x1 = t + tlen;

  if (x0 > tmax) return 0; // signal starts after the upper range of output array -- do nothing
  if (x0 < m_tmin) {
    if (x1 < m_tmin) return 0; // signal ends before lower range of output array -- do nothing
    x0 = m_tmin; // adjust signal with range of output array
  }

  int imax = m_nsmp; // length of sampled signal is long enough so
  // the last touched element is the last element
  // of the output array
  if (x1 < tmax) { // if initial time is too early we need to adjust
    // the last touched element of output array to avoid
    // out-of-bound situation in m_ft
    imax = x1 - m_tmin; // imax is always positive so floor can be
    // replace by simple typecast
    imax += 1; // like s.end()
  }

  double imind = ceil(x0 - m_tmin); // store result in double to avoid int->double conversion below
  // the ceil function today at modern CPUs is surprisingly fast (before it was horribly slow)
  int imin = imind; // starting point to fill output array
  double w = ((m_tmin - t) + imind) * double(m_ns);
  int jmin = w; // starting point in sampled signal array
  w -= jmin;

  // use linear interpolation between samples. Since signal samples
  // are aligned with output samples only two weights are need to
  // calculate to fill output array
  const double w1 = a * w, w0 = a - w1;
  double sum = 0;
  for (int i = imin, j = jmin; i < imax; i++, j += m_ns) {
    double amp = w0 * m_ft[j] + w1 * m_ft[j + 1];
    s[i] += amp;
    sum  += amp;
  }
  return sum * m_sumscale;
}

ECLDigitizerModule::ECLDigitizerModule() : Module()
{
  //Set module properties
  setDescription("Creates ECLDigiHits from ECLHits.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("Background", m_background, "Flag to use the Digitizer configuration with backgrounds; Default is no background", false);
  addParam("Calibration", m_calibration, "Flag to use the Digitizer for Waveform fit Covariance Matrix calibration; Default is false",
           false);
}

ECLDigitizerModule::~ECLDigitizerModule()
{
}

void ECLDigitizerModule::initialize()
{
  m_nEvent  = 0 ;
  StoreArray<ECLDsp>    ecldsp;  ecldsp.registerInDataStore();
  StoreArray<ECLDigit> ecldigi; ecldigi.registerInDataStore();
  StoreArray<ECLTrig>  ecltrig; ecltrig.registerInDataStore();

  readDSPDB();

  m_adc.resize(m_nch);

  EclConfiguration::get().setBackground(m_background);

}

void ECLDigitizerModule::beginRun()
{
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

void ECLDigitizerModule::event()
{
  //Input Array
  StoreArray<ECLHit> eclHits;
  if (!eclHits) {
    B2DEBUG(100, "ECLHit in empty in event " << m_nEvent);
  }

  // Output Arrays
  StoreArray<ECLDigit> eclDigits;
  StoreArray<ECLDsp> eclDsps;
  StoreArray<ECLTrig> eclTrigs;

  const double trgtick = m_tick / m_ntrg;   // trigger tick
  const double  DeltaT = gRandom->Uniform(0, double(m_ntrg)); // trigger decision can come in any time ???
  const double timeInt = DeltaT * trgtick;
  const int      ttrig = int(DeltaT);

  const auto eclTrig = eclTrigs.appendNew();
  eclTrig->setTimeTrig(timeInt); //t0 (us)= (1520 - m_ltr)*24.*

  // clear the storage for the event
  memset(m_adc.data(), 0, m_adc.size()*sizeof(adccounts_t));

  // emulate response for ECL hits after ADC measurements
  for (const auto& eclHit : eclHits) {
    int j = eclHit.getCellId() - 1; //0~8735
    double hitE       = eclHit.getEnergyDep() / Unit::GeV;
    double hitTimeAve = eclHit.getTimeAve() / Unit::us;
    m_adc[j].AddHit(hitE, hitTimeAve + timeInt - 0.32 , m_ss[m_tbl[j].iss]);
  }

  // loop over entire calorimeter
  for (int j = 0; j < m_nch; j++) {
    adccounts_t& a = m_adc[j];

    if (m_calibration) {
      // This has been added by Alex Bobrov for calibration
      // of covariance matrix artificially generate 100 MeV in time for each crystal
      double hitE = 0.1, hitTimeAve = 0.0;
      a.AddHit(hitE, hitTimeAve + timeInt - 0.32 , m_ss[m_tbl[j].iss]);
    } else if (a.total < 0.0001)
      continue;

    //Noise generation
    float z[m_nsmp];
    for (int i = 0; i < m_nsmp; i++)
      z[i] = gRandom->Gaus(0, 1);

    float AdcNoise[m_nsmp];
    m_noise[m_tbl[j].inoise].generateCorrelatedNoise(z, AdcNoise);

    int FitA[m_nsmp];
    for (int  i = 0; i < m_nsmp; i++)
      FitA[i] = 20 * (1000 * a.c[i] + AdcNoise[i]) + 3000;

    int  energyFit = 0; // fit output : Amplitude 18 bits
    int       tFit = 0; // fit output : T_ave     12 bits
    int qualityFit = 0; // fit output : quality    2 bits
    int        chi = 0; // fit output : chi square   it is not available in the experiment


    shapeFitterWrapper(j, FitA, ttrig, energyFit, tFit, qualityFit, chi);

    if (energyFit > 0) {
      int CellId = j + 1;
      const auto eclDsp = eclDsps.appendNew();
      eclDsp->setCellId(CellId);
      eclDsp->setDspA(FitA);

      const auto eclDigit = eclDigits.appendNew();
      eclDigit->setCellId(CellId); // cellId in range from 1 to 8736
      eclDigit->setAmp(energyFit); // E (GeV) = energyFit/20000;
      eclDigit->setTimeFit(tFit);  // t0 (us)= (1520 - m_ltr)*24.*12/508/(3072/2) ;
      eclDigit->setQuality(qualityFit);
      eclDigit->setChi(chi);

    }
  } //store each crystal hit

  m_nEvent++;
}

void ECLDigitizerModule::endRun()
{
}

void ECLDigitizerModule::terminate()
{
}

void ECLDigitizerModule::readDSPDB()
{
  string dataFileName;
  if (m_background) {
    dataFileName = FileSystem::findFile("/data/ecl/ECL-WF-BG.root");
    B2INFO("ECLDigitizer: Reading configuration data with background from: " << dataFileName);
  } else {
    dataFileName = FileSystem::findFile("/data/ecl/ECL-WF.root");
    B2INFO("ECLDigitizer: Reading configuration data without background from: " << dataFileName);
  }
  assert(! dataFileName.empty());

  TFile rootfile(dataFileName.c_str());
  TTree* tree  = (TTree*) rootfile.Get("EclWF");
  TTree* tree2 = (TTree*) rootfile.Get("EclAlgo");
  TTree* tree3 = (TTree*) rootfile.Get("EclNoise");

  if (tree == 0 || tree2 == 0 || tree3 == 0) B2FATAL("Data not found");

  m_tbl.resize(m_nch);

  const int maxncellid = 512;
  int ncellId;
  vector<int> cellId(maxncellid);//[ncellId] buffer for crystal identification number

  tree->SetBranchAddress("ncellId", &ncellId);
  tree->SetBranchAddress("cellId", cellId.data());

  vector<int> eclWaveformDataTable(m_nch);
  for (Long64_t j = 0, jmax = tree->GetEntries(); j < jmax; j++) {
    tree->GetEntry(j);
    assert(ncellId <= maxncellid);
    for (int i = 0; i < ncellId; ++i)
      eclWaveformDataTable[cellId[i] - 1] = j;
  }
  B2INFO("ECLDigitizer: " << tree->GetEntries() << " sets of wave form covariance matricies will be used.");

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
  B2INFO("ECLDigitizer: " << eclWFAlgoParams.size() << " parameter sets of fitting algorithm were read.");

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
      for (int i = 0; i < m_nch; i++)
        m_tbl[i].inoise = 0;
      break;
    } else {
      for (int i = 0; i < ncellId; ++i)
        m_tbl[cellId[i] - 1].inoise = j;
    }
  }
  if (noise) delete noise;
  B2INFO("ECLDigitizer: " << eclWFAlgoParams.size() << " noise matricies were loaded.");

  // repack fitting algorithm parameters
  m_idn.resize(eclWFAlgoParams.size());
  for (int i = 0, imax = eclWFAlgoParams.size(); i < imax; i++)
    repack(eclWFAlgoParams[i], m_idn[i]);

  vector<uint_pair_t> pairIdx;
  for (int i = 0; i < m_nch; i++) {
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
  B2INFO("ECLDigitizer: " << m_fitparams.size() << " fitting crystals groups were created.");

  // at the moment there is only one sampled signal shape in the pool
  // since all shaper parameters are the same for all crystals
  m_ss.resize(1);
  float MP[10]; eclWFData->getWaveformParArray(MP);
  m_ss[0].InitSample(MP);
  for (int i = 0; i < m_nch; i++) m_tbl[i].iss = 0;
  B2INFO("ECLDigitizer: " << m_ss.size() << " sampled signal templates were created.");

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
  double ssd[16][16];
  eclWFData.getMatrix(ssd);
  vector<double> MP(10);
  eclWFData.getWaveformParArray(MP.data());

  // shape function parameters
  int iff = 14;   //Alex Bobrov for correct chi square

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
  dsp.settimestride(m_tick);
  dsp.setseedoffset(m_tick / m_ndt);
  dsp.settimeseed(-(m_tick - (m_tick / m_ndt)));
  vector<dd_t> f(16);
  for (int k = 0; k < 2 * m_ndt; k++, dsp.nextseed()) { // calculate fit parameters around 0 +- 1 ADC tick
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
    int jk = 24 - ((k + 3) >> 2);
    if (jk >= 0 && jk < 24 && (k + 3) % 4 == 0) {
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
