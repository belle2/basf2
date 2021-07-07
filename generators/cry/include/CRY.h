/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

    /** Sets the date used for generation (the cosmic-ray distribution is adjusted to account for the eleven year, sunspot cycle).
     * @param date date used for generation (month-day-year).
     */
    void setDate(const std::string& date) { m_date = date; }

    /** Sets the maximum number of trials.
     * @param maxtrials maximum number of trials.
     */
    void setMaxTrials(int maxtrials) { m_maxTrials = maxtrials; }

    /** Set the size of the square nxn plane where CRY generates cosmics.
     *
     * According to the CRY documentation good values are 1, 3, 10, 30, 100 and
     * 300 meters but anything smaller than 300 m should work.
     *
     * The value should be given in cm but will be rounded down to full meters
     *
     * @param length length of the side in standard units (cm)
     */
    void setBoxLength(double length) { m_subboxLength = length / Unit::m; }

    /** Set whether or not CRY should return gammas
     *
     * @param gammas true if gammas should be returned, else false
     */
    void setReturnGammas(bool gammas) { m_returnGammas = gammas; }

    /** Set whether or not CRY should return kaons
     *
     * @param kaons true if kaons should be returned, else false
     */
    void setReturnKaons(bool kaons) { m_returnKaons = kaons; }

    /** Set whether or not CRY should return pions
     *
     * @param pions true if pions should be returned, else false
     */
    void setReturnPions(bool pions) { m_returnPions = pions; }

    /** Set whether or not CRY should return protons
     *
     * @param protons true if protons should be returned, else false
     */
    void setReturnProtons(bool protons) { m_returnProtons = protons; }

    /** Set whether or not CRY should return neutrons
     *
     * @param neutrons true if neutrons should be returned, else false
     */
    void setReturnNeutrons(bool neutrons) { m_returnNeutrons = neutrons; }

    /** Set whether or not CRY should return electrons
     *
     * @param electrons true if electrons should be returned, else false
     */
    void setReturnElectrons(bool electrons) { m_returnElectrons = electrons; }

    /** Set whether or not CRY should return muons
     *
     * @param muons true if muons should be returned, else false
     */
    void setReturnMuons(bool muons) { m_returnMuons = muons; }

    /**
     * Terminates the generator.
     * Closes the internal generator.
     */
    void term();

  protected:
    std::string m_cosmicDataDir;  /**< directory that holds cosmic data files. */
    int m_subboxLength{100}; /**< length of the square n-n plane in Cry in meters */
    std::vector<double> m_acceptSize; /**< Shape parameters for the acceptance shape */
    double m_timeOffset{0};  /**< time offset in nanoseconds. */
    double m_kineticEnergyThreshold{0};  /**< kinetic energy threshold. */
    std::string m_date{"1-1-2019"}; /**< date used for generation (month-day-year). */
    int m_maxTrials{0};   /**< number of trials per event. */
    int m_totalTrials{0}; /**< total number of thrown events. */
    bool m_returnGammas{true}; /**< Whether or not CRY should return gammas */
    bool m_returnKaons{true}; /**< Whether or not CRY should return kaons */
    bool m_returnPions{true}; /**< Whether or not CRY should return pions */
    bool m_returnProtons{true}; /**< Whether or not CRY should return protons */
    bool m_returnNeutrons{true}; /**< Whether or not CRY should return neutrons */
    bool m_returnElectrons{true}; /**< Whether or not CRY should return electrons */
    bool m_returnMuons{true}; /**< Whether or not CRY should return muons */

    std::unique_ptr<CRYSetup> m_crySetup;   /**< The CRY generator setup. */
    std::unique_ptr<CRYGenerator> m_cryGenerator;   /**< The CRY generator. */
    std::unique_ptr<vecgeom::VUnplacedVolume> m_world; /**< world box shape */
    std::unique_ptr<vecgeom::VUnplacedVolume> m_acceptance; /**< acceptance shape */
  };
} // end namespace Belle2
