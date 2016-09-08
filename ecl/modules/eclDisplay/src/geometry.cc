#include <ecl/modules/eclDisplay/geometry.h>

using namespace Belle2;

int Belle2::GetCrateId(int ch_num)
{
  return ch_num / 192;
}
// Identical to GetCrateId
int Belle2::GetCollectorId(int ch_num)
{
  return GetCrateId(ch_num);
}

int Belle2::GetCrateChannelId(int ch_num)
{
  return ch_num % 192;
}

int Belle2::GetShaperId(int ch_num)
{
  return GetCrateChannelId(ch_num) / 16;
}

int Belle2::GetShaperChannelId(int ch_num)
{
  return GetCrateChannelId(ch_num) % 16;
}

int Belle2::GetChannelCount()
{
  return 6912;
}
