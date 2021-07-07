/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHBadChannels.h>

#include <algorithm>

using namespace Belle2;
using namespace std;

int ARICHBadChannels::getHapdCutChannel(unsigned int i) const
{
  if (i < m_hapdCutChannels.size()) return m_hapdCutChannels[i];
  else return -1;
}


int ARICHBadChannels::getHapdBadChannel(unsigned int i) const
{
  if (i < m_hapdBadChannels.size()) return m_hapdBadChannels[i];
  else return -1;
}

void ARICHBadChannels::setHapdCutChannel(std::vector<int> channels)
{
  m_hapdCutChannels = channels;
  m_hapdListOfBadChannels.insert(m_hapdListOfBadChannels.end(), channels.begin(), channels.end());
  std::sort(m_hapdListOfBadChannels.begin(), m_hapdListOfBadChannels.end());
  m_hapdListOfBadChannels.erase(std::unique(m_hapdListOfBadChannels.begin(), m_hapdListOfBadChannels.end()),
                                m_hapdListOfBadChannels.end());
}

void ARICHBadChannels::setHapdBadChannel(std::vector<int> channels)
{
  m_hapdBadChannels = channels;
  m_hapdListOfBadChannels.insert(m_hapdListOfBadChannels.end(), channels.begin(), channels.end());
  std::sort(m_hapdListOfBadChannels.begin(), m_hapdListOfBadChannels.end());
  m_hapdListOfBadChannels.erase(std::unique(m_hapdListOfBadChannels.begin(), m_hapdListOfBadChannels.end()),
                                m_hapdListOfBadChannels.end());
}

int ARICHBadChannels::getHapdListOfBadChannel(unsigned int i) const
{
  std::vector<int> m_hapdAllBadCHs = ARICHBadChannels::getHapdListOfBadChannels();
  if (i < m_hapdAllBadCHs.size()) return m_hapdAllBadCHs[i];
  else return -1;
}

int ARICHBadChannels::getFebDeadChannel(unsigned int i) const
{
  if (i < m_febDeadChannels.size()) return m_febDeadChannels[i];
  else return -1;
}


int ARICHBadChannels::getAsicDeadChannel(unsigned int i) const
{
  if (i < m_asicDeadChannels.size()) return m_asicDeadChannels[i];
  else return -1;
}


int ARICHBadChannels::getAsicBadConnChannel(unsigned int i) const
{
  if (i < m_asicBadConnChannels.size()) return m_asicBadConnChannels[i];
  else return -1;
}


int ARICHBadChannels::getAsicBadOffsetChannel(unsigned int i) const
{
  if (i < m_asicBadOffsetChannels.size()) return m_asicBadOffsetChannels[i];
  else return -1;
}


int ARICHBadChannels::getAsicBadLinChannel(unsigned int i) const
{
  if (i < m_asicBadLinChannels.size()) return m_asicBadLinChannels[i];
  else return -1;
}

void ARICHBadChannels::setFebDeadChannels(std::vector<int> deadChannels)
{
  m_febDeadChannels = deadChannels;
  m_febListOfBadChannels.insert(m_febListOfBadChannels.end(), deadChannels.begin(), deadChannels.end());
  std::sort(m_febListOfBadChannels.begin(), m_febListOfBadChannels.end());
  m_febListOfBadChannels.erase(std::unique(m_febListOfBadChannels.begin(), m_febListOfBadChannels.end()),
                               m_febListOfBadChannels.end());
}


void ARICHBadChannels::setAsicDeadChannels(std::vector<int> deadChannels)
{
  m_asicDeadChannels = deadChannels;
  m_febListOfBadChannels.insert(m_febListOfBadChannels.end(), deadChannels.begin(), deadChannels.end());
  std::sort(m_febListOfBadChannels.begin(), m_febListOfBadChannels.end());
  m_febListOfBadChannels.erase(std::unique(m_febListOfBadChannels.begin(), m_febListOfBadChannels.end()),
                               m_febListOfBadChannels.end());
}

void ARICHBadChannels::setAsicBadConnChannels(std::vector<int> badConnChannels)
{
  m_asicBadConnChannels = badConnChannels;
  m_febListOfBadChannels.insert(m_febListOfBadChannels.end(), badConnChannels.begin(), badConnChannels.end());
  std::sort(m_febListOfBadChannels.begin(), m_febListOfBadChannels.end());
  m_febListOfBadChannels.erase(std::unique(m_febListOfBadChannels.begin(), m_febListOfBadChannels.end()),
                               m_febListOfBadChannels.end());
}

void ARICHBadChannels::setAsicBadOffsetChannels(std::vector<int> badOffsetChannels)
{
  m_asicBadOffsetChannels = badOffsetChannels;
  m_febListOfBadChannels.insert(m_febListOfBadChannels.end(), badOffsetChannels.begin(), badOffsetChannels.end());
  std::sort(m_febListOfBadChannels.begin(), m_febListOfBadChannels.end());
  m_febListOfBadChannels.erase(std::unique(m_febListOfBadChannels.begin(), m_febListOfBadChannels.end()),
                               m_febListOfBadChannels.end());
}


void ARICHBadChannels::setAsicBadLinChannels(std::vector<int> badLinChannels)
{
  m_asicBadLinChannels = badLinChannels;
  m_febListOfBadChannels.insert(m_febListOfBadChannels.end(), badLinChannels.begin(), badLinChannels.end());
  std::sort(m_febListOfBadChannels.begin(), m_febListOfBadChannels.end());
  m_febListOfBadChannels.erase(std::unique(m_febListOfBadChannels.begin(), m_febListOfBadChannels.end()),
                               m_febListOfBadChannels.end());
}

int ARICHBadChannels::getFebListOfBadChannel(unsigned int i) const
{
  std::vector<int> m_febAllBadCHs = ARICHBadChannels::getFebListOfBadChannels();
  if (i < m_febAllBadCHs.size()) return m_febAllBadCHs[i];
  else return -1;
}
