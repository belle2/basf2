/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <analysis/dataobjects/Particle.h>


namespace Belle2 {
  class Particle;

  /**
   * Adds continuum suppression variables
   */
  void addContinuumSuppression(const Particle* particle, const std::string& maskName);

}
