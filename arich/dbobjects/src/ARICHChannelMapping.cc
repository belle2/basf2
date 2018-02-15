/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <arich/dbobjects/ARICHChannelMapping.h>
#include <iostream>
#include<iomanip>

using namespace std;
using namespace Belle2;

ARICHChannelMapping::ARICHChannelMapping()
{
  m_xy2asic.assign(N_XCHANNELS * N_YCHANNELS, -1);
  m_asic2xy.assign(N_XCHANNELS * N_YCHANNELS, -1);
}

bool ARICHChannelMapping::getXYFromAsic(unsigned asicChn, int& xChn, int& yChn) const
{
  if (asicChn > N_XCHANNELS * N_YCHANNELS - 1)  {
    B2ERROR("ARICHChannelMapping::getXYFromAsic: invalid channel asic number!");
    return false;
  }
  int chId = (int)m_asic2xy[asicChn];
  xChn = chId % N_XCHANNELS;
  yChn = chId / N_XCHANNELS;
  return true;
}

int ARICHChannelMapping::getAsicFromXY(unsigned xChn, unsigned yChn) const
{
  unsigned chId = yChn * N_XCHANNELS + xChn;
  if (chId > N_XCHANNELS * N_YCHANNELS - 1)  B2FATAL("ARICHChannelMapping::getAsicFromXY: invalid channel X,Y number!");
  return (int)m_xy2asic[chId];
}

void ARICHChannelMapping::mapXY2Asic(unsigned xChn, unsigned yChn, unsigned asicChn)
{
  if (asicChn > N_XCHANNELS * N_YCHANNELS - 1 || xChn > N_XCHANNELS - 1
      || yChn > N_YCHANNELS - 1)  B2ERROR("ARICHChannelMapping::mapXY2Asich: invalid channel number!");
  unsigned chId = yChn * N_XCHANNELS + xChn;
  m_xy2asic.at(chId) = asicChn;
  m_asic2xy.at(asicChn) = chId;
}

void ARICHChannelMapping::print()
{
  std::cout << std::endl;
  std::cout << "HAPD X,Y channel mapping to asic channels" << std::endl << std::endl;
  std::cout << "      " << setfill('-') << setw(73) << "-" << std::endl;
  for (int y = N_YCHANNELS - 1; y > -1; y--) {
    std::cout << setfill(' ') << "y " << setw(2) << y << " ";
    for (int x = 0; x < N_XCHANNELS; x++) {
      std::cout << " | " << setw(3) << getAsicFromXY(x, y);
    }
    std::cout << " |" << std::endl;
    std::cout << "      " << setfill('-') << setw(73) << "-" << std::endl;
  }
  std::cout << setfill(' ') << "       ";
  for (int x = 0; x < N_XCHANNELS; x++) std::cout << "x " << setw(2) << x << "  ";
  std::cout << std::endl;
}
