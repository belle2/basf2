/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/dbobjects/ECLDspData.h>

using namespace Belle2;

void ECLDspData::packCoefVector(const std::vector<short int>& src, std::vector<short int>& dst)
{
  const int N_CHANNELS = 16;
  int size = src.size();

  // cppcheck-suppress knownConditionTrueFalse
  if (getPackerVersion() == 0) {
    dst = src;
    // cppcheck-suppress knownConditionTrueFalse
  } else if (getPackerVersion() >= 1) {
    // Apply reversible transformation to shrink the range
    // of saved coefficients into much smaller interval.
    // This improves the compression by a factor of ~2.

    dst.resize(size);
    for (int i = size - 1; i >= 0; i--) {
      if (i >= 2 * N_CHANNELS)
        dst[i] = src[i] - 2 * src[i - N_CHANNELS] + src[i - 2 * N_CHANNELS];
      else if (i >= N_CHANNELS)
        dst[i] = src[i] - 2 * src[i - N_CHANNELS];
      else
        dst[i] = src[i];
    }
  }

  // cppcheck-suppress knownConditionTrueFalse
  if (getPackerVersion() == 2) {
    // Rearrange the data in such a way that each coeff
    // only takes up 4 bits (special value 0xF indicates
    // coefficients that are too big to be packed)
    //
    // Vector structure after packing:
    // [big_coefs | packed_coefs | packed_coefs.size()]

    // Bits allocated for each value
    const int value_bits = 4;
    const long value_max = 0xF;
    const int values_packed = sizeof(short) * 8 / value_bits;

    int shift = -6;

    std::vector<short> packed(size / values_packed, 0);
    int len = -1;

    for (int i = 0; i < size; i++) {
      short val = dst[i] - shift;
      if (val >= 0 && val < value_max) {
        if (len < 0) len = i;
        packed[i / values_packed] |= val << (value_bits * (i % values_packed));
      } else {
        if (len >= 0 && len < i) {
          dst[len++] = dst[i];
        }
        packed[i / values_packed] |= short(value_max) << (value_bits * (i % values_packed));
      }
    }

    int packed_size = 0;

    if (len >= 0) {
      dst.resize(len + packed.size());
      packed_size = packed.size();
      for (int i = 0; i < packed_size; i++) {
        dst[len + i] = packed[i];
      }
    }
    dst.push_back(packed_size);
    dst.shrink_to_fit();
  }
}
void ECLDspData::unpackCoefVector(const std::vector<short int>& src, std::vector<short int>& dst) const
{
  const int N_CHANNELS = 16;

  int packer_version = m_extraData[1];
  int dst_size = 0;

  if (packer_version == 1) {
    dst_size = src.size();
    dst.resize(dst_size);
  }
  if (packer_version >= 2) {
    // Number of bits allocated for each value
    const int value_bits = 4;
    const long value_max = 0xF;
    const int values_packed = sizeof(short) * 8 / value_bits;

    int size = src.size();
    int packed_size = src[--size];

    int packed_start = size - packed_size;
    dst_size = packed_size * values_packed;

    dst.resize(dst_size);

    if (packed_size > 0) {
      int unpacked_index = 0;
      int dst_index = 0;
      const int shift = -6;

      for (int i = packed_start; i < size; i++) {
        auto package = src[i];
        for (int k = 0; k < values_packed; k++) {
          short val = package & value_max;
          if (val != value_max) {
            dst[dst_index++] = val + shift;
          } else {
            dst[dst_index++] = src[unpacked_index++];
          }
          package >>= value_bits;
        }
      }
    } else {
      dst = src;
      dst.resize(size);
    }
  }


  if (packer_version == 0) {
    dst = src;
  } else if (packer_version >= 1) {
    const std::vector<short int>& new_src = packer_version >= 2 ? dst : src;

    for (int i = 0; i < dst_size; i++) {
      if (i >= 2 * N_CHANNELS)
        dst[i] = new_src[i] + 2 * dst[i - N_CHANNELS] - dst[i - 2 * N_CHANNELS];
      else if (i >= N_CHANNELS)
        dst[i] = new_src[i] + 2 * dst[i - N_CHANNELS];
      else
        dst[i] = new_src[i];
    }
  }
}

