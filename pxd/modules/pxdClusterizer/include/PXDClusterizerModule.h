/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Zbynek Drasal, Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// /////////////////////////////////////////////////////////////////////// //
//                                                                         //
// PXDClusterizerModule - basf2 Clusterizer for the DEPFET pixel sensors   //
//                                                                         //
// /////////////////////////////////////////////////////////////////////// //

#ifndef PXDCLUSTERIZER_H_
#define PXDCLUSTERIZER_H_ 1

// Define ROOT output if needed
// PQ??: Can we turn it into a parameter?
// To determine error bars on hit positions, a calibration run will be done each
// time with the Clusterizer. "Each time" means each time the simulation changes.
// Otherwise, the available resolution calibration will be re-used, to the
// responsibility of the user.
#undef ROOT_OUTPUT

#include <framework/core/Module.h>

// Include Digi header files
#include <pxd/geopxd/SiGeoCache.h>

#include <pxd/dataobjects/PXDHit.h>
#include <pxd/dataobjects/RelationHolder.h>
#include <pxd/dataobjects/ClsDigit.h>
#include <pxd/modules/pxdDigitizer/PXDcheater.h>


// Include basic C++
#include <string>
#include <vector>
#include <map>

// Include ROOT classes
#ifdef ROOT_OUTPUT
#include <TH1F.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TFile.h>
#include <TTree.h>
#endif // ROOT_OUTPUT


namespace Belle2 {

// typedefs

  /* -- none at the moment -- */

  /**
   * PXDClusterizerModule - basf2 PXD Clusterizer Module.
   *
   * The PXDClusterizerModule is a basf2 module that carries out DEPFET PXD
   * clustering and hit reconstruction. <br>
   * This software is a direct ancestor of the clustering part of the SiPxlDigi
   * Marlin processor, developed by Zbynek Drasal for the ILC software framework.
   * <p>
   * PXDDigits created by a digitizer module are converted into cluster structures
   * containing a the StoreArray index of the original digit, its encoded position
   * within the sensor, seed flag, and cluster number. The cluster structures
   * are inserted in a boost::multi_index structure, and clusters are consecutively
   * formed from seed digits indexed by their position by examining their nearest
   * neighbors.<p>
   * Position of hits is estimated using either a center of
   * gravity (for clusters consisting of 1 or 2 pixels in either dimension,
   * or analog head-tail algorithm for clusters larger than 2 pixels in a
   * dimension.
   * As the Analog-To-Digital converter is going to be used in the data processing
   * pipeline, the analog signals are converted into digital values and then,
   * based on seed cuts and neighbor pixel cuts (user-adjustable), the clustering
   * is carried out. The output of the processor is a collection of PXDHits,
   * where each reconstructed hit has a defined position, total collected charge
   * (either analog signal (no ADC) in [electrons] or digital (with ADC) in [ADU].
   * (1 ADU = ADCRange/2^nADCBits), covariance matrix (position resolution, which
   * has to be calculated separately from results obtained with ROOT_OUTPUT on),
   * sensor identifier (from which ladder/layer/sensor the hit comes from), and
   * relations to MCParticles and PXDDigits that contributed. The latter relations
   * allow to reconstruct the digits of clusters that formed a given hit.
   * <p>
   * Hit position is corrected for mean Lorentz shift. As the hit resolution is
   * given as an external parameter and is not calculated automatically, one has
   * to perform the digitization with #define ROOT_OUTPUT set first, analyse the
   * histograms, calculate resolutions and supply them back as input parameters.
   * This procedure is hard to automate, but a different approach based on resampling
   * of deposited energy is planned.
   */

  class PXDClusterizerModule : public Module {

  public:

    /** Constructor.*/
    PXDClusterizerModule();

    /** Destructor.*/
    virtual ~PXDClusterizerModule();

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

    /** Compute PXDHit data from a cluster. */
    void makePXDHit(ClusterDigits& cluster, PXDHit* storeHit);

    // UTILITY

    /** Return collected charge in ADC units. */
    short int getInADCUnits(double charge) const
    { return static_cast<short>(charge / m_ADCUnit); }

    // PRINT METHODS

    /** Print clusters. */
    // TODO void printClusters(DigitMap& digits) const;

    /** Print module parameters. */
    void printModuleParams() const;

