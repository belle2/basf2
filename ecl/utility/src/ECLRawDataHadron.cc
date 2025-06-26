#include <ecl/utility/ECLRawDataHadron.h>

#ifdef ECL_RAW_DATA_HADRON_STANDALONE_BUILD
#include <cstdio>  // printf
#include <cstdlib> // exit
#else
#include <framework/logging/Logger.h>
#endif

/**
 * See ecl/utility/include/ECLRawDataHadron.h for details
 */

namespace Belle2::ECL::RawDataHadron {

  /*                               Amplitude                                  */
  /* ------------------------------------------------------------------------ */

  unsigned long long packAmplitude(long long peak_amp)
  {
    // amplitude packing (from 18 bits int to 14 bits float)
    // sign      -- not used, 0 bits
    // exponent  -- 3 bits
    // fraction  -- 11 bits
    if (peak_amp < 0) {
#ifdef ECL_RAW_DATA_HADRON_STANDALONE_BUILD
      printf("\n\033[31m");
      printf("%s:%d: Error! Amplitude can never be negative, you have to call this function as packAmplitude(peak_amp + 128).\n",
             __FILE__, __LINE__);
      printf("\033[0m\n");
      exit(1);
#else
      B2FATAL("Amplitude can never be negative, you have to call this function as packAmplitude(peak_amp + 128).");
#endif
    }
    unsigned long long exponent;
    unsigned long long fraction;
    unsigned long long amp_packed = 0;

    exponent = 0;
    fraction = peak_amp;

    if ((fraction & 0x20000) != 0)
      exponent = 7;
    else if ((fraction & 0x10000) != 0)
      exponent = 6;
    else if ((fraction & 0x08000) != 0)
      exponent = 5;
    else if ((fraction & 0x04000) != 0)
      exponent = 4;
    else if ((fraction & 0x02000) != 0)
      exponent = 3;
    else if ((fraction & 0x01000) != 0)
      exponent = 2;
    else if ((fraction & 0x00800) != 0)
      exponent = 1;
    else
      exponent = 0;

    if (exponent > 0) {
      if (exponent > 1) {
        fraction += 1 << (exponent - 2);
        fraction = (fraction >> (exponent - 1)) - (1 << 11);
      }
    }
    //###############################################
    // ACCOUNTING FOR OVERFLOW
    //###############################################
    //   See the comments in packTime function
    //   for the explanation
    //###############################################
    if ((fraction & 0x00800) != 0)
      fraction -= 1;

    amp_packed = (exponent << 11) | fraction;
    return amp_packed;
  }

  unsigned long long unpackAmplitude(unsigned long long amp_packed)
  {
    unsigned int exponent = (amp_packed >> 11) & 0b111;
    unsigned int fraction = (amp_packed) & 0b11111111111;
    if (exponent == 0) {
      return fraction - 128;
    } else {
      return (1 << (10 + exponent)) + fraction * (1 << (exponent - 1)) - 128;
    }
  }

  /*                                   Time                                   */
  /* ------------------------------------------------------------------------ */

  int packTime(int peak_time)
  {
    // time packing (from 12 bits int to 11 bits float) as per IEEE 754
    // https://en.wikipedia.org/wiki/Double-precision_floating-point_format#IEEE_754_double-precision_binary_floating-point_format:_binary64
    // sign     -- 1 bit
    // exponent -- 2 bits
    // fraction -- 8 bits
    unsigned int exponent = 0;
    unsigned int sign     = 0;
    int fraction = peak_time;      // time from -2048 to 2047
    if ((fraction & 0x800) != 0) {
      sign = 1;
      fraction = -fraction;
    }

    if ((fraction & 0x400) != 0)
      exponent = 3;
    else if ((fraction & 0x200) != 0)
      exponent = 2;
    else if ((fraction & 0x100) != 0)
      exponent = 1;
    else
      exponent = 0;


    if (exponent > 0) {
      if (exponent > 1) {
        // Adjust initial value so that the value is rounded to
        // closest possible number.
        fraction += 1 << (exponent - 2);
      }
      fraction = (fraction >> (exponent - 1)) - (1 << 8);
      //###############################################
      // ACCOUNTING FOR OVERFLOW
      //###############################################
      // In several specific cases, rounding can cause
      // the value to overflow, this has to be handled.
      // (for example:
      //  We would like to compress 63 from 5 bits to 3 bits.
      //  Thus, we divide 63 by 2^(5-3) == 4
      //  63 + (4 / 2) == 65  # apply rounding
      //  65 / 4       == 16  # divide by 2^(5-3)
      //  And 16 will exceed the bit width of 3!
      //  So we need to account for such cases
      //###############################################
      if ((fraction & 0x100) != 0)
        fraction -= 1;
    }

    int peak_time_packed = (sign << 10) | (exponent << 8) | fraction;
    return peak_time_packed;
  }

  int unpackTime(int time_packed)
  {
    unsigned int sign     = (time_packed >> 10) & 0b1;
    unsigned int exponent = (time_packed >> 8) & 0b11;
    unsigned int fraction = (time_packed) & 0b11111111;

    int result;

    if (exponent == 0)
      result = fraction;
    else
      result = ((1 << (7 + exponent)) + fraction * (1 << (exponent - 1)));
    if (sign == 1)
      result = -result;
    return result;
  }

