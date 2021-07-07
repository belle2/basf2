/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHAsicInfo.h>

using namespace Belle2;
using namespace std;

int ARICHAsicInfo::getDeadChannel(unsigned int i) const
{
  if (i < m_deadChannel.size()) return m_deadChannel[i];
  else return -1;
}


int ARICHAsicInfo::getBadConnChannel(unsigned int i) const
{
  if (i < m_badConnChannel.size()) return m_badConnChannel[i];
  else return -1;
}


int ARICHAsicInfo::getBadOffsetChannel(unsigned int i) const
{
  if (i < m_badOffsetChannel.size()) return m_badOffsetChannel[i];
  else return -1;
}


int ARICHAsicInfo::getBadLinChannel(unsigned int i) const
{
  if (i < m_badLinChannel.size()) return m_badLinChannel[i];
  else return -1;
}


TH3F* ARICHAsicInfo::getGainMeasurement(unsigned int i) const
{
  if (i < m_gain.size()) return m_gain[i];
  else return NULL;
}


void ARICHAsicInfo::setGainMeasurement(std::vector<TH3F*> gain)
{
  for (unsigned int i = 0; i < gain.size(); i++) m_gain.push_back(gain[i]);
}


TH3F* ARICHAsicInfo::getOffsetMeasurement(unsigned int i) const
{
  if (i < m_offset.size()) return m_offset[i];
  else return NULL;
}


void ARICHAsicInfo::setOffsetMeasurement(std::vector<TH3F*> offset)
{
  for (unsigned int i = 0; i < offset.size(); i++) m_offset.push_back(offset[i]);
}


