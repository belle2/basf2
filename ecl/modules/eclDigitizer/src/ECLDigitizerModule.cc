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

int** ECLDigitizerModule::allocateMatrix(unsigned int nrows, unsigned int ncols) const
{
  int** pointer = new int* [nrows];
  assert(pointer != nullptr);
  int* data = new int[ncols * nrows];
  assert(data != nullptr);
  pointer[0] = data;
  for (unsigned int i = 1; i < nrows; ++i)
    pointer[i] = pointer[i - 1] + ncols;
  return pointer;
}

void ECLDigitizerModule::deallocate(std::vector<int**> matrices) const
{
  for (int** element : matrices) deallocate(element);
}
void ECLDigitizerModule::deallocate(int** pointer) const
{
  delete[] pointer[0];
  delete[] pointer;
}

void ECLDigitizerModule::event()
{


  StoreObjPtr< ECLLookupTable> eclWFAlgoParamsTable("ECLWFAlgoParamsTable", DataStore::c_Persistent);

  //Input Array
  StoreArray<ECLHit> eclHits;
  if (!eclHits) {
    B2DEBUG(100, "ECLHit in empty in event " << m_nEvent);
  }

  // Output Arrays
  StoreArray<ECLDigit> eclDigits;
  StoreArray<ECLDsp> eclDsps;
  StoreArray<ECLTrig> eclTrigs;


  int energyFit[8736] = {0}; //fit output : Amplitude
  int tFit[8736] = {0};    //fit output : T_ave
  int qualityFit[8736] = {0};    //fit output : T_ave
  array2d HitEnergy(boost::extents[8736][31]);
  std::fill(HitEnergy.origin(), HitEnergy.origin() + HitEnergy.size(), 0.0);
  double E_tmp[8736] = {0};
  double test_A[31] = {0};
  float AdcNoise[31];
  float genNoise[31];

  double dt = .02; //delta t for interpolation
  int n = 1250;//provide a shape array for interpolation
  double DeltaT =  gRandom->Uniform(0, 144);


  if (! m_calibration) { // normal running

    for (const auto& eclHit : eclHits) {
      int hitCellId       =  eclHit.getCellId() - 1; //0~8735
      double hitE         =  eclHit.getEnergyDep()  / Unit::GeV;
      double hitTimeAve   =  eclHit.getTimeAve() / Unit::us;

      if (hitTimeAve > 8.5) { continue;}
      E_tmp[hitCellId] = hitE + E_tmp[hitCellId];//for summation deposit energy; do fit if this summation > 0.1 MeV

      for (int T_clock = 0; T_clock < 31; T_clock++) {
        double timeInt =  DeltaT * 2. / 508.; //us
        double sampleTime = (24. * 12. / 508.)  * (T_clock - 15) - hitTimeAve - timeInt + 0.32
                            ;//There is some time shift~0.32 us that is found Alex 2013.06.19.
        //        DspSamplingArray(&n, &sampleTime, &dt, m_ft, &test_A[T_clock]);//interpolation from shape array n=1250; dt =20ns
        test_A[T_clock] = DspSamplingArray(n, sampleTime, dt, & (*m_ft)[0]);//interpolation from shape array n=1250; dt =20ns
        HitEnergy[hitCellId][T_clock] = test_A[T_clock]  * hitE  +  HitEnergy[hitCellId][T_clock];
      }//for T_clock 31 clock

    } //end loop over eclHitArray ii

  } else { // calibration mode

    // This has been added by Alex Bobrov for calibration
    // of covariance matrix artificially generate 100 MeV in time for each crystal

    int JiP;
    for (JiP = 0; JiP < 8736; JiP++) {
      double hitE = 0.1;
      double hitTimeAve = 0.0;
      E_tmp[JiP] = hitE + E_tmp[JiP];//for summation deposit energy; do fit if this summation > 0.1 MeV

      for (int T_clock = 0; T_clock < 31; T_clock++) {
        double timeInt =  DeltaT * 2. / 508.; //us
        double sampleTime = (24. * 12. / 508.)  * (T_clock - 15) - hitTimeAve - timeInt + 0.32
                            ;//There is some time shift~0.32 us that is found Alex 2013.06.19.
        test_A[T_clock] = DspSamplingArray(n, sampleTime, dt, & (*m_ft)[0]);//interpolation from shape array n=1250; dt =20ns
        //        DspSamplingArray(&n, &sampleTime, &dt, m_ft, &test_A[T_clock]);//interpolation from shape array n=1250; dt =20ns
        HitEnergy[JiP][T_clock] = test_A[T_clock]  * hitE  +  HitEnergy[JiP][T_clock];
      }//for T_clock 31 clock
    }
    // end of Alex Bobrov ad-hoc calibration...

  } // end of normal / calibration mode branches


  for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
    if (m_calibration || E_tmp[iECLCell] > 0.0001) { // Bobrov removes this cut in calibration
      //Noise generation
      for (int iCal = 0; iCal < 31; iCal++) {
        genNoise[iCal] =  gRandom->Gaus(0, 1);
      }

      for (int T_clock = 0; T_clock < 31; T_clock++) {
        AdcNoise[T_clock] = 0;
        for (int iCal = 0; iCal < T_clock; iCal++) {
          AdcNoise[T_clock] = m_elecNoiseM[T_clock][iCal] * genNoise[iCal] + AdcNoise[T_clock];
        }
      }

      for (int  T_clock = 0; T_clock < 31; T_clock++) {
        FitA[T_clock] = (int)(HitEnergy[iECLCell][T_clock] * 20000 + 3000 + AdcNoise[T_clock] * 20) ;
      }

      m_n16 = 16;
      m_lar = 0;
      m_ltr = 0;
      m_lq = 0;

      m_ttrig = int(DeltaT) ;
      if (m_ttrig < 0)m_ttrig = 0;
      if (m_ttrig > 143)m_ttrig = 143;

      unsigned int idIdx = (*eclWFAlgoParamsTable)[ iECLCell + 1 ]; //lookup table uses cellID definition [1,8736]
      unsigned int funcIdx = m_funcTable[iECLCell + 1 ];
      shapeFitter(&(m_idn[idIdx][0]), &(m_f[funcIdx][0][0]), &(m_f1[funcIdx][0][0]), &(m_fg41[funcIdx][0][0]), &(m_fg43[funcIdx][0][0]),
                  &(m_fg31[funcIdx][0][0]), &(m_fg32[funcIdx][0][0]), &(m_fg33[funcIdx][0][0]), &(FitA[0]), &m_ttrig,  &m_n16,  &m_lar, &m_ltr,
                  &m_lq);

      energyFit[iECLCell] = m_lar; //fit output : Amplitude 18-bits
      tFit[iECLCell] = m_ltr;    //fit output : T_ave 12 bits
      qualityFit[iECLCell] = m_lq;    //fit output : quality 2 bits

      if (energyFit[iECLCell] > 0) {
        const auto eclDsp = eclDsps.appendNew();
        eclDsp->setCellId(iECLCell + 1);
        eclDsp->setDspA(FitA);

        const auto eclDigit = eclDigits.appendNew();
        eclDigit->setCellId(iECLCell + 1);//iECLCell + 1= 1~8736
        eclDigit->setAmp(energyFit[iECLCell]);//E (GeV) = energyFit/20000;
        eclDigit->setTimeFit(tFit[iECLCell]);//t0 (us)= (1520 - m_ltr)*24.*12/508/(3072/2) ;
        eclDigit->setQuality(qualityFit[iECLCell]);
      }

    }//if Energy > 0.1 MeV
  } //store  each crystal hit

  const auto eclTrig = eclTrigs.appendNew();
  //  eclTrigArray[m_hitNum2]->setTimeTrig(DeltaT * 12. / 508.); //t0 (us)= (1520 - m_ltr)*24.*12/508/(3072/2) ;
  eclTrig->setTimeTrig(DeltaT  / 508.); //t0 (us)= (1520 - m_ltr)*24.*

  m_nEvent++;
}

