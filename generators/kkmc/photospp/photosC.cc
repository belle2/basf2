#include "Photos.h"
#include "forW-MEc.h"
#include "forZ-MEc.h"
#include "pairs.h"
#include "Log.h"
#include <cstdio>
#include <cmath>
#include <iostream>
#include "photosC.h"
#include "HEPEVT_struct.h"
#include "PhotosUtilities.h"
using std::cout;
using std::endl;
using std::max;
using namespace Photospp;
using namespace PhotosUtilities;

namespace Photospp {

// Instantiating structs declared in photosC.h

  struct HEPEVT hep;
  struct HEPEVT pho;

  struct PHOCOP  phocop;
  struct PHNUM   phnum;
  struct PHOKEY  phokey;
  struct PHOSTA  phosta;
  struct PHOLUN  pholun;
  struct PHOPHS  phophs;
  struct TOFROM  tofrom;
  struct PHOPRO  phopro;
  struct PHOREST phorest;
  struct PHWT    phwt;
  struct PHOCORWT  phocorwt;
  struct PHOMOM  phomom;
  struct PHOCMS  phocms;
  struct PHOEXP  phoexp;
  struct PHLUPY  phlupy;
  struct DARKR   darkr;
  /** Logical function used deep inside algorithm to check if emitted
      particles are to emit. For mother it blocks the vertex,
      but for daughters individually: bad sisters will not prevent electron to emit.
      top quark has further exception method. */
  bool F(int m, int i)
  {
    return Photos::IPHQRK_setQarknoEmission(0, i) && (i <= 41 || i > 100)
           && i != 21
           && i != 2101 && i != 3101 && i != 3201
           && i != 1103 && i != 2103 && i != 2203
           && i != 3103 && i != 3203 && i != 3303;
  }


// --- can be used with  VARIANT A. For B use  PHINT1 or 2 --------------
//----------------------------------------------------------------------
//
//    PHINT:   PHotos universal INTerference correction weight
//
//    Purpose:  calculates correction weight as expressed by
//               formula (17) from CPC 79 (1994), 291.
//
//    Input Parameters:  Common /PHOEVT/, with photon added.
//
//    Output Parameters: correction weight
//
//    Author(s):  Z. Was, P.Golonka               Created at:  19/01/05
//                                                Last Update: 23/06/13
//
//----------------------------------------------------------------------

  double PHINT(int IDUM)
  {

    double PHINT2;
    double EPS1[4], EPS2[4], PH[4], PL[4];
    static int i = 1;
    int K, L;
    //      DOUBLE PRECISION EMU,MCHREN,BETA,COSTHG,MPASQR,XPH, XC1, XC2
    double  XNUM1, XNUM2, XDENO, XC1, XC2;

    //      REAL*8 PHOCHA
    //--

    //       Calculate polarimetric vector: ph, eps1, eps2 are orthogonal

    for (K = 1; K <= 4; K++) {
      PH[K - i] = pho.phep[pho.nhep - i][K - i];
      EPS2[K - i] = 1.0;
    }


    PHOEPS(PH, EPS2, EPS1);
    PHOEPS(PH, EPS1, EPS2);


    XNUM1 = 0.0;
    XNUM2 = 0.0;
    XDENO = 0.0;

    for (K = pho.jdahep[1 - i][1 - i]; K <= pho.nhep - i; K++) { //! or jdahep[1-i][2-i]

      // momenta of charged particle in PL

      for (L = 1; L <= 4; L++) PL[L - i] = pho.phep[K - i][L - i];

      // scalar products: epsilon*p/k*p

      XC1 = - PHOCHA(pho.idhep[K - i]) *
            (PL[1 - i] * EPS1[1 - i] + PL[2 - i] * EPS1[2 - i] + PL[3 - i] * EPS1[3 - i]) /
            (PH[4 - i] * PL[4 - i]   - PH[1 - i] * PL[1 - i]   - PH[2 - i] * PL[2 - i] - PH[3 - i] * PL[3 - i]);

      XC2 = - PHOCHA(pho.idhep[K - i]) *
            (PL[1 - i] * EPS2[1 - i] + PL[2 - i] * EPS2[2 - i] + PL[3 - i] * EPS2[3 - i]) /
            (PH[4 - i] * PL[4 - i]   - PH[1 - i] * PL[1 - i]   - PH[2 - i] * PL[2 - i] - PH[3 - i] * PL[3 - i]);


      // accumulate the currents
      XNUM1  = XNUM1 + XC1;
      XNUM2  = XNUM2 + XC2;

      XDENO = XDENO + XC1 * XC1 + XC2 * XC2;
    }

    PHINT2 = (XNUM1 * XNUM1 + XNUM2 * XNUM2) / XDENO;
    return PHINT2;

  }



//----------------------------------------------------------------------
//
//    PHINT:   PHotos INTerference (Old version kept for tests only.
//
//    Purpose:  Calculates interference between emission of photons from
//              different possible chaged daughters stored in
//              the  HEP common /PHOEVT/.
//
//    Input Parameter:    commons /PHOEVT/ /PHOMOM/ /PHOPHS/
//
//
//    Output Parameters:
//
//
//    Author(s):  Z. Was,                         Created at:  10/08/93
//                                                Last Update: 15/03/99
//
//----------------------------------------------------------------------

  double PHINT1(int IDUM)
  {

    double PHINT;

    /*
        DOUBLE PRECISION MCHSQR,MNESQR
        REAL*8 PNEUTR
        COMMON/PHOMOM/MCHSQR,MNESQR,PNEUTR(5)
        DOUBLE PRECISION COSTHG,SINTHG
        REAL*8 XPHMAX,XPHOTO
        COMMON/PHOPHS/XPHMAX,XPHOTO,COSTHG,SINTHG

    */
    double MPASQR, XX, BETA;
    bool IFINT;
    int K, IDENT;
    double& COSTHG = phophs.costhg;
    double& XPHOTO = phophs.xphoto;
    //double *PNEUTR = phomom.pneutr; // unused variable
    double& MCHSQR = phomom.mchsqr;
    double& MNESQR = phomom.mnesqr;

    static int i = 1;
    IDENT = pho.nhep;
    //
    for (K = pho.jdahep[1 - i][2 - i]; K >= pho.jdahep[1 - i][1 - i]; K--) {
      if (pho.idhep[K - i] != 22) {
        IDENT = K;
        break;
      }
    }

    // check if there is a photon
    IFINT = pho.nhep > IDENT;
    // check if it is two body + gammas reaction
    IFINT = IFINT && (IDENT - pho.jdahep[1 - i][1 - i]) == 1;
    // check if two body was particle antiparticle
    IFINT = IFINT && pho.idhep[pho.jdahep[1 - i][1 - i] - i] == -pho.idhep[IDENT - i];
    // check if particles were charged
    IFINT = IFINT && PHOCHA(pho.idhep[IDENT - i]) != 0;
    // calculates interference weight contribution
    if (IFINT) {
      MPASQR = pho.phep[1 - i][5 - i] * pho.phep[1 - i][5 - i];
      XX = 4.0 * MCHSQR / MPASQR * (1.0 - XPHOTO) / (1.0 - XPHOTO + (MCHSQR - MNESQR) / MPASQR) / (1.0 - XPHOTO +
           (MCHSQR - MNESQR) / MPASQR);
      BETA = sqrt(1.0 - XX);
      PHINT  = 2.0 / (1.0 + COSTHG * COSTHG * BETA * BETA);
    } else {
      PHINT  = 1.0;
    }

    return  PHINT;
  }


//----------------------------------------------------------------------
//
//    PHINT:   PHotos INTerference
//
//    Purpose:  Calculates interference between emission of photons from
//              different possible chaged daughters stored in
//              the  HEP common /PHOEVT/.
//
//    Input Parameter:    commons /PHOEVT/ /PHOMOM/ /PHOPHS/
//
//
//    Output Parameters:
//
//
//    Author(s):  Z. Was,                         Created at:  10/08/93
//                                                Last Update:
//
//----------------------------------------------------------------------

  double PHINT2(int IDUM)
  {


    /*
        DOUBLE PRECISION MCHSQR,MNESQR
        REAL*8 PNEUTR
        COMMON/PHOMOM/MCHSQR,MNESQR,PNEUTR(5)
        DOUBLE PRECISION COSTHG,SINTHG
        REAL*8 XPHMAX,XPHOTO
        COMMON/PHOPHS/XPHMAX,XPHOTO,COSTHG,SINTHG
    */
    double& COSTHG = phophs.costhg;
    double& XPHOTO = phophs.xphoto;
    double& MCHSQR = phomom.mchsqr;
    double& MNESQR = phomom.mnesqr;


    double MPASQR, XX, BETA, pq1[4], pq2[4], pphot[4];
    double SS, PP2, PP, E1, E2, q1, q2, costhe, PHINT;
    bool IFINT;
    int K, k, IDENT;
    static int i = 1;
    IDENT = pho.nhep;
    //
    for (K = pho.jdahep[1 - i][2 - i]; K >= pho.jdahep[1 - i][1 - i]; K--) {
      if (pho.idhep[K - i] != 22) {
        IDENT = K;
        break;
      }
    }

    // check if there is a photon
    IFINT = pho.nhep > IDENT;
    // check if it is two body + gammas reaction
    IFINT = IFINT && (IDENT - pho.jdahep[1 - i][1 - i]) == 1;
    // check if two body was particle antiparticle (we improve on it !
    //      IFINT= IFINT.AND.pho.idhep(JDAPHO(1,1)).EQ.-pho.idhep(IDENT)
    // check if particles were charged
    IFINT = IFINT && fabs(PHOCHA(pho.idhep[IDENT - i])) > 0.01;
    // check if they have both charge
    IFINT = IFINT && fabs(PHOCHA(pho.idhep[pho.jdahep[1 - i][1 - i] - i])) > 0.01;
    // calculates interference weight contribution
    if (IFINT) {
      MPASQR = pho.phep[1 - i][5 - i] * pho.phep[1 - i][5 - i];
      XX = 4.0 * MCHSQR / MPASQR * (1.0 - XPHOTO) / pow(1. - XPHOTO + (MCHSQR - MNESQR) / MPASQR, 2);
      BETA = sqrt(1.0 - XX);
      PHINT  = 2.0 / (1.0 + COSTHG * COSTHG * BETA * BETA);
      SS = MPASQR * (1.0 - XPHOTO);
      PP2 = ((SS - MCHSQR - MNESQR) * (SS - MCHSQR - MNESQR) - 4 * MCHSQR * MNESQR) / SS / 4;
      PP = sqrt(PP2);
      E1 = sqrt(PP2 + MCHSQR);
      E2 = sqrt(PP2 + MNESQR);
      PHINT = (E1 + E2) * (E1 + E2) / ((E2 + COSTHG * PP) * (E2 + COSTHG * PP) + (E1 - COSTHG * PP) * (E1 - COSTHG * PP));
      // return PHINT;
      //
      q1 = PHOCHA(pho.idhep[pho.jdahep[1 - i][1 - i] - i]);
      q2 = PHOCHA(pho.idhep[IDENT - i]);
      for (k = 1; k <= 4; k++) {
        pq1[k - i] = pho.phep[pho.jdahep[1 - i][1 - i] - i][k - i];
        pq2[k - i] = pho.phep[pho.jdahep[1 - i][1 - i] + 1 - i][k - i];
        pphot[k - i] = pho.phep[pho.nhep - i][k - i];
      }
      costhe = (pphot[1 - i] * pq1[1 - i] + pphot[2 - i] * pq1[2 - i] + pphot[3 - i] * pq1[3 - i]);
      costhe = costhe / sqrt(pq1[1 - i] * pq1[1 - i] + pq1[2 - i] * pq1[2 - i] + pq1[3 - i] * pq1[3 - i]);
      costhe = costhe / sqrt(pphot[1 - i] * pphot[1 - i] + pphot[2 - i] * pphot[2 - i] + pphot[3 - i] * pphot[3 - i]);
      //
      // --- this IF checks whether JDAPHO(1,1) was MCH or MNE.
      // --- COSTHG angle (and in-generation variables) may be better choice
      // --- than costhe. note that in the formulae below amplitudes were
      // --- multiplied by (E2+COSTHG*PP)*(E1-COSTHG*PP).
      if (COSTHG * costhe > 0) {

        PHINT = pow(q1 * (E2 + COSTHG * PP) - q2 * (E1 - COSTHG * PP),
                    2) / (q1 * q1 * (E2 + COSTHG * PP) * (E2 + COSTHG * PP) + q2 * q2 * (E1 - COSTHG * PP) * (E1 - COSTHG * PP));
      } else {

        PHINT = pow(q1 * (E1 - COSTHG * PP) - q2 * (E2 + COSTHG * PP),
                    2) / (q1 * q1 * (E1 - COSTHG * PP) * (E1 - COSTHG * PP) + q2 * q2 * (E2 + COSTHG * PP) * (E2 + COSTHG * PP));
      }
    } else {
      PHINT  = 1.0;
    }
    return PHINT;
  }


//*****************************************************************
//*****************************************************************
//*****************************************************************
// beginning of the class of methods reading from  PH_HEPEVT
//*****************************************************************
//*****************************************************************
//*****************************************************************


//----------------------------------------------------------------------
//
//    PHOTOS:   PHOton radiation in decays event DuMP routine
//
//    Purpose:  Print event record.
//
//    Input Parameters:   Common /PH_HEPEVT/
//
//    Output Parameters:  None
//
//    Author(s):  B. van Eijk                     Created at:  05/06/90
//                                                Last Update: 20/06/13
//
//----------------------------------------------------------------------
  void PHODMP()
  {

    double  SUMVEC[5];
    int I, J;
    static int i = 1;
    const char eq80[81]  = "================================================================================";
    const char X29[30] = "                             ";
    const char X23[24 ] = "                       ";
    const char X1[2] = " ";
    const char X2[3] = "  ";
    const char X3[4] = "   ";
    const char X4[5] = "    ";
    const char X6[7] = "      ";
    const char X7[8] = "       ";
    FILE* PHLUN = stdout;

    for (I = 0; I < 5; I++)  SUMVEC[I] = 0.0;
    //--
    //--   Print event number...
    fprintf(PHLUN, "%s", eq80);
    fprintf(PHLUN, "%s Event No.: %10i\n", X29, hep.nevhep);
    fprintf(PHLUN, "%s Particle Parameters\n", X6);
    fprintf(PHLUN, "%s Nr %s Type %s Parent(s) %s Daughter(s) %s Px %s Py %s Pz %s E %s Inv. M.\n", X1, X3, X3, X2, X6, X7, X7, X7, X4);
    for (I = 1; I <= hep.nhep; I++) {
      //--
      //--   For 'stable particle' calculate vector momentum sum
      if (hep.jdahep[I - i][1 - i] == 0) {
        for (J = 1; J <= 4; J++) {
          SUMVEC[J - i] = SUMVEC[J - i] + hep.phep[I - i][J - i];
        }
        if (hep.jmohep[I - i][2 - i] == 0) {
          fprintf(PHLUN, "%4i %7i %s %4i %s Stable %9.2f %9.2f %9.2f %9.2f %9.2f\n" ,  I, hep.idhep[I - i], X3, hep.jmohep[I - i][1 - i], X7,
                  hep.phep[I - i][1 - i], hep.phep[I - i][2 - i], hep.phep[I - i][3 - i], hep.phep[I - i][4 - i], hep.phep[I - i][5 - i]);
        } else {
          fprintf(PHLUN, "%4i %7i %4i - %4i %s Stable %9.2f %9.2f %9.2f %9.2f %9.2f\n", I, hep.idhep[I - i], hep.jmohep[I - i][1 - i],
                  hep.jmohep[I - i][2 - i], X4, hep.phep[I - i][1 - i], hep.phep[I - i][2 - i], hep.phep[I - i][3 - i], hep.phep[I - i][4 - i],
                  hep.phep[I - i][5 - i]);
        }
      } else {
        if (hep.jmohep[I - i][2 - i] == 0) {
          fprintf(PHLUN, "%4i %7i %s %4i %s %4i - %4i %9.2f %9.2f %9.2f %9.2f %9.2f\n" ,  I, hep.idhep[I - i], X3, hep.jmohep[I - i][1 - i],
                  X2, hep.jdahep[I - i][1 - i], hep.jdahep[I - i][2 - i], hep.phep[I - i][1 - i], hep.phep[I - i][2 - i], hep.phep[I - i][3 - i],
                  hep.phep[I - i][4 - i], hep.phep[I - i][5 - i]);
        } else {
          fprintf(PHLUN, "%4i %7i %4i - %4i %4i - %4i %9.2f %9.2f %9.2f %9.2f %9.2f\n",  I, hep.idhep[I - i], hep.jmohep[I - i][1 - i],
                  hep.jmohep[I - i][2 - i], hep.jdahep[I - i][1 - i], hep.jdahep[I - i][2 - i], hep.phep[I - i][1 - i], hep.phep[I - i][2 - i],
                  hep.phep[I - i][3 - i], hep.phep[I - i][4 - i], hep.phep[I - i][5 - i]);
        }
      }
    }
    SUMVEC[5 - i] = sqrt(SUMVEC[4 - i] * SUMVEC[4 - i] - SUMVEC[1 - i] * SUMVEC[1 - i] - SUMVEC[2 - i] * SUMVEC[2 - i] - SUMVEC[3 - i] *
                         SUMVEC[3 - i]);
    fprintf(PHLUN, "%s  Vector Sum: %9.2f %9.2f %9.2f %9.2f %9.2f\n", X23, SUMVEC[1 - i], SUMVEC[2 - i], SUMVEC[3 - i], SUMVEC[4 - i],
            SUMVEC[5 - i]);




// 9030 FORMAT(1H ,I4,I7,3X,I4,9X,'Stable',2X,5F9.2)
//"%4i %7i %s  %4i %s Stable %s  %9.2f %9.2f %9.2f %9.2f %9.2f "  X3,9X,X2

    // 9050 FORMAT(1H ,I4,I7,3X,I4,6X,I4,' - ',I4,5F9.2)
    //"%4i %7i %s  %4i %s %4i  -  %4i  %9.2f %9.2f %9.2f %9.2f %9.2f "  X3,X6




    //"%4i %7i %4i  -  %4i %s Stable %s  %9.2f %9.2f %9.2f %9.2f %9.2f "  X5,X2


//9060 FORMAT(1H ,I4,I7,I4,' - ',I4,2X,I4,' - ',I4,5F9.2)
    //"%4i %7i %4i  -  %4i %s %4i -   %4i %9.2f %9.2f %9.2f %9.2f %9.2f "  X2,
  }



//----------------------------------------------------------------------
//
//    PHLUPAB:   debugging tool
//
//    Purpose:  NONE, eventually may printout content of the
//              /PH_HEPEVT/ common
//
//    Input Parameters:   Common /PH_HEPEVT/ and /PHNUM/
//                        latter may have number of the event.
//
//    Output Parameters:  None
//
//    Author(s):  Z. Was                          Created at:  30/05/93
//                                                Last Update: 20/06/13
//
//----------------------------------------------------------------------