  /*                             Hadron fraction                              */
  /* ------------------------------------------------------------------------ */

  int integer_division_32(int dividend, int divisor)
  {
    // Basically the fastest algorithm for approximate integer division.
    // (with the precision (1/32))
    //
    // Algorithm description
    // =====================
    //
    // Calculating (n / m)
    //
    // 1. Determine p \in [0, 18], such that:
    //
    //         ⎛      p ⎞
    //  32 <=  ⎝ m / 2  ⎠ < 64
    //
    // 2. Define shifted (divided) values n_S, m_S:
    //
    //            p
    //  n  = n / 2  ∈ [0, 63]
    //   S
    //            p
    //  m  = m / 2  ∈ [32, 63]
    //   S
    //
    // 3. Calculate (n / m) as follows:
    //
    // \frac{n}{m} = \frac{n_S}{m_S} = n_S \cdot \left( \frac{2^{16}}{m_S} \right) \cdot \frac{1}{2^{16}}
    //
    //       n          ⎛  16 ⎞
    //   n    S         ⎜ 2   ⎟     1
    //   ─ = ── = n   ∙ ⎜ ─── ⎟  ∙ ───
    //   m   m     S    ⎜ m   ⎟     16
    //        S         ⎝  S  ⎠    2
    //
    //   ┌─────────────└───────┘────────────────────────────────┐
    //    Since m_S can take only 32 different values, we
    //    pre-calculate 32 different constants for (2^{16} / m_S)
    //
    // 4. Correction to improve division accuracy:
    //
    //  Instead of n_S, use (n_S + m_S / 2), this will result in
    //  correct rounding for the division.
    //
    if (divisor < 32)
      // Division can be done for divisor < 32 but it is meaningless:
      //  hadron component fit is meaningless for amp < 32
      return 0;

    if (dividend >= (1 << 18) || dividend < 0) {
      fprintf(stderr, "\n\033[31m");
      fprintf(stderr, "%s:%d: Error! Dividend outside of expected range: %d\n", __FILE__, __LINE__, dividend);
      fprintf(stderr, "\033[0m\n");
      return 0; // exit(1);
    }
    if (divisor >= (1 << 19)) {
      fprintf(stderr, "\n\033[31m");
      fprintf(stderr, "%s:%d: Error! Divisor outside of expected range: %d\n", __FILE__, __LINE__, divisor);
      fprintf(stderr, "\033[0m\n");
      return 0; // exit(1);
    }

    // while divisor < 32:
    //     dividend *= 2
    //     divisor *= 2
    //###############################################
    // GET CORRECT BITSHIFT
    //###############################################
    int i = 18;
    int p = i - 5;
    while (i >= 5) {
      if ((divisor & (1 << i)) != 0) {
        p = (i - 5);
        break;
      }
      i = i - 1;
    }
    //#######################
    // Equivalent math expression:
    //
    //  i = ⎣  log2(divisor) ⎦
    //  p = i - 5
    //
    //#######################
    // Equivalent code (same as a while loop above)
    // if   (divisor & 0x40000) != 0:
    //     p = 13
    // elif (divisor & 0x20000) != 0:
    //     p = 12
    // elif (divisor & 0x10000) != 0:
    //     p = 11
    // elif (divisor & 0x08000) != 0:
    //     p = 10
    // ...
    // elif (divisor & 0x00020) != 0:
    //     p = 0
    //#######################

    // NOTE: Because of this multiplication, dividend requires 23 bits.
    // The code can be rewritten to manage with 18 bits.
    dividend = dividend << 5;

    int n_S = dividend >> p;
    int m_S = divisor  >> p;

    // The values of round((2^16) / m_S), where m_S is in 32..63 range
    const int precalculated_constants[] = {
      // round(2**16 / (x+0.5)) for x in range(32, 64)
      // ===>
      2016, 1956, 1900, 1846, 1796, 1748, 1702, 1659,
      1618, 1579, 1542, 1507, 1473, 1440, 1409, 1380,
      1351, 1324, 1298, 1273, 1248, 1225, 1202, 1181,
      1160, 1140, 1120, 1101, 1083, 1066, 1049, 1032
    };

    //     n         ⎛  16 ⎞
    // n    S        ⎜ 2   ⎟    1
    // ─ = ── = n  ∙ ⎜ ─── ⎟ ∙ ───
    // m   m     S   ⎜ m   ⎟    16
    //      S        ⎝  S  ⎠   2

    return ((n_S + m_S / 2) * precalculated_constants[m_S - 32]) >> 16;
  }

  int packHadronFraction(int A_hadron, int A_total)
  {
    // The actual expression is ( (A_hadron / A_total + 0.25) / 0.75 ) * 32
    // (for the firmware, dividend and divisor are multiplied by 4 to simplify calculations)
    int dividend        = 4 * A_hadron + A_total;
    if (dividend < 0) return 0;
    int packed_fraction = integer_division_32(dividend, 3 * A_total);
    return packed_fraction;
  }

  double unpackHadronFraction(int fraction_packed)
  {
    return fraction_packed / 32.0 * 0.75 - 0.25;
  }
}

