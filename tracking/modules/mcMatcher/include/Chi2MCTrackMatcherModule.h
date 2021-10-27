/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  /**
   * Monte Carlo matcher using the helix parameters for matching by chi2-method
   */
  class Chi2MCTrackMatcherModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    Chi2MCTrackMatcherModule();


    /** Register input and output data */
    void initialize() override;


    /** Do matching for each event */
    void event() override;

    /** Provides debug statistics*/
    void terminate() override;
  private: //Parameters

    /** Parameter : Defines the chi2 cut-off values for each charged particle. This cut-off values define
     *              weather a matching candidate pair`s chi2-value is reasonable.
     *              The cut-off values in the list for the  different pdgs have to be in the
     *              following order  [11,13,211,2212,321,1000010020].
     *              The default values determined from a small study investigating the chi2 value destribution
     *              from Track-MCParticle pairs for events with only one Track and one MCParticle. Latter
     *              makes the matching trivial and therefore one can determine the so-called 99% border that
     *              gives the cut-off value for each charged stable particle.
     */
    std::vector<double> m_param_CutOffs{128024, 95, 173, 424, 90, 424};
    /** Parameter: Possibility to switch beween ROOT and Eigen for inversion of the covariance matrix
     *  false: ROOT is used for matrix inversion
     *  true: Eigen is used for matrix inversion
     */
    bool m_param_linalg;
    /** Variable: Makes m_MCParticles available in whole class */
    StoreArray<MCParticle> m_MCParticles;
    /** Variable: Makes m_Tracks available in whole class */
    StoreArray<Track> m_Tracks;

    /** Variables for Debug module statistics */
    /** Variable: Counts the total number of not invertable Covariance5 matrices.*/
    int m_notInvertableCount = 0;
    /** Variable: Counts the number of tracks without a relation.*/
    int m_noMatchCount = 0;
    /** Variable: Counts the total number of tracks to calculate a percentage feedback.*/
    int m_trackCount = 0;
    /** Variable: Counts the total number of Covaraince5 matrices.*/
    int m_covarianceMatrixCount = 0;
    /** Variable: Counts the total number of tracks without a possible MCParticle as matching candidate*/
    int m_noMatchingCandidateCount = 0;
    /** Variable: Counts total event number.*/
    int m_event = 0;
  };
}
