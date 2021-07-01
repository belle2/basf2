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

  namespace Variable {
    // B modes
    /**
     * returns identifier of generated B+ mode
     */
    double BplusMode(const Particle*);

    /**
     * returns identifier of generated B- mode
     */
    double BminusMode(const Particle*);

    /**
     * returns identifier of generated B0 mode
     */
    double B0Mode(const Particle*);

    /**
     * returns identifier of generated B0bar mode
     */
    double Bbar0Mode(const Particle*);

    /**
     * returns identifier of generated Bs0 mode
     */
    double Bs0Mode(const Particle*);

    /**
     * returns identifier of generated Bs0bar mode
     */
    double Bsbar0Mode(const Particle*);

    // D modes
    /**
     * returns identifier of generated D*+ mode
     */
    double DstplusMode(const Particle*);

    /**
     * returns identifier of generated D*- mode
     */
    double DstminusMode(const Particle*);

    /**
     * returns identifier of generated Ds+ mode
     */
    double DsplusMode(const Particle*);

    /**
     * returns identifier of generated Ds- mode
     */
    double DsminusMode(const Particle*);

    /**
     * returns identifier of generated D+ mode
     */
    double DplusMode(const Particle*);

    /**
     * returns identifier of generated D- mode
     */
    double DminusMode(const Particle*);

    /**
     * returns identifier of generated D0 mode
     */
    double D0Mode(const Particle*);

    /**
     * returns identifier of generated D0bar mode
     */
    double Dbar0Mode(const Particle*);

    // Tau Modes
    /**
     * returns identifier of generated tau+ mode
     */
    double TauplusMode(const Particle*);

    /**
     * returns identifier of generated tau- mode
     */
    double TauminusMode(const Particle*);
  }

} // namepspace Belle2

