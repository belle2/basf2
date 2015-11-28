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
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/digitization/ECLSampledShaper.h>
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

void ECLDigitizerPureCsIModule::signalsample_t::InitSample(const TH1F* sampledfun)
{
  const int N = m_ns * m_nl;
  ECLSampledShaper dsp(sampledfun);
  dsp.fillarray(N, m_ft);
  double sum = 0;
  for (int i = 0; i < N; i++) sum += m_ft[i];
  m_sumscale = m_ns / sum;
}

void ECLDigitizerPureCsIModule::adccounts_t::AddHit(const double a, const double t0,
                                                    const ECLDigitizerPureCsIModule::signalsample_t& s)
{
  total += s.Accumulate(a, t0, c);
}

double ECLDigitizerPureCsIModule::signalsample_t::Accumulate(const double a, const double t0, double* s) const
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
}

ECLDigitizerPureCsIModule::~ECLDigitizerPureCsIModule()
{
}

void ECLDigitizerPureCsIModule::initialize()
{
  m_nEvent  = 0 ;
  StoreArray<ECLDsp>    ecldsp(eclDspArrayName()); ecldsp.registerInDataStore();
  StoreArray<ECLDigit> ecldigi(eclDigitArrayName()); ecldigi.registerInDataStore();
  //  StoreArray<ECLTrig>  ecltrig; ecltrig.registerInDataStore();

  readDSPDB();

  m_adc.resize(m_nch);

  EclConfiguration::get().setBackground(m_background);

}

void ECLDigitizerPureCsIModule::beginRun()
{
}

// interface to C shape fitting function function

void ECLDigitizerPureCsIModule::shapeFitterWrapperPureCsI(const int /* j */, const int* FitA, const int /* ttrig */ ,
                                                          int& m_lar, int& m_ltr, int& m_lq) const
{
  int baseline = accumulate(FitA, FitA + 15, 0) / 15;
  const int* imax =  max_element(FitA, FitA + m_nsmp);

  m_lar = *imax - baseline;
  m_ltr = imax - FitA;
  m_lq = 0;
}


void ECLDigitizerPureCsIModule::event()
{
  //  B2INFO("ECLDigitizerPureCsI: event");
  //Input Array
  StoreArray<ECLHit> eclHits;
  if (!eclHits) {
    B2DEBUG(100, "ECLHit array is empty in event " << m_nEvent);
  }

  // Output Arrays
  StoreArray<ECLDigit> eclDigits(eclDigitArrayName());
  StoreArray<ECLDsp> eclDsps(eclDspArrayName());
  StoreArray<ECLTrig> eclTrigs;

  double timeInt = 0;
  const double trgtick = m_tick / m_ntrg;   // trigger tick
  if (eclTrigs.getEntries() == 1) {
    timeInt = eclTrigs[0]->getTimeTrig();
  }
  const double DeltaT = timeInt / trgtick;
  const int      ttrig = int(DeltaT);

  /*
  const double trgtick = m_tick / m_ntrg;   // trigger tick
  const double  DeltaT = gRandom->Uniform(0, double(m_ntrg)); // trigger decision can come in any time ???
  const double timeInt = DeltaT * trgtick;
  const int      ttrig = int(DeltaT);

  const auto eclTrig = eclTrigs.appendNew();
  eclTrig->setTimeTrig(timeInt); //t0 (us)= (1520 - m_ltr)*24.*
  */


  // clear the storage for the event
  memset(m_adc.data(), 0, m_adc.size()*sizeof(adccounts_t));

  // emulate response for ECL hits after ADC measurements
  for (const auto& eclHit : eclHits) {
    int j = eclHit.getCellId() - 1; //0~8735
    if (isPureCsI(j + 1)) {
      double hitE       = eclHit.getEnergyDep() / Unit::GeV;
      double hitTime    = eclHit.getTimeAve() / Unit::us;

      //m_adc[j].AddHit(hitE, hitTime + timeInt - 0.32, m_ss[m_tbl[j].iss]);
      m_adc[j].AddHit(hitE, hitTime - 0.030, m_ss[m_tbl[j].iss]);
    }
  }

  // loop over entire calorimeter
  for (int j = 0; j < m_nch; j++) {
    if (! isPureCsI(j + 1)) continue;
    adccounts_t& a = m_adc[j];

    if (m_calibration) {
      // This has been added by Alex Bobrov for calibration
      // of covariance matrix artificially generate 100 MeV in time for each crystal
      double hitE = 0.1, hitTimeAve = 0.0;
      a.AddHit(hitE, hitTimeAve + timeInt - 0.32, m_ss[m_tbl[j].iss]);
    } else if (a.total < 0.0001)
      continue;

    //Skip Noise generation for now
    /*
    float z[m_nsmp];
    for (int i = 0; i < m_nsmp; i++)
      z[i] = gRandom->Gaus(0, 1);

    float AdcNoise[m_nsmp];
    m_noise[m_tbl[j].inoise].generateCorrelatedNoise(z, AdcNoise);
    */

    //    cout << "=======ECL Fit Data ============" << endl;
    int FitA[m_nsmp];
    for (int  i = 0; i < m_nsmp; i++) {
      // skip noise for now
      // FitA[i] = 20 * (1000 * a.c[i] + AdcNoise[i]) + 3000;
      FitA[i] = 20 * (1000 * a.c[i]) + 3000;

      // cout << FitA[i] << endl;
    }

    int  energyFit = 0; // fit output : Amplitude 18 bits
    int       tFit = 0; // fit output : T_ave     12 bits
    int qualityFit = 0; // fit output : quality    2 bits

    shapeFitterWrapperPureCsI(j, FitA, ttrig, energyFit, tFit, qualityFit);

    //cout << "Ampl = " << energyFit << endl;
    //cout << "T = " << tFit << endl;
    //cout << "===================" << endl;


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

  m_tbl.resize(m_nch);

  // at the moment there is only one sampled signal shape in the pool
  // since all shaper parameters are the same for all crystals
  m_ss.resize(1);
  m_ss[0].InitSample(sampledWF);
  for (int i = 0; i < m_nch; i++) m_tbl[i].iss = 0;
  B2INFO("ECLDigitizerPureCsI: " << m_ss.size() << " sampled signal templates were created.");

  rootfile.Close();
}
