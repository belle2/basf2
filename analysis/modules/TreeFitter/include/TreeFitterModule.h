/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/core/Module.h>

namespace Belle2 {
  class Particle;

  /** Module to fit an entire decay tree.
   * The newton method is used to minimize the chi2 derivative.
   * We use a kalman filter witihin the newton method to smooth the statevector.   */
  class TreeFitterModule : public Module {

  public:
    /** constructor   */
    TreeFitterModule();

    /**   initialize  */
    virtual void initialize() override;

    /** performed at the start of run */
    virtual void beginRun() override;

    /** performed for each event   */
    virtual void event() override;

    /** stuff at the end */
    virtual void terminate() override;

  private:

    /** plot ascii art and statistics */
    void plotFancyASCII();

    /**   name of the particle list fed to the fitter  */
    std::string m_particleList;

    /** minimum confidence level to accept fit   */
    double m_confidenceLevel;

    /** convergence precision for the newton method  */
    double m_precision;

    /**  babar verbosity to be removed  */
    int m_verbose;

    /** unused    */
    std::vector<int> m_massConstraintList;

    /** Use x-y-z beamspot constraint (int 3) or x-y beamtube constraint (int 2) or nothing (default int 0).
     * The Beamspot will be treated as the mother of the particle you feed. */
    int m_ipConstraintDimension;

    /** this fits all particle candidates contained in the m_particleList  */
    bool fitTree(Particle* head);

    /** before the fit */
    unsigned int m_nCandidatesBeforeFit;

    /** after the fit  */
    unsigned int m_nCandidatesAfter;

    /** flag if you want to update all particles in the decay tree.
     * False means only the head of the tree will be updated */
    bool m_updateDaughters;
  };
}