  void PHLUPAB(int IPOINT)
  {
    char name[12] = "/PH_HEPEVT/";
    int I, J;
    static int IPOIN0 = -5;
    static int i = 1;
    double  SUM[5];
    FILE* PHLUN = stdout;

    if (IPOIN0 < 0) {
      IPOIN0 = 400000; //  ! maximal no-print point
      phlupy.ipoin = IPOIN0;
      phlupy.ipoinm = 400001; // ! minimal no-print point
    }

    if (IPOINT <= phlupy.ipoinm || IPOINT >= phlupy.ipoin) return;
    if ((int)phnum.iev < 1000) {
      for (I = 1; I <= 5; I++) SUM[I - i] = 0.0;

      fprintf(PHLUN, "EVENT NR= %i WE ARE TESTING %s at IPOINT=%i \n", (int)phnum.iev, name, IPOINT);
      fprintf(PHLUN, "  ID      p_x      p_y      p_z      E        m        ID-MO_DA1 ID-MO_DA2\n");
      I = 1;
      fprintf(PHLUN, "%4i %14.9f %14.9f %14.9f %14.9f %14.9f %9i %9i\n", hep.idhep[I - i], hep.phep[I - i][1 - i], hep.phep[I - i][2 - i],
              hep.phep[I - i][3 - i], hep.phep[I - i][4 - i], hep.phep[I - i][5 - i], hep.jdahep[I - i][1 - i], hep.jdahep[I - i][2 - i]);
      I = 2;
      fprintf(PHLUN, "%4i %14.9f %14.9f %14.9f %14.9f %14.9f %9i %9i\n", hep.idhep[I - i], hep.phep[I - i][1 - i], hep.phep[I - i][2 - i],
              hep.phep[I - i][3 - i], hep.phep[I - i][4 - i], hep.phep[I - i][5 - i], hep.jdahep[I - i][1 - i], hep.jdahep[I - i][2 - i]);
      fprintf(PHLUN, " \n");
      for (I = 3; I <= hep.nhep; I++) {
        fprintf(PHLUN, "%4i %14.9f %14.9f %14.9f %14.9f %14.9f %9i %9i\n", hep.idhep[I - i], hep.phep[I - i][1 - i], hep.phep[I - i][2 - i],
                hep.phep[I - i][3 - i], hep.phep[I - i][4 - i], hep.phep[I - i][5 - i], hep.jmohep[I - i][1 - i], hep.jmohep[I - i][2 - i]);
        for (J = 1; J <= 4; J++) SUM[J - i] = SUM[J - i] + hep.phep[I - i][J - i];
      }


      SUM[5 - i] = sqrt(fabs(SUM[4 - i] * SUM[4 - i] - SUM[1 - i] * SUM[1 - i] - SUM[2 - i] * SUM[2 - i] - SUM[3 - i] * SUM[3 - i]));
      fprintf(PHLUN, " SUM %14.9f %14.9f %14.9f %14.9f %14.9f\n", SUM[1 - i], SUM[2 - i], SUM[3 - i], SUM[4 - i], SUM[5 - i]);

    }


    // 10   FORMAT(1X,'  ID      ','p_x      ','p_y      ','p_z      ',
    //$                   'E        ','m        ',
    //$                   'ID-MO_DA1','ID-MO DA2' )
    // 20   FORMAT(1X,I4,5(F14.9),2I9)
    //"%i4 %14.9f %14.9f %14.9f %14.9f %i9 i9"
    // 30   FORMAT(1X,' SUM',5(F14.9))
  }









//----------------------------------------------------------------------
//
//    PHLUPA:   debugging tool
//
//    Purpose:  NONE, eventually may printout content of the
//              /PHOEVT/ common
//
//    Input Parameters:   Common /PHOEVT/ and /PHNUM/
//                        latter may have number of the event.
//
//    Output Parameters:  None
//
//    Author(s):  Z. Was                          Created at:  30/05/93
//                                                Last Update: 21/06/13
//
//----------------------------------------------------------------------

  void PHLUPA(int IPOINT)
  {
    char name[9] = "/PHOEVT/";
    int I, J;
    static int IPOIN0 = -5;
    static int i = 1;
    double  SUM[5];
    FILE* PHLUN = stdout;

    if (IPOIN0 < 0) {
      IPOIN0 = 400000; //  ! maximal no-print point
      phlupy.ipoin = IPOIN0;
      phlupy.ipoinm = 400001; // ! minimal no-print point
    }

    if (IPOINT <= phlupy.ipoinm || IPOINT >= phlupy.ipoin) return;
    if ((int)phnum.iev < 1000) {
      for (I = 1; I <= 5; I++) SUM[I - i] = 0.0;

      fprintf(PHLUN, "EVENT NR= %i WE ARE TESTING %s at IPOINT=%i \n", (int)phnum.iev, name, IPOINT);
      fprintf(PHLUN, "  ID      p_x      p_y      p_z      E        m        ID-MO_DA1 ID-MO_DA2\n");
      I = 1;
      fprintf(PHLUN, "%4i %14.9f %14.9f %14.9f %14.9f %14.9f %9i %9i\n", pho.idhep[I - i], pho.phep[I - i][1 - i], pho.phep[I - i][2 - i],
              pho.phep[I - i][3 - i], pho.phep[I - i][4 - i], pho.phep[I - i][5 - i], pho.jdahep[I - i][1 - i], pho.jdahep[I - i][2 - i]);
      I = 2;
      fprintf(PHLUN, "%4i %14.9f %14.9f %14.9f %14.9f %14.9f %9i %9i\n", pho.idhep[I - i], pho.phep[I - i][1 - i], pho.phep[I - i][2 - i],
              pho.phep[I - i][3 - i], pho.phep[I - i][4 - i], pho.phep[I - i][5 - i], pho.jdahep[I - i][1 - i], pho.jdahep[I - i][2 - i]);
      fprintf(PHLUN, " \n");
      for (I = 3; I <= pho.nhep; I++) {
        fprintf(PHLUN, "%4i %14.9f %14.9f %14.9f %14.9f %14.9f %9i %9i\n", pho.idhep[I - i], pho.phep[I - i][1 - i], pho.phep[I - i][2 - i],
                pho.phep[I - i][3 - i], pho.phep[I - i][4 - i], pho.phep[I - i][5 - i], pho.jmohep[I - i][1 - i], pho.jmohep[I - i][2 - i]);
        for (J = 1; J <= 4; J++) SUM[J - i] = SUM[J - i] + pho.phep[I - i][J - i];
      }


      SUM[5 - i] = sqrt(fabs(SUM[4 - i] * SUM[4 - i] - SUM[1 - i] * SUM[1 - i] - SUM[2 - i] * SUM[2 - i] - SUM[3 - i] * SUM[3 - i]));
      fprintf(PHLUN, " SUM %14.9f %14.9f %14.9f %14.9f %14.9f\n", SUM[1 - i], SUM[2 - i], SUM[3 - i], SUM[4 - i], SUM[5 - i]);

    }


    // 10   FORMAT(1X,'  ID      ','p_x      ','p_y      ','p_z      ',
    //$                   'E        ','m        ',
    //$                   'ID-MO_DA1','ID-MO DA2' )
    // 20   FORMAT(1X,I4,5(F14.9),2I9)
    //"%4i %14.9f %14.9f %14.9f %14.9f %9i %9i"
    // 30   FORMAT(1X,' SUM',5(F14.9))
  }


  void PHOtoRF()
  {


    //      COMMON /PH_TOFROM/ QQ[4],XM,th1,fi1
    double PP[4], RR[4];

    int K, L;
    static int i = 1;

    for (K = 1; K <= 4; K++) {
      tofrom.QQ[K - i] = 0.0;
    }
    for (L = hep.jdahep[hep.jmohep[hep.nhep - i][1 - i] - i][1 - i]; L <= hep.jdahep[hep.jmohep[hep.nhep - i][1 - i] - i][2 - i]; L++) {
      for (K = 1; K <= 4; K++) {
        tofrom.QQ[K - i] = tofrom.QQ[K - i] + hep.phep[L - i][K - i];
      }
    }
    tofrom.XM = tofrom.QQ[4 - i] * tofrom.QQ[4 - i] - tofrom.QQ[3 - i] * tofrom.QQ[3 - i] - tofrom.QQ[2 - i] * tofrom.QQ[2 - i] -
                tofrom.QQ[1 - i] * tofrom.QQ[1 - i];
    if (tofrom.XM > 0.0) tofrom.XM = sqrt(tofrom.XM);
    if (tofrom.XM <= 0.0) return;

    for (L = 1; L <= hep.nhep; L++) {
      for (K = 1; K <= 4; K++) {
        PP[K - i] = hep.phep[L - i][K - i];
      }
      bostdq(1, tofrom.QQ, PP, RR);
      for (K = 1; K <= 4; K++) {
        hep.phep[L - i][K - i] = RR[K - i];
      }
    }

    tofrom.fi1 = 0.0;
    tofrom.th1 = 0.0;
    if (fabs(hep.phep[1 - i][1 - i]) + fabs(hep.phep[1 - i][2 - i]) > 0.0) tofrom.fi1 = PHOAN1(hep.phep[1 - i][1 - i],
          hep.phep[1 - i][2 - i]);
    if (fabs(hep.phep[1 - i][1 - i]) + fabs(hep.phep[1 - i][2 - i]) + fabs(hep.phep[1 - i][3 - i]) > 0.0)
      tofrom.th1 = PHOAN2(hep.phep[1 - i][3 - i],
                          sqrt(hep.phep[1 - i][1 - i] * hep.phep[1 - i][1 - i] + hep.phep[1 - i][2 - i] * hep.phep[1 - i][2 - i]));

    for (L = 1; L <= hep.nhep; L++) {
      for (K = 1; K <= 4; K++) {
        RR[K - i] = hep.phep[L - i][K - i];
      }

      PHORO3(-tofrom.fi1, RR);
      PHORO2(-tofrom.th1, RR);
      for (K = 1; K <= 4; K++) {
        hep.phep[L - i][K - i] = RR[K - i];
      }
    }

    return;
  }

  void PHOtoLAB()
  {

    //  //      REAL*8 QQ(4),XM,th1,fi1
    //     COMMON /PH_TOFROM/ QQ,XM,th1,fi1
    double PP[4], RR[4];
    int K, L;
    static int i = 1;

    if (tofrom.XM <= 0.0) return;


    for (L = 1; L <= hep.nhep; L++) {
      for (K = 1; K <= 4; K++) {
        PP[K - i] = hep.phep[L - i][K - i];
      }

      PHORO2(tofrom.th1, PP);
      PHORO3(tofrom.fi1, PP);
      bostdq(-1, tofrom.QQ, PP, RR);

      for (K = 1; K <= 4; K++) {
        hep.phep[L - i][K - i] = RR[K - i];
      }
    }
    return;
  }
  void PHOcorrectDARK(int IDaction)
  {
    int K, L;
    static int i = 1;
    // if(darkr.ifspecial==1)
    {
      for (L = 1; L <= hep.nhep; L++) {
        if (hep.idhep[L - i] == darkr.IDspecial) {
          //PHODMP();
          hep.jdahep[L - i][1 - i] = L - i + 1 + 1;
          hep.jdahep[L - i][2 - i] = L - i + 2 + 1;
          hep.jmohep[L - i + 1][1 - i] = L - i + 1;
          hep.jmohep[L - i + 2][1 - i] = L - i + 1;

          hep.jmohep[L - i - 2][2 - i] = 0;
          hep.jmohep[L - i - 1][2 - i] = 0;
          hep.jmohep[L - i][2 - i] = 0;

          hep.jmohep[L - i + 1][2 - i] = 0;
          hep.jmohep[L - i + 2][2 - i] = 0;
          // cout << "adres prosze= " << hep.jmohep[L-i][1-i] << endl;
          hep.jdahep[ hep.jmohep[L - i][1 - i] - i ][2 - i] = hep.jdahep[ hep.jmohep[L - i][1 - i] - i  ][2 - i] - 2;
          //PHODMP();
        }
      }
    }
  }






//             2) GENERAL INTERFACE:
//                                      PHOTOS_GET
//                                      PHOTOS_MAKE


//   COMMONS:
//   NAME     USED IN SECT. # OF OC//     Comment
//   PHOQED   1) 2)            3      Flags whether emisson to be gen.
//   PHOLUN   1) 4)            6      Output device number
//   PHOCOP   1) 3)            4      photon coupling & min energy
//   PHPICO   1) 3) 4)         5      PI & 2*PI
//   PHSEED   1) 4)            3      RN seed
//   PHOSTA   1) 4)            3      Status information
//   PHOKEY   1) 2) 3)         7      Keys for nonstandard application
//   PHOVER   1)               1      Version info for outside
//   HEPEVT   2)               2      PDG common
//   PH_HEPEVT2)               8      PDG common internal
//   PHOEVT   2) 3)           10      PDG branch
//   PHOIF    2) 3)            2      emission flags for PDG branch
//   PHOMOM   3)               5      param of char-neutr system
//   PHOPHS   3)               5      photon momentum parameters
//   PHOPRO   3)               4      var. for photon rep. (in branch)
//   PHOCMS   2)               3      parameters of boost to branch CMS
//   PHNUM    4)               1      event number from outside
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//
//    PHOTOS_MAKE:   General search routine
//
//    Purpose:  Search through the /PH_HEPEVT/ standard HEP common, sta-
//              rting from  the IPPAR-th  particle.  Whenevr  branching
//              point is found routine PHTYPE(IP) is called.
//              Finally if calls on PHTYPE(IP) modified entries, common
//               /PH_HEPEVT/ is ordered.
//
//    Input Parameter:    IPPAR:  Pointer   to   decaying  particle  in
//                                /PH_HEPEVT/ and the common itself,
//
//    Output Parameters:  Common  /PH_HEPEVT/, either with or without
//                                new particles added.
//
//    Author(s):  Z. Was, B. van Eijk             Created at:  26/11/89
//                                                Last Update: 30/08/93
//
//----------------------------------------------------------------------

