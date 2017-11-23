/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *               Stefano Lacaprara                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <TVector3.h>
#include <TRotation.h>

// DataStore
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>

namespace Belle2 {

  /**
   * Source of optical photons
   */
  class OpticalGunModule : public Module {

  public:

    /**
     * Constructor
     */
    OpticalGunModule();

    /**
     * Destructor
     */
    virtual ~OpticalGunModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    // steering parameters
    double m_x = 0;          /**< source position in x */
    double m_y = 0;          /**< source position in y */
    double m_z = 0;          /**< source position in z */
    double m_diameter = 0;   /**< source diameter */
    double m_alpha = 0;      /**< source emission angle [deg] */
    double m_na = 0;         /**< source numerical aperture */
    double m_wavelength = 0; /**< source wavelenght [nm] */
    double m_phi = 0;        /**< first rotation angle (around z) [deg] */
    double m_theta = 0;      /**< second rotation angle (around x) [deg] */
    double m_psi = 0;        /**< third rotation angle (around z) [deg] */
    double m_startTime = 0;  /**< start time */
    double m_pulseWidth = 0; /**< pulse duration (Gaussian sigma) */
    double m_numPhotons = 0; /**< average number of photons in a pulse */
    int m_barID = 0;         /**< TOP bar ID */
    double m_slitDX = 0;     /**< slit size in x */
    double m_slitDY = 0;     /**< slit size in y */
    double m_slitX0 = 0;     /**< slit x-offset in respect to source */
    double m_slitY0 = 0;     /**< slit y-offset in respect to source */
    double m_slitZ = 0;      /**< slit distance from source */
    std::string m_angularDistribution; /**< emission angular distribution */

    // others
    double m_cosAlpha = 0;   /**< cos(alpha) */
    double m_energy = 0;     /**< photon energy (from wavelength) */

    TVector3 m_translate;  /**< translation to Belle II frame */
    TRotation m_rotate;    /**< rotation to Belle II frame */
    TRotation m_rotateBar; /**< rotation of a bar */

    // data store objects
    StoreArray<MCParticle> m_MCParticles; /**< MC particles collection */

    /**
     * Checks if photon flies through the slit
     * @param point photon emission point
     * @param direction photon emission direction
     * @return true, if photon flies through the slit
     */
    bool isInsideSlit(const TVector3& point, const TVector3& direction) const;

    /**
     * Return photon direction according to a 2D gaussian distribution based on
     * numerical aperture NA
     */
    TVector3 getDirectionGaussian() const;

    /**
     * Return photon direction according to a uniform distribution with opening
     * angle alpha
     */
    TVector3 getDirectionUniform() const;

    /**
     * Return photon direction according to a lambertian distribution with
     * opening angle alpha
     */
    TVector3 getDirectionLambertian() const;
  };

} // Belle2 namespace

