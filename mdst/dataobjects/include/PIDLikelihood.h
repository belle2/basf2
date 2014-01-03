/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PIDLIKELIHOOD_H
#define PIDLIKELIHOOD_H

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  /** \addtogroup dataobjects
   * @{
   */

  /**
   * Class to collect log likelihoods from TOP, ARICH, dEdx, ECL and KLM
   * aimed for output to mdst
   * includes a function to return combined likelihood probability (like Belle1 atc_pid)
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
     * @param set  set of detector IDs
     * @return     true if the given set of detectors contributed to the PID information
     */
    bool isAvailable(Const::PIDDetectorSet set) const {return  m_detectors.contains(set);}

    /**
     * Return log likelihood for a given detector set and particle
     * @param set  set of detector IDs
     * @param part particle type
     * @return     log likelihood
     */
    float getLogL(const Const::ChargedStable& part, Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;

    /**
     * Return combined likelihood probability for particle being p1 and not p2 assuming equal prior probablilites
     * @param p1 particle enumerator
     * @param p2 particle enumerator
     * @param set set of detector IDs
     * @return likelihood probability P_{p1/p2} (a value btw. 0 and 1)
     */
    double getProbability(const Const::ChargedStable& p1, const Const::ChargedStable& p2, Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;

  private:

    enum {c_PIDDetectorSetSize = 6}; /**< temporary solution for the size */

    Const::DetectorSet m_detectors;   /**< set of detectors with PID information */
    float m_logl[c_PIDDetectorSetSize][Const::ChargedStable::c_SetSize]; /**< log likelihoods, FIXME: replace hard coded value */

    /**
     * Calculate likelihood probability from log likelihood difference logl1-logl2 assuming equal prior probablilites
     * @param logl1 log likelihood
     * @param logl2 log likelihood
     * @return likelihood probability (a value btw. 0 and 1)
     */
    double probability(float logl1, float logl2) const;

    ClassDef(PIDLikelihood, 2); /**< class definition */

  };

} // end namespace Belle2

#endif
