/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <analysis/VertexFitting/TreeFitter/ConstraintConfiguration.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

namespace Belle2 {
  class Particle;

  /** Module to fit an entire decay tree.
   * The newton method is used to minimize the chi2 derivative.
   * We use a kalman filter within the newton method to smooth the statevector.   */
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

    /** input particle list */
    StoreObjPtr<ParticleList> m_plist;

    /**   name of the particle list fed to the fitter  */
    std::string m_particleList;

    /** minimum confidence level to accept fit
     * calculated as f(chiSquared, NDF)
     * -2: accept all
     *  0: only accept fit survivors
     *  0.001 loose cut
     *  0.1 (too) tight cut
     *  */
    double m_confidenceLevel;

    /** convergence precision for the newton method
     * When the delta chiSquared between 2 iterations divided by the chiSquared of the previous iteration
    * is smaller than this stop the fit and call it converged
     * optimized - don't touch
     * */
    double m_precision;

    /** vector carrying the PDG codes of the particles to be mass constraint */
    std::vector<int> m_massConstraintList;

    /** list of pdg codes of particles to use a geo constraint for */
    std::vector<int> m_geoConstraintListPDG;

    /** list of pdg codes of particles where we use the same vertex for production and decay
     * which is the vertex of the mother */
    std::vector<int> m_fixedToMotherVertexListPDG;

    /** vector carrying the names of the particles to be mass constraint */
    std::vector<std::string> m_massConstraintListParticlename;
    /** type of the mass constraint false: use normal one. true: use parameters of daughters experimental!
     *  WARNING not even guaranteed that it works
     * */
    int m_massConstraintType;

    /** PDG code of particle to be constrained to the beam 4-momentum */
    int m_beamConstraintPDG;

    /** Use x-y-z beamspot constraint.
     * The Beamspot will be treated as the mother of the particle you feed,
     * thus pinning down the PRODUCTION vertex of the mother to the IP
     * */
    bool  m_ipConstraint;

    /** this fits all particle candidates contained in the m_particleList  */
    bool fitTree(Particle* head);

    /** before the fit */
    unsigned int m_nCandidatesBeforeFit;

    /** after the fit  */
    unsigned int m_nCandidatesAfter;

    /** flag if you want to update all particle momenta in the decay tree.
     * False means only the head of the tree will be updated
     * */
    bool m_updateDaughters;

    /** use a custom vertex as the production vertex of the highest hierarchy particle
     * */
    bool m_customOrigin;

    /** linearise around a previous state of the Kalman Filter */
    bool m_useReferencing;

    /** vertex coordinates of the custom origin  */
    std::vector<double> m_customOriginVertex;

    /** covariance of the custom origin */
    std::vector<double> m_customOriginCovariance;

    /** list of constraints not to be applied in tree fit
     *  WARNING only use if you know what you are doing
     * */
    std::vector<std::string> m_removeConstraintList;

    /** should the vertex be joined with the mother and should it be geometrically constrained?
     *  'I dont know hat I am doing'
     * */
    bool m_automatic_vertex_constraining;

    /** dimension to use for beam/origin constraint  */
    int m_originDimension;

    /** inflate beamspot covariance of z by this number */
    int m_inflationFactorCovZ;

    /** beam four-momentum */
    Eigen::Matrix<double, 4, 1> m_beamMomE;

    /** beam covariance matrix */
    Eigen::Matrix<double, 4, 4> m_beamCovariance;

    /** decay string to select one particle that will be treated as invisible */
    std::string  m_treatAsInvisible;

    /** decay string to select one particle that will be ignored to determine the vertex position */
    std::string  m_ignoreFromVertexFit;

    /** Decay descriptor of the invisible particles */
    DecayDescriptor m_pDDescriptorInvisibles;

    /** Decay descriptor of the ignored particles */
    DecayDescriptor m_pDDescriptorForIgnoring;

    /** StoreArray of Particles */
    StoreArray<Particle> m_particles;

  };
}