  void PHOTOS_MAKE_C(int IPARR)
  {
    static int i = 1;
    int IPPAR, I, J, NLAST, MOTHER;
    //--
    PHLUPAB(3);

    //      write(*,*) 'at poczatek'
    //       PHODMP();
    IPPAR = abs(IPARR);
    //--   Store pointers for cascade treatement...
    NLAST = hep.nhep;


    //--
    //--   Check decay multiplicity and minimum of correctness..
    if ((hep.jdahep[IPPAR - i][1 - i] == 0) || (hep.jmohep[hep.jdahep[IPPAR - i][1 - i] - i][1 - i] != IPPAR)) return;

    PHOtoRF();

    //      write(*,*) 'at przygotowany'
    //       PHODMP();

    //--
    //-- single branch mode
    //-- IPPAR is original position where the program was called

    //-- let-s do generation
    PHTYPE(IPPAR);


    //--   rearrange  /PH_HEPEVT/  for added particles.
    //--   at present this may be not needed as information
    //--   is set at HepMC level.
    if (hep.nhep > NLAST) {
      for (I = NLAST + 1; I <= hep.nhep; I++) {
        //--
        //--   Photon mother and vertex...
        MOTHER = hep.jmohep[I - i][1 - i];
        hep.jdahep[MOTHER - i][2 - i] = I;
        for (J = 1; J <= 4; J++) {
          hep.vhep[I - i][J - i] = hep.vhep[I - 1 - i][J - i];
        }
      }
    }
    //      write(*,*) 'at po dzialaniu '
    //      PHODMP();
    PHOtoLAB();
    PHOcorrectDARK(1);
    //      write(*,*) 'at koniec'
    //      PHODMP();
    return;
  }




//----------------------------------------------------------------------
//
//    PHCORK: corrects kinmatics of subbranch needed if host program
//            produces events with the shaky momentum conservation
//
//    Input Parameters:   Common /PHOEVT/, MODCOR
//                        MODCOR >0 type of action
//                               =1 no action
//                               =2 corrects energy from mass
//                               =3 corrects mass from energy
//                               =4 corrects energy from mass for
//                                  particles up to .4 GeV mass,
//                                  for heavier ones corrects mass,
//                               =5 most complete correct also of mother
//                                  often necessary for exponentiation.
//                               =0 execution mode
//
//    Output Parameters:  corrected /PHOEVT/
//
//    Author(s):  P.Golonka, Z. Was               Created at:  01/02/99
//              Modified  :  07/07/13
//----------------------------------------------------------------------

  void PHCORK(int MODCOR)
  {

    double M, P2, PX, PY, PZ, E, EN, XMS;
    int I, K;
    FILE* PHLUN = stdout;


    static int MODOP = 0;
    static int IPRINT = 0;
    static double  MCUT = 0.4;
    static int i = 1;

    if (MODCOR != 0) {
      //       INITIALIZATION
      MODOP = MODCOR;

      fprintf(PHLUN, "Message from PHCORK(MODCOR):: initialization\n");
      if (MODOP == 1) fprintf(PHLUN, "MODOP=1 -- no corrections on event: DEFAULT\n");
      else if (MODOP == 2) fprintf(PHLUN, "MODOP=2 -- corrects Energy from mass\n");
      else if (MODOP == 3) fprintf(PHLUN, "MODOP=3 -- corrects mass from Energy\n");
      else if (MODOP == 4) {
        fprintf(PHLUN, "MODOP=4 -- corrects Energy from mass to Mcut\n");
        fprintf(PHLUN, "           and mass from  energy above  Mcut\n");
        fprintf(PHLUN, " Mcut=%6.3f GeV", MCUT);
      } else if (MODOP == 5) fprintf(PHLUN, "MODOP=5 -- corrects Energy from mass+flow\n");

      else {
        fprintf(PHLUN, "PHCORK wrong MODCOR=%4i\n", MODCOR);
        exit(-1);
      }
      return;
    }

    if (MODOP == 0 && MODCOR == 0) {
      fprintf(PHLUN, "PHCORK lack of initialization\n");
      exit(-1);
    }

    // execution mode
    // ==============
    // ==============


    PX = 0.0;
    PY = 0.0;
    PZ = 0.0;
    E = 0.0;

    if (MODOP == 1) {
      //     -----------------------
      //       In this case we do nothing
      return;
    } else if (MODOP == 2) {
      //     -----------------------
      //      lets loop thru all daughters and correct their energies
      //      according to E^2=p^2+m^2

      for (I = 3; I <= pho.nhep; I++) {

        PX = PX + pho.phep[I - i][1 - i];
        PY = PY + pho.phep[I - i][2 - i];
        PZ = PZ + pho.phep[I - i][3 - i];

        P2 = pho.phep[I - i][1 - i] * pho.phep[I - i][1 - i] + pho.phep[I - i][2 - i] * pho.phep[I - i][2 - i] + pho.phep[I - i][3 - i] *
             pho.phep[I - i][3 - i];

        EN = sqrt(pho.phep[I - i][5 - i] * pho.phep[I - i][5 - i] + P2);

        if (IPRINT == 1)fprintf(PHLUN, "CORRECTING ENERGY OF %6i: %14.9f => %14.9f\n", I, pho.phep[I - i][4 - i], EN);

        pho.phep[I - i][4 - i] = EN;
        E = E + pho.phep[I - i][4 - i];

      }
    }

    else if (MODOP == 5) {
      //     -----------------------
      //C      lets loop thru all daughters and correct their energies
      //C      according to E^2=p^2+m^2

      for (I = 3; I <= pho.nhep; I++) {
        PX = PX + pho.phep[I - i][1 - i];
        PY = PY + pho.phep[I - i][2 - i];
        PZ = PZ + pho.phep[I - i][3 - i];

        P2 = pho.phep[I - i][1 - i] * pho.phep[I - i][1 - i] + pho.phep[I - i][2 - i] * pho.phep[I - i][2 - i] + pho.phep[I - i][3 - i] *
             pho.phep[I - i][3 - i];

        EN = sqrt(pho.phep[I - i][5 - i] * pho.phep[I - i][5 - i] + P2);

        if (IPRINT == 1)fprintf(PHLUN, "CORRECTING ENERGY OF %6i: %14.9f => %14.9f\n", I, pho.phep[I - i][4 - i], EN);

        pho.phep[I - i][4 - i] = EN;
        E = E + pho.phep[I - i][4 - i];

      }
      for (K = 1; K <= 4; K++) {
        pho.phep[1 - i][K - i] = 0.0;
        for (I = 3; I <= pho.nhep; I++) {
          pho.phep[1 - i][K - i] = pho.phep[1 - i][K - i] + pho.phep[I - i][K - i];
        }
      }
      XMS = sqrt(pho.phep[1 - i][4 - i] * pho.phep[1 - i][4 - i] - pho.phep[1 - i][3 - i] * pho.phep[1 - i][3 - i] - pho.phep[1 - i][2 -
                 i] * pho.phep[1 - i][2 - i] - pho.phep[1 - i][1 - i] * pho.phep[1 - i][1 - i]);
      pho.phep[1 - i][5 - i] = XMS;
    } else if (MODOP == 3) {
      //     -----------------------

      //      lets loop thru all daughters and correct their masses
      //     according to E^2=p^2+m^2

      for (I = 3; I <= pho.nhep; I++) {

        PX = PX + pho.phep[I - i][1 - i];
        PY = PY + pho.phep[I - i][2 - i];
        PZ = PZ + pho.phep[I - i][3 - i];
        E = E + pho.phep[I - i][4 - i];

        P2 = pho.phep[I - i][1 - i] * pho.phep[I - i][1 - i] + pho.phep[I - i][2 - i] * pho.phep[I - i][2 - i] + pho.phep[I - i][3 - i] *
             pho.phep[I - i][3 - i];

        M = sqrt(fabs(pho.phep[I - i][4 - i] * pho.phep[I - i][4 - i] - P2));

        if (IPRINT == 1) fprintf(PHLUN, "CORRECTING MASS OF %6i: %14.9f => %14.9f\n", I, pho.phep[I - i][5 - i], M);

        pho.phep[I - i][5 - i] = M;

      }

    } else if (MODOP == 4) {
      //     -----------------------

      //      lets loop thru all daughters and correct their masses
      //      or energies according to E^2=p^2+m^2

      for (I = 3; I <= pho.nhep; I++) {

        PX = PX + pho.phep[I - i][1 - i];
        PY = PY + pho.phep[I - i][2 - i];
        PZ = PZ + pho.phep[I - i][3 - i];
        P2 = pho.phep[I - i][1 - i] * pho.phep[I - i][1 - i] + pho.phep[I - i][2 - i] * pho.phep[I - i][2 - i] + pho.phep[I - i][3 - i] *
             pho.phep[I - i][3 - i];
        M = sqrt(fabs(pho.phep[I - i][4 - i] * pho.phep[I - i][4 - i] - P2));


        if (M > MCUT) {
          if (IPRINT == 1) fprintf(PHLUN, "CORRECTING MASS OF %6i: %14.9f => %14.9f\n", I, pho.phep[I - i][5 - i], M);
          pho.phep[I - i][5 - i] = M;
          E = E + pho.phep[I - i][4 - i];
        } else {

          EN = sqrt(pho.phep[I - i][5 - i] * pho.phep[I - i][5 - i] + P2);
          if (IPRINT == 1) fprintf(PHLUN, "CORRECTING ENERGY OF %6i: %14.9f =>% 14.9f\n", I , pho.phep[I - i][4 - i], EN);

          pho.phep[I - i][4 - i] = EN;
          E = E + pho.phep[I - i][4 - i];
        }


      }
    }

    //     -----

    if (IPRINT == 1) {
      fprintf(PHLUN, "CORRECTING MOTHER");
      fprintf(PHLUN, "PX:%14.9f =>%14.9f", pho.phep[1 - i][1 - i], PX - pho.phep[2 - i][1 - i]);
      fprintf(PHLUN, "PY:%14.9f =>%14.9f", pho.phep[1 - i][2 - i], PY - pho.phep[2 - i][2 - i]);
      fprintf(PHLUN, "PZ:%14.9f =>%14.9f", pho.phep[1 - i][3 - i], PZ - pho.phep[2 - i][3 - i]);
      fprintf(PHLUN, " E:%14.9f =>%14.9f", pho.phep[1 - i][4 - i], E - pho.phep[2 - i][4 - i]);
    }

    pho.phep[1 - i][1 - i] = PX - pho.phep[2 - i][1 - i];
    pho.phep[1 - i][2 - i] = PY - pho.phep[2 - i][2 - i];
    pho.phep[1 - i][3 - i] = PZ - pho.phep[2 - i][3 - i];
    pho.phep[1 - i][4 - i] = E - pho.phep[2 - i][4 - i];


    P2 = pho.phep[1 - i][1 - i] * pho.phep[1 - i][1 - i] + pho.phep[1 - i][2 - i] * pho.phep[1 - i][2 - i] + pho.phep[1 - i][3 - i] *
         pho.phep[1 - i][3 - i];
    if (pho.phep[1 - i][4 - i]*pho.phep[1 - i][4 - i] > P2) {
      M = sqrt(pho.phep[1 - i][4 - i] * pho.phep[1 - i][4 - i] - P2);
      if (IPRINT == 1)fprintf(PHLUN, " M: %14.9f => %14.9f\n", pho.phep[1 - i][5 - i], M);
      pho.phep[1 - i][5 - i] = M;
    }

    PHLUPA(25);

  }






//----------------------------------------------------------------------
//
//    PHOTOS:   PHOton radiation in  decays DOing of KINematics
//
//    Purpose:  Starting  from   the  charged  particle energy/momentum,
//              PNEUTR, photon  energy  fraction and photon  angle  with
//              respect  to  the axis formed by charged particle energy/
//              momentum  vector  and PNEUTR, scale the energy/momentum,
//              keeping the original direction of the neutral system  in
//              the lab. frame untouched.
//
//    Input Parameters:   IP:      Pointer  to   decaying  particle   in
//                                 /PHOEVT/  and   the   common   itself
//                        NCHARB:  pointer to the charged radiating
//                                 daughter in /PHOEVT/.
//                        NEUDAU:  pointer to the first neutral daughter
//    Output Parameters:  Common /PHOEVT/, with photon added.
//
//    Author(s):  Z. Was, B. van Eijk             Created at:  26/11/89
//                                                Last Update: 27/05/93
//
//----------------------------------------------------------------------

