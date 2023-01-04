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
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/database/DBObjPtr.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/dbobjects/PIDDetectorWeights.h>


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


  private:

    /**
     * StoreArray of Particles
     */
    StoreArray<Particle> m_particles;

    /**
     * The name of the input charged stable particle list, or composite particle w/ charged stable daughters for which distances are to be calculated.
     */
    std::string m_decayString;

    /**
     * The number of selected daughters in the decay string.
     */
    unsigned short m_nSelectedDaughters;

    /**
     * The name of the input ParticleList of reference tracks.
     */
    std::string m_pListReferenceName;

    /**
     * The name of the detector at whose inner (cylindrical) surface we extrapolate each track's polar and azimuthal angle.
     */
    std::string m_detName;

    /**
     * The number of layers for the input detector.
     */
    unsigned int m_nLayers;

    /**
     * Map that associates to each detector layer the name of the variable
     * representing the distance to the closest particle in the reference list,
     * based on the track helix extrapolation.
     * Each variable is added as particle extraInfo.
     */
    std::unordered_map<std::string, std::string>  m_detLayerToDistVariable;

    /**
     * Map that associates to each detector layer the name of the variable
     * representing the  mdst array index of the closest particle in the reference list.
     * Each variable is added as particle extraInfo.
     */
    std::unordered_map<std::string, std::string>  m_detLayerToRefPartIdxVariable;

    /**
     * The name of the variable
     * representing the track isolation score in this detector.
     * Added as particle extraInfo.
     */
    std::string m_isoScoreVariable;

    /**
     * Map that associates to each detector its list of valid layers.
     */
    std::unordered_map<std::string, std::vector<int>> m_detToLayers = {
      {"CDC", {0, 1, 2, 3, 4, 5, 6, 7, 8}},
      {"TOP", {0}},
      {"ARICH", {0}},
      {"ECL", {0, 1}},
      {"KLM", {0}}
    };

    /**
     * Map that associates to each detector layer its valid cylindrical surface's boundaries.
     * Values are taken from the B2 TDR.
     */
    std::unordered_map<std::string, DetSurfCylBoundaries> m_detLayerToSurfBoundaries = {
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
      {"ECL1", DetSurfCylBoundaries(140.0, 211.0, -117.0, 0.21, 0.56, 2.24, 2.70)},
      {"KLM0", DetSurfCylBoundaries(202.0, 283.9, -189.9, 0.40, 0.82, 2.13, 2.60)},
    };

    /**
     * Threshold values for the distance (in [cm]) to closest ext. helix to define isolated particles.
     * One for each detector layer.
     */
    std::map<std::pair<std::string, int>, double> m_distThreshPerDetLayer = {
      { {Const::parseDetectors(Const::CDC), 0}, 5.0 },
      { {Const::parseDetectors(Const::CDC), 1}, 5.0 },
      { {Const::parseDetectors(Const::CDC), 2}, 5.0 },
      { {Const::parseDetectors(Const::CDC), 3}, 5.0 },
      { {Const::parseDetectors(Const::CDC), 4}, 5.0 },
      { {Const::parseDetectors(Const::CDC), 5}, 5.0 },
      { {Const::parseDetectors(Const::CDC), 6}, 5.0 },
      { {Const::parseDetectors(Const::CDC), 7}, 5.0 },
      { {Const::parseDetectors(Const::CDC), 8}, 5.0 },
      { {Const::parseDetectors(Const::TOP), 0}, 22.0 },
      { {Const::parseDetectors(Const::ARICH), 0}, 10.0 },
      { {Const::parseDetectors(Const::ECL), 0}, 36.0 },
      { {Const::parseDetectors(Const::ECL), 1}, 36.0 },
      { {Const::parseDetectors(Const::KLM), 0}, 20.0 }
    };

    /**
     * The event information. Used for debugging purposes.
     */
    StoreObjPtr<EventMetaData> m_event_metadata;

    /**
     * The input ParticleList object for which distances are to be calculated.
     */
    StoreObjPtr<ParticleList> m_pListTarget;

    /**
     *< Decay descriptor of decays to look for.
     */
    DecayDescriptor m_decaydescriptor;

    /**
     * The input ParticleList object of reference tracks.
     */
    StoreObjPtr<ParticleList> m_pListReference;

    /**
     * If this option is set, the helix extrapolation for the target and reference particles will use the track fit result
     *  for the most probable mass hypothesis, namely, the one that gives the highest chi2Prob of the fit.
     */
    bool m_useHighestProbMassForExt;

    /**
     * Exclude the PID detector weights for the isolation score definition.
     */
    bool m_excludePIDDetWeights;

    /**
     * The name of the database payload object with the MVA weights.
     */
    std::string m_payloadName;

    /**
     * Interface to get the database payload with the PID detector weights.
     */
    std::unique_ptr<DBObjPtr<PIDDetectorWeights>> m_DBWeights;

    /**
     * Calculate the distance between the points where the two input
     * extrapolated track helices cross the given detector layer's cylindrical surface.
     */
    double getDistAtDetSurface(const Particle* iParticle, const Particle* jParticle, const std::string& detLayerName) const;

    /**
     * Define a semi-continuous variable to quantify the isolation of a standard charged particle
     * in the given detector \f$d\f$ with \f$N_{d}\f$ layers in total.
     * The definition of the score is based on the counting of layers \f$n_{d}\f$ where a close-enough particle is found,
     * and also (if `m_excludePIDDetWeights = true`) on the weight \f$w_{d}\f$ that each sub-detector
     * has on the PID of the given particle hypothesis:

     \f{equation}{
       s_{d} = 1 - \left(-w_{d} \cdot \frac{n_{d}}{N_{d}}\right).
     \f}

     * The distance to closest track helix extrapolation defined in `double getDistAtDetSurface()` is used.
     * Note that if the PID detector weighting is switched off, \f$w_{d} = -1\f$.
     *
     * The per-detector score is normalised in \f$s_{d}\in [0, 1]\f$: values closer to 1 indicate well-isolated particles.
     */
    double getIsoScore(const Particle* iParticle) const;

    /**
     * Get the threshold value per detctor layer for the distance to closest ext. helix
     * that is used to define locally isolated particles at that layer.
     * @param det the input PID detector.
     * @param layer the input detector layer.
     */
    double getDistThreshold(Const::EDetector det, int layer) const
    {
      auto detAndLayer = std::make_pair(Const::parseDetectors(det), layer);
      return m_distThreshPerDetLayer.at(detAndLayer);
    };

    /**
     * Check whether input particle list and reference list are of a valid charged stable particle.
     */
    bool onlySelectedStdChargedInDecay();

    /**
     * Get the enum type for this detector name.
     */
    Const::EDetector getDetEnum(const std::string& detName) const
    {

      if (detName == "CDC") return Const::CDC;
      else if (detName == "TOP") return Const::TOP;
      else if (detName == "ARICH") return Const::ARICH;
      else if (detName == "ECL") return Const::ECL;
      else if (detName == "KLM") return Const::KLM;
      else B2FATAL("Unknown detector component: " << detName);

    };

  };
}
