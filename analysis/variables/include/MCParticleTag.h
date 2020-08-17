/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Patra, Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <analysis/dataobjects/Particle.h>
#include <vector>


namespace Belle2 {
  class Particle;

  namespace Variable {
    // B modes
    int BplusMode(const Particle* part);
    int BminusMode(const Particle* part);
    int B0Mode(const Particle* part);
    int B0Mode(const Particle* part);
    int Bs0Mode(const Particle* part);
    int Bsbar0Mode(const Particle* part);
    // D modes
    int DstplusMode(const Particle* part);
    int DstminusMode(const Particle* part);
    int DsplusMode(const Particle* part);
    int DsminusMode(const Particle* part);
    int DplusMode(const Particle* part);
    int DminusMode(const Particle* part);
    int D0Mode(const Particle* part);
    int Dbar0Mode(const Particle* part);
    // Tau Mode
    int TauplusMode(const Particle* part);
    int TauminusMode(const Particle* part);
  }

} // namepspace Belle2


