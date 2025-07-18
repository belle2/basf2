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
  double SVDClusterCharge(const Particle*, const std::vector<double>& indices);
  double SVDClusterSNR(const Particle*, const std::vector<double>& indices);
  int SVDClusterSize(const Particle*, const std::vector<double>& indices);
  double SVDClusterTime(const Particle*, const std::vector<double>& indices);
  double SVDTrackPrime(const Particle*, const std::vector<double>& indices);
  double SVDTrackPositionErrorUnbiased(const Particle*, const std::vector<double>& indices);
  double SVDTruePosition(const Particle*, const std::vector<double>& indices);
  double SVDResidual(const Particle*, const std::vector<double>& indices);
  int SVDLayer(const Particle*, const std::vector<double>& indices);
  int SVDLadder(const Particle*, const std::vector<double>& indices);
  int SVDSensor(const Particle*, const std::vector<double>& indices);
  bool SVDSide(const Particle*, const std::vector<double>& indices);
}