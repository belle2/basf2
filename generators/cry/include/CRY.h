/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/gearbox/Unit.h>

#include <cry/CRYGenerator.h>
#include <cry/CRYSetup.h>
#include <VecGeom/volumes/UnplacedVolume.h>

namespace Belle2 {
  class MCParticleGraph;
  /**
   * C++ Interface for the generator CRY.
   *
   */

  class CRY final {
  public:

    /** Constructor.
     * Sets the default settings.
     */
    CRY() = default;

    /** Destructor. */
    ~CRY() = default;

    /**
     * Initializes the generator.
     */
    void init();

    /** Generates one single event.
     * @param mcGraph Reference to the MonteCarlo graph into which the generated particles will be stored.
     */
    void generateEvent(MCParticleGraph& mcGraph);

    /** Sets the directory that holds cosmic data.
     * @param cosmicdatadir directory that holds the data.
     */
    void setCosmicDataDir(const std::string& cosmicdatadir) { m_cosmicDataDir = cosmicdatadir; }

    /** Sets the size of the acceptance volume
     * @param size either one, two or three values corresponding to the radius
     *     of a sphere, radius and half length of a cylinder or width, height
     *     and length of a box
     */
    void setAcceptance(const std::vector<double>& size) { m_acceptSize = size; }

    /** Sets the time offset.
     * @param timeoffset time offset for particles starting at the world box.
     */
    void setTimeOffset(double timeoffset) { m_timeOffset = timeoffset; }

    /** Sets the kinetic energy threshold, particles below that value will be ignored.
     * @param kineticenergythreshold kinetic energy threshold.
     */
    void setKineticEnergyThreshold(double kineticenergythreshold) { m_kineticEnergyThreshold = kineticenergythreshold; }

    /** Sets the maximum number of trials.
     * @param maxtrials maximum number of trials.
     */
    void setMaxTrials(int maxtrials) { m_maxTrials = maxtrials; }

    /**
     * Terminates the generator.
     * Closes the internal generator.
     */
    void term();

  protected:
    std::string m_cosmicDataDir;  /**< directory that holds cosmic data files. */
    double m_subboxLength{100}; /**< length of the square n-n plane in Cry in meters */
    std::vector<double> m_acceptSize; /**< Shape parameters for the acceptance shape */
    double m_timeOffset{0};  /**< time offset in nanoseconds. */
    double m_kineticEnergyThreshold{0};  /**< kinetic energy threshold. */
    int m_maxTrials{0};   /**< number of trials per event. */
    int m_totalTrials{0}; /**< total number of thrown events. */

    std::unique_ptr<CRYSetup> m_crySetup;   /**< The CRY generator setup. */
    std::unique_ptr<CRYGenerator> m_cryGenerator;   /**< The CRY generator. */
    std::unique_ptr<vecgeom::VUnplacedVolume> m_world; /**< world box shape */
    std::unique_ptr<vecgeom::VUnplacedVolume> m_acceptance; /**< acceptance shape */
  };
} // end namespace Belle2
