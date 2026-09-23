/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//ECL
#include <ecl/utility/ECLDspEmulator.h>
#include <ecl/digitization/EclConfigurationUp.h>
//Framework
#include <framework/logging/Logger.h>

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
      enum QualityFlag {
        c_GoodQuality = 0,
        c_InternalError = 1,
        c_LowAmp = 2,
        c_BadChi2 = 3
      };
      bool amplitudeOverflow(long long amp)
      {
        // There are 18 bits reserved for the amplitude,
        // in range [-128, 262015]
        static const long long max_amp = 0x3FFFF - 128;
        return amp > max_amp;
      }
    }
  }
}

namespace Belle2 {
  namespace ECL {
    template <typename INT>
    ECLShapeFit lftda_(const INT* f, const INT* f1, const INT* fg41,
                       const INT* fg43, const INT* fg31, const INT* fg32,
                       const INT* fg33, int* y, int ttrig2, int la_thr,
                       int hit_thr, int skip_thr, int k_a, int k_b,
                       int k_c, int k_16, int k1_chi, int k2_chi,
                       int chi_thres, bool adjusted_timing)
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
      int it0;
      if (!adjusted_timing) {
        it0 = 48 + ((23 - ttrig) << 2);
      } else {
        it0 = 48 + ((143 - ttrig2) << 2) / 6;
      }
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

      // Quality flag (https://xwiki.desy.de/xwiki/rest/p/ca7f6)
      int validity_code = c_GoodQuality;
      bool skip_fit = false;
      // Set to true if amplitude is less than SKIP_THR
      bool skip_thr_flag = false;

      //== Calculate sum of first 16 points in the waveform.
      //   This sum is used for pedestal estimation.

      long long z00 = 0;
      for (int i = k_16; i < 16; i++)
        z00 += y[i];

      const int kz_s = 0;
      const long long z0 = z00 >> kz_s;

      // Struct with fit results
      ECLShapeFit result;
      result.fit.resize(31);

      //== Check for pedestal amplitude overflow

      if (z00 > 0x3FFFF) {
        skip_fit = true;
        validity_code = c_InternalError;
        A1 = -128;
      }

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
      //   (https://xwiki.desy.de/xwiki/rest/p/545bc)

      bool low_ampl = false;
      if (A2 < la_thr) low_ampl = true;

      /***   MAIN PART   ***/

      //== Main part of the algorithm, estimate amplitude, time and pedestal

      // Time estimation in ADC ticks.
      // (See TDC time in https://xwiki.desy.de/xwiki/rest/p/4630a)
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
          //   (https://xwiki.desy.de/xwiki/rest/p/545bc)

          if (A1 < la_thr) {
            it = it0;
            low_ampl = true;
            break;
          }

          //== Check if amplitude is less than SKIP_THR
          //   (https://xwiki.desy.de/xwiki/rest/p/545bc)

          if (A1 < skip_thr) {
            skip_thr_flag = true;
          }

          // Internal variables for fit algo
          long long B2, B3;

          //== Estimate t_new as t_0 - B/A

          if (k_b >= 13) {
            B2 = B1 >> (k_b - 13);
          } else {
            B2 = B1 << (13 - k_b);
          }
          B2 += (A1 << 13);
          B3 = (B2 / A1);

          int it_uncut = (B3 + 16) >> 5;
          int delta_it = it_uncut - 256;
          delta_it = setInRange(delta_it, -191, 191);

          int it_current = it;

          it += delta_it;
          it = setInRange(it, it_l, it_h);