void ECLDigitizerModule::endRun()
{
}

void ECLDigitizerModule::terminate()
{
  deallocate(m_f);
  deallocate(m_f1);
  deallocate(m_fg31);
  deallocate(m_fg32);
  deallocate(m_fg33);
  deallocate(m_fg41);
  deallocate(m_fg43);

  for (auto i : m_idn) delete[] i;
  delete m_ft;
}

int ECLDigitizerModule::myPow(int x, int p)
{
  if (p == 0) return 1;
  if (p == 1) return x;
  return x * myPow(x, p - 1);
}

/*
double ECLDigitizerModule::DspSamplingArray(int templateSignalNBins, double time, double templateSignalBinLen, const double* templateFun) const
{
  int i, inext;
  double s1, s2;
  i = time / templateSignalBinLen;

  if (i < 0 || (i + 1) >= templateSignalNBins )
    return 0;

  s1 = templateFun[i];

  inext = i + 1;
  if (i < templateSignalNBins) {
    s2 = templateFun[inext];
  } else {
    s2 = 0;
  }
  return s1 + (s2 - s1) * (time / templateSignalBinLen - i);
}


void ECLDigitizerModule::DspSamplingArray(int* n, double* t, double* dt, double* ft, double* ff)
{
  int i, i1;
  double s1, s2;
  i = (*t) / (*dt);

  if (i < 0 || (i + 1) >= (*n)) {
    *ff = 0;
    return;
  }
  s1 = ft[i];

  i1 = i + 1;
  if (i < *n) {
    s2 = ft[i1];
  } else {
    s2 = 0;
  }
  *ff = s1 + (s2 - s1) * (*t / (*dt) - i);

  return;
}


double ECLDigitizerModule::ShaperDSP_F(double Ti, float* ss)
{
  double svp = 0;
  double tr1 = Ti * 0.881944444;

  double s[12] = {0, 27.7221, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  s[2] = (double) * (ss + 0);
  s[3] = (double) * (ss + 1);
  s[4] = (double) * (ss + 2);
  s[5] = (double) * (ss + 3);
  s[6] = (double) * (ss + 4);
  s[7] = (double) * (ss + 5);
  s[8] = (double) * (ss + 6);
  s[9] = (double) * (ss + 7);
  s[10] = (double) * (ss + 8);
  s[11] = (double) * (ss + 9);

  double tr = tr1 - s[2];
  double tr2 = tr + .2;
  double tr3 = tr - .2;
  if (tr2 > 0.) {

    svp = (Sv123(tr , s[4], s[5], s[9], s[10], s[3], s[6]) * (1 - s[11])
           + s[11] * .5 * (Sv123(tr2, s[4], s[5], s[9], s[10], s[3], s[6])
                           + Sv123(tr3, s[4], s[5], s[9], s[10], s[3], s[6])));
    double x = tr / s[4];


    svp = s[1] * (svp - s[7] * (exp(-tr / s[8]) *
                                (1 - exp(-x) * (1 + x + x * x / 2 + x * x * x / 6 + x * x * x * x / 24 + x * x * x * x * x / 120))));
  } else svp = 0 ;
  return svp;

}

double ECLDigitizerModule::ShaperDSP(double Ti)
{
  double svp = 0;
  double tr1 = Ti * 0.881944444;

  double s[12] = {0, 27.7221, 0.5, 0.6483, 0.4017, 0.3741, 0.8494, 0.00144547, 4.7071, 0.8156, 0.5556, 0.2752};


  double tr = tr1 - s[2];
  double tr2 = tr + .2;
  double tr3 = tr - .2;
  if (tr2 > 0.) {

    svp = (Sv123(tr , s[4], s[5], s[9], s[10], s[3], s[6]) * (1 - s[11])
           + s[11] * .5 * (Sv123(tr2, s[4], s[5], s[9], s[10], s[3], s[6])
                           + Sv123(tr3, s[4], s[5], s[9], s[10], s[3], s[6])));
    double x = tr / s[4];


    svp = s[1] * (svp - s[7] * (exp(-tr / s[8]) *
                                (1 - exp(-x) * (1 + x + x * x / 2 + x * x * x / 6 + x * x * x * x / 24 + x * x * x * x * x / 120))));
  } else svp = 0 ;
  return svp;

}

double  ECLDigitizerModule::Sv123(double t, double t01, double tb1, double t02, double tb2, double td1, double ts1)
{

  double sv123 = 0.;
  double  dks0, dks1, dksm,
          dw0, dw1, dwp, dwm, das1, dac1, das0, dac0, dzna, dksm2, ds, dd,
          dcs0, dsn0, dzn0, td, ts, dr,
          dcs0s, dsn0s, dcs0d, dsn0d, dcs1s, dsn1s, dcs1d, dsn1d;


  if (t < 0.) return 0.;

  dr = (ts1 - td1) / td1;
  if (abs(dr) >= 0.0000001) {
    td = td1;
    ts = ts1;
  } else {
    td = td1;
    if (ts1 > td1) {
      ts = td1 * 1.00001;
    } else {
      ts = td1 * 0.99999;
    }
  }

  dr = ((t01 - t02) * (t01 - t02) + (tb1 - tb2) * (tb1 - tb2)) / ((t01) * (t01) + (tb1) * (tb1));
  dks0 = 1.0 / t01;
  dks1 = 1.0 / t02;

  if (dr < 0.0000000001) {

    if (dks0 > dks1) {
      dks0 = dks1 * 1.00001;
    } else {
      dks0 = dks1 * 0.99999;
    }
  }

  if (t < 0.) return 0;



  dksm = dks1 - dks0;

  ds = 1. / ts;
  dd = 1. / td;

  dw0 = 1. / tb1;
  dw1 = 1. / tb2;
  dwp = dw0 + dw1;
  dwm = dw1 - dw0;

  dksm2 = dksm * dksm;

  dzna = (dksm2 + dwm * dwm) * (dksm2 + dwp * dwp);


  das0 = dw1 * (dksm2 + dwp * dwm);
  dac0 = -2 * dksm * dw0 * dw1;
  das1 = dw0 * (dksm2 - dwp * dwm);
  dac1 = -dac0;





  dsn0 = (ds - dks0);
  dcs0 = -dw0;
  dzn0 = dcs0 * dcs0 + dsn0 * dsn0;

  dsn0s = (dsn0 * das0 - dcs0 * dac0) / dzn0;
  dcs0s = (dcs0 * das0 + dsn0 * dac0) / dzn0;

  dsn0 = (ds - dks1);
  dcs0 = -dw1;
  dzn0 = dcs0 * dcs0 + dsn0 * dsn0;

  dsn1s = (dsn0 * das1 - dcs0 * dac1) / dzn0;
  dcs1s = (dcs0 * das1 + dsn0 * dac1) / dzn0;


  dsn0 = (dd - dks0);
  dcs0 = -dw0;
  dzn0 = dcs0 * dcs0 + dsn0 * dsn0;

  dsn0d = (dsn0 * das0 - dcs0 * dac0) / dzn0;
  dcs0d = (dcs0 * das0 + dsn0 * dac0) / dzn0;

  dsn0 = (dd - dks1);
  dcs0 = -dw1;
  dzn0 = dcs0 * dcs0 + dsn0 * dsn0;

  dsn1d = (dsn0 * das1 - dcs0 * dac1) / dzn0;
  dcs1d = (dcs0 * das1 + dsn0 * dac1) / dzn0;

  //cppcheck dr = (ts - td) / td;




  sv123 = ((((dsn0s - dsn0d) * sin(dw0 * t)
             + (dcs0s - dcs0d) * cos(dw0 * t)) * exp(-t * dks0)
            - (dcs0s + dcs1s) * exp(-t * ds) + (dcs0d + dcs1d) * exp(-t * dd)
            + ((dsn1s - dsn1d) * sin(dw1 * t)
               + (dcs1s - dcs1d) * cos(dw1 * t)) * exp(-t * dks1)) / dzna / (ts - td));

  return sv123;


}

*/
void ECLDigitizerModule::shapeFitter(short int* id, int* f, int* f1, int* fg41, int* fg43, int* fg31, int* fg32, int* fg33, int* y,
                                     int* ttrig2, int* n16,  int* lar, int* ltr, int* lq)
{



  static long long int k_np[16] = {
    65536,
    32768,
    21845,
    16384,
    13107,
    10923,
    9362,
    8192,
    7282,
    6554,
    5958,
    5461,
    5041,
    4681,
    4369,
    4096
  };


  int A0  = (int) * (id + 0) - 128;
  int Askip  = (int) * (id + 1) - 128;

  int ttrig;
  int Ahard  = (int) * (id + 2);
  int k_a = (int) * ((unsigned char*)id + 26);
  int k_b = (int) * ((unsigned char*)id + 27);
  int k_c = (int) * ((unsigned char*)id + 28);
  int k_16 = (int) * ((unsigned char*)id + 29);
  int k1_chi = (int) * ((unsigned char*)id + 24);
  int k2_chi = (int) * ((unsigned char*)id + 25);

  int chi_thres = (int) * (id + 15);



  int s1, s2;

  long long int z00;

  int ys;
  ys = 0;
  int it, it0;
//  long long d_it;
  int it_h, it_l;
  long long A1, B1, A2, C1, ch1, ch2, B2, B3, B5 ;
  int low_ampl, i, T, iter;

  ttrig = *ttrig2 / 6;


  if (k_16 + *n16 != 16) {
    printf("disagreement in number of the points %d %d \n", k_16, *n16);
  }


  int validity_code = 0;
  for (i = ys, z00 = 0; i < 16; i++) {
    z00 += y[i];

  }
  //initial time index



  it0 = 48 + ((23 - ttrig) << 2);//Alex modify


  if (ttrig > 23) {cout << "*Ttrig  Warning" << ttrig << endl; ttrig = 23;}
  if (ttrig < 0) {cout << "*Ttrig  Warning" << ttrig << endl; ttrig = 0;}




  it_h = 191;
  it_l = 0;
  if (it0 < it_l)it0 = it_l;
  if (it0 > it_h)it0 = it_h;
  it = it0;

  //first approximation without time correction

  //  int it00=23-it0;

  s1 = (*(fg41 + ttrig * 16));


  A2 = (s1 * z00);



  for (i = 1; i < 16; i++) {
    s1 = (*(fg41 + ttrig * 16 + i));
    B3 = y[15 + i];
    B3 = s1 * B3;
    A2 += B3;


  }


  A2 += (1 << (k_a - 1));
  A2 >>= k_a;
  T = 0;
  //too large amplitude
  if (A2 > 262143) {
    A1 = A2 >> 3;
    validity_code = 1;

    goto ou;
  }


  low_ampl = 0;



  if (A2 >= A0) {

    for (iter = 0, it = it0; iter < 3;) {
      iter++;
      s1 = (*(fg31 + it * 16));
      s2 = (*(fg32 + it * 16));
      A1 = (s1 * z00);
      B1 = (s2 * z00);



      for (i = 1; i < 16; i++) {
        s1 = (*(fg31 + i + it * 16));
        s2 = (*(fg32 + i + it * 16));

        B5 = y[15 + i];

        B5 = s1 * B5;
        A1 += B5;


        B3 = y[15 + i];
        B3 = s2 * B3;
        B1 += B3;
      }
      A1 += (1 << (k_a - 1));
      A1 = A1 >> k_a;



      if (A1 > 262143)
        goto ou;
      if (A1 < A0) {

        low_ampl = 1;
        it = it0;

        goto lam;
      }

      if (iter != 3) {

        B2 = B1 >> (k_b - 9);
        B1 = B2 >> 9;

        B2 += (A1 << 9);


        B3 = (B2 / A1);


        it += ((B3 + 1) >> 1) - 256;
        it = it > it_h ? it_h : it;
        it = it < it_l ? it_l : it;
      } else {
        B2 = B1 >> (k_b - 13);
        B5 = B1 >> (k_b - 9);


        B1 = B2 >> 13;
        B2 += (A1 << 13);
        B3 = (B2 / A1);


        T = ((it) << 3) + ((it) << 2) + (((B3 >> 1) + (B3) + 2) >> 2) - 3072;

        T = ((210 - *ttrig2) << 3) - T;



        B1 = B5 >> 9;
        B5 += (A1 << 9);
        B3 = (B5 / A1);
        it += ((B3 + 1) >> 1) - 256;
        it = it > it_h ? it_h : it;
        it = it < it_l ? it_l : it;


        T = T > 2047 ?  T - 4096 * ((T + 2048) / 4096) : T;

        T = T < -2048 ? T + 4096 * ((-T + 2048) / 4096) : T;


        C1 = (*(fg33 + it * 16) * z00);

        for (i = 1; i < 16; i++)
          C1 += *(fg33 + i + it * 16) * y[15 + i];
        C1 += (1 << (k_c - 1));
        C1 >>= k_c;


      }

    }
  } else
    low_ampl = 1;

  if (low_ampl == 1) {

lam:
    A1 = A2;
    validity_code = 0;
    B1 = 0;
    C1 = (*(fg43 + ttrig * 16) * z00);
    for (i = 1; i < 16; i++) {
      B5 = y[15 + i];
      C1 += *(fg43 + i + ttrig * 16) * B5;
    }
    C1 += (1 << (k_c - 1));
    C1 >>= k_c;
  }
  ch2 = (A1** (f + it * 16) + B1** (f1 + it * 16)) >> k1_chi;
  ch2 += C1;
  //  ch2 = z0 - *n16 * ch2;
  ch2 = z00 - *n16 * C1;
  ch1 = ((ch2) * (ch2));
  ch1 = ch1 * k_np[*n16 - 1];
  ch1 = ch1 >> 16;
  for (i = 1; i < 16; i++) {
    ch2 = A1 * (*(f + i + it * 16)) + B1 * (*(f1 + i + it * 16));
    ch2 >>= k1_chi;
    ch2 = (y[i + 15] - ch2 - C1);

    ch1 = ch1 + ch2 * ch2;

  }
  B2 = (A1 >> 1) * (A1 >> 1);
  B2 >>= (k2_chi - 2);
  B2 += chi_thres;
  if (ch1 > B2)validity_code = 3;
ou:

  *lar = A1;
  *ltr = T;

  if (A1 < Askip)validity_code = validity_code + 8;

  int ss = (y[20] + y[21]);

  if (ss <= Ahard)validity_code = validity_code + 4;


  *lq = validity_code;


  return ;
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


  StoreArray<ECLWaveformData> eclWaveformData("ECLWaveformData", DataStore::c_Persistent);
  eclWaveformData.registerInDataStore();
  StoreObjPtr< ECLLookupTable > eclWaveformDataTable("ECLWaveformDataTable", DataStore::c_Persistent);;
  eclWaveformDataTable.registerInDataStore();
  eclWaveformDataTable.create();
  StoreArray<ECLWFAlgoParams> eclWFAlgoParams("ECLWFAlgoParams", DataStore::c_Persistent);
  eclWFAlgoParams.registerInDataStore();
  StoreObjPtr< ECLLookupTable> eclWFAlgoParamsTable("ECLWFAlgoParamsTable", DataStore::c_Persistent);
  eclWFAlgoParamsTable.registerInDataStore();
  eclWFAlgoParamsTable.create();
  StoreArray<ECLNoiseData> eclNoiseData("ECLNoiseData", DataStore::c_Persistent);
  eclNoiseData.registerInDataStore();
  StoreObjPtr< ECLLookupTable > eclNoiseDataTable("ECLNoiseDataTable", DataStore::c_Persistent);;
  eclNoiseDataTable.registerInDataStore();
  eclNoiseDataTable.create();

  TFile rootfile(dataFileName.c_str());
  TTree* tree = (TTree*) rootfile.Get("EclWF");
  TTree* tree2 = (TTree*) rootfile.Get("EclAlgo");
  TTree* tree3 = (TTree*) rootfile.Get("EclNoise");

  if (tree == 0 || tree2 == 0 || tree3 == 0)
    B2FATAL("Data not found");

  Int_t ncellId;
  Int_t cellId[8736];
  Int_t ncellId2;
  Int_t cellId2[8736];
  Int_t ncellId3;
  Int_t cellId3[8736];


  tree->GetBranch("CovarianceM")->SetAutoDelete(kFALSE);
  tree->SetBranchAddress("ncellId", &ncellId);
  tree->SetBranchAddress("cellId", cellId);

  tree2->GetBranch("Algopars")->SetAutoDelete(kFALSE);
  tree2->SetBranchAddress("ncellId", &ncellId2);
  tree2->SetBranchAddress("cellId", cellId2);

  tree3->GetBranch("NoiseM")->SetAutoDelete(kFALSE);
  tree3->SetBranchAddress("ncellId", &ncellId3);
  tree3->SetBranchAddress("cellId", cellId3);

  ECLWaveformData* info = new ECLWaveformData;
  tree->SetBranchAddress("CovarianceM", &info);
  Long64_t nentries = tree->GetEntries();
  for (Long64_t ev = 0; ev < nentries; ev++) {
    tree->GetEntry(ev);
    eclWaveformData.appendNew(*info);
    for (Int_t i = 0; i < ncellId; ++i)
      (*eclWaveformDataTable) [cellId[i]] = ev;
  }
  delete info;

  ECLWFAlgoParams* algo = new ECLWFAlgoParams;
  tree2->SetBranchAddress("Algopars", &algo);
  nentries = tree2->GetEntries();
  for (Long64_t ev = 0; ev < nentries; ev++) {
    tree2->GetEntry(ev);
    eclWFAlgoParams.appendNew(*algo);
    m_idn.push_back(new short int [16]);
    for (Int_t i = 0; i < ncellId2; ++i)
      (*eclWFAlgoParamsTable) [cellId2[i]] = ev;
  }
  delete algo;

  ECLNoiseData* noise = new ECLNoiseData;
  tree3->SetBranchAddress("NoiseM", &noise);
  nentries = tree3->GetEntries();
  for (Long64_t ev = 0; ev < nentries; ev++) {
    tree3->GetEntry(ev);
    eclNoiseData.appendNew(*noise);
    if (ncellId3 == 0) {
      for (int i = 1; i <= 8736; i++)(*eclNoiseDataTable)[i] = 0;
      break;
    } else {
      for (Int_t i = 0; i < ncellId3; ++i)
        (*eclNoiseDataTable)[cellId3[i]] = ev;
    }
  }
  delete noise;

  rootfile.Close();

  typedef vector< pair<unsigned int, unsigned int> > PairIdx;
  PairIdx pairIdx;

  for (int icell = 1; icell <= 8736; icell++) {
    unsigned int wfIdx = (*eclWaveformDataTable) [ icell ];
    unsigned int algoIdx = (*eclWFAlgoParamsTable) [ icell ];
    pair<unsigned int, unsigned int> wfAlgoIdx = make_pair(wfIdx, algoIdx) ;
    bool found(false);
    for (unsigned int ielem = 0; ielem < pairIdx.size(); ++ielem) {
      if (wfAlgoIdx == pairIdx[ielem]) {
        m_funcTable[ icell ] = ielem;
        found = true;
        break;
      }
    }

    if (!found) {
      m_funcTable[ icell ] = pairIdx.size();
      m_f.push_back(allocateMatrix(192, 16));
      m_f1.push_back(allocateMatrix(192, 16));
      m_fg31.push_back(allocateMatrix(192, 16));
      m_fg32.push_back(allocateMatrix(192, 16));
      m_fg33.push_back(allocateMatrix(192, 16));
      m_fg41.push_back(allocateMatrix(24, 16));
      m_fg43.push_back(allocateMatrix(24, 16));
      pairIdx.push_back(wfAlgoIdx);
    }

  }

  float MP[10];

  //Presently all crystals share the same  Waveform function parameters.
  const ECLWaveformData* eclWFData = eclWaveformData[0];
  eclWFData->getWaveformParArray(MP);

  // varible for one channel

  float ss1[16][16];

  unsigned int ChN;

  double  g1g1[192], gg[192], gg1[192], dgg[192];
  double  sg1[16][192], sg[16][192], sg2[16][192], gg2[192], g1g2[192], g2g2[192];
  double  dgg1[192], dgg2[192];


  double f[192][16];
  double f1[192][16];
  double fg31[192][16];
  double fg32[192][16];
  double fg33[192][16];

  double fg41[24][16];
  double fg43[24][16];



  memset(g1g1, 0, sizeof(g1g1));
  memset(gg, 0, sizeof(gg));
  memset(gg1, 0, sizeof(gg1));
  memset(dgg, 0, sizeof(dgg));
  memset(sg1, 0, sizeof(sg1));
  memset(sg, 0, sizeof(sg));
  memset(sg2, 0, sizeof(sg2));
  memset(gg2, 0, sizeof(gg2));
  memset(g1g2, 0, sizeof(g1g2));
  memset(g2g2, 0, sizeof(g2g2));
  memset(dgg1, 0, sizeof(dgg1));
  memset(dgg2, 0, sizeof(dgg2));


  double dt;
  double ts0;
  double del;


  int j1, endc, j, i;
  double ndt;
  double adt, dt0, t0, ddt, tc1;

  // set but not used
  //double tin;

  double t, tmd, tpd, ssssj, ssssj1, ssssi, ssssi1;
  double svp, svm;


  int ibb, iaa, idd, icc;
  int ia, ib, ic;

  // set but not used!
  // int i16;


  int ipardsp13;
  int ipardsp14;

  int n16;
  int k;

  int c_a;
  int c_b;
  int c_c;
  int c_16;
  int c1_chi;
  int c2_chi;
  int chi_thres;
  int Bhard;
  int Bskip;
  int B0;
  int tr0;
  int tr1;

  // = {tr0 , tr1 , Bhard , 17952 , 19529 , 69 , 0 , 0 , 257 , -1 , 0 , 0 , 256*c1_chi+c2_chi , c_a+256*c_b , c_c+256*c_16 ,chi_thres };


  for (unsigned int ichannel = 0; ichannel < pairIdx.size(); ichannel++) {
    // if (ichannel % 1000 == 0) {printf("!!CnN=%d\n", ichannel);}
    ChN = pairIdx[ichannel].first;
    const ECLWaveformData* eclWFData = eclWaveformData[ ChN ];
    eclWFData->getMatrix(ss1);
    ChN = pairIdx[ichannel].second;
    const ECLWFAlgoParams* eclWFAlgo = eclWFAlgoParams[ChN];
    // shape function parameters

    c_a = eclWFAlgo->getka();
    c_b = eclWFAlgo->getkb();
    c_c = eclWFAlgo->getkc();
    c_16 = eclWFAlgo->gety0s();
    c1_chi = eclWFAlgo->getk1();
    c2_chi = eclWFAlgo->getk2();
    chi_thres = eclWFAlgo->getcT();
    Bhard = eclWFAlgo->gethT();
    B0 = eclWFAlgo->getlAT();
    Bskip = eclWFAlgo->getsT();


    tr0 = B0 + 128;
    tr1 = Bskip + 128;

    // = {tr0 , tr1 , Bhard , 17952 , 19529 , 69 , 0 , 0 , 257 , -1 , 0 , 0 , 256*c1_chi+c2_chi , c_a+256*c_b , c_c+256*c_16 ,chi_thres };

    m_idn[ ChN][ 0] = (short int)tr0;
    m_idn[ ChN][ 1] = (short int)tr1;
    m_idn[ ChN][ 2] = (short int)Bhard;
    m_idn[ ChN][ 3] = (short int)17952;
    m_idn[ ChN][ 4] = (short int)19529;
    m_idn[ ChN][ 5] = (short int)69;
    m_idn[ChN][ 6] = (short int)0;
    m_idn[ChN][ 7] = (short int)0;
    m_idn[ChN][ 8] = (short int)257;
    m_idn[ChN][ 9] = (short int) - 1;
    m_idn[ChN][10] = (short int)0;
    m_idn[ChN][11] = (short int)0;
    m_idn[ChN][12] = (short int)(256 * c1_chi + c2_chi);
    m_idn[ChN][13] = (short int)(c_a + 256 * c_b);
    m_idn[ChN][14] = (short int)(c_c + 256 * (c_16 - 16));
    m_idn[ChN][15] = (short int)(chi_thres);




    ///////////////////rragtds

    del = 0.;
    ts0 = 0.5;
    dt = 0.5;


    ndt = 96.;
    adt = 1. / ndt;
    endc = 2 * ndt;


    dt0 = adt * dt;
    t0 = -dt0 * ndt;




    for (j1 = 0; j1 < endc; j1++) {
      t0 = t0 + dt0;
      t = t0 - dt - del;

      // set but not used!
      //tin = t + dt;

      for (j = 0; j < 16; j++) {
        t = t + dt;

        if (t > 0) {


          f[j1][j] = ShaperDSP_F(t / 0.881944444, MP);

          ddt = 0.005 * dt;
          tc1 = t - ts0;
          tpd = t + ddt;
          tmd = t - ddt;


          if (tc1 > ddt) {
            svp = ShaperDSP_F(tpd / 0.881944444, MP);
            svm = ShaperDSP_F(tmd / 0.881944444, MP);

            f1[j1][j] = (svp - svm) / 2. / ddt;

          } else {


            f1[j1][j] = ShaperDSP_F(tpd / 0.881944444, MP) / (ddt + tc1);


          }// else tc1>ddt


        } //if t>0
        else {

          f[j1][j] = 0.;
          f1[j1][j] = 0.;

        }

      } //for j


    }



    for (j1 = 0; j1 < endc; j1++) {

      gg[j1] = 0.;
      gg1[j1] = 0.;
      g1g1[j1] = 0.;
      gg2[j1] = 0.;
      g1g2[j1] = 0.;
      g2g2[j1] = 0.;
      for (j = 0; j < 16; j++) {
        sg[j][j1] = 0.;
        sg1[j][j1] = 0.;
        sg2[j][j1] = 0.;

        ssssj1 = f1[j1][j];
        ssssj = f[j1][j];

        for (i = 0; i < 16; i++) {



          sg[j][j1] = sg[j][j1] + ss1[j][i] * f[j1][i];
          sg1[j][j1] = sg1[j][j1] + ss1[j][i] * f1[j1][i];

          sg2[j][j1] = sg2[j][j1] + ss1[j][i];




          ssssi = f[j1][i];
          ssssi1 = f1[j1][i];

          gg[j1] = gg[j1] + ss1[j][i] * ssssj * ssssi;



          gg1[j1] = gg1[j1] + ss1[j][i] * ssssj * ssssi1;
          g1g1[j1] = g1g1[j1] + ss1[j][i] * ssssi1 * ssssj1;

          gg2[j1] = gg2[j1] + ss1[j][i] * ssssj;
          g1g2[j1] = g1g2[j1] + ss1[j][i] * ssssj1;
          g2g2[j1] = g2g2[j1] + ss1[j][i];


        }   // for i


      } //for j

      dgg[j1] = gg[j1] * g1g1[j1] - gg1[j1] * gg1[j1];
      dgg1[j1] = gg[j1] * g2g2[j1] - gg2[j1] * gg2[j1];
      dgg2[j1] = gg[j1] * g1g1[j1] * g2g2[j1] - gg1[j1] * gg1[j1] * g2g2[j1] + 2 * gg1[j1] * g1g2[j1] * gg2[j1] - gg2[j1] * gg2[j1] *
                 g1g1[j1] - g1g2[j1] * g1g2[j1] * gg[j1];


      for (i = 0; i < 16; i++) {
        if (dgg2[j1] != 0) {

          fg31[j1][i] = ((g1g1[j1] * g2g2[j1] - g1g2[j1] * g1g2[j1]) * sg[i][j1] + (g1g2[j1] * gg2[j1] - gg1[j1] * g2g2[j1]) * sg1[i][j1] +
                         (gg1[j1] * g1g2[j1] - g1g1[j1] * gg2[j1]) * sg2[i][j1]) / dgg2[j1];



          fg32[j1][i] = ((g1g2[j1] * gg2[j1] - gg1[j1] * g2g2[j1]) * sg[i][j1] + (gg[j1] * g2g2[j1] - gg2[j1] * gg2[j1]) * sg1[i][j1] +
                         (gg1[j1] * gg2[j1] - gg[j1] * g1g2[j1]) * sg2[i][j1]) / dgg2[j1];


          fg33[j1][i] = ((gg1[j1] * g1g2[j1] - g1g1[j1] * gg2[j1]) * sg[i][j1] + (gg1[j1] * gg2[j1] - gg[j1] * g1g2[j1]) * sg1[i][j1] +
                         (gg[j1] * g1g1[j1] - gg1[j1] * gg1[j1]) * sg2[i][j1]) / dgg2[j1];


        }


        if (dgg1[j1] != 0) {
          if (j1 < 24) {

            fg41[j1][i] = (g2g2[j1] * sg[i][j1] - gg2[j1] * sg2[i][j1]) / dgg1[j1];
            fg43[j1][i] = (gg[j1] * sg2[i][j1] - gg2[j1] * sg[i][j1]) / dgg1[j1];
          }

        }





      }  // for i


    } // for j1 <endc




    //%%%%%%%%%%%%%%%%%%%adduction to integer

    n16 = 16;

    ipardsp13 = 14 + 14 * 256;
    ipardsp14 = 0 * 256 + 17;

    ibb = ipardsp13 / 256;
    iaa = ipardsp13 - ibb * 256;
//  idd=ipardsp14/256;
    icc = ipardsp14 - idd * 256;

    iaa = c_a;
    ibb = c_b;
    icc = c_c;


    ia = myPow(2, iaa);
    ib = myPow(2, ibb);
    ic = myPow(2, icc);

    // set but not used!
    // i16 = myPow(2, 15);


    ChN = ichannel;

    for (i = 0; i < 16; i++) {
      if (i == 0) {idd = n16;}
      else {idd = 1;}
      for (k = 0; k < 192; k++) {

        //  cout << "Reading channel " << ChN << " " << k << " " << i << endl;
        (m_f[ChN])[k][i] = (int)(f[k][i] * ia / idd + ia + 0.5) - ia;

        (m_f1[ChN])[k][i] = (int)(4 * f1[k][i] * ia / idd / 3 + ia + 0.5) - ia;




        (m_fg31[ChN])[k][i] = (int)(fg31[k][i] * ia / idd + ia + 0.5) - ia;

        (m_fg32[ChN])[k][i] = (int)(3 * fg32[k][i] * ib / idd / 4 + ib + 0.5) - ib;



        (m_fg33[ChN])[k][i] = (int)(fg33[k][i] * ic / idd + ic + 0.5) - ic;


        if (k <= 23) {

          (m_fg41[ChN])[k][i] = (int)(fg41[k][i] * ia / idd + ia + 0.5) - ia;


          (m_fg43[ChN])[k][i] = (int)(fg43[k][i] * ic / idd + ic + 0.5) - ic;



        }  // if k<=23


      } // for k


    }  // for i


  }

  const ECLNoiseData* eclNoise = eclNoiseData[0];
  eclNoise->getMatrix(m_elecNoiseM);

  TTree* tree4 = (TTree*) rootfile.Get("EclSampledSignalWF");
  if (tree4 != 0) {
    tree4->SetBranchAddress("EclSampledSignalWF", &m_ft);
    tree4->GetEntry(0);
  } else  m_ft = createDefSampledSignalWF();

}
