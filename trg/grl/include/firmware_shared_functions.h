#ifndef FIRMWARE_SHARED_FUNCTIONS_HH
#define FIRMWARE_SHARED_FUNCTIONS_HH

#include <cmath>

// Simulate ap_fixed/ap_ufixed quantization
// HLS ap_fixed defaults to AP_TRN (truncate toward negative infinity) and AP_WRAP (bit truncation)
inline double firmware_quantize(double value, int W, int I, bool is_signed = false)
{
  int frac_bits = W - I;
  double scale = std::pow(2.0, frac_bits);

  // Quantize - use AP_TRN (floor toward negative infinity, matches HLS default)
  double quantized = std::floor(value * scale) / scale;

  // Wrap-around (bit truncation) - matches HLS AP_WRAP behavior
  int64_t int_val = static_cast<int64_t>(quantized * scale);
  int64_t bit_mask = (1LL << W) - 1;
  int_val = int_val & bit_mask;

  // For signed values, handle sign extension
  if (is_signed) {
    int64_t sign_bit = 1LL << (W - 1);
    if (int_val & sign_bit) {
      int_val |= ~bit_mask;  // Sign extend
    }
  }

  return static_cast<double>(int_val) / scale;
}

// bit_shift function
inline double firmware_bit_shift(int N, double val)
{
  if (N >= 0) {
    return val * (1LL << N);
  } else {
    return val / (1LL << (-N));
  }
}

#endif // FIRMWARE_SHARED_FUNCTIONS_HH
