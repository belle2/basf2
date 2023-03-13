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
     * The list of names of the detectors at whose inner (cylindrical) surface we extrapolate each track's polar and azimuthal angle.
     */
    std::vector<std::string> m_detNames;

    /**
     * Map that associates to each detector layer (e.g., 'CDC6') the name of the variable
     * representing the distance to the closest particle in the reference list,
     * based on the track helix extrapolation.
     * Each variable is added as particle extraInfo.
     */
    std::unordered_map<std::string, std::string>  m_detLayerToDistVariable;

    /**
     * Map that associates to each detector layer (e.g, 'CDC6') the name of the variable
     * representing the mdst array index of the closest particle in the reference list.
     * Each variable is added as particle extraInfo.
     */
    std::unordered_map<std::string, std::string>  m_detLayerToRefPartIdxVariable;

    /**
     * The name of the variable representing the track isolation score.
     * Added as particle extraInfo.
     */
    std::string m_isoScoreVariable;

    /**
     * The name of the variable representing the track isolation score.
     * Added as particle extraInfo.
     */
    std::string m_isoScoreVariableAsWeightedAvg;

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
     * Get the PID weight, \f$w_{d} \in [-1, 0]\f$, for this particle and detector reading it from the payload, if selected.
     * Otherwise return a default weight of -1.
     */
    double getDetectorWeight(const Particle* iParticle, const std::string& detName) const;

    /**
     * Get the sum of layers with a close-by track, divided by the total number of layers,
     * for the given detector \f$d\f$, weighted by the PID detector separation score (if requested):

     \f{equation}{
       s_{d} = 1 - \left(-w_{d} \cdot \frac{n_{d}}{N_{d}}\right).
     \f}

     * where \f$n_{d}\f$ is the number of layers where a close-enough particle is found,
     * and \f$w_{d}\f$ is the weight that each sub-detector
     * has on the PID of the given particle hypothesis (if `m_excludePIDDetWeights = true`):
     *
     * The distance to closest track helix extrapolation defined in `double getDistAtDetSurface()` is used.
     * Note that if the PID detector weighting is switched off, \f$w_{d} = -1\f$.
     */
    double getWeightedSumNonIsoLayers(const Particle* iParticle, const std::string& detName, const float detWeight) const;

    /**
     * Get the sum of the inverse (scaled) minimum distances
     * over the given detector \f$d\f$ layers, weighted by the PID detector separation score (if requested):

     \f{equation}{
       S_{d} = \sum_{d} w_{d} * \frac{D_{d}^{thresh}}{D_{d}}
     \f}

     * The distance \f$D_{d}\f$ to the closest track helix extrapolation defined in `double getDistAtDetSurface()` is used.
     * The scaling at the numerator is the threshold distance for this detector to define close-by tracks.
     * Note that if the PID detector weighting is switched off, \f$w_{d} = -1\f$.
     * By construction, \f$S_{d}\f$ is a negative number.
     */
    double getWeightedSumInvDists(const Particle* iParticle, const std::string& detName, const float detWeight) const;

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
