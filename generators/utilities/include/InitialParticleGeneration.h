/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/utilities/MultivariateNormalGenerator.h>
#include <framework/dbobjects/BeamParameters.h>
#include <framework/dataobjects/MCInitialParticles.h>

#include <framework/utilities/ConditionalGaussGenerator.h>

#include <TMatrixDSym.h>

namespace Belle2 {

  /** Generate Collision.
   * This class is meant to be a common interface to be used by physics
   * generators for generating the initial beam collision.
   *
   * A generator module should have this class as a member. In each event, a call
   * to generate() will return the initial event information and also
   * automatically put it in the datastore.
   */
  class InitialParticleGeneration {
  public:
    /** constructor
     * @param allowedFlags allowed generation flags from MCInitialParticles.
     *        Default is to allow no smearing. Generation in CMS is always permitted
     */
    explicit InitialParticleGeneration(int allowedFlags = 0): m_allowedFlags(allowedFlags | MCInitialParticles::c_generateCMS) {};

    /** Generate a new event */
    MCInitialParticles& generate();

    /** Generate vertex position and possibly update the generator of Lorentz transformation */
    ROOT::Math::XYZVector getVertexConditional();

    /** Update the vertex position:
     *
     * 1. If there is no initial particles object generate a new one with nominal values
     *    without smearing
     * 2. If the BeamParameters disallow smearing of the vertex it does nothing
     * 3. If initial particles already exist check if the vertex smearing has already
     *    been applied. If not, apply vertex smearing if allowed.
     * 4. Return the **shift** in vertex to the previous value
     *    (or the origin if there was no previous value).
     *
     * This function does not update the energies as this would possibly introduce
     * inconsistency between values used by the generator and the values contained
     * in the initial particles. But it is useful to smear the vertex after generation.
     *
     * @param force if true the vertex will be regenerated even if vertex smearing
     *              was already applied.
     */
    ROOT::Math::XYZVector updateVertex(bool force = false);

    /** Return reference to nominal beam parameters */
    const BeamParameters& getBeamParameters() const { return *m_beamParams; }

    /** function to be executed on initialize() */
    void initialize();

    /** Set allowed flags. */
    void setAllowedFlags(int allowedFlags)
    {
      m_allowedFlags = allowedFlags | MCInitialParticles::c_generateCMS;
    }

    /** Initialize the conditional generator using HER & LER 4-vectors and HER & LER covariance matrices describing spread */
    ConditionalGaussGenerator initConditionalGenerator(const ROOT::Math::PxPyPzEVector& pHER,  const ROOT::Math::PxPyPzEVector& pLER,
                                                       const TMatrixDSym& covHER, const TMatrixDSym& covLER);

    /** Get the CMS energy of collisions */
    double getNominalEcms()       { return m_beamParams->getMass(); }

    /** Get spread of CMS collision energy calculated from beam parameters */
    double getNominalEcmsSpread() { return  m_generateLorentzTransformation.getX0spread(); }

    /** Get the generator for the Lorentz transformation */
    const ConditionalGaussGenerator& getLorentzGenerator() { return m_generateLorentzTransformation; }

  private:

    /**
     * Generate a new event wit a particular set of allowed flags.
     * @param[in] allowedFlags Allowed flags.
     */
    MCInitialParticles& generate(int allowedFlags);

    /** adjust smearing covariance matrix based on the generation flags */
    TMatrixDSym adjustCovMatrix(TMatrixDSym cov) const;


    /** generate the vertex
     * @param initial nominal vertex position
     * @param cov covariance of the vertex position
     * @param gen multivariate normal generator to be used
     */
    ROOT::Math::XYZVector generateVertex(const ROOT::Math::XYZVector& initial, const TMatrixDSym& cov,
                                         MultivariateNormalGenerator& gen) const;
    /** generate 4 vector for one beam
     * @param initial beam
     * @param cov covariance of the beam momentum (E, theta_x, theta_y)
     * @param gen multivariate normal generator to be used
     */
    ROOT::Math::PxPyPzEVector generateBeam(const ROOT::Math::PxPyPzEVector& initial, const TMatrixDSym& cov,
                                           MultivariateNormalGenerator& gen) const;
    /** Datastore object containing the nominal beam parameters */
    DBObjPtr<BeamParameters> m_beamParams;
    /** Datastore object containing the generated event */
    StoreObjPtr<MCInitialParticles> m_event;
    /** Generator for HER */
    MultivariateNormalGenerator m_generateHER;
    /** Generator for LER */
    MultivariateNormalGenerator m_generateLER;
    /** Generator for Vertex */
    MultivariateNormalGenerator m_generateVertex;

    /** Generator of the Lorentz transformation */
    ConditionalGaussGenerator m_generateLorentzTransformation;

    /** Allowed generation flags */
    int m_allowedFlags;
  };


} //Belle2 namespace
