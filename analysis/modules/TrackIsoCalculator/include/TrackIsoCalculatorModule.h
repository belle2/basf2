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
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/LogConfig.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>


namespace Belle2 {

  /**
   * Simple class to encapsulate a detector surface's boundaries
   * in cylindrical coordinates.
   */
  class DetSurfCylBoundaries {

  public:
    /**
     * Default constructor.
     */
    DetSurfCylBoundaries() {};
    /**
     * Constructor with parameters.
     */
    DetSurfCylBoundaries(float rho, float zfwd, float zbwd, float th_fwd, float th_fwd_brl, float th_bwd_brl, float th_bwd)
    {
      m_rho = rho;
      m_zfwd = zfwd;
      m_zbwd = zbwd;
      m_th_fwd = th_fwd;
      m_th_fwd_brl = th_fwd_brl;
      m_th_bwd_brl = th_bwd_brl;
      m_th_bwd = th_bwd;
    };
    float m_rho; /**< Inner surface radius [cm] */
    float m_zfwd; /**< Inner surface z fwd [cm] */
    float m_zbwd; /**< Inner surface z bwd [cm] */
    float m_th_fwd; /**< Lower theta edge of fwd region [rad] */
    float m_th_fwd_brl; /**< fwd/barrel separation theta [rad] */
    float m_th_bwd_brl; /**< bwd/barrel separation theta [rad] */
    float m_th_bwd; /**< Upper theta edge of bwd region [rad] */
  };

  /**
   * Calculate track isolation variables on the input ParticleList.
   */
  class TrackIsoCalculatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TrackIsoCalculatorModule();

    /**
     * Destructor, use this to clean up anything you created in the constructor.
     */
    ~TrackIsoCalculatorModule() override;

    /**
     * Use this to initialize resources or memory your module needs.
     *
     * Also register any outputs of your module (StoreArrays, StoreObjPtrs, relations) here,
     * see the respective class documentation for details.
     */
    void initialize() override;

    /**
     * Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    void event() override;

    /**
     * Module terminate().
     */
    void terminate() override;

  private:

    /**
     * The name of the input ParticleList.
     */
    std::string m_pListName;

    /**
     * The name of the input ParticleList of reference tracks.
     */
    std::string m_pListReferenceName;

    /**
     * The name of the detector at whose inner (cylindrical) surface we extrapolate each track's polar and azimuthal angle.
     */
    std::string m_detSurface;

    /**
     * Associate the detector flag to a boolean flag to quickly tell which detector it belongs too.
     */
    std::unordered_map<std::string, bool> m_isSurfaceInDet;

    /**
     * The name of the distance variable to be added to each particle as extraInfo.
     */
    std::string m_extraInfoName;

    /**
     * Map that associates to each detector its valid cylindrical surface layer's boundaries.
     * Values are taken from the B2 TDR.
     */
    std::unordered_map<std::string, DetSurfCylBoundaries> m_detSurfBoundaries = {
      {"CDC0", DetSurfCylBoundaries(16.8, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
      {"CDC1", DetSurfCylBoundaries(25.7, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
      {"CDC2", DetSurfCylBoundaries(36.52, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
      {"CDC3", DetSurfCylBoundaries(47.69, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
      {"CDC4", DetSurfCylBoundaries(58.41, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
      {"CDC5", DetSurfCylBoundaries(69.53, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
      {"CDC6", DetSurfCylBoundaries(80.25, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
      {"CDC7", DetSurfCylBoundaries(91.37, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
      {"CDC8", DetSurfCylBoundaries(102.09, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
      {"TOP0", DetSurfCylBoundaries(117.8, 193.0, -94.0, 0.24, 0.52, 2.23, 3.14)},
      {"ARICH0", DetSurfCylBoundaries(117.8, 193.0, -94.0, 0.24, 0.52, 2.23, 3.14)},
      {"ECL0", DetSurfCylBoundaries(125.0, 196.0, -102.0, 0.21, 0.56, 2.24, 2.70)},
      {"KLM0", DetSurfCylBoundaries(202.0, 283.9, -189.9, 0.40, 0.82, 2.13, 2.60)},
    };

    /**
     * The event information. Used for debugging purposes.
     */
    StoreObjPtr<EventMetaData> m_event_metadata;

    /**
     * The input ParticleList object.
     */
    StoreObjPtr<ParticleList> m_pList;

    /**
     * The input ParticleList object of reference tracks.
     */
    StoreObjPtr<ParticleList> m_pListReference;

    /**
     * Calculate the distance between the points where the two input
     * extrapolated track helices cross the given detector's cylindrical surface.
     */
    double getDistAtDetSurface(Particle* iParticle, Particle* jParticle);

    /**
     * Check whether input particle list and reference list are of a valid charged stable particle.
     */
    bool isStdChargedList();

    /**
     * Print 2D array of pair-wise distances.
     */
    void printDistancesArr(const std::vector<std::vector<double>>& arr, int size_x, int size_y);

  };
}