          if (iter == 3) {
            int delta_T;

            if (B3 >= 0x37FE)
              delta_T = 2047;
            else if ((B3 >> 1) <= 0x400)
              delta_T = -2047;
            else
              delta_T = ((B3 * 3 + 4) >> 3) - 3072;

            int t_uncut = it_current * 12 + delta_T;
            t_uncut = setInRange(t_uncut, -4096, 4095);

            T = -t_uncut;
            if (!adjusted_timing) {
              T += ((210 - ttrig2) << 3);
            } else {
              T += ((215 - ttrig2) << 3) - 4;
            }

            T = setInRange(T, -2048, 2047);

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
      long long chi_sq = 0;

      if (!skip_fit) {
        //== Get fit function values in 31 points

        int B1_chi = B1 >> k_b;

        // Points 0-15 contain identical pedestal value
        result.fit[0] = A1 * f[it * 16] + B1_chi * f1[it * 16];
        result.fit[0] >>= k1_chi;
        result.fit[0] += C1;
        for (int i = 1; i <= 15; i++) {
          result.fit[i] = result.fit[0];
        }
        // Points 16-30 contain the signal
        for (int i = 1; i < 16; i++) {
          result.fit[i + 15] = A1 * f[it * 16 + i] + B1_chi * f1[it * 16 + i];
          result.fit[i + 15] >>= k1_chi;
          result.fit[i + 15] += C1;
        }

        //== Get actual threshold for chi^2, based on amplitude fit.

        long long chi_thr;
        chi_thr = (A1 >> 1) * (A1 >> 1);
        chi_thr >>= (k2_chi - 2); // a1n
        chi_thr += chi_thres; // ch2_int

        //== Get chi^2

        long long chi;

        chi = n_16 * result.fit[0] - z00;

        chi_sq = chi * chi;
        chi_sq *= k_np[n_16 - 1];
        chi_sq >>= 16;
        for (int i = 1; i < 16; i++) {
          chi = result.fit[i + 15] - y[i + 15];
          chi_sq += chi * chi;
        }

        if (chi_sq > chi_thr && validity_code != c_LowAmp) validity_code = c_BadChi2;
        if (C1 < 0 && validity_code == c_GoodQuality) validity_code = c_BadChi2;
      }

      /***      ***/

      //== Compare signal peak to HIT_THR
      //   (See https://xwiki.desy.de/xwiki/rest/p/545bc)

      int hit_val = y[20] + y[21] - (y[12] + y[13] + y[14] + y[15]) / 2;
      if (hit_val <= hit_thr) {
        validity_code += 4;
      }

      //== Compare amplitude to SKIP_THR
      //   (See https://xwiki.desy.de/xwiki/rest/p/545bc)

      if (A1 < skip_thr && validity_code != c_InternalError) {
        skip_thr_flag = true;
      }

      //== Set output values

      result.amp = A1;
      result.time = T;
      result.quality = validity_code % 4;
      result.hit_thr = validity_code / 4;
      result.skip_thr = skip_thr_flag;
      result.low_amp = low_ampl;
      result.pedestal = result.fit[0];

      result.chi2 = chi_sq;

      return result;
    }

    template ECLShapeFit lftda_<short>(const short* f, const short* f1, const short* fg41,
                                       const short* fg43, const short* fg31, const short* fg32,
                                       const short* fg33, int* y, int ttrig2, int la_thr,
                                       int hit_thr, int skip_thr, int k_a, int k_b,
                                       int k_c, int k_16, int k1_chi, int k2_chi,
                                       int chi_thres, bool adjusted_timing);
    template ECLShapeFit lftda_<int>(const int* f, const int* f1, const int* fg41,
                                     const int* fg43, const int* fg31, const int* fg32,
                                     const int* fg33, int* y, int ttrig2, int la_thr,
                                     int hit_thr, int skip_thr, int k_a, int k_b,
                                     int k_c, int k_16, int k1_chi, int k2_chi,
                                     int chi_thres, bool adjusted_timing);

