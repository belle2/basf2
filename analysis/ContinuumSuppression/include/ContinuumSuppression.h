/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Pablo Goldenzweig,                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
