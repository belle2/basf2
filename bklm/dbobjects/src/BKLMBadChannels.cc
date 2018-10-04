/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/dbobjects/BKLMBadChannels.h>
#include <iostream>

using namespace Belle2;
using namespace std;

void BKLMBadChannels::appendDeadChannel(int channel)
{
  /*bool isexist=false;
  for(unsigned int ii=0;ii<m_DeadChannels.size(); ii++)
  {
  if(channel==m_DeadChannels[ii]) { isexis=true; std::cout<<"dead channel " <<channel<< " is already in DataBase"<<std::endl;}
  }
  if(!isexist)
  */
  m_DeadChannels.push_back(channel);
  std::sort(m_DeadChannels.begin(), m_DeadChannels.end());
  m_DeadChannels.erase(std::unique(m_DeadChannels.begin(), m_DeadChannels.end()),
                       m_DeadChannels.end());
}

void BKLMBadChannels::appendDeadChannel(int isForward, int sector, int layer, int plane, int strip)
{
  int channel = geometryToChannelId(isForward, sector, layer, plane, strip);
  appendDeadChannel(channel);
}

void BKLMBadChannels::appendHotChannel(int channel)
{
  m_HotChannels.push_back(channel);
  std::sort(m_HotChannels.begin(), m_HotChannels.end());
  m_HotChannels.erase(std::unique(m_HotChannels.begin(), m_HotChannels.end()),
                      m_HotChannels.end());
}

void BKLMBadChannels::appendHotChannel(int isForward, int sector, int layer, int plane, int strip)
{
  int channel = geometryToChannelId(isForward, sector, layer, plane, strip);
  appendHotChannel(channel);
}

bool BKLMBadChannels::isHotChannel(int channel) const
{
  bool isHot = false;
  for (unsigned int ii = 0; ii < m_HotChannels.size(); ii++) {
    if (channel == m_HotChannels[ii]) { isHot = true; break;}
  }
  return isHot;
}

bool BKLMBadChannels::isHotChannel(int isForward, int sector, int layer, int plane, int strip) const
{
  int channel =  geometryToChannelId(isForward, sector, layer, plane, strip);
  return isHotChannel(channel);
}

bool BKLMBadChannels::isDeadChannel(int channel) const
{
  bool isDead = false;
  for (unsigned int ii = 0; ii < m_DeadChannels.size(); ii++) {
    if (channel == m_DeadChannels[ii]) { isDead = true; break;}
  }
  return isDead;
}

bool BKLMBadChannels::isDeadChannel(int isForward, int sector, int layer, int plane, int strip) const
{
  int channel = geometryToChannelId(isForward, sector, layer, plane, strip);
  return isDeadChannel(channel);
}

int BKLMBadChannels::geometryToChannelId(int isForward, int sector, int layer, int plane, int strip) const
{
  int forOrBack = isForward ? 0 : 1;
  int channel = (forOrBack << BKLM_END_BIT)
                | ((sector - 1) << BKLM_SECTOR_BIT)
                | ((layer - 1) << BKLM_LAYER_BIT)
                | ((plane) << BKLM_PLANE_BIT)
                | ((strip - 1) << BKLM_STRIP_BIT);

  return channel;
}

void BKLMBadChannels::printHotChannels() const
{
  if (m_HotChannels.size() == 0) { cout << "none bklm hot channel was found" << endl; return; }
  cout << "here are bklm hot channels: " << endl;
  cout << "Foward" << "\t" << "sector" << "\t" << "layer" << "\t" << "phi/z " << "\t" << "strip" << endl;
  for (unsigned int ii = 0; ii < m_HotChannels.size(); ii++) {
    int isForward = (m_HotChannels[ii] & BKLM_END_MASK) == 0 ? 1 : 0;
    int sector =  m_HotChannels[ii] & BKLM_SECTOR_MASK ;
    int layer =  m_HotChannels[ii] & BKLM_LAYER_MASK ;
    int plane =  m_HotChannels[ii] & BKLM_PLANE_MASK ;
    int strip =  m_HotChannels[ii] & BKLM_STRIP_MASK ;
    cout << isForward << "\t" << sector + 1 << "\t" << layer + 1 << "\t" << plane << "\t" << strip + 1 << endl;
  }
}

void BKLMBadChannels::printDeadChannels() const
{
  if (m_DeadChannels.size() == 0) { cout << "none bklm dead channel was found" << endl; return; }
  cout << "here are bklm dead channels: " << endl;
  cout << "Foward" << "\t" << "sector" << "\t" << "layer" << "\t" << "phi/z" << "\t" << "strip" << endl;
  for (unsigned int ii = 0; ii < m_DeadChannels.size(); ii++) {
    int isForward = (m_DeadChannels[ii] & BKLM_END_MASK) == 0 ? 1 : 0;
    int sector =  m_DeadChannels[ii] & BKLM_SECTOR_MASK ;
    int layer =  m_DeadChannels[ii] & BKLM_LAYER_MASK ;
    int plane =  m_DeadChannels[ii] & BKLM_PLANE_MASK ;
    int strip =  m_DeadChannels[ii] & BKLM_STRIP_MASK ;
    cout << isForward << "\t" << sector + 1 << "\t" << layer + 1 << "\t" << plane << "\t" << strip + 1 << endl;
  }
}
