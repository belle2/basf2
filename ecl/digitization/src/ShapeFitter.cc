/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/digitization/algorithms.h>
#include <iostream>

using namespace std;

void Belle2::ECL::shapeFitter(short int* id, int* f, int* f1, int* fg41, int* fg43, int* fg31, int* fg32, int* fg33, int* y,
                              int* ttrig2, int* n16,  int* lar, int* ltr, int* lq, int* hi2)
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
  ch1 = -1;

  ttrig = *ttrig2 / 6;


  if (k_16 + *n16 != 16) {
    cout << "disagreement in number of the points " << k_16 << "and " << *n16 << endl;
  }


  int validity_code = 0;
  for (i = ys, z00 = 0; i < 16; i++) {
    z00 += y[i];

  }
  //initial time index



  it0 = 48 + ((143 - *ttrig2) << 2) / 6;


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
  T = -2048;
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
        B2 = B1 >> (k_b - 11);
        B5 = B1 >> (k_b - 9);

        B2 += (A1 << 11);
        B3 = (B2 / A1);

        T = ((it) << 3) + ((it) << 2) + ((B3 + 1) >> 1) + B3 - 3072;

        T = ((215 - *ttrig2) << 3) - 4 - T;

        B1 = B5 >> 9;
        B5 += (A1 << 9);
        B3 = (B5 / A1);
        it += ((B3 + 1) >> 1) - 256;
        it = it > it_h ? it_h : it;
        it = it < it_l ? it_l : it;

        T = T > 2046 ?  2047 : T;

        T = T < -2046 ? -2047 : T;

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
  *hi2 = ch1;


  if (A1 < Askip)validity_code = validity_code + 8;

  int ss = (y[20] + y[21]);

  if (ss <= Ahard)validity_code = validity_code + 4;


  *lq = validity_code;


  return ;
}