  private:
    // DATA MEMBERS

    // Collections
    std::string m_mcColName; /**< MC particles collection name. */
    std::string m_digitColName; /**< Digits collection name. */
    std::string m_relDigitName; /**< MCParticles-to-digits relation name. */
    std::string m_hitColName;   /**< PXDHits collection name. */
    std::string m_clusterColName; /**< Name Cluster-to-Digit relation coll. */
    std::string m_relHitName;   /** Name of MCParticle-to-PXDHit relation coll. */

    int m_nHitsSaved;         /**< Number of saved hits per event. */
    int m_nRelationsSaved;      /**< Number of saved relations per event. */

    // Digitization parameters - readout
    bool m_ADC;          /**< Simulate ADC or not. */
    int m_ADCRange;       /**< ADC range is from 0 - ? (in electrons). */
    int m_ADCBits;        /**< ADC has 0 - (2^? -1) digital values. */
    double m_ADCUnit;          /**< ADC unit in electrons. */
    double  m_elNoise;        /**< Electronic noise of individual pixels in e. */

    // PQ!!: These should be set by sensor! - will move to VolumeUserInfo.
    double m_SNAdjacent; /**< Threshold signal/noise level for zero suppression
                               and cluster membership.*/
    double m_SNSeed;     /**< Threshold signal/noise level for cluster seeds.*/
    double m_seedCut;    /**< Valid seed cut value (depends on charge/ADC units).*/
    double m_SNTotal;    /**< Threshold total cluster charge.*/
    double m_clusterChargeCut; /**< Valid cluster charge cut value (unit dependent).*/

    // Digitization parameters - charge transport
    // PQ??: Wonder if we can make use of this in clustering.
    // bool  m_doublePixel;        /**< Define if double pixel structure is used. */

    // Current sensor parameters from geometry
    short int m_currentLayerID;       /**< Current layer ID. */
    short int m_currentLadderID;      /**< Current ladder ID. */
    short int m_currentSensorID;      /**< Current sensor ID. */
    short int m_currentSensorUniID;   /**< Current compact ID (layer/ladder/sensor). */

    float m_sensorUPitch;         /**< Current sensor pitch in U (r-phi). */
    int m_sensorUCells;           /**< Number of cells in U (r-phi). */
    float m_sensorVPitch;         /**< Current sensor pitch in V (Z). */
    int m_sensorVCells;           /**< Number of cells in V (Z). */

    //TVector3 m_magField;          /**< Magnetic field in T in detector LRF!! Not used. */
    double  m_tanLorentzAngle;        /**< Tangent of Lorentz angle. */

    SiGeoCache * m_geometry;        /**< Geometry data on active sensors. */
    PXDCheater * m_cheater;         /**< Provides hit resolution estimates. */

    // Root output
#ifdef ROOT_OUTPUT

    TFile * m_rootFile;           /**< ROOT output file for resolution data. */
    TTree * m_rootTree;           /**< ROOT output tree for resolution data. */

    TH1F * m_rootMultiplicity;        /**< Histogram of hit multiplicity. */
    TH1F * m_rootEfficiency;        /**< Histogram of hit efficiency. */

    TH1F * m_rootOccupancy_0;       /**< Histogram of PXD layer 1 occupancy. */
    TH1F * m_rootOccupancy_1;       /**< Histogram of PXD layer 2 occupancy. */

    TH2F  * m_rootDigitsMap;        /**< Histogram used as a map of found digits. */

    std::map<int, int> m_rootMCGenHitMap; /**< Map of generated primary hits. */

    int m_rootLayerID;            /**< layer ID. */
    int m_rootLadderID;           /**< ladder ID. */
    int m_rootSensorID;           /**< sensor ID. */

    int m_rootMCPDG;            /**< PDG of the particle that created the current hit (MC Particle with highest weight). */
    int m_rootIsFromMCPrim;         /**< The reconstructed hit is from a primary MC particle. */
    int m_rootEvtNum;           /**< Event number to which the hit belongs. */

#endif //ROOT_OUTPUT

    double m_timeCPU;           /**< CPU time. */
    int m_nRun;               /**< Run number. */
    int m_nEvt;               /**< Event number. */

  }; // Class PXDClusterizerModule

} // Namespace Belle2

#endif // PXDDIGITIZER_H_

