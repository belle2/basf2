/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 *    Calculation some parameters for files that download                 *
 * at eclectronics. These parameters are discribe bit capacity            *
 * of the weighting coefficients for time and amplitude calculation       *
 *                                                                        *
 * Contributors: Alexander Bobrov (a.v.bobrov@inp.nsk.su) ,               *
 * Guglielmo De Nardo                                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <stdio.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <cstring>
#include <string>
#include <ecl/digitization/algorithms.h>
#include <ecl/digitization/WrapArray2D.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

int myPow(int x, int p)
{
  if (p == 0) return 1;
  if (p == 1) return x;
  return x * myPow(x, p - 1);
}

short  int bit_val(double f, int idd, short int bit, int a, int b, int lim)
{
  short int i;
  i = bit;

  int flag;
  flag = 0;
  if (i < 11)
    i = 11;

  do {
    const int R(myPow(2, i));
    const int val((int)(a * R * f / idd / b + R + 0.5) - R);

    if ((val < lim && val > -lim)) {
      flag = 1;
    } else {
      i = i - 1;

    }

  } while (i > 8 && flag == 0);


  if (i > bit) {i = bit;}
  return i;

}

int PhiC(int k)
{
  int R;
  if (k > -1 && k < 8736) {
    if (k < 96)R = 48;
    else if (k < 288)R = 64;
    else if (k < 864)R = 96;
    else if (k < 8064)R = 144;
    else if (k < 8544)R = 96;
    else R = 64;
  } else {R = -1;}
  return R;
}


int Rest(int k)
{
  int R;
  if (k > -1 && k < 8736) {
    if (k < 96)R = 0;
    else if (k < 288)R = 96;
    else if (k < 864)R = 288;
    else if (k < 8064)R = 864;
    else if (k < 8544)R = 8064;
    else R = 8544;
  } else {R = -1;}
  return R;
}


int Rest1(int k)
{
  int R;
  if (k > -1 && k < 8736) {
    if (k < 96)R = 0;
    else if (k < 288)R = 2;
    else if (k < 864)R = 5;
    else if (k < 8064)R = 11;
    else if (k < 8544)R = 61;
    else R = 66;
  } else {R = -1;}
  return R;
}




int ThetR(int k)
{
  int R;

  if (k > -1 && k < 8736) {
    const int l = Rest(k);
    const int b = PhiC(k);
    R = (k - l) / b + Rest1(k);

  } else
    R = -1;

  return R;
}