  void PHODO(int IP, int NCHARB, int NEUDAU)
  {
    static int i = 1;
    double QNEW, QOLD, EPHOTO, PMAVIR;
    double GNEUT, DATA;
    double CCOSTH, SSINTH, PVEC[4], PARNE;
    double TH3, FI3, TH4, FI4, FI5, ANGLE;
    int I, J, FIRST, LAST;
    double& COSTHG = phophs.costhg;
    double& SINTHG = phophs.sinthg;
    double& XPHOTO = phophs.xphoto;
    double* PNEUTR = phomom.pneutr;
    double& MNESQR = phomom.mnesqr;

    //--
    EPHOTO = XPHOTO * pho.phep[IP - i][5 - i] / 2.0;
    PMAVIR = sqrt(pho.phep[IP - i][5 - i] * (pho.phep[IP - i][5 - i] - 2.0 * EPHOTO));
    //--
    //--   Reconstruct  kinematics  of  charged particle  and  neutral system
    phorest.fi1 = PHOAN1(PNEUTR[1 - i], PNEUTR[2 - i]);
    //--
    //--   Choose axis along  z of  PNEUTR, calculate  angle  between x and y
    //--   components  and z  and x-y plane and  perform Lorentz transform...
    phorest.th1 = PHOAN2(PNEUTR[3 - i], sqrt(PNEUTR[1 - i] * PNEUTR[1 - i] + PNEUTR[2 - i] * PNEUTR[2 - i]));
    PHORO3(-phorest.fi1, PNEUTR);
    PHORO2(-phorest.th1, PNEUTR);
    //--
    //--   Take  away  photon energy from charged particle and PNEUTR !  Thus
    //--   the onshell charged particle  decays into virtual charged particle
    //--   and photon.  The virtual charged  particle mass becomes:
    //--   SQRT(pho.phep[5,IP)*(pho.phep[5,IP)-2*EPHOTO)).  Construct  new PNEUTR mo-
    //--   mentum in the rest frame of the parent:
    //--   1) Scaling parameters...
    QNEW = PHOTRI(PMAVIR, PNEUTR[5 - i], pho.phep[NCHARB - i][5 - i]);
    QOLD = PNEUTR[3 - i];
    GNEUT = (QNEW * QNEW + QOLD * QOLD + MNESQR) / (QNEW * QOLD + sqrt((QNEW * QNEW + MNESQR) * (QOLD * QOLD + MNESQR)));
    if (GNEUT < 1.0) {
      DATA = 0.0;
      PHOERR(4, "PHOKIN", DATA);
    }
    PARNE = GNEUT - sqrt(max(GNEUT * GNEUT - 1.0, 0.0));
    //--
    //--   2) ...reductive boost...
    PHOBO3(PARNE, PNEUTR);
    //--
    //--   ...calculate photon energy in the reduced system...
    pho.nhep = pho.nhep + 1;
    pho.isthep[pho.nhep - i] = 1;
    pho.idhep[pho.nhep - i] = 22;
    //--   Photon mother and daughter pointers !
    pho.jmohep[pho.nhep - i][1 - i] = IP;
    pho.jmohep[pho.nhep - i][2 - i] = 0;
    pho.jdahep[pho.nhep - i][1 - i] = 0;
    pho.jdahep[pho.nhep - i][2 - i] = 0;
    pho.phep[pho.nhep - i][4 - i] = EPHOTO * pho.phep[IP - i][5 - i] / PMAVIR;
    //--
    //--   ...and photon momenta
    CCOSTH = -COSTHG;
    SSINTH = SINTHG;
    TH3 = PHOAN2(CCOSTH, SSINTH);
    FI3 = TWOPI * Photos::randomDouble();
    pho.phep[pho.nhep - i][1 - i] = pho.phep[pho.nhep - i][4 - i] * SINTHG * cos(FI3);
    pho.phep[pho.nhep - i][2 - i] = pho.phep[pho.nhep - i][4 - i] * SINTHG * sin(FI3);
    //--
    //--   Minus sign because axis opposite direction of charged particle !
    pho.phep[pho.nhep - i][3 - i] = -pho.phep[pho.nhep - i][4 - i] * COSTHG;
    pho.phep[pho.nhep - i][5 - i] = 0.0;
    //--
    //--   Rotate in order to get photon along z-axis
    PHORO3(-FI3, PNEUTR);
    PHORO3(-FI3, pho.phep[pho.nhep - i]);
    PHORO2(-TH3, PNEUTR);
    PHORO2(-TH3, pho.phep[pho.nhep - i]);
    ANGLE = EPHOTO / pho.phep[pho.nhep - i][4 - i];
    //--
    //--   Boost to the rest frame of decaying particle
    PHOBO3(ANGLE, PNEUTR);
    PHOBO3(ANGLE, pho.phep[pho.nhep - i]);
    //--
    //--   Back in the parent rest frame but PNEUTR not yet oriented !
    FI4 = PHOAN1(PNEUTR[1 - i], PNEUTR[2 - i]);
    TH4 = PHOAN2(PNEUTR[3 - i], sqrt(PNEUTR[1 - i] * PNEUTR[1 - i] + PNEUTR[2 - i] * PNEUTR[2 - i]));
    PHORO3(FI4, PNEUTR);
    PHORO3(FI4, pho.phep[pho.nhep - i]);
    //--
    for (I = 2; I <= 4; I++) PVEC[I - i] = 0.0;
    PVEC[1 - i] = 1.0;

    PHORO3(-FI3, PVEC);
    PHORO2(-TH3, PVEC);
    PHOBO3(ANGLE, PVEC);
    PHORO3(FI4, PVEC);
    PHORO2(-TH4, PNEUTR);
    PHORO2(-TH4, pho.phep[pho.nhep - i]);
    PHORO2(-TH4, PVEC);
    FI5 = PHOAN1(PVEC[1 - i], PVEC[2 - i]);
    //--
    //--   Charged particle restores original direction
    PHORO3(-FI5, PNEUTR);
    PHORO3(-FI5, pho.phep[pho.nhep - i]);
    PHORO2(phorest.th1, PNEUTR);
    PHORO2(phorest.th1, pho.phep[pho.nhep - i]);
    PHORO3(phorest.fi1, PNEUTR);
    PHORO3(phorest.fi1, pho.phep[pho.nhep - i]);
    //--   See whether neutral system has multiplicity larger than 1...

    if ((pho.jdahep[IP - i][2 - i] - pho.jdahep[IP - i][1 - i]) > 1) {
      //--   Find pointers to components of 'neutral' system
      //--
      FIRST = NEUDAU;
      LAST = pho.jdahep[IP - i][2 - i];
      for (I = FIRST; I <= LAST; I++) {
        if (I != NCHARB && (pho.jmohep[I - i][1 - i] == IP)) {
          //--
          //--   Reconstruct kinematics...
          PHORO3(-phorest.fi1, pho.phep[I - i]);
          PHORO2(-phorest.th1, pho.phep[I - i]);
          //--
          //--   ...reductive boost
          PHOBO3(PARNE, pho.phep[I - i]);
          //--
          //--   Rotate in order to get photon along z-axis
          PHORO3(-FI3, pho.phep[I - i]);
          PHORO2(-TH3, pho.phep[I - i]);
          //--
          //--   Boost to the rest frame of decaying particle
          PHOBO3(ANGLE, pho.phep[I - i]);
          //--
          //--   Back in the parent rest-frame but PNEUTR not yet oriented.
          PHORO3(FI4, pho.phep[I - i]);
          PHORO2(-TH4, pho.phep[I - i]);
          //--
          //--   Charged particle restores original direction
          PHORO3(-FI5, pho.phep[I - i]);
          PHORO2(phorest.th1, pho.phep[I - i]);
          PHORO3(phorest.fi1, pho.phep[I - i]);
        }
      }
    } else {
      //--
      //   ...only one 'neutral' particle in addition to photon!
      for (J = 1; J <= 4; J++) pho.phep[NEUDAU - i][J - i] = PNEUTR[J - i];
    }
    //--
    //--   All 'neutrals' treated, fill /PHOEVT/ for charged particle...
    for (J = 1; J <= 3; J++) pho.phep[NCHARB - i][J - i] = -(pho.phep[pho.nhep - i][J - i] + PNEUTR[J - i]);
    pho.phep[NCHARB - i][4 - i] = pho.phep[IP - i][5 - i] - (pho.phep[pho.nhep - i][4 - i] + PNEUTR[4 - i]);
    //--
  }


//----------------------------------------------------------------------
//
//    PHOTOS:   PHOtos Boson W correction weight
//
//    Purpose:  calculates correction weight due to amplitudes of
//              emission from W boson.
//
//
//
//
//
//    Input Parameters:  Common /PHOEVT/, with photon added.
//                       wt  to be corrected
//
//
//
//    Output Parameters: wt
//
//    Author(s):  G. Nanava, Z. Was               Created at:  13/03/03
//                                                Last Update: 08/07/13
//
//----------------------------------------------------------------------

  void PHOBW(double* WT)
  {
    static int i = 1;
    int I;
    double EMU, MCHREN, BETA, COSTHG, MPASQR, XPH;
    //--
    if (abs(pho.idhep[1 - i]) == 24 &&
        abs(pho.idhep[pho.jdahep[1 - i][1 - i] - i])  >= 11 &&
        abs(pho.idhep[pho.jdahep[1 - i][1 - i] - i])  <= 16 &&
        abs(pho.idhep[pho.jdahep[1 - i][1 - i] + 1 - i]) >= 11 &&
        abs(pho.idhep[pho.jdahep[1 - i][1 - i] + 1 - i]) <= 16) {

      if (
        abs(pho.idhep[pho.jdahep[1 - i][1 - i] - i]) == 11 ||
        abs(pho.idhep[pho.jdahep[1 - i][1 - i] - i]) == 13 ||
        abs(pho.idhep[pho.jdahep[1 - i][1 - i] - i]) == 15) {
        I = pho.jdahep[1 - i][1 - i];
      } else {
        I = pho.jdahep[1 - i][1 - i] + 1;
      }

      EMU = pho.phep[I - i][4 - i];
      MCHREN = fabs(pow(pho.phep[I - i][4 - i], 2) - pow(pho.phep[I - i][3 - i], 2)
                    - pow(pho.phep[I - i][2 - i], 2) - pow(pho.phep[I - i][1 - i], 2));
      BETA = sqrt(1.0 - MCHREN / pho.phep[I - i][4 - i] / pho.phep[I - i][4 - i]);
      COSTHG = (pho.phep[I - i][3 - i] * pho.phep[pho.nhep - i][3 - i] + pho.phep[I - i][2 - i] * pho.phep[pho.nhep - i][2 - i]
                + pho.phep[I - i][1 - i] * pho.phep[pho.nhep - i][1 - i]) /
               sqrt(pho.phep[I - i][3 - i] * pho.phep[I - i][3 - i] + pho.phep[I - i][2 - i] * pho.phep[I - i][2 - i] + pho.phep[I - i][1 - i] *
                    pho.phep[I - i][1 - i]) /
               sqrt(pho.phep[pho.nhep - i][3 - i] * pho.phep[pho.nhep - i][3 - i] + pho.phep[pho.nhep - i][2 - i] * pho.phep[pho.nhep - i][2 - i] +
                    pho.phep[pho.nhep - i][1 - i] * pho.phep[pho.nhep - i][1 - i]);
      MPASQR = pho.phep[1 - i][4 - i] * pho.phep[1 - i][4 - i];
      XPH = pho.phep[pho.nhep - i][4 - i];
      *WT = (*WT) * (1 - 8 * EMU * XPH * (1 - COSTHG * BETA) *
                     (MCHREN + 2 * XPH * sqrt(MPASQR)) /
                     (MPASQR * MPASQR) / (1 - MCHREN / MPASQR) / (4 - MCHREN / MPASQR));
    }
    //        write(*,*) pho.idhep[1),pho.idhep[pho.jdahep[1,1)),pho.idhep[pho.jdahep[1,1)+1)
    //        write(*,*) emu,xph,costhg,beta,mpasqr,mchren

  }



//----------------------------------------------------------------------
//
//    PHOTOS:   PHOton radiation in decays control FACtor
//
//    Purpose:  This is the control function for the photon spectrum and
//              final weighting.  It is  called  from PHOENE for genera-
//              ting the raw photon energy spectrum (MODE=0) and in PHO-
//              COR to scale the final weight (MODE=1).  The factor con-
//              sists of 3 terms.  Addition of  the factor FF which mul-
//              tiplies PHOFAC for MODE=0 and divides PHOFAC for MODE=1,
//              does not affect  the results for  the MC generation.  An
//              appropriate choice  for FF can speed up the calculation.
//              Note that a too small value of FF may cause weight over-
//              flow in PHOCOR  and will generate a warning, halting the
//              execution.  PRX  should  be  included for repeated calls
//              for  the  same event, allowing more particles to radiate
//              photons.  At  the  first  call IREP=0, for  more  than 1
//              charged  decay  products, IREP >= 1.  Thus,  PRSOFT  (no
//              photon radiation  probability  in  the  previous  calls)
//              appropriately scales the strength of the bremsstrahlung.
//
//    Input Parameters:  MODE, PROBH, XF
//
//    Output Parameter:  Function value
//
//    Author(s):  S. Jadach, Z. Was               Created at:  01/01/89
//                B. van Eijk, P.Golonka          Last Update: 09/07/13
//
//----------------------------------------------------------------------

  double PHOFAC(int MODE)
  {
    static  double FF = 0.0, PRX = 0.0;

    if (phokey.iexp)  return 1.0; // In case of exponentiation this routine is useles

    if (MODE == -1) {
      PRX = 1.0;
      FF = 1.0;
      phopro.probh = 0.0;
    } else if (MODE == 0) {
      if (phopro.irep == 0) PRX = 1.0;
      PRX = PRX / (1.0 - phopro.probh);
      FF = 1.0;
      //--
      //--   Following options are not considered for the time being...
      //--   (1) Good choice, but does not save very much time:
      //--       FF=(1.0-sqrt(phopro.xf)/2.0)/(1.0+sqrt(phopro.xf)/2.0)
      //--   (2) Taken from the blue, but works without weight overflows...
      //--       FF=(1.0-phopro.xf/(1-pow((1-sqrt(phopro.xf)),2)))*(1+(1-sqrt(phopro.xf))/sqrt(1-phopro.xf))/2.0
      return FF * PRX;
    } else {
      return 1.0 / FF;
    }

    return NAN;
  }



// ######
//  replace with,
// ######

//----------------------------------------------------------------------
//
//    PHOTOS:   PHOton radiation in decays CORrection weight from
//              matrix elements This version for spin 1/2 is verified for
//              W decay only
//    Purpose:  Calculate  photon  angle.  The reshaping functions  will
//              have  to  depend  on the spin S of the charged particle.
//              We define:  ME = 2 * S + 1 !
//              THIS IS POSSIBLY ALWAYS BETTER THAN PHOCOR()
//
//    Input Parameters:  MPASQR:  Parent mass squared,
//                       MCHREN:  Renormalised mass of charged system,
//                       ME:      2 * spin + 1 determines matrix element
//
//    Output Parameter:  Function value.
//
//    Author(s):  Z. Was, B. van Eijk, G. Nanava  Created at:  26/11/89
//                                                Last Update: 01/11/12
//
//----------------------------------------------------------------------