    template <typename INT>
    ECLShapeFit lftdaUp_(const INT* f, const INT* f1, const INT* fg41,
                         const INT* fg43, const INT* fg31, const INT* fg32,
                         const INT* fg33, int* y, int ttrig2, int la_thr,
                         int hit_thr, int skip_thr, int k_a, int k_b,
                         int k_c, int k1_chi, int k2_chi,
                         int chi_thres, bool adjusted_timing)
    {
      //                Typical plot of y_i (i=0..64)
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
      // +-------------------------------------------------------+
      //
      // 0                 21                 42                64
      //                                            (point number)
      //

      using namespace ShapeFitter;

      /***   VARIABLE DEFINITION   ***/
      constexpr int N_COEF = EclConfigurationUp::c_nFitSamples;    // DSP vector length (== c_nADCsignal)
      constexpr int N_ADC = EclConfigurationUp::c_nAdcSamples;   // number of ADC samples
      constexpr int SIG_OFFSET = N_ADC - N_COEF;      // signal starts at y[SIG_OFFSET]
      constexpr int N_DT = EclConfigurationUp::c_nFitPointsPerAdcTick;    // number of time divisions in 1 step
      constexpr int N_TF = EclConfigurationUp::c_nFitTimeShifts;    // number of time time shifts
      constexpr int N_TRG = EclConfigurationUp::c_nTriggerCounts; // number of trigger divisions in 1 step
      constexpr int N_TA = N_DT / 4;   // number of time divisions in 1 step for low amplitude case
      constexpr int N_SUB = EclConfigurationUp::c_nSubTimeCorr;   // number of sub-divisions for time correction
      constexpr int N_PED = N_ADC + EclConfigurationUp::c_fitTimeMin;     // pedestal samples for rough amplitude estimation
      constexpr int MAX_ITER = 20; // max number of iterations for time correction
      constexpr int N_TICKS = N_DT * N_TF * N_SUB; // number of time ticks in the fit window

      // Trigger time [0, N_TA-1]
      const int ttrig = setInRange((ttrig2 * N_TA) / N_TRG, 0, N_TA - 1);

      // Initial time index
      int it0;
      if (!adjusted_timing) {
        it0 = lround(
                ((N_TF - 1) * N_DT / 2) - 1 + ((N_TA - 1 - ttrig) * N_DT) / N_TA
              );
      } else {
        it0 = lround(
                ((N_TF - 1) * N_DT / 2) - 1 + ((N_TRG - 1 - ttrig2) * N_DT) / N_TRG
              );
      }
      // Min time index, max time index
      int it_l = 0, it_h = (N_TF * N_DT) - 1;
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

      // Quality flag (https://xwiki.desy.de/xwiki/rest/p/ca7f6)
      int validity_code = c_GoodQuality;
      bool skip_fit = false;
      // Set to true if amplitude is less than SKIP_THR
      bool skip_thr_flag = false;

      // Struct with fit results
      ECLShapeFit result;
      result.fit.resize(SIG_OFFSET + N_COEF);

      /***   FIRST APPROXIMATION   ***/

      //== First approximation without time correction
      //   (assuming t_0 == trigger time)

      A2 = 0;

      for (int i = 0; i < N_COEF; i++)
        A2 += y[SIG_OFFSET + i] * (long long)fg41[ttrig * N_COEF + i];

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
      //   (https://xwiki.desy.de/xwiki/rest/p/545bc)

      bool low_ampl = false;
      if (A2 < la_thr) low_ampl = true;

      /***   MAIN PART   ***/

      //== Main part of the algorithm, estimate amplitude, time and pedestal

      // Time estimation in ADC ticks.
      // (See TDC time in https://xwiki.desy.de/xwiki/rest/p/4630a)
      int T = 0;
      int converge_flag = 0;

      const int iterations = MAX_ITER; // number of iterations for time correction, can be tuned for better performance
      if (!skip_fit && !low_ampl) {
        for (int iter = 1; iter <= iterations; iter++) {

          //== Get amplitude and (ampl*time)
          //   at time index 'it'

          A1 = 0;
          B1 = 0;

          for (int i = 0; i < N_COEF; i++) {
            A1 += fg31[it * N_COEF + i] * (long long)y[SIG_OFFSET + i];
            B1 += fg32[it * N_COEF + i] * (long long)y[SIG_OFFSET + i];
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
          //   (https://xwiki.desy.de/xwiki/rest/p/545bc)

          if (A1 < la_thr) {
            it = it0;
            low_ampl = true;
            break;
          }

          //== Check if amplitude is less than SKIP_THR
          //   (https://xwiki.desy.de/xwiki/rest/p/545bc)

          if (A1 < skip_thr) {
            skip_thr_flag = true;
          }

          // Internal variables for fit algo
          long long B2, B3;

          //== Estimate t_new as t_0 - B/A

          if (k_b >= 15) {
            B2 = B1 >> (k_b - 15);
          } else {
            B2 = B1 << (15 - k_b);
          }
          B2 += (A1 << 15);
          B3 = (B2 / A1);

          int it_uncut = (B3 + 64) >> 7;
          int delta_it = it_uncut - (N_TF * N_DT / 2);
          delta_it = setInRange(delta_it, -(N_TF * N_DT - 1), N_TF * N_DT - 1);

          int it_current = it;

          it += delta_it;
          it = setInRange(it, it_l, it_h);

          int r = static_cast<int>(B3 - (1 << 15));
          const int half_bin = (1 << 15) / (2 * N_TF * N_DT);

          if ((delta_it == 0) && (converge_flag == 0)) {
            if (r > half_bin) it += 1;
            else if (r < -half_bin) it -= 1;
            it = setInRange(it, it_l, it_h);
            converge_flag = 1;
          }

          if ((iter == iterations) || ((it == it_current) && (converge_flag == 1))) {

            int residual = B3 - (1 << 15);
            int delta_T;
            if (residual >= 0)
              delta_T = (residual + 2) >> 2;
            else
              delta_T = -(((-residual) + 2) >> 2);
            delta_T = setInRange(delta_T, -N_TICKS / 2, N_TICKS / 2 - 1);

            int t_uncut = (it_current - (N_TF * N_DT / 2)) * N_SUB + delta_T;
            t_uncut = setInRange(t_uncut, -N_TICKS, N_TICKS - 1);

            int trig_offset = static_cast<int>(std::lround(
                                                 double(ttrig2 - N_TRG / 2) * double(N_DT * N_SUB) / double(N_TRG)
                                               ));
            T = -t_uncut - trig_offset;

            T = setInRange(T, -N_TICKS, N_TICKS - 1);

            //== Estimate pedestal

            C1 = 0;
            for (int i = 0; i < N_COEF; i++)
              C1 += fg33[it * N_COEF + i] * (long long)y[SIG_OFFSET + i];
            C1 += (1 << (k_c - 1));
            C1 >>= k_c;
            break;
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

          C1 = 0;
          for (int i = 0; i < N_COEF; i++)
            C1 += fg43[ttrig * N_COEF + i] * (long long)y[SIG_OFFSET + i];
          C1 += (1 << (k_c - 1));
          C1 >>= k_c;
        }
      }

      //== Estimate chi^2
      long long chi_sq = 0;

      if (!skip_fit) {
        //== Get fit function values in 64 points

        int B1_chi = B1 >> k_b;

        // Points 0-64 contain the signal
        for (int i = 0; i < N_COEF; i++) {
          result.fit[i + SIG_OFFSET] = A1 * f[it * N_COEF + i] + B1_chi * f1[it * N_COEF + i];
          result.fit[i + SIG_OFFSET] >>= k1_chi;
          result.fit[i + SIG_OFFSET] += C1;
        }

        //== Get actual threshold for chi^2, based on amplitude fit.

        long long chi_thr;
        chi_thr = (A1 >> 1) * (A1 >> 1);
        chi_thr >>= (k2_chi - 7); // a1n
        chi_thr += chi_thres; // ch2_int

        //== Get chi^2

        long long chi;

        for (int i = 0; i < N_COEF; i++) {
          chi = result.fit[i + SIG_OFFSET] - y[i + SIG_OFFSET];
          chi_sq += chi * chi;
        }

        if (chi_sq > chi_thr && validity_code != c_LowAmp) validity_code = c_BadChi2;
        if (C1 < 0 && validity_code == c_GoodQuality) validity_code = c_BadChi2;
      }

      /***      ***/

      //== Compare signal peak to HIT_THR
      //   (See https://xwiki.desy.de/xwiki/rest/p/545bc)

      int hit_val = y[N_PED + 5] + y[N_PED + 6] - (y[N_PED - 4] + y[N_PED - 3] + y[N_PED - 2] + y[N_PED - 1]) / 2;
      if (hit_val <= hit_thr) {
        validity_code += 4;
      }

      //== Compare amplitude to SKIP_THR
      //   (See https://xwiki.desy.de/xwiki/rest/p/545bc)

      if (A1 < skip_thr && validity_code != c_InternalError) {
        skip_thr_flag = true;
      }

      //== Set output values

      result.amp = A1;
      result.time = T;
      result.quality = validity_code % 4;
      result.hit_thr = validity_code / 4;
      result.skip_thr = skip_thr_flag;
      result.low_amp = low_ampl;
      result.pedestal = C1;

      result.chi2 = chi_sq;

      return result;
    }

    template ECLShapeFit lftdaUp_<short>(const short* f, const short* f1, const short* fg41,
                                         const short* fg43, const short* fg31, const short* fg32,
                                         const short* fg33, int* y, int ttrig2, int la_thr,
                                         int hit_thr, int skip_thr, int k_a, int k_b,
                                         int k_c, int k1_chi, int k2_chi,
                                         int chi_thres, bool adjusted_timing);
    template ECLShapeFit lftdaUp_<int>(const int* f, const int* f1, const int* fg41,
                                       const int* fg43, const int* fg31, const int* fg32,
                                       const int* fg33, int* y, int ttrig2, int la_thr,
                                       int hit_thr, int skip_thr, int k_a, int k_b,
                                       int k_c, int k1_chi, int k2_chi,
                                       int chi_thres, bool adjusted_timing);
  }
}

