/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <arich/dbobjects/ARICHChannelMask.h>
#include <iostream>

using namespace std;
using namespace Belle2;

ARICHChannelMask::ARICHChannelMask()
{
  std::fill_n(m_DetectorMask, N_HAPDS * N_CHANNELS / 32 + 1, 0xFFFFFFFF);
}

void ARICHChannelMask::setActiveCh(unsigned module, unsigned channel, bool active)
{
  if (module > N_HAPDS || channel > N_CHANNELS) { B2ERROR("ARICHChannelMask::setActiveCh: module ID / channel ID out of range!"); return;}
  int ch  = (module - 1) * N_CHANNELS + channel;
  int bit = ch % 32;
  unsigned int idx = ch / 32;
  if (active) m_DetectorMask[idx] |= (1 << bit);
  else        m_DetectorMask[idx] &= ~(1 << bit);
}

void ARICHChannelMask::setActiveHAPD(unsigned module, bool active)
{
  for (int i = 0; i < N_CHANNELS; i++) {
    setActiveCh(module, i, active);
  }
}

void ARICHChannelMask::setActiveAPD(unsigned module, unsigned apd, bool active)
{
  int fst = N_CHANNELS / 4 * apd;
  for (int i = fst; i < fst + N_CHANNELS / 4; i++) {
    setActiveCh(module, i, active);
  }
}

bool ARICHChannelMask::isActive(unsigned moduleID, unsigned channelID) const
{
  int ch = (moduleID - 1) * N_CHANNELS + channelID;
  int bit = ch % 32;
  unsigned int idx = ch / 32;
  if (m_DetectorMask[idx] & (1 << bit)) return true;
  return false;
}

void ARICHChannelMask::print() const
{

  cout << endl << "ARICH masked channels" << endl << endl;
  for (int i = 0; i <  N_HAPDS; i++) {
    int first = 1;
    //cout << "Module "<< i+1 << ": " ;
    for (int j = 0; j < N_CHANNELS; j++) {
      if (!isActive(i + 1, j)) {
        if (first) cout << "Module " << i + 1 << ": " ;
        cout << j << " ";
        first = 0;
      }
    }
    if (!first) cout << endl << endl;

  }
}
