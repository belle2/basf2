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
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLTrig.h>

#include <ecl/geometry/ECLGeometryPar.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>


//C++ STL
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <utility> //contains pair

#define PI 3.14159265358979323846


// ROOT
#include <TVector3.h>
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
}


ECLDigitizerModule::~ECLDigitizerModule()
{

}

void ECLDigitizerModule::initialize()
{
  m_nEvent  = 0 ;
  StoreArray<ECLDsp> ecldsp;
  StoreArray<ECLDigit> ecldigi;
  StoreArray<ECLTrig> ecltrig;
  ecldsp.registerInDataStore();
  ecldigi.registerInDataStore();
  ecltrig.registerInDataStore();
  readDSPDB();
}

void ECLDigitizerModule::beginRun()
{
}

// interface to C shape fitting function function
void ECLDigitizerModule::shapeFitterWrapper(const int j, const int* FitA, const int m_ttrig,
                                            int& m_lar, int& m_ltr, int& m_lq)
{
  const int n16 = 16; // number of points before signal n16 = 16
  unsigned int idIdx = m_eclWFAlgoParamsTable[j]; //lookup table uses cellID definition [1,8736]
  const fitparams_t& r = m_fitparams[m_funcTable[j]];
  shapeFitter((short int*)m_idn[idIdx].id, (int*)r.f, (int*)r.f1, (int*)r.fg41, (int*)r.fg43, (int*)r.fg31, (int*)r.fg32,
              (int*)r.fg33, (int*)FitA,
              (int*)&m_ttrig, (int*)&n16, &m_lar, &m_ltr, &m_lq);
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

  array2d HitEnergy(boost::extents[8736][31]);
  std::fill(HitEnergy.origin(), HitEnergy.origin() + HitEnergy.size(), 0.0);
  vector<double> totalEnergyDeposit(8736, 0);

  const double    tick = 24.*12. / 508.; // digitization clock tick (in microseconds ???)
  const double trgtick = tick / 144;   // trigger tick
  const double      dt = 0.02;         // delta t for interpolation

  const double  DeltaT = gRandom->Uniform(0, 144);
  const double timeInt = DeltaT * trgtick; //us
  const int      ttrig = int(DeltaT);

  const auto eclTrig = eclTrigs.appendNew();
  eclTrig->setTimeTrig(DeltaT * trgtick); //t0 (us)= (1520 - m_ltr)*24.*

  for (const auto& eclHit : eclHits) {
    int j = eclHit.getCellId() - 1; //0~8735
    double hitE       = eclHit.getEnergyDep()  / Unit::GeV;
    double hitTimeAve = eclHit.getTimeAve() / Unit::us;

    if (hitTimeAve > 8.5) continue;
    totalEnergyDeposit[j] += hitE; //for summation deposit energy; do fit if this summation > 0.1 MeV

    for (int i = 0; i < 31; i++) {
      double sampleTime = tick * (i - 15) - hitTimeAve - timeInt + 0.32; //There is some time shift~0.32 us that is found Alex 2013.06.19.
      HitEnergy[j][i] += hitE * DspSamplingArray(m_ft.size(), sampleTime, dt, &m_ft[0]); //interpolation from shape array n=1250; dt =20ns
    }
  }

  if (m_calibration) {
    // This has been added by Alex Bobrov for calibration
    // of covariance matrix artificially generate 100 MeV in time for each crystal
    for (int j = 0; j < 8736; j++) {
      double hitE = 0.1;
      double hitTimeAve = 0.0;

      for (int i = 0; i < 31; i++) {
        double sampleTime = tick * (i - 15) - hitTimeAve - timeInt + 0.32; //There is some time shift~0.32 us that is found Alex 2013.06.19.
        HitEnergy[j][i] += hitE * DspSamplingArray(m_ft.size(), sampleTime, dt, &m_ft[0]); //interpolation from shape array n=1250; dt =20ns
      }
    }
  } // end of Alex Bobrov ad-hoc calibration...

  for (int j = 0; j < 8736; j++) {
    if (m_calibration || totalEnergyDeposit[j] > 0.0001) { // Bobrov removes this cut in calibration

      //Noise generation
      float z[31];
      for (int i = 0; i < 31; i++)
        z[i] = gRandom->Gaus(0, 1);

      float AdcNoise[31];
      m_eclNoiseData[m_eclNoiseDataTable[j]].generateCorrelatedNoise(z, AdcNoise);

      int FitA[31];
      for (int  i = 0; i < 31; i++)
        FitA[i] = (int)(HitEnergy[j][i] * 20000 + 3000 + AdcNoise[i] * 20) ;

      int  energyFit = 0; // fit output : Amplitude 18 bits
      int       tFit = 0; // fit output : T_ave     12 bits
      int qualityFit = 0; // fit output : quality    2 bits

      shapeFitterWrapper(j, FitA, ttrig, energyFit, tFit, qualityFit);

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

    }//if Energy > 0.1 MeV
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

  m_eclWaveformDataTable.resize(8736);
  m_eclWFAlgoParamsTable.resize(8736);
  m_eclNoiseDataTable.resize(8736);
  m_funcTable.resize(8736);

  const int maxncellid = 512;
  int ncellId;
  vector<int> cellId(maxncellid);//[ncellId] buffer for crystal identification number

  tree->SetBranchAddress("ncellId", &ncellId);
  tree->SetBranchAddress("cellId", cellId.data());

  for (Long64_t j = 0, jmax = tree->GetEntries(); j < jmax; j++) {
    tree->GetEntry(j);
    assert(ncellId <= maxncellid);
    for (int i = 0; i < ncellId; ++i)
      m_eclWaveformDataTable[cellId[i] - 1] = j;
  }

  ECLWFAlgoParams* algo = new ECLWFAlgoParams;
  tree2->SetBranchAddress("Algopars", &algo);
  tree2->SetBranchAddress("ncellId", &ncellId);
  tree2->SetBranchAddress("cellId", cellId.data());
  Long64_t jmax2 = tree2->GetEntries();
  m_eclWFAlgoParams.reserve(jmax2);
  for (Long64_t j = 0; j < jmax2; j++) {
    tree2->GetEntry(j);
    assert(ncellId <= maxncellid);
    m_eclWFAlgoParams.push_back(*algo);
    for (int i = 0; i < ncellId; ++i)
      m_eclWFAlgoParamsTable[cellId[i] - 1] = j;
  }
  if (algo) delete algo;

  ECLNoiseData* noise = new ECLNoiseData;
  tree3->SetBranchAddress("NoiseM", &noise);
  tree3->SetBranchAddress("ncellId", &ncellId);
  tree3->SetBranchAddress("cellId", cellId.data());

  Long64_t jmax3 = tree3->GetEntries();
  m_eclNoiseData.reserve(jmax3);
  for (Long64_t j = 0; j < jmax3; j++) {
    tree3->GetEntry(j);
    assert(ncellId <= maxncellid);
    m_eclNoiseData.push_back(*noise);
    if (ncellId == 0) {
      for (int i = 0; i < 8736; i++)
        m_eclNoiseDataTable[i] = 0;
      break;
    } else {
      for (int i = 0; i < ncellId; ++i)
        m_eclNoiseDataTable[cellId[i] - 1] = j;
    }
  }
  if (noise) delete noise;

  TTree* tree4 = (TTree*) rootfile.Get("EclSampledSignalWF");
  if (tree4 != 0) {
    vector<double>* t = 0;
    tree4->SetBranchAddress("EclSampledSignalWF", &t);
    tree4->GetEntry(0);
    swap(m_ft, *t);
  } else {
    swap(m_ft, *createDefSampledSignalWF());
  }

  // repack fitting algorithm parameters
  m_idn.resize(m_eclWFAlgoParams.size());
  for (int i = 0, imax = m_eclWFAlgoParams.size(); i < imax; i++) {
    const ECLWFAlgoParams& eclWFAlgo = m_eclWFAlgoParams[i];
    shortint_array_16_t& a = m_idn[i].id;
    a[ 0] = eclWFAlgo.getlAT() + 128;
    a[ 1] = eclWFAlgo.getsT()  + 128;
    a[ 2] = eclWFAlgo.gethT();
    a[ 3] = 17952;
    a[ 4] = 19529;
    a[ 5] = 69;
    a[ 6] = 0;
    a[ 7] = 0;
    a[ 8] = 257;
    a[ 9] = -1;
    a[10] = 0;
    a[11] = 0;
    a[12] = eclWFAlgo.getk2() + 256 * eclWFAlgo.getk1();
    a[13] = eclWFAlgo.getka() + 256 * eclWFAlgo.getkb();
    a[14] = eclWFAlgo.getkc() + 256 * (eclWFAlgo.gety0s() - 16);
    a[15] = eclWFAlgo.getcT();
  }

  vector<uint_pair_t> pairIdx;
  for (int i = 0; i < 8736; i++) {
    unsigned int   wfIdx = m_eclWaveformDataTable[i];
    unsigned int algoIdx = m_eclWFAlgoParamsTable[i];
    uint_pair_t p(wfIdx, algoIdx);
    vector<uint_pair_t>::iterator ip = find(pairIdx.begin(), pairIdx.end(), p);
    if (ip != pairIdx.end()) { // crystal i already have the same parameters as before
      m_funcTable[i] = ip - pairIdx.begin();
    } else {                  // new combination of parameters
      m_funcTable[i] = pairIdx.size();
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
    const ECLWFAlgoParams& eclWFAlgo = m_eclWFAlgoParams[p.second];

    float s[16][16], MP[10];

    eclWFData->getMatrix(s);
    eclWFData->getWaveformParArray(MP);

    // shape function parameters
    int ia = 1 << eclWFAlgo.getka();
    int ib = 1 << eclWFAlgo.getkb();
    int ic = 1 << eclWFAlgo.getkc();

    const int    ndt = 96;
    const double  dt = 72. / 127.;
    const double ddt = 0.001 * dt;

    int_array_192x16_t& ref_f    = m_fitparams[ip].f;
    int_array_192x16_t& ref_f1   = m_fitparams[ip].f1;
    int_array_192x16_t& ref_fg31 = m_fitparams[ip].fg31;
    int_array_192x16_t& ref_fg32 = m_fitparams[ip].fg32;
    int_array_192x16_t& ref_fg33 = m_fitparams[ip].fg33;
    int_array_24x16_t& ref_fg41 = m_fitparams[ip].fg41;
    int_array_24x16_t& ref_fg43 = m_fitparams[ip].fg43;

    for (int k = 0; k < 192; k++) {
      double t0 = dt - (k + 1) * dt / ndt;

      double f0[16], f1[16];
      for (int j = 0; j < 16; j++) {
        double t = j * dt - t0;

        double fx = ShaperDSP_F(t      , MP);
        double fp = ShaperDSP_F(t + ddt, MP);
        double fm = ShaperDSP_F(t - ddt, MP);
        f0[j] =  fx;
        f1[j] = (fp - fm) / (2 * ddt);
      }

      double g0g0 = 0, g0g1 = 0, g1g1 = 0, g0g2 = 0, g1g2 = 0, g2g2 = 0;
      double sg0[16], sg1[16], sg2[16];
      for (int j = 0; j < 16; j++) {
        sg0[j] = sg1[j] = sg2[j] = 0;
        double fj0 = f0[j];
        double fj1 = f1[j];
        for (int i = 0; i < 16; i++) {
          double fi0 = f0[i];
          double fi1 = f1[i];
          double sji = s[j][i];
          sg0[j] += sji * fi0;
          sg1[j] += sji * fi1;
          sg2[j] += sji;
          g0g0   += sji * fj0 * fi0;
          g0g1   += sji * fj0 * fi1;
          g1g1   += sji * fi1 * fj1;
          g0g2   += sji * fj0;
          g1g2   += sji * fj1;
          g2g2   += sji;
        }
      }

      double a00 = g1g1 * g2g2 - g1g2 * g1g2;
      double a11 = g0g0 * g2g2 - g0g2 * g0g2;
      double a22 = g0g0 * g1g1 - g0g1 * g0g1;
      double a01 = g1g2 * g0g2 - g0g1 * g2g2;
      double a02 = g0g1 * g1g2 - g1g1 * g0g2;
      double a12 = g0g1 * g0g2 - g0g0 * g1g2;

      double igg2 = 1 / (a11 * g1g1 + g0g1 * a01 + g1g2 * a12);
      // to fixed point representation
      const double sd = 4. / 3., isd = 3. / 4.;
      for (int i = 0; i < 16; i++) {
        double idd = i ? 1.0 : 1. / 16.;

        ref_f   [k][i] = lrint(f0[i] * ia * idd);
        ref_f1  [k][i] = lrint(sd * f1[i] * ia * idd);

        double fg31 = (a00 * sg0[i] + a01 * sg1[i] + a02 * sg2[i]) * igg2;
        double fg32 = (a01 * sg0[i] + a11 * sg1[i] + a12 * sg2[i]) * igg2;
        double fg33 = (a02 * sg0[i] + a12 * sg1[i] + a22 * sg2[i]) * igg2;

        ref_fg31[k][i] = lrint(fg31 * ia * idd);
        ref_fg32[k][i] = lrint(isd * fg32 * ib * idd);
        ref_fg33[k][i] = lrint(fg33 * ic * idd);
      }
      //first approximation without time correction
      int jk = 24 - ((k + 3) >> 2);
      if (jk >= 0 && jk < 24 && (k + 3) % 4 == 0) {
        double igg1 = 1 / a11;
        // to fixed point
        for (int i = 0; i < 16; i++) {
          double idd = i ? 1.0 : 1. / 16.;

          double fg41 = (g2g2 * sg0[i] - g0g2 * sg2[i]) * igg1;
          double fg43 = (g0g0 * sg2[i] - g0g2 * sg0[i]) * igg1;
          ref_fg41[jk][i] = lrint(fg41 * ia * idd);
          ref_fg43[jk][i] = lrint(fg43 * ic * idd);
        }
      }
    }
  }
  if (eclWFData) delete eclWFData;
  rootfile.Close();
}
