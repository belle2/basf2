#include <ecl/dbobjects/ECLDspData.h>

using namespace Belle2;

void ECLDspData::packCoefVector(const std::vector<short int>& src, std::vector<short int>& dst)
{
  const int N_CHANNELS = 16;
  int size = src.size();

  if (getPackerVersion() == 0) {
    dst = src;
  } else if (getPackerVersion() == 1) {
    // Apply reversible transformation to shrink the range
    // of saved coefficients into much smaller interval.
    // This reduces payload size from 200 MB to 75 MB.

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
}
void ECLDspData::unpackCoefVector(const std::vector<short int>& src, std::vector<short int>& dst) const
{
  const int N_CHANNELS = 16;
  int size = src.size();
  int packer_version = m_extraData[1];

  if (packer_version == 0) {
    dst = src;
  } else if (packer_version == 1) {
    dst.resize(size);
    for (int i = 0; i < size; i++) {
      if (i >= 2 * N_CHANNELS)
        dst[i] = src[i] + 2 * dst[i - N_CHANNELS] - dst[i - 2 * N_CHANNELS];
      else if (i >= N_CHANNELS)
        dst[i] = src[i] + 2 * dst[i - N_CHANNELS];
      else
        dst[i] = src[i];
    }
  }
}

