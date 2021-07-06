/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/geometry/B2Vector3.h> // faster than TVector3

class TTree;
class TFile;

namespace Belle2 {

  class MCParticle;
  class ECLHit;
  class ECLDigit;
  class ECLCalDigit;
  class ECLLocalMaximum;
  class ECLConnectedRegion;

  namespace ECL {
    class ECLNeighbours;
    class ECLGeometryPar;
  }

  /** Class to find connected regions */
  class ECLLocalMaximumFinderModule : public Module {

  public:
    /** Constructor. */
    ECLLocalMaximumFinderModule();

    /** Destructor. */
    virtual ~ECLLocalMaximumFinderModule();

    /** Initialize. */
    virtual void initialize() override;

    /** Begin. */
    virtual void beginRun() override;

    /** Event. */
    virtual void event() override;

    /** End run. */
    virtual void endRun() override;

    /** Terminate (close ROOT files here if you have opened any). */
    virtual void terminate() override;

    /** Store array: MCParticle. */
    StoreArray<MCParticle> m_mcParticles;

    /** Store array: ECLHit. */
    StoreArray<ECLHit> m_eclHits;

    /** Store array: ECLDigit. */
    StoreArray<ECLDigit> m_eclDigits;

    /** Store array: ECLCalDigit. */
    StoreArray<ECLCalDigit> m_eclCalDigits;

    /** Store array: ECLConnectedRegion. */
    StoreArray<ECLConnectedRegion> m_eclConnectedRegions;

    /** Store array: ECLLocalMaximum. */
    StoreArray<ECLLocalMaximum> m_eclLocalMaximums;

    /** MCParticles.*/
    virtual const char* mcParticleArrayName() const
    { return "MCParticles" ; }

    /** Name to be used for default or PureCsI option: ECLHits.*/
    virtual const char* eclHitArrayName() const
    { return "ECLHits" ; }

    /** Name to be used for default or PureCsI option: ECLDigits.*/
    virtual const char* eclDigitArrayName() const
    { return "ECLDigits" ; }

    /** Name to be used for default or PureCsI option: ECLCalDigits.*/
    virtual const char* eclCalDigitArrayName() const
    { return "ECLCalDigits" ; }

    /** Name to be used for default option: ECLConnectedRegions.*/
    virtual const char* eclConnectedRegionArrayName() const
    { return "ECLConnectedRegions" ; }

    /** Name to be used for default option: ECLLocalMaximums.*/
    virtual const char* eclLocalMaximumArrayName() const
    { return "ECLLocalMaximums" ; }

    /** Reset Classifier Variables.*/
    void resetClassifierVariables();

    /** Reset Debug Variables.*/
    void resetTrainingVariables();

    /** Make local maximum for a given connected region. */
    void makeLocalMaximum(const ECLConnectedRegion& aCR, const int cellId, const int lmId);

    /** Add energy to vector. */
    void addToSignalEnergy(int motherpdg, int motherindex, int pi0index, double weight);

    /** Check if particle is produced outside of the ECL. */
    bool isEnteringECL(const B2Vector3D& vec);

    /** Get enterging mother of this particle. */
    void getEnteringMother(const MCParticle& particle, int& pdg, int& arrayindex, int& pi0arrayindex);

    /** Get Id position in the vector. */
    int getIdPosition(const int type, const int id);

    /** Get the highest energy deposition particle type. */
    void getMax(int& maxtype, int& maxpos);

  private:

    // Module parameters
    double m_energyCut; /**< energy cut for seed */
    int m_isTrainingMode; /**< training mode for MVA methods (i.e. create weights) */
    std::string m_outfileName; /**< file name prefix of the training output file */
    std::string m_method; /**< Method to find the local maximum */
    double m_truthFraction; /**< MC truth fraction */
    double m_cutOffset; /**< cut offset */
    double m_cutSlope; /**< cut slope. */
    double m_cutRatioCorrection; /**< correction for nominator and denominator of the ratio. */

    // Constants
    const double c_minEnergyCut = 5.0 * Belle2::Unit::MeV; /**< Minimum LM energy */

    /** vector (8736+1 entries) with cell id to store array positions */
    std::vector< int > m_StoreArrPosition;

    /** Neighbour maps. */
    ECL::ECLNeighbours* m_neighbourMap{nullptr};

    /** Geometry */
    ECL::ECLGeometryPar* m_geom{nullptr};

    /** Output training files and trees. */
    TFile* m_outfile{nullptr}; /**< outfiles that contain tree */
    TTree* m_tree{nullptr}; /**< tree that contain information for MVA training */

    /** Variables (possibly) used for MVA classification. Must be float for our framework mvas. */
    const static unsigned c_nMaxNeighbours = 12; /**< maximal number of neighbours (can be more than 8 only in the endcaps) */
    float m_energyRatioNeighbour[c_nMaxNeighbours] {}; /**< energy ratio of neighbour 0..9 to center */
    float m_time = 0.0;  /**< time */

    /** Variables to monitor the MVA training.*/
    float m_energy = 0.0; /**< energy of the center cell */
    float m_thetaId = 0.0;  /**< local maximum center theta Id */
    float m_phiId = 0.0;  /**< local maximum center theta Id */
    float m_cellId = 0.0;  /**< local maximum center cell Id */
    float m_maxNeighbourEnergy = 0.0;  /**< highest energy of all neighbours */
    float m_timeResolution = 0.0;  /**< time resolution */
    float m_timeFitFailed = 0.0;  /**< failed fit */
    float m_CRId = 0.0; /**< CR ID */
    float m_LMId = 0.0; /**< LM ID */

    /** MC truth variables */
    float m_target = 0.0; /**< target type */
    float m_targetindex = 0.0; /**< target array index */
    float m_targetpi0index = 0.0; /**< target array index */

    /** Variables (possibly) used for cut classification. */
    float m_nNeighbours10 = 0.0;  /**< Number of neighbours above 1.0 MeV. */
    float m_maxEnergyRatio = 0.0;  /**< Highest energetic neighbour energy divided by LM energy. */

    double m_totalSignalEnergy = 0.0; /**< total energy of this digit */
    double m_signalEnergy[10][5] {}; /**< total energy per MC matching type of this digit */
    int m_signalId[10][5] {}; /**< total energy per MC matching type of this digit */
  };

  /** Class to find connected regions, pureCsI version*/
  class ECLLocalMaximumFinderPureCsIModule : public ECLLocalMaximumFinderModule {
  public:
    /** Name to be used for PureCsI option: ECLHitsPureCsI.*/
    //virtual const char* eclHitArrayName() const override
    //{ return "ECLHitsPureCsI" ; }

    /** Name to be used for PureCsI option: ECLDigitsPureCsI.*/
    virtual const char* eclDigitArrayName() const override
    { return "ECLDigitsPureCsI" ; }

    /** Name to be used for PureCsI option: ECLCalDigitsPureCsI.*/
    virtual const char* eclCalDigitArrayName() const override
    { return "ECLCalDigitsPureCsI" ; }

    /** Name to be used for PureCsI option: ECLConnectedRegionsPureCsI.*/
    virtual const char* eclConnectedRegionArrayName() const override
    { return "ECLConnectedRegionsPureCsI" ; }

    /** Name to be used for PureCsI option: ECLLocalMaximumsPureCsI.*/
    virtual const char* eclLocalMaximumArrayName() const override
    { return "ECLLocalMaximumsPureCsI" ; }

  }; // end of ECLLocalMaximumFinderPureCsIModule

} // end of Belle2 namespace
