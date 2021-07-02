/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <arich/dbobjects/ARICHModulesInfo.h>
#include <iostream>
#include<iomanip>

using namespace std;
using namespace Belle2;

ARICHModulesInfo::ARICHModulesInfo()
{
  m_ChannelQE.assign(N_HAPDS * N_CHANNELS, 0);
  std::fill_n(m_installed, N_HAPDS, false);
  std::fill_n(m_active, N_HAPDS, false);
}

double ARICHModulesInfo::getChannelQE(unsigned moduleID, unsigned channelID) const
{
  if (moduleID > N_HAPDS
      || channelID > N_CHANNELS - 1) B2ERROR("ARICHModulesInfo::getChannelQE: module ID or channel ID out of range");

  int id = (moduleID - 1) * N_CHANNELS + channelID;
  return (double)m_ChannelQE[id] / 100.0;
}

void ARICHModulesInfo::setChannelQE(unsigned modId, unsigned chId, double qe)
{
  if (modId > N_HAPDS || chId > N_CHANNELS - 1) B2ERROR("ARICHModulesInfo::setChannelQE: module ID or channel ID out of range");

  int chid = (modId - 1) * N_CHANNELS + chId;
  m_ChannelQE[chid] = uint8_t(qe * 100 + 0.5);
}

void ARICHModulesInfo::addModule(unsigned modId, std::vector<float>& qeList, bool active)
{

  if (modId > N_HAPDS) B2ERROR("ARICHModulesInfo::addModule: module ID is out of range");
  if (qeList.size() != N_CHANNELS)  B2ERROR("ARICHModulesInfo::addModule: incomplete list of channels QEs");

  m_installed[modId - 1] = true;
  m_active[modId - 1] = active;

  std::vector<uint8_t>::iterator it = m_ChannelQE.begin() + (modId - 1) * N_CHANNELS;
  for (auto qe : qeList) {
    *it = uint8_t(qe + 0.5);
    ++it;
  }
}

bool ARICHModulesInfo::isInstalled(unsigned modId) const
{
  if (modId > N_HAPDS) B2ERROR("ARICHModulesInfo::isInstalled: module ID is out of range");

  return m_installed[modId - 1];
}

bool ARICHModulesInfo::isActive(unsigned modId) const
{
  if (modId > N_HAPDS) B2ERROR("ARICHModulesInfo::isActive: module ID is out of range");

  return m_active[modId - 1];
}

void ARICHModulesInfo::print() const
{
  cout << "ARICHModulesInfo: " << endl;
  cout << " Modules status " << endl;
  cout << " ID / installed / active / avg. QE" << endl;
  double maxQE = 0;
  for (int i = 1; i < N_HAPDS + 1; i++) {
    cout << right << setfill(' ') << setw(4) << i << ": ";
    if (isInstalled(i)) cout << 1;
    else cout << 0;
    cout << " ";
    if (isActive(i)) cout << 1;
    else cout << 0;
    double avgQE = 0;
    for (int j = 0; j < N_CHANNELS; j++) {
      double chQE =  getChannelQE(i, j);
      avgQE += chQE;
      if (chQE > maxQE) maxQE = chQE;
    }
    avgQE /= double(N_CHANNELS);
    cout << " " << left << setfill('0') << setw(4) << setprecision(2) << avgQE << "  ";
    if ((i) % 10 == 0) cout << endl;
  }
  cout << " Maximal channel QE: " << maxQE << endl;
}
