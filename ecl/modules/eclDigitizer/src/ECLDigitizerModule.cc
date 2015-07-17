/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclDigitizer/ECLDigitizerModule.h>
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

//  addParam("RandomSeed", m_randSeed, "User-supplied random seed; Default 0 for ctime", (unsigned int)(0));


}


ECLDigitizerModule::~ECLDigitizerModule()
{

}

void ECLDigitizerModule::initialize()
{
  if (m_background)
    m_nEvent  = 0 ;
  readDSPDB();

  StoreArray<ECLDsp>::registerPersistent();
  StoreArray<ECLDigit>::registerPersistent();
  StoreArray<ECLTrig>::registerPersistent();

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

  for (const auto& eclHit : eclHits) {
    int hitCellId       =  eclHit.getCellId() - 1; //0~8735
    double hitE         =  eclHit.getEnergyDep()  / Unit::GeV;
    double hitTimeAve   =  eclHit.getTimeAve() / Unit::us;
    double sampleTime;

    if (hitTimeAve > 8.5) { continue;}
    E_tmp[hitCellId] = hitE + E_tmp[hitCellId];//for summation deposit energy; do fit if this summation > 0.1 MeV

    for (int T_clock = 0; T_clock < 31; T_clock++) {
      double timeInt =  DeltaT * 2. / 508.; //us
      sampleTime = (24. * 12. / 508.)  * (T_clock - 15) - hitTimeAve - timeInt + 0.32
                   ;//There is some time shift~0.32 us that is found Alex 2013.06.19.
      DspSamplingArray(&n, &sampleTime, &dt, m_ft, &test_A[T_clock]);//interpolation from shape array n=1250; dt =20ns
      HitEnergy[hitCellId][T_clock] = test_A[T_clock]  * hitE  +  HitEnergy[hitCellId][T_clock];
    }//for T_clock 31 clock

    /* This has been added by Alex Bobrov for calibration
    of covariance matrix and should be removed when we will have a proper
    calibration procedure

    if(ii == 0){

      int JiP;
      for(JiP=0;JiP<8736;JiP++){
    hitE=0.1;
    hitTimeAve=0.0;
    E_tmp[JiP] = hitE + E_tmp[JiP];//for summation deposit energy; do fit if this summation > 0.1 MeV

    for (int T_clock = 0; T_clock < 31; T_clock++) {
    double timeInt =  DeltaT*2. / 508.; //us

    sampleTime = (24. * 12. / 508.)  * (T_clock - 15) - hitTimeAve - timeInt + 0.32 ;//There is some time shift~0.32 us that is found Alex 2013.06.19.
    DspSamplingArray(&n, &sampleTime, &dt, m_ft, &test_A[T_clock]);//interpolation from shape array n=1250; dt =20ns
    HitEnergy[JiP][T_clock] = test_A[T_clock]  * hitE  +  HitEnergy[JiP][T_clock];
    }//for T_clock 31 clock

      }
    }

    end of Alex Bobrov ad-hoc calibration...
    */

  } //end loop over eclHitArray ii


  for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
    if (E_tmp[iECLCell] > 0.0001) { // Bobrov removes this cut in calibration
      //Noise generation
      for (int iCal = 0; iCal < 31; iCal++) {
        genNoise[iCal] =  gRandom->Gaus(0, 1);
      }

      for (int T_clock = 0; T_clock < 31; T_clock++) {
        AdcNoise[T_clock] = 0;
        for (int iCal = 0; iCal < T_clock; iCal++) {
          AdcNoise[T_clock] = m_vmat[T_clock][iCal] * genNoise[iCal] + AdcNoise[T_clock];
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
}

int ECLDigitizerModule::myPow(int x, int p)
{
  if (p == 0) return 1;
  if (p == 1) return x;
  return x * myPow(x, p - 1);
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




//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%5



  double par_shape[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, -7.51697e-07, -3.78028e-07,
                        -8.67619e-08, 3.41302e-07, 1.23072e-06, 3.04439e-06, 6.40817e-06, 1.21281e-05, 2.12008e-05, 3.48187e-05, 5.43696e-05, 8.14324e-05,
                        0.000117797, 0.000165613, 0.000227605, 0.000307253, 0.000408928, 0.000537989, 0.000700847, 0.00090499, 0.00115899, 0.00147249,
                        0.00185614, 0.00232157, 0.00288129, 0.00354872, 0.00433806, 0.00526424, 0.00634286, 0.00759005, 0.00902239, 0.0106568,
                        0.0125104, 0.0146005, 0.0169442, 0.0195586, 0.0224606, 0.0256664, 0.0291921, 0.0330529, 0.0372636, 0.0418379,
                        0.0467888, 0.0521282, 0.0578671, 0.0640153, 0.0705814, 0.077573, 0.084996, 0.0928553, 0.101154, 0.109896,
                        0.119079, 0.128705, 0.138772, 0.149274, 0.160209, 0.17157, 0.183349, 0.195538, 0.208126, 0.221103,
                        0.234457, 0.248173, 0.262237, 0.276634, 0.291346, 0.306357, 0.321649, 0.337201, 0.352995, 0.369009,
                        0.385223, 0.401615, 0.418163, 0.434844, 0.451637, 0.468517, 0.485461, 0.502447, 0.519451, 0.536449,
                        0.553419, 0.570336, 0.587178, 0.603922, 0.620546, 0.637027, 0.653343, 0.669473, 0.685397, 0.701093,
                        0.716542, 0.731725, 0.746622, 0.761216, 0.775489, 0.789426, 0.803009, 0.816223, 0.829055, 0.84149,
                        0.853516, 0.86512, 0.876292, 0.88702, 0.897296, 0.907109, 0.916454, 0.925321, 0.933705, 0.941601,
                        0.949003, 0.955907, 0.962311, 0.968212, 0.973608, 0.978499, 0.982884, 0.986764, 0.99014, 0.993014,
                        0.995389, 0.997267, 0.998653, 0.999552, 0.999967, 0.999905, 0.999371, 0.998373, 0.996918, 0.995012,
                        0.992664, 0.989882, 0.986675, 0.983052, 0.979022, 0.974596, 0.969782, 0.964592, 0.959037, 0.953126,
                        0.946871, 0.940282, 0.933372, 0.926152, 0.918632, 0.910826, 0.902744, 0.894397, 0.885799, 0.876961,
                        0.867894, 0.85861, 0.849121, 0.839439, 0.829576, 0.819541, 0.809349, 0.799008, 0.788532, 0.77793,
                        0.767214, 0.756394, 0.745481, 0.734486, 0.723418, 0.712288, 0.701106, 0.689881, 0.678622, 0.66734,
                        0.656041, 0.644737, 0.633434, 0.622142, 0.610868, 0.59962, 0.588405, 0.577232, 0.566106, 0.555035,
                        0.544025, 0.533082, 0.522213, 0.511423, 0.500718, 0.490102, 0.479582, 0.469161, 0.458845, 0.448637,
                        0.438542, 0.428563, 0.418704, 0.408969, 0.399359, 0.389879, 0.380531, 0.371318, 0.362241, 0.353302,
                        0.344505, 0.335849, 0.327337, 0.31897, 0.310749, 0.302674, 0.294748, 0.286969, 0.279339, 0.271858,
                        0.264525, 0.257342, 0.250307, 0.24342, 0.236682, 0.23009, 0.223645, 0.217346, 0.211192, 0.205182,
                        0.199314, 0.193587, 0.188001, 0.182553, 0.177243, 0.172068, 0.167028, 0.162119, 0.157341, 0.152692,
                        0.14817, 0.143773, 0.139499, 0.135345, 0.131311, 0.127393, 0.123591, 0.1199, 0.116321, 0.112849,
                        0.109484, 0.106222, 0.103062, 0.100002, 0.0970386, 0.0941702, 0.0913946, 0.0887094, 0.0861124, 0.0836015,
                        0.0811744, 0.0788289, 0.0765629, 0.0743742, 0.0722606, 0.0702201, 0.0682505, 0.0663496, 0.0645156, 0.0627463,
                        0.0610398, 0.059394, 0.057807, 0.0562769, 0.0548018, 0.0533799, 0.0520093, 0.0506882, 0.049415, 0.0481878,
                        0.047005, 0.0458649, 0.044766, 0.0437067, 0.0426853, 0.0417005, 0.0407508, 0.0398346, 0.0389507, 0.0380977,
                        0.0372743, 0.0364791, 0.035711, 0.0349687, 0.0342511, 0.0335571, 0.0328854, 0.0322352, 0.0316053, 0.0309948,
                        0.0304027, 0.0298281, 0.0292701, 0.0287278, 0.0282004, 0.0276872, 0.0271873, 0.0267001, 0.0262248, 0.0257607,
                        0.0253073, 0.0248639, 0.02443, 0.0240049, 0.0235881, 0.0231792, 0.0227776, 0.022383, 0.0219948, 0.0216127,
                        0.0212363, 0.0208651, 0.020499, 0.0201375, 0.0197804, 0.0194273, 0.0190781, 0.0187324, 0.01839, 0.0180508,
                        0.0177145, 0.0173809, 0.01705, 0.0167215, 0.0163953, 0.0160712, 0.0157492, 0.0154292, 0.0151111, 0.0147948,
                        0.0144802, 0.0141673, 0.013856, 0.0135463, 0.0132382, 0.0129316, 0.0126265, 0.012323, 0.012021, 0.0117205,
                        0.0114215, 0.011124, 0.0108282, 0.0105339, 0.0102412, 0.00995022, 0.00966089, 0.00937329, 0.00908744, 0.0088034,
                        0.00852122, 0.00824093, 0.00796259, 0.00768625, 0.00741196, 0.00713978, 0.00686975, 0.00660192, 0.00633636, 0.00607311,
                        0.00581223, 0.00555377, 0.00529778, 0.00504432, 0.00479343, 0.00454516, 0.00429957, 0.00405669, 0.00381658, 0.00357928,
                        0.00334484, 0.00311329, 0.00288467, 0.00265902, 0.00243638, 0.00221679, 0.00200027, 0.00178685, 0.00157657, 0.00136944,
                        0.0011655, 0.000964766, 0.000767257, 0.000572992, 0.000381986, 0.000194254, 9.80634e-06, -0.000171347, -0.000349198, -0.000523742,
                        -0.000694976, -0.000862898, -0.00102751, -0.00118881, -0.00134681, -0.00150151, -0.00165293, -0.00180106, -0.00194593, -0.00208755,
                        -0.00222593, -0.00236109, -0.00249306, -0.00262184, -0.00274746, -0.00286995, -0.00298932, -0.00310562, -0.00321886, -0.00332907,
                        -0.00343628, -0.00354052, -0.00364184, -0.00374025, -0.00383579, -0.00392851, -0.00401842, -0.00410558, -0.00419002, -0.00427178,
                        -0.00435089, -0.0044274, -0.00450135, -0.00457277, -0.00464171, -0.00470822, -0.00477232, -0.00483407, -0.00489351, -0.00495068,
                        -0.00500563, -0.00505839, -0.00510901, -0.00515753, -0.005204, -0.00524846, -0.00529096, -0.00533153, -0.00537023, -0.00540708,
                        -0.00544215, -0.00547546, -0.00550707, -0.005537, -0.00556532, -0.00559205, -0.00561723, -0.00564092, -0.00566314, -0.00568395,
                        -0.00570337, -0.00572145, -0.00573822, -0.00575373, -0.00576801, -0.0057811, -0.00579303, -0.00580384, -0.00581356, -0.00582224,
                        -0.0058299, -0.00583658, -0.00584231, -0.00584712, -0.00585104, -0.00585411, -0.00585635, -0.0058578, -0.00585849, -0.00585843,
                        -0.00585767, -0.00585622, -0.00585412, -0.00585139, -0.00584805, -0.00584413, -0.00583965, -0.00583464, -0.00582912, -0.00582311,
                        -0.00581663, -0.00580971, -0.00580236, -0.0057946, -0.00578645, -0.00577794, -0.00576907, -0.00575986, -0.00575034, -0.00574052,
                        -0.00573041, -0.00572002, -0.00570939, -0.0056985, -0.00568739, -0.00567606, -0.00566453, -0.0056528, -0.00564089, -0.00562881,
                        -0.00561657, -0.00560419, -0.00559166, -0.005579, -0.00556622, -0.00555333, -0.00554033, -0.00552723, -0.00551405, -0.00550078,
                        -0.00548743, -0.00547401, -0.00546053, -0.00544699, -0.0054334, -0.00541976, -0.00540607, -0.00539234, -0.00537858, -0.00536479,
                        -0.00535097, -0.00533713, -0.00532326, -0.00530938, -0.00529549, -0.00528158, -0.00526766, -0.00525373, -0.0052398, -0.00522586,
                        -0.00521192, -0.00519798, -0.00518404, -0.00517011, -0.00515617, -0.00514225, -0.00512832, -0.00511441, -0.0051005, -0.00508659,
                        -0.0050727, -0.00505881, -0.00504493, -0.00503106, -0.0050172, -0.00500335, -0.00498951, -0.00497568, -0.00496186, -0.00494805,
                        -0.00493424, -0.00492045, -0.00490666, -0.00489289, -0.00487912, -0.00486536, -0.00485161, -0.00483786, -0.00482413, -0.0048104,
                        -0.00479668, -0.00478296, -0.00476925, -0.00475555, -0.00474185, -0.00472815, -0.00471447, -0.00470078, -0.0046871, -0.00467343,
                        -0.00465976, -0.00464609, -0.00463242, -0.00461876, -0.0046051, -0.00459145, -0.00457779, -0.00456414, -0.00455049, -0.00453685,
                        -0.0045232, -0.00450956, -0.00449591, -0.00448227, -0.00446864, -0.004455, -0.00444136, -0.00442773, -0.0044141, -0.00440047,
                        -0.00438684, -0.00437322, -0.0043596, -0.00434598, -0.00433236, -0.00431875, -0.00430513, -0.00429153, -0.00427792, -0.00426432,
                        -0.00425073, -0.00423713, -0.00422355, -0.00420997, -0.00419639, -0.00418282, -0.00416926, -0.0041557, -0.00414215, -0.00412861,
                        -0.00411508, -0.00410155, -0.00408803, -0.00407453, -0.00406103, -0.00404754, -0.00403407, -0.0040206, -0.00400715, -0.00399371,
                        -0.00398028, -0.00396687, -0.00395347, -0.00394008, -0.00392671, -0.00391336, -0.00390002, -0.00388669, -0.00387339, -0.0038601,
                        -0.00384683, -0.00383358, -0.00382035, -0.00380713, -0.00379394, -0.00378077, -0.00376762, -0.00375449, -0.00374139, -0.0037283,
                        -0.00371525, -0.00370221, -0.0036892, -0.00367621, -0.00366325, -0.00365031, -0.0036374, -0.00362452, -0.00361166, -0.00359884,
                        -0.00358603, -0.00357326, -0.00356052, -0.0035478, -0.00353512, -0.00352246, -0.00350984, -0.00349725, -0.00348468, -0.00347215,
                        -0.00345965, -0.00344718, -0.00343475, -0.00342234, -0.00340997, -0.00339764, -0.00338534, -0.00337307, -0.00336083, -0.00334863,
                        -0.00333646, -0.00332433, -0.00331224, -0.00330018, -0.00328815, -0.00327616, -0.00326421, -0.00325229, -0.00324041, -0.00322856,
                        -0.00321675, -0.00320498, -0.00319325, -0.00318155, -0.00316989, -0.00315826, -0.00314667, -0.00313513, -0.00312361, -0.00311214,
                        -0.0031007, -0.0030893, -0.00307794, -0.00306662, -0.00305533, -0.00304408, -0.00303287, -0.0030217, -0.00301057, -0.00299947,
                        -0.00298841, -0.00297739, -0.00296641, -0.00295547, -0.00294456, -0.00293369, -0.00292286, -0.00291207, -0.00290131, -0.0028906,
                        -0.00287992, -0.00286927, -0.00285867, -0.0028481, -0.00283757, -0.00282708, -0.00281663, -0.00280621, -0.00279583, -0.00278549,
                        -0.00277518, -0.00276491, -0.00275468, -0.00274449, -0.00273433, -0.00272421, -0.00271412, -0.00270407, -0.00269406, -0.00268408,
                        -0.00267414, -0.00266424, -0.00265437, -0.00264454, -0.00263474, -0.00262498, -0.00261525, -0.00260556, -0.00259591, -0.00258629,
                        -0.0025767, -0.00256715, -0.00255764, -0.00254816, -0.00253871, -0.0025293, -0.00251992, -0.00251058, -0.00250127, -0.00249199,
                        -0.00248275, -0.00247354, -0.00246437, -0.00245522, -0.00244612, -0.00243704, -0.002428, -0.00241899, -0.00241001, -0.00240107,
                        -0.00239216, -0.00238328, -0.00237443, -0.00236562, -0.00235684, -0.00234809, -0.00233937, -0.00233068, -0.00232203, -0.00231341,
                        -0.00230482, -0.00229625, -0.00228773, -0.00227923, -0.00227076, -0.00226232, -0.00225392, -0.00224554, -0.0022372, -0.00222888,
                        -0.0022206, -0.00221234, -0.00220412, -0.00219593, -0.00218776, -0.00217963, -0.00217152, -0.00216345, -0.0021554, -0.00214738,
                        -0.00213939, -0.00213144, -0.00212351, -0.00211561, -0.00210773, -0.00209989, -0.00209207, -0.00208429, -0.00207653, -0.0020688,
                        -0.0020611, -0.00205343, -0.00204578, -0.00203816, -0.00203057, -0.00202301, -0.00201548, -0.00200797, -0.00200049, -0.00199304,
                        -0.00198561, -0.00197821, -0.00197084, -0.0019635, -0.00195618, -0.00194889, -0.00194163, -0.00193439, -0.00192718, -0.00191999,
                        -0.00191283, -0.0019057, -0.0018986, -0.00189152, -0.00188446, -0.00187744, -0.00187043, -0.00186346, -0.00185651, -0.00184958,
                        -0.00184268, -0.00183581, -0.00182896, -0.00182213, -0.00181533, -0.00180856, -0.00180181, -0.00179508, -0.00178839, -0.00178171,
                        -0.00177506, -0.00176843, -0.00176183, -0.00175526, -0.0017487, -0.00174217, -0.00173567, -0.00172919, -0.00172273, -0.0017163,
                        -0.00170989, -0.0017035, -0.00169714, -0.0016908, -0.00168449, -0.0016782, -0.00167193, -0.00166568, -0.00165946, -0.00165326,
                        -0.00164709, -0.00164093, -0.0016348, -0.0016287, -0.00162261, -0.00161655, -0.00161051, -0.00160449, -0.0015985, -0.00159252,
                        -0.00158657, -0.00158064, -0.00157474, -0.00156885, -0.00156299, -0.00155715, -0.00155133, -0.00154553, -0.00153976, -0.001534,
                        -0.00152827, -0.00152256, -0.00151687, -0.0015112, -0.00150555, -0.00149992, -0.00149432, -0.00148873, -0.00148316, -0.00147762,
                        -0.0014721, -0.00146659, -0.00146111, -0.00145565, -0.00145021, -0.00144479, -0.00143939, -0.00143401, -0.00142865, -0.00142331,
                        -0.00141798, -0.00141268, -0.0014074, -0.00140214, -0.0013969, -0.00139168, -0.00138647, -0.00138129, -0.00137613, -0.00137098,
                        -0.00136586, -0.00136075, -0.00135566, -0.00135059, -0.00134554, -0.00134051, -0.0013355, -0.00133051, -0.00132553, -0.00132058,
                        -0.00131564, -0.00131072, -0.00130582, -0.00130094, -0.00129607, -0.00129123, -0.0012864, -0.00128159, -0.0012768, -0.00127203,
                        -0.00126727, -0.00126253, -0.00125781, -0.00125311, -0.00124842, -0.00124375, -0.0012391, -0.00123447, -0.00122985, -0.00122526,
                        -0.00122067, -0.00121611, -0.00121156, -0.00120703, -0.00120252, -0.00119802, -0.00119354, -0.00118908, -0.00118463, -0.00118021,
                        -0.00117579, -0.0011714, -0.00116702, -0.00116265, -0.0011583, -0.00115397, -0.00114966, -0.00114536, -0.00114108, -0.00113681,
                        -0.00113256, -0.00112832, -0.0011241, -0.0011199, -0.00111571, -0.00111154, -0.00110738, -0.00110324, -0.00109912, -0.00109501,
                        -0.00109091, -0.00108683, -0.00108277, -0.00107872, -0.00107469, -0.00107067, -0.00106666, -0.00106268, -0.0010587, -0.00105474,
                        -0.0010508, -0.00104687, -0.00104295, -0.00103905, -0.00103517, -0.0010313, -0.00102744, -0.0010236, -0.00101977, -0.00101596,
                        -0.00101216, -0.00100837, -0.0010046, -0.00100085, -0.000997103, -0.000993374, -0.000989659, -0.000985958, -0.000982271, -0.000978597,
                        -0.000974938, -0.000971292, -0.000967659, -0.000964041, -0.000960435, -0.000956843, -0.000953265, -0.0009497, -0.000946148, -0.00094261,
                        -0.000939085, -0.000935573, -0.000932074, -0.000928588, -0.000925115, -0.000921656, -0.000918209, -0.000914775, -0.000911354, -0.000907945,
                        -0.00090455, -0.000901167, -0.000897796, -0.000894439, -0.000891094, -0.000887761, -0.000884441, -0.000881133, -0.000877838, -0.000874555,
                        -0.000871284, -0.000868025, -0.000864779, -0.000861545, -0.000858322, -0.000855112, -0.000851914, -0.000848728, -0.000845554, -0.000842391,
                        -0.000839241, -0.000836102, -0.000832975, -0.000829859, -0.000826756, -0.000823664, -0.000820583, -0.000817514, -0.000814456, -0.00081141,
                        -0.000808376, -0.000805352, -0.00080234, -0.000799339, -0.00079635, -0.000793371, -0.000790404, -0.000787448, -0.000784503, -0.000781568,
                        -0.000778645, -0.000775733, -0.000772832, -0.000769941, -0.000767062, -0.000764193, -0.000761334, -0.000758487, -0.00075565, -0.000752824,
                        -0.000750008, -0.000747203, -0.000744408, -0.000741624, -0.00073885, -0.000736087, -0.000733334, -0.000730591, -0.000727858, -0.000725136,
                        -0.000722424, -0.000719722, -0.00071703, -0.000714348, -0.000711676, -0.000709015, -0.000706363, -0.000703721, -0.000701089, -0.000698467,
                        -0.000695854, -0.000693252, -0.000690659, -0.000688075, -0.000685502, -0.000682938, -0.000680384, -0.000677839, -0.000675304, -0.000672778,
                        -0.000670261, -0.000667755, -0.000665257, -0.000662769, -0.00066029, -0.00065782, -0.00065536, -0.000652909, -0.000650467, -0.000648034,
                        -0.00064561, -0.000643195, -0.00064079, -0.000638393, -0.000636005, -0.000633626, -0.000631256, -0.000628895, -0.000626543, -0.0006242,
                        -0.000621865, -0.000619539, -0.000617222, -0.000614913, -0.000612613, -0.000610322, -0.000608039, -0.000605765, -0.0006035, -0.000601242,
                        -0.000598993, -0.000596753, -0.000594521, -0.000592297, -0.000590082, -0.000587875, -0.000585676, -0.000583486, -0.000581303, -0.000579129,
                        -0.000576963, -0.000574805, -0.000572655, -0.000570513, -0.000568379, -0.000566254, -0.000564136, -0.000562026, -0.000559924, -0.000557829,
                        -0.000555743, -0.000553664, -0.000551593, -0.00054953, -0.000547475, -0.000545427, -0.000543387, -0.000541355, -0.00053933, -0.000537313,
                        -0.000535303, -0.000533301, -0.000531306, -0.000529319, -0.000527339, -0.000525367, -0.000523402, -0.000521444, -0.000519494, -0.000517551,
                        -0.000515615, -0.000513687, -0.000511765, -0.000509851, -0.000507944, -0.000506044, -0.000504152, -0.000502266, -0.000500387, -0.000498516,
                        -0.000496651, -0.000494793, -0.000492943, -0.000491099, -0.000489262, -0.000487432, -0.000485609, -0.000483793, -0.000481983, -0.000480181,
                        -0.000478385, -0.000476595, -0.000474813, -0.000473037, -0.000471268, -0.000469505, -0.000467749, -0.000465999, -0.000464256, -0.00046252,
                        -0.00046079, -0.000459066, -0.000457349, -0.000455639, -0.000453935, -0.000452237, -0.000450545, -0.00044886, -0.000447181, -0.000445509,
                        -0.000443842, -0.000442182, -0.000440528, -0.000438881, -0.000437239, -0.000435604, -0.000433975, -0.000432351, -0.000430734, -0.000429123,
                        -0.000427518, -0.000425919, -0.000424326, -0.000422739, -0.000421158, -0.000419583, -0.000418013, -0.00041645, -0.000414892, -0.00041334
                       };



  float NoiseM[31][31];
  const ECLNoiseData* eclNoise = eclNoiseData[0];

  eclNoise->getMatrix(NoiseM);


  //Should ShaperDSP() be used instead of par_shape[] ?
//double t=0;
//double dt=.02;
//int n=1250;
  for (int i = 0; i < 1250 ; i++) {
    m_ft[i] = par_shape[i];
    //  if(abs (par_shape[i] -ShaperDSP(t) )>1e-8 )
    //  cout<<i<<" "<<t<<" " << m_ft[i]<<" "<<ShaperDSP(t) <<endl;
    //  t=t+dt;

  }

  for (int i = 0; i < 31 ; i++) {
    for (int j = 0; j < 31 ; j++) {
      m_vmat[i][j] = NoiseM[i][j];
    }
  }


}




