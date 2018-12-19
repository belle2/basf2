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
    template<typename T>
    T setInRange(T val, T min, T max)
    {
      if (val < min) return min;
      if (val > max) return max;
      return val;
    }

    namespace ShapeFitter {
      bool amplitudeOverflow(long long amp)
      {
        return amp > 262015;
      }
    }
  }
}

namespace Belle2 {
  namespace ECL {
    void lftda_(short int* f, short int* f1, short int* fg41, short int* fg43, short int* fg31, short int* fg32, short int* fg33,
                int* y, int& ttrig2, int& A0, int& Ahard, int& k_a, int& k_b, int& k_c, int& k_16, int& k1_chi, int& k2_chi, int& chi_thres,
                int& m_AmpFit, int& m_TimeFit, int& m_QualityFit)
    {
      using namespace ShapeFitter;
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

      const int ttrig = ttrig2 > 0 ? ttrig2 / 6 : 0;
      const int n16 = 16;

      if (k_16 + n16 != 16) {
        cout << "disagreement in number of the points " << k_16 << "and " << n16 << endl;
      }

      // Initial time index
      int it0 = 48 + ((23 - ttrig) << 2);
      // Time index
      int it;
      // Min time value, max time value
      int it_l, it_h;
      long long A1, B1, A2, C1, ch2, B2, B3, B5;

      long long z00 = 0;
      for (int i = k_16; i < 16; i++) {
        z00 += y[i];
      }

      const int kz_s = 0;
      const long long z0 = z00 >> kz_s;

      it_l = 0;
      it_h = 191;
      it = it0 = setInRange(it0, it_l, it_h);

      //first approximation without time correction

      //  int it00=23-it0;

      A2 = fg41[ttrig * 16] * z0;

      for (int i = 1; i < 16; i++)
        A2 += y[15 + i] * (long long)fg41[ttrig * 16 + i];

      A2 += (1 << (k_a - 1));
      A2 >>= k_a;

      int T = 0;
      long long ch1;
      int validity_code = 0;
      bool skip_fit = false;

      //************* SCOPE (A1, validity_code) <= (A2)
      //too large amplitude
      if (amplitudeOverflow(A2)) {
        A1 = A2 >> 3;
        validity_code = 1;

        if (amplitudeOverflow(A1)) A1 >>= 3;
        A1 -= 112;
        printf("%lld 2 \n", A1);
        skip_fit = true;
      }
      //************* ENDSCOPE

      bool low_ampl = false;
      if (A2 < A0) low_ampl = true;

      // fitForNormalAmplitude(fg31, fg32, fg33, it0, A0, it_l, it_h, k_a, k_b, k_c, A1, B1, B2, B3, B5, C1, T, validity_code);

      //=== SCOPE (A1, B1, B2, B3, B5, C1, T, validity_code) <= (it0, A0, fg__, it_l, it_h, k_a, k_b, k_c)
      if (!skip_fit && !low_ampl) {
        for (int iter = 1; iter <= 3; iter++) {
          A1 = fg31[it * 16] * z0;
          B1 = fg32[it * 16] * z0;

          for (int i = 1; i < 16; i++) {
            A1 += fg31[it * 16 + i] * (long long)y[15 + i];
            B1 += fg32[it * 16 + i] * (long long)y[15 + i];
          }
          A1 += (1 << (k_a - 1));
          A1 >>= k_a;

          //=== SCOPE (A1, validity_code) <- (A1, A2)
          if (A1 < -128) {
            validity_code = 1;
            A1 = -128;
            if (A2 > 0) {
              validity_code = 2;
              A1 = A2;
            }
            skip_fit = true;
            break;
          }
          //===

          //=== SCOPE (A1, validity_code) <- (A1)
          if (amplitudeOverflow(A1)) {
            validity_code = 1;
            A1 = A1 >> 3;
            if (amplitudeOverflow(A1)) A1 = A1 >> 3;
            A1 = A1 - 112;
            printf("%lld 1\n", A1);
            skip_fit = true;
            break;
          }
          //===

          if (A1 < A0) {
            it = it0;
            low_ampl = true;
            break;
          }

          if (iter != 3) {
            B2 = B1 >> (k_b - 9);
            B1 = B2 >> 9;

            B2 += (A1 << 9);

            B3 = (B2 / A1);

            it += ((B3 + 1) >> 1) - 256;
            it = setInRange(it, it_l, it_h);
          } else {
            B2 = B1 >> (k_b - 13);
            B5 = B1 >> (k_b - 9);

            B2 += (A1 << 13);
            B3 = (B2 / A1);

            T = (it << 3) + (it << 2) + (((B3 >> 1) + B3 + 2) >> 2) - 3072;

            T = ((210 - ttrig2) << 3) - T;

            B1 = B5 >> 9;
            B5 += A1 << 9;
            B3 = B5 / A1;
            it += ((B3 + 1) >> 1) - 256;
            it = setInRange(it, it_l, it_h);

            T = setInRange(T, -2048, 2047);

            C1 = fg33[it * 16] * z0;
            for (int i = 1; i < 16; i++)
              C1 += fg33[it * 16 + i] * y[15 + i];
            C1 += (1 << (k_c - 1));
            C1 >>= k_c;
          }
        } // for (iter...)
      } // if (low_ampl == 0)

      if (!skip_fit && low_ampl) {
        A1 = A2;
        if (A1 < -128) {
          validity_code = 1;
          A1 = -128;
          skip_fit = true;
        } else {
          validity_code = 2;
          B1 = 0;
          C1 = fg43[ttrig * 16] * z0;
          for (int i = 1; i < 16; i++)
            C1 += fg43[ttrig * 16 + i] * y[15 + i];
          C1 += (1 << (k_c - 1));
          C1 >>= k_c;
        }
      }

      if (!skip_fit) {
        ch2 = z00 - n16 * C1;

        ch1 = ch2 * ch2;
        ch1 *= k_np[n16 - 1];
        ch1 >>= 16;

        for (int i = 1; i < 16; i++) {
          ch2 = A1 * f[it * 16 + i] + B1 * f1[it * 16 + i];
          ch2 >>= k1_chi;
          ch2 += C1 - y[i + 15];

          ch1 += ch2 * ch2;
        }
        B2 = (A1 >> 1) * (A1 >> 1);
        B2 >>= (k2_chi - 2);
        B2 += chi_thres;
        if (ch1 > B2 && validity_code != 2) validity_code = 3;
        if (C1 < 0 && validity_code == 0) validity_code = 3;
      }

      m_AmpFit = A1;
      m_TimeFit = T;

      int ss = (y[20] + y[21]);

      if (ss <= Ahard) validity_code += 4;

      m_QualityFit = validity_code;

      return ;
    }

  }
}
