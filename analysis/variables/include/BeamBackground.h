/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {

  class Particle;

  namespace Variable {

    /**
     * Return the reuse rate of the beam background overlay sample.
     */
    int beamBackgroundReuseRate(const Particle*);

    /**
     * Return the number of events in the beam background overlay sample.
     */
    int beamBackgroundEvents(const Particle*);

  }

}
