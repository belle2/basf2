#include "Photos.h"
#include "pairs.h"

#include <cmath>
#include <stdio.h>
#include <stdlib.h>
using namespace Photospp;

namespace Photospp {


//
  inline double xlam(double A, double B, double C) {return sqrt((A - B - C) * (A - B - C) - 4.0 * B * C);}

  inline double max(double a, double b)
  {
    return (a > b) ? a : b;
  }
  //
  //extern "C" void varran_( double RRR[], int *N);
  double angfi(double X, double Y)
  {
    double THE;
    //const double PI=3.141592653589793238462643;

    if (X == 0.0 && Y == 0.0) return 0.0;
    if (fabs(Y) < fabs(X)) {
      THE = atan(fabs(Y / X));
      if (X <= 0.0) THE = PI - THE;
    } else {
      THE = acos(X / sqrt(X * X + Y * Y));
    }
    if (Y < 0.0) THE = 2 * PI - THE;
    return THE;
  }

  double angxy(double X, double Y)
  {
    double THE;
    //const double PI=3.141592653589793238462643;

    if (X == 0.0 && Y == 0.0) return 0.0;

    if (fabs(Y) < fabs(X)) {
      THE = atan(fabs(Y / X));
      if (X <= 0.0) THE = PI - THE;
    } else {
      THE = acos(X / sqrt(X * X + Y * Y));
    }
    return THE;
  }

  void  bostd3(double EXE, double PVEC[4], double QVEC[4])
  {
    // ----------------------------------------------------------------------
    // BOOST ALONG Z AXIS, EXE=EXP(ETA), ETA= HIPERBOLIC VELOCITY.
    //
    //     USED BY : KORALZ RADKOR
    /// ----------------------------------------------------------------------
    int j = 1; // convention of indices of Riemann space must be preserved.
    double RPL, RMI, QPL, QMI;
    double RVEC[4];


    RVEC[1 - j] = PVEC[1 - j];
    RVEC[2 - j] = PVEC[2 - j];
    RVEC[3 - j] = PVEC[3 - j];
    RVEC[4 - j] = PVEC[4 - j];
    RPL = RVEC[4 - j] + RVEC[3 - j];
    RMI = RVEC[4 - j] - RVEC[3 - j];
    QPL = RPL * EXE;
    QMI = RMI / EXE;
    QVEC[1 - j] = RVEC[1 - j];
    QVEC[2 - j] = RVEC[2 - j];
    QVEC[3 - j] = (QPL - QMI) / 2;
    QVEC[4 - j] = (QPL + QMI) / 2;
  }

// after investigations PHORO3 of PhotosUtilities.cxx will be used instead
// but it must be checked first if it works

  void rotod3(double ANGLE, double PVEC[4], double QVEC[4])
  {


    int j = 1; // convention of indices of Riemann space must be preserved.
    double CS, SN;
    //  printf ("%5.2f\n",cos(ANGLE));
    CS = cos(ANGLE) * PVEC[1 - j] - sin(ANGLE) * PVEC[2 - j];
    SN = sin(ANGLE) * PVEC[1 - j] + cos(ANGLE) * PVEC[2 - j];

    QVEC[1 - j] = CS;
    QVEC[2 - j] = SN;
    QVEC[3 - j] = PVEC[3 - j];
    QVEC[4 - j] = PVEC[4 - j];
  }



  void   rotod2(double PHI, double PVEC[4], double QVEC[4])
  {

    double RVEC[4];
    int j = 1; // convention of indices of Riemann space must be preserved.
    double CS, SN;

    CS = cos(PHI);
    SN = sin(PHI);

    RVEC[1 - j] = PVEC[1 - j];
    RVEC[2 - j] = PVEC[2 - j];
    RVEC[3 - j] = PVEC[3 - j];
    RVEC[4 - j] = PVEC[4 - j];

    QVEC[1 - j] = CS * RVEC[1 - j] + SN * RVEC[3 - j];
    QVEC[2 - j] = RVEC[2 - j];
    QVEC[3 - j] = -SN * RVEC[1 - j] + CS * RVEC[3 - j];
    QVEC[4 - j] = RVEC[4 - j];
    //   printf ("%15.12f %15.12f %15.12f %15.12f \n",QVEC[0],QVEC[1],QVEC[2],QVEC[3]);
    // exit(-1);
  }

  void   lortra(int KEY, double PRM, double PNEUTR[4], double PNU[4], double PAA[4], double PP[4], double PE[4])
  {
    // ---------------------------------------------------------------------
    // THIS ROUTINE PERFORMS LORENTZ TRANSFORMATION ON MANY 4-VECTORS
    // KEY   =1    BOOST    ALONG   3RD AXIS
    //       =2    ROTATION AROUND 2ND AXIS
    //       =3    ROTATION AROUND 3RD AXIS
    // PRM         TRANSFORMATION PARAMETER - ANGLE OR EXP(HIPERANGLE).
    //
    //    called by : RADCOR
    // ---------------------------------------------------------------------
    if (KEY == 1) {
      bostd3(PRM, PNEUTR, PNEUTR);
      bostd3(PRM, PNU , PNU);
      bostd3(PRM, PAA , PAA);
      bostd3(PRM, PE , PE);
      bostd3(PRM, PP , PP);
    } else if (KEY == 2) {
      rotod2(PRM, PNEUTR, PNEUTR);
      rotod2(PRM, PNU , PNU);
      rotod2(PRM, PAA , PAA);
      rotod2(PRM, PE  , PE);
      rotod2(PRM, PP  , PP);
    } else if (KEY == 3) {
      rotod3(PRM, PNEUTR, PNEUTR);
      rotod3(PRM, PNU , PNU);
      rotod3(PRM, PAA , PAA);
      rotod3(PRM, PE  , PE);
      rotod3(PRM, PP  , PP);
    } else {
      printf(" STOP IN LOTRA. WRONG KEYTRA");
      exit(-1);
    }
  }

