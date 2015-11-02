/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc, Thomas Keck                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * returns true if event doesn't contain an Y(4S)
     */
    double isContinuumEvent(const Particle*);

    /**
     * return number of tracks in event
     */
    double nTracks(const Particle*);

    /**
     * return number of ECL clusters in event
     */
    double nECLClusters(const Particle*);

    /**
     * return number of KLM clusters in event
     */
    double nKLMClusters(const Particle*);

    /**
     * return total energy in ECL clusters in event
     */
    double ECLEnergy(const Particle*);

    /**
     * return total energy in KLM clusters in event
     */
    double KLMEnergy(const Particle*);

    /**
    * return experiment number
    */
    double expNum(const Particle*);

    /**
    * return event number
    */
    double evtNum(const Particle*);

    /**
    * return run number
    */
    double runNum(const Particle*);

  }
} // Belle2 namespace

