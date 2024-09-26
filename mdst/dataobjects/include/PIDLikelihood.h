/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

#include <map>
#include <string>

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
     * Subtract the maximum of log likelihoods of each detector component
     * in order to reduce the range of values.
     */
    void subtractMaximum();

    /**
     * Check whether PID information is available for at least one of the detectors in a given set.
     * Redefined after release-8.
     * @param set a set of PID detectors
     * @return true if at least one of the detectors in the set provides likelihoods
     */
    bool isAvailable(Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;

    /**
     * Check whether PID information is available for all detectors in a given set.
     * Was named isAvailable in release-8 or older.
     * @param set a set of PID detectors
     * @return true if all detectors in the set provide likelihoods
     */
    bool areAllAvailable(Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const
    {
      return  m_detectors.contains(set);
    }

    /**
     * Return log likelihood for a given detector set and particle
     * @param part charged stable particle
     * @param set  a set of PID detectors to use
     * @return log likelihood
     */
    float getLogL(const Const::ChargedStable& part,
                  Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;

    /**
     * Return log likelihood difference between two particles for a given detector set
     * @param p1 charged stable particle
     * @param p2 charged stable particle
     * @param set  a set of PID detectors to use
     * @return log likelihood difference between p1 and p2: logL(p1) - logL(p2)
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
     * @return binary likelihood probability
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
     * @return binary likelihood probability
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
     * @return global likelihood probability
     */
    double getProbability(const Const::ChargedStable& part,
                          const double* fractions = nullptr,
                          Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;

    /**
     * Return logarithmic equivalent of likelihood probability defined as log(p/(1-p)),
     * where p is the combined likelihood probability of a particle according to chargedStableSet.
     * This one gives a smooth peak-like distribution (opposite to probability distribution which has spikes at 0 and 1).
     * If prior fractions are not given, equal prior probabilities are assumed.
     * @param part charged stable particle
     * @param fractions array of prior probabilities in the order defined in Const::ChargedStable
     * @param set  a set of PID detectors to use
     * @return logarithmic equivalent of global likelihood probability
     */
    double getLogarithmicProbability(const Const::ChargedStable& part,
                                     const double* fractions = nullptr,
                                     Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;

    /**
     * Return most likely particle among chargedStableSet;
     * if prior fractions not given equal prior probabilities assumed.
     * @param fractions array of prior probabilities in the order defined in Const::ChargedStable
     * @param set  a set of PID detectors to use
     * @return most likely particle
     */
    Const::ChargedStable getMostLikely(const double* fractions = nullptr,
                                       Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;
    /**
     * Prints the content of a private array of log likelihoods
     */
    void printArray() const;

    /**
     * Return HTML Info of PID Likelihoods
     */
    std::string getInfoHTML() const override;

    /**
     * Add the pre-official likelihood
     * @param preOfficialIdentifier Name of the pre-official likelihood
     * @param preOfficialLikelihood Value of the pre-official likelihood
     */
    void addPreOfficialLikelihood(const std::string& preOfficialIdentifier,
                                  const double preOfficialLikelihood);

    /**
     * Get the pre-official likelihood
     * @param preOfficialIdentifier Name of the pre-official likelihood
     * @return Value of the pre-official likelihood
     */
    double getPreOfficialLikelihood(const std::string& preOfficialIdentifier) const;

  private:

    /**
     * Calculate likelihood probabilities
     * @param fractions array of prior fractions (not needed to be normalized)
     * @param probabilities array of resulting probabilities
     * @param detSet  a set of PID detectors to use
     */
    void probability(double probabilities[],
                     const double* fractions,
                     Const::PIDDetectorSet detSet) const;

    Const::DetectorSet m_detectors;   /**< set of detectors with PID information */
    float m_logl[Const::PIDDetectors::c_size][Const::ChargedStable::c_SetSize]; /**< log likelihoods */

    /// Internal storage of pre-official likelihood.
    std::map<std::string, double> m_preOfficialLikelihoods;

    ClassDefOverride(PIDLikelihood, 4); /**< Collect log likelihoods from TOP, ARICH, dEdx, ECL and KLM. */

  };

} // end namespace Belle2