  double PHOCORN(double MPASQR, double MCHREN, int ME)
  {
    double wt1, wt2, wt3;
    double  beta0, beta1, XX, YY, DATA;
    double S1, PHOCOR;
    double& COSTHG = phophs.costhg;
    double& XPHMAX = phophs.xphmax;
    double& XPHOTO = phophs.xphoto;
    double& MCHSQR = phomom.mchsqr;
    double& MNESQR = phomom.mnesqr;



    //--
    //--   Shaping (modified by ZW)...
    XX = 4.0 * MCHSQR / MPASQR * (1.0 - XPHOTO) / pow(1.0 - XPHOTO + (MCHSQR - MNESQR) / MPASQR, 2);
    if (ME == 1) {
      S1 = MPASQR  * (1.0 - XPHOTO);
      beta0 = 2 * PHOTRI(1.0, sqrt(MCHSQR / MPASQR), sqrt(MNESQR / MPASQR));
      beta1 = 2 * PHOTRI(1.0, sqrt(MCHSQR / S1), sqrt(MNESQR / S1));
      wt1 = (1.0 - COSTHG * sqrt(1.0 - MCHREN))
            / ((1.0 + pow(1.0 - XPHOTO / XPHMAX, 2)) / 2.0) * XPHOTO; // de-presampler

      wt2 = beta1 / beta0 * XPHOTO;                                                   //phase space jacobians
      wt3 =  beta1 * beta1 * (1.0 - COSTHG * COSTHG) * (1.0 - XPHOTO) / XPHOTO / XPHOTO
             / pow(1.0 + MCHSQR / S1 - MNESQR / S1 - beta1 * COSTHG, 2) / 2.0; // matrix element
    } else if (ME == 2) {
      S1 = MPASQR  * (1.0 - XPHOTO);
      beta0 = 2 * PHOTRI(1.0, sqrt(MCHSQR / MPASQR), sqrt(MNESQR / MPASQR));
      beta1 = 2 * PHOTRI(1.0, sqrt(MCHSQR / S1), sqrt(MNESQR / S1));
      wt1 = (1.0 - COSTHG * sqrt(1.0 - MCHREN))
            / ((1.0 + pow(1.0 - XPHOTO / XPHMAX, 2)) / 2.0) * XPHOTO; // de-presampler

      wt2 = beta1 / beta0 * XPHOTO;                             // phase space jacobians

      wt3 = beta1 * beta1 * (1.0 - COSTHG * COSTHG) * (1.0 - XPHOTO) / XPHOTO / XPHOTO // matrix element
            / pow(1.0 + MCHSQR / S1 - MNESQR / S1 - beta1 * COSTHG, 2) / 2.0 ;
      wt3 = wt3 * (1 - XPHOTO / XPHMAX + 0.5 * pow(XPHOTO / XPHMAX, 2)) / (1 - XPHOTO / XPHMAX);
      //       print*,"wt3=",wt3
      phocorwt.phocorwt3 = wt3;
      phocorwt.phocorwt2 = wt2;
      phocorwt.phocorwt1 = wt1;

      //       YY=0.5D0*(1.D0-XPHOTO/XPHMAX+1.D0/(1.D0-XPHOTO/XPHMAX))
      //       phwt.beta=SQRT(1.D0-XX)
      //       wt1=(1.D0-COSTHG*SQRT(1.D0-MCHREN))/(1.D0-COSTHG*phwt.beta)
      //       wt2=(1.D0-XX/YY/(1.D0-phwt.beta**2*COSTHG**2))*(1.D0+COSTHG*phwt.beta)/2.D0
      //       wt3=1.D0
    } else if ((ME == 3) || (ME == 4) || (ME == 5)) {
      YY = 1.0;
      phwt.beta = sqrt(1.0 - XX);
      wt1 = (1.0 - COSTHG * sqrt(1.0 - MCHREN)) / (1.0 - COSTHG * phwt.beta);
      wt2 = (1.0 - XX / YY / (1.0 - phwt.beta * phwt.beta * COSTHG * COSTHG)) * (1.0 + COSTHG * phwt.beta) / 2.0;
      wt3 = (1.0 + pow(1.0 - XPHOTO / XPHMAX, 2) - pow(XPHOTO / XPHMAX, 3)) /
            (1.0 + pow(1.0 - XPHOTO / XPHMAX, 2));
    } else {
      DATA = (ME - 1.0) / 2.0;
      PHOERR(6, "PHOCORN", DATA);
      YY = 1.0;
      phwt.beta = sqrt(1.0 - XX);
      wt1 = (1.0 - COSTHG * sqrt(1.0 - MCHREN)) / (1.0 - COSTHG * phwt.beta);
      wt2 = (1.0 - XX / YY / (1.0 - phwt.beta * phwt.beta * COSTHG * COSTHG)) * (1.0 + COSTHG * phwt.beta) / 2.0;
      wt3 = 1.0;
    }
    wt2 = wt2 * PHOFAC(1);
    PHOCOR = wt1 * wt2 * wt3;

    phopro.corwt = PHOCOR;
    if (PHOCOR > 1.0) {
      DATA = PHOCOR;
      PHOERR(3, "PHOCOR", DATA);
    }
    return PHOCOR;
  }





//----------------------------------------------------------------------
//
//    PHOTOS:   PHOton radiation in decays CORrection weight from
//              matrix elements
//
//    Purpose:  Calculate  photon  angle.  The reshaping functions  will
//              have  to  depend  on the spin S of the charged particle.
//              We define:  ME = 2 * S + 1 !
//
//    Input Parameters:  MPASQR:  Parent mass squared,
//                       MCHREN:  Renormalised mass of charged system,
//                       ME:      2 * spin + 1 determines matrix element
//
//    Output Parameter:  Function value.
//
//    Author(s):  Z. Was, B. van Eijk             Created at:  26/11/89
//                                                Last Update: 21/03/93
//
//----------------------------------------------------------------------

  double  PHOCOR(double MPASQR, double MCHREN, int ME)
  {
    double XX, YY, DATA;
    double PHOC;
    int IscaNLO;
    double& COSTHG = phophs.costhg;
    double& XPHMAX = phophs.xphmax;
    double& XPHOTO = phophs.xphoto;
    double& MCHSQR = phomom.mchsqr;
    double& MNESQR = phomom.mnesqr;


    //--
    //--   Shaping (modified by ZW)...
    XX = 4.0 * MCHSQR / MPASQR * (1.0 - XPHOTO) / pow((1.0 - XPHOTO + (MCHSQR - MNESQR) / MPASQR), 2);
    if (ME == 1) {
      YY = 1.0;
      phwt.wt3 = (1.0 - XPHOTO / XPHMAX) / ((1.0 + pow((1.0 - XPHOTO / XPHMAX), 2)) / 2.0);
    } else if (ME == 2) {
      YY = 0.5 * (1.0 - XPHOTO / XPHMAX + 1.0 / (1.0 - XPHOTO / XPHMAX));
      phwt.wt3 = 1.0;
    } else if ((ME == 3) || (ME == 4) || (ME == 5)) {
      YY = 1.0;
      phwt.wt3 = (1.0 + pow(1.0 - XPHOTO / XPHMAX, 2) - pow(XPHOTO / XPHMAX, 3)) /
                 (1.0 + pow(1.0 - XPHOTO / XPHMAX, 2));
    } else {
      DATA = (ME - 1.0) / 2.0;
      PHOERR(6, "PHOCOR", DATA);
      YY = 1.0;
      phwt.wt3 = 1.0;
    }


    phwt.beta = sqrt(1.0 - XX);
    phwt.wt1 = (1.0 - COSTHG * sqrt(1.0 - MCHREN)) / (1.0 - COSTHG * phwt.beta);
    phwt.wt2 = (1.0 - XX / YY / (1.0 - phwt.beta * phwt.beta * COSTHG * COSTHG)) * (1.0 + COSTHG * phwt.beta) / 2.0;


    IscaNLO = Photos::meCorrectionWtForScalar;
    if (ME == 1 && IscaNLO == 1) { // this  switch NLO in scalar decays.
      // overrules default calculation.
      // Need tests including basic ones
      PHOC = PHOCORN(MPASQR, MCHREN, ME);
      phwt.wt1 = 1.0;
      phwt.wt2 = 1.0;
      phwt.wt3 = PHOC;
    } else {
      phwt.wt2 = phwt.wt2 * PHOFAC(1);
    }
    PHOC = phwt.wt1 * phwt.wt2 * phwt.wt3;

    phopro.corwt = PHOC;
    if (PHOC > 1.0) {
      DATA = PHOC;
      PHOERR(3, "PHOCOR", DATA);
    }
    return PHOC;
  }


//----------------------------------------------------------------------
//
//    PHOTWO:   PHOtos but TWO mothers allowed
//
//    Purpose:  Combines two mothers into one in /PHOEVT/
//              necessary eg in case of g g (q qbar) --> t tbar
//
//    Input Parameters: Common /PHOEVT/ (/PHOCMS/)
//
//    Output Parameters:  Common /PHOEVT/, (stored mothers)
//
//    Author(s):  Z. Was                          Created at:  5/08/93
//                                                Last Update:10/08/93
//
//----------------------------------------------------------------------

  void PHOTWO(int MODE)
  {

    int I;
    static int i = 1;
    double MPASQR;
    bool  IFRAD;
    // logical IFRAD is used to tag cases when two mothers may be
    // merged to the sole one.
    // So far used in case:
    //                      1) of t tbar production
    //
    // t tbar case
    if (MODE == 0) {
      IFRAD = (pho.idhep[1 - i] == 21) && (pho.idhep[2 - i] == 21);
      IFRAD = IFRAD || (pho.idhep[1 - i] == -pho.idhep[2 - i] && abs(pho.idhep[1 - i]) <= 6);
      IFRAD = IFRAD && (abs(pho.idhep[3 - i]) == 6) && (abs(pho.idhep[4 - i]) == 6);
      MPASQR =  pow(pho.phep[1 - i][4 - i] + pho.phep[2 - i][4 - i], 2) - pow(pho.phep[1 - i][3 - i] + pho.phep[2 - i][3 - i], 2)
                - pow(pho.phep[1 - i][2 - i] + pho.phep[2 - i][2 - i], 2) - pow(pho.phep[1 - i][1 - i] + pho.phep[2 - i][1 - i], 2);
      IFRAD = IFRAD && (MPASQR > 0.0);
      if (IFRAD) {
        //.....combining first and second mother
        for (I = 1; I <= 4; I++) {
          pho.phep[1 - i][I - i] = pho.phep[1 - i][I - i] + pho.phep[2 - i][I - i];
        }
        pho.phep[1 - i][5 - i] = sqrt(MPASQR);
        //.....removing second mother,
        for (I = 1; I <= 5; I++) {
          pho.phep[2 - i][I - i] = 0.0;
        }
      }
    } else {
      // boosting of the mothers to the reaction frame not implemented yet.
      // to do it in mode 0 original mothers have to be stored in new comon (?)
      // and in mode 1 boosted to cms.
    }
  }



//----------------------------------------------------------------------
//
//    PHOTOS:   PHOtos CDE-s
//
//    Purpose:  Keep definitions  for PHOTOS QED correction Monte Carlo.
//
//    Input Parameters:   None
//
//    Output Parameters:  None
//
//    Author(s):  Z. Was, B. van Eijk             Created at:  29/11/89
//                                                Last Update: 10/08/93
//
// =========================================================
//    General Structure Information:                       =
// =========================================================
//:   ROUTINES:
//             1) INITIALIZATION (all in C++ now)
//             2) GENERAL INTERFACE:
//                                      PHOBOS
//                                      PHOIN
//                                      PHOTWO (specific interface
//                                      PHOOUT
//                                      PHOCHK
//                                      PHTYPE (specific interface
//                                      PHOMAK (specific interface
//             3) QED PHOTON GENERATION:
//                                      PHINT
//                                      PHOBW
//                                      PHOPRE
//                                      PHOOMA
//                                      PHOENE
//                                      PHOCOR
//                                      PHOFAC
//                                      PHODO
//             4) UTILITIES:
//                                      PHOTRI
//                                      PHOAN1
//                                      PHOAN2
//                                      PHOBO3
//                                      PHORO2
//                                      PHORO3
//                                      PHOCHA
//                                      PHOSPI
//                                      PHOERR
//                                      PHOREP
//                                      PHLUPA
//                                      PHCORK
//                                      IPHQRK
//                                      IPHEKL
//   COMMONS:
//   NAME     USED IN SECT. # OF OC//     Comment
//   PHOQED   1) 2)            3      Flags whether emisson to be gen.
//   PHOLUN   1) 4)            6      Output device number
//   PHOCOP   1) 3)            4      photon coupling & min energy
//   PHPICO   1) 3) 4)         5      PI & 2*PI
//   PHOSTA   1) 4)            3      Status information
//   PHOKEY   1) 2) 3)         7      Keys for nonstandard application
//   PHOVER   1)               1      Version info for outside
//   HEPEVT   2)               2      PDG common
//   PH_HEPEVT2)               8      PDG common internal
//   PHOEVT   2) 3)           10      PDG branch
//   PHOIF    2) 3)            2      emission flags for PDG branch
//   PHOMOM   3)               5      param of char-neutr system
//   PHOPHS   3)               5      photon momentum parameters
//   PHOPRO   3)               4      var. for photon rep. (in branch)
//   PHOCMS   2)               3      parameters of boost to branch CMS
//   PHNUM    4)               1      event number from outside
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//
//    PHOIN:   PHOtos INput
//
//    Purpose:  copies IP branch of the common /PH_HEPEVT/ into /PHOEVT/
//              moves branch into its CMS system.
//
//    Input Parameters:       IP:  pointer of particle starting branch
//                                 to be copied
//                        BOOST:   Flag whether boost to CMS was or was
//     .                          replace stri  not performed.
//
//    Output Parameters:  Commons: /PHOEVT/, /PHOCMS/
//
//    Author(s):  Z. Was                          Created at:  24/05/93
//                                                Last Update: 16/11/93
//
//----------------------------------------------------------------------
  void PHOIN(int IP, bool* BOOST, int* NHEP0)
  {
    int FIRST, LAST, I, LL, IP2, J, NA;
    double PB;
    static int i = 1;
    int& nhep0 = *NHEP0;
    // double &BET[3]=BET;
    double& GAM = phocms.gam;
    double* BET = phocms.bet;

    //--
    // let-s calculate size of the little common entry
    FIRST = hep.jdahep[IP - i][1 - i];
    LAST = hep.jdahep[IP - i][2 - i];
    pho.nhep = 3 + LAST - FIRST + hep.nhep - nhep0;
    pho.nevhep = pho.nhep;

    // let-s take in decaying particle
    pho.idhep[1 - i] = hep.idhep[IP - i];
    pho.jdahep[1 - i][1 - i] = 3;
    pho.jdahep[1 - i][2 - i] = 3 + LAST - FIRST;
    for (I = 1; I <= 5; I++) pho.phep[1 - i][I - i] = hep.phep[IP - i][I - i];

    // let-s take in eventual second mother
    IP2 = hep.jmohep[hep.jdahep[IP - i][1 - i] - i][2 - i];
    if ((IP2 != 0) && (IP2 != IP)) {
      pho.idhep[2 - i] = hep.idhep[IP2 - i];
      pho.jdahep[2 - i][1 - i] = 3;
      pho.jdahep[2 - i][2 - i] = 3 + LAST - FIRST;
      for (I = 1; I <= 5; I++)
        pho.phep[2 - i][I - i] = hep.phep[IP2 - i][I - i];
    } else {
      pho.idhep[2 - i] = 0;
      for (I = 1; I <= 5; I++)  pho.phep[2 - i][I - i] = 0.0;
    }

    // let-s take in daughters
    for (LL = 0; LL <= LAST - FIRST; LL++) {
      pho.idhep[3 + LL - i] = hep.idhep[FIRST + LL - i];
      pho.jmohep[3 + LL - i][1 - i] = hep.jmohep[FIRST + LL - i][1 - i];
      if (hep.jmohep[FIRST + LL - i][1 - i] == IP) pho.jmohep[3 + LL - i][1 - i] = 1;
      for (I = 1; I <= 5; I++) pho.phep[3 + LL - i][I - i] = hep.phep[FIRST + LL - i][I - i];

    }
    if (hep.nhep > nhep0) {
      // let-s take in illegitimate daughters
      NA = 3 + LAST - FIRST;
      for (LL = 1; LL <= hep.nhep - nhep0; LL++) {
        pho.idhep[NA + LL - i] = hep.idhep[nhep0 + LL - i];
        pho.jmohep[NA + LL - i][1 - i] = hep.jmohep[nhep0 + LL - i][1 - i];
        if (hep.jmohep[nhep0 + LL - i][1 - i] == IP) pho.jmohep[NA + LL - i][1 - i] = 1;
        for (I = 1; I <= 5; I++) pho.phep[NA + LL - i][I - i] = hep.phep[nhep0 + LL - i][I - i];

      }
      //--        there is hep.nhep-nhep0 daugters more.
      pho.jdahep[1 - i][2 - i] = 3 + LAST - FIRST + hep.nhep - nhep0;
    }
    if (pho.idhep[pho.nhep - i] == 22) PHLUPA(100001);
    // if (pho.idhep[pho.nhep-i]==22) exit(-1);
    PHCORK(0);
    if (pho.idhep[pho.nhep - i] == 22) PHLUPA(100002);

    // special case of t tbar production process
    if (phokey.iftop) PHOTWO(0);
    *BOOST = false;

    //--   Check whether parent is in its rest frame...
    // ZBW ND  27.07.2009:
    // bug reported by Vladimir Savinov localized and fixed.
    // protection against rounding error was back-firing if soft
    // momentum of mother was physical. Consequence was that PHCORK was
    // messing up masses of final state particles in vertex of the decay.
    // Only configurations with previously generated photons of energy fraction
    // smaller than 0.0001 were affected. Effect was numerically insignificant.

    //      IF (     (ABS(pho.phep[4,1)-pho.phep[5,1)).GT.pho.phep[5,1)*1.D-8)
    //     $    .AND.(pho.phep[5,1).NE.0))                            THEN

    if ((fabs(pho.phep[1 - i][1 - i] + fabs(pho.phep[1 - i][2 - i]) + fabs(pho.phep[1 - i][3 - i])) >
         pho.phep[1 - i][5 - i] * 1.E-8) && (pho.phep[1 - i][5 - i] != 0)) {

      *BOOST = true;
      //PHOERR(404,"PHOIN",1.0);  // we need to improve this warning:  program should never
      // enter this place
      //  may be   exit(-1);
      //--
      //--   Boost daughter particles to rest frame of parent...
      //--   Resultant neutral system already calculated in rest frame !
      for (J = 1; J <= 3; J++) BET[J - i] = -pho.phep[1 - i][J - i] / pho.phep[1 - i][5 - i];
      GAM = pho.phep[1 - i][4 - i] / pho.phep[1 - i][5 - i];
      for (I = pho.jdahep[1 - i][1 - i]; I <= pho.jdahep[1 - i][2 - i]; I++) {
        PB = BET[1 - i] * pho.phep[I - i][1 - i] + BET[2 - i] * pho.phep[I - i][2 - i] + BET[3 - i] * pho.phep[I - i][3 - i];
        for (J = 1; J <= 3;
             J++)   pho.phep[I - i][J - i] = pho.phep[I - i][J - i] + BET[J - i] * (pho.phep[I - i][4 - i] + PB / (GAM + 1.0));
        pho.phep[I - i][4 - i] = GAM * pho.phep[I - i][4 - i] + PB;
      }
      //--    Finally boost mother as well
      I = 1;
      PB = BET[1 - i] * pho.phep[I - i][1 - i] + BET[2 - i] * pho.phep[I - i][2 - i] + BET[3 - i] * pho.phep[I - i][3 - i];
      for (J = 1; J <= 3; J++) pho.phep[I - i][J - i] = pho.phep[I - i][J - i] + BET[J - i] * (pho.phep[I - i][4 - i] + PB / (GAM + 1.0));

      pho.phep[I - i][4 - i] = GAM * pho.phep[I - i][4 - i] + PB;
    }


    // special case of t tbar production process
    if (phokey.iftop) PHOTWO(1);
    PHLUPA(2);
    if (pho.idhep[pho.nhep - i] == 22) PHLUPA(10000);
    //if (pho.idhep[pho.nhep-1-i]==22) exit(-1);  // this is probably form very old times ...
    return;
  }


//----------------------------------------------------------------------
//
//    PHOOUT:   PHOtos OUTput
//
//    Purpose:  copies back IP branch of the common /PH_HEPEVT/ from
//              /PHOEVT/ moves branch back from its CMS system.
//
//    Input Parameters:       IP:  pointer of particle starting branch
//                                 to be given back.
//                        BOOST:   Flag whether boost to CMS was or was
//     .                            not performed.
//
//    Output Parameters:  Common /PHOEVT/,
//
//    Author(s):  Z. Was                          Created at:  24/05/93
//                                                Last Update:
//
//----------------------------------------------------------------------
  void PHOOUT(int IP, bool BOOST, int nhep0)
  {
    int LL, FIRST, LAST, I;
    int NN, J, K, NA;
    double PB;
    double& GAM = phocms.gam;
    double* BET = phocms.bet;

    static int i = 1;
    if (pho.nhep == pho.nevhep) return;
    //--   When parent was not in its rest-frame, boost back...
    PHLUPA(10);
    if (BOOST) {
      //PHOERR(404,"PHOOUT",1.0);  // we need to improve this warning:  program should never
      // enter this place

      double phocms_check = fabs(1 - GAM) + fabs(BET[1 - i]) + fabs(BET[2 - i]) + fabs(BET[3 - i]);
      if (phocms_check > 0.001) {
        Log::Error() << "Msg. from PHOOUT: possible problems with boosting due to the rounding errors." << endl
                     << "Boost parameters:   (" << GAM << ","
                     << BET[1 - i] << "," << BET[2 - i] << "," << BET[3 - i] << ")" << endl
                     << "should be equal to: (1,0,0,0) up to at least several digits." << endl;
      } else {
        Log::Warning() << "Msg. from PHOOUT: possible problems with boosting due to the rounding errors." << endl
                       << "Boost parameters:   (" << GAM << ","
                       << BET[1 - i] << "," << BET[2 - i] << "," << BET[3 - i] << ")" << endl
                       << "should be equal to: (1,0,0,0) up to at least several digits." << endl;
      }

      for (J = pho.jdahep[1 - i][1 - i]; J <= pho.jdahep[1 - i][2 - i]; J++) {
        PB = -BET[1 - i] * pho.phep[J - i][1 - i] - BET[2 - i] * pho.phep[J - i][2 - i] - BET[3 - i] * pho.phep[J - i][3 - i];
        for (K = 1; K <= 3; K++) pho.phep[J - i][K - i] = pho.phep[J - i][K - i] - BET[K - i] * (pho.phep[J - i][4 - i] + PB / (GAM + 1.0));
        pho.phep[J - i][4 - i] = GAM * pho.phep[J - i][4 - i] + PB;
      }

      //--   ...boost photon, or whatever else has shown up
      for (NN = pho.nevhep + 1; NN <= pho.nhep; NN++) {
        PB = -BET[1 - i] * pho.phep[NN - i][1 - i] - BET[2 - i] * pho.phep[NN - i][2 - i] - BET[3 - i] * pho.phep[NN - i][3 - i];
        for (K = 1; K <= 3;
             K++) pho.phep[NN - i][K - i] = pho.phep[NN - i][K - i] - BET[K - i] * (pho.phep[NN - i][4 - i] + PB / (GAM + 1.0));
        pho.phep[NN - i][4 - i] = GAM * pho.phep[NN][4 - i] + PB;
      }
    }
    PHCORK(0);   // we have to use it because it clears input
    // for grandaughters modified in C++
    FIRST = hep.jdahep[IP - i][1 - i];
    LAST = hep.jdahep[IP - i][2 - i];
    // let-s take in original daughters
    for (LL = 0; LL <= LAST - FIRST; LL++) {
      hep.idhep[FIRST + LL - i] = pho.idhep[3 + LL - i];
      for (I = 1; I <= 5; I++) hep.phep[FIRST + LL - i][I - i] = pho.phep[3 + LL - i][I - i];
    }

    // let-s take newcomers to the end of HEPEVT.
    NA = 3 + LAST - FIRST;
    for (LL = 1; LL <= pho.nhep - NA; LL++) {
      hep.idhep[nhep0 + LL - i] = pho.idhep[NA + LL - i];
      hep.isthep[nhep0 + LL - i] = pho.isthep[NA + LL - i];
      hep.jmohep[nhep0 + LL - i][1 - i] = IP;
      hep.jmohep[nhep0 + LL - i][2 - i] = hep.jmohep[hep.jdahep[IP - i][1 - i] - i][2 - i];
      hep.jdahep[nhep0 + LL - i][1 - i] = 0;
      hep.jdahep[nhep0 + LL - i][2 - i] = 0;
      for (I = 1; I <= 5; I++) hep.phep[nhep0 + LL - i][I - i] = pho.phep[NA + LL - i][I - i];
    }
    hep.nhep = hep.nhep + pho.nhep - pho.nevhep;
    PHLUPA(20);
    return;
  }

//----------------------------------------------------------------------
//
//    PHOCHK:   checking branch.
//
//    Purpose:  checks whether particles in the common block /PHOEVT/
//              can be served by PHOMAK.
//              JFIRST is the position in /PH_HEPEVT/ (!) of the first
//              daughter of sub-branch under action.
//
//
//    Author(s):  Z. Was                           Created at: 22/10/92
//                                                Last Update: 11/12/00
//
//----------------------------------------------------------------------
//     ********************

