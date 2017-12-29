/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include <arich/dbobjects/ARICHBiasVoltages.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;

ARICHBiasVoltages::ARICHBiasVoltages()
{
}

std::vector<int> ARICHBiasVoltages::getBiasVoltages(const std::string& hapdID) const
{
  return m_hapd2voltages.find(hapdID)->second;
}

void ARICHBiasVoltages::addVoltages(const std::string& hapdID, std::vector<int> biasVoltages)
{
  m_hapd2voltages.insert(std::pair<std::string, std::vector<int>>(hapdID, biasVoltages));
}

void ARICHBiasVoltages::print()
{

  for (auto const& x : m_hapd2voltages)  {
    std::vector<int> voltages = x.second;
    B2INFO("voltages: " << x.first << ": bias A = " << voltages[0] << ", bias B = " << voltages[1] << ", bias C = " << voltages[2] <<
           ", bias D = " << voltages[3] << ", guard = " << voltages[4]);
  }
}

