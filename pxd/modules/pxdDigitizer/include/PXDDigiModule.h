/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Zbynek Drasal, Benjamin Schwenker, Andreas Moll,         *
 *               Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// /////////////////////////////////////////////////////////////////////// //
//                                                                         //
// PXDDigiModule - basf2 Digitizer/Clusterizer for the DEPFET pixel sensors //
//                                                                         //
// /////////////////////////////////////////////////////////////////////// //

#ifndef PXDDIGITIZER_H_
#define PXDDIGITIZER_H_ 1

// Define ROOT output if needed
// PQ??: Can we turn it into a parameter?
// A calibration run will be done each time with the Digitizer. "Each time" means each time
// the simulation changes. Otherwise, the available resolution calibration will be re-used, to the
// responsibility of  the user.
#define ROOT_OUTPUT

#include <framework/core/Module.h>

// Include basic C++
#include <string>
#include <vector>
#include <map>

// Include Digi header files
#include <pxd/geopxd/SiGeoCache.h>
#include <pxd/modules/pxdDigitizer/SiEnergyFluct.h>

#include <pxd/dataobjects/HitSorter.h>
#include <pxd/dataobjects/RelationHolder.h>

// Include ROOT classes
#ifdef ROOT_OUTPUT
#include <TH1F.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TFile.h>
#include <TTree.h>
#endif // ROOT_OUTPUT

class TRandom3;

namespace Belle2 {

// Typedefs

  typedef std::map<StoreIndex, float> HitRelationMap;   // weight by hit
  typedef std::map<StoreIndex, float>::iterator HitRelationMapItr;

  /**
   * Digit structure - will go to PXDDigits and relations.
   */
  struct Digit {
    int cellIDZ;
    int cellIDRPhi;
    double cellPosZ;
    double cellPosRPhi;
    double charge;

    HitRelationMap pxdSimHitMap;
  };

  /**
   * Ionization point is an amount of charge generated along a short track segment,
   * located at the centre of the track segment.
   * It is an elementary quantity of re-distributable charge that undergoes drift
   * and diffusion.
   */
  struct IonizationPoint {
    TVector3 position;
    double eLoss;
  };

  /**
   * A signal point is a charge cloud at the face of the sensor.
   * It is formed by drift/diffusion of an ionization point.
   */
  struct SignalPoint {
    TVector3 position;
    TVector3 sigma;
    double charge;
  };

  /**
   * A space point of a track
   */
  struct TrackPoint {
    TVector3 position;
    TVector3 direction;
  };

  typedef std::vector<IonizationPoint> IonizationPointVec;
  typedef std::vector<SignalPoint> SignalPointVec;

  /** Digits by pixel ID. */
  typedef std::map<int, Digit> DigitMap;
  typedef std::map<int, Digit>::iterator DigitMapItr;

  /**
   * PXDDigiModule - basf2 PXD Digitizer/Clusterizer Module.
   *
   * The PXDDigiModule code is a basf2 module that carries out a detailed DEPFET PXD
   * digitization and hit reconstruction. <br>
   * For each simulated PXDSimHit, obtained from Geant4 and created by a particle (delta
   * electrons are handled separately), intersection points with the inner and outer boundary
   * of the respective silicon layer are calculated; within these boundaries, the track segment
   * is approximated with a straight line. The track segment is then divided into short subsegments
   * ("ionization points"), whose charge is smeared according to the Landau distribution (the Landau
   * smearing - SiEnergyFluct - follows a Geant4 class called G4UniversalFluctuation, so one obtains
   * exactly the same results as with Geant4 directly, but the limits on the step-size are implemented
   * here based on a different threshold for delta-electron and space precision) as in the full detector
   * simulation. The ionization points are then drifted to the readout plane, Lorentz-shifted in the
   * presence of a magnetic field (Lorentz shift), and smeared by a Gaussian distribution to account
   * for diffusion during the drift time. Next, the total charge of an ionization point is split
   * into carrier groups of ~100 electrons. For each carrier group, we sample a random walk in the
   * readout plane until the internal gate area of a pixel cell is hit. Thus, for each electrode one
   * gets a digit (with either a zero or non-zero signal, depending on how many ionization points
   * contributed).
   * Finally, the digits are converted into real hits (clustering procedure + hit resolution calculation).
   *  Within the clustering procedure, one can define if bricked structure in R-Phi is used (even rows are
   * shifted with respect to odd rows by half a pitch). As a clustering algorithm either a center of
   * gravity (for clusters consisting of 1 or 2 pixels, each direction is taken separately) or
   * analog head-tail algorithm (for clusters larger than 2 pixels) is used.
   * In order to simulate the procedure correctly, first the sensor is populated with all the hits,
   * that is, by random noise hits, signal hits, and background hits, and then clustering is performed.
   * As the Analog-To-Digital converter is going to be used in the data processing pipeline, the analog
   * signals are converted first into digital values and then based on seed cuts and neighbor pixel
   * cuts (user-adjustable), the clustering is performed. The output of the processor is a collection
   * of PXDHits, where each reconstructed hit has a defined position, total collected charge (either
   * analog signal (no ADC) in [electrons] or digital (with ADC) in [ADU] (1 ADU = ADCRange/2^nADCBits),
   * covariance matrix (position resolution, which has to be calculated separately from results obtained
   * with ROOT_OUTPUT on), sensor identifier (from which ladder/layer/sensor the hit comes from), and a
   * vector of PXDSimHits that contributed. The position is corrected for the mean Lorentz shift. By default,
   * the input collection is "PXDSimHitCollection", and that of the output is "PXDHitCollection". As the
   * hit resolution is given as an external parameter and is not calculated automatically, one has to
   * perform the digitization with #define ROOT_OUTPUT set first, analyse the histograms, calculate
   * resolutions and supply them back as input parameters. This procedure is hard to automate.
   */

