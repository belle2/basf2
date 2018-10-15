#include <ecl/dbobjects/ECLDspData.h>

using namespace Belle2;

void ECLDspData::packCoefVector(const std::vector<short int>& src, std::vector<short int>& dst)
{
  const int N_CHANNELS = 16;
  int size = src.size();

  dst.resize(size);
  for (int i = size - 1; i >= 0; i--) {
    if (i >= N_CHANNELS)
      dst[i] = src[i] - src[i - N_CHANNELS];
    else
      dst[i] = src[i];
  }
}
void ECLDspData::unpackCoefVector(const std::vector<short int>& src, std::vector<short int>& dst)
{
  const int N_CHANNELS = 16;
  int size = src.size();

  dst.resize(size);
  for (int i = 0; i < size; i++) {
    if (i < N_CHANNELS)
      dst[i] = src[i];
    else
      dst[i] = src[i] + src[i - N_CHANNELS];
  }
}

