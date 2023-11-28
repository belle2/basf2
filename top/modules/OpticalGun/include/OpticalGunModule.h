/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <Math/Transform3D.h>
#include <Math/Vector3D.h>
#include <Math/Point3D.h>
#include <TF1.h>

// DataStore
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPSimCalPulse.h>

namespace Belle2 {

  /**
   * Source of optical photons for the simulation of the TOP laser system.
   * The angular distribution can be picked up in a set of pre-defined functions:
   * - 'uniform'  (between minAlpha and maxAlpha),
   * - 'Lambertian'  (between minAlpha and maxAlpha),
   * - 'Gaussian'  (uses numerical aperture instead of minAlpha and maxAlpha),
   * or passed as a string (TFormula-compliat) function of the polar direction theta.
   * When using the latter option, be careful about normalizing over the spherical unit area proprely
   * (i.e. multiply by sin(theta)).
   * In all cases we assume the emission is symmetric around the axis of the source.
   *
   * Polarization of photons is passed to FullSim using the decayVertex member of MC particle class.
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
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    // steering parameters
    double m_x = 0;          /**< source position in x */
    double m_y = 0;          /**< source position in y */
    double m_z = 0;          /**< source position in z */
    double m_diameter = 0;   /**< source diameter */
    double m_maxAlpha = 0;   /**< maximum emission angle */
    double m_minAlpha = 0;   /**< minimum emission angle */
    double m_na = 0;         /**< source numerical aperture. Used only by the Gaussian emission model. */
    double m_wavelength = 0; /**< source wavelenght [nm] */
    double m_phi = 0;        /**< first rotation angle (around z) [deg] */
    double m_theta = 0;      /**< second rotation angle (around x) [deg] */
    double m_psi = 0;        /**< third rotation angle (around z) [deg] */
    double m_startTime = 0;  /**< start time */
    double m_pulseWidth = 0; /**< pulse duration (Gaussian sigma) */
    double m_numPhotons = 0; /**< average number of photons in a pulse */
    int m_slotID = 0;        /**< TOP slot ID */
    double m_slitDX = 0;     /**< slit size in x */
    double m_slitDY = 0;     /**< slit size in y */
    double m_slitX0 = 0;     /**< slit x-offset in respect to source */
    double m_slitY0 = 0;     /**< slit y-offset in respect to source */
    double m_slitZ = 0;      /**< slit distance from source */
    std::string m_angularDistribution; /**< source angular distribution */

    // others
    double m_cosMinAlpha = 0; /**< cos of m_minAlpha */
    double m_cosMaxAlpha = 0; /**< cos of m_maxAlpha */
    double m_energy = 0;     /**< photon energy (from wavelength) */
    ROOT::Math::Transform3D m_transform; /**< transformation to BelleII frame */
    TF1* m_customDistribution = 0; /**< Custom angular distribution, that uses m_angularDistribution as formula. */

    // data store objects
    StoreArray<MCParticle> m_MCParticles; /**< MC particles collection */
    StoreArray<TOPSimCalPulse> m_simCalPulses; /**< simulated cal pulse collection */

    /**
     * Checks if photon passes the slit
     * @param point photon emission point
     * @param direction photon emission direction
     * @return true on success
     */
    bool isInsideSlit(const ROOT::Math::XYZPoint& point, const ROOT::Math::XYZVector& direction) const;

    /**
     * Return photon direction according to a projected 2D gaussian distribution based on numerical aperture NA.
     * @return photon direction (unit vector)
     */
    ROOT::Math::XYZVector getDirectionGaussian() const;

    /**
     * Return photon direction according to a projected uniform distribution with opening angle alpha. Be careful.
     * @return photon direction (unit vector)
     */
    ROOT::Math::XYZVector getDirectionUniform() const;

    /**
     * Return photon direction according to a lambertian distribution with opening angle alpha.
     * @return photon direction (unit vector)
     */
    ROOT::Math::XYZVector getDirectionLambertian() const;

    /**
     * Return photon direction according to a uniform distribution within m_minAlpha and m_maxAlpha.
     * @return photon direction (unit vector)
     */
    ROOT::Math::XYZVector getDirectionUniformRingAngle() const;

    /**
     * Return photon direction according to a custom angular distribution given by TFormula.
     * @return photon direction (unit vector)
     */
    ROOT::Math::XYZVector getDirectionCustom() const;

  };

} // Belle2 namespace