  void PHOCHK(int JFIRST)
  {

    int IDABS, NLAST, I;
    bool IFRAD;
    int IDENT, K;
    static int i = 1, IPPAR = 1;

    NLAST = pho.nhep;
    //

    for (I = IPPAR; I <= NLAST; I++) {
      IDABS    = abs(pho.idhep[I - i]);
      // possibly call on PHZODE is a dead (to be omitted) code.
      pho.qedrad[I - i] = pho.qedrad[I - i] && F(0, IDABS)  && F(0, abs(pho.idhep[1 - i]))
                          && (pho.idhep[2 - i] == 0);

      if (I > 2) pho.qedrad[I - i] = pho.qedrad[I - i] && hep.qedrad[JFIRST + I - IPPAR - 2 - i];
    }

    //--
    // now we go to special cases, where pho.qedrad[I) will be overwritten
    //--
    IDENT = pho.nhep;
    if (phokey.iftop) {
      // special case of top pair production
      for (K = pho.jdahep[1 - i][2 - i]; K >= pho.jdahep[1 - i][1 - i]; K--) {
        if (pho.idhep[K - i] != 22) {
          IDENT = K;
          break;   // from loop over K
        }
      }

      IFRAD = ((pho.idhep[1 - i] == 21)      && (pho.idhep[2 - i] == 21))
              || ((abs(pho.idhep[1 - i]) <= 6)  && (pho.idhep[2 - i] == (-pho.idhep[1 - i])));
      IFRAD = IFRAD
              && (abs(pho.idhep[3 - i]) == 6) && (pho.idhep[4 - i] == (-pho.idhep[3 - i]))
              && (IDENT == 4);
      if (IFRAD) {
        for (I = IPPAR; I <= NLAST; I++) {
          pho.qedrad[I - i] = true;
          if (I > 2) pho.qedrad[I - i] = pho.qedrad[I - i] && hep.qedrad[JFIRST + I - IPPAR - 2 - i];
        }
      }
    }
    //--
    //--
    if (phokey.iftop) {
      // special case of top decay
      for (K = pho.jdahep[1 - i][2 - i]; K >= pho.jdahep[1 - i][1 - i]; K--) {
        if (pho.idhep[K - i] != 22) {
          IDENT = K;
          break;
        }
      }
      IFRAD = ((abs(pho.idhep[1 - i]) == 6) && (pho.idhep[2 - i] == 0));
      IFRAD = IFRAD
              && (((abs(pho.idhep[3 - i]) == 24) && (abs(pho.idhep[4 - i]) == 5))
                  || ((abs(pho.idhep[3 - i]) == 5) && (abs(pho.idhep[4 - i]) == 24)))
              && (IDENT == 4);

      if (IFRAD) {
        for (I = IPPAR; I <= NLAST; I++) {
          pho.qedrad[I - i] = true;
          if (I > 2) pho.qedrad[I - i] = (pho.qedrad[I - i] && hep.qedrad[JFIRST + I - IPPAR - 2 - i]);
        }
      }
    }
    //--
    //--
    return;
  }



//----------------------------------------------------------------------
//
//    PHOTOS:   PHOton radiation in decays calculation  of photon ENErgy
//              fraction
//
//    Purpose:  Subroutine  returns  photon  energy fraction (in (parent
//              mass)/2 units) for the decay bremsstrahlung.
//
//    Input Parameters:  MPASQR:  Mass of decaying system squared,
//                       XPHCUT:  Minimum energy fraction of photon,
//                       XPHMAX:  Maximum energy fraction of photon.
//
//    Output Parameter:  MCHREN:  Renormalised mass squared,
//                       BETA:    Beta factor due to renormalisation,
//                       XPHOTO:  Photon energy fraction,
//                       XF:      Correction factor for PHOFA//
//
//    Author(s):  S. Jadach, Z. Was               Created at:  01/01/89
//                B. van Eijk, P.Golonka          Last Update: 11/07/13
//
//----------------------------------------------------------------------

  void PHOENE(double MPASQR, double* pMCHREN, double* pBETA, double* pBIGLOG, int IDENT)
  {
    double  DATA;
    double PRSOFT, PRHARD;
    double PRKILL, RRR;
    int K, IDME;
    double PRSUM;
    static int i = 1;
    double& MCHREN = *pMCHREN;
    double& BETA   = *pBETA;
    double& BIGLOG = *pBIGLOG;
    int& NCHAN = phoexp.nchan;
    double& XPHMAX = phophs.xphmax;
    double& XPHOTO = phophs.xphoto;
    double& MCHSQR = phomom.mchsqr;
    double& MNESQR = phomom.mnesqr;

    //--
    if (XPHMAX <= phocop.xphcut) {
      BETA = PHOFAC(-1);  // to zero counter, here beta is dummy
      XPHOTO = 0.0;
      return;
    }
    //--   Probabilities for hard and soft bremstrahlung...
    MCHREN = 4.0 * MCHSQR / MPASQR / pow(1.0 + MCHSQR / MPASQR, 2);
    BETA = sqrt(1.0 - MCHREN);

#ifdef VARIANTB
    // ----------- VARIANT B ------------------
    // we replace 1D0/BETA*BIGLOG with (1.0/BETA*BIGLOG+2*phokey.fint)
    // for integral of new crude
    BIGLOG = log(MPASQR / MCHSQR * (1.0 + BETA) * (1.0 + BETA) / 4.0 *
                 pow(1.0 + MCHSQR / MPASQR, 2));
    PRHARD = phocop.alpha / PI * (1.0 / BETA * BIGLOG + 2 * phokey.fint)
             * (log(XPHMAX / phocop.xphcut) - .75 + phocop.xphcut / XPHMAX - .25 * phocop.xphcut * phocop.xphcut / XPHMAX / XPHMAX);
    PRHARD = PRHARD * PHOCHA(IDENT) * PHOCHA(IDENT) * phokey.fsec;
    // ----------- END OF VARIANT B ------------------
#else
    // ----------- VARIANT A ------------------
    BIGLOG = log(MPASQR / MCHSQR * (1.0 + BETA) * (1.0 + BETA) / 4.0 *
                 pow(1.0 + MCHSQR / MPASQR, 2));
    PRHARD = phocop.alpha / PI * (1.0 / BETA * BIGLOG) *
             (log(XPHMAX / phocop.xphcut) - .75 + phocop.xphcut / XPHMAX - .25 * phocop.xphcut * phocop.xphcut / XPHMAX / XPHMAX);
    PRHARD = PRHARD * PHOCHA(IDENT) * PHOCHA(IDENT) * phokey.fsec * phokey.fint;
    //me_channel_(&IDME);
    IDME = HEPEVT_struct::ME_channel;
    //        write(*,*) 'KANALIK IDME=',IDME
    if (IDME == 0) {
      // do nothing
    }

    else if (IDME == 1) {
      PRHARD = PRHARD / (1.0 + 0.75 * phocop.alpha / PI); //  NLO
    } else if (IDME == 2) {
      // work on virtual crrections in W decay to be done.
    } else {
      cout << "problem with ME_CHANNEL  IDME= " << IDME << endl;
      exit(-1);
    }

    //----------- END OF VARIANT A ------------------
#endif
    if (phopro.irep == 0) phopro.probh = 0.0;
    PRKILL = 0.0;
    if (phokey.iexp) {         // IEXP
      NCHAN = NCHAN + 1;
      if (phoexp.expini) {  // EXPINI
        phoexp.pro[NCHAN - i] = PRHARD + 0.25 * (1.0 + phokey.fint); // we store hard photon emission prob
        //for leg NCHAN
        PRHARD = 0.0;                                              // to kill emission at initialization call
        phopro.probh = PRHARD;
      } else {             // EXPINI
        PRSUM = 0.0;
        for (K = NCHAN; K <= phoexp.NX; K++) PRSUM = PRSUM + phoexp.pro[K - i];
        PRHARD = PRHARD / PRSUM; // note that PRHARD may be smaller than
        //phoexp.pro[NCHAN) because it is calculated
        // for kinematical configuartion as is
        // (with effects of previous photons)
        PRKILL = phoexp.pro[NCHAN - i] / PRSUM - PRHARD;

      }                     // EXPINI
      PRSOFT = 1.0 - PRHARD;
    } else {                    // IEXP
      PRHARD = PRHARD * PHOFAC(0); // PHOFAC is used to control eikonal
      // formfactors for non exp version only
      // here PHOFAC(0)=1 at least now.
      phopro.probh = PRHARD;
    }                         // IEXP
    PRSOFT = 1.0 - PRHARD;
    //--
    //--   Check on kinematical bounds
    if (phokey.iexp) {
      if (PRSOFT < -5.0E-8) {
        DATA = PRSOFT;
        PHOERR(2, "PHOENE", DATA);
      }
    } else {
      if (PRSOFT < 0.1) {
        DATA = PRSOFT;
        PHOERR(2, "PHOENE", DATA);
      }
    }

    RRR = Photos::randomDouble();
    if (RRR < PRSOFT) {
      //--
      //--   No photon... (ie. photon too soft)
      XPHOTO = 0.0;
      if (RRR < PRKILL) XPHOTO = -5.0; //No photon...no further trials
    } else {
      //--
      //--   Hard  photon... (ie.  photon  hard enough).
      //--   Calculate  Altarelli-Parisi Kernel
      do {
        XPHOTO = exp(Photos::randomDouble() * log(phocop.xphcut / XPHMAX));
        XPHOTO = XPHOTO * XPHMAX;
      } while (Photos::randomDouble() > ((1.0 + pow(1.0 - XPHOTO / XPHMAX, 2)) / 2.0));
    }

    //--
    //--   Calculate parameter for PHOFAC function
    phopro.xf = 4.0 * MCHSQR * MPASQR / pow(MPASQR + MCHSQR - MNESQR, 2);
    return;
  }


//----------------------------------------------------------------------
//
//    PHOTOS:   Photon radiation in decays
//
//    Purpose:  Order (alpha) radiative corrections  are  generated  in
//              the decay of the IPPAR-th particle in the HEP-like
//              common /PHOEVT/.  Photon radiation takes place from one
//              of the charged daughters of the decaying particle IPPAR
//              WT is calculated, eventual rejection will be performed
//              later after inclusion of interference weight.
//
//    Input Parameter:    IPPAR:  Pointer   to   decaying  particle  in
//                                /PHOEVT/ and the common itself,
//
//    Output Parameters:  Common  /PHOEVT/, either  with  or  without a
//                                photon(s) added.
//                        WT      weight of the configuration
//
//    Author(s):  Z. Was, B. van Eijk             Created at:  26/11/89
//                                                Last Update: 12/07/13
//
//----------------------------------------------------------------------

