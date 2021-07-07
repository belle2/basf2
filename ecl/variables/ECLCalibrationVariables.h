/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * return 1.0 if cluster matched to a trigger cluster
     */
    double eclClusterTrigger(const Particle* particle);

    /**
     * return three digit energy sum FWD barrel
     */
    double eclEnergy3FWDBarrel(const Particle* particle);

    /**
     * return three digit energy sum FWD endcap
     */
    double eclEnergy3FWDEndcap(const Particle* particle);

    /**
     * return three digit energy sum BWD barrel
     */
    double eclEnergy3BWDBarrel(const Particle* particle);

    /**
     * return three digit energy sum BWD endcap
     */
    double eclEnergy3BWDEndcap(const Particle* particle);

    /**
     * return the number of TCs above threshold
     */
    double getNumberOfTCs(const Particle*, const std::vector<double>& vars);

    /**
     * return the TC energy
     */
    double getEnergyTC(const Particle*, const std::vector<double>& vars);

    /**
     * return the TC energy based on ECLCalDigits
     */
    double getEnergyTCECLCalDigit(const Particle*, const std::vector<double>& vars);

    /**
     * return the TC timing
     */
    double getTimingTC(const Particle*, const std::vector<double>& vars);

    /**
     * return the TC event timing
     */
    double getEvtTimingTC(const Particle*);

    /**
     * return the  TC Id with maximum FADC
     */
    double getMaximumTCId(const Particle*);

    /**
     * returns the TC hit window
     */
    double eclHitWindowTC(const Particle*, const std::vector<double>& vars);

    /**
     * return the TC timing based on ECLCalDigits
     */
    double getTimingTCECLCalDigit(const Particle*, const std::vector<double>& vars);

    /**
    * return the TC energy sum
    */
    double eclEnergySumTC(const Particle*, const std::vector<double>& vars);

    /**
    * return the TC energy sum based on ECLCalDigits
    */
    double eclEnergySumTCECLCalDigit(const Particle*, const std::vector<double>& vars);

    /**
     * return the TC energy sum based on ECLCalDigits that are in ECLClusters above threshold
     */
    double eclEnergySumTCECLCalDigitInECLCluster(const Particle*);

    /**
     * return the energy sum based on ECLCalDigits that are in ECLClusters above threshold
     */
    double eclEnergySumECLCalDigitInECLCluster(const Particle*);

    /**
    * return the threshold TC energy sum based on ECLCalDigits that are in ECLClusters
    */
    double eclEnergySumTCECLCalDigitInECLClusterThreshold(const Particle*);

    /**
     * Returns the number of TCs above threshold
     */
    double eclNumberOfTCsForCluster(const Particle* particle, const std::vector<double>& vars);

    /**
     * Returns the FADC sum above threshold for given hit windows
     */
    double eclTCFADCForCluster(const Particle* particle, const std::vector<double>& vars);

    /**
     * Returns true if cluster is related to maximum TC
     */
    double eclTCIsMaximumForCluster(const Particle* particle);

  }
} // Belle2 namespace