void bit(int num, char* inputPath)
{


  FILE* BMcoIN;

  // varible for one channel

  double ss1[16][16];

  int ChN;


  double  g1g1[192], gg[192], gg1[192];//, dgg[192]; dgg is never used (AH)
  WrapArray2D<double>  sg1(16, 192), sg(16, 192), sg2(16, 192);
  double  gg2[192], g1g2[192], g2g2[192];
  double  dgg1[192], dgg2[192];

  WrapArray2D<double> f(192, 16);
  WrapArray2D<double> f1(192, 16);
  WrapArray2D<double> fg31(192, 16);
  WrapArray2D<double> fg32(192, 16);
  WrapArray2D<double> fg33(192, 16);

  WrapArray2D<double> fg41(24, 16);
  WrapArray2D<double> fg43(24, 16);


  WrapArray2D<int> m_f(192, 16);
  WrapArray2D<int> m_f1(192, 16);
  WrapArray2D<int> m_fg31(192, 16);
  WrapArray2D<int> m_fg32(192, 16);
  WrapArray2D<int> m_fg33(192, 16);

  WrapArray2D<int> m_fg41(24, 16);
  WrapArray2D<int> m_fg43(24, 16);


  double val_f;

  memset(g1g1, 0, sizeof(g1g1));
  memset(gg, 0, sizeof(gg));
  memset(gg1, 0, sizeof(gg1));
//   memset(dgg, 0, sizeof(dgg)); dgg is never used (AH)
  memset(sg1, 0, sizeof(sg1));
  memset(sg, 0, sizeof(sg));
  memset(sg2, 0, sizeof(sg2));
  memset(gg2, 0, sizeof(gg2));
  memset(g1g2, 0, sizeof(g1g2));
  memset(g2g2, 0, sizeof(g2g2));
  memset(dgg1, 0, sizeof(dgg1));
  memset(dgg2, 0, sizeof(dgg2));


  double dt;

  double fdd;


  int j1, j, i;
  double adt, ddt, tc1;
  double tmd, tpd, ssssj, ssssj1, ssssi, ssssi1;
  double svp, svm;


  int idd;
  int i16;
  int mxf, mxf1, mxfg31, mxfg32, mxfg33, mxfg41, mxfg43;
  mxf = 0;
  mxf1 = 0;
  mxfg31 = 0;
  mxfg32 = 0;
  mxfg33 = 0;
  mxfg41 = 0;
  mxfg43 = 0;

  int k;

  short int bitf;
  short int bitf1;
  short int bitfg31;
  short int bitfg32;
  short int bitfg33;
  short int bitfg41;
  short int bitfg43;


  bitf = 16;
  bitf1 = 16;
  bitfg31 = 16;
  bitfg41 = 16;


  bitfg32 = 16;


  bitfg33 = 19;
  bitfg43 = 19;


  const int mapsize = 217;

  short int bfg31[mapsize];

  short int bfg32[mapsize];
  short int bfg33[mapsize];
  short int bfg41[mapsize];

  short int bfg43[mapsize];
  short int  bf[mapsize];
  short int bf1[mapsize];
  short int bad[mapsize];

  int badN;
  badN = 0;
  for (j = 0; j < mapsize; j++) {
    bfg31[j] = 16;
    bfg32[j] = 16;
    bfg33[j] = 19;
    bf[j] = 16;
    bf1[j] = 16;

    bfg41[j] = 16;
    bfg43[j] = 19;
    bad[j] = 0;
  }
  double xf;
  double xf1;
  double xfg31;
  double xfg32;
  double xfg33;
  double xfg41;
  double xfg43;



  xf = 0.;
  xf1 = 0.;
  xfg31 = 0.;
  xfg32 = 0.;
  xfg33 = 0.;
  xfg41 = 0.;
  xfg43 = 0.;


  int cf;
  int cf1;
  int cf31;
  int cf32;
  int cf33;
  int cf41;
  int cf43;

  cf = 12;
  cf1 = 12;
  cf31 = 12;
  cf32 = 12;
  cf33 = 12;
  cf41 = 12;
  cf43 = 12;

  dt = 0.5;


  const double ndt = 96.;
  adt = 1. / ndt;
  const int endc = 2 * ndt;


  const double dt0 = adt * dt;
  double t0 = -dt0 * ndt;




  //%%%%%%%%%%%%%%%%%%%%%%%%%55


  const double del = 0.;
  const double ts0 = 0.5;
  dt = 0.5;

  for (j1 = 0; j1 < endc; j1++) {
    t0 = t0 + dt0;
    double t = t0 - dt - del;

//     const double tin(t + dt); //tin value unused (AH)

    for (j = 0; j < 16; j++) {
      t = t + dt;

      if (t > 0) {


        f[j1][j] = ShaperDSP(t / 0.881944444);

        ddt = 0.005 * dt;
        tc1 = t - ts0;
        tpd = t + ddt;
        tmd = t - ddt;


        if (tc1 > ddt) {
          svp = ShaperDSP(tpd / 0.881944444);
          svm = ShaperDSP(tmd / 0.881944444);

          f1[j1][j] = (svp - svm) / 2. / ddt;

        } else {


          f1[j1][j] = ShaperDSP(tpd / 0.881944444) / (ddt + tc1);


        }// else tc1>ddt
        //                       if(fabs(f1[ChN][j1][j])>100.||(j==2&&j1==156)){

        if (fabs(f1[j1][j]) > 100. || (j == 2 && j1 == 156)) {

        }
      } //if t>0
      else {
        f[j1][j] = 0.;
        f1[j1][j] = 0.;

      }

    } //for j






  }




  ///%%%%%%%%%%%%%%%%%%%%%%55


  badN = 0;
  for (ChN = 0; ChN < mapsize; ChN++) {
    if (ChN % (mapsize / 50) == 0) {printf("CnN=%d badN=%d \n", ChN, badN);}
    //read matrix

    {
      string filename(inputPath);
      //filename += "/corr";
      filename += to_string(num);
      filename += "/Binmcor";
      filename += to_string(ChN);
      filename += "_L.dat";
      //      sprintf(BMin, "/hsm/belle/bdata2/users/avbobrov/belle2/corr%d/Binmcor%d_L.dat", num, ChN);

      if ((BMcoIN = fopen(filename.c_str(), "rb")) == nullptr) {
        printf(" file %s is absent \n", filename.c_str());
        exit(1);
      }

      //     2
      if (fread(ss1, sizeof(double), 256, BMcoIN) != 256) {
        printf("Error writing file %s \n", filename.c_str());
        exit(1);
      }

      fclose(BMcoIN);

    }



    // shape function parameters




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

//       dgg[j1] = gg[j1] * g1g1[j1] - gg1[j1] * gg1[j1]; dgg is never used (AH)
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


        //cold
        if (dgg1[j1] != 0) {

          fg41[j1][i] = (g2g2[j1] * sg[i][j1] - gg2[j1] * sg2[i][j1]) / dgg1[j1];
          fg43[j1][i] = (gg[j1] * sg2[i][j1] - gg2[j1] * sg[i][j1]) / dgg1[j1];


        }
        //cold


      }  // for i



    } // for j1 <endc




    //%%%%%%%%%%%%%%%%%%%adduction to integer

    constexpr int n16 = 16;
    const int ipardsp14 = 0 * 256 + 17;
    idd = ipardsp14 / 256;
    i16 = myPow(2, 15);


    for (i = 0; i < 16; i++) {
      if (i == 0) {
        idd = n16;
        fdd = 16.;
      } else {
        idd = 1;
        fdd = 1.;
      }
      for (k = 0; k < 192; k++) {

        val_f = f[k][i];
        bf[ChN] = bit_val(val_f, idd, bf[ChN],  1, 1, i16);
        if (bf[ChN] < bitf) {bitf = bf[ChN];}
        val_f = f1[k][i];

        bf1[ChN] = bit_val(val_f, idd, bf1[ChN],  4, 3, i16);
        if (bf1[ChN] < bitf1) {bitf1 = bf1[ChN];}
        val_f = fg31[k][i];
        bfg31[ChN] = bit_val(val_f, idd, bfg31[ChN],  1, 1, i16);
        if (bfg31[ChN] < bitfg31) {bitfg31 = bfg31[ChN];}
        val_f = fg32[k][i];
        bfg32[ChN] = bit_val(val_f, idd, bfg32[ChN],  3, 4, i16);
        if (bfg32[ChN] < bitfg32) {bitfg32 = bfg32[ChN];}
        val_f = fg33[k][i];
        bfg33[ChN] = bit_val(val_f, idd, bfg33[ChN],  1, 1, i16);
        if (bfg33[ChN] < bitfg33) {bitfg33 = bfg33[ChN];}
        if (k < 24) {
          val_f = fg41[k][i];
          bfg41[ChN] = bit_val(val_f, idd, bfg41[ChN],  1, 1, i16);
          if (bfg41[ChN] < bitfg41) {bitfg41 = bfg41[ChN];}
          val_f = fg43[k][i];
          bfg43[ChN] = bit_val(val_f, idd, bfg43[ChN],  1, 1, i16);
          if (bfg43[ChN] < bitfg43) {bitfg43 = bfg43[ChN];}
        }



        if (fabs(f[k][i] / fdd)    > xf)   {xf    = fabs(f[k][i] / fdd);}
        if (fabs(f1[k][i] / fdd)   > xf1)   {xf1   = fabs(f1[k][i] / fdd);}
        if (fabs(fg31[k][i] / fdd) > xfg31) {xfg31 = fabs(fg31[k][i] / fdd);}
        if (fabs(fg32[k][i] / fdd) > xfg32) {xfg32 = fabs(fg32[k][i] / fdd);}
        if (fabs(fg33[k][i] / fdd) > xfg33) {xfg33 = fabs(fg33[k][i] / fdd);}
        if (k < 24) {
          if (fabs(fg41[k][i] / fdd) > xfg41) {xfg41 = fabs(fg41[k][i] / fdd);}
          if (fabs(fg43[k][i] / fdd) > xfg43) {xfg43 = fabs(fg43[k][i] / fdd);}
        }


      } // for k


    }  // for i


    if (bf[ChN] < cf || bf1[ChN] < cf1 || bfg31[ChN] < cf31 || bfg32[ChN] < cf32 || bfg33[ChN] < cf33 || bfg41[ChN] < cf41
        || bfg43[ChN] < cf43) {
      badN = badN + 1;

      if (bf[ChN] < cf) {bad[ChN] = bad[ChN] + 1;}
      if (bf1[ChN] < cf1) {bad[ChN] = bad[ChN] + 2;}
      if (bfg31[ChN] < cf31) {bad[ChN] = bad[ChN] + 4;}
      if (bfg32[ChN] < cf32) {bad[ChN] = bad[ChN] + 8;}
      if (bfg33[ChN] < cf33) {bad[ChN] = bad[ChN] + 16;}
      if (bfg41[ChN] < cf41) {bad[ChN] = bad[ChN] + 32;}
      if (bfg43[ChN] < cf43) {bad[ChN] = bad[ChN] + 64;}

    }
  }

  //    printf("mxf=%d mxf1=%d mxfg31=%d mxfg32=%d mxfg33=%d mxfg41=%d mxfg43=%d \n",mxf,mxf1,mxfg31,mxfg32,mxfg33,mxfg41,mxfg43);


  printf("bxf=%d bxf1=%d bxfg31=%d bxfg32=%d bxfg33=%d bxfg41=%d bxfg43=%d \n", bitf, bitf1, bitfg31, bitfg32, bitfg33, bitfg41,
         bitfg43);
  printf("xf=%lf xf1=%lf xfg31=%lf xfg32=%lf xfg33=%lf xfg41=%lf xfg43=%lf \n", xf, xf1, xfg31, xfg32, xfg33, xfg41, xfg43);

  int ia = myPow(2, bitf);
  mxf = (int)(xf * ia / idd + ia + 0.5) - ia;

  ia = myPow(2, bitf1);
  mxf1 = (int)(4 * xf1 * ia / idd / 3 + ia + 0.5) - ia;
  ia = myPow(2, bitfg31);
  mxfg31 = (int)(xfg31 * ia / idd + ia + 0.5) - ia;
  ia = myPow(2, bitfg32);
  mxfg32 = (int)(3 * xfg32 * ia / idd / 4 + ia + 0.5) - ia;
  ia = myPow(2, bitfg33);
  mxfg33 = (int)(xfg33 * ia / idd + ia + 0.5) - ia;
  ia = myPow(2, bitfg41);
  mxfg41 = (int)(xfg41 * ia / idd + ia + 0.5) - ia;
  ia = myPow(2, bitfg43);
  mxfg43 = (int)(xfg43 * ia / idd + ia + 0.5) - ia;

  printf("mf=%d mf1=%d mfg31=%d mfg32=%d mfg33=%d mfg41=%d mfg43=%d \n", mxf, mxf1, mxfg31, mxfg32, mxfg33, mxfg41, mxfg43);


  printf("Rf=%lf Rf1=%lf Rfg31=%lf Rfg32=%lf Rfg33=%lf Rfg41=%lf Rfg43=%lf \n", (double)mxf / (double)i16, (double)mxf1 / (double)i16,
         (double)mxfg31 / (double)i16, (double)mxfg32 / (double)i16, (double)mxfg33 / (double)i16, (double)mxfg41 / (double)i16,
         (double)mxfg43 / (double)i16);
  printf("\n");


  i16 = myPow(2, 15);

  if (bitf == 16) {
    ia = myPow(2, bitf);
    mxf = (int)(xf * ia / idd + ia + 0.5) - ia;

    printf("f at limit %lf\n", (double)mxf / (double)i16);
    if ((double)mxf / (double)i16 > 1) {printf("wronf bit for f\n");}
  } else {
    ia = myPow(2, bitf);
    mxf = (int)(xf * ia / idd + ia + 0.5) - ia;

    printf("f %lf \n", (double)mxf / (double)i16);
    if ((double)mxf / (double)i16 > 1) {printf("wronf bit for f\n");}


    ia = myPow(2, bitf + 1);
    mxf = (int)(xf * ia / idd + ia + 0.5) - ia;

    printf("f+1  %lf\n", (double)mxf / (double)i16);

    ia = myPow(2, bitf - 1);
    mxf = (int)(xf * ia / idd + ia + 0.5) - ia;

    printf("f-1  %lf\n", (double)mxf / (double)i16);


  }


  printf("\n");

  if (bitf1 == 16) {
    ia = myPow(2, bitf1);
    mxf1 = (int)(4 * xf1 * ia / idd / 3 + ia + 0.5) - ia;

    printf("f at limit %lf\n", (double)mxf1 / (double)i16);
    if ((double)mxf / (double)i16 > 1) {printf("wronf bit for f1\n");}
  } else {
    ia = myPow(2, bitf1);
    mxf1 = (int)(4 * xf1 * ia / idd / 3 + ia + 0.5) - ia;

    printf("f1  %lf\n", (double)mxf1 / (double)i16);
    if ((double)mxf1 / (double)i16 > 1) {printf("wronf bit for f1\n");}


    ia = myPow(2, bitf1 + 1);
    mxf1 = (int)(4 * xf1 * ia / idd / 3 + ia + 0.5) - ia;

    printf("f1+1 %lf \n", (double)mxf1 / (double)i16);

    ia = myPow(2, bitf1 - 1);
    printf("ia=%d i16=%d \n", ia, i16);

    mxf1 = (int)(4 * xf1 * ia / idd / 3 + ia + 0.5) - ia;

    printf("f1-1  %lf\n", (double)mxf1 / (double)i16);


  }


  printf("\n");

  if (bitfg31 == 16) {
    ia = myPow(2, bitfg31);
    mxfg31 = (int)(xfg31 * ia / idd + ia + 0.5) - ia;

    printf("fg31 at limit %lf\n", (double)mxfg31 / (double)i16);
    if ((double)mxfg31 / (double)i16 > 1) {printf("wronf bit for fg31\n");}
  } else {
    ia = myPow(2, bitfg31);
    mxfg31 = (int)(xfg31 * ia / idd + ia + 0.5) - ia;

    printf("fg31  %lf\n", (double)mxfg31 / (double)i16);
    if ((double)mxfg31 / (double)i16 > 1) {printf("wronf bit for fg31\n");}


    ia = myPow(2, bitfg31 + 1);
    mxfg31 = (int)(xfg31 * ia / idd + ia + 0.5) - ia;

    printf("fg31+1 %lf \n", (double)mxfg31 / (double)i16);

    ia = myPow(2, bitfg31 - 1);
    mxfg31 = (int)(xfg31 * ia / idd + ia + 0.5) - ia;

    printf("fg31-1  %lf\n", (double)mxfg31 / (double)i16);


  }


  printf("\n");

  if (bitfg32 == 16) {
    ia = myPow(2, bitfg32);
    mxfg31 = (int)(3 * xfg32 * ia / idd / 4 + ia + 0.5) - ia;

    printf("fg32 at limit %lf\n", (double)mxfg32 / (double)i16);
    if ((double)mxfg32 / (double)i16 > 1) {printf("wronf bit for f32\n");}
  } else {
    ia = myPow(2, bitfg32);
    mxfg32 = (int)(3 * xfg32 * ia / idd / 4 + ia + 0.5) - ia;

    printf("fg32  %lf\n", (double)mxfg32 / (double)i16);
    if ((double)mxfg32 / (double)i16 > 1) {printf("wronf bit for f\n");}


    ia = myPow(2, bitfg32 + 1);
    mxfg31 = (int)(3 * xfg32 * ia / idd / 4 + ia + 0.5) - ia;

    printf("fg32+1 %lf \n", (double)mxfg31 / (double)i16);

    ia = myPow(2, bitfg31 - 1);
    mxfg31 = (int)(3 * xfg32 * ia / idd / 4 + ia + 0.5) - ia;

    printf("fg32-1  %lf\n", (double)mxfg31 / (double)i16);


  }




  printf("\n");

  if (bitfg33 == 19) {
    ia = myPow(2, bitfg33);
    mxfg33 = (int)(xfg33 * ia / idd + ia + 0.5) - ia;

    printf("fg33 at limit %lf\n", (double)mxfg33 / (double)i16);
    if ((double)mxfg33 / (double)i16 > 1) {printf("wronf bit for fg33\n");}
  } else {
    ia = myPow(2, bitfg33);
    mxfg33 = (int)(xfg33 * ia / idd + ia + 0.5) - ia;

    printf("fg33  %lf\n", (double)mxfg33 / (double)i16);
    if ((double)mxfg33 / (double)i16 > 1) {printf("wronf bit for fg33\n");}


    ia = myPow(2, bitfg33 + 1);
    mxfg33 = (int)(xfg33 * ia / idd + ia + 0.5) - ia;

    printf("fg33+1 %lf \n", (double)mxfg33 / (double)i16);

    ia = myPow(2, bitfg33 - 1);
    mxfg33 = (int)(xfg33 * ia / idd + ia + 0.5) - ia;

    printf("fg33-1  %lf\n", (double)mxfg33 / (double)i16);


  }


  printf("\n");


  if (bitfg41 == 16) {
    ia = myPow(2, bitfg41);
    mxfg41 = (int)(xfg41 * ia / idd + ia + 0.5) - ia;

    printf("fg41 at limit %lf\n", (double)mxfg41 / (double)i16);
    if ((double)mxfg41 / (double)i16 > 1) {printf("wronf bit for fg41\n");}
  } else {
    ia = myPow(2, bitfg41);
    mxfg41 = (int)(xfg41 * ia / idd + ia + 0.5) - ia;

    printf("fg41  %lf\n", (double)mxfg41 / (double)i16);
    if ((double)mxfg41 / (double)i16 > 1) {printf("wronf bit for fg41\n");}


    ia = myPow(2, bitfg41 + 1);
    mxfg41 = (int)(xfg41 * ia / idd + ia + 0.5) - ia;

    printf("fg41+1 %lf \n", (double)mxfg41 / (double)i16);

    ia = myPow(2, bitfg41 - 1);
    mxfg41 = (int)(xfg41 * ia / idd + ia + 0.5) - ia;

    printf("fg41-1  %lf\n", (double)mxfg41 / (double)i16);


  }


  printf("\n");

  if (bitfg43 == 19) {
    ia = myPow(2, bitfg43);
    mxfg43 = (int)(xfg43 * ia / idd + ia + 0.5) - ia;

    printf("fg43 at limit %lf\n", (double)mxfg43 / (double)i16);
    if ((double)mxfg43 / (double)i16 > 1) {printf("wronf bit for fg33\n");}
  } else {
    ia = myPow(2, bitfg43);
    mxfg43 = (int)(xfg43 * ia / idd + ia + 0.5) - ia;

    printf("fg43  %lf\n", (double)mxfg43 / (double)i16);
    if ((double)mxfg33 / (double)i16 > 1) {printf("wronf bit for fg33\n");}


    ia = myPow(2, bitfg43 + 1);
    mxfg43 = (int)(xfg43 * ia / idd + ia + 0.5) - ia;

    printf("fg43+1 %lf \n", (double)mxfg43 / (double)i16);

    ia = myPow(2, bitfg43 - 1);
    mxfg43 = (int)(xfg43 * ia / idd + ia + 0.5) - ia;

    printf("fg43-1  %lf\n", (double)mxfg43 / (double)i16);


  }




  printf("\n");

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%5

  if (bitf < 13) {printf("problem bitf=%d \n", bitf);}
  if (bitf1 < 13) {printf("problem bitf1=%d \n", bitf1);}
  if (bitfg31 < 13) {printf("problem bitfg31=%d \n", bitfg31);}
  if (bitfg32 < 13) {printf("problem bitfg32=%d \n", bitfg32);}
  if (bitfg33 < 16) {printf("problem bitfg33=%d \n", bitfg33);}

  if (bitfg41 < 13) {printf("problem bitfg41=%d \n", bitfg41);}
  if (bitfg43 < 16) {printf("problem bitfg43=%d \n", bitfg43);}


  printf("\n");

  printf("bxf=%d bxf1=%d bxfg31=%d bxfg32=%d bxfg33=%d bxfg41=%d bxfg43=%d \n", bitf, bitf1, bitfg31, bitfg32, bitfg33, bitfg41,
         bitfg43);

  printf("badN=%d \n", badN);