  double amast(double VEC[4])
  {
    int i = 1; // convention of indices of Riemann space must be preserved
    double ama = VEC[4 - i] * VEC[4 - i] - VEC[1 - i] * VEC[1 - i] - VEC[2 - i] * VEC[2 - i] - VEC[3 - i] * VEC[3 - i];
    ama = sqrt(fabs(ama));
    return ama;
  }

  void spaj(int KUDA, double P2[4], double Q2[4], double PP[4], double PE[4])
  {
    //     **********************
    // THIS PRINTS OUT FOUR MOMENTA OF PHOTONS
    // ON OUTPUT UNIT NOUT

    double SUM[4];
    const int KLUCZ = 0;
    if (KLUCZ == 0) return;

    printf(" %10i =====================SPAJ==================== \n", KUDA);
    printf(" P2 %18.13f %18.13f %18.13f %18.13f \n", P2[0], P2[1], P2[2], P2[3]);
    printf(" Q2 %18.13f %18.13f %18.13f %18.13f \n", Q2[0], Q2[1], Q2[2], Q2[3]);
    printf(" PE %18.13f %18.13f %18.13f %18.13f \n", PE[0], PE[1], PE[2], PE[3]);
    printf(" PP %18.13f %18.13f %18.13f %18.13f \n", PP[0], PP[1], PP[2], PP[3]);

    for (int k = 0; k <= 3; k++) SUM[k] = P2[k] + Q2[k] + PE[k] + PP[k];

    printf("SUM %18.13f %18.13f %18.13f %18.13f \n", SUM[0], SUM[1], SUM[2], SUM[3]);
  }

//extern "C" {
  struct PARKIN {
    double fi0; // FI0
    double fi1; // FI1
    double fi2; // FI2
    double fi3; // FI3
    double fi4; // FI4
    double fi5; // FI5
    double th0; // TH0
    double th1; // TH1
    double th3; // TH3
    double th4; // TH4
    double parneu; // PARNEU
    double parch; // PARCH
    double bpar; // BPAR
    double bsta; // BSTA
    double bstb; // BSTB
  } parkin;
//}

//struct PARKIN parkin;

  void partra(int IBRAN, double PHOT[4])
  {



    rotod3(-parkin.fi0, PHOT, PHOT);
    rotod2(-parkin.th0, PHOT, PHOT);
    bostd3(parkin.bsta, PHOT, PHOT);
    rotod3(-parkin.fi1, PHOT, PHOT);
    rotod2(-parkin.th1, PHOT, PHOT);
    rotod3(-parkin.fi2, PHOT, PHOT);

    if (IBRAN == -1) {
      bostd3(parkin.parneu, PHOT, PHOT);
    } else {
      bostd3(parkin.parch, PHOT, PHOT);
    }

    rotod3(-parkin.fi3, PHOT, PHOT);
    rotod2(-parkin.th3, PHOT, PHOT);
    bostd3(parkin.bpar, PHOT, PHOT);
    rotod3(parkin.fi4, PHOT, PHOT);
    rotod2(-parkin.th4, PHOT, PHOT);
    rotod3(-parkin.fi5, PHOT, PHOT);
    rotod3(parkin.fi2, PHOT, PHOT);
    rotod2(parkin.th1, PHOT, PHOT);
    rotod3(parkin.fi1, PHOT, PHOT);
    bostd3(parkin.bstb, PHOT, PHOT);
    rotod2(parkin.th0, PHOT, PHOT);
    rotod3(parkin.fi0, PHOT, PHOT);

  }