  void PHOPRE(int IPARR, double* pWT, int* pNEUDAU, int* pNCHARB)
  {
    int CHAPOI[pho.nmxhep];
    double MINMAS, MPASQR, MCHREN;
    double EPS, DEL1, DEL2, DATA, BIGLOG;
    double MASSUM;
    int IP, IPPAR, I, J, ME, NCHARG, NEUPOI, NLAST;
    int IDABS;
    double WGT;
    int IDME;
    double a, b;
    double& WT = *pWT;
    int& NEUDAU = *pNEUDAU;
    int& NCHARB = *pNCHARB;
    double& COSTHG = phophs.costhg;
    double& SINTHG = phophs.sinthg;
    double& XPHOTO = phophs.xphoto;
    double& XPHMAX = phophs.xphmax;
    double* PNEUTR = phomom.pneutr;
    double& MCHSQR = phomom.mchsqr;
    double& MNESQR = phomom.mnesqr;

    static int i = 1;

    //--
    IPPAR = IPARR;
    //--   Store pointers for cascade treatement...
    IP = IPPAR;
    NLAST = pho.nhep;

    //--
    //--   Check decay multiplicity..
    if (pho.jdahep[IP - i][1 - i] == 0) return;

    //--
    //--   Loop over daughters, determine charge multiplicity

    NCHARG = 0;
    phopro.irep = 0;
    MINMAS = 0.0;
    MASSUM = 0.0;
    for (I = pho.jdahep[IP - i][1 - i]; I <= pho.jdahep[IP - i][2 - i]; I++) {
      //--
      //--
      //--   Exclude marked particles, quarks and gluons etc...
      IDABS = abs(pho.idhep[I - i]);
      if (pho.qedrad[I - pho.jdahep[IP - i][1 - i] + 3 - i]) {
        if (PHOCHA(pho.idhep[I - i]) != 0) {
          NCHARG = NCHARG + 1;
          if (NCHARG > pho.nmxhep) {
            DATA = NCHARG;
            PHOERR(1, "PHOTOS", DATA);
          }
          CHAPOI[NCHARG - i] = I;
        }
        MINMAS = MINMAS + pho.phep[I - i][5 - i] * pho.phep[I - i][5 - i];
      }
      MASSUM = MASSUM + pho.phep[I - i][5 - i];
    }

    if (NCHARG != 0) {
      //--
      //--   Check that sum of daughter masses does not exceed parent mass
      if ((pho.phep[IP - i][5 - i] - MASSUM) / pho.phep[IP - i][5 - i] > 2.0 * phocop.xphcut) {
        //--
label30:

//  do{

        for (J = 1; J <= 3; J++) PNEUTR[J - i] = -pho.phep[CHAPOI[NCHARG - i] - i][J - i];
        PNEUTR[4 - i] = pho.phep[IP - i][5 - i] - pho.phep[CHAPOI[NCHARG - i] - i][4 - i];
        //--
        //--   Calculate  invariant  mass of 'neutral' etc. systems
        MPASQR = pho.phep[IP - i][5 - i] * pho.phep[IP - i][5 - i];
        MCHSQR = pow(pho.phep[CHAPOI[NCHARG - i] - i][5 - i], 2);
        if ((pho.jdahep[IP - i][2 - i] - pho.jdahep[IP - i][1 - i]) == 1) {
          NEUPOI = pho.jdahep[IP - i][1 - i];
          if (NEUPOI == CHAPOI[NCHARG - i]) NEUPOI = pho.jdahep[IP - i][2 - i];
          MNESQR = pho.phep[NEUPOI - i][5 - i] * pho.phep[NEUPOI - i][5 - i];
          PNEUTR[5 - i] = pho.phep[NEUPOI - i][5 - i];
        } else {
          MNESQR = pow(PNEUTR[4 - i], 2) - pow(PNEUTR[1 - i], 2) - pow(PNEUTR[2 - i], 2) - pow(PNEUTR[3 - i], 2);
          MNESQR = max(MNESQR, MINMAS - MCHSQR);
          PNEUTR[5 - i] = sqrt(MNESQR);
        }

        //--
        //--   Determine kinematical limit...
        XPHMAX = (MPASQR - pow(PNEUTR[5 - i] + pho.phep[CHAPOI[NCHARG - i] - i][5 - i], 2)) / MPASQR;

        //--
        //--   Photon energy fraction...
        PHOENE(MPASQR, &MCHREN, &phwt.beta, &BIGLOG, pho.idhep[CHAPOI[NCHARG - i] - i]);
        //--

        if (XPHOTO < -4.0) {
          NCHARG = 0;               // we really stop trials
          XPHOTO = 0.0;     // in this case !!
          //--   Energy fraction not too large (very seldom) ? Define angle.
        } else if ((XPHOTO < phocop.xphcut) || (XPHOTO > XPHMAX)) {
          //--
          //--   No radiation was accepted, check  for more daughters  that may ra-
          //--   diate and correct radiation probability...
          NCHARG = NCHARG - 1;
          if (NCHARG > 0)  phopro.irep = phopro.irep + 1;
          if (NCHARG > 0) goto label30;
        } else {
          //--
          //--   Angle is generated  in  the  frame defined  by  charged vector and
          //--   PNEUTR, distribution is taken in the infrared limit...
          EPS = MCHREN / (1.0 + phwt.beta);
          //--
          //--   Calculate sin(theta) and cos(theta) from interval variables
          DEL1 = (2.0 - EPS) * pow(EPS / (2.0 - EPS), Photos::randomDouble());
          DEL2 = 2.0 - DEL1;

#ifdef VARIANTB
          // ----------- VARIANT B ------------------
          // corrections for more efiicient interference correction,
          // instead of doubling crude distribution, we add flat parallel channel
          if (Photos::randomDouble() < BIGLOG / phwt.beta / (BIGLOG / phwt.beta + 2 * phokey.fint)) {
            COSTHG = (1.0 - DEL1) / phwt.beta;
            SINTHG = sqrt(DEL1 * DEL2 - MCHREN) / phwt.beta;
          } else {
            COSTHG = -1.0 + 2 * Photos::randomDouble();
            SINTHG = sqrt(1.0 - COSTHG * COSTHG);
          }

          if (phokey.fint > 1.0) {

            WGT = 1.0 / (1.0 - phwt.beta * COSTHG);
            WGT = WGT / (WGT + phokey.fint);
            //       WGT=1.0   // ??
          } else {
            WGT = 1.0;
          }
          //
          // ----------- END OF VARIANT B ------------------
#else
          // ----------- VARIANT A ------------------
          COSTHG = (1.0 - DEL1) / phwt.beta;
          SINTHG = sqrt(DEL1 * DEL2 - MCHREN) / phwt.beta;
          WGT = 1.0;
          // ----------- END OF VARIANT A ------------------
#endif
          //--
          //--   Determine spin of  particle and construct code  for matrix element
          ME = (int)(2.0 * PHOSPI(pho.idhep[CHAPOI[NCHARG - i] - i]) + 1.0);
          //--
          //--   Weighting procedure with 'exact' matrix element, reconstruct kine-
          //--   matics for photon, neutral and charged system and update /PHOEVT/.
          //--   Find pointer to the first component of 'neutral' system
          for (I = pho.jdahep[IP - i][1 - i]; I <= pho.jdahep[IP - i][2 - i]; I++) {
            if (I != CHAPOI[NCHARG - i]) {
              NEUDAU = I;
              goto label51;   //break; // to 51
            }
          }
          //--
          //--   Pointer not found...
          DATA = NCHARG;
          PHOERR(5, "PHOKIN", DATA);
label51:

          NCHARB = CHAPOI[NCHARG - i];
          NCHARB = NCHARB - pho.jdahep[IP - i][1 - i] + 3;
          NEUDAU = NEUDAU - pho.jdahep[IP - i][1 - i] + 3;

          IDME = HEPEVT_struct::ME_channel;
          //  two options introduced temporarily.
          //  In future always PHOCOR-->PHOCORN
          //  Tests and adjustment of wts for Znlo needed.
          //  otherwise simple change. PHOCORN implements
          //  exact ME for scalar to 2 scalar decays.
          if (IDME == 2) {
            b = PHOCORN(MPASQR, MCHREN, ME);
            WT = b * WGT;
            WT = WT / (1 - XPHOTO / XPHMAX + 0.5 * pow(XPHOTO / XPHMAX, 2)) * (1 - XPHOTO / XPHMAX) / 2; // factor to go to WnloWT
          } else if (IDME == 1) {

            a = PHOCOR(MPASQR, MCHREN, ME);
            b = PHOCORN(MPASQR, MCHREN, ME);
            WT = b * WGT ;
            WT = WT * phwt.wt1 * phwt.wt2 * phwt.wt3 / phocorwt.phocorwt1 / phocorwt.phocorwt2 / phocorwt.phocorwt3; // factor to go to ZnloWT
            //        write(*,*) ' -----------'
            //        write(*,*)   phwt.wt1,' ',phwt.wt2,' ',phwt.wt3
            //        write(*,*)   phocorwt.phocorwt1,' ',phocorwt.phocorwt2,' ',phocorwt.phocorwt3
          } else {
            a = PHOCOR(MPASQR, MCHREN, ME);
            WT = a * WGT;
//          WT=b*WGT; // /(1-XPHOTO/XPHMAX+0.5*pow(XPHOTO/XPHMAX,2))*(1-XPHOTO/XPHMAX)/2;
          }



        }
      } else {
        DATA = pho.phep[IP - i][5 - i] - MASSUM;
        PHOERR(10, "PHOTOS", DATA);
      }
    }

    //--
    return;
  }


//----------------------------------------------------------------------
//
//    PHOMAK:   PHOtos MAKe
//
//    Purpose:  Single or double bremstrahlung radiative corrections
//              are generated in  the decay of the IPPAR-th particle in
//              the  HEP common /PH_HEPEVT/. Example of the use of
//              general tools.
//
//    Input Parameter:    IPPAR:  Pointer   to   decaying  particle  in
//                                /PH_HEPEVT/ and the common itself
//
//    Output Parameters:  Common  /PH_HEPEVT/, either  with  or  without
//                                particles added.
//
//    Author(s):  Z. Was,                         Created at:  26/05/93
//                                                Last Update: 29/01/05
//
//----------------------------------------------------------------------

