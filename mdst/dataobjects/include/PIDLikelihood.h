/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PIDLIKELIHOOD_H
#define PIDLIKELIHOOD_H

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  /**
   * Class to collect log likelihoods from TOP, ARICH, dEdx, ECL and KLM
   * aimed for output to mdst
   * includes functions to return combined likelihood probability
   */

  class PIDLikelihood : public RelationsObject {

  public:

    /**
     * Default constructor: log likelihoods and flags set to 0
     */
    PIDLikelihood();

    /**
     * Set log likelihood for a given detector and particle
     * @param det detector enumerator
     * @param part charged stable particle
     * @param logl log likelihood
     */
    void setLogLikelihood(Const::EDetector det,
                          const Const::ChargedStable& part,
                          float logl);

    /**
     * Check whether PID information from a given set of detectors is available
     * @param set a set of PID detectors
     * @return true if the given set of detectors contributed to the PID information
     */
    bool isAvailable(Const::PIDDetectorSet set) const {return  m_detectors.contains(set);}

    /**
     * Return log likelihood for a given detector set and particle
     * @param part charged stable particle
     * @param set  a set of PID detectors to use
     * @return log likelihood
     */
    float getLogL(const Const::ChargedStable& part,
                  Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;

    /**
     * Return log likelihood difference for a given detector set and particles
     * @param p1 charged stable particle
     * @param p2 charged stable particle
     * @param set  a set of PID detectors to use
     * @return log likelihood difference logL_p1 - logL_p2
     */
    float getDeltaLogL(const Const::ChargedStable& p1,
                       const Const::ChargedStable& p2,
                       Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const
    {
      return getLogL(p1, set) - getLogL(p2, set);
    }

    /**
     * Return combined likelihood probability for a particle being p1 and not p2,
     * assuming equal prior probabilities.
     * @param p1 charged stable particle
     * @param p2 charged stable particle
     * @param set  a set of PID detectors to use
     * @return likelihood probability
     */
    double getProbability(const Const::ChargedStable& p1,
                          const Const::ChargedStable& p2,
                          Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const
    {
      return getProbability(p1, p2, 1.0, set);
    }

    /**
     * Return combined likelihood probability for a particle being p1 and not p2
     * @param p1 charged stable particle
     * @param p2 charged stable particle
     * @param ratio ratio of prior probabilities (p1/p2)
     * @param set  a set of PID detectors to use
     * @return likelihood probability
     */
    double getProbability(const Const::ChargedStable& p1,
                          const Const::ChargedStable& p2,
                          double ratio,
                          Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;


    /**
     * Return combined likelihood probability for a particle according to chargedStableSet;
     * if prior fractions not given equal prior probabilities assumed.
     * @param part charged stable particle
     * @param fractions array of prior probabilities in the order defined in Const::ChargedStable
     * @param set  a set of PID detectors to use
     * @return likelihood probability (a value btw. 0 and 1)
     */
    double getProbability(const Const::ChargedStable& part,
                          const double* fractions = 0,
                          Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;

    /**
     * Return most likely particle among chargedStableSet;
     * if prior fractions not given equal prior probabilities assumed.
     * @param fractions array of prior probabilities in the order defined in Const::ChargedStable
     * @param set  a set of PID detectors to use
     * @return particle type
     */
    Const::ChargedStable getMostLikely(const double* fractions = 0,
                                       Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;

    /**
     * Prints the content of a private array of log likelihoods
     */
    void printArray() const;

    /**
     * Return HTML Info of PID Likelihoods
     */
    std::string getInfoHTML() const;

  private:

    Const::DetectorSet m_detectors;   /**< set of detectors with PID information */
    float m_logl[Const::PIDDetectors::c_size][Const::ChargedStable::c_SetSize]; /**< log likelihoods */

    /**
     * Calculate likelihood probabilities
     * @param fractions array of prior fractions (not needed to be normalized)
     * @param probabilities array of resulting probabilities
     * @param detSet  a set of PID detectors to use
     */
    void probability(double probabilities[],
                     const double* fractions,
                     Const::PIDDetectorSet detSet) const;


    ClassDef(PIDLikelihood, 3); /**< Collect log likelihoods from TOP, ARICH, dEdx, ECL and KLM. */

  };

} // end namespace Belle2

#endif