  void trypar(bool* JESLI, double* pSTRENG, double AMCH, double AMEL, double PA[4], double PB[4], double PE[4], double PP[4],
              bool* sameflav)
  {
    double& STRENG = *pSTRENG;
    //      COMMON  /PARKIN/
    double& FI0 = parkin.fi0;
    double& FI1 = parkin.fi1;
    double& FI2 = parkin.fi2;
    double& FI3 = parkin.fi3;
    double& FI4 = parkin.fi4;
    double& FI5 = parkin.fi5;
    double& TH0 = parkin.th0;
    double& TH1 = parkin.th1;
    double& TH3 = parkin.th3;
    double& TH4 = parkin.th4;
    double& PARNEU = parkin.parneu;
    double& PARCH = parkin.parch;
    double& BPAR = parkin.bpar;
    double& BSTA = parkin.bsta;
    double& BSTB = parkin.bstb;

    double  PNEUTR[4], PAA[4], PHOT[4], PSUM[4];
    double VEC[4];
    double RRR[8];
    bool JESLIK;
    //const double PI=3.141592653589793238462643;
    const double ALFINV = 137.01;
    const int j = 1; // convention of indices of Riemann space must be preserved.

    PA[4 - j] = max(PA[4 - j], sqrt(PA[1 - j] * PA[1 - j] + PA[2 - j] * PA[2 - j] + PA[3 - j] * PA[3 - j]));
    PB[4 - j] = max(PB[4 - j], sqrt(PB[1 - j] * PB[1 - j] + PB[2 - j] * PB[2 - j] + PB[3 - j] * PB[3 - j]));

    // 4-MOMENTUM OF THE NEUTRAL SYSTEM
    for (int k = 0; k <= 3; k++) {
      PE[k]    = 0.0;
      PP[k]    = 0.0;
      PSUM[k]  = PA[k] + PB[k];
      PAA[k]   = PA[k];
      PNEUTR[k] = PB[k];
    }
    if ((PAA[4 - j] + PNEUTR[4 - j]) < 0.01) {
      printf(" too small energy to emit %10.7f\n", PAA[4 - j] + PNEUTR[4 - j]);
      *JESLI = false;
      return;
    }

    // MASSES OF THE NEUTRAL AND CHARGED SYSTEMS AND OVERALL MASS
    // FIRST WE HAVE TO GO TO THE RESTFRAME TO GET RID OF INSTABILITIES
    // FROM BHLUMI OR ANYTHING ELSE
    // THIRD AXIS ALONG PNEUTR
    double X1 = PSUM[1 - j];
    double X2 = PSUM[2 - j];
    FI0  = angfi(X1, X2);
    X1 = PSUM[3 - j];
    X2 = sqrt(PSUM[1 - j] * PSUM[1 - j] + PSUM[2 - j] * PSUM[2 - j]);
    TH0  = angxy(X1, X2) ;
    spaj(-2, PNEUTR, PAA, PP, PE);
    lortra(3, -FI0, PNEUTR, VEC, PAA, PP, PE);
    lortra(2, -TH0, PNEUTR, VEC, PAA, PP, PE);
    rotod3(-FI0, PSUM, PSUM);
    rotod2(-TH0, PSUM, PSUM);
    BSTA = (PSUM[4 - j] - PSUM[3 - j]) / sqrt(PSUM[4 - j] * PSUM[4 - j] - PSUM[3 - j] * PSUM[3 - j]);
    BSTB = (PSUM[4 - j] + PSUM[3 - j]) / sqrt(PSUM[4 - j] * PSUM[4 - j] - PSUM[3 - j] * PSUM[3 - j]);
    lortra(1, BSTA, PNEUTR, VEC, PAA, PP, PE);
    spaj(-1, PNEUTR, PAA, PP, PE);
    double AMNE = amast(PNEUTR);
    AMCH = amast(PAA); // to be improved. May be dangerous because of rounding error
    if (AMCH < 0.0) AMCH = AMEL;
    if (AMNE < 0.0) AMNE = 0.0;
    double AMTO = PAA[4 - j] + PNEUTR[4 - j];


    for (int k = 0; k <= 7; k++) RRR[k] = Photos::randomDouble();

    if (STRENG == 0.0) {*JESLI = false;  return;}

    double PRHARD;
    PRHARD = STRENG // NOTE: logs from phase space presamplers not MEs
             * 0.5 * (1.0 / PI / ALFINV) * (1.0 / PI / ALFINV) // normalization of triple log  1/36 from
             // journals.aps.org/prd/pdf/10.1103/PhysRevD.49.1178
             // must come from rejection
             // 0.5 is because it is for 1-leg only
             // STRENG=0,5 because of calls before and after photons
             // other logs should come from rejection
             * 2 * log(AMTO / AMEL / 2.0)                  // virtuality
             *log(AMTO / AMEL / 2.0)                     // soft
             *log((AMTO * AMTO + 2 * AMCH * AMCH) / 2.0 / AMCH / AMCH); // collinear
    // ZBW-2021
    //  artificial ad hoc increase of probability for e+e-/mu+mu- pair appearance.
    // Should be calculated from MXX GXX etc. but now it remains a shadow of QED.
    if (darkr.ifspecial == 1) {
      if (AMEL < 0.001) PRHARD = PRHARD * darkr.NormFact;
      else           PRHARD = PRHARD * darkr.NormFmu; // for muons we need even more.
      // PRHARD= PRHARD*darkr.NormFact;
      //if(AMEL>0.001)  PRHARD=PRHARD*darkr.NormFmu;
    }
// printf("  PRHARD/amel= %18.13f   %18.13f \n",PRHARD, AMEL);
    // ZBW-2021 end

    double FREJECT = 2.; // to make room for interference second pair posiblty.
    PRHARD = PRHARD * FREJECT;
    //   PRHARD=PRHARD*50; // to increase number of pairs in test of mu mu from mu
    //   fror mumuee set *15
    // enforces hard pairs to be generated 'always'
    // for the sake of tests with high statistics, also for flat phase space.
    //   PRHARD=0.99* STRENG*2;
    //   STRENG=0.0;
    if (PRHARD > 1.0) {
      printf(" stop from Photos pairs.cxx PRHARD= %18.13f \n", PRHARD);
      exit(0);
    }
// delta is for tests with PhysRevD.49.1178, default is AMTO*2 no restriction on pair phase space
    double delta = AMTO * 2; //5;//.125; //AMTO*2; //.125; //AMTO*2; ;0.25;


    if (RRR[7 - j] > PRHARD) {    // compensate crude probablilities; for pairs from consecutive sources
      STRENG = STRENG / (1.0 - PRHARD);
      *JESLI = false;
      return;
    } else STRENG = 0.0;





    //

    //virtuality of lepton pair
    // ZBW-2021
    // mass and width of intermediate resoinance.
    double XMP, ALP1, ALP2, ALP;
    if (darkr.ifspecial == 1) {
      ALP1 = atan((4 * AMEL * AMEL - darkr.MXX * darkr.MXX) / darkr.MXX / darkr.GXX);
      ALP2 = atan((AMTO * AMTO - darkr.MXX) / darkr.MXX / darkr.GXX);
      ALP = ALP1 + RRR[1 - j] * (ALP2 - ALP1);
      XMP = sqrt(darkr.MXX * darkr.MXX + darkr.MXX * darkr.GXX * tan(ALP));
      // ZBW-2021 end
    } else {
      XMP = 2.0 * AMEL * exp(RRR[1 - j] * log(AMTO / 2.0 / AMEL));
      // XMP=2.0*AMEL*2.0*AMEL+RRR[1-j]*(AMTO-2.0*AMEL)*(AMTO-2.0*AMEL); XMP=sqrt(XMP); // option of no presampler
    }

    // energy of lepton pair replace   virtuality of CHAR+NEU system in phase space parametrization
    double XPmin = 2.0 * AMEL;
    if (darkr.ifspecial == 1) {
      XPmin = darkr.MXX - 5 * darkr.GXX;
      if (XPmin < 2.0 * AMEL) XPmin = 2.0 * AMEL;
      // ZBW-2021 end
    }

    double XPdelta = AMTO - XPmin;
    double XP =  XPmin * exp(RRR[2 - j] * log((XPdelta + XPmin) / XPmin));
    //     XP=  XPmin +RRR[2-j]*XPdelta;                                  // option of no presampler
    double XMK2 = (AMTO * AMTO + XMP * XMP) - 2.0 * AMTO * XP;

    // angles of lepton pair
    double eps = 4.0 * AMCH * AMCH / AMTO / AMTO;
    if (darkr.ifspecial == 1) {
      eps = (darkr.MXX - 5 * darkr.GXX) * (darkr.MXX - 5 * darkr.GXX) / AMTO / AMTO;
      if (eps < 4.0 * AMCH * AMCH / AMTO / AMTO) eps = 4.0 * AMCH * AMCH / AMTO / AMTO;
      // ZBW-2021 end
    }
    double C1 = 1.0 + eps - eps * exp(RRR[3 - j] * log((2 + eps) / eps));
    //   C1=1.0-2.0*RRR[3-j];                                       // option of no presampler
    double FIX1 = 2.0 * PI * RRR[4 - j];

    // angles of lepton in restframe of lepton pair
    double C2  = 1.0 - 2.0 * RRR[5 - j];
    double FIX2 = 2.0 * PI * RRR[6 - j];



    // histograming .......................
    JESLIK = (XP < ((AMTO * AMTO + XMP * XMP - (AMCH + AMNE) * (AMCH + AMNE)) / 2.0 / AMTO));
    double WTA = 0.0;
    WTA = WTA * 5;
    if (JESLIK) WTA = 1.0;
    //      GMONIT( 0,101   ,WTA,1D0,0D0)
    JESLIK = (XMP < (AMTO - AMNE - AMCH));
    WTA = 0.0;
    if (JESLIK) WTA = 1.0;
    //      GMONIT( 0,102   ,WTA,1D0,0D0)
    JESLIK = (XMP < (AMTO - AMNE - AMCH)) && (XP > XMP);

    WTA = 0.0;
    if (JESLIK) WTA = 1.0;
    //      GMONIT( 0,103   ,WTA,1D0,0D0)
    JESLIK =
      (XMP < (AMTO - AMNE - AMCH)) &&
      (XP > XMP)             &&
      (XP < ((AMTO * AMTO + XMP * XMP - (AMCH + AMNE) * (AMCH + AMNE)) / 2.0 / AMTO));
    WTA = 0.0;
    if (JESLIK) WTA = 1.0;
    //      GMONIT( 0,104   ,WTA,1D0,0D0)
    // end of histograming ................

    // phase space limits rejection variable
    *JESLI = (RRR[7 - j] < PRHARD)       &&
             (XMP < (AMTO - AMNE - AMCH))  &&
             (XP > XMP)               &&
             (XP < ((AMTO * AMTO + XMP * XMP - (AMCH + AMNE) * (AMCH + AMNE)) / 2.0 / AMTO));


// rejection for phase space restriction:  for tests with PhysRevD.49.1178
    *JESLI = *JESLI && XP < delta;
    if (!*JESLI) return;

    // for events in phase: jacobians weights etc.

    // virtuality of added lepton pair
    double F = (AMTO * AMTO - 4.0 * AMEL * AMEL)   // flat phase space
               / (AMTO * AMTO - 4.0 * AMEL * AMEL)  * XMP * XMP; // use this when presampler is on  (log moved to PRHARD)
    // ZBW-2021
    if (darkr.ifspecial == 1) F = (ALP2 - ALP1) * ((XMP * XMP - darkr.MXX * darkr.MXX) * (XMP * XMP - darkr.MXX * darkr.MXX) +
                                                     (darkr.MXX * darkr.GXX) * (darkr.MXX * darkr.GXX)) / (darkr.MXX * darkr.GXX);
    // ZBW-2021 end
    // Energy of added lepton pair represented  by  virtuality of CH+N pair
    double G = 2 * AMTO * XPdelta                 // flat phase space
               / (2 * AMTO * XPdelta)   * 2 * AMTO * XP; // use this  when presampler is on  (log moved to PRHARD)


    // scattering angle of emitted lepton pair (also flat factors for other angles)
    double H =   2.0               // flat phase space
                 / 2.0 * (1.0 + eps - C1) / 2.0; // use this when presampler is on  (log moved to PRHARD)

    double H1 = 2.0                 // for other generation arm:   char neutr replaced
                / 2.0  * (1.0 + eps - C1);
    double H2 = 2.0
                / 2.0  * (1.0 + eps + C1);
    H = 1. / (0.5 / H1 + 0.5 / H2);

    //*2*PI*4*PI  /2/PI/4/PI;      // other angles normalization of transformation to random numbers.

    double XPMAX = (AMTO * AMTO + XMP * XMP - (AMCH + AMNE) * (AMCH + AMNE)) / 2.0 / AMTO;

    double YOT3 = F * G * H; // jacobians for phase space variables
    double YOT2 =      // lambda factors:
      xlam(1.0, AMEL * AMEL / XMP / XMP, AMEL * AMEL / XMP / XMP) *
      xlam(1.0, XMK2 / AMTO / AMTO, XMP * XMP / AMTO / AMTO) *
      xlam(1.0, AMCH * AMCH / XMK2, AMNE * AMNE / XMK2)
      / xlam(1.0, AMCH * AMCH / AMTO / AMTO, AMNE * AMNE / AMTO / AMTO);
    // if(darkr.ifspecial==1) YOT2=YOT2/xlam(1.0,XMK2/AMTO/AMTO,XMP*XMP/AMTO/AMTO);

    //C histograming .......................
    //      GMONIT( 0,105   ,WT  ,1D0,0D0)
    //      GMONIT( 0,106   ,YOT1,1D0,0D0)
    //      GMONIT( 0,107   ,YOT2,1D0,0D0)
    //      GMONIT( 0,108   ,YOT3,1D0,0D0)
    //      GMONIT( 0,109   ,YOT4,1D0,0D0)
    // end of histograming ................


    //
    //
    // FRAGMENTATION COMES !!
    //
    // THIRD AXIS ALONG PNEUTR
    X1 = PNEUTR[1 - j];
    X2 = PNEUTR[2 - j];
    FI1  = angfi(X1, X2);
    X1 = PNEUTR[3 - j];
    X2 = sqrt(PNEUTR[1 - j] * PNEUTR[1 - j] + PNEUTR[2 - j] * PNEUTR[2 - j]) ;
    TH1  = angxy(X1, X2);
    spaj(0, PNEUTR, PAA, PP, PE);
    lortra(3, -FI1, PNEUTR, VEC, PAA, PP, PE);
    lortra(2, -TH1, PNEUTR, VEC, PAA, PP, PE);
    VEC[4 - j] = 0.0;
    VEC[3 - j] = 0.0;
    VEC[2 - j] = 0.0;
    VEC[1 - j] = 1.0;
    FI2 = angfi(VEC[1 - j], VEC[2 - j]);
    lortra(3, -FI2, PNEUTR, VEC, PAA, PP, PE);
    spaj(1, PNEUTR, PAA, PP, PE);

    // STEALING FROM PAA AND PNEUTR ENERGY FOR THE pair
    // ====================================================
    // NEW MOMENTUM OF PAA AND PNEUTR (IN THEIR VERY REST FRAME)
    // 1) PARAMETERS.....
    double AMCH2 = AMCH * AMCH;
    double AMNE2 = AMNE * AMNE;
    double AMTOST = XMK2;
    double QNEW = xlam(AMTOST, AMNE2, AMCH2) / sqrt(AMTOST) / 2.0;
    double QOLD = PNEUTR[3 - j];
    double GCHAR = (QNEW * QNEW + QOLD * QOLD + AMCH * AMCH) /
                   (QNEW * QOLD + sqrt((QNEW * QNEW + AMCH * AMCH) * (QOLD * QOLD + AMCH * AMCH)));
    double GNEU = (QNEW * QNEW + QOLD * QOLD + AMNE * AMNE) /
                  (QNEW * QOLD + sqrt((QNEW * QNEW + AMNE * AMNE) * (QOLD * QOLD + AMNE * AMNE)));

    //      GCHAR=(QOLD**2-QNEW**2)/(
    //     &       QNEW*SQRT(QOLD**2+AMCH2)+QOLD*SQRT(QNEW**2+AMCH2)
    //     &                        )
    //      GCHAR=SQRT(1D0+GCHAR**2)
    //      GNEU=(QOLD**2-QNEW**2)/(
    //     &       QNEW*SQRT(QOLD**2+AMNE2)+QOLD*SQRT(QNEW**2+AMNE2)
    //     &                        )
    //      GNEU=SQRT(1D0+GNEU**2)
    if (GNEU < 1. || GCHAR < 1.) {
      printf(" PHOTOS TRYPAR GBOOST LT 1., LIMIT OF PHASE SPACE %18.13f %18.13f %18.13f %18.13f %18.13f %18.13f %18.13f %18.13f \n"
             , GNEU, GCHAR, QNEW, QOLD, AMTO, AMTOST, AMNE, AMCH);
      return;
    }
    PARCH = GCHAR + sqrt(GCHAR * GCHAR - 1.0);
    PARNEU = GNEU - sqrt(GNEU * GNEU - 1.0);

    // 2) REDUCTIEV BOOSTS
    bostd3(PARNEU, VEC , VEC);
    bostd3(PARNEU, PNEUTR, PNEUTR);
    bostd3(PARCH, PAA , PAA);
    spaj(2, PNEUTR, PAA, PP, PE);

    // TIME FOR THE PHOTON that is electron pair
    double PMOD = xlam(XMP * XMP, AMEL * AMEL, AMEL * AMEL) / XMP / 2.0;
    double S2 = sqrt(1.0 - C2 * C2);
    PP[4 - j] = XMP / 2.0;
    PP[3 - j] = PMOD * C2;
    PP[2 - j] = PMOD * S2 * cos(FIX2);
    PP[1 - j] = PMOD * S2 * sin(FIX2);
    PE[4 - j] = PP[4 - j];
    PE[3 - j] = -PP[3 - j];
    PE[2 - j] = -PP[2 - j];
    PE[1 - j] = -PP[1 - j];
    // PHOTON ENERGY and momentum IN THE REDUCED SYSTEM
    double PENE = (AMTO * AMTO - XMP * XMP - XMK2) / 2.0 / sqrt(XMK2);
    double PPED = sqrt(PENE * PENE - XMP * XMP);
    FI3 = FIX1;
    double COSTHG = C1;
    double SINTHG = sqrt(1.0 - C1 * C1);
    X1 = -COSTHG;
    X2 =  SINTHG;
    TH3  = angxy(X1, X2);
    PHOT[1 - j] = PMOD * SINTHG * cos(FI3);
    PHOT[2 - j] = PMOD * SINTHG * sin(FI3);
    // MINUS BECAUSE AXIS OPPOSITE TO PAA
    PHOT[3 - j] = -PMOD * COSTHG;
    PHOT[4 - j] = PENE;
    // ROTATE TO PUT PHOTON ALONG THIRD AXIS
    X1 = PHOT[1 - j];
    X2 = PHOT[2 - j];
    lortra(3, -FI3, PNEUTR, VEC, PAA, PP, PE);
    rotod3(-FI3, PHOT, PHOT);
    lortra(2, -TH3, PNEUTR, VEC, PAA, PP, PE);
    rotod2(-TH3, PHOT, PHOT);
    spaj(21, PNEUTR, PAA, PP, PE);
    // ... now get the pair !
    double PAIRB = PENE / XMP + PPED / XMP;
    bostd3(PAIRB, PE, PE);
    bostd3(PAIRB, PP, PP);
    spaj(3, PNEUTR, PAA, PP, PE);
    double GAMM = (PNEUTR[4 - j] + PAA[4 - j] + PP[4 - j] + PE[4 - j]) / AMTO;

    // TP and ZW: 25.II.2015: fix for cases when mother is very close
    //            to its rest frame and pair is generated after photon emission.
    //            Then GAMM can be slightly less than 1.0 due to rounding error
    if (GAMM < 1.0) {
      if (GAMM > 0.9999999) GAMM = 1.0;
      else {
        printf("Photos::partra: GAMM = %20.18e\n", GAMM);
        printf("                BELOW  0.9999999 THRESHOLD!\n");
        GAMM = 1.0;
      }
    }

    BPAR = GAMM - sqrt(GAMM * GAMM - 1.0);
    lortra(1, BPAR, PNEUTR, VEC, PAA, PP, PE);
    bostd3(BPAR, PHOT, PHOT);
    spaj(4, PNEUTR, PAA, PP, PE);
    // BACK IN THE TAU REST FRAME BUT PNEUTR NOT YET ORIENTED.
    X1 = PNEUTR[1 - j];
    X2 = PNEUTR[2 - j];
    FI4  = angfi(X1, X2);
    X1 = PNEUTR[3 - j];
    X2 = sqrt(PNEUTR[1 - j] * PNEUTR[1 - j] + PNEUTR[2 - j] * PNEUTR[2 - j]);
    TH4  = angxy(X1, X2);
    lortra(3, FI4, PNEUTR, VEC, PAA, PP, PE);
    rotod3(FI4, PHOT, PHOT);
    lortra(2, -TH4, PNEUTR, VEC, PAA, PP, PE);
    rotod2(-TH4, PHOT, PHOT);
    X1 = VEC[1 - j];
    X2 = VEC[2 - j];
    FI5 = angfi(X1, X2);
    lortra(3, -FI5, PNEUTR, VEC, PAA, PP, PE);
    rotod3(-FI5, PHOT, PHOT);
    // PAA RESTORES ORIGINAL DIRECTION
    lortra(3, FI2, PNEUTR, VEC, PAA, PP, PE);
    rotod3(FI2, PHOT, PHOT);
    lortra(2, TH1, PNEUTR, VEC, PAA, PP, PE);
    rotod2(TH1, PHOT, PHOT);
    lortra(3, FI1, PNEUTR, VEC, PAA, PP, PE);
    rotod3(FI1, PHOT, PHOT);
    spaj(10, PNEUTR, PAA, PP, PE);
    lortra(1, BSTB, PNEUTR, VEC, PAA, PP, PE);
    lortra(2, TH0, PNEUTR, VEC, PAA, PP, PE);
    lortra(3, FI0, PNEUTR, VEC, PAA, PP, PE);
    spaj(11, PNEUTR, PAA, PP, PE);


// matrix element as formula 1 from journals.aps.org/prd/pdf/10.1103/PhysRevD.49.1178

    double pq =      PAA[3] * PP[3] - PAA[2] * PP[2] - PAA[1] * PP[1] - PAA[0] * PP[0];
    pq = pq   + PAA[3] * PE[3] - PAA[2] * PE[2] - PAA[1] * PE[1] - PAA[0] * PE[0];

    double ppq =     PNEUTR[3] * PP[3] - PNEUTR[2] * PP[2] - PNEUTR[1] * PP[1] - PNEUTR[0] * PP[0];
    ppq = ppq + PNEUTR[3] * PE[3] - PNEUTR[2] * PE[2] - PNEUTR[1] * PE[1] - PNEUTR[0] * PE[0];
    double pq1 = PAA[3] * PP[3] - PAA[2] * PP[2] - PAA[1] * PP[1] - PAA[0] * PP[0];
    double pq2 = PAA[3] * PE[3] - PAA[2] * PE[2] - PAA[1] * PE[1] - PAA[0] * PE[0];

    double ppq1 = PNEUTR[3] * PP[3] - PNEUTR[2] * PP[2] - PNEUTR[1] * PP[1] - PNEUTR[0] * PP[0];
    double ppq2 = PNEUTR[3] * PE[3] - PNEUTR[2] * PE[2] - PNEUTR[1] * PE[1] - PNEUTR[0] * PE[0];

    double ppp = PNEUTR[3] * PAA[3] - PNEUTR[2] * PAA[2] - PNEUTR[1] * PAA[1] - PNEUTR[0] * PAA[0];
    double mneutr2 = PNEUTR[3] * PNEUTR[3] - PNEUTR[2] * PNEUTR[2] - PNEUTR[1] * PNEUTR[1] - PNEUTR[0] * PNEUTR[0];
    double maa2 = PAA[3] * PAA[3] - PAA[2] * PAA[2] - PAA[1] * PAA[1] - PAA[0] * PAA[0];

    double YOT1 = 1. / 2. / XMP / XMP / XMP / XMP *
                  (4 * (pq1 / pq - ppq1 / ppq) * (pq2 / pq - ppq2 / ppq)
                   - XMP * XMP * (AMCH2 / pq / pq + AMNE2 / ppq / ppq - ppp / pq / ppq - ppp / pq / ppq));
    //         ZBW-2021

    if (darkr.ifspecial == 1 && darkr.iboson == 1) {
      YOT1 = YOT1 * XMP * XMP * XMP * XMP / ((darkr.MXX * darkr.MXX - XMP * XMP) * (darkr.MXX * darkr.MXX - XMP * XMP) + darkr.GXX *
                                             darkr.GXX * darkr.MXX * darkr.MXX);
      YOT1 = YOT1 * darkr.GXX * darkr.MXX ; // factor of total rate should be elsewhere
    } else if (darkr.ifspecial == 1) {
      double mcr = 0.5 * darkr.MXX * darkr.MXX;
      mcr = 0.5 * XMP * XMP;
      YOT1 = 1. / 2. / XMP / XMP / XMP / XMP *
             (4 * (1. / (pq + mcr) - 1. / (ppq + mcr)) * (1. / (pq + mcr) - 1. / (ppq + mcr)) * (XMP * XMP + 0 * AMCH * AMCH)
              - 4 * XMP * XMP / AMTO / AMTO * (AMCH2 / pq / pq + AMNE2 / ppq / ppq - ppp / pq / ppq - ppp / pq / ppq));

      YOT1 = YOT1 * XMP * XMP * XMP * XMP / ((darkr.MXX * darkr.MXX - XMP * XMP) * (darkr.MXX * darkr.MXX - XMP * XMP) + darkr.GXX *
                                             darkr.GXX * darkr.MXX * darkr.MXX);
      YOT1 = YOT1 * darkr.GXX * darkr.MXX ; // factor of total rate should be elsewhere
      YOT1 = YOT1 * (AMTO * AMTO - 4 * AMCH * AMCH) / (AMTO * AMTO);
      YOT1 = YOT1 * XMK2 / (XMK2 - 4 * AMCH * AMCH); //* XMK2/(AMTO*AMTO);
      YOT1 = YOT1 * AMTO / sqrt(XMK2);
      YOT1 = YOT1 * AMTO / sqrt(XMK2);
      YOT1 = YOT1 * sqrt((AMTO * AMTO - XMK2 + 2 * AMCH * AMCH) / (AMTO * AMTO - XMK2)); //sep 5 13:10

    }
//         ZBW-2021 end


    if (*sameflav) {
// we interchange: PAA <--> pp
      for (int k = 0; k <= 3; k++) {
        double stored = PAA[k];
        PAA[k] = PE[k];
        PE[k] = stored;
      }

      pq =      PAA[3] * PP[3] - PAA[2] * PP[2] - PAA[1] * PP[1] - PAA[0] * PP[0];
      pq = pq   + PAA[3] * PE[3] - PAA[2] * PE[2] - PAA[1] * PE[1] - PAA[0] * PE[0];

      ppq =     PNEUTR[3] * PP[3] - PNEUTR[2] * PP[2] - PNEUTR[1] * PP[1] - PNEUTR[0] * PP[0];
      ppq = ppq + PNEUTR[3] * PE[3] - PNEUTR[2] * PE[2] - PNEUTR[1] * PE[1] - PNEUTR[0] * PE[0];
      pq1 = PAA[3] * PP[3] - PAA[2] * PP[2] - PAA[1] * PP[1] - PAA[0] * PP[0];
      pq2 = PAA[3] * PE[3] - PAA[2] * PE[2] - PAA[1] * PE[1] - PAA[0] * PE[0];

      ppq1 = PNEUTR[3] * PP[3] - PNEUTR[2] * PP[2] - PNEUTR[1] * PP[1] - PNEUTR[0] * PP[0];
      ppq2 = PNEUTR[3] * PE[3] - PNEUTR[2] * PE[2] - PNEUTR[1] * PE[1] - PNEUTR[0] * PE[0];

      ppp = PNEUTR[3] * PAA[3] - PNEUTR[2] * PAA[2] - PNEUTR[1] * PAA[1] - PNEUTR[0] * PAA[0];

      XMP = -(PP[0] + PE[0]) * (PP[0] + PE[0]) - (PP[1] + PE[1]) * (PP[1] + PE[1])
            - (PP[2] + PE[2]) * (PP[2] + PE[2]) + (PP[3] + PE[3]) * (PP[3] + PE[3]);
      XMP = sqrt(fabs(XMP));


      double YOT1p = 1. / 2. / XMP / XMP / XMP / XMP *
                     (4 * (pq1 / pq - ppq1 / ppq) * (pq2 / pq - ppq2 / ppq)
                      - XMP * XMP * (AMCH2 / pq / pq + AMNE2 / ppq / ppq - ppp / pq / ppq - ppp / pq / ppq));
      //   *(1-XP/XPMAX+0.5*(XP/XPMAX)*(XP/XPMAX));  // A-P kernel divide by (1-XP/XPMAX)?
      double wtint = 0.; // not yet installed
      wtint = 1; //(YOT1+YOT1p+wtint)/(YOT1+YOT1p);
//         ZBW-2021
//if(darkr.ifspecial==1) YOT1=YOT1*XMP*XMP*XMP*XMP/((MXX*MXX-XMP*XMP)*(MXX*MXX-XMP*XMP)+GXX*GXX*MXX*MXX);
//         ZBW-2021 end
      YOT1 = YOT1 * wtint;

// we interchange: PAA <--> pp back into place
      for (int k = 0; k <= 3; k++) {
        double stored = PAA[k];
        PAA[k] = PE[k];
        PE[k] = stored;
      }
    } // end sameflav

    double WT = YOT1 * YOT2 * YOT3;

    WT = WT / 8 / FREJECT; //   origin must be understood
    if (darkr.ifspecial == 1 && darkr.ifforce == 1 && AMEL < 0.001) {
      darkr.Fel = max(darkr.Fel, WT);
      darkr.Fel = std::min(darkr.Fel, 1.0);
      WT = WT / darkr.Fel;
    }
    if (darkr.ifspecial == 1 && darkr.ifforce == 1 && AMEL > 0.001) {
      darkr.Fmu = max(darkr.Fmu, WT);
      darkr.Fmu = std::min(darkr.Fmu, 1.0);
      WT = WT / darkr.Fmu;
    }
    //    printf (" from Photos pairs.cxx Fel/Fmu= %15.8f %15.8f  %5i   \n",darkr.Fel,darkr.Fmu,darkr.ifforce);
    //    if(WT>1.0){
    //      printf (" from Photos pairs.cxx WT= %15.8f  \n",WT);
    //      printf (" from Photos pairs.cxx WT= %15.8f  %15.8f %15.8f\n",YOT1,YOT2,YOT3);
    //      if(WT>20.0){
    //        printf ("XMP,MXX,GXX = %15.8f  %15.8f %15.8f\n",XMP,darkr.MXX,darkr.GXX);
    //        printf ("AMEL AMTO =   %15.8f %15.8f\n",AMEL,AMTO);
    //        printf ("ALP1 ALP2 ALP = %15.8f  %15.8f %15.8f\n",ALP1,ALP2,ALP);
    //        printf ("YOT1,YOT2,YOT3 = %15.8f  %15.8f %15.8f\n",YOT1,YOT2,YOT3);
    //      }
    //    }
    //  printf("  WT/amel/frej,yot1/yot2/yot3 = %15.10f %15.10f %15.10f %15.10f   %15.10f %15.10f  \n",WT, AMEL,FREJECT,YOT1,YOT2,YOT3);
    if (RRR[8 - j] > WT) {
      *JESLI = false;
      return;
    }

  }

} // namespace Photospp

