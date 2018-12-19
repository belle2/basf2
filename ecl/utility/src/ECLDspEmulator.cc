//ECL
#include <ecl/utility/ECLDspEmulator.h>
//STL
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

namespace Belle2 {
  namespace ECL {
    void lftda_(short int* f, short int* f1, short int* fg41, short int* fg43, short int* fg31, short int* fg32, short int* fg33,
                int* y, int& ttrig2, int& A0, int& Ahard, int& k_a, int& k_b, int& k_c, int& k_16, int& k1_chi, int& k2_chi, int& chi_thres,
                int& m_AmpFit, int& m_TimeFit, int& m_QualityFit)
    {
      using namespace std;
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

      int ttrig;
      int n16 = 16;
      int kz_s =  0;







      int s1, s2;

      long long int z00;
      long long int z0;


      int it, it0;
      int it_h, it_l;
      long long A1, B1, A2, C1, ch1, ch2, B2, B3, B5 ;
      int low_ampl, i, T, iter;
      ch1 = -1;
      int lch3;

      ttrig = ttrig2 / 6;
      if (ttrig < 0) ttrig = 0;


      if (k_16 + n16 != 16) {
        cout << "disagreement in number of the points " << k_16 << "and " << n16 << endl;
      }



      int validity_code = 0;
      for (i = k_16, z00 = 0; i < 16; i++) {
        z00 += y[i];

      }
      //initial time index



      it0 = 48 + ((23 - ttrig) << 2);
      z0 = z00 >> kz_s;






      it_h = 191;
      it_l = 0;
      if (it0 < it_l)it0 = it_l;
      if (it0 > it_h)it0 = it_h;
      it = it0;

      //first approximation without time correction

      //  int it00=23-it0;

      s1 = (*(fg41 + ttrig * 16));


      A2 = (s1 * z0);



      for (i = 1; i < 16; i++) {
        s1 = (*(fg41 + ttrig * 16 + i));
        B3 = y[15 + i];
        B3 = s1 * B3;
        A2 += B3;


      }


      A2 += (1 << (k_a - 1));
      A2 >>= k_a;
      T = 0;
      lch3 = A2;
      //too large amplitude
      if (A2 > 262015) {
        A1 = A2 >> 3;
        validity_code = 1;

        if (A1 > 262015) A1 = A1 >> 3;
        A1 = A1 - 112;
        printf("%lld 2 \n", A1);
        lch3 = A1;
        goto ou;
      }


      low_ampl = 0;



      if (A2 >= A0) {

        for (iter = 0, it = it0; iter < 3;) {
          iter++;
          s1 = (*(fg31 + it * 16));
          s2 = (*(fg32 + it * 16));
          A1 = (s1 * z0);
          B1 = (s2 * z0);



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
          if (A1 < -128) {
            validity_code = 1;
            A1 = -128;
            if (lch3 > 0) {
              validity_code = 2;
              A1 = lch3;
            }
            goto ou;
          }



          if (A1 > 262015) {
            validity_code = 1;
            A1 = A1 >> 3;
            if (A1 > 262015) A1 = A1 >> 3;
            A1 = A1 - 112;
            printf("%lld 1\n", A1);
            goto ou;
          }
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


            // NOTE: The only change to the code
            //B1 = B2 >> 13; //Variable 'B1' is reassigned a value before the old one has been used. (TF)
            B2 += (A1 << 13);
            B3 = (B2 / A1);


            T = ((it) << 3) + ((it) << 2) + (((B3 >> 1) + B3 + 2) >> 2) - 3072;

            T = ((210 - ttrig2) << 3) - T;



            B1 = B5 >> 9;
            B5 += (A1 << 9);
            B3 = (B5 / A1);
            it += ((B3 + 1) >> 1) - 256;
            it = it > it_h ? it_h : it;
            it = it < it_l ? it_l : it;


            T = T > 2047 ?  2047 : T;

            T = T < -2048 ? -2048 : T;

            C1 = (*(fg33 + it * 16) * z0);

            for (i = 1; i < 16; i++)
              C1 += *(fg33 + i + it * 16) * y[15 + i];
            C1 += (1 << (k_c - 1));
            C1 >>= k_c;


          }

        } // for (iter...)
      } // if (A2>A0)
      else
        low_ampl = 1;

      if (low_ampl == 1) {

lam:
        A1 = A2;
        if (A1 < -128) {
          validity_code = 1;
          A1 = -128;
          goto ou;
        }
        validity_code = 2;
        B1 = 0;
        C1 = (*(fg43 + ttrig * 16) * z0);
        for (i = 1; i < 16; i++) {
          B5 = y[15 + i];
          C1 += *(fg43 + i + ttrig * 16) * B5;
        }
        C1 += (1 << (k_c - 1));
        C1 >>= k_c;
      }
      ch2 = z00 - n16 * C1;
      ch1 = ((ch2) * (ch2));
      ch1 = ch1 * k_np[n16 - 1];
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
      if ((ch1 > B2) && (validity_code != 2))validity_code = 3;
      if ((C1 < 0) && (validity_code == 0))validity_code = 3;
ou:

      m_AmpFit = A1;
      m_TimeFit = T;

      int ss = (y[20] + y[21]);

      if (ss <= Ahard)validity_code = validity_code + 4;


      m_QualityFit = validity_code;


      return ;
    }

  }
}
