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

#ifndef OPTICALGUNMODULE_H
#define OPTICALGUNMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <top/geometry/TOPGeometryPar.h>
#include <TVector3.h>
#include <TRotation.h>

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
    double m_x;          /**< source position in x */
    double m_y;          /**< source position in y */
    double m_z;          /**< source position in z */
    double m_diameter;   /**< source diameter */
    double m_alpha;      /**< source emission angle [deg] */
    double m_na;         /**< source numerical aperture */
    double m_wavelength; /**< source wavelenght [nm] */
    double m_phi;        /**< first rotation angle (around z) [deg] */
    double m_theta;      /**< second rotation angle (around x) [deg] */
    double m_psi;        /**< third rotation angle (around z) [deg] */
    double m_startTime;  /**< start time */
    double m_pulseWidth; /**< pulse duration (Gaussian sigma) */
    double m_numPhotons; /**< average number of photons in a pulse */
    int m_barID;         /**< TOP bar ID */
    double m_slitDX;     /**< slit size in x */
    double m_slitDY;     /**< slit size in y */
    double m_slitX0;     /**< slit x-offset in respect to source */
    double m_slitY0;     /**< slit y-offset in respect to source */
    double m_slitZ;      /**< slit distance from source */
    std::string m_angularDistribution; /**< emission angular distribution */

    // others
    double m_cosAlpha;   /**< cos(alpha) */
    double m_energy;     /**< photon energy (from wavelength) */
    TOP::TOPGeometryPar* m_topgp;   /**< geometry parameters of TOP */

    TVector3 m_translate;  /**< translation to Belle II frame */
    TRotation m_rotate;    /**< rotation to Belle II frame */
    TRotation m_rotateBar; /**< rotation of a bar */

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

#endif