  void PHOMAK(int IPPAR, int NHEP0)
  {

    double DATA;
    int IP, NCHARG, IDME;
    int IDUM;
    int NCHARB, NEUDAU;
    double RN, WT;
    bool BOOST;
    static int i = 1;
    //--
    IP = IPPAR;
    IDUM = 1;
    NCHARG = 0;
    //--
    PHOIN(IP, &BOOST, &NHEP0);
    PHOCHK(hep.jdahep[IP - i][1 - i]);
    WT = 0.0;
    PHOPRE(1, &WT, &NEUDAU, &NCHARB);

    if (WT == 0.0) return;
    RN = Photos::randomDouble();
    // PHODO is caling randomDouble(), thus change of series if it is moved before if
    PHODO(1, NCHARB, NEUDAU);

#ifdef VARIANTB
    // we eliminate divisions  /phokey.fint in variant B.  ???
#endif
    // get ID of channel dependent ME, ID=0 means no

    IDME = HEPEVT_struct::ME_channel;
    // corrections for matrix elements
    // controlled by IDME
    // write(*,*) 'KANALIK IDME=',IDME

    if (IDME == 0) {                                      // default

      if (phokey.interf) WT = WT * PHINT(IDUM);
      if (phokey.ifw) PHOBW(&WT);                         // extra weight for leptonic W decay
    } else if (IDME == 2) {                               // ME weight for leptonic W decay

      PhotosMEforW::PHOBWnlo(&WT);
      WT = WT * 2.0;
    } else if (IDME == 1) {                                //  ME weight for leptonic Z decay

      WT = WT * PhotosMEforZ::phwtnlo();
    } else {
      cout << "problem with ME_CHANNEL  IDME= " << IDME << endl;
      exit(-1);
    }

#ifndef VARIANTB
    WT = WT / phokey.fint; // FINT must be in variant A
#endif

    DATA = WT;
    if (WT > 1.0) PHOERR(3, "WT_INT", DATA);
    // weighting
    if (RN <= WT) {
      PHOOUT(IP, BOOST, NHEP0);
    }
    return;
  }

//----------------------------------------------------------------------
//
//    PHTYPE:   Central manadgement routine.
//
//    Purpose:   defines what kind of the
//              actions will be performed at point ID.
//
//    Input Parameters:       ID:  pointer of particle starting branch
//                                 in /PH_HEPEVT/ to be treated.
//
//    Output Parameters:  Common /PH_HEPEVT/.
//
//    Author(s):  Z. Was                          Created at:  24/05/93
//                P. Golonka                      Last Update: 27/06/04
//
//----------------------------------------------------------------------
  void PHTYPE(int ID)
  {

    int K;
    double PRSUM, ESU;
    int NHEP0;
    bool IPAIR;
    bool IPHOT;
    double RN, SUM;
    bool IFOUR;
    int& NCHAN = phoexp.nchan;

    static int i = 1;


    //--
    IFOUR =          phokey.itre; // we can make internal choice whether
    // we want 3 or four photons at most.
    IPAIR = false;
    IPAIR = Photos::IfPair;
    IPHOT = true;
    IPHOT = Photos::IfPhot;
    //--   Check decay multiplicity..
    if (hep.jdahep[ID - i][1 - i] == 0) return;
    //      if (hep.jdahep[ID-i][1-i]==hep.jdahep[ID-i][2-i]) return;
    //--
    NHEP0 = hep.nhep;

    // initialization of pho.qedrad for new event.
    // some of (old style and doubling) restrictions introduced with PHOCHK,
    // also new pairs have emissions blocked with   pho.qedrad[]
    // most of the restrictions are introduced prior decay vertex is copied
    // to struct pho.

    // Establish size for the struct pho: number of daughters + 2 places for mothers (no grandmothers)
    // This solution is  `hep.nhep hardy'. Use of hep.nhep  was perilous
    // if decaying particle (ID-i) was the first in the event. That was the case of EvtGen
    // interface. We adopt to such non-standard HepMC fill.
    // NOTE: here 'max' is used as a safety for future changes to hep or pho content.
    // TP ZW (26.09.15): Thanks to Michal Kreps and John Back

    int pho_size = max(NHEP0, (hep.jdahep[ID - i][2 - i] - hep.jdahep[ID - i][1 - i] + 1) + 2);

    for (int I = 0; I < pho_size; ++I) {
      pho.qedrad[I] = true;
    }

    double elMass = 0.000511;
    double muMass = 0.1057;
    double STRENG = 0.5;

    if (IPAIR)  {

      switch (Photos::momentumUnit) {
        case Photos::GEV:
          elMass = 0.000511;
          muMass = 0.1057;
          break;
        case Photos::MEV:
          elMass = 0.511;
          muMass = 105.7;
          break;
        default:
          Log::Error() << "GEV or MEV unit must be set for pair emission" << endl;
          break;
      };
      PHOPAR(ID, NHEP0, 11, elMass, &STRENG);
      PHOPAR(ID, NHEP0, 13, muMass, &STRENG);
    }
    //--
    if (IPHOT) {
      if (phokey.iexp) {
        phoexp.expini = true;    // Initialization/cleaning
        for (NCHAN = 1; NCHAN <= phoexp.NX; NCHAN++)
          phoexp.pro[NCHAN - i] = 0.0;
        NCHAN = 0;

        phokey.fsec = 1.0;
        PHOMAK(ID, NHEP0);         // Initialization/crude formfactors into
        // phoexp.pro[NCHAN)
        phoexp.expini = false;
        RN = Photos::randomDouble();
        PRSUM = 0.0;
        for (K = 1; K <= phoexp.NX; K++)PRSUM = PRSUM + phoexp.pro[K - i];

        ESU = exp(-PRSUM);
        // exponent for crude Poissonian multiplicity
        // distribution, will be later overwritten
        // to give probability for k
        SUM = ESU;
        // distribuant for the crude Poissonian
        // at first for k=0
        for (K = 1; K <= 100; K++) { // hard coded max (photon) multiplicity is 100
          if (RN < SUM) break;
          ESU = ESU * PRSUM / K;  // we get at K ESU=EXP(-PRSUM)*PRSUM**K/K!
          SUM = SUM + ESU;        // thus we get distribuant at K.
          NCHAN = 0;
          PHOMAK(ID, NHEP0);      // LOOPING
          if (SUM > 1.0 - phokey.expeps) break;
        }

      } else if (IFOUR) {
        //-- quatro photon emission
        phokey.fsec = 1.0;
        RN = Photos::randomDouble();
        if (RN >= 23.0 / 24.0) {
          PHOMAK(ID, NHEP0);
          PHOMAK(ID, NHEP0);
          PHOMAK(ID, NHEP0);
          PHOMAK(ID, NHEP0);
        } else if (RN >= 17.0 / 24.0) {
          PHOMAK(ID, NHEP0);
          PHOMAK(ID, NHEP0);
        } else if (RN >= 9.0 / 24.0) {
          PHOMAK(ID, NHEP0);
        } else {
        }
      } else if (phokey.itre) {
        //-- triple photon emission
        phokey.fsec = 1.0;
        RN = Photos::randomDouble();
        if (RN >= 5.0 / 6.0) {
          PHOMAK(ID, NHEP0);
          PHOMAK(ID, NHEP0);
          PHOMAK(ID, NHEP0);
        } else if (RN >= 2.0 / 6.0) {
          PHOMAK(ID, NHEP0);
        }
      } else if (phokey.isec) {
        //-- double photon emission
        phokey.fsec = 1.0;
        RN = Photos::randomDouble();
        if (RN >= 0.5) {
          PHOMAK(ID, NHEP0);
          PHOMAK(ID, NHEP0);
        }
      } else {
        //-- single photon emission
        phokey.fsec = 1.0;
        PHOMAK(ID, NHEP0);
      }
    }
    //--
    //-- lepton anti-lepton pair(s)
    // we prepare to migrate half of tries to before photons accordingly to LL
    // pho.qedrad is not yet used by PHOPAR
    if (IPAIR)  {
      PHOPAR(ID, NHEP0, 11, elMass, &STRENG);
      PHOPAR(ID, NHEP0, 13, muMass, &STRENG);
    }

// Fill Photos::EventNo in user main program to provide
// debug input for specific events, e.g.:
// if(Photos::EventNo==1331094) printf("PHOTOS: event no: %10i finished\n",Photos::EventNo);
  }

  /*----------------------------------------------------------------------

        PHOTOS:   Photon radiation in decays

        Purpose:  e+e- pairs  are  generated  in
                  the decay of the IPPAR-th particle in the HEP-like
                  common /PHOEVT/.  Radiation takes place from one
                  of the charged daughters of the decaying particle IPPAR



        Input Parameter:    IPPAR:  Pointer   to   decaying  particle  in
                                    /PHOEVT/ and the common itself,
                                    NHEP0 length of the /HEPEVT/ entry
                                    before starting any activity on this
                                    IPPAR decay.
        Output Parameters:  Common  /HEPEVT/, either  with  or  without a
                                    e+e-(s) added.


        Author(s):  Z. Was,                         Created at:  01/06/93
                                                    Last Update:

    ----------------------------------------------------------------------*/
  void PHOPAR(int IPARR, int NHEP0, int idlep, double masslep, double* pSTRENG)
  {
    double PCHAR[4], PNEU[4], PELE[4], PPOZ[4], BUF[4];
    int    IP, IPPAR, NLAST;
    bool   BOOST, JESLI;
    static int i = 1;
    IPPAR = IPARR;

    double& STRENG = *pSTRENG;

    // Store pointers for cascade treatment...
    IP    = 0;
    NLAST = pho.nhep;
    // Check decay multiplicity..
    PHOIN(IPPAR, &BOOST, &NHEP0);
    PHOCHK(pho.jdahep[IP][0]); // should be loop over all mothers?
    PHLUPA(100);

    if (pho.jdahep[IP][0] == 0) return;
    if (pho.jdahep[IP][0] == pho.jdahep[IP][1]) return;

    // Loop over charged daughters
    for (int I = pho.jdahep[IP][0] - i; I <= pho.jdahep[IP][1] - i; ++I) {


      // Skip this particle if it has no charge
      if (PHOCHA(pho.idhep[I]) == 0) continue;

      int IDABS    = abs(pho.idhep[I]);
      // at the moment the following re-definition make not much sense as constraints
      // were already checked before for  photons tries.
      // we have to come back to this when we will have pairs emitted before photons.

      pho.qedrad[I] =  pho.qedrad[I] && F(0, IDABS)  && F(0, abs(pho.idhep[1 - i]))
                       && (pho.idhep[2 - i] == 0);

      if (!pho.qedrad[I]) continue; //


      // Set  3-vectors
      for (int J = 0; J < 3; ++J) {
        PCHAR[J] = pho.phep[I][J];
        PNEU [J] = -pho.phep[I][J];
      }

      // Set energy
      PNEU[3]  = pho.phep[IP][3] - pho.phep[I][3];
      PCHAR[3] = pho.phep[I][3];
      // Set mass
      double AMCH = pho.phep[I][4];
      //here we attempt generating pair from PCHAR. One of the charged
      //decay products; that is why algorithm works in a loop.
      //PNEU is four vector of all decay products except PCHAR
      //we do not care on rare cases when two pairs could be generated
      //we assume it is negligibly rare and fourth order in alpha anyway
      //TRYPAR should take as an input electron mass.
      //then it can be used for muons.
      //  printf ("wrotki %10.7f\n",STRENG);
      /*
      double PCH[4]={0};
      double PNEu[4]={0};
      double CC1;
      double CC2;

      for(int K = 0; K<4; ++K) {
       PCH[K]=PCHAR[K];
       PNEu[K]=PNEU[K];
      }
      */
      //        printf ("idlep,pdgidid= %10i %10i\n",idlep,pho.idhep[I]);

      // arrangements for the case when emitted lept6ons have
      // the same flavour as emitters
      bool sameflav = abs(idlep) == abs(pho.idhep[I]);
      int idsign = 1;
      if (pho.idhep[I] < 0) idsign = -1; // this is to ensure
      //that first lepton has the same PDGID as emitter

      trypar(&JESLI, &STRENG, AMCH, masslep, PCHAR, PNEU, PELE, PPOZ, &sameflav);
      //  printf ("rowerek %10.7f\n",STRENG);
      //emitted pair four momenta are stored in PELE PPOZ
      //then JESLI=.true.
      /*
      if (JESLI) {
      // printf ("PCHAR   %10.7f %10.7f  %10.7f  %10.7f\n",PCHAR[0],PCHAR[1],PCHAR[2],PCHAR[3]);
      //printf ("PNEU    %10.7f %10.7f  %10.7f  %10.7f\n",PNEU[0],PNEU[1],PNEU[2],PNEU[3]);

      // printf ("PNEu    %10.7f %10.7f  %10.7f  %10.7f\n",PNEu[0],PNEu[1],PNEu[2],PNEu[3]);

      printf ("PELE    %10.7f %10.7f  %10.7f  %10.7f\n",PELE[0],PELE[1],PELE[2],PELE[3]);
      printf ("PPOZ    %10.7f %10.7f  %10.7f  %10.7f\n",PPOZ[0],PPOZ[1],PPOZ[2],PPOZ[3]);
      printf ("-----------------\n");
      printf ("PCH     %10.7f %10.7f  %10.7f  %10.7f\n",PCH[0],PCH[1],PCH[2],PCH[3]);
      CC1=(PELE[0]*PCHAR[0]+PELE[1]*PCHAR[1]+PELE[2]*PCHAR[2])/sqrt(PELE[0]*PELE[0]+PELE[1]*PELE[1]+PELE[2]*PELE[2])/sqrt(PCHAR[0]*PCHAR[0]+PCHAR[1]*PCHAR[1]+PCHAR[2]*PCHAR[2]);
      CC2=(PPOZ[0]*PCHAR[0]+PPOZ[1]*PCHAR[1]+PPOZ[2]*PCHAR[2])/sqrt(PPOZ[0]*PPOZ[0]+PPOZ[1]*PPOZ[1]+PPOZ[2]*PPOZ[2])/sqrt(PCHAR[0]*PCHAR[0]+PCHAR[1]*PCHAR[1]+PCHAR[2]*PCHAR[2]);

      printf ("-=================-\n");

      }
      */
      // If JESLI = true, we modify old particles of the vertex
      if (JESLI) {
        PHLUPA(1010);

        // we have to correct 4-momenta
        // of all decay products
        // we use PARTRA for that
        // PELE PPOZ are in right frame
        for (int J = pho.jdahep[IP][0] - i; J <= pho.jdahep[IP][1] - i; ++J) {
          for (int K = 0; K < 4; ++K) {
            BUF[K] = pho.phep[J][K];
          }
          if (J == I) partra(1, BUF);
          else        partra(-1, BUF);

          for (int K = 0; K < 4; ++K) {
            pho.phep[J][K] = BUF[K];
          }
          /*
          if (J == I){
            printf ("PCHar   %10.7f %10.7f  %10.7f  %10.7f\n",pho.phep[J][0],pho.phep[J][1],pho.phep[J][2],pho.phep[J][3]);
            printf ("c1=  %10.7f\n",CC1);
            printf ("c2=  %10.7f\n",CC2);
            printf ("-=#####################################====-\n");
          }
          */
        }

        PHLUPA(1011);

        if (darkr.ifspecial == 1) {
          // virtual adding to vertex
          pho.nhep = pho.nhep + 1;
          pho.isthep[pho.nhep - i] = 2;
          pho.idhep [pho.nhep - i] = darkr.IDspecial;
          pho.jmohep[pho.nhep - i][0] = IP;
          pho.jmohep[pho.nhep - i][1] = 0;
          pho.jdahep[pho.nhep - i][0] = 0;
          pho.jdahep[pho.nhep - i][1] = 0;
          pho.qedrad[pho.nhep - i] = false;

          pho.phep[pho.nhep - i][4] = sqrt(-(PELE[0] + PPOZ[0]) * (PELE[0] + PPOZ[0])
                                           - (PELE[1] + PPOZ[1]) * (PELE[1] + PPOZ[1])
                                           - (PELE[2] + PPOZ[2]) * (PELE[2] + PPOZ[2])
                                           + (PELE[3] + PPOZ[3]) * (PELE[3] + PPOZ[3]));
        }
        //double life=darkr.SpecialLife;
        double life = darkr.SpecialLife * (-log(Photos::randomDouble()));

        // here was missing if(darkr.ifspecial==1)   zbw:16.09.2021
        if (darkr.ifspecial == 1) {
          for (int K = 1; K <= 4; ++K) {
            pho.phep[pho.nhep - i][K - i] = PELE[K - i] + PPOZ[K - i];
            pho.vhep[pho.nhep - i][K - i] = pho.vhep[IP][K - i]
                                            + (PELE[K - i] + PPOZ[K - i]) / pho.phep[pho.nhep - i][4] * life;
          }
        }

        // electron: adding to vertex
        pho.nhep = pho.nhep + 1;
        pho.isthep[pho.nhep - i] = 1;
        pho.idhep [pho.nhep - i] = idlep * idsign;
        pho.jmohep[pho.nhep - i][0] = IP;
        pho.jmohep[pho.nhep - i][1] = 0;
        pho.jdahep[pho.nhep - i][0] = 0;
        pho.jdahep[pho.nhep - i][1] = 0;
        pho.qedrad[pho.nhep - i] = false;


        for (int K = 1; K <= 4; ++K) {
          pho.phep[pho.nhep - i][K - i] = PELE[K - i];
          if (darkr.ifspecial == 1)
            pho.vhep[pho.nhep - i][K - i] = pho.vhep[pho.nhep - i - 1][K - i];
        }

        pho.phep[pho.nhep - i][4] = masslep;

        // positron: adding
        pho.nhep = pho.nhep + 1;
        pho.isthep[pho.nhep - i] = 1;
        pho.idhep [pho.nhep - i] = -idlep * idsign;
        pho.jmohep[pho.nhep - i][0] = IP;
        pho.jmohep[pho.nhep - i][1] = 0;
        pho.jdahep[pho.nhep - i][0] = 0;
        pho.jdahep[pho.nhep - i][1] = 0;
        pho.qedrad[pho.nhep - i] = false;

        for (int K = 1; K <= 4; ++K) {
          pho.phep[pho.nhep - i][K - i] = PPOZ[K - i];
          if (darkr.ifspecial == 1)
            pho.vhep[pho.nhep - i][K - i] = pho.vhep[pho.nhep - i - 1][K - i];
        }

        // for mc-test with KORALW, mumu from mu mu emissions: BEGIN
        /*
        double RRX[2];
        for( int k=0;k<=1;k++) RRX[k]=Photos::randomDouble();

        for(int KK=0;KK<=pho.nhep-i;KK++){
        for(int KJ=KK+1;KJ<=pho.nhep-i;KJ++){
        // 1 <-> 3
        if(RRX[0]>.5&&pho.idhep[KK]==13&&pho.idhep[KJ]==13){
        for( int k=0;k<=3;k++){
        double stored=pho.phep[KK][k];
        pho.phep[KK][k]=pho.phep[KJ][k];
        pho.phep[KJ][k]=stored;
        }
        }
        // 2 <-> 4

        if(RRX[1]>.5&&pho.idhep[KK]==-13&&pho.idhep[KJ]==-13){
        for( int k=0;k<=3;k++){
        double stored=pho.phep[KK][k];
        pho.phep[KK][k]=pho.phep[KJ][k];
        pho.phep[KJ][k]=stored;

        }
        }

        }
        }

        // for mc-test with KORALW, mumu from mu mu emissions: END
        */

        pho.phep[pho.nhep - i][4] = masslep;
        PHCORK(0);
        // write in
        PHLUPA(1012);
        PHOOUT(IPPAR, BOOST, NHEP0);
        PHOIN(IPPAR, &BOOST, &NHEP0);
        PHLUPA(1013);
      } // end of if (JESLI)
    } // end of loop over charged particles
  }


} // namespace Photospp

