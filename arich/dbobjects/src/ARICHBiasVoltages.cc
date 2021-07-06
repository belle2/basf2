/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <arich/dbobjects/ARICHBiasVoltages.h>
#include <iostream>

using namespace std;
using namespace Belle2;

ARICHBiasVoltages::ARICHBiasVoltages()
{
}

std::vector<int> ARICHBiasVoltages::getBiasVoltages(const std::string& hapdID) const
{
  if (hapdID != "") {
    return m_hapd2voltages.find(hapdID)->second;
  } else {
    return std::vector<int>(5, 0);
  }

}

void ARICHBiasVoltages::addVoltages(const std::string& hapdID, std::vector<int> biasVoltages)
{
  m_hapd2voltages.insert(std::pair<std::string, std::vector<int>>(hapdID, biasVoltages));
}

void ARICHBiasVoltages::print() const
{

  for (auto const& x : m_hapd2voltages)  {
    std::vector<int> voltages = x.second;
    B2INFO("voltages: " << x.first << ": bias A = " << voltages[0] << ", bias B = " << voltages[1] << ", bias C = " << voltages[2] <<
           ", bias D = " << voltages[3] << ", guard = " << voltages[4]);
  }
}

