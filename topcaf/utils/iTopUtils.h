#ifndef iTopUtils_H
#define iTopUtils_H

#include <iostream>
#include <sstream>


template <typename T> std::string NumberToString(T Number)
{
  std::ostringstream ss;
  ss << Number;
  return ss.str();
}

unsigned int pixelNumber_to_pixelRow(unsigned int pixel)
{
  return (pixel - 1) / 64 + 1;
}
unsigned int pixelNumber_to_pixelColumn(unsigned int pixel)
{
  return (pixel - 1) % 64 + 1;
}
unsigned int pixelNumber_to_PMTNumber(unsigned int pixel)
{
  return ((pixel - 1) / 4) % 16 + ((pixel - 1) / 256) * 16 + 1;
}
unsigned int pixelNumber_to_channelNumber(unsigned int pixel)
{
  return (pixel - 1) % 4 + (((pixel - 1) % 256) / 64) * 4 + 1;
}
unsigned int channelNumber_to_pixelNumber(unsigned int PMT, unsigned int channel)
{
  return (channel - 1) % 4 + (PMT - 1) * 4 + ((channel - 1) / 4) * 64 + ((PMT - 1) / 16) * 192 + 1;
}
unsigned int pixel_to_electronicsModuleNumber(unsigned int pixel)
{
  return ((pixel - 1) % 64) / 16;
}
unsigned int pixel_to_asicRow(unsigned int pixel)
{
  return (pixel - 1) / 128;
}
unsigned int pixel_to_asicColumn(unsigned int pixel)
{
  return ((pixel - 1) % 16) / 4;
}
unsigned int pixel_to_asicChannel(unsigned int pixel)
{
  return 2 * ((3 - (pixel - 1) % 4)) + ((pixel - 1) % 128) / 64;
}
unsigned int asic_to_pixel(unsigned int electronicsModule, unsigned int asicRow, unsigned int asicColumn, unsigned int asicChannel)
{
  return 4 - (asicChannel % 8) / 2 + (asicChannel % 2) * 64 + asicColumn * 4 + electronicsModule * 16 + asicRow * 128;
}
int electronics_to_pixel(int boardstack, int carrier, int asic, int channel)
{
  return 1 + 128 * carrier + channel * (asic % 2) + (71 - channel) * ((asic + 1) % 2) + 8 * ((asic / 2) % 2) + 16 * boardstack;
}

#endif