  class PXDDigiModule : public Module {

  public:

    /** Constructor.*/
    PXDDigiModule();

    /** Destructor.*/
    virtual ~PXDDigiModule();

    /**
     * Initialize the module.
     *
     * This method is called at the beginning of data processing.
     * Initialize module parameters, collect geometry info, set up the digitizer.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     *
     * Set run dependent things like run header parameters, resolutions, mag. field etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     *
     * Convert PXDSimHits of the event to PXDDigits and/or PXDHits.
     */
    virtual void event();

    /**
     * End-of-run action.
     *
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Terminate the module.
     *
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  protected:

    // Main processing methods, operating on a single module.

    /** Calculate ionization points along the path from a given PXDSimHit. */
    void produceIonizationPoints(StoreIndex iHit, IonizationPointVec & ionizationPoints);

    /** Calculate signal points from ionization points. */
    void produceSignalPoints(const IonizationPointVec & ionizationPoints, SignalPointVec & signalPoints);

    /** Calculate digits from signal points - diffusion, Poisson and electronics effects added. */
    void produceDigits(const SignalPointVec & signalPoints, StoreIndex simHitIdx, DigitMap & digits);

    /** Simulate the effect of ganged pixels (upper half of the sensor mapped to the lower half and vice versa). */
    // Not implemented for basf2 yet.
    //void simulateGangedPixels(DigitMap & digitMap);

    /** Add noise to collected digits. */
    void addPixelNoise(DigitMap & digits);

    /** Produce noise digits and add them among the recorded digits. */
    void addNoiseDigits(DigitMap & digits);

    /** Save digits collected for a sensor to the DataStore as PXDDigits.*/
    void saveDigits(DigitMap & digits);

    // UTILITY

    /** Return collected charge in ADC units. */
    short int getInADCUnits(double charge) const
    { return static_cast<short>(charge / m_ADCUnit); }

    // PRINT METHODS

    /** Print module parameters. */
    void printModuleParams() const;

  private:
    // DATA MEMBERS

    // Collections
    std::string m_mcColName; /**< MC particles collection name. */
    std::string m_simColName; /**< PXDSimHits collection name. */
    std::string m_relSimName; /**< MCParticle-to-input hits relation name. */
    std::string m_digitColName; /**< Digits collection name. */
    std::string m_relDigitName; /**< MCParticles-to-digits relation name. */

    int m_nDigitsSaved;         /**< Number of saved digits per event. */
    int m_nRelationsSaved;      /**< Number of saved relations per event. */
    // Relation container
    TwoSidedRelationSet m_relMCSim; /**< A two-index structure to handle relations.*/

    // Digitization parameters - readout
    bool  m_ADC;          /**< Simulate ADC or not. */
    int m_ADCRange;       /**< ADC range is from 0 - ? (in electrons). */
    int m_ADCBits;        /**< ADC has 0 - (2^? -1) digital values. */
    double m_ADCUnit;          /**< ADC unit in electrons. */
    bool  m_electronicEffects;  /**< Add noise or not. */
    double  m_elNoise;        /**< Electronic noise of individual pixels in e. */
    bool  m_PoissonSmearing;    /**< Set Poisson smearing or not (applies only if electronics effects set). */
    int m_gangedPixels;     /**< Ganged pixels mode (0 - off, 1 - on, 2 - on, R/O with special algorithm). */
    bool  m_integrationWindow;  /**< Use integration window?. */
    double m_SNAdjacent; /**< Threshold signal/noise level for zero suppression.*/

    // PQ!!: Most of these have to go to a configuration file.
    double  m_startIntegrationFirstLayer; /**< Global start time of sensors' integration in ns (anything before this value will not be digitized). */
    double  m_stopIntegrationFirstLayer;  /**< Global stop time of sensors' integration in ns (anything after this value will not be digitized). */
    double  m_startIntegrationOtherLayer; /**< Start time of sensors' integration in ns (anything before this time will not be digitized). */
    double  m_stopIntegrationOtherLayer;  /**< Stop time of sensors' integration in ns (anything after this time will not be digitized). */

