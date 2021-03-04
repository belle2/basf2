/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2020 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Marco Milesi                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
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
   * Simple class to encapsulate a detector inner surface's boundaries
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

    /**
     * Calculate the 3D distance between the points where the two input
     * extraplolated track helices cross the input detector's inner surface.
     */
    double get3DDistAtDetSurface(Particle* iParticle, Particle* jParticle);

    /**
     * Calculate the 2D distance between the points where the two input
     * extraplolated track helices cross the input detector's inner surface
     * on the (rho, phi) plane. Namely, this is the cord length of the arc
     * that subtends deltaPhi.
     */
    double get2DRhoPhiDistAsChordLength(Particle* iParticle, Particle* jParticle);

  private:

    /**
     * The name of the input ParticleList.
     */
    std::string m_pListName;

    /**
     * The name of the detector at whose innermost layer we extrapolate each track's polar and azimuthal angle.
     */
    std::string m_detInnerSurface;

    /**
     * If true, will calculate the pair-wise track distance as the cord length on the (rho, phi) projection.
     */
    bool m_use2DRhoPhiDist = false;

    /**
     * The name of the distance variable to be added to each particle as extraInfo.
     */
    std::string m_extraInfoName;

    /**
     * Map that associates to each detector its innermost layer's boundaries.
     */
    std::unordered_map<std::string, DetSurfCylBoundaries> m_detSurfBoundaries = {
      {"CDC", DetSurfCylBoundaries(16.0, 150.0, -75.0, 0.0, 0.29, 2.61, 3.14)},
      {"PID", DetSurfCylBoundaries(116.4, 167.0, -335.1, 0.24, 0.52, 2.23, 3.14)}, // PID : TOP, ARICH
      {"ECL", DetSurfCylBoundaries(125.0, 196.0, -102.0, 0.21, 0.56, 2.24, 2.70)},
      {"KLM", DetSurfCylBoundaries(202.0, 283.9, -189.9, 0.40, 0.82, 2.13, 2.60)},
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
     * Check whether input particle list is of a valid charged stable particle.
     */
    inline bool isStdChargedList()
    {
      return (Const::chargedStableSet.find(m_pList->getPDGCode()) != Const::invalidParticle);
    };

    /**
     * Print 2D array of pair-wise distances.
     */
    void printDistancesArr(const std::vector<std::vector<double>>& arr, int size);

  };
}
