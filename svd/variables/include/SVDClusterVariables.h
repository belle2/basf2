/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <string>
#include <vector>

namespace Belle2::Variable {
  Manager::FunctionPtr SVDClusterCharge(const std::vector<std::string>& arguments);
  Manager::FunctionPtr SVDClusterSNR(const std::vector<std::string>& arguments);
  Manager::FunctionPtr SVDClusterSize(const std::vector<std::string>& arguments);
  Manager::FunctionPtr SVDClusterTime(const std::vector<std::string>& arguments);
  Manager::FunctionPtr SVDTrackPrime(const std::vector<std::string>& arguments);
  Manager::FunctionPtr SVDTrackPositionErrorUnbiased(const std::vector<std::string>& arguments);
  Manager::FunctionPtr SVDTruePosition(const std::vector<std::string>& arguments);
  Manager::FunctionPtr SVDResidual(const std::vector<std::string>& arguments);
  Manager::FunctionPtr SVDLayer(const std::vector<std::string>& arguments);
  Manager::FunctionPtr SVDLadder(const std::vector<std::string>& arguments);
  Manager::FunctionPtr SVDSensor(const std::vector<std::string>& arguments);
  Manager::FunctionPtr SVDSide(const std::vector<std::string>& arguments);
}