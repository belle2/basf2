/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sourav Patra, Vishal Bhardwaj                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>

namespace Belle2 {

  namespace Variable {
    // B modes
    /**
     * returns identifier of generated B+ mode
     */
    int BplusMode(const Particle*);

    /**
     * returns identifier of generated B- mode
     */
    int BminusMode(const Particle*);

    /**
     * returns identifier of generated B0 mode
     */
    int B0Mode(const Particle*);

    /**
     * returns identifier of generated B0bar mode
     */
    int Bbar0Mode(const Particle*);

    /**
     * returns identifier of generated Bs0 mode
     */
    int Bs0Mode(const Particle*);

    /**
     * returns identifier of generated Bs0bar mode
     */
    int Bsbar0Mode(const Particle*);

    // D modes
    /**
     * returns identifier of generated D*+ mode
     */
    int DstplusMode(const Particle*);

    /**
     * returns identifier of generated D*- mode
     */
    int DstminusMode(const Particle*);

    /**
     * returns identifier of generated Ds+ mode
     */
    int DsplusMode(const Particle*);

    /**
     * returns identifier of generated Ds- mode
     */
    int DsminusMode(const Particle*);

    /**
     * returns identifier of generated D+ mode
     */
    int DplusMode(const Particle*);

    /**
     * returns identifier of generated D- mode
     */
    int DminusMode(const Particle*);

    /**
     * returns identifier of generated D0 mode
     */
    int D0Mode(const Particle*);

    /**
     * returns identifier of generated D0bar mode
     */
    int Dbar0Mode(const Particle*);

    // Tau Modes
    /**
     * returns identifier of generated tau+ mode
     */
    int TauplusMode(const Particle*);

    /**
     * returns identifier of generated tau- mode
     */
    int TauminusMode(const Particle*);
  }

} // namepspace Belle2

