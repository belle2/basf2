/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef GENERATORS_UTILITIES_INITIALPARTICLEGENERATION_H
#define GENERATORS_UTILITIES_INITIALPARTICLEGENERATION_H

#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/utilities/MultivariateNormalGenerator.h>
#include <framework/dbobjects/BeamParameters.h>
#include <framework/dataobjects/MCInitialParticles.h>

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

    /** Return reference to nominal beam parameters */
    const BeamParameters& getBeamParameters() const { return *m_beamParams; }

    /** function to be executed on initialize() */
    void initialize();

    /** Set allowed flags. */
    void setAllowedFlags(int allowedFlags)
    {
      m_allowedFlags = allowedFlags | MCInitialParticles::c_generateCMS;
    }

  private:
    /** generate the vertex
     * @param initial nominal vertex position
     * @param cov covariance of the vertex position
     * @param gen multivariate normal generator to be used
     */
    TVector3 generateVertex(const TVector3& initial, const TMatrixDSym& cov, MultivariateNormalGenerator& gen);
    /** generate 4 vector for one beam
     * @param initial beam
     * @param cov covariance of the beam momentum (E, theta_x, theta_y)
     * @param gen multivariate normal generator to be used
     */
    TLorentzVector generateBeam(const TLorentzVector& initial, const TMatrixDSym& cov, MultivariateNormalGenerator& gen);
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
    /** Allowed generation flags */
    int m_allowedFlags;
  };


} //Belle2 namespace
#endif // GENERATORS_UTILITIES_INITIALPARTICLEGENERATION_H
