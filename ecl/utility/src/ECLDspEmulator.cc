//ECL
#include <ecl/utility/ECLDspEmulator.h>
//Framework
#include <framework/logging/Logger.h>
//STL
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <stdlib.h>
#include <string.h>

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
      //                Typical plot of y_i (i=0..31)
      // +-------------------------------------------------------+
      // |                                                       |
      // |               -------------------------               |
      // |               ^             xxxx                      |
      // |               |            xx  x                      |
      // |               |           x     x                     |
      // |               |           x     xx                    |
      // |          A1   |          xx      x                    |
      // |        (ampl) |          x       x                    |
      // |               |          x       xx                   |
      // |               |         xx        x                   |
      // |               |         x         x                   |
      // |               |        xx          x                  |
      // |               |        x            xx                |
      // |               |       xx             xx               |
      // |               v       x               xxxx            |
      // |xxxxxxxxxxxxxxxxxxxxxxxx                  xxxxxxxxxxxxxx
      // |  ^                                                    |
      // |<-|------------------->                                |
      // |  |      n_16                                          |
      // |  |                                                    |
      // |  | C1                                                 |
      // |  v (pedestal)                                         |
      // +-------------------------------------------------------+
      //
      // 0                 10                 20                30
      //                                            (point number)
      //

      using namespace ShapeFitter;

      enum QualityFlag {
        c_GoodQuality = 0,
        c_InternalError = 1,
        c_LowAmp = 2,
        c_BadChi2 = 3
      };

      /***   VARIABLE DEFINITION   ***/

      const static long long int k_np[16] = {
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

      // Trigger time 0-23
      const int ttrig = ttrig2 > 0 ? ttrig2 / 6 : 0;
      // Number of points for pedestal measurement
      const int n_16 = 16;

      if (k_16 + n_16 != 16) {
        B2ERROR("Disagreement in the number of points: "
                << k_16 << " and " << n_16);
      }

      // Initial time index
      int it0 = 48 + ((23 - ttrig) << 2);
      // Min time index, max time index
      int it_l = 0, it_h = 191;
      // Time index, must be within [it_l, it_h]
      int it = setInRange(it0, it_l, it_h);
      // Amplitude from the fit without time correction
      // (assuming t_0 == trigger time)
      long long A2;
      // Amplitude from the fit with time correction
      long long A1;
      // Estimation of (Ampl * delta T) value
      long long B1;
      // Pedestal, used to calculate chi^2
      long long C1 = 0;

      // Quality flag (https://confluence.desy.de/display/BI/ECL+Quality+flag)
      int validity_code = c_GoodQuality;
      bool skip_fit = false;

      //== Calculate sum of first 16 points in the waveform.
      //   This sum is used for pedestal estimation.

      long long z00 = 0;
      for (int i = k_16; i < 16; i++)
        z00 += y[i];

      const int kz_s = 0;
      const long long z0 = z00 >> kz_s;

      /***   FIRST APPROXIMATION   ***/

      //== First approximation without time correction
      //   (assuming t_0 == trigger time)

      A2 = fg41[ttrig * 16] * z0;

      for (int i = 1; i < 16; i++)
        A2 += y[15 + i] * (long long)fg41[ttrig * 16 + i];

      A2 += (1 << (k_a - 1));
      A2 >>= k_a;

      //== Check if amplitude estimation is too large.

      if (amplitudeOverflow(A2)) {
        A1 = A2 >> 3;
        if (amplitudeOverflow(A1)) A1 >>= 3;
        A1 -= 112;
        B2DEBUG(15, A1 << " 2");

        validity_code = c_InternalError;
        skip_fit = true;
      }

      //== Check if amplitude is less than LA_THR
      //   (https://confluence.desy.de/display/BI/Electronics+Thresholds)

      bool low_ampl = false;
      if (A2 < A0) low_ampl = true;

      /***   MAIN PART   ***/

      //== Main part of the algorithm, estimate amplitude, time and pedestal

      // Time estimation in ADC ticks.
      // (See TDC time in https://confluence.desy.de/display/BI/ECL+Technical+Notes)
      int T = 0;

      if (!skip_fit && !low_ampl) {
        for (int iter = 1; iter <= 3; iter++) {

          //== Get amplitude and (ampl*time)
          //   at time index 'it'

          A1 = fg31[it * 16] * z0;
          B1 = fg32[it * 16] * z0;

          for (int i = 1; i < 16; i++) {
            A1 += fg31[it * 16 + i] * (long long)y[15 + i];
            B1 += fg32[it * 16 + i] * (long long)y[15 + i];
          }
          A1 += (1 << (k_a - 1));
          A1 >>= k_a;

          //== Check if amplitude estimation is negative.

          if (A1 < -128) {
            if (A2 > 0) {
              A1 = A2;
              validity_code = c_LowAmp;
            } else {
              A1 = -128;
              validity_code = c_InternalError;
            }

            skip_fit = true;
            break;
          }

          //== Check if amplitude estimation is too large.

          if (amplitudeOverflow(A1)) {
            A1 >>= 3;
            if (amplitudeOverflow(A1)) A1 >>= 3;
            A1 -= 112;
            B2DEBUG(15, A1 << " 1");

            validity_code = c_InternalError;
            skip_fit = true;
            break;
          }

          //== Check if amplitude is less than LA_THR
          //   (https://confluence.desy.de/display/BI/Electronics+Thresholds)

          if (A1 < A0) {
            it = it0;
            low_ampl = true;
            break;
          }

          // Internal variables for fit algo
          long long B2, B3;

          //== Estimate t_new as t_0 - B/A

          if (iter != 3) {
            B2 = B1 >> (k_b - 9);
            B2 += (A1 << 9);
            B3 = (B2 / A1);

            B1 >>= k_b;

            it += ((B3 + 1) >> 1) - 256;
            it = setInRange(it, it_l, it_h);
          } else {
            B2 = B1 >> (k_b - 13);
            B2 += (A1 << 13);
            B3 = (B2 / A1);

            T = 3072 - (it << 3) - (it << 2) - (((B3 >> 1) + B3 + 2) >> 2);
            T += ((210 - ttrig2) << 3);
            T = setInRange(T, -2048, 2047);

            B2 = B1 >> (k_b - 9);
            B2 += A1 << 9;
            B3 = B2 / A1;

            B1 >>= k_b;

            it += ((B3 + 1) >> 1) - 256;
            it = setInRange(it, it_l, it_h);

            //== Estimate pedestal

            C1 = fg33[it * 16] * z0;
            for (int i = 1; i < 16; i++)
              C1 += fg33[it * 16 + i] * (long long)y[15 + i];
            C1 += (1 << (k_c - 1));
            C1 >>= k_c;
          }
        } // for (int iter...)
      } // if (!skip_fit && !low_ampl)

      //== Use initial time estimation for low amplitude

      if (!skip_fit && low_ampl) {
        A1 = A2;
        if (A1 < -128) {
          skip_fit = true;
          validity_code = c_InternalError;
          A1 = -128;
        } else {
          validity_code = c_LowAmp;
          B1 = 0;

          //== Estimate pedestal

          C1 = fg43[ttrig * 16] * z0;
          for (int i = 1; i < 16; i++)
            C1 += fg43[ttrig * 16 + i] * (long long)y[15 + i];
          C1 += (1 << (k_c - 1));
          C1 >>= k_c;
        }
      }

      //== Estimate chi^2

      if (!skip_fit) {
        //== Get actual threshold for chi^2, based on amplitude fit.
        //
        long long chi_thr;
        chi_thr = (A1 >> 1) * (A1 >> 1);
        chi_thr >>= (k2_chi - 2);
        chi_thr += chi_thres;

        //== Get chi^2
        //
        long long chi_sq;
        long long chi;

        chi = z00 - n_16 * C1;

        chi_sq = chi * chi;
        chi_sq *= k_np[n_16 - 1];
        chi_sq >>= 16;

        for (int i = 1; i < 16; i++) {
          chi = A1 * f[it * 16 + i] + B1 * f1[it * 16 + i];
          chi >>= k1_chi;
          chi += C1 - y[i + 15];

          chi_sq += chi * chi;
        }

        if (chi_sq > chi_thr && validity_code != c_LowAmp) validity_code = c_BadChi2;
        if (C1 < 0 && validity_code == c_GoodQuality) validity_code = c_BadChi2;
      }

      /***      ***/

      //== Compare signal peak to HIT_THR (aka Ahard)
      //   (See https://confluence.desy.de/display/BI/Electronics+Thresholds)

      int ss = (y[20] + y[21]);
      if (ss <= Ahard) validity_code += 4;

      //==

      // TODO: Compare on large dataset.
      // if (validity_code == c_LowAmp) T = 0;

      m_AmpFit = A1;
      m_TimeFit = T;
      m_QualityFit = validity_code;

      return ;
    }

  }
}