//   int RF[69]; RF is never used (AH)
//   int RF1[69]; RF1 is never used (AH)
  int RFG31[69];
  int RFG32[69];
  int RFG33[69];
//   int RFG41[69]; RFG41 is never used (AH)
  int RFG43[69];


  int BF[20];
  int BF1[20];
  int BFG31[20];
  int BFG32[20];
  int BFG33[20];
  int BFG41[20];
  int BFG43[20];

  for (i = 0; i < 69; i++) {
    if (i < 20) {
      BF[i] = 0;
      BF1[i] = 0;
      BFG31[i] = 0;
      BFG32[i] = 0;
      BFG32[i] = 0;
      BFG31[i] = 0;
      BFG43[i] = 0;
    }

//     RF[i] = 0; RF is never used (AH)
//     RF1[i] = 0; RF is never used (AH)
    RFG31[i] = 0;
    RFG32[i] = 0;
    RFG32[i] = 0;
    RFG31[i] = 0;
    RFG43[i] = 0;

  }

  for (j = 0; j < mapsize; j++) {

    for (i = 10; i < 20; i++) {
      if (bf[j] == i) {BF[i] = BF[i] + 1;}
      if (bf1[j] == i) {BF1[i] = BF1[i] + 1;}
      if (bfg31[j] == i) {BFG31[i] = BFG31[i] + 1;}
      if (bfg32[j] == i) {BFG32[i] = BFG32[i] + 1;}
      if (bfg33[j] == i) {BFG33[i] = BFG33[i] + 1;}
      if (bfg41[j] == i) {BFG41[i] = BFG41[i] + 1;}
      if (bfg43[j] == i) {BFG43[i] = BFG43[i] + 1;}
    }
    if (bf[j] < cf || bf1[j] < cf1 || bfg31[j] < cf31 || bfg32[j] < cf32 || bfg33[j] < cf33 || bfg41[j] < cf41 || bfg43[j] < cf43) {
      if (bfg31[j] < cf31) { //printf("fg31 ring=%d ",ThetR(j));
        RFG31[ThetR(j)] = RFG31[ThetR(j)] + 1;
      }
      if (bfg32[j] < cf32) { //printf("fg32 ring=%d ",ThetR(j));
        RFG32[ThetR(j)] = RFG32[ThetR(j)] + 1;
      }
      if (bfg33[j] < cf33) { //printf("fg33 ring=%d ",ThetR(j));
        RFG33[ThetR(j)] = RFG33[ThetR(j)] + 1;
      }
      if (bfg43[j] < cf43) { //printf("fg41 ring=%d ",ThetR(j));
        RFG43[ThetR(j)] = RFG43[ThetR(j)] + 1;
      }
      //      printf("\n");
    }
  }

  for (i = 10; i < 20; i++) {
    if (BF[i] > 0 || BF1[i] > 0 || BFG31[i] > 0 || BFG32[i] > 0 || BFG33[i] > 0 || BFG41[i] > 0 || BFG43[i] > 0) {
      printf("bit=%d BF=%d BF1=%d BFG31=%d BFG32=%d BFG33=%d BFG41=%d BFG43=%d\n", i, BF[i], BF1[i], BFG31[i], BFG32[i], BFG33[i],
             BFG41[i], BFG43[i]);
    }

  }

  char BMin[256];
  sprintf(BMin, "bitst%d.dat", num);

  if ((BMcoIN = fopen(BMin, "w")) == nullptr) {
    printf(" file %s is absent \n", BMin);
    exit(1);
  }

  for (i = 0; i < mapsize; i++) {
    fprintf(BMcoIN, "%d %d %d %d %d %d %d %d \n", i, bf[i], bf1[i], bfg31[i], bfg32[i], bfg33[i], bfg41[i], bfg43[i]);
  }
  fclose(BMcoIN);

}


int main(int argc, char** argv)
{
  assert(argc == 1 || argc == 3);
  if (argc == 1) {
    cout << "Usage:" << endl;
    cout << argv[0] << " <type> <covmat_path>" << endl;
    cout << "type is a numerical integer value to define the source of the calibration" << endl;
    cout << "covmat_path is the path to look for the directory corr<type> that contains the covariance matrix files Binmcorr<type>_L.dat"
         << endl;
    cout << " An output file with the translation to integers in written in the work directory" << endl;
  } else bit(atoi(argv[1]), argv[2]);
}