    // Digitization parameters - ionization
    bool  m_landauFluct;          /**< Set to use internal Landau fluctuations instead of Geant4. */
    double  m_landauBetaGammaCut;     /**< Below this beta*gamma factor internal Landau fluctuations are not used. */
    double  m_prodThreshOnDeltaRays;    /**< Production threshold cut on delta electrons in keV (for Landau fluct.) - use the same as in Geant4 (80keV ~ 0.05 mm). */
    double  m_segmentLength;        /**< Length of path subsegment - spatial precision of charge distribution simulation. */

    // Digitization parameters - charge transport
    double  m_diffusionCoefficient;     /**< Diffusion coefficient of electrons in silicon. */
    double m_widthOfDiffusCloud;           /**< Range of charge cloud integration in sigma.*/
    double  m_tanLorentzAngle;        /**< Tangent of Lorentz angle. */
    bool  m_doublePixel;          /**< Define if double pixel structure is used. */
    double  m_bulkDoping;         /**< Net bulk doping concentration in sensors, in um^-3. */
    double  m_Uback;            /**< Back plane voltage wrt. source, in Volts. */
    double  m_sourceBorderLength;     /**< Source border region with small drift fields, in um. */
    double  m_drainBorderLength;      /**< Drain border region with small drift fields, in um. */
    double  m_clearBorderLength;      /**< Clear border region with small drift fields, in um. */
    double  m_eGroupSize;         /**< Split Signalpoints in smaller groups of N electrons (in e). */
    double  m_eStepTime;          /**< Time step for tracking electron groups in readout plane (in ns). */

    // Current sensor parameters from geometry
    short int m_currentLayerID;       /**< Actual layer ID. */
    short int m_currentLadderID;      /**< Actual ladder ID. */
    short int m_currentSensorID;      /**< Actual sensor ID. */
    short int m_currentSensorUID;         /**< Actual compact ID (layer/ladder/sensor). */
    float m_sensorThick;          /**< Actual sensor - Si wafer thickness in system length units. */
    float m_sensorWidth;          /**< Actual sensor width in system length units. */
    float m_sensorLength;         /**< Actual sensor length in system length units. */

    TVector3 m_magField;          /**< Magnetic field in T in detector LRF!! Not used. */

    SiGeoCache * m_geometry;        /**< Geometry data on active sensors. */

    TRandom3 * m_random;          /**< Random generator, original name m_gslRandGen. */

    SiEnergyFluct * m_fluctuate;      /**< Landau fluctuation simulator. */

    // Root output
#ifdef ROOT_OUTPUT

    TFile * m_rootFile;           /**< ROOT output file for resolution data. */
    TTree * m_rootTree;           /**< ROOT output tree for resolution data. */

    TH1D * m_rootChargeCollectionTime;    /**< charge collection time in readout plane [ns]. */
    TH1D * m_rootRandomWalkSteps;     /**< number of random walk steps in readout plane. */
    TH1D * m_rootKilledRandomWalks;     /**< number of killed random walks (too many steps). */

    TH1F * m_rootMultiplicity;        /**< Histogram of hit multiplicity. */
    TH1F * m_rootEfficiency;        /**< Histogram of hit efficiency. */
    TH1F  * m_rootELossDigi;              /**< Histogram of energy losses fluctuations generated by SiPxlDigi. */
    TH1F  * m_rootELossG4;                /**< Histogram of energy losses fluctuations generated by Geant4. */
    TH1F * m_rootOccupancy_0;       /**< Histogram of PXD layer 1 occupancy. */
    TH1F * m_rootOccupancy_1;       /**< Histogram of PXD layer 2 occupancy. */

    TH2F  * m_rootDigitsMap;        /**< Histogram used as a map of found digits. */

    std::map<int, int> m_rootMCGenHitMap; /**< Map of generated primary hits. */

    int m_rootLayerID;            /**< layer ID. */
    int m_rootLadderID;           /**< ladder ID. */
    int m_rootSensorID;           /**< sensor ID. */

    int m_rootMCPDG;            /**< PDG of the particle that created the current hit (MC Particle with highest weight). */
    int m_rootIsFromMCPrim;         /**< The reconstructed hit is from a primary MC particle (and not ganged). */
    int m_rootIsGanged;           /**< The hit created by a primary particle is ganged. */
    int m_rootEvtNum;           /**< Event number to which the hit belongs. */

    double m_rootSimRPhi;         /**< Simulated hit position in R-Phi (PXDSimHit with highest weight). */
    double m_rootSimZ;            /**< Simulated hit position in Z (PXDSimHit with highest weight). */

    double m_rootDepEG4;          /**< Deposited energy generated by Geant4. */
    double m_rootDepEDigi;          /**< Deposited energy generated by internal fluctuator. */

#endif //ROOT_OUTPUT

    double m_timeCPU;           /**< CPU time. */
    int m_nRun;               /**< Run number. */
    int m_nEvt;               /**< Event number. */

  }; // Class PXDDigiModule

} // Namespace Belle2

#endif // PXDDIGITIZER_H_

